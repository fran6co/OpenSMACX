"""Compare compiled residuals outside explicitly mapped function ownership.

This proves neither the original executable nor C++ structure. In particular,
an unchanged caller can still call a wrongly reconstructed scoped function.
Use with fresh baseline/current compiles and the independent restoration gates.
"""

from collections import Counter

from .coff_replica import _object_evidence
from .model import Result, digest
from .tasks import entity_id


def _residual(obj, allowed, outside, *, retained=frozenset(), startup=None, common=None):
    excluded, seen, keys, entries, found = set(), set(), {}, {}, {}
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
        if owned & seen:
            raise ValueError("overlapping scoped compiler ownership")
        seen.update(owned)
        if any((s.name in outside or s.name in retained and s.name != name) and s.section in owned
               for s in obj.symbols.values()):
            raise ValueError("scoped COMDAT also contains an outside source binding")
        if name in retained:
            continue  # Newly annotating existing code does not waive its baseline preservation.
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
    if startup is None:
        if common is not None:
            raise ValueError("common request coverage requires its authorized startup projection")
        value = _object_evidence(obj, excluded=frozenset(excluded), section_keys=keys, entry_names=entries)
    else:
        from .startup_coverage import consume_startup, consume_startup_fpo
        from .startup_projection import project_startup
        slots = consume_startup(startup, obj)
        fpo = consume_startup_fpo(startup, obj)
        startup_roots = {obj.symbols[index].section for _, _, index in slots} | {child for child, _, _ in fpo}
        if any(symbol.section in startup_roots and (symbol.name in outside or symbol.name in retained)
               for symbol in obj.symbols.values()):
            raise ValueError("generated startup root also contains a protected outside source binding")
        value, projection = project_startup(obj, slots, excluded=frozenset(excluded),
                                            section_keys=keys, entry_names=entries, fpo=fpo)
        if common is not None:
            from .common_request_coverage import _project_requests
            value = _project_requests(common, obj, value, projection, startup=startup)
    return value, found


def compare_outside_functions(baseline, current, baseline_scope, current_scope, *,
                              baseline_outside=frozenset(), current_outside=frozenset(),
                              baseline_startup=None, current_startup=None,
                              baseline_common=None, current_common=None):
    """Compare all non-scoped COFF contents and exact symbolic references.

    Scope maps emitted names to original entity identities supplied by source,
    never by similarity or by the compiler's output. Function renames normalize
    only the scoped entry identity. Supported numeric local labels use their
    verified function/associative owner and exact location, not decimal spellings;
    this never aliases external or unnamed-namespace symbols. Local label offsets, weak/common symbols,
    data, directives, types and COMDAT policies remain strict comparisons.
    Reports contain digests and names, not proprietary object bytes.
    """
    # A seed-to-product binding can annotate code that already compiled in the
    # baseline without a source binding. Do not make that old residual appear
    # removed, or infer historical ownership from its current name. Keep the
    # current same-named definition fully protected too. Actual new definitions
    # remain removable under their explicit current source scope.
    defined_before = {symbol.name for symbol in baseline.symbols.values() if symbol.section > 0}
    retained = {name: entity for name, entity in current_scope.items()
                if name in defined_before and name not in baseline_scope}
    old, old_scope = _residual(baseline, baseline_scope, baseline_outside, startup=baseline_startup,
                               common=baseline_common)
    new, new_scope = _residual(current, current_scope, current_outside, retained=retained.keys(), startup=current_startup,
                               common=current_common)
    return _compare_residuals(old, new, old_scope, new_scope, retained)


def compare_unit_presence(obj, scope, *, present, outside=frozenset(), protected=frozenset(), startup=None, common=None):
    """Report a present TU's residual against explicitly established absence.

    The caller must independently establish source AND fresh recipe membership
    in both versions, compile the present version, and verify its replica. This
    function supplies none of those prerequisites: it never fabricates or
    compiles an empty historical TU. Every retained section/symbol is a real
    difference, including directives, types, empty section metadata and implicit
    linker requirements. No guessed compiler scaffolding is waived.

    For an added unit, protected names are definitions from the complete
    baseline that lacked task ownership. Moving/duplicating such a definition
    into a newly annotated TU must not waive its historical preservation.
    """
    if present not in ("baseline", "current"):
        raise ValueError("unit presence requires baseline or current")
    retained = {name: entity for name, entity in scope.items() if name in protected}
    value, found = _residual(obj, scope, outside, retained=retained.keys(), startup=startup, common=common)
    absent = {"sections": [], "symbols": [], "associations": []}
    old, new = (value, absent) if present == "baseline" else (absent, value)
    old_scope, new_scope = (found, {}) if present == "baseline" else ({}, found)
    report = _compare_residuals(old, new, old_scope, new_scope, retained)
    report["unit_presence"] = {"baseline": present == "baseline", "current": present == "current"}
    return report


def _compare_residuals(old, new, old_scope, new_scope, retained):
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
            "newly_bound_protected_functions": retained,
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
