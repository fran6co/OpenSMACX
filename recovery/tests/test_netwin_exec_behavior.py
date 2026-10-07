from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


def project_root() -> Path:
    override = os.environ.get("NETWIN_SOURCE_ROOT")
    return Path(override).resolve() if override else Path(__file__).resolve().parents[2]


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char in ('"', "'"):
                state = "string" if char == '"' else "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start : index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        else:
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError(f"unclosed body: {signature}")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_netwin_exec_and_row_actual_bodies(compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    root = project_root()
    source_path = root / "src" / "netwin.cpp"
    fixture_path = Path(__file__).with_name("netwin_exec_host_control.cpp")
    source = source_path.read_text(encoding="utf-8")
    fixture = fixture_path.read_text(encoding="utf-8")

    helper = extract_function(source, "static void netwin_set_rect(")
    exec_body = extract_function(source, "int NetWin::exec()")
    row_body = extract_function(source, "void NetWinLobbyRow::initialize_empty_slot()")
    row_callback = extract_function(
        (root / "src/netwin.h").read_text(encoding="utf-8"),
        "static void __fastcall initialize_empty_slot_callback(void *element)",
    )
    initializer = extract_function(
        (root / "src/array_initialization.cpp").read_text(encoding="utf-8"),
        'extern "C" void __stdcall initialize_strided_array(',
    )
    assert fixture.count("/* ROW_BODY_INSERTION */") == 1
    assert fixture.count("/* EXEC_BODY_INSERTION */") == 1
    generated = fixture.replace("/* ROW_BODY_INSERTION */", row_body)
    assert fixture.count("/* ROW_CALLBACK_INSERTION */") == 1
    generated = generated.replace("/* ROW_CALLBACK_INSERTION */", row_callback)
    generated = generated.replace(
        "/* EXEC_BODY_INSERTION */", initializer + "\n\n" + helper + "\n\n" + exec_body
    )

    with tempfile.TemporaryDirectory(prefix="netwin-exec-test-") as directory:
        generated_path = Path(directory) / "actual_bodies.cpp"
        executable = Path(directory) / "actual_bodies"
        generated_path.write_text(generated, encoding="utf-8")
        host_double_warning = (
            "-Wno-class-memaccess" if compiler_name == "g++"
            else "-Wno-nontrivial-memcall"
        )
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
             host_double_warning,
             str(generated_path), "-o", str(executable)],
            capture_output=True,
            text=True,
            timeout=60,
            check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run(
            [str(executable)], capture_output=True, text=True, timeout=30, check=False
        )
        assert ran.returncode == 0, ran.stdout + ran.stderr
