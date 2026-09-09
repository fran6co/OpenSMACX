"""Compile each upstream TU once and verify its source-owned image obligations."""

from collections import Counter
from pathlib import Path
from typing import Callable

from .bound_objects import FunctionBinding, check_objects
from .components import ToolchainManifest, VendorManifest, local_path
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


def verify_toolchain(manifest: ToolchainManifest, image_data: bytes, archive_data: bytes) -> dict:
    """Verify explicitly bounded functions in a pinned precompiled library.

    Archive directory indexes are not a final-link witness. Only the selected
    member bytes are read; no object is extracted, compiled, or executed.
    Missing dependency bindings remain unknown, never operand-derived aliases.
    """
    import hashlib
    from .coff_archive import CoffArchive
    pins = {binding.origin.image_sha256 for binding in manifest.bindings}
    spaces = {binding.origin.address_space for binding in manifest.bindings}
    if len(pins) != 1 or spaces != {"pe:va"}:
        raise ValueError("i386 toolchain verification requires one PE virtual-address image")
    image = PeImage(image_data, pins.pop())
    actual = hashlib.sha256(archive_data).hexdigest()
    archive_result = Result.PASS if actual == manifest.archive_sha256 else Result.FAIL
    objects, errors, rows = {}, {}, []
    members = sorted({binding.unit for binding in manifest.bindings})
    if archive_result == Result.PASS:
        archive = CoffArchive(archive_data)
        for member in members:
            try:
                objects[member] = Coff(archive.member(member))
            except ValueError as error:
                errors[member] = str(error)
        rows = check_objects(tuple(FunctionBinding(item.origin, item.spans, item.symbol, item.unit)
                                   for item in manifest.bindings), objects, image, errors=errors)
    outcomes = {row["result"] for row in rows} | {archive_result}
    overall = (Result.FAIL if Result.FAIL in outcomes else Result.UNKNOWN
               if Result.UNKNOWN in outcomes or not rows else Result.PASS)
    return {"component_id": manifest.component.component_id, "check": "toolchain_binary_references",
            "result": overall, "archive": {"result": archive_result, "sha256": actual},
            "functions": rows, "counts": dict(sorted(Counter(row["result"] for row in rows).items())),
            "selected_members": members, "member_errors": errors,
            "member_reads": len(members) if archive_result == Result.PASS else 0, "compile_attempts": 0,
            "unresolved_symbols": sorted({relocation["symbol"] for row in rows
                                           for relocation in row.get("relocations", [])
                                           if relocation["result"] == Result.UNKNOWN}),
            "application_recoveries": 0, "structure_verification": Result.UNKNOWN,
            "runtime_verification": Result.UNKNOWN, "link_verification": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False,
            "limitations": ["selected original bindings only, not whole-library identification or coverage",
                            "original extents and symbol interpretations require independent source evidence",
                            "archive indexes, linker member selection, ABI and runtime behavior remain unproved",
                            "unbound external functions, static data and imports remain unresolved",
                            "this report does not publish targets into application acceptance"]}
