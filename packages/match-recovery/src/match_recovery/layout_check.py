"""Check source-owned layout expectations across fresh product compilation.

All emitted definitions and configured flag sets must agree. Absence in a valid
type stream is recorded, not a proof that its TU has no ABI dependency. This is
compiler-observation coverage, not full hierarchy/lifetime or migration admission.
"""

from collections import Counter
from dataclasses import asdict
from pathlib import Path

from .application_layout import check_base, check_member
from .codeview import ClassLayouts, ClassNotEmitted
from .compiler import Compiler
from .layout_source import read_bases, read_members
from .mapping_check import build_units
from .measurement import Measurement
from .model import Result, digest
from .pe_coff import Coff
from .records import read_json
from .source_inventory import assert_snapshot_current, image_pin, source_snapshot


def check_layout(root: Path, *, units=None, compiler=None, command_for=None) -> dict:
    root = root.resolve()
    settings = Measurement.read(root)
    if units is not None and (not isinstance(units, (list, tuple)) or not units
            or any(not isinstance(unit, str) or not unit for unit in units) or len(set(units)) != len(units)):
        raise ValueError("layout selection requires unique nonempty translation-unit paths")
    metadata = {"units": units}
    before = settings.snapshot(root, metadata)
    snapshot = source_snapshot(root)
    pin = image_pin(root)
    members, bases = read_members(root, pin), read_bases(root, pin)
    expectations = (*members, *bases)
    if not expectations:
        raise ValueError("no source-owned member or base layout expectations")
    mappings = read_json(root / "recovery/project.json").get("mappings")
    if not isinstance(mappings, dict) or "compile_commands" not in mappings:
        raise ValueError("layout coverage requires a configured compilation database")
    database = settings.source.path(root, mappings["compile_commands"])
    database_bytes = database.read_bytes()
    built, external = build_units(root, database_bytes)
    product = {path for path in built if path in snapshot["files"] and not settings.source.is_seed(path)
               and Path(path).suffix in settings.unit_extensions}
    if not product:
        raise ValueError("layout coverage has no configured product translation units")
    for unit in units or ():
        settings.source.path(root, unit)
        if unit not in product:
            raise ValueError("layout selection is not a configured product translation unit: " + unit)
    selected = sorted(product if units is None else units)
    classes = sorted({item.class_name for item in expectations})
    rows, observations, errors, attempts = [], [], [], 0
    command_for = command_for if command_for is not None else lambda path: ()

    def unchanged():
        if settings.snapshot(root, metadata) != before or database.read_bytes() != database_bytes:
            raise ValueError("source, compiler policy or compilation database changed during layout coverage")

    for unit in selected:
        for flags in settings.flag_sets:
            unchanged()
            layouts, unavailable, missing = {}, {}, set()
            try:
                command = tuple(command_for(root / unit))
                if any(word.lower().startswith(("/yu", "/yc", "/fp", "-yu", "-yc", "-fp", "@"))
                       for word in (*command, *flags, *settings.layout_flags)):
                    raise ValueError("layout checks require standalone compilation without PCH or response files")
                if compiler is None:
                    compiler = Compiler(root, profile=settings.compiler_profile)
                attempts += 1
                data = compiler(root / unit, command, flags + settings.layout_flags)
                if not isinstance(data, bytes) or not data:
                    raise ValueError("compiler returned no object bytes")
                obj = Coff(data)
                types = ClassLayouts(obj)
                for name in classes:
                    try:
                        layouts[name] = types.layout(name)
                    except ClassNotEmitted as error:
                        missing.add(name)
                        unavailable[name] = str(error)
                    except ValueError as error:
                        unavailable[name] = str(error)
            except (OSError, ValueError) as error:
                unavailable = {name: str(error) for name in classes}
            unchanged()
            for name in classes:
                observation = ("observed" if name in layouts else
                               "not_emitted" if name in missing else "unmeasurable")
                observations.append({"class_name": name, "unit": unit, "flags": list(flags),
                                     "observation": observation,
                                     "reason": unavailable.get(name, "complete compiler definition parsed")})
                if observation == "unmeasurable":
                    errors.append(observations[-1])
            for kind, items, compare in (("member", members, check_member), ("base", bases, check_base)):
                for item in items:
                    if item.class_name not in layouts:
                        continue
                    rows.append({"kind": kind, "expectation": asdict(item),
                                 "interpretation_id": item.interpretation_id,
                                 "unit": unit, "flags": list(flags),
                                 **compare(item, layouts[item.class_name])})
    # Unlike spelling trials, ABI observations are not alternatives. Every
    # complete definition must agree; one good TU cannot hide another's failure.
    summaries = []
    for item in expectations:
        observed = [row for row in rows if row["interpretation_id"] == item.interpretation_id]
        missing_flags = [list(flags) for flags in settings.flag_sets
                         if not any(row["flags"] == list(flags) for row in observed)]
        unavailable = any(row["class_name"] == item.class_name for row in errors)
        result = (Result.FAIL if any(row["result"] == Result.FAIL for row in observed) else
                  Result.UNKNOWN if missing_flags or unavailable or any(row["result"] == Result.UNKNOWN for row in observed)
                  else Result.PASS)
        summaries.append({"expectation": asdict(item), "interpretation_id": item.interpretation_id,
                          "result": result, "observed_definitions": len(observed),
                          "missing_flag_sets": missing_flags})
    unchanged()
    assert_snapshot_current(root, snapshot)
    counts = Counter(row["result"] for row in summaries)
    omitted = sorted(product - set(selected))
    unsupported = sorted(built - product)
    result = (Result.FAIL if counts[Result.FAIL] else
              Result.UNKNOWN if counts[Result.UNKNOWN] or omitted or unsupported or external else Result.PASS)
    return {"schema_version": 1, "check": "source_layout_coverage", "result": result,
            "counts": dict(sorted(counts.items())), "expectations": summaries, "observations": rows,
            "coverage": observations, "errors": errors, "translation_units": selected,
            "omitted_units": omitted, "unsupported_build_inputs": unsupported, "external_build_inputs": external,
            "compile_attempts": attempts, "selection": units,
            "input_fingerprint": digest({"inputs": before, "database": database_bytes.hex()}),
            "persisted": False, "ready_for_collection": False, "structure_verification": Result.UNKNOWN,
            "limitations": ["checks explicit source member/base interpretations, not legacy offset comments or self-naming fields",
                            "all emitted definitions must agree; non-emission does not prove a TU has no ABI dependency",
                            "each expectation needs a compiler witness in every configured flag set",
                            "original expectations are source evidence, not re-derived from image bytes",
                            "complete types, virtual-base placement, hierarchy, lifetime and final-link selection remain unproved",
                            "selected-unit diagnostics cannot waive omitted or unsupported build inputs"]}
