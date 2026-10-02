"""C++03 controls compile actual BaseWin hurry source, never a proposal copy."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

_candidates = list(Path(__file__).resolve().parents) + [Path.cwd()]
ROOT = next(path for path in _candidates if (path / "src" / "basewin.cpp").is_file())
SOURCE = ROOT / "src" / "basewin.cpp"
HARNESS = Path(__file__).with_name("basewin-hurry-harness.cpp")
INSERTION = "/* RECOVERED_SOURCE_INSERTION_POINT */"


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated source definition: " + signature)


def actual_source() -> str:
    source = SOURCE.read_text()
    method_start = source.index("void BaseWin::hurry()")
    method_end = source.index("// The retired scaffold", method_start)
    if "BODY GOES HERE." in source[method_start:method_end]:
        pytest.fail("actual source still has the hurry placeholder")
    helper_start = source.index("int hurry_signed_bits(")
    helper_end = source.index("// ORIGINAL: 0x00417040", helper_start)
    helper_region = source[helper_start:helper_end]
    signatures = ("int hurry_signed_bits(", "int hurry_add(", "int hurry_sub(",
                  "int hurry_mul(", "struct HurryPrice", "HurryPrice hurry_price(",
                  "char *expand_hurry_dialog_item()")
    for signature in signatures:
        assert signature in helper_region, "missing source-owned helper: " + signature
    return "\n".join((
        definition(source, "int hurry_signed_bits("),
        definition(source, "int hurry_add("),
        definition(source, "int hurry_sub("),
        definition(source, "int hurry_mul("),
        source[source.index("struct HurryPrice {"):source.index("HurryPrice hurry_price(")],
        definition(source, "HurryPrice hurry_price("),
        definition(source, "char *expand_hurry_dialog_item()"),
        definition(source, "void BaseWin::compute_selected_base()"),
        definition(source, "void BaseWin::hurry()"),
    ))


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_hurry_body_and_controls(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    template = HARNESS.read_text()
    assert template.count(INSERTION) == 1
    cpp = tmp_path / "basewin_hurry.cpp"
    exe = tmp_path / "basewin_hurry"
    cpp.write_text(template.replace(INSERTION, actual_source()))
    built = subprocess.run((compiler, "-std=c++03", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=address,undefined", str(cpp), "-o", str(exe)),
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run((str(exe),), capture_output=True, text=True, timeout=30,
                         env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))
    assert ran.returncode == 0, ran.stderr
