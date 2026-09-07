"""Native, read-only initial-data checks through configured compiler interaction."""

from collections import Counter
from pathlib import Path

from .application_binary import select_bindings
from .compiler import Compiler
from .generated_binary import check_generated
from .generated_source import read_generated
from .import_source import import_targets, read_imports
from .measurement import Measurement
from .model import Result, digest
from .object_data import check_initial_data, referenced_initial_object_units
from .object_source import object_targets, read_objects
from .ownership_source import project_view
from .pe_coff import Coff, PeImage


def check_data(root: Path, image_path: Path, *, symbols=None, compiler=None):
    root = root.resolve()
    settings = Measurement.read(root)
    if symbols is not None and (not isinstance(symbols, (list, tuple)) or not symbols
            or any(not isinstance(s, str) or not s for s in symbols) or len(symbols) != len(set(symbols))):
        raise ValueError("data selection requires nonempty unique emitted symbols")
    metadata = {"symbols": symbols}
    before = settings.snapshot(root, metadata)
    view, snapshot = project_view(root, allow_empty=True)
    pin = snapshot["image_sha256"]
    image_bytes = image_path.read_bytes()
    image = PeImage(image_bytes, pin)
    bindings = read_objects(root, pin)
    selected = [b for b in bindings if symbols is None or b.symbol in symbols]
    if not selected or symbols is not None and {b.symbol for b in selected} != set(symbols):
        raise ValueError("data selection needs source-owned original objects; missing or empty coverage")
    generated, imports = read_generated(root, pin), read_imports(root)
    _, _, functions = select_bindings(view, frozenset())
    groups = [set(functions), {b.symbol for b in bindings}, {b.symbol for b in generated}, {b.symbol for b in imports}]
    if any(group & other for i, group in enumerate(groups) for other in groups[:i]):
        raise ValueError("conflicting function/object/generated/import bindings")
    flags_list = tuple(flags + settings.layout_flags for flags in settings.flag_sets)
    compiler = compiler if compiler is not None else Compiler(root, profile=settings.compiler_profile)
    rows, dependencies, attempts = [], [], 0
    for flags in flags_list:
        objects, errors = {}, {}
        pending = {b.unit for b in selected}
        while pending:
            for unit in sorted(pending):
                try:
                    if Path(unit).suffix not in settings.unit_extensions:
                        raise ValueError("object owner is not a configured translation unit")
                    attempts += 1
                    objects[unit] = Coff(compiler(root / unit, (), flags))
                except (OSError, ValueError) as error:
                    errors[unit] = str(error)
            pending = referenced_initial_object_units(selected, bindings, objects) - objects.keys() - errors.keys()
        targets, _ = object_targets(bindings, objects)
        imported, _ = import_targets(imports, objects, image)
        tables, _ = check_generated(generated, objects, image, {**functions, **targets})
        measured = check_initial_data(selected, objects, image, {**functions, **targets, **imported, **tables}, errors=errors)
        rows.extend({**row, "flags": list(flags)} for row in measured)
        dependencies.append({"flags": list(flags), "compiled_units": sorted(objects), "errors": errors})
    if settings.snapshot(root, metadata) != before or image_path.read_bytes() != image_bytes:
        raise ValueError("source, compiler inputs or image changed during initial-data measurement")
    counts = Counter(row["content"] for row in rows)
    result = Result.FAIL if counts[Result.FAIL] else Result.UNKNOWN if counts[Result.UNKNOWN] else Result.PASS
    return {"check": "original_initial_data", "schema_version": 1, "result": result,
            "objects": rows, "selected_objects": len(selected), "counts": dict(counts),
            "compile_attempts": attempts, "trials": dependencies,
            "flag_sets": [list(flags) for flags in flags_list], "input_fingerprint": digest(before),
            "persisted": False, "ready_for_collection": False,
            "structure_verification": Result.UNKNOWN, "runtime_verification": Result.UNKNOWN,
            "limitations": ["only source-bound objects; legacy trailing-address comments are not coverage",
                            "initial image storage only, not CRT constructors, final link selection or runtime state",
                            "CodeView 5 standalone data extents; unsupported records and missing bounds remain unknown",
                            "all requested compiler trials checked independently; no borrowing data from another trial"]}
