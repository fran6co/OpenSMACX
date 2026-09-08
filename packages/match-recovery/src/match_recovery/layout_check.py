"""Check source-owned layout expectations across fresh product compilation.

All emitted definitions and configured flag sets must agree. Absence in a valid
type stream is recorded, not a proof that its TU has no ABI dependency. This is
compiler-observation coverage, not full hierarchy/lifetime or migration admission.
"""

from collections import Counter, defaultdict
from dataclasses import asdict
from pathlib import Path
from time import monotonic

from .application_layout import check_base, check_member
from .codeview import ClassLayouts, ClassNotEmitted
from .compiler import Compiler
from .conventional_layout import read_offsets, check_offset
from .layout_source import read_bases, read_members
from .mapping_check import build_units
from .measurement import Measurement
from .layout_witness import declaration_absent
from .model import Result, digest
from .pe_coff import Coff
from .records import read_json
from .source_inventory import assert_snapshot_current, image_pin, source_snapshot


def check_layout(root: Path, *, units=None, compiler=None, command_for=None, progress=None, witness_missing=False,
                 conventional_offsets=False) -> dict:
    root = root.resolve()
    settings = Measurement.read(root)
    if units is not None and (not isinstance(units, (list, tuple)) or not units
            or any(not isinstance(unit, str) or not unit for unit in units) or len(set(units)) != len(units)):
        raise ValueError("layout selection requires unique nonempty translation-unit paths")
    metadata = {"units": units, "witness_missing": witness_missing, "conventional_offsets": conventional_offsets}
    before = settings.snapshot(root, metadata)
    snapshot = source_snapshot(root)
    pin = image_pin(root)
    members, bases = read_members(root, pin), read_bases(root, pin)
    expectations = (*members, *bases)
    hints, hint_errors, dependencies = read_offsets(snapshot, settings.source) if conventional_offsets else ((), [], [])
    if not expectations and not hints:
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
    classes = sorted({item.class_name for item in (*expectations, *hints)})
    hint_trials = {item.interpretation_id: {"count": 0, "flags": set(), "outcomes": set()} for item in hints}
    rows, observations, errors, attempts = [], [], [], 0
    command_for = command_for if command_for is not None else lambda path: ()

    def unchanged():
        if settings.snapshot(root, metadata) != before or database.read_bytes() != database_bytes:
            raise ValueError("source, compiler policy or compilation database changed during layout coverage")

    trial, total = 0, len(selected) * len(settings.flag_sets)
    for unit in selected:
        for flags in settings.flag_sets:
            trial += 1
            started = monotonic()
            event = {"unit": unit, "flags": list(flags), "trial": trial, "total": total}
            if progress:
                progress({**event, "event": "trial_start"})
            unchanged()
            layouts, unavailable, missing, witnesses, absent = {}, {}, set(), {}, set()
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
                if witness_missing and missing:
                    from .layout_witness import missing_layout_witnesses
                    probed = missing_layout_witnesses(root, unit, sorted(missing), obj, compiler,
                                                     command, flags + settings.layout_flags)
                    attempts += probed["compile_attempts"]
                    for witness in probed["witnesses"]:
                        name = witness["class_name"]
                        witnesses[name] = witness
                        unavailable[name] = witness["reason"]
                        if declaration_absent(witness):
                            absent.add(name)
                        elif witness["result"] == Result.PASS:
                            layouts[name] = witness["compiler_layout"]
            except (OSError, ValueError) as error:
                unavailable = {name: str(error) for name in classes}
            unchanged()
            for name in classes:
                observation = ("no_complete_definition" if name in absent else
                               "witnessed" if name in layouts and name in missing else
                               "observed" if name in layouts else
                               "not_emitted" if name in missing else "unmeasurable")
                observations.append({"class_name": name, "unit": unit, "flags": list(flags),
                                     "observation": observation,
                                     **({"witness": witnesses[name]} if name in witnesses else {}),
                                     "reason": unavailable.get(name, "complete compiler definition parsed")})
                if observation == "unmeasurable" or (name in witnesses and witnesses[name]["result"] != Result.PASS):
                    errors.append(observations[-1])
            for kind, items, compare in (("member", members, check_member), ("base", bases, check_base)):
                for item in items:
                    if item.class_name not in layouts:
                        continue
                    rows.append({"kind": kind, "expectation": asdict(item),
                                 "interpretation_id": item.interpretation_id,
                                 "unit": unit, "flags": list(flags),
                                 "layout_basis": "same_tu_probe" if item.class_name in witnesses else "product_object",
                                 **compare(item, layouts[item.class_name])})
            for item in hints:
                if item.class_name not in layouts:
                    continue
                compared = check_offset(item, layouts[item.class_name])
                trial_info = hint_trials[item.interpretation_id]
                trial_info["count"] += 1
                trial_info["flags"].add(flags)
                trial_info["outcomes"].add(compared["result"])
                # Keep every counterexample, but not millions of repeated good
                # header observations. Coverage and counts retain their trials.
                if compared["result"] != Result.PASS:
                    rows.append({"kind": "conventional_member", "expectation": asdict(item),
                                 "interpretation_id": item.interpretation_id,
                                 "unit": unit, "flags": list(flags),
                                 "layout_basis": "same_tu_probe" if item.class_name in witnesses else "product_object",
                                 **compared})
            if progress:
                progress({**event, "event": "trial_finish", "elapsed_seconds": monotonic() - started})
    # Unlike spelling trials, ABI observations are not alternatives. Every
    # complete definition must agree; one good TU cannot hide another's failure.
    summaries = []
    by_interpretation = defaultdict(list)
    for row in rows:
        by_interpretation[row["interpretation_id"]].append(row)
    unavailable_classes = {row["class_name"] for row in errors}
    for item in expectations:
        observed = by_interpretation[item.interpretation_id]
        missing_flags = [list(flags) for flags in settings.flag_sets
                         if not any(row["flags"] == list(flags) for row in observed)]
        unavailable = item.class_name in unavailable_classes
        result = (Result.FAIL if any(row["result"] == Result.FAIL for row in observed) else
                  Result.UNKNOWN if missing_flags or unavailable or any(row["result"] == Result.UNKNOWN for row in observed)
                  else Result.PASS)
        summaries.append({"expectation": asdict(item), "interpretation_id": item.interpretation_id,
                          "result": result, "observed_definitions": len(observed),
                          "missing_flag_sets": missing_flags})
    for item in hints:
        trial_info = hint_trials[item.interpretation_id]
        missing_flags = [list(flags) for flags in settings.flag_sets if flags not in trial_info["flags"]]
        result = (Result.FAIL if Result.FAIL in trial_info["outcomes"] else
                  Result.UNKNOWN if missing_flags or item.class_name in unavailable_classes
                  or Result.UNKNOWN in trial_info["outcomes"] else Result.PASS)
        summaries.append({"kind": "conventional_member", "expectation": asdict(item),
                          "interpretation_id": item.interpretation_id, "result": result,
                          "observed_definitions": trial_info["count"], "missing_flag_sets": missing_flags})
    unchanged()
    assert_snapshot_current(root, snapshot)
    counts = Counter(row["result"] for row in summaries)
    omitted = sorted(product - set(selected))
    unsupported = sorted(built - product)
    result = (Result.FAIL if counts[Result.FAIL] else
              Result.UNKNOWN if counts[Result.UNKNOWN] or hint_errors or omitted or unsupported or external else Result.PASS)
    return {"schema_version": 1, "check": "source_layout_coverage", "result": result,
            "counts": dict(sorted(counts.items())), "expectations": summaries, "observations": rows,
            "coverage": observations, "errors": errors, "translation_units": selected,
            "omitted_units": omitted, "unsupported_build_inputs": unsupported, "external_build_inputs": external,
            "compile_attempts": attempts, "selection": units,
            "conventional_offsets": {"enabled": conventional_offsets, "hints": len(hints), "errors": hint_errors,
                                     "excluded_component_roots": dependencies},
            "witness_missing": witness_missing,
            "witnessed_definitions": sum(row["observation"] == "witnessed" for row in observations),
            "absent_definitions": sum(row["observation"] == "no_complete_definition" for row in observations),
            "input_fingerprint": digest({"inputs": before, "database": database_bytes.hex()}),
            "persisted": False, "ready_for_collection": False, "structure_verification": Result.UNKNOWN,
            "limitations": ["conventional hints are checked only when requested; they are not evidence-backed original-layout interpretations",
                            "conventional parsing is a limited declaration lens, not full C++ or conditional-preprocessing analysis",
                            "passing conventional trials are aggregated; nonpassing observations retain their individual witnesses",
                            "all emitted definitions must agree; non-emission does not prove a TU has no ABI dependency",
                            "each expectation needs a compiler witness in every configured flag set",
                            "original expectations are source evidence, not re-derived from image bytes",
                            "complete types, virtual-base placement, hierarchy, lifetime and final-link selection remain unproved",
                            "selected-unit diagnostics cannot waive omitted or unsupported build inputs",
                            "optional same-TU probes witness visible declarations only, not product preservation or ABI independence"]}
