from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from match_recovery.source import Mode, read_file


def project_root() -> Path:
    override = os.environ.get("NETWIN_SOURCE_ROOT")
    return Path(override).resolve() if override else Path(__file__).resolve().parents[2]


def test_compiler_array_mapping_and_passive_wire_storage():
    root = project_root()
    header = (root / "src/netwin.h").read_text()
    wire = header[header.index("struct NetWinLobbyRow {"):]
    wire = wire[:wire.index("};")]
    assert "NetWinLobbyRow(" not in wire and "virtual " not in wire
    assert "initialize_empty_slot" not in header
    source = (root / "src/netwin.cpp").read_text()
    assert "NetWinLobbyRow NetWinLobbyRows[8];" in source
    assert "initialize_strided_array" not in source
    assert not (root / "src/array_initialization.cpp").exists()
    assert not (root / "src/array_initialization.h").exists()
    rows = {row.address: row for row in read_file(root / "src/netwin.cpp")}
    helper = rows[0x00402EE0]
    assert helper.name == "sub_402ee0" and helper.mode is Mode.FILE
    assert helper.symbol == "??_H@YGXPAXIHP6EX0@Z@Z"
    assert helper.image_spans == ((0x00402EE0, 0x00402F08),)
    assert helper.byte_exact
    ctor = rows[0x00483820]
    assert ctor.name == "?UNK5@NetWin@@QAEXXZ"
    assert ctor.symbol == "??0NetWinConstructedLobbyRow@@QAE@XZ"
    assert ctor.image_spans == ((0x00483820, 0x0048383C),)


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
    row_ctor = extract_function(source, "NetWinConstructedLobbyRow::NetWinConstructedLobbyRow()")
    ctor_body = row_ctor[row_ctor.index("{") + 1:row_ctor.rfind("}")]
    assert fixture.count("/* ROW_CTOR_BODY_INSERTION */") == 1
    assert fixture.count("/* EXEC_BODY_INSERTION */") == 1
    generated = fixture.replace("/* ROW_CTOR_BODY_INSERTION */", ctor_body)
    generated = generated.replace(
        "/* EXEC_BODY_INSERTION */", helper + "\n\n" + exec_body
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
