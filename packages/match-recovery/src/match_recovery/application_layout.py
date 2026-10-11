"""Fresh compiler checks of source-owned member/base layout interpretations."""

from dataclasses import asdict
from pathlib import Path

from match_recovery.layout_source import read_bases, read_members
from match_recovery.model import Result, digest
from match_recovery.pe_coff import Coff
from match_recovery.tasks import TaskStore
from match_recovery.source_inventory import image_pin, source_snapshot
from match_recovery.ownership_source import class_hint
from match_recovery.codeview import ClassLayouts, ClassNotEmitted
from .layout_witness import declaration_absent
from .compiler import Compiler
from .measurement import Measurement


def check_member(item, layout):
    """Compare a source interpretation with one complete compiler definition."""
    actual = layout["members"].get(item.member) if layout else None
    offset_result = Result.UNKNOWN if actual is None else Result.PASS if actual == item.offset else Result.FAIL
    observed = layout["member_types"].get(item.member, {"kind": "unknown"}) if layout else {"kind": "unknown"}
    kind = observed["kind"]
    actual_class = observed.get("class") if kind == "class_value" else None
    target = observed.get("target", {}) if kind == "pointer" else {}
    actual_pointer_class = target.get("class") if target.get("kind") == "class_value" else None
    element = observed.get("target", {}) if kind == "array" else {}
    actual_array_class = element.get("class") if element.get("kind") == "class_value" else None
    type_result = None
    count_result = size_result = None
    if item.class_type is not None or item.class_pointer_type is not None:
        correct = (actual_class == item.class_type if item.class_type is not None else
                   actual_pointer_class == item.class_pointer_type)
        type_result = Result.UNKNOWN if kind == "unknown" else Result.PASS if correct else Result.FAIL
    if item.class_array_type is not None:
        type_result = (Result.UNKNOWN if kind == "unknown" else
                       Result.PASS if actual_array_class == item.class_array_type else Result.FAIL)
        def compare_extent(key, expected):
            actual = observed.get(key) if kind == "array" else None
            return Result.UNKNOWN if actual is None else Result.PASS if actual == expected else Result.FAIL
        count_result = compare_extent("count", item.array_count)
        size_result = compare_extent("size", item.array_size)
    outcomes = (offset_result, type_result, count_result, size_result)
    result = (Result.FAIL if Result.FAIL in outcomes else Result.UNKNOWN if Result.UNKNOWN in outcomes else Result.PASS)
    return {"actual_class_type": actual_class, "actual_class_pointer_type": actual_pointer_class,
            "actual_class_array_type": actual_array_class,
            "array_count_result": count_result, "array_size_result": size_result,
            "member_type_kind": kind, "observed_type": observed, "actual_offset": actual,
            "offset_result": offset_result, "type_result": type_result, "result": result,
            "reason": ("member absent or compiler definition unavailable" if actual is None else
                       "member offset, class-array element, count and byte extent compared" if item.class_array_type else
                       "member offset and unqualified class-pointer target compared" if item.class_pointer_type else
                       "member offset and class-value type compared" if item.class_type else "direct member offset compared")}


def check_base(item, layout):
    """Compare one named nonvirtual base without guessing virtual placement."""
    matches = [base for base in layout["bases"] if base["class"] == item.base] if layout else []
    result = (Result.UNKNOWN if layout is None or len(matches) > 1 else
              Result.PASS if len(matches) == 1 and not matches[0]["virtual"] and matches[0]["offset"] == item.offset
              else Result.FAIL)
    return {"observed_bases": matches, "result": result,
            "reason": ("compiler definition unavailable" if layout is None else
                       "ambiguous named base" if len(matches) > 1 else
                       "named base absent from complete class definition" if not matches else
                       "declared nonvirtual base and offset compared")}


def verify_layout(root: Path, task_name: str, *, compiler=None, command_for=None, witness_missing=False) -> dict:
    root = root.resolve()
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    settings = Measurement.read(root)
    selected, files = store._scope(task, decisions)
    metadata = {"witness_missing": witness_missing}
    before = settings.snapshot(root, metadata)
    pin = image_pin(root)
    if any(subject.image_sha256 != pin or subject.address_space != "pe:va" for subject in task.subjects):
        raise ValueError("task refers to another original image")
    expected = [item for item in read_members(root, pin) if item.path in files]
    expected_bases = [item for item in read_bases(root, pin) if item.path in files]
    # Task authorization includes test/compiler support, not only product code.
    # Select by configured source ownership, never by build-database presence:
    # a missing recipe for a scoped product TU must remain unmeasurable.
    units = sorted(name for name in files if Path(name).suffix in settings.unit_extensions
                   and any(Path(name).is_relative_to(directory) for directory in settings.source.roots)
                   and not settings.source.is_seed(name))
    # Method mappings are search hints, not proof that an original class existed
    # or a new requirement to match this candidate's current shape.
    hints = {unit: {} for unit in units}
    for item in source_snapshot(root)["records"]:
        if item["entity_id"] not in selected or item["candidate_kind"] != "source" or item["path"] not in hints:
            continue
        catalogued = class_hint(item["catalogued_symbol"]) if settings.source.symbol_format == "msvc" else None
        emitted = class_hint(item["emitted_symbol_hint"]) if settings.source.symbol_format == "msvc" else None
        # The current source may have replaced a scaffold or corrected an old
        # catalogue owner. Inspect its actual class, retaining the alternative
        # as provenance rather than searching for a fictional thunk class.
        name = emitted or catalogued
        if name:
            hints[item["path"]].setdefault(name, {})[item["entity_id"]] = {
                "basis": "source_symbol" if emitted else "catalogued_symbol",
                "catalogued_owner_hint": catalogued}
    required_classes = {item.class_name for item in (*expected, *expected_bases)}
    command_for = command_for if command_for is not None else lambda path: ()
    rows, observations, attempts = [], [], 0
    base_rows = []
    for unit in units:
        classes = sorted(required_classes | hints[unit].keys())
        if not classes:
            continue
        for flags in settings.flags_for(unit):
            layouts, errors, missing, witnesses, absent = {}, {}, set(), {}, set()
            try:
                command = tuple(command_for(root / unit))
                if any(word.lower().startswith(("/yu", "/yc", "/fp", "-yu", "-yc", "-fp", "@"))
                       for word in (*command, *flags, *settings.layout_flags)):
                    raise ValueError("layout checks require standalone compilation without PCH or response files")
                attempts += 1
                if compiler is None:
                    compiler = Compiler(root, profile=settings.compiler_profile)
                obj = Coff(compiler(root / unit, command, flags + settings.layout_flags))
                types = ClassLayouts(obj)
                for name in classes:
                    try:
                        layouts[name] = types.layout(name)
                    except ClassNotEmitted as error:
                        errors[name] = str(error)
                        missing.add(name)
                    except ValueError as error:
                        errors[name] = str(error)
                if witness_missing and missing & required_classes:
                    from .layout_witness import missing_layout_witnesses
                    probed = missing_layout_witnesses(root, unit, sorted(missing & required_classes),
                                                     obj, compiler, command, flags + settings.layout_flags)
                    attempts += probed["compile_attempts"]
                    for witness in probed["witnesses"]:
                        name = witness["class_name"]
                        witnesses[name] = witness
                        errors[name] = witness["reason"]
                        if declaration_absent(witness):
                            absent.add(name)
                        elif witness["result"] == Result.PASS:
                            layouts[name] = witness["compiler_layout"]
            except (OSError, ValueError) as error:
                errors = {name: str(error) for name in classes}
            for name in classes:
                state = ("no_complete_definition" if name in absent else
                         "witnessed" if name in missing and name in layouts else
                         "not_emitted" if name in missing else "observed" if name in layouts else "unmeasurable")
                observations.append({"class_name": name, "unit": unit, "flags": list(flags),
                                     "basis": "source_expectation" if name in required_classes else "mapped_method_hint",
                                     "hint_entities": sorted(hints[unit].get(name, [])),
                                     "ownership_hints": [dict(entity_id=entity, **hint)
                                                         for entity, hint in sorted(hints[unit].get(name, {}).items())],
                                     "compiler_layout": layouts.get(name),
                                     **({"witness": witnesses[name]} if name in witnesses else {}),
                                     "observation": state, "reason": errors.get(name, "complete compiler definition parsed"),
                                     "next_action": ("verify the mapped class name and ownership; this hint is not a layout obligation"
                                                     if state == "not_emitted" and name not in required_classes else
                                                     "establish a layout witness or independently prove this TU irrelevant; do not waive coverage"
                                                     if state == "not_emitted" else
                                                     "repair compilation or unsupported debug parsing before checking offsets"
                                                     if state == "unmeasurable" else None)})
            for item in expected:
                if item.class_name in absent:
                    continue  # Coverage evidence, not an invented member-offset observation.
                checked = check_member(item, layouts.get(item.class_name))
                rows.append({"expectation": asdict(item), "interpretation_id": item.interpretation_id,
                             "unit": unit, "flags": list(flags),
                             "layout_basis": "same_tu_probe" if item.class_name in witnesses and item.class_name in layouts else "product_object",
                             **checked, "reason": errors.get(item.class_name, checked["reason"])})
            for item in expected_bases:
                if item.class_name in absent:
                    continue
                checked = check_base(item, layouts.get(item.class_name))
                base_rows.append({"expectation": asdict(item), "interpretation_id": item.interpretation_id,
                                  "unit": unit, "flags": list(flags),
                                  "layout_basis": "same_tu_probe" if item.class_name in witnesses and item.class_name in layouts else "product_object",
                                  **checked, "reason": errors.get(item.class_name, checked["reason"])})
    if (before != settings.snapshot(root, metadata)
            or task != store.read(task_name) or decisions != store.decisions(task_name)):
        raise ValueError("source, build inputs or task changed during layout verification")
    # Every present definition must agree. Controlled absence closes declaration
    # coverage only; each requirement still needs a real witness in every trial.
    all_rows = rows + base_rows
    def covered(item, unit, flags):
        # A controlled absence can establish that this TU has no declaration
        # to compare. Ordinary non-emission is only an unknown observation.
        return (any(row["interpretation_id"] == item.interpretation_id and row["unit"] == unit
                    and row["flags"] == list(flags) for row in all_rows)
                or any(row["class_name"] == item.class_name and row["unit"] == unit
                       and row["flags"] == list(flags)
                       and row["observation"] == "no_complete_definition"
                       for row in observations))

    missing_witnesses = [{"interpretation_id": item.interpretation_id, "unit": unit, "flags": list(flags)}
                         for item in (*expected, *expected_bases) for unit in units
                         for flags in settings.flags_for(unit)
                         if not covered(item, unit, flags)]
    # Absence controls may cover irrelevant TUs, but cannot establish the
    # requirement itself. Every configured flag profile still needs at least
    # one actual class definition and layout observation somewhere in scope.
    for item in (*expected, *expected_bases):
        actual_flags = {tuple(row["flags"]) for row in observations
                        if row["class_name"] == item.class_name
                        and row["observation"] in ("observed", "witnessed")}
        for flags in sorted({flags for unit in units for flags in settings.flags_for(unit)} - actual_flags):
            if not any(row["interpretation_id"] == item.interpretation_id
                       and row["flags"] == list(flags)
                       for row in missing_witnesses):
                missing_witnesses.append({"interpretation_id": item.interpretation_id,
                                          "unit": None, "flags": list(flags)})
    result = (Result.FAIL if any(row["result"] == Result.FAIL for row in all_rows) else
              Result.UNKNOWN if not all_rows or missing_witnesses or any(row["result"] == Result.UNKNOWN for row in all_rows) else Result.PASS)
    return {"task_id": task_name, "check": "declared_member_and_base_offsets", "result": result,
            "members": rows, "bases": base_rows, "coverage": observations,
            "translation_units": units, "flag_sets": [list(flags) for flags in settings.flag_sets],
            "unit_flag_sets": settings.witness_grid(units),
            "layout_flags": list(settings.layout_flags),
            "counts": {state.value: sum(row["result"] == state for row in all_rows) for state in Result},
            "compile_attempts": attempts, "input_fingerprint": digest(before),
            "witness_missing": witness_missing,
            "witnessed_definitions": sum(row["observation"] == "witnessed" for row in observations),
            "absent_definitions": sum(row["observation"] == "no_complete_definition" for row in observations),
            "missing_witnesses": missing_witnesses,
            "scope_complete": False, "structure_verification": Result.UNKNOWN, "persisted": False,
            "limitations": ["source expectations are original-layout interpretations, not freshly re-derived image evidence",
                            "declared direct members, unqualified class values/pointer targets and nonvirtual bases only; complete hierarchy, types, extent and lifetime remain unproved",
                            "method-derived class hints supply current-code observations, never original-layout expectations",
                            "all scoped product TUs need declaration observations or controlled absence; non-emission alone is unknown",
                            "optional same-TU probes witness visible declarations only, not product preservation or ABI independence"]}
