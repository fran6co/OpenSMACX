"""Host behavior controls for the installed Planetfall C++ owner."""
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import Mode, read_file

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/game.cpp").is_file())
GAME = ROOT / "src/game.cpp"
FIXTURE = Path(__file__).parent / "fixtures/planetfall_owner_harness.cpp"
ADDRESS = 0x00589180
SYMBOL = "?planetfall@@YAXH@Z"
SIGNATURE = "void __cdecl planetfall(int faction_id) {"


def actual_source_body():
    source = GAME.read_text()
    assert source.count(SIGNATURE) == 1, "expected one installed planetfall definition"
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and following == "/": state = "line"; index += 2; continue
            if char == "/" and following == "*": state = "block"; index += 2; continue
            if char == '"': state = "string"
            elif char == "'": state = "char"
            elif char == "{": depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    body = source[start:index + 1]
                    assert "BODY GOES HERE." not in body
                    records = read_file(GAME)
                    rows = [row for row in records if row.address == ADDRESS]
                    assert len(rows) == 1
                    row, = rows
                    assert row.name == SYMBOL and row.mode is Mode.BODY
                    assert row.image_spans == ((0x00589180, 0x00589329),)
                    return body
        elif state == "line":
            if char == "\n": state = "code"
        elif state == "block":
            if char == "*" and following == "/": state = "code"; index += 2; continue
        elif state in ("string", "char"):
            if char == "\\": index += 2; continue
            if (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        index += 1
    raise AssertionError("planetfall definition is not brace-balanced")


def compile_and_run(tmp_path, body):
    compiler = shutil.which("g++") or shutil.which("clang++") or shutil.which("c++")
    if not compiler:
        pytest.skip("host C++ compiler is unavailable")
    template = FIXTURE.read_text()
    marker = "/* INSERT INSTALLED PLANETFALL BODY HERE */"
    assert template.count(marker) == 1
    source = tmp_path / "planetfall_actual_source.cpp"
    executable = tmp_path / "planetfall_actual_source"
    source.write_text(template.replace(marker, body))
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Werror", "-pedantic",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_installed_planetfall_actual_source_behavior(tmp_path):
    result = compile_and_run(tmp_path, actual_source_body())
    assert result.returncode == 0, result.stderr


def test_behavior_control_rejects_wrong_parse_call_order(tmp_path):
    body = actual_source_body()
    expected = "parse_says(2, player.title_leader, -1, -1);"
    assert expected in body
    mutant = body.replace(expected, "parse_says(0, player.title_leader, -1, -1);", 1)
    result = compile_and_run(tmp_path, mutant)
    assert result.returncode != 0
