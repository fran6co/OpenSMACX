"""Native physical section obligations, independent of executable reachability.

This is an input domain for live application comparison, not retained-selection
proof. In particular, no missing MAP symbol makes a physical section disappear.
"""

from collections import defaultdict
from contextlib import ExitStack
from copy import deepcopy

from .linked_code_ownership import _contents
from .linked_code_region import select_code_region
from .linked_input_epoch import input_epoch, _borrow_epoch
from .linked_input_selection import observe_input_selection
from .linked_storage_census import _identity, _section_rows
from .model import Result, digest
from .pe_coff import Coff, CoffRecords


def observe_sections(context, objects, *, epoch=None, native=None, max_sections=100000):
    """Enumerate every candidate, and seed only independently selected inputs.

    Anonymous/unsupported sections and ambiguous families remain obligations.
    Roots describe physical code entries, never guessed source function bounds.
    A returned root or data section still needs its native placement/content check.
    """
    if type(max_sections) is not int or not 1 <= max_sections <= 1000000:
        raise ValueError("application section domain requires a bounded section limit")
    if native is not None:
        from .linked_context import LinkedBuild
        if (type(native) is not LinkedBuild or native.context is not context
                or native.inputs.objects is not objects):
            raise ValueError("section selection requires this exact native linked context")
    if epoch is None:
        with input_epoch(context, objects) as owned:
            return observe_sections(context, objects, epoch=owned, native=native, max_sections=max_sections)
    with ExitStack() as stack:
        state = stack.enter_context(_borrow_epoch(epoch, context, objects))
        choices = None
        if native is not None:
            from .linked_comdat_selection import comdat_selection
            choices = stack.enter_context(comdat_selection(native, epoch=epoch))
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
                if choices is not None and section.flags & 0x1000:
                    refusal = None
                    try:
                        duplicate = choices.discarded(owner, number)
                        if duplicate is None:
                            duplicate = choices.loser(owner, number)
                    except ValueError as error:
                        refusal = error
                        duplicate = None
                    if duplicate is None:
                        # No removal certificate: a separately placed NODUP root
                        # is positive retention evidence, never a census
                        # exclusion. Placed roots fall through and stay code
                        # roots with all their obligations. The placement proof
                        # refuses genuine REF/ICF contradictions itself, so an
                        # unsupported removal form (for example a private root)
                        # does not block its own placement annotation.
                        try:
                            placement = choices.placed(owner, number)
                        except ValueError as placement_error:
                            # A refused selection leaves the original physical
                            # root and section obligation. Missing MAP is never
                            # discard.
                            gap("COMDAT selection remains unqualified: "
                                + "; ".join(text for text in (str(refusal), str(placement_error)) if text), number)
                            placement = None
                        else:
                            if placement is None and refusal is not None:
                                gap("COMDAT selection remains unqualified: " + str(refusal), number)
                        if placement is not None:
                            if (placement["queried_section"] != number
                                    or number not in placement["sections"]):
                                raise ValueError("COMDAT placement does not cover its queried physical section")
                            if placement["disposition"] == "selected_any_winner":
                                item.update(retention=Result.PASS,
                                            disposition=placement["disposition"],
                                            selection_certificate=placement["certificate_id"],
                                            root_identity_kind=placement["root_identity_kind"])
                            else:
                                item.update(retention=Result.PASS,
                                            disposition=placement["disposition"],
                                            selection_placement=placement["placement_id"],
                                            root_identity_kind=placement["root_identity_kind"])
                    if duplicate is not None:
                        if (duplicate["queried_section"] != number
                                or number not in duplicate["sections"]):
                            raise ValueError("COMDAT certificate does not cover its queried physical section")
                        if duplicate["disposition"] not in ("discarded_any_duplicate", "discarded_unreferenced_comdat"):
                            raise ValueError("COMDAT certificate has an unsupported disposition")
                        disposition = ("unselected_comdat_duplicate" if duplicate["disposition"] == "discarded_any_duplicate"
                                       else "discarded_unreferenced_comdat")
                        item.update(retention=Result.PASS, disposition=disposition,
                                    selection_certificate=duplicate["certificate_id"])
                        # Keep the original input, section, symbols and all
                        # association descendants visible. This does not lend
                        # the winner's source identity to an unselected copy.
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
        selected_sections = [section for row in rows for section in row["sections"]]
        retained = (Result.PASS if selected_sections
                    and all(section["retention"] == Result.PASS for section in selected_sections)
                    else Result.UNKNOWN)
        return {"check": "application_physical_section_domain", "result": Result.UNKNOWN,
                "enumeration": Result.PASS, "selection": selection, "inputs": rows,
                "section_count": count, "code_roots": roots, "data_sections": data, "gaps": gaps,
                "comdat_selection_certificates": choices.certificates() if choices is not None else [],
                "comdat_folded_groups": choices.folded_groups() if choices is not None else [],
                "comdat_unique_placements": choices.unique_placements() if choices is not None else [],
                "ref_discard_observations": choices.discard_observations() if choices is not None else None,
                "retained_section_selection": retained, "ready_for_collection": False}
