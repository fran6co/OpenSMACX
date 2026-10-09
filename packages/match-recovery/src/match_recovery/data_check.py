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
from .object_source import (object_targets, read_objects, referenced_object_units,
                            require_disjoint_object_symbols)
from .literal_source import read_literals
from .literal_data import check_literals
from .ownership_source import project_view
from .pe_coff import Coff, PeImage
from .shared_source import read_shared, referenced_shared_units, validate_shared
from .shared_binary import check_shared


def check_data(root: Path, image_path: Path, *, symbols=None, compiler=None):
    root = root.resolve()
    settings = Measurement.read(root)
    if symbols is not None and (not isinstance(symbols, (list, tuple)) or not symbols
            or any(not isinstance(s, str) or not s for s in symbols) or len(symbols) != len(set(symbols))):
        raise ValueError("data selection requires nonempty unique source-bound symbols")
    metadata = {"symbols": symbols}
    before = settings.snapshot(root, metadata)
    view, snapshot = project_view(root, allow_empty=True)
    pin = snapshot["image_sha256"]
    image_bytes = image_path.read_bytes()
    image = PeImage(image_bytes, pin)
    bindings = read_objects(root, pin)
    literals = read_literals(root, pin)
    selected = [b for b in bindings if symbols is None or b.symbol in symbols]
    selected_literals = [b for b in literals if symbols is None or b.symbol in symbols]
    selection = [*selected, *selected_literals]
    if symbols is not None and len({b.symbol for b in selection}) != len(selection):
        raise ValueError("bare-symbol data selection is ambiguous across literal owners; check all data")
    if not selection or symbols is not None and {b.symbol for b in selection} != set(symbols):
        raise ValueError("data selection needs source-owned historical object IDs or literal symbols; missing or empty coverage")
    generated, imports = read_generated(root, pin), read_imports(root)
    shared = read_shared(root, pin)
    validate_shared(shared, view)
    _, _, functions = select_bindings(view, frozenset())
    object_symbols = require_disjoint_object_symbols(
        bindings, set(functions), {b.symbol for b in generated}, {b.symbol for b in imports},
        {b.symbol for b in shared})
    groups = [set(functions), set(object_symbols), {b.symbol for b in generated},
              {b.symbol for b in imports}, {b.symbol for b in shared}]
    if any(group & other for i, group in enumerate(groups) for other in groups[:i]):
        raise ValueError("conflicting function/object/generated/import bindings")
    flags_list = tuple(flags + settings.layout_flags for flags in settings.flag_sets)
    compiler = compiler if compiler is not None else Compiler(root, profile=settings.compiler_profile)
    rows, literal_rows, dependencies, shared_checks, attempts = [], [], [], [], 0
    object_cache = {}
    observed_grid = {}

    def effective(unit, requested):
        suffix = settings.layout_flags if settings.layout_flags and requested[-len(settings.layout_flags):] == settings.layout_flags else ()
        base = requested[:-len(suffix)] if suffix else requested
        return settings.effective_flags(unit, base) + suffix

    for flags in flags_list:
        objects, errors = {}, {}
        effective_by_unit = {}
        pending = {b.unit for b in selection}
        while pending:
            for unit in sorted(pending):
                try:
                    if Path(unit).suffix not in settings.unit_extensions:
                        raise ValueError("object owner is not a configured translation unit")
                    unit_flags = effective(unit, flags)
                    effective_by_unit[unit] = list(unit_flags)
                    observed_grid.setdefault(unit, [])
                    if list(unit_flags) not in observed_grid[unit]:
                        observed_grid[unit].append(list(unit_flags))
                    cache_key = (unit, unit_flags)
                    if cache_key not in object_cache:
                        attempts += 1
                        try:
                            object_cache[cache_key] = (Coff(compiler(root / unit, (), unit_flags)), None)
                        except (OSError, ValueError) as error:
                            object_cache[cache_key] = (None, str(error))
                    obj, error = object_cache[cache_key]
                    if error is not None:
                        errors[unit] = error
                    else:
                        objects[unit] = obj
                except (OSError, ValueError) as error:
                    errors[unit] = str(error)
            pending = (referenced_initial_object_units(selected, bindings, objects)
                       | referenced_shared_units(shared, objects)
                       | referenced_object_units(bindings, {item.unit: objects[item.unit]
                           for item in shared if item.unit in objects})) - objects.keys() - errors.keys()
        targets, _ = object_targets(bindings, objects)
        active_literals = [b for b in literals if b.unit in objects or b.unit in errors]
        local_targets, checked_literals = check_literals(active_literals, objects, image, errors=errors)
        wanted_literals = {b.interpretation_id for b in selected_literals}
        literal_rows.extend({**row, "flags": list(effective(row["unit"], flags)), "graph_flags": list(flags)}
                            for row in checked_literals if row["interpretation_id"] in wanted_literals)
        imported, _ = import_targets(imports, objects, image)
        tables, _ = check_generated(generated, objects, image, {**functions, **targets})
        active_shared = [item for item in shared if item.unit in objects or item.unit in errors]
        shared_targets, checked_shared = check_shared(active_shared, objects, image,
            {**functions, **targets, **tables}, import_targets=imported, literal_targets=local_targets, errors=errors)
        shared_checks.extend({**row, "flags": list(effective(row["unit"], flags)), "graph_flags": list(flags)}
                             for row in checked_shared)
        measured = check_initial_data(selected, objects, image,
            {**functions, **targets, **imported, **tables, **shared_targets},
            literal_targets=local_targets, errors=errors)
        rows.extend({**row, "flags": list(effective(row["unit"], flags)), "graph_flags": list(flags)}
                    for row in measured)
        dependencies.append({"flags": list(flags), "compiled_units": sorted(objects), "errors": errors,
                             "unit_flag_sets": {unit: effective_by_unit[unit] for unit in sorted(effective_by_unit)}})
    if settings.snapshot(root, metadata) != before or image_path.read_bytes() != image_bytes:
        raise ValueError("source, compiler inputs or image changed during initial-data measurement")
    counts = Counter(row["content"] for row in (*rows, *literal_rows))
    results = {row["result"] for row in shared_checks} | set(counts)
    result = Result.FAIL if Result.FAIL in results else Result.UNKNOWN if Result.UNKNOWN in results else Result.PASS
    return {"check": "original_initial_data", "schema_version": 1, "result": result,
            "objects": rows, "selected_objects": len(selected), "counts": dict(counts),
            "literals": literal_rows, "selected_literals": len(selected_literals),
            "shared_functions": shared_checks,
            "compile_attempts": attempts, "trials": dependencies,
            "flag_sets": [list(flags) for flags in flags_list],
            "unit_flag_sets": {unit: trials for unit, trials in sorted(observed_grid.items())},
            "input_fingerprint": digest(before),
            "persisted": False, "ready_for_collection": False,
            "structure_verification": Result.UNKNOWN, "runtime_verification": Result.UNKNOWN,
            "limitations": ["only source-bound objects; legacy trailing-address comments are not coverage",
                            "initial image storage only, not CRT constructors, final link selection or runtime state",
                            "CodeView 5 standalone data extents; unsupported records and missing bounds remain unknown",
                            "all requested compiler trials checked independently; no borrowing data from another trial"]}
