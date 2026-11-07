"""Guarded correction of one mapping's current emitted-symbol fact.

The `ORIGINAL` marker's decorated name is the retained catalogue identity and
is never rewritten here. A signature correction (most often a catalogued `H`
return that the image never produces) changes what this tree's compiler emits,
so it is recorded as the separate `// symbol` fact the reader and measurement
already use. Address, spans, historical claims, lessons, bindings and every
other annotation stay byte-for-byte guarded by the native symbol writer.

Evidence binding: the corrected symbol must be defined as an executable
function in a fresh configured compilation of the mapping's emitting TU, and
the previous emitted symbol must no longer be defined there. The decorated
difference is restricted to return type, calling convention and ordinary/static
member storage unless the caller explicitly accepts an identity change. This
never measures original bytes or grants a claim.
"""

import hashlib
from pathlib import Path

from .compiler import Compiler
from .mapping_check import build_units
from .measurement import Measurement
from .model import Origin, Result, require_digest
from .pe_coff import Coff
from .records import read_json
from .source import read_text
from .source.mangled import signature_differences
from .source.model import Mode
from .source.publication import publish
from .source.writer import write_symbol
from .source_inventory import image_pin
from .source_replica import artifact_parent
from .symbol_check import function_definition


def _claims(records) -> list[tuple]:
    return sorted((row.address, row.byte_exact, row.semantic) for row in records)


def _identity(row) -> tuple:
    return (row.address, row.name, row.image_spans, row.mode, row.state,
            row.byte_exact, row.semantic, row.body, row.context)


def correct_symbol(root: Path, *, address: int, source: str, symbol: str, source_sha256: str,
                   reason: str, allow_identity_change: bool = False, compiler=None) -> dict:
    root = root.resolve()
    require_digest(source_sha256)
    if type(address) is not int or not 0 <= address <= 0xFFFFFFFF:
        raise ValueError("symbol correction requires a PE32 original entry address")
    if not isinstance(reason, str) or not reason.strip() or "\n" in reason or "\r" in reason:
        raise ValueError("symbol correction requires a nonempty single-line reason")
    settings = Measurement.read(root)
    config = settings.source
    path = config.path(root, source)
    if config.is_seed(source) or path.suffix in config.interface_extensions or not path.is_file():
        raise ValueError("symbol correction host must be an existing non-seed source translation unit")
    raw = path.read_bytes()
    if hashlib.sha256(raw).hexdigest() != source_sha256:
        raise ValueError("symbol correction source digest is stale")
    text = raw.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n")
    records = read_text(text, path)
    selected = [row for row in records if row.address == address]
    if len(selected) != 1:
        raise ValueError(f"expected one mapping for 0x{address:08X} in {source}, found {len(selected)}")
    record, = selected
    if record.mode is Mode.UNBOUND:
        raise ValueError("an UNBOUND mapping has no emitted symbol to correct")
    current = record.symbol or record.name
    if symbol == current:
        raise ValueError("mapping already emits the requested symbol")
    differences = signature_differences(current, symbol)
    if differences["identity"] and not allow_identity_change:
        raise ValueError("symbol correction changes more than return type, calling convention or static "
                         f"storage ({', '.join(differences['identity'])}); pass --allow-identity-change "
                         "only after reviewing that identity change")
    for row in records:
        if row is not record and symbol in (row.symbol or row.name, row.name):
            raise ValueError(f"symbol is already the identity of the mapping at line {row.line}")

    unit = record.context or source
    project = read_json(root / "recovery/project.json")
    mappings = project.get("mappings")
    if not isinstance(mappings, dict) or "compile_commands" not in mappings:
        raise ValueError("symbol correction requires a configured compilation database")
    database = config.path(root, mappings["compile_commands"])
    units, _ = build_units(root, database.read_bytes())
    if unit not in units or Path(unit).suffix not in settings.unit_extensions:
        raise ValueError(f"emitting unit is not a configured build translation unit: {unit}")
    metadata = {"operation": "correct-symbol", "address": address, "source": source}
    before = settings.snapshot(root, metadata)

    def fresh():
        if settings.snapshot(root, metadata) != before or path.read_bytes() != raw:
            raise ValueError("source, compiler policy or compilation database changed during symbol correction")

    trials, observed = [], None
    for flags in settings.flags_for(unit):
        fresh()
        if compiler is None:
            compiler = Compiler(root, profile=settings.compiler_profile)
        try:
            data = compiler(root / unit, (), flags)
            if not isinstance(data, bytes) or not data:
                raise ValueError("compiler returned no object bytes")
            obj = Coff(data)
        except (OSError, ValueError) as error:
            trials.append({"flags": list(flags), "result": Result.UNKNOWN, "reason": str(error)})
            continue
        fresh()
        new = function_definition(obj, symbol)
        old = function_definition(obj, current)
        trial = {"flags": list(flags), "corrected": new["result"], "previous": old["result"]}
        # The previous symbol must be wholly absent: a remaining definition
        # or reference means the declaration change is incomplete.
        if (new["result"] == Result.PASS and not old.get("undefined_or_weak")
                and old.get("reason") == "no defined function with the mapped name"):
            trials.append({**trial, "result": Result.PASS})
            observed = flags
            break
        trials.append({**trial, "result": Result.FAIL,
                       "alternatives": new.get("alternatives", [])})
    if observed is None:
        raise ValueError("no configured compilation of the emitting unit defines the corrected symbol "
                         f"without the previous one: {trials}")

    changed = write_symbol(raw, record, symbol)
    after = read_text(changed.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n"), path)
    corrected = [row for row in after if row.address == address]
    if (len(after) != len(records) or len(corrected) != 1 or _claims(after) != _claims(records)
            or _identity(corrected[0]) != _identity(record) or (corrected[0].symbol or corrected[0].name) != symbol):
        raise ValueError("symbol correction would change identity, spans or historical claims")
    staging = artifact_parent(root, "build/recovery/publications", excluded_roots=config.roots)
    publish({path: raw}, {path: changed}, validate=fresh, staging_directory=staging)
    origin = Origin(image_pin(root, config), config.address_space, address)
    return {"schema_version": 1, "operation": "correct-symbol",
            "entity_id": origin.entity_id,
            "address": f"0x{address:08X}", "path": source, "line": corrected[0].line,
            "original_name": record.name, "previous_symbol": current, "symbol": symbol,
            "differences": differences, "identity_change_allowed": allow_identity_change,
            "reason": reason, "emitting_unit": unit, "emission_flags": list(observed), "trials": trials,
            "historical_claims": {"byte_exact": record.byte_exact, "instruction_shape": record.semantic},
            "verification": "emission_only", "persisted": True, "ready_for_collection": False,
            "next_action": "measure the mapping; bank an exact result only with `recover record`; "
                           "carry the reason in the commit message",
            "limitations": ["the ORIGINAL catalogue name is retained unchanged as historical identity",
                            "compiler emission is not original-byte, caller or linked-selection proof"]}
