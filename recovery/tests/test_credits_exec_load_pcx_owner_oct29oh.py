"""The actual Credits call must pass through GraphicWin's guarded wrapper."""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/credits.cpp"
GRAPHICWIN = ROOT / "src/graphicwin.cpp"


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[start : position + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _credits_load_call(source: str) -> str:
    body = _definition(source, "void Credits::exec(")
    match = re.search(
        r"^\s*(?:GraphicWin|Buffer)::load_pcx\([^;]+;\s*$", body, re.MULTILINE
    )
    assert match, "Credits::exec must contain the production PCX load call"
    return match.group(0).strip()


def _harness(source: str) -> str:
    wrapper = _definition(
        GRAPHICWIN.read_text(encoding="utf-8"), "int GraphicWin::load_pcx("
    )
    call = _credits_load_call(source)
    return r"""
#include <assert.h>
#include <string.h>

struct Palette {};
static char StringTemp[32];

struct Buffer {
    int calls;
    char *last_path;
    Palette *last_palette;
    int last_flags;
    int last_transparency;
    Buffer() : calls(0), last_path(0), last_palette(0), last_flags(0),
               last_transparency(0) {}
    int load_pcx(char *path, Palette *palette, int flags, int transparency) {
        ++calls;
        last_path = path;
        last_palette = palette;
        last_flags = flags;
        last_transparency = transparency;
        return 0;
    }
};

struct GraphicWin : Buffer {
    int iFlags_;
    GraphicWin() : iFlags_(0) {}
    int load_pcx(char *path, Palette *palette, int flags, int transparency);
};
""" + wrapper + r"""

struct Credits : GraphicWin {
    void load_credits_art() {
        CALL_STATEMENT
    }
};

int main() {
    Credits credits;
    strcpy(StringTemp, "credits.pcx");

    credits.iFlags_ = 0x800000;
    credits.load_credits_art();
    if (credits.calls != 0) return 1;

    credits.iFlags_ = 0;
    credits.load_credits_art();
    if (credits.calls != 1 || credits.last_path != StringTemp
        || credits.last_palette != 0 || credits.last_flags != 10
        || credits.last_transparency != 0xEC) return 2;
    return 0;
}
""".replace("CALL_STATEMENT", call)


def _compile_and_run(tmp_path: Path, compiler: str, source: str, name: str) -> int:
    cpp = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(executable)],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10).returncode


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_credits_exec_load_pcx_uses_guarded_graphicwin_wrapper(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host C++ compiler unavailable: {compiler_name}")

    source = SOURCE.read_text(encoding="utf-8")
    harness = _harness(source)
    assert _compile_and_run(tmp_path, compiler, harness, "credits_guarded_call") == 0

    direct_buffer_source = source.replace(
        "GraphicWin::load_pcx(StringTemp, 0, 10, 0xEC);",
        "Buffer::load_pcx(StringTemp, 0, 10, 0xEC);",
    )
    assert direct_buffer_source != source
    direct_buffer_harness = _harness(direct_buffer_source)
    assert _compile_and_run(
        tmp_path, compiler, direct_buffer_harness, "credits_direct_buffer_mutant"
    ) != 0
