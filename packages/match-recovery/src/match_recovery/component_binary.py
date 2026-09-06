"""Compile each upstream TU once and verify its source-owned image obligations."""

from collections import Counter
from pathlib import Path
from typing import Callable

from .bound_objects import FunctionBinding, check_objects
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
    rows = check_objects(tuple(FunctionBinding(item.origin, item.spans, item.symbol, item.unit)
                               for item in manifest.bindings), objects, image, errors=errors)
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
