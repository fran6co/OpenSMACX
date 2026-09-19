"""Image-backed linker thunk storage, not physical input selection or acceptance.

The enclosing native epoch binds these observations to its fresh PE/PDB pair.
An import-shaped byte string alone cannot close a contribution obligation.
"""

from .binary import Relocation, RelocationKind, compare_bound_code
from .linked_imports import import_slots
from .linked_interval_index import IntervalIndex
from .model import Result, Span


def assign_linker_contributions(contributions, modules, image):
    """Assign only isolated executable thunk extents with exact loader fixups.

    This establishes a supported output recipe, not its physical library producer,
    input retention, allocation isolation or equivalence across application builds.
    Unknown recipes and damaged evidence remain unmatched.
    """
    by_index = {}
    for row in modules:
        if row["index"] in by_index:
            raise ValueError("duplicate linker contribution module index")
        by_index[row["index"]] = row
    intervals = IntervalIndex()
    for position, row in enumerate(contributions):
        if row["size"] > 0:
            intervals.insert(position, row["address"], row["address"] + row["size"])
    # Use the same independently validated readable, non-executable loader
    # slots as native short-import verification, not raw descriptor addresses.
    slots = None
    slot_error = None
    records = []
    for row in contributions:
        module = by_index.get(row["module"])
        if module is None or module.get("name") != "* Linker *":
            continue
        record = {"index": row["index"], "address": row["address"], "size": row["size"],
                  "module": row["module"], "assignment": None, "result": Result.UNKNOWN}
        records.append(record)
        try:
            if module.get("kind") != "linker_generated" or module.get("object_name") != "":
                raise ValueError("linker label lacks native generated-module identity")
            if row["size"] != 6:
                raise ValueError("unsupported linker contribution recipe extent")
            span = Span(row["address"], row["address"] + row["size"])
            matches = [(number, region, flags) for number, ((region, _, _), flags) in enumerate(
                zip(image.sections, image.section_flags, strict=True), 1)
                if region.start <= span.start and span.end <= region.end]
            if len(matches) != 1:
                raise ValueError("linker thunk lacks one wholly containing image section")
            number, region, flags = matches[0]
            if row.get("section") != number or row.get("offset") != span.start - region.start:
                raise ValueError("linker contribution and image coordinates disagree")
            # Code/read/execute, not writable or initialized/uninitialized data.
            if flags & 0xE00000E0 != 0x60000020 or row.get("flags", 0) & 0xE00000E0 != 0x60000020:
                raise ValueError("linker thunk requires executable code characteristics")
            if len(intervals.overlaps(span.start, span.end)) != 1:
                raise ValueError("linker contribution overlaps another contribution")
            if slots is None and slot_error is None:
                try:
                    slots = import_slots(image)
                except ValueError as error:
                    slot_error = str(error)
            if slot_error is not None:
                raise ValueError(slot_error)
            data = image.read(span)
            if len(data) != 6 or data[:2] != b"\xff\x25":
                raise ValueError("unsupported linker thunk instruction recipe")
            target = int.from_bytes(data[2:6], "little")
            bound = slots.get(target)
            if bound is None:
                raise ValueError("linker thunk operand does not name a validated loader slot")
            proof = compare_bound_code(span, data, b"\xff\x25\0\0\0\0",
                (Relocation(2, RelocationKind.ABSOLUTE32, "import_slot"),),
                {"import_slot": bound["address"]}, original_absolute_sites=image.absolute_sites(span))
            record["proof"] = proof
            if proof["result"] != Result.PASS:
                raise ValueError("linker thunk bytes or loader relocation disagree with recipe")
            record.update(result=Result.PASS,
                          assignment={"kind": "import_thunk", "module_name": bound["module"],
                                      "symbol": bound["name"], "ordinal": bound["ordinal"],
                                      "slot": f"0x{target:08X}"})
        except ValueError as error:
            record["reason"] = str(error)
    assigned = sum(record["result"] == Result.PASS for record in records)
    return {"check": "linker_generated_storage",
            "result": Result.PASS if assigned == len(records) else Result.UNKNOWN,
            "records": records, "assigned": assigned, "unassigned": len(records) - assigned,
            "limitations": ["verified thunk recipes only, not whole linker-output attribution",
                            "assigned storage is generated identity, never product retention",
                            "unsupported linker contributions stay unmatched and keep the domain open"],
            "ready_for_collection": False, "persisted": False}
