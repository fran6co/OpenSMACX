"""Compose live common retention without granting a byte or declaration waiver.

Uniform physical requests, selected placement and exact PDB extent establish
retention separately from producer characteristics. The native caller owns the
epoch and complete ordinary-section census; saved reports grant no authority.
"""

from collections import Counter

from .common_allocation import _verify_common_inputs
from .linked_contributions import _common_contributions
from .linked_interval_index import IntervalIndex
from .model import Result


def observe_common_allocations(inputs, sections):
    """Private same-epoch composition, including unsuccessful competitors."""
    names = tuple(sorted({symbol["name"] for record in inputs.records for symbol in record["symbols"]
                          if symbol["declaration"] == "common"}
                         | {row.symbol for row in inputs.mapping.publics if row.owner == "<common>"}))
    if len(names) > 100000:
        raise ValueError("common allocation population exceeds verification resource bound")
    gaps, rows = [], []
    if names:
        commons = _verify_common_inputs(inputs.evidence, inputs.coverage, inputs.records,
                                       inputs.image, inputs.mapping, names)
        joined = _common_contributions(inputs, commons)
        for common, match in zip(commons["allocations"], joined["contributions"], strict=True):
            row = {"symbol": common["symbol"], "proof": common, "correspondence": match,
                   "input_consistency": common["result"], "retention": Result.UNKNOWN,
                   "characteristics_agreement": Result.UNKNOWN, "allocation_isolation": Result.UNKNOWN}
            rows.append(row)
            if common.get("extent_verification") == Result.PASS:
                row["envelope"] = {key: common[key] for key in ("address", "size")}
            if common["result"] != Result.PASS or match["correspondence"] != Result.PASS:
                gaps.append({"symbol": row["symbol"], "reason": "common allocation lacks verified exact native correspondence"})
                continue
            item, = match["intersections"]
            row["pdb_flags"] = item["flags"]
            flags = item["flags"]
            alignment = (flags >> 20) & 15
            # Common requests do not have an input section header. Check the
            # declared BSS/RW form and actual alignment, retaining the known
            # producer's reserved bit as an explicit unsatisfied obligation.
            if (flags & ~0x00F02000 != 0xC0000080 or not 1 <= alignment <= 14
                    or common["address"] % (1 << (alignment - 1))):
                gaps.append({"symbol": row["symbol"], "reason": "unsupported common contribution characteristics or alignment"})
                continue
            begin, end = common["address"], common["address"] + common["size"]
            if inputs.fixups is None or any(begin < inputs.image.base + site + 4
                    and inputs.image.base + site < end for site in inputs.fixups):
                gaps.append({"symbol": row["symbol"], "reason": "common storage lacks independent absence of PDB fixup writes"})
                continue
            row.update(retention=Result.PASS, contribution=item["index"])
            if flags & 0x2000:
                gaps.append({"symbol": row["symbol"], "reason": "common contribution characteristics remain unexplained"})
            else:
                row["characteristics_agreement"] = Result.PASS
    uses = Counter(row["contribution"] for row in rows if "contribution" in row)
    ordinary = IntervalIndex()
    for index, section in enumerate(sections):
        if "envelope" in section and section["size"] > 0:
            begin = section["envelope"]["address"]
            ordinary.insert(index, begin, begin + section["size"])
    common_ranges = IntervalIndex()
    for index, row in enumerate(rows):
        if "envelope" in row:
            span = row["envelope"]
            common_ranges.insert(index, span["address"], span["address"] + span["size"])
    for index, row in enumerate(rows):
        if "contribution" in row and uses[row["contribution"]] != 1:
            row["retention"] = Result.UNKNOWN
            gaps.append({"symbol": row["symbol"], "reason": "common contribution has multiple allocation claimants"})
        if "envelope" not in row:
            continue
        begin = row["envelope"]["address"]
        end = begin + row["envelope"]["size"]
        row["ordinary_overlaps"] = [other for other, _, _ in ordinary.overlaps(begin, end)]
        row["common_overlaps"] = [other for other, _, _ in common_ranges.overlaps(begin, end) if other != index]
        if row["ordinary_overlaps"] or row["common_overlaps"]:
            row.update(retention=Result.UNKNOWN, allocation_isolation=Result.FAIL)
            for other in row["ordinary_overlaps"]:
                sections[other]["allocation_isolation"] = Result.FAIL
            gaps.append({"symbol": row["symbol"], "reason": "common storage intersects another independent allocation"})
    damaged = any(row["input_consistency"] == Result.FAIL for row in rows)
    overlap = any(row["allocation_isolation"] == Result.FAIL for row in rows)
    return {"check": "common_allocation_coverage", "allocations": rows, "gaps": gaps,
            "result": Result.FAIL if damaged or overlap else Result.UNKNOWN if gaps else Result.PASS,
            "input_consistency": Result.FAIL if damaged else Result.UNKNOWN if gaps else Result.PASS,
            "allocation_isolation": Result.FAIL if overlap else Result.UNKNOWN,
            "retained_contributions": sorted({row["contribution"] for row in rows if row["retention"] == Result.PASS}),
            "remaining_linked_effects": Result.UNKNOWN, "ready_for_collection": False, "persisted": False}
