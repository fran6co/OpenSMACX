"""Fresh compiler witnesses for copies of source-bound header functions.

This attributes function COMDATs, not callers, exports, type records or data.
The canonical binding stays singular. Reports never supply reusable authority.
"""

from pathlib import PurePosixPath

from .coff_replica import _object_evidence, _section_keys, function_signature
from .inline_body import damage_body
from .model import Result, digest


def definition_signature(obj, name):
    """Full owned contents/reference identities, unlike the damage-only hash.

    Keep COMDAT policy, associative children, public aliases and exact local
    relocation targets. Unrelated symbol declarations are not function contents.
    Unsupported/ambiguous COFF identities refuse rather than compare loosely.
    """
    symbol = obj.function(name)
    if symbol.storage != 2:
        raise ValueError("inline ownership requires an external function definition")
    owned = {symbol.section}
    while True:
        expanded = owned | {child for child, parent in obj.associations.items() if parent in owned}
        if expanded == owned:
            break
        owned = expanded
    for number in owned:
        for _, target, _ in obj.sections[number - 1].relocations:
            reference = obj.symbols[target]
            if reference.section not in owned and (
                    reference.storage != 2 or reference.section < 0 or "@?" in reference.name):
                # MSVC encodes unnamed namespaces/local scopes even on symbols
                # labelled EXTERNAL by COFF. An equal spelling is not enough
                # to identify those dependencies across independently compiled
                # TUs. Absolute/debug symbols likewise lack a storage binding.
                raise ValueError("inline definition has an unbound local, weak or absolute dependency outside its owned sections")
    keys = _section_keys(obj, {}, {})
    wanted = {keys[number] for number in owned}
    evidence = _object_evidence(obj)
    sections = [row for row in evidence["sections"] if row[0] in wanted]
    referenced = {target for row in sections for _, target, _ in row[5]}
    symbols = [row for row in evidence["symbols"] if row[0][2] in wanted or row[0] in referenced]
    associations = [row for row in evidence["associations"] if row[0] in wanted]
    return digest({"sections": sections, "symbols": symbols, "associations": associations})


def observe_copies(obj, unit, bindings, selected, editable, files, settings, verified, damaged):
    """Return only compiler-witnessed extra scope, plus explicit refusals.

    `verified(unit)` supplies a fresh object with a passing identical-source
    replica control in this version/flag set. `damaged(unit, header, bytes)`
    compiles the same complete source version with just that header replaced.
    The caller owns freshness, caching within the invocation and attempt counts.
    """
    allowed, reports = {}, []
    defined = {symbol.name for symbol in obj.symbols.values() if symbol.section > 0}
    symbol_owners = {}
    for entity, rows in bindings.items():
        for row in rows:
            symbol_owners.setdefault(row["symbol"], set()).add(entity)
    for entity in sorted(selected):
        rows = bindings.get(entity, [])
        for row in rows:
            name, header = row["symbol"], row["body"]
            if row["unit"] == unit or not header or name not in defined:
                continue
            report = {"entity_id": entity, "symbol": name, "canonical_unit": row["unit"],
                      "body_path": header, "result": Result.UNKNOWN}
            reports.append(report)
            try:
                if (len(rows) != 1 or symbol_owners[name] != {entity}
                        or header not in editable or header not in files
                        or PurePosixPath(header).suffix in settings.unit_extensions
                        or PurePosixPath(header).suffix not in settings.source.extensions
                        or settings.source.is_seed(header)):
                    raise ValueError("inline ownership requires one scoped binding and an authorized source header")
                replacement = damage_body(files[header], name, settings.source.symbol_format)
                canonical = verified(row["unit"])
                signature = definition_signature(canonical, name)
                if definition_signature(obj, name) != signature:
                    raise ValueError("emitted copy differs from its fresh canonical definition")
                canonical_damage = damaged(row["unit"], header, replacement)
                copy_damage = damaged(unit, header, replacement)
                if (function_signature(canonical_damage, name) == function_signature(canonical, name)
                        or function_signature(copy_damage, name) == function_signature(obj, name)):
                    raise ValueError("header body damage did not change both emitted definitions")
                # Damage must still produce the same independently owned function
                # in both contexts; an arbitrary compile error is never a witness.
                if definition_signature(canonical_damage, name) != definition_signature(copy_damage, name):
                    raise ValueError("damaged emitted copies disagree")
                allowed[name] = entity
                report.update(result=Result.PASS, definition_fingerprint=signature,
                              witness="canonical_and_copy_body_damage")
            except (OSError, ValueError) as error:
                report["reason"] = str(error)
    return allowed, reports
