"""Compare compiled residuals outside explicitly mapped function ownership.

This proves neither the original executable nor C++ structure. In particular,
an unchanged caller can still call a wrongly reconstructed scoped function.
Use with fresh baseline/current compiles and the independent restoration gates.
"""

from collections import Counter

from .coff_replica import _object_evidence
from .model import Result, digest
from .tasks import entity_id


def _residual(obj, allowed, outside):
    excluded, keys, entries, found = set(), {}, {}, {}
    if len(set(allowed.values())) != len(allowed):
        raise ValueError("ambiguous scoped function identity")
    for name, entity in sorted(allowed.items()):
        entity_id(entity)
        # Missing emitted code can reflect inlining or removal. Do not invent a
        # section; only actually defined, exclusively owned COMDATs are removed.
        defined = [s for s in obj.symbols.values() if s.name == name and s.section > 0]
        if not defined:
            continue
        symbol = obj.function(name)
        owned = {symbol.section}
        while True:
            expanded = owned | {child for child, parent in obj.associations.items() if parent in owned}
            if expanded == owned:
                break
            owned = expanded
        if owned & excluded:
            raise ValueError("overlapping scoped compiler ownership")
        if any(s.name in outside and s.section in owned for s in obj.symbols.values()):
            raise ValueError("scoped COMDAT also contains an outside source binding")
        excluded.update(owned)
        keys[symbol.section] = "scoped-function:" + entity
        entries[symbol.index] = "scoped-entry:" + entity
        # Associative children need stable compiler ownership, not section
        # numbers or a guessed name-to-byte pairing. Duplicate child identities
        # are refused by the shared normalizer.
        pending = owned - {symbol.section}
        while pending:
            for child in sorted(pending):
                parent = obj.associations[child]
                if parent in keys:
                    keys[child] = keys[parent] + "/associated:" + obj.sections[child - 1].name
            pending -= keys.keys()
        found[entity] = {"symbol": name, "owned_sections": len(owned)}
    value = _object_evidence(obj, excluded=frozenset(excluded), section_keys=keys, entry_names=entries)
    return value, found


def compare_outside_functions(baseline, current, baseline_scope, current_scope, *,
                              baseline_outside=frozenset(), current_outside=frozenset()):
    """Compare all non-scoped COFF contents and exact symbolic references.

    Scope maps emitted names to original entity identities supplied by source,
    never by similarity or by the compiler's output. Function renames normalize
    only the scoped entry identity. Local label offsets, weak/common symbols,
    data, directives, types and COMDAT policies remain strict comparisons.
    Reports contain digests and names, not proprietary object bytes.
    """
    old, old_scope = _residual(baseline, baseline_scope, baseline_outside)
    new, new_scope = _residual(current, current_scope, current_outside)
    changed_sections = []
    old_sections = {row[0]: row for row in old["sections"]}
    new_sections = {row[0]: row for row in new["sections"]}

    def symbols(value, key, section_name):
        return [symbol_details(row[0]) for row in value["symbols"] if row[0][2] == key
                and not (row[0][0] == section_name and row[0][3:] == (0, 3))]

    def references(row):
        return row[5] if row is not None else []

    for key in sorted(old_sections.keys() | new_sections.keys()):
        if old_sections.get(key) != new_sections.get(key):
            before, after = old_sections.get(key), new_sections.get(key)
            row = after or before
            changed_sections.append({"identity": key, "name": row[1],
                                     "change": "added" if key not in old_sections else
                                               "removed" if key not in new_sections else "changed",
                                     "baseline_symbols": symbols(old, key, row[1]),
                                     "current_symbols": symbols(new, key, row[1]),
                                     "changed_dimensions": [label for index, label in ((2, "flags"), (3, "extent"),
                                                                                       (4, "bytes"), (5, "references"))
                                                            if before is not None and after is not None and before[index] != after[index]],
                                     "references_removed": reference_difference(references(before), references(after)),
                                     "references_added": reference_difference(references(after), references(before))})
    return {"result": Result.PASS if old == new else Result.FAIL,
            "baseline_fingerprint": digest(old), "current_fingerprint": digest(new),
            "baseline_scoped_functions": old_scope, "current_scoped_functions": new_scope,
            "changed_sections": changed_sections,
            "symbol_changes": symbol_difference(old["symbols"], new["symbols"]),
            "symbols_changed": old["symbols"] != new["symbols"],
            "associations_changed": old["associations"] != new["associations"],
            "residual_sections": {"baseline": len(old_sections), "current": len(new_sections)}}


def symbol_details(key):
    """Describe a normalized symbol identity without auxiliary/object bytes."""
    name, value, section, kind, storage = key
    return {"symbol": name, "offset": value, "section_identity": section,
            "type": kind, "storage": storage}


def reference_difference(left, right):
    """Exact multiset difference; no guessed pairing of renamed local targets."""
    return [{"offset": offset, "kind": kind, "target": symbol_details(target), "count": count}
            for (offset, target, kind), count in sorted((Counter(left) - Counter(right)).items())]


def symbol_difference(baseline, current):
    # Key by the complete normalized record. A renamed symbol or an auxiliary
    # metadata change remains removed/added evidence, never an inferred alias.
    rows = {digest(row): row for row in (*baseline, *current)}
    old, new = Counter(map(digest, baseline)), Counter(map(digest, current))
    return [{"change": change, **symbol_details(rows[key][0]),
             "auxiliary_fingerprint": digest(rows[key][1]), "count": count}
            for change, delta in (("removed", old - new), ("added", new - old))
            for key, count in sorted(delta.items())]
