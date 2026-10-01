"""Actual-product-body controls for the technology/message-pump owner batch.

The harness is only a host projection of Win32 and game-owned objects. The
four tested method bodies are extracted from mapped product source at runtime;
no method body is copied into the fixture.
"""
from __future__ import annotations

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name("core_short_owner_harness.cpp")
METHODS = {
    "TECH_SECRET": (
        0x005BADC0,
        ROOT / "src/technology.cpp",
        r"void __cdecl tech_secret\(int tech_id, int faction_id\)",
    ),
    "DO_TASK": (
        0x005FC7F0,
        ROOT / "src/win.cpp",
        r"int __cdecl do_task\(\)",
    ),
    "WAIT_TASK": (
        0x005FC700,
        ROOT / "src/win.cpp",
        r"void __cdecl wait_task\(\)",
    ),
    "DO_ALL_TASKS": (
        0x005FC8F0,
        ROOT / "src/win.cpp",
        r"void __cdecl do_all_tasks\(\)",
    ),
}


def extract_body(source: str, signature_pattern: str) -> str:
    match = re.search(signature_pattern + r"\s*\{", source)
    assert match is not None, f"product definition not found: {signature_pattern}"
    opening = source.index("{", match.start())
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[match.start():end + 1]
    raise AssertionError(f"unterminated product body: {signature_pattern}")


def actual_bodies() -> dict[str, str]:
    by_path: dict[Path, list] = {}
    for _, path, _ in METHODS.values():
        by_path.setdefault(path, read_file(path))

    bodies: dict[str, str] = {}
    for key, (address, path, pattern) in METHODS.items():
        matches = [record for record in by_path[path] if record.address == address]
        assert len(matches) == 1, (key, address, matches)
        record = matches[0]
        assert record.state.value == "implemented", (key, record.state)
        assert Path(record.path) == path, (key, record.path, path)
        body = extract_body(path.read_text(), pattern)
        assert "BODY GOES HERE" not in body, key
        assert "PENDING_BODY" not in body, key
        assert len(body) > 80, key

        bodies[key] = body
    return bodies


def compile_and_run(tmp_path: Path, bodies: dict[str, str]) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which("clang++") or shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler required for sanitizer-backed source fixture")
    fixture = FIXTURE.read_text()
    marker = "// __ACTUAL_BODIES_SLOT__"
    assert fixture.count(marker) == 1
    injected = "\n\n".join(bodies[key] for key in METHODS)
    source = tmp_path / "core-short-owner.cpp"
    binary = tmp_path / "core-short-owner"
    source.write_text(fixture.replace(marker, injected))
    built = subprocess.run(
        [compiler, "-std=c++17", "-O1", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         "-fno-omit-frame-pointer", str(source), "-o", str(binary)],
        text=True, capture_output=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run(
        [str(binary)], text=True, capture_output=True, timeout=10,
        env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"),
    )


def test_actual_tech_secret_and_task_pump_bodies(tmp_path: Path) -> None:
    result = compile_and_run(tmp_path, actual_bodies())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("case", [
    "do_task_special_dispatch",
    "wait_task_special_dispatch",
    "do_all_tasks_special_dispatch",
    "do_task_network_null",
    "wait_task_network_null",
    "do_task_paint_poll_range",
    "wait_task_paint_poll_range",
    "do_task_initial_order",
    "do_all_tasks_final_cleanup",
    "tech_secret_helper_order",
    "tech_secret_remote_number",
])
def test_actual_body_damage_controls(tmp_path: Path, case: str) -> None:
    bodies = actual_bodies()
    if case == "do_task_special_dispatch":
        key = "DO_TASK"
        old = "message.message == 0x402"
        new = "message.message != 0x402"
    elif case == "wait_task_special_dispatch":
        key = "WAIT_TASK"
        old = "message.message == 0x402"
        new = "message.message != 0x402"
    elif case == "do_all_tasks_special_dispatch":
        key = "DO_ALL_TASKS"
        old = "message.message == 0x402"
        new = "message.message != 0x402"
    elif case in ("do_task_network_null", "wait_task_network_null"):
        key = "DO_TASK" if case.startswith("do_task") else "WAIT_TASK"
        old = "if (ActiveNet != 0) {"
        new = "if (true) {"
    elif case == "do_task_paint_poll_range":
        key = "DO_TASK"
        old = "PeekMessageA(&message, 0, WM_PAINT, WM_PAINT, PM_REMOVE)"
        new = "PeekMessageA(&message, 0, 0x0000, WM_PAINT, PM_REMOVE)"
    elif case == "wait_task_paint_poll_range":
        key = "WAIT_TASK"
        old = "PeekMessageA(&message, 0, WM_PAINT, WM_PAINT, PM_REMOVE)"
        new = "PeekMessageA(&message, 0, 0x0000, WM_PAINT, PM_REMOVE)"
    elif case == "do_task_initial_order":
        key = "DO_TASK"
        old = "do_net();\n    check_net();"
        new = "check_net();\n    do_net();"
    elif case == "do_all_tasks_final_cleanup":
        key = "DO_ALL_TASKS"
        old = "MsgStatus = 0;\n            do_net();\n            return;"
        new = "MsgStatus = 0;\n            return;"
    elif case == "tech_secret_helper_order":
        key = "TECH_SECRET"
        old = "parse_say(0, reinterpret_cast<intptr_t>(Technology[tech_id].name), -1, -1);\n    parse_says(1, Players[faction_id].adj_name_faction, -1, -1);"
        new = "parse_says(1, Players[faction_id].adj_name_faction, -1, -1);\n    parse_say(0, reinterpret_cast<intptr_t>(Technology[tech_id].name), -1, -1);"
    else:
        key = "TECH_SECRET"
        old = "say_num(faction_id != ConsoleGlobal.faction_id_);"
        new = "say_num(faction_id == ConsoleGlobal.faction_id_);"

    changed, count = bodies[key].replace(old, new, 1), bodies[key].count(old)
    assert count >= 1 and changed != bodies[key], (case, key, old)
    bodies[key] = changed
    result = compile_and_run(tmp_path, bodies)
    assert result.returncode != 0, (case, result.stderr)
