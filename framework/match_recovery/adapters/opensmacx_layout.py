"""Fresh compiler checks of source-owned original member-offset expectations."""

from dataclasses import asdict
from pathlib import Path

from ..fingerprint import input_snapshot
from ..layout_source import read_members
from ..model import Result, digest
from ..pe_coff import Coff
from ..tasks import TaskStore
from .opensmacx import image_pin
from .opensmacx_binary import _inputs
from .vc6 import FLAG_SETS, compile_object
from .vc6_layout import class_layout


def verify_layout(root: Path, task_name: str, *, compiler=compile_object, command_for=None) -> dict:
    root = root.resolve()
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    if task.target != "opensmacx":
        raise ValueError("layout adapter requires an OpenSMACX task")
    _, files = store._scope(task, decisions)
    before = input_snapshot(root, _inputs(root), {"flags": FLAG_SETS, "debug": "/Z7"})
    pin = image_pin(root)
    if any(subject.image_sha256 != pin or subject.address_space != "pe:va" for subject in task.subjects):
        raise ValueError("task refers to another original image")
    expected = [item for item in read_members(root, pin) if item.path in files]
    units = sorted(name for name in files if Path(name).suffix == ".cpp"
                   and not any(part in ("recovered", "unrecovered") for part in Path(name).parts))
    if command_for is None:
        from decomp.asm import build_command
        command_for = lambda path: tuple(build_command(root / "build/compile_commands.json", path))
    rows, attempts = [], 0
    for unit in units if expected else []:
        for flags in FLAG_SETS:
            layouts, errors = {}, {}
            try:
                command = tuple(command_for(root / unit))
                if any(word.lower().startswith(("/yu", "/yc", "/fp", "-yu", "-yc", "-fp", "@")) for word in command):
                    raise ValueError("layout checks require standalone compilation without PCH or response files")
                attempts += 1
                obj = Coff(compiler(root / unit, command, flags + ("/Z7",)))
                for name in sorted({item.class_name for item in expected}):
                    try:
                        layouts[name] = class_layout(obj, name)
                    except ValueError as error:
                        errors[name] = str(error)
            except (OSError, ValueError) as error:
                errors = {item.class_name: str(error) for item in expected}
            for item in expected:
                layout = layouts.get(item.class_name)
                actual = layout["members"].get(item.member) if layout else None
                result = Result.UNKNOWN if actual is None else Result.PASS if actual == item.offset else Result.FAIL
                rows.append({"expectation": asdict(item), "unit": unit, "flags": list(flags),
                             "actual_offset": actual, "result": result,
                             "reason": errors.get(item.class_name, "member absent" if actual is None else "direct member offset compared")})
    if (before != input_snapshot(root, _inputs(root), {"flags": FLAG_SETS, "debug": "/Z7"})
            or task != store.read(task_name) or decisions != store.decisions(task_name)):
        raise ValueError("source, build inputs or task changed during layout verification")
    # Every requested TU/configuration must agree; never select a passing layout
    # from one TU to hide an ABI disagreement in another.
    result = (Result.UNKNOWN if not rows or any(row["result"] == Result.UNKNOWN for row in rows)
              else Result.FAIL if any(row["result"] == Result.FAIL for row in rows) else Result.PASS)
    return {"task_id": task_name, "check": "declared_direct_member_offsets", "result": result,
            "members": rows, "compile_attempts": attempts, "input_fingerprint": digest(before),
            "scope_complete": False, "structure_verification": Result.UNKNOWN, "persisted": False,
            "limitations": ["source expectations are original-layout interpretations, not freshly re-derived image evidence",
                            "direct members only; base hierarchy, member types, full extent and lifetime remain unproved",
                            "all scoped product TUs are checked conservatively; missing debug definitions are unknown"]}
