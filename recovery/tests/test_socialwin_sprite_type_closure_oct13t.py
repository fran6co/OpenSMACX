"""Actual-source controls for SocialWin's Sprite fields and CRT calls.

The extracted snippets are bounded host controls. They do not establish VC6
layout, application ABI, full initialization, or game acceptance.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOCIALWIN = ROOT / "src/socialwin.cpp"
SPRITE = ROOT / "src/sprite.h"


def braced_definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def accessor_definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    closing = text.index("}", opening)
    return text[start:closing + 1]


def socialwin_parts():
    text = SOCIALWIN.read_text(encoding="utf-8")
    signatures = (
        "void SocialWin::draw_overview()",
        "int SocialWin::on_button_text_draw(",
        "void SocialWin::draw_mouseover()",
        "void SocialWin::draw_info()",
    )
    return text, {signature: braced_definition(text, signature)
                  for signature in signatures}


def itoa_statements(parts):
    statements = []
    for signature, body in parts.items():
        calls = [line.strip() for line in body.splitlines() if "_itoa(" in line]
        assert calls, f"no _itoa call retained in {signature}"
        statements.extend(calls)
    return statements


SPRITE_HARNESS = r'''#include <cstdio>
#include <cstdlib>
struct Sprite {
    char *pcBits_;
    unsigned char cTransparentIndex_;
    char *filename_;
ACCESSORS
};
static void fail(const char *message) {
    std::fprintf(stderr, "%s\n", message);
    std::exit(98);
}
int main() {
    Sprite sprite = {};
    sprite.filename_ = const_cast<char *>("icon.pcx");
    sprite.cTransparentIndex_ = 0x80;
    if (sprite.transparent_index() != 0x80) fail("transparent 0x80");
    if (sprite.has_pixel_pointer()) fail("null pcBits_ with filename present");
    sprite.cTransparentIndex_ = 0xff;
    if (sprite.transparent_index() != 0xff) fail("transparent 0xff");
    // This pointer deliberately names the filename. The query reports only
    // the presence of pcBits_; it does not classify the pointee or prove that
    // the Sprite has completed initialization.
    sprite.pcBits_ = sprite.filename_;
    if (!sprite.has_pixel_pointer()) fail("nonnull pcBits_");
    return 0;
}
'''


def compile_run(tmp_path, compiler, source, stem):
    cpp = tmp_path / f"{stem}.cpp"
    executable = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    build = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(executable)], capture_output=True,
                           text=True, timeout=30)
    assert build.returncode == 0, build.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


COMPILERS = [path for path in (shutil.which("g++"), shutil.which("clang++")) if path]


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_sprite_accessors_host_controls(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    header = SPRITE.read_text(encoding="utf-8")
    accessors = "\n".join((
        accessor_definition(header, "unsigned char transparent_index() const"),
        accessor_definition(header, "bool has_pixel_pointer() const"),
    ))
    result = compile_run(tmp_path, compiler,
                         SPRITE_HARNESS.replace("ACCESSORS", accessors),
                         "socialwin-sprite-accessors")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", COMPILERS or [None])
@pytest.mark.parametrize(
    ("signature", "before", "after", "expected_message"),
    [
        ("unsigned char transparent_index() const", "return cTransparentIndex_;",
         "return cTransparentIndex_ & 0x7f;", "transparent 0x80"),
        ("bool has_pixel_pointer() const", "return pcBits_ != 0;",
         "return filename_ != 0;", "null pcBits_ with filename present"),
    ],
)
def test_sprite_accessor_mutants_compile_and_fail_named_control(
        tmp_path, compiler, signature, before, after, expected_message):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    header = SPRITE.read_text(encoding="utf-8")
    accessors = "\n".join((
        accessor_definition(header, "unsigned char transparent_index() const"),
        accessor_definition(header, "bool has_pixel_pointer() const"),
    ))
    accessor = accessor_definition(accessors, signature)
    assert before in accessor, f"actual accessor lacks expected expression: {signature}"
    mutated = accessors.replace(accessor, accessor.replace(before, after, 1), 1)
    result = compile_run(tmp_path, compiler,
                         SPRITE_HARNESS.replace("ACCESSORS", mutated),
                         "socialwin-sprite-accessor-mutant")
    assert result.returncode == 98
    assert expected_message in result.stderr


def test_socialwin_uses_typed_sprite_members_and_direct_crt_calls():
    text, parts = socialwin_parts()
    mouseover = parts["void SocialWin::draw_mouseover()"]
    init = braced_definition(text, "void SocialWin::init(")
    assert "unsigned char frame = icon->transparent_index();" in mouseover
    assert "icon->draw(buf, frame," in mouseover
    assert "reinterpret_cast<unsigned char *>(icon)[8]" not in mouseover
    assert "effectIcons_[0].has_pixel_pointer()" in init
    assert "reinterpret_cast<char *>(&effectIcons_[0]) + 4" not in init
    assert "StringUpperSlot" not in text
    assert "ItoaFn" not in text
    assert "reinterpret_cast<ItoaFn>(_itoa)" not in text
    statements = itoa_statements(parts)
    assert len(statements) == 11
    assert all("reinterpret_cast" not in statement for statement in statements)
    assert all(statement.startswith("_itoa(") for statement in statements)


def itoa_harness(statements):
    return r'''#include <cstdio>
#include <cstdlib>
#include <cstring>
struct Call { int value; int radix; char *destination; };
static Call calls[11];
static int call_count;
static char * _itoa(int value, char *destination, int radix) {
    if (call_count >= 11) std::exit(97);
    calls[call_count++] = {value, radix, destination};
    std::sprintf(destination, "%d", value);
    return destination;
}
int value=17, current=21, magnitude=0x80;
int NetIncomeGlobal=-7, BreakthroughTurnsGlobal=0xff;
int shown=5, percent2=6, percent=7, upheaval=8;
char number[80];
int main() {
    STATEMENTS
    const int expected[] = {17, 21, 0x80, -7, 0xff, -5, -6, -7, -7, -8, 8};
    if (call_count != 11) { std::fprintf(stderr, "itoa call count/order\n"); return 98; }
    for (int i = 0; i < call_count; ++i) {
        if (calls[i].value != expected[i] || calls[i].radix != 10 ||
            calls[i].destination != number) {
            std::fprintf(stderr, "itoa call arguments/order at %d\n", i);
            return 98;
        }
    }
    return 0;
}
'''.replace("STATEMENTS", "\n".join(statements))


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_actual_itoa_call_expressions_preserve_values_radix_and_order(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    _, parts = socialwin_parts()
    result = compile_run(tmp_path, compiler, itoa_harness(itoa_statements(parts)),
                         "socialwin-itoa-source-expressions")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", COMPILERS or [None])
def test_itoa_argument_mutant_compiles_and_fails_named_control(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler required")
    _, parts = socialwin_parts()
    statements = itoa_statements(parts)
    statements[2] = statements[2].replace("magnitude", "-magnitude", 1)
    result = compile_run(tmp_path, compiler, itoa_harness(statements),
                         "socialwin-itoa-mutant")
    assert result.returncode == 98
    assert "itoa call arguments/order at 2" in result.stderr
