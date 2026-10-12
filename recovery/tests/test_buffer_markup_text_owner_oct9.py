"""Actual-source host control for Buffer's six-byte markup token dispatch."""
from __future__ import annotations

import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


def repository_root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "CMakeLists.txt").is_file() and (parent / "src/buffer.cpp").is_file():
            return parent
    raise RuntimeError("cannot locate OpenSMACX repository root")


def extract_definition(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0 or source.find(signature, start + len(signature)) >= 0:
        raise AssertionError(f"expected exactly one installed definition: {signature}")
    opening = source.find("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated body: {signature}")


PRELUDE = r"""
#include <cassert>
#include <cstring>
#define __cdecl
typedef const char *LPCSTR;
const char *__cdecl memrchr(LPCSTR, LPCSTR, char);
struct Font;
static Font *FontDefault;
static int call_count;
static int call_font[8];
static int call_len[8];
static char *call_text[8];
struct Font {
    int identity;
    char *find_line_break_l(char *text, int *, int length) {
        call_font[call_count] = identity;
        call_len[call_count] = length;
        call_text[call_count] = text;
        ++call_count;
        return 0;
    }
};
class Buffer {
public:
    char *find_line_break_l(char *, int *, int);
    Font *font1_;
    Font *font2_;
    Font *font3_;
    Font *font4_;
    int markup_enabled_;
    int field_514_;
    int markup_pending_;
};
"""

MAIN = r"""
int main() {
    Font f0 = {0}, f1 = {1}, f2 = {2}, f3 = {3};
    FontDefault = &f0;
    Buffer buffer = {&f0, &f1, &f2, &f3, 1, 0, 0};
    char text[] = "$LINK<abc";
    int break_len = 9;
    char *result = buffer.find_line_break_l(text, &break_len, 9);
    assert(result == 0);
    assert(call_count == 2);
    assert(call_font[0] == 0 && call_text[0] == text && call_len[0] == 0);
    assert(call_font[1] == 3 && call_text[1] == text + 6 && call_len[1] == 3);
    assert(buffer.markup_pending_ == 0 && buffer.field_514_ == 3);
    return 0;
}
"""


def run_control(compiler: str, source: str, *, succeeds: bool) -> None:
    with tempfile.TemporaryDirectory(prefix="buffer-markup-token-") as directory:
        tmp = Path(directory)
        cpp = tmp / "control.cpp"
        exe = tmp / "control"
        cpp.write_text(source)
        built = subprocess.run(
            [compiler, "-std=c++11", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30, check=False)
        assert built.returncode == 0, built.stderr
        result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10, check=False)
        if succeeds:
            assert result.returncode == 0, result.stderr
        else:
            assert result.returncode != 0


COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]
pytestmark = pytest.mark.skipif(not COMPILERS, reason="no supported host C++ compiler found")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_markup_token_actual_parser_and_mutant(compiler: str) -> None:
    source_path = repository_root() / "src/buffer.cpp"
    product = source_path.read_text(encoding="utf-8")
    token_match = re.search(r'^char BufferMarkupLinkToken\[\] = "([^"\\]*)";', product, re.M)
    assert token_match is not None
    assert token_match.group(1) == "$LINK<"
    assert "strncmp(esi, BufferMarkupLinkToken, 6) == 0" in product
    body = extract_definition(product, "char *Buffer::find_line_break_l(char * a1, int * a2, int a3)")
    search = extract_definition(
        (repository_root() / "src/general.cpp").read_text(encoding="utf-8"),
        "const char *__cdecl memrchr(LPCSTR start, LPCSTR end, char value)")

    control = PRELUDE + search + "\n" + token_match.group(0) + "\n" + body + "\n" + MAIN
    run_control(compiler, control, succeeds=True)

    damaged = product.replace('char BufferMarkupLinkToken[] = "$LINK<";',
                              'char BufferMarkupLinkToken[] = "$LINK?";', 1)
    damaged_match = re.search(r'^char BufferMarkupLinkToken\[\] = "([^"\\]*)";', damaged, re.M)
    assert damaged_match is not None
    mutated = PRELUDE + search + "\n" + damaged_match.group(0) + "\n" + body + "\n" + MAIN
    run_control(compiler, mutated, succeeds=False)
