"""Compile each upstream TU once and verify its source-owned image obligations."""

from collections import Counter
from pathlib import Path
from typing import Callable

from .binary import compare_bound_code
from .components import VendorManifest, local_path
from .fingerprint import input_snapshot
from .model import Result
from .pe_coff import Coff, PeImage


def verify_component(root: Path, manifest: VendorManifest, image_data: bytes,
                     compile_source: Callable[[Path, tuple[str, ...]], bytes]) -> dict:
    pins = {binding.origin.image_sha256 for binding in manifest.bindings}
    spaces = {binding.origin.address_space for binding in manifest.bindings}
    if len(pins) != 1 or spaces != {"pe:va"}:
        raise ValueError("i386 component verification requires one PE virtual-address image")
    image = PeImage(image_data, pins.pop())
    source = local_path(root, manifest.source_root)
    inputs = tuple(manifest.source_root + "/" + item.path for item in manifest.files)
    before = input_snapshot(root, inputs, manifest.to_dict())
    objects, errors = {}, {}
    for unit in manifest.units:
        try:
            objects[unit] = Coff(compile_source(local_path(source, unit), manifest.flags))
        except (OSError, ValueError) as error:
            errors[unit] = str(error)
    # Build only from explicit bindings. Never discover a target by reading the
    # image operand that this very check is supposed to prove.
    sections, externals, subjects = {}, {}, {}
    for binding in manifest.bindings:
        obj = objects.get(binding.unit)
        if obj is None:
            continue
        try:
            symbol = obj.function(binding.symbol)
        except ValueError as error:
            subjects[binding.origin.entity_id] = str(error)
            continue
        key = (binding.unit, symbol.section)
        base = binding.origin.address - symbol.value
        if key in sections and sections[key] != base:
            raise ValueError("conflicting original placement of a COFF section")
        sections[key] = base
        if symbol.storage == 2:
            if symbol.name in externals and externals[symbol.name] != binding.origin.address:
                raise ValueError("conflicting external symbol bindings")
            externals[symbol.name] = binding.origin.address
        subjects[binding.origin.entity_id] = symbol
    rows = []
    for binding in manifest.bindings:
        row = {"entity_id": binding.origin.entity_id, "address": f"0x{binding.origin.address:08X}",
               "symbol": binding.symbol, "unit": binding.unit}
        try:
            if binding.unit in errors:
                raise ValueError(errors[binding.unit])
            subject = subjects[binding.origin.entity_id]
            if isinstance(subject, str):
                raise ValueError(subject)
            if len(binding.spans) != 1:
                raise ValueError("multi-span placement requires an explicit section layout")
            obj = objects[binding.unit]
            targets = {}
            for index, symbol in obj.symbols.items():
                if symbol.section > 0:
                    base = sections.get((binding.unit, symbol.section))
                    if base is not None:
                        targets[str(index)] = base + symbol.value
                elif symbol.section == 0 and symbol.value == 0 and symbol.storage == 2:
                    address = externals.get(symbol.name)
                    if address is not None:
                        targets[str(index)] = address
            code, relocations, alignment = obj.subject(subject)
            span = binding.spans[0]
            proof = compare_bound_code(span, image.read(span), code, relocations, targets,
                                       original_absolute_sites=image.absolute_sites(span), alignment=alignment)
            for relocation in proof.get("relocations", []):
                relocation["symbol"] = obj.symbols[int(relocation["target"])].name
            row.update(proof)
        except ValueError as error:
            row.update(result=Result.UNKNOWN, reason=str(error))
        rows.append(row)
    results = [row["result"] for row in rows]
    if input_snapshot(root, inputs, manifest.to_dict()) != before:
        raise ValueError("component sources changed during measurement; retry at a stable checkpoint")
    overall = (Result.FAIL if Result.FAIL in results else
               Result.UNKNOWN if not results or Result.UNKNOWN in results else Result.PASS)
    return {"component_id": manifest.component.component_id, "check": "binary_references",
            "result": overall, "functions": rows, "compile_attempts": len(manifest.units),
            "counts": dict(sorted(Counter(results).items())),
            "unresolved_symbols": sorted({relocation["symbol"] for row in rows
                                           for relocation in row.get("relocations", [])
                                           if relocation["result"] == Result.UNKNOWN}),
            "unmeasurable": [{"address": row["address"], "symbol": row["symbol"], "reason": row["reason"]}
                              for row in rows if "reason" in row],
            "application_recoveries": 0, "structure_verification": Result.UNKNOWN,
            "runtime_verification": Result.UNKNOWN}
