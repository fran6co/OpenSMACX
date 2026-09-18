"""Verified linker-generated storage: import thunks the image itself proves.

A ``* Linker *`` PDB module has no physical object, so its contributions can
never join one. Each assigned row carries a complete recipe verification
against the live image: the contribution's bytes must be exactly the VC6
import-thunk spelling, and its operand must name a slot from the image's own
import directory. Anything else stays unmatched; assignment is never inferred
from an address, size or module alone.
"""

from .model import Result, Span


def assign_linker_contributions(contributions, modules, image):
    """Assign linker-module contributions to verified import-thunk recipes.

    Returns one record per linker contribution. A pass record names the
    import its thunk dispatches; an unmatched record keeps its contribution
    in the caller's unmatched set. This observes generated storage only -
    it grants no retention, isolation or acceptance.
    """
    slots = {}
    for item in image.imports():
        slots.setdefault(item.address, item)
    by_index = {row["index"]: row for row in modules}
    records = []
    for row in contributions:
        module = by_index.get(row["module"])
        if module is None or module.get("name") != "* Linker *":
            continue
        record = {"index": row["index"], "address": row["address"], "size": row["size"],
                  "module": row["module"], "assignment": None, "result": Result.UNKNOWN}
        try:
            data = image.read(Span(row["address"], row["address"] + row["size"]))
        except ValueError:
            data = b""
        # VC6 spelling: jmp dword ptr [slot] -> FF 25 <abs32>, the operand
        # bound to a slot the image's import directory itself declares.
        if row["size"] == 6 and len(data) == 6 and data[:2] == b"\xff\x25":
            target = int.from_bytes(data[2:6], "little")
            bound = slots.get(target)
            if bound is not None:
                record.update(result=Result.PASS,
                              assignment={"kind": "import_thunk", "module_name": bound.module,
                                          "symbol": bound.name, "ordinal": bound.ordinal,
                                          "slot": f"0x{target:08X}"})
        records.append(record)
    assigned = [record for record in records if record["result"] == Result.PASS]
    return {"check": "linker_generated_storage",
            "result": Result.UNKNOWN if any(record["result"] != Result.PASS for record in records)
            else Result.PASS if records else Result.PASS,
            "records": records,
            "assigned": len(assigned),
            "unassigned": len(records) - len(assigned),
            "limitations": ["verified thunk recipes only, not whole linker-output attribution",
                            "assigned storage is generated identity, never product retention",
                            "unsupported linker contributions stay unmatched and keep the domain open"],
            "ready_for_collection": False, "persisted": False}
