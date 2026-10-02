"""Actual tech_pick and PickTech lifecycle statements with explicit host doubles.

This projects the full real PickTech class declaration without its native sizeof
assertion. Component layout, native EH, actual modal behavior and restoration are
not proved. No recovered caller or lifecycle body lives in the fixture.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/picktech.cpp"
HEADER = ROOT / "src/picktech.h"
FIXTURE = Path(__file__).with_name("tech_pick_host_fixture.inc")
LEXEMES = re.compile(
    r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|'
    r"'(?:\\.|[^'\\])*'|[{}]"
)

def extract_braced(source, marker, declaration=False):
    match = re.search(r"(?m)^" + re.escape(marker), source)
    assert match is not None, marker
    opening = source.index("{", match.start())
    depth = 0
    for token in LEXEMES.finditer(source, opening):
        if token.group() == "{":
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if depth == 0:
                end = token.end()
                if declaration:
                    assert source[end] == ";"
                    end += 1
                return source[match.start():end]
    raise AssertionError(marker)

def projected_unit(damage):
    source = SOURCE.read_text()
    header = HEADER.read_text()
    assert 'static_assert(sizeof(PickTech) == 0xC5C' in header
    declaration = extract_braced(header, "class PickTech : public GraphicWin", True)
    methods = "\n\n".join(extract_braced(source, marker) for marker in (
        "PickTech::PickTech()", "void PickTech::close()", "PickTech::~PickTech()",
        "int __cdecl tech_pick(",
    ))
    if damage is not None:
        replacements = {
            "double_close": (
                "return picker.pick(faction, mode, count, section);",
                "int result = picker.pick(faction, mode, count, section);\n"
                "    picker.close();\n    return result;",
            ),
            "argument_order": (
                "picker.pick(faction, mode, count, section)",
                "picker.pick(faction, count, mode, section)",
            ),
            "lost_result": (
                "return picker.pick(faction, mode, count, section);",
                "picker.pick(faction, mode, count, section); return 0;",
            ),
        }
        old, new = replacements[damage]
        assert methods.count(old) == 1
        methods = methods.replace(old, new, 1)
    fixture = FIXTURE.read_text()
    assert "class PickTech :" not in fixture
    for marker in ("/* PRODUCT_DECLARATION */", "/* PRODUCT_LIFETIME_AND_CALLER */"):
        assert fixture.count(marker) == 1
    return fixture.replace("/* PRODUCT_DECLARATION */", declaration).replace(
        "/* PRODUCT_LIFETIME_AND_CALLER */", methods
    )

@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "double_close", "argument_order", "lost_result"])
def test_caller_lifetime(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    unit = tmp_path / "tech_pick_host.cpp"
    unit.write_text(projected_unit(damage))
    executable = tmp_path / "tech_pick_host"
    compiled = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(unit), "-o", str(executable)],
        capture_output=True, text=True, timeout=60,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    run = subprocess.run([str(executable)], capture_output=True, text=True,
                         env=environment, timeout=30)
    if damage is None:
        assert run.returncode == 0, run.stdout + run.stderr
    else:
        assert run.returncode != 0
        assert "Assertion" in run.stderr or "assertion" in run.stderr

