"""Resolution: one record per address, and the map held against the catalogue.

The parsing machinery - the lesson tokens, region extraction, store
recognition, the tree walk - lives in `reader`, and the patterns and the
marker-recognition rule live in `grammar`. What stays here is the layer
that decides BETWEEN records: `resolve` settles several claimants on one
address by precedence, collapsing two spellings of the same code and
surfacing genuine conflicts rather than deciding them, and
`cross_reference` holds the resolved map against a catalogue and reports
drift without failing on it.
"""

from __future__ import annotations

from .model import DecompilationState

# ----------------------------------------------------------------- resolving


def _precedence(annotation: DecompilationState) -> tuple[int, ...]:
    """Which claimant owns an address when several annotate it.

    Lower wins. A proved body (byte-exact, re-verified by every collect)
    beats an inline block; a preserved unit is a RECORD of a measurement,
    so it yields to any live claim on the same address. A tie at one
    precedence is a conflict the caller must surface, never decide.
    """
    if annotation.recipe == "writeback":
        return (0,)
    if annotation.recipe == "census":
        return (1,)
    return (2,)


def _code_only(region: str) -> str:
    """The region with comments and blank lines removed.

    Two stacked doc comments above one definition extract DIFFERENT raw
    regions - each starts at its own marker - but they are one piece of
    code, and comparing code rather than text is what says so.
    """
    kept, in_block = [], False
    for line in region.splitlines():
        text = line.strip()
        if in_block:
            if "*/" in text:
                in_block = False
                text = text.split("*/", 1)[1].strip()
            else:
                continue
        # Inline comments are cut before comparison: the annotation layer
        # lives in comments, and two regions that differ only in their
        # annotations are one piece of code. A `//` inside a string is cut
        # on BOTH sides of that comparison, so the cut cannot manufacture
        # a difference.
        if "//" in text:
            text = text.split("//", 1)[0].strip()
        if text.startswith("/*"):
            if "*/" not in text:
                in_block = True
            continue
        if not text:
            continue
        kept.append(text)
    return "\n".join(kept)


def resolve(annotations: list[DecompilationState]) -> tuple[
        list[DecompilationState], dict[int, list[DecompilationState]]]:
    """One annotation per address; only GENUINE conflicts survive.

    Two spellings whose regions hold the SAME code are one piece commented
    twice - a shape the tree actually contains (two stacked doc comments
    above one definition) - and collapse to the first silently. Different
    code for one address is resolved by precedence; a tie at the same
    precedence is a conflict the caller must surface, never decide.
    """
    by_address: dict = {}
    for annotation in annotations:
        by_address.setdefault(annotation.address, []).append(annotation)
    resolved, conflicts = [], {}
    for address, group in sorted(by_address.items()):
        if len(group) == 1:
            resolved.append(group[0])
            continue
        if len({_code_only(a.region) for a in group}) == 1:
            resolved.append(group[0])
            continue
        group.sort(key=_precedence)
        best = group[0]
        tied = [a for a in group if _precedence(a) == _precedence(best)]
        if len(tied) > 1:
            conflicts[address] = group
        else:
            resolved.append(best)
    return resolved, conflicts


# ------------------------------------------------------------- cross-reference


def cross_reference(annotations: list[DecompilationState],
                    catalog: dict[int, dict]) -> dict:
    """Hold the map against the catalogue; report drift, never fail on it.

    Returns `{matched, duplicates, catalog_only, uncatalogued}`: matched is
    `{address: record}`, duplicates `{address: [records]}`, catalog_only the
    addresses no annotation claims, uncatalogued the records no catalogue row
    knows.

    `catalog` is `{int_address: row}` as loaded by
    `emit_translation_unit.load_functions()` (catalogue corrections applied).
    """
    result = {"matched": {}, "duplicates": {},
              "catalog_only": [], "uncatalogued": []}
    seen: dict = {}
    for annotation in annotations:
        if annotation.address in seen:
            result["duplicates"].setdefault(annotation.address,
                                            [seen[annotation.address]])
            result["duplicates"][annotation.address].append(annotation)
            continue
        seen[annotation.address] = annotation
        if annotation.address in catalog:
            result["matched"][annotation.address] = annotation
        else:
            result["uncatalogued"].append(annotation)
    for address in seen:
        if address in result["duplicates"]:
            result["matched"].pop(address, None)
    result["catalog_only"] = sorted(set(catalog) - set(seen))
    return result
