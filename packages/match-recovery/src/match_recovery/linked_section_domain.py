"""Native physical section obligations, independent of executable reachability.

This is an input domain for live application comparison, not retained-selection
proof. In particular, no missing MAP symbol makes a physical section disappear.
"""

from collections import defaultdict
from copy import deepcopy

from .linked_code_ownership import _contents
from .linked_code_region import select_code_region
from .linked_input_epoch import input_epoch, _borrow_epoch
from .linked_input_selection import observe_input_selection
from .linked_storage_census import _identity, _section_rows
from .model import Result, digest
from .pe_coff import Coff, CoffRecords


def observe_sections(context, objects, *, epoch=None, max_sections=100000):
    """Enumerate every candidate, and seed only independently selected inputs.

    Anonymous/unsupported sections and ambiguous families remain obligations.
    Roots describe physical code entries, never guessed source function bounds.
    A returned root or data section still needs its native placement/content check.
    """
    if type(max_sections) is not int or not 1 <= max_sections <= 1000000:
        raise ValueError("application section domain requires a bounded section limit")
    if epoch is None:
        with input_epoch(context, objects) as owned:
            return observe_sections(context, objects, epoch=owned, max_sections=max_sections)
    with _borrow_epoch(epoch, context, objects) as state:
        selection = observe_input_selection(context, objects, epoch=epoch)
        records = {digest(_identity(record)): record for record in state.records}
        if len(records) != len(state.records):
            raise ValueError("application domain has duplicate physical input identities")
        families = defaultdict(list)
        for family in selection["families"]:
            for candidate in family["candidates"]:
                families[digest(candidate)].append(family)
        rows, roots, data, gaps = [], [], [], []
        count = 0
        for candidate in selection["inventory"]:
            bound = families[digest(candidate)]
            selected = len(bound) == 1 and bound[0]["selection"] == Result.PASS
            participation = ("final_pass" if bound else "archive_candidate_only"
                             if "member" in candidate["input"] else "explicit_unobserved")
            owner = bound[0]["owner"] if selected else None
            row = {**deepcopy(candidate), "families": [family["index"] for family in bound],
                   "owner": owner, "participation": participation, "sections": [],
                   "selection": Result.PASS if selected else Result.UNKNOWN}
            rows.append(row)

            def gap(reason, number=None):
                gaps.append({"input": deepcopy(candidate["input"]), "reason": reason,
                             **({"object_section": number} if number is not None else {})})

            if participation != "archive_candidate_only" and not selected:
                gap("physical input lacks one independently selected final-pass owner")
            record = records[digest(candidate["input"])]
            if record["sha256"] != candidate["object_sha256"]:
                raise ValueError("application section domain differs from its physical candidate")
            row["nonsection_declarations"] = [deepcopy(symbol) for symbol in record["symbols"]
                                               if symbol["section"] <= 0]
            row["directives"] = deepcopy(record["directives"])
            row["imports"] = deepcopy(record["imports"])
            raw = _contents(candidate["input"], candidate["object_sha256"], state.inputs)
            if candidate["format"] != "coff":
                if participation != "archive_candidate_only":
                    gap("non-COFF input requires separate generated-section verification")
                continue
            obj = CoffRecords(raw)
            count += len(obj.sections)
            if count > max_sections:
                raise ValueError("application section domain exceeds its section bound")
            code = None
            for item in _section_rows(obj, record):
                number, category, symbols = item["object_section"], item["classification"], item["symbols"]
                section = obj.sections[number - 1]
                item.update(relocations=[list(value) for value in section.relocations], retention=Result.UNKNOWN)
                row["sections"].append(item)
                if not selected:
                    continue
                if category == "ordinary_data":
                    data.append({"owner": owner, "section": number})
                elif category == "code":
                    # Ordinary sections are whole-section regions. One entry
                    # places all their bytes/labels; COMDAT entries have the
                    # existing exact function/bundle verifier instead.
                    entries = []
                    try:
                        if code is None:
                            code = Coff(raw)
                        for symbol in symbols:
                            if symbol["storage"] not in (2, 3) or not symbol["type"] & 0x20:
                                continue
                            try:
                                entry = select_code_region(code, symbol["name"])
                            except ValueError:
                                continue
                            if entry.section == number:
                                entries.append(entry.name)
                    except ValueError as error:
                        gap(str(error), number)
                    if not entries:
                        gap("physical code section lacks a supported bounded entry; absence is not discard", number)
                    else:
                        roots.append({"owner": owner, "symbol": sorted(entries)[0]})
                else:
                    gap("section requires separate retention verification: " + category, number)
        return {"check": "application_physical_section_domain", "result": Result.UNKNOWN,
                "enumeration": Result.PASS, "selection": selection, "inputs": rows,
                "section_count": count, "code_roots": roots, "data_sections": data, "gaps": gaps,
                "retained_section_selection": Result.UNKNOWN, "ready_for_collection": False}
