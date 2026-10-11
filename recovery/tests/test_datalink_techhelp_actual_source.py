"""Apply-ready test: reads only installed source, never the proposal emitter."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.object_source import parse_objects
from match_recovery.source import read_file

PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def repository_root() -> Path:
    for path in Path(__file__).resolve().parents:
        if (path / "CMakeLists.txt").is_file() and (path / "src/IMAGE_PIN").is_file():
            return path
    raise RuntimeError("repository root not found")


def extract_function(text: str, signature: str) -> str:
    assert text.count(signature) == 1
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    quote = None
    escaped = False
    for pos in range(opening, len(text)):
        ch = text[pos]
        if quote:
            if escaped:
                escaped = False
            elif ch == "\\":
                escaped = True
            elif ch == quote:
                quote = None
            continue
        if ch in "\"'":
            quote = ch
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError("unterminated source definition")


def test_installed_datalink_owner_identity_and_objects():
    root = repository_root()
    owner = root / "src/datalink.cpp"
    rows = [row for row in read_file(owner) if row.address == 0x0044B340]
    assert len(rows) == 1
    row = rows[0]
    assert row.path == owner
    assert row.mode.value == "body"
    assert row.name == "sub_44b340"
    assert row.symbol == "?sub_44b340@@YAXP6AXPAD@Z@Z"
    assert row.image_spans == ((0x0044B340, 0x0044B37B),)
    assert row.byte_exact
    objects = parse_objects(owner.read_text(), "src/datalink.cpp", PIN,
                            address_space="pe:va")
    by_address = {item.origin.address: item for item in objects}
    assert set(by_address) >= {0x7591BC, 0x684634, 0x684638, 0x68463C, 0x684648}
    assert by_address[0x684638].span is None
    assert by_address[0x68463C].span is None
    assert by_address[0x684648].span is None


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_installed_body_text_callback_controls(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    root = repository_root()
    owner = (root / "src/datalink.cpp").read_text()
    function = extract_function(
        owner, "void __cdecl sub_44b340(void (__cdecl *callback)(char *))")
    globals_source = "\n".join((
        "int TechnologyHelpPrefixEmitted;",
        "int TechnologyHelpSectionSelector = -1;",
        'char TechnologyHelpPrefixText[] = "^";',
        "static char *TextBufferGetPtr = 0;",
    ))
    harness = r"""
#include <cassert>
#define __cdecl
__GLOBALS__
static char *got[4];
static int count = 0;
static char *redirect = 0;
static void __cdecl collect(char *value) {
    got[count++] = value;
    if (value == TechnologyHelpPrefixText) {
        assert(TechnologyHelpPrefixEmitted == 1);
        if (redirect) TextBufferGetPtr = redirect;
    }
}
__FUNCTION__
int main() {
    char text[] = "staged";
    TextBufferGetPtr = text;
    sub_44b340(collect);
    assert(count == 2);
    assert(got[0] == TechnologyHelpPrefixText);
    assert(got[1] == text);
    assert(TechnologyHelpPrefixEmitted == 1);
    count = 0;
    sub_44b340(collect);
    assert(count == 1 && got[0] == text);
    TechnologyHelpPrefixEmitted = 0;
    TechnologyHelpSectionSelector = 0;
    count = 0;
    sub_44b340(collect);
    assert(count == 1 && got[0] == text);
    assert(TechnologyHelpPrefixEmitted == 0);
    char redirected[] = "redirected";
    redirect = redirected;
    TechnologyHelpSectionSelector = -1;
    count = 0;
    sub_44b340(collect);
    assert(count == 2 && got[0] == TechnologyHelpPrefixText && got[1] == redirected);
    return 0;
}
"""
    cpp = tmp_path / "installed_datalink_44b340.cpp"
    cpp.write_text(harness.replace("__GLOBALS__", globals_source)
                   .replace("__FUNCTION__", function))
    binary = tmp_path / "installed_datalink_44b340"
    compiled = subprocess.run(
        [executable, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert ran.returncode == 0, ran.stdout + ran.stderr
