"""Cross-check native common extents against independent PDB contributions.

Coordinates can agree without proving a module's physical object identity or
complete retained storage. A contribution may aggregate storage, so a larger
intersecting contribution is not automatically a distinct overlapping object.
"""

from dataclasses import asdict

from .common_allocation import _verify_common_inputs
from .common_preservation import _symbols
from .model import Result
from .pdb_contributions import bound_contributions


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
            if begin < start + contribution.size and start < end:
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
            "allocation_isolation": Result.UNKNOWN, "retained_storage_coverage": Result.UNKNOWN,
            "physical_object_binding": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
            "limitations": ["exact common/PDB coordinates do not prove complete storage or module identity",
                            "an overlapping contribution can aggregate the same storage, not a distinct allocation",
                            "no flags are normalized, rows discarded, or common coverage permission granted"]}
