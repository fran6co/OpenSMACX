"""Compare compiled residuals outside explicitly mapped function ownership.

This proves neither the original executable nor C++ structure. In particular,
an unchanged caller can still call a wrongly reconstructed scoped function.
Use with fresh baseline/current compiles and the independent restoration gates.
"""

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
    for key in sorted(old_sections.keys() | new_sections.keys()):
        if old_sections.get(key) != new_sections.get(key):
            row = new_sections.get(key, old_sections.get(key))
            changed_sections.append({"identity": key, "name": row[1],
                                     "change": "added" if key not in old_sections else
                                               "removed" if key not in new_sections else "changed"})
    return {"result": Result.PASS if old == new else Result.FAIL,
            "baseline_fingerprint": digest(old), "current_fingerprint": digest(new),
            "baseline_scoped_functions": old_scope, "current_scoped_functions": new_scope,
            "changed_sections": changed_sections,
            "symbols_changed": old["symbols"] != new["symbols"],
            "associations_changed": old["associations"] != new["associations"],
            "residual_sections": {"baseline": len(old_sections), "current": len(new_sections)}}
