"""Cross-check native common extents against independent PDB contributions.

Coordinates can agree without proving a module's physical object identity or
complete retained storage. A contribution may aggregate storage, so a larger
intersecting contribution is not automatically a distinct overlapping object.
"""

from collections import Counter, defaultdict
from dataclasses import asdict

from .common_allocation import _verify_common_inputs
from .common_preservation import _symbols
from .model import Result, digest
from .pdb_contributions import bound_contributions


def ordinary_contributions(context, inputs):
    """Join independently placed physical sections, not caller-supplied bounds.

    A V60 row has no original COFF section index. Module, extent and independent
    placement must all agree; duplicate or overlapping rows cannot select one.
    Unmatched candidates and contributions remain in the complete observations.
    """
    from .linked_modules import observe_modules
    from .linked_storage_census import observe_storage

    before = digest(context.report)
    _, fresh = inputs.objects._constant_population()
    try:
        context.check_current()
        modules = observe_modules(context, inputs)
        census = observe_storage(inputs)
        inventory = bound_contributions(inputs.image, inputs.artifacts["msvc-pdb"])
        if (inventory.image_sha256 != modules["image_sha256"]
                or inventory.pdb_sha256 != modules["pdb_sha256"]):
            raise ValueError("ordinary contribution observations belong to different image/PDB pairs")
        owners = defaultdict(list)
        for module in modules["modules"]:
            if module["result"] == Result.PASS:
                owners[digest(module["input"])].append(module)
        contributions = [{"index": index, **asdict(item), "address":
                          inputs.image.sections[item.section - 1][0].start + item.offset}
                         for index, item in enumerate(inventory.contributions)]
        by_section = defaultdict(list)
        for row in contributions:
            by_section[row["section"]].append(row)
        sections = []
        for candidate in census["inputs"]:
            for section in candidate["sections"]:
                if section["classification"] != "ordinary_data":
                    continue
                row = {"input": dict(candidate["input"]), "object_sha256": candidate["object_sha256"],
                       **{key: section[key] for key in ("object_section", "section_name", "flags", "size")},
                       "correspondence": Result.UNKNOWN, "characteristics_agreement": Result.UNKNOWN,
                       "intersections": []}
                sections.append(row)
                envelope = section.get("envelope")
                if envelope is None or envelope["placement"] != Result.PASS:
                    row["reason"] = "ordinary section lacks independent native placement"
                    continue
                if (envelope["input"] != row["input"] or envelope["object_sha256"] != row["object_sha256"]
                        or envelope["object_section"] != row["object_section"] or envelope["size"] != row["size"]):
                    raise ValueError("ordinary envelope differs from its physical census section")
                row["envelope"] = envelope
                begin, end = envelope["address"], envelope["address"] + envelope["size"]
                row["intersections"] = [item["index"] for item in by_section[envelope["image_section"]]
                                        if item["size"] > 0 and begin < item["address"] + item["size"]
                                        and item["address"] < end]
                physical = owners[digest(row["input"])]
                if len(physical) != 1 or physical[0]["object_sha256"] != row["object_sha256"]:
                    row["reason"] = "ordinary section lacks one independently bound physical PDB module"
                    continue
                module, = physical
                row["module"] = module["index"]
                if len(row["intersections"]) != 1:
                    row["reason"] = "ordinary section has no unique non-overlapping PDB contribution"
                    continue
                item = contributions[row["intersections"][0]]
                if (item["module"] != module["index"] or item["address"] != begin
                        or item["size"] != row["size"]):
                    row["reason"] = "PDB contribution has different module or whole-section coordinates"
                    continue
                # Reject contradictory kind, access or alignment bits. Retain
                # ALL bits separately: agreement here does not interpret or
                # normalize producer-specific flags into semantic equivalence.
                if (item["flags"] & 0x1F009A00
                        or (item["flags"] ^ row["flags"]) & 0xE0F000E0):
                    row["reason"] = "PDB contribution contradicts ordinary section form, kind, access or alignment"
                    continue
                row.update(correspondence=Result.PASS, contribution=item["index"],
                           characteristics_agreement=Result.PASS if item["flags"] == row["flags"] else Result.UNKNOWN)
        # Every independently placed competitor matters, even if its own
        # module/size proof failed. Filtering to successful joins could let
        # one section borrow a row also intersecting a different real section.
        matched = Counter(index for row in sections for index in row["intersections"])
        for row in sections:
            if row["correspondence"] == Result.PASS and matched[row["contribution"]] != 1:
                row.update(correspondence=Result.UNKNOWN,
                           reason="multiple independently placed ordinary sections intersect the same contribution")
        used = {row["contribution"] for row in sections if row["correspondence"] == Result.PASS}
        return {"check": "linked_ordinary_contributions", "result": Result.UNKNOWN,
                "modules": modules, "census": census, "sections": sections, "contributions": contributions,
                "unmatched_contributions": [row["index"] for row in contributions if row["index"] not in used],
                "counts": dict(Counter(row["correspondence"] for row in sections)),
                "image_sha256": inventory.image_sha256, "pdb_sha256": inventory.pdb_sha256,
                "retained_storage_coverage": Result.UNKNOWN, "allocation_isolation": Result.UNKNOWN,
                "contents": Result.UNKNOWN, "remaining_linked_effects": Result.UNKNOWN,
                "persisted": False, "ready_for_collection": False,
                "limitations": ["independent physical ordinary-section/PDB correspondence only",
                                "unmatched contributions or sections have no retention, discard or placement exemption",
                                "raw characteristics remain distinct; no unexplained flag transformation is certified",
                                "COMDAT, imports, common and linker-generated storage require separate proof",
                                "no contents, reference, isolation, residual coverage or acceptance permission"]}
    finally:
        try:
            fresh()
            if digest(context.report) != before:
                raise ValueError("ordinary contribution application report changed during observation")
        finally:
            context.check_current()


def common_contributions(inputs, symbols):
    names = _symbols(symbols)
    commons = _verify_common_inputs(inputs.evidence, inputs.coverage, inputs.records,
                                    inputs.image, inputs.mapping, names)
    return _common_contributions(inputs, commons)


def _common_contributions(inputs, commons):
    """Internal same-epoch composition; never accept saved allocation reports."""
    pdb = inputs.artifacts.get("msvc-pdb")
    if pdb is None:
        raise ValueError("common contribution checks require this build's PDB sidecar")
    inventory = bound_contributions(inputs.image, pdb)
    rows = []
    for common in commons["allocations"]:
        row = {"symbol": common["symbol"], "correspondence": Result.UNKNOWN, "intersections": []}
        rows.append(row)
        if common["result"] != Result.PASS:
            row["reason"] = "native common extent and initial contents are unverified"
            continue
        begin, end = common["address"], common["address"] + common["size"]
        for index, contribution in enumerate(inventory.contributions):
            start = inputs.image.sections[contribution.section - 1][0].start + contribution.offset
            if contribution.size > 0 and begin < start + contribution.size and start < end:
                module = inventory.modules[contribution.module]
                row["intersections"].append({"index": index, **asdict(contribution),
                    "address": start, "module_name": module.name, "object_name": module.object_name})
        matches = row["intersections"]
        if len(matches) == 1:
            item, = matches
            # This is an exact coordinate/extent cross-check, not a module
            # selection contract. Keep every characteristic bit in diagnostics.
            if (item["address"] == begin and item["size"] == common["size"]
                    and item["flags"] & 0xE0000000 == 0xC0000000
                    and item["module_name"] == "* Linker *" and not item["object_name"]):
                row["correspondence"] = Result.PASS
        if row["correspondence"] != Result.PASS:
            row["reason"] = "no unique exact linker-labelled contribution extent"
    return {"check": "linked_common_contributions", "result": Result.UNKNOWN,
            "common_allocations": commons, "contributions": rows,
            "image_sha256": inventory.image_sha256, "pdb_sha256": inventory.pdb_sha256,
            "contribution_count": len(inventory.contributions), "module_count": len(inventory.modules),
            "empty_contributions": [{"index": index, **asdict(row)}
                                    for index, row in enumerate(inventory.contributions) if row.size == 0],
            "allocation_isolation": Result.UNKNOWN, "retained_storage_coverage": Result.UNKNOWN,
            "physical_object_binding": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
            "limitations": ["exact common/PDB coordinates do not prove complete storage or module identity",
                            "an overlapping contribution can aggregate the same storage, not a distinct allocation",
                            "no flags are normalized, rows discarded, or common coverage permission granted"]}
