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
from .layout_hosts import witness_hosts
from .model import Result, digest
from .pe_coff import Coff
from .records import read_json
from .source_inventory import assert_snapshot_current, image_pin, source_snapshot


def check_layout(root: Path, *, units=None, compiler=None, command_for=None, progress=None, witness_missing=False,
                 conventional_offsets=None, witness_unobserved=None) -> dict:
    root = root.resolve()
    settings = Measurement.read(root)
    policy = read_json(root / "recovery/project.json").get("layout", {})
    if (not isinstance(policy, dict) or policy.keys() - {"conventional_offsets", "witness_unobserved"}
            or any(type(value) is not bool for value in policy.values())):
        raise ValueError("layout policy accepts only boolean conventional_offsets and witness_unobserved")
    conventional_offsets = policy.get("conventional_offsets", False) if conventional_offsets is None else conventional_offsets
    witness_unobserved = (policy.get("witness_unobserved", False) and not witness_missing
                          if witness_unobserved is None else witness_unobserved)
    if type(conventional_offsets) is not bool or type(witness_unobserved) is not bool or witness_missing and witness_unobserved:
        raise ValueError("layout witness modes must be boolean and mutually exclusive")
    if units is not None and (not isinstance(units, (list, tuple)) or not units
            or any(not isinstance(unit, str) or not unit for unit in units) or len(set(units)) != len(units)):
        raise ValueError("layout selection requires unique nonempty translation-unit paths")
    metadata = {"units": units, "witness_missing": witness_missing, "conventional_offsets": conventional_offsets,
                "witness_unobserved": witness_unobserved}
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
    hosts = witness_hosts(snapshot, settings.source, selected, (*expectations, *hints)) if witness_unobserved else {}
    retained_hosts = {row["unit"] for row in hosts.values()} - {None}
    baselines, observed_flags, supplements = {}, defaultdict(set), []
    hint_trials = {item.interpretation_id: {"count": 0, "flags": set(), "outcomes": set()} for item in hints}
    rows, observations, errors, attempts = [], [], [], 0
    command_for = command_for if command_for is not None else lambda path: ()

    def unchanged():
        if settings.snapshot(root, metadata) != before or database.read_bytes() != database_bytes:
            raise ValueError("source, compiler policy or compilation database changed during layout coverage")

    def record_layouts(unit, flags, layouts, witnesses):
        for name in layouts:
            observed_flags[name].add(flags)
        for kind, items, compare in (("member", members, check_member), ("base", bases, check_base),
                                     ("conventional_member", hints, check_offset)):
            for item in items:
                if item.class_name not in layouts:
                    continue
                compared = compare(item, layouts[item.class_name])
                if kind == "conventional_member":
                    trial_info = hint_trials[item.interpretation_id]
                    trial_info["count"] += 1
                    trial_info["flags"].add(flags)
                    trial_info["outcomes"].add(compared["result"])
                    if compared["result"] == Result.PASS:
                        continue
                rows.append({"kind": kind, "expectation": asdict(item),
                             "interpretation_id": item.interpretation_id,
                             "unit": unit, "flags": list(flags),
                             "layout_basis": "same_tu_probe" if item.class_name in witnesses else "product_object",
                             **compared})

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
                if unit in retained_hosts:
                    baselines[unit, flags] = obj, command, set(missing)
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
            record_layouts(unit, flags, layouts, witnesses)
            if progress:
                progress({**event, "event": "trial_finish", "elapsed_seconds": monotonic() - started})
    if witness_unobserved:
        from .layout_witness import missing_layout_witnesses
        groups = defaultdict(list)
        for name in classes:
            for flags in settings.flag_sets:
                if flags in observed_flags[name]:
                    continue
                unit = hosts[name]["unit"]
                saved = baselines.get((unit, flags))
                if saved is None or name not in saved[2]:
                    supplements.append({"class_name": name, "unit": unit, "flags": list(flags),
                                        "result": Result.UNKNOWN,
                                        "reason": "no valid non-emitting baseline at the selected literal-include host"})
                else:
                    groups[unit, flags].append(name)
        for (unit, flags), names in sorted(groups.items()):
            unchanged()
            started = monotonic()
            event = {"unit": unit, "flags": list(flags), "classes": names,
                     "trial": trial + 1, "total": total + len(groups), "phase": "unobserved_witness"}
            trial += 1
            if progress:
                progress({**event, "event": "trial_start"})
            obj, command, _ = baselines[unit, flags]
            try:
                probed = missing_layout_witnesses(root, unit, names, obj, compiler, command, flags + settings.layout_flags)
                attempts += probed["compile_attempts"]
                witnessed = probed["witnesses"]
            except (OSError, ValueError) as error:
                witnessed = [{"class_name": name, "result": Result.UNKNOWN, "compiler_layout": None,
                              "reason": str(error)} for name in names]
            unchanged()
            layouts, witnesses = {}, {}
            for witness in witnessed:
                name = witness["class_name"]
                present = witness["result"] == Result.PASS and witness["compiler_layout"] is not None
                if present:
                    layouts[name], witnesses[name] = witness["compiler_layout"], witness
                observation = {"class_name": name, "unit": unit, "flags": list(flags),
                               "observation": "witnessed" if present else "no_complete_definition" if declaration_absent(witness) else "unmeasurable",
                               "witness": witness, "reason": witness["reason"], "phase": "unobserved_witness"}
                observations.append(observation)
                supplements.append({"class_name": name, "unit": unit, "flags": list(flags),
                                    "result": Result.PASS if present else Result.UNKNOWN, "reason": witness["reason"]})
                if not present:
                    errors.append(observation)
            record_layouts(unit, flags, layouts, witnesses)
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
            "unobserved_witnesses": {"enabled": witness_unobserved, "hosts": hosts, "attempts": supplements},
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
