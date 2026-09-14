"""Complete physical storage census, without inventing retained allocations.

Enumerate the conservative native input population, including explicit objects
without final labels and every candidate in ambiguous archive families. Section
enumeration, independent placement and retention are different proof dimensions.
This observer cannot grant isolation, contents preservation or task acceptance.
"""

from collections import Counter, defaultdict
from hashlib import sha256
from pathlib import Path, PurePosixPath, PureWindowsPath

from .coff_archive import CoffArchive
from .common_allocation import _policy
from .linked_objects import _owner, _read
from .linked_storage import _data_sections
from .model import Result
from .pe_coff import CoffRecords


def _identity(record):
    return {key: record[key] for key in ("path", "member", "occurrence", "generated") if key in record}


def _candidates(row, records):
    """Repeat the native candidate relation, never a basename selection grant."""
    name = PureWindowsPath(row["member"]).name.casefold()
    if row["library"] is None:
        return [index for index, record in enumerate(records) if "member" not in record
                and Path(record["path"]).name.casefold() == name]
    return [index for index, record in enumerate(records) if "member" in record
            and record["path"] in row["paths"]
            and PurePosixPath(record["member"]).name.casefold() == name]


def _classification(section, size):
    if not size:
        return "zero_extent_with_writes" if any(kind != 0 for _, _, kind in section.relocations) else "empty"
    name = section.name.lower()
    if name == ".drectve":
        return "linker_directives"
    if name.startswith(".debug"):
        return "debug_or_metadata"
    if name.startswith((".tls", ".idata", ".edata", ".rsrc")):
        return "special_linker_storage"
    kind = section.flags & 0xE0
    if kind == 0x20:
        return "code"
    if kind in (0x40, 0x80):
        return "comdat_data" if section.flags & 0x1000 else "ordinary_data"
    return "unsupported_section"


def observe_storage(inputs, *, max_sections=100000):
    """Derive all candidate sections from one guarded native linked epoch.

    No caller supplies owners, extents, exclusions or previous census reports.
    Candidate-only archive sections remain visible without asserting retention
    or discard. Import headers and commons are separate storage obligations.
    The enclosing LinkedBuild owns before/final output and parsed-state guards.
    """
    if type(max_sections) is not int or not 1 <= max_sections <= 1000000:
        raise ValueError("storage census requires an exact bounded section limit")
    objects = inputs.objects
    records, fresh = objects._constant_population()
    try:
        fresh()
        owners, gaps = defaultdict(list), []
        counts = Counter(_owner(row).casefold() for row in inputs.coverage["inputs"])
        for row in inputs.coverage["inputs"]:
            owner = _owner(row)
            candidates = _candidates(row, records)
            if (not candidates or type(row["candidate_count"]) is not int
                    or len(candidates) != row["candidate_count"]
                    or set(row["paths"]) != {records[index]["path"] for index in candidates}):
                raise ValueError("storage census final family differs from physical candidates")
            for index in candidates:
                owners[index].append(owner)
            if counts[owner.casefold()] != 1 or len(candidates) != 1 or row["selection"] != Result.PASS:
                gaps.append({"owner": owner, "reason": "ambiguous final-pass storage owner",
                             "candidates": [_identity(records[index]) for index in candidates]})
        files, archives, observations, regions = {}, {}, [], []
        section_count, complete = 0, True
        policy = _policy(inputs.evidence, records)
        gaps.extend({"reason": "unsupported linker policy or directive", "detail": row} for row in policy)
        with _data_sections(inputs) as observe:
            for index, record in enumerate(records):
                identity = _identity(record)
                bound = owners[index]
                participation = ("final_pass" if bound else "archive_candidate_only"
                                 if "member" in identity else "explicit_unobserved")
                row = {"input": identity, "object_sha256": record["sha256"], "format": record["format"],
                       "owners": list(bound), "participation": participation, "sections": [],
                       "common_requests": [dict(symbol) for symbol in record["symbols"]
                                           if symbol["declaration"] == "common"],
                       "nonsection_declarations": [dict(symbol) for symbol in record["symbols"]
                                                   if symbol["section"] <= 0],
                       "imports": record["imports"], "directives": record["directives"]}
                observations.append(row)
                if participation == "explicit_unobserved":
                    gaps.append({"input": identity, "reason": "explicit object has no final-pass owner; retention unproved"})
                for kind in ("common_requests", "imports", "directives"):
                    if row[kind] and participation != "archive_candidate_only":
                        gaps.append({"input": identity, "reason": kind + " require separate linked-effect verification"})
                if record["format"] != "coff":
                    # Short headers produce linker storage despite lacking
                    # ordinary COFF sections. No zero-section absence waiver.
                    if participation != "archive_candidate_only":
                        gaps.append({"input": identity, "reason": "non-COFF input can generate linked storage"})
                    continue
                path = identity["path"]
                if path not in files:
                    files[path] = _read(path, objects._stamps[path])
                raw = files[path]
                if "member" in identity:
                    if path not in archives:
                        archives[path] = CoffArchive(raw)
                    raw = archives[path].members[identity["member"]][identity["occurrence"]]
                if sha256(raw).hexdigest() != record["sha256"]:
                    raise ValueError("storage census object differs from physical inventory")
                try:
                    obj = CoffRecords(raw)
                except ValueError as error:
                    complete = False
                    gaps.append({"input": identity, "reason": "unreadable section inventory: " + str(error)})
                    continue
                section_count += len(obj.sections)
                if section_count > max_sections:
                    raise ValueError("storage census exceeds its section resource bound")
                selected = None
                if len(bound) == 1 and counts[bound[0].casefold()] == 1:
                    try:
                        selected = objects.resolve(bound[0])
                    except ValueError as error:
                        gaps.append({"input": identity, "reason": str(error)})
                    if selected is not None and (selected["identity"] != identity
                                                or selected["sha256"] != record["sha256"]):
                        raise ValueError("storage census selected owner differs from physical candidate")
                for number, (section, size) in enumerate(zip(obj.sections, obj.section_sizes), 1):
                    category = _classification(section, size)
                    symbols = [dict(symbol) for symbol in record["symbols"] if symbol["section"] == number]
                    if category == "empty" and any(symbol["name"] != section.name
                            or symbol["storage"] != 3 or symbol["value"] != 0 or symbol["type"] != 0
                            or symbol["auxiliary_size"] != 18 for symbol in symbols):
                        category = "zero_extent_with_symbols"
                    item = {"object_section": number, "section_name": section.name, "size": size,
                            "flags": section.flags, "classification": category, "symbols": symbols,
                            "placement": Result.UNKNOWN}
                    row["sections"].append(item)
                    if category == "ordinary_data" and selected is not None:
                        try:
                            envelope = observe(bound[0], number)
                            item.update(placement=envelope["placement"], envelope=envelope)
                            regions.append(envelope)
                        except (OSError, ValueError) as error:
                            gaps.append({"input": identity, "object_section": number, "reason": str(error)})
                    elif category not in ("code", "empty") and participation != "archive_candidate_only":
                        gaps.append({"input": identity, "object_section": number,
                                     "reason": "unplaced or separately verified section: " + category})
        return {"check": "linked_storage_census", "result": Result.UNKNOWN,
                "enumeration": Result.PASS if complete else Result.UNKNOWN,
                "input_count": len(observations), "section_count": section_count,
                "positive_section_count": sum(section["size"] > 0 for row in observations for section in row["sections"]),
                "inputs": observations, "section_envelopes": regions, "gaps": gaps,
                "coverage": "complete_physical_candidate_population", "retained_data_coverage": Result.UNKNOWN,
                "allocation_isolation": Result.UNKNOWN, "remaining_linked_effects": Result.UNKNOWN,
                "persisted": False, "ready_for_collection": False,
                "limitations": ["enumeration does not prove which candidate sections were retained or discarded",
                                "code and empty sections are enumerated, not granted an exclusion",
                                "commons, imports, directives, debug and other generated storage need separate verification",
                                "placed ordinary envelopes do not prove contents, isolation or task acceptance"]}
    finally:
        fresh()
