"""Fresh compiler checks of source-owned member/base layout interpretations."""

from dataclasses import asdict
from pathlib import Path

from ..fingerprint import input_snapshot
from ..layout_source import read_bases, read_members
from ..model import Result, digest
from ..pe_coff import Coff
from ..tasks import TaskStore
from .opensmacx import image_pin, legacy_snapshot
from .opensmacx_binary import _inputs
from .opensmacx_project import class_hint
from .vc6 import FLAG_SETS, compile_object
from .vc6_layout import ClassNotEmitted, class_layout


def verify_layout(root: Path, task_name: str, *, compiler=compile_object, command_for=None) -> dict:
    root = root.resolve()
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    if task.target != "opensmacx":
        raise ValueError("layout adapter requires an OpenSMACX task")
    selected, files = store._scope(task, decisions)
    before = input_snapshot(root, _inputs(root), {"flags": FLAG_SETS, "debug": "/Z7"})
    pin = image_pin(root)
    if any(subject.image_sha256 != pin or subject.address_space != "pe:va" for subject in task.subjects):
        raise ValueError("task refers to another original image")
    expected = [item for item in read_members(root, pin) if item.path in files]
    expected_bases = [item for item in read_bases(root, pin) if item.path in files]
    units = sorted(name for name in files if Path(name).suffix == ".cpp"
                   and not any(part in ("recovered", "unrecovered") for part in Path(name).parts))
    # Method mappings are search hints, not proof that an original class existed
    # or a new requirement to match this candidate's current shape.
    hints = {unit: {} for unit in units}
    for item in legacy_snapshot(root)["records"]:
        if item["entity_id"] not in selected or item["candidate_kind"] != "source" or item["path"] not in hints:
            continue
        name = class_hint(item["catalogued_symbol"])
        if name:
            hints[item["path"]].setdefault(name, set()).add(item["entity_id"])
    required_classes = {item.class_name for item in (*expected, *expected_bases)}
    if command_for is None:
        from decomp.asm import build_command
        command_for = lambda path: tuple(build_command(root / "build/compile_commands.json", path))
    rows, observations, attempts = [], [], 0
    base_rows = []
    for unit in units:
        classes = sorted(required_classes | hints[unit].keys())
        if not classes:
            continue
        for flags in FLAG_SETS:
            layouts, errors, missing = {}, {}, set()
            try:
                command = tuple(command_for(root / unit))
                if any(word.lower().startswith(("/yu", "/yc", "/fp", "-yu", "-yc", "-fp", "@")) for word in command):
                    raise ValueError("layout checks require standalone compilation without PCH or response files")
                attempts += 1
                obj = Coff(compiler(root / unit, command, flags + ("/Z7",)))
                for name in classes:
                    try:
                        layouts[name] = class_layout(obj, name)
                    except ClassNotEmitted as error:
                        errors[name] = str(error)
                        missing.add(name)
                    except ValueError as error:
                        errors[name] = str(error)
            except (OSError, ValueError) as error:
                errors = {name: str(error) for name in classes}
            for name in classes:
                state = "not_emitted" if name in missing else "observed" if name in layouts else "unmeasurable"
                observations.append({"class_name": name, "unit": unit, "flags": list(flags),
                                     "basis": "source_expectation" if name in required_classes else "mapped_method_hint",
                                     "hint_entities": sorted(hints[unit].get(name, [])),
                                     "compiler_layout": layouts.get(name),
                                     "observation": state, "reason": errors.get(name, "complete compiler definition parsed"),
                                     "next_action": ("verify the mapped class name and ownership; this hint is not a layout obligation"
                                                     if state == "not_emitted" and name not in required_classes else
                                                     "establish a layout witness or independently prove this TU irrelevant; do not waive coverage"
                                                     if state == "not_emitted" else
                                                     "repair compilation or unsupported debug parsing before checking offsets"
                                                     if state == "unmeasurable" else None)})
            for item in expected:
                layout = layouts.get(item.class_name)
                actual = layout["members"].get(item.member) if layout else None
                result = Result.UNKNOWN if actual is None else Result.PASS if actual == item.offset else Result.FAIL
                rows.append({"expectation": asdict(item), "unit": unit, "flags": list(flags),
                             "actual_offset": actual, "result": result,
                             "reason": errors.get(item.class_name, "member absent" if actual is None else "direct member offset compared")})
            for item in expected_bases:
                layout = layouts.get(item.class_name)
                matches = [base for base in layout["bases"] if base["class"] == item.base] if layout else []
                result = (Result.UNKNOWN if layout is None or len(matches) > 1 else
                          Result.PASS if len(matches) == 1 and not matches[0]["virtual"] and matches[0]["offset"] == item.offset
                          else Result.FAIL)
                base_rows.append({"expectation": asdict(item), "unit": unit, "flags": list(flags),
                                  "observed_bases": matches, "result": result,
                                  "reason": errors.get(item.class_name,
                                                       "ambiguous named base" if len(matches) > 1 else
                                                       "named base absent from complete class definition" if not matches else
                                                       "declared nonvirtual base and offset compared")})
    if (before != input_snapshot(root, _inputs(root), {"flags": FLAG_SETS, "debug": "/Z7"})
            or task != store.read(task_name) or decisions != store.decisions(task_name)):
        raise ValueError("source, build inputs or task changed during layout verification")
    # Every requested TU/configuration must agree; never select a passing layout
    # from one TU to hide an ABI disagreement in another.
    all_rows = rows + base_rows
    result = (Result.UNKNOWN if not all_rows or any(row["result"] == Result.UNKNOWN for row in all_rows)
              else Result.FAIL if any(row["result"] == Result.FAIL for row in all_rows) else Result.PASS)
    return {"task_id": task_name, "check": "declared_member_and_base_offsets", "result": result,
            "members": rows, "bases": base_rows, "coverage": observations,
            "counts": {state.value: sum(row["result"] == state for row in all_rows) for state in Result},
            "compile_attempts": attempts, "input_fingerprint": digest(before),
            "scope_complete": False, "structure_verification": Result.UNKNOWN, "persisted": False,
            "limitations": ["source expectations are original-layout interpretations, not freshly re-derived image evidence",
                            "declared direct members and nonvirtual bases only; complete hierarchy, types, extent and lifetime remain unproved",
                            "method-derived class hints supply current-code observations, never original-layout expectations",
                            "all scoped product TUs are checked conservatively; missing debug definitions are unknown"]}
