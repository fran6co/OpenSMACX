from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
FIXTURE = HERE / "graphicwin_palette_control.cpp"


def extract_definition(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            index += 1
            continue
        if block_comment:
            if char == "*" and nxt == "/":
                block_comment = False
                index += 2
                continue
            index += 1
            continue
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
            index += 1
            continue
        if char == "/" and nxt == "/":
            line_comment = True
            index += 2
            continue
        if char == "/" and nxt == "*":
            block_comment = True
            index += 2
            continue
        if char in ("'", '"'):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError("unterminated proposal definition: " + signature)


def actual_bodies(source):
    signatures = (
        "void GraphicWin::blit_palette_pattern_rows()",
        "void GraphicWin::blit_palette_pattern_rows_entry()",
    )
    return "\n\n".join(extract_definition(source, name) for name in signatures)


def compilers():
    result = []
    for name in ("c++", "clang++"):
        path = shutil.which(name)
        if path and path not in result:
            result.append(path)
    if not result:
        pytest.skip("no host C++ compiler available")
    return result


def compile_and_run(bodies, expected_success=True):
    fixture = FIXTURE.read_text()
    anchor = "/* PRODUCT DEFINITIONS */"
    assert fixture.count(anchor) == 1
    source = fixture.replace(anchor, bodies)
    with tempfile.TemporaryDirectory(prefix="graphicwin-palette-source-") as directory:
        cpp = Path(directory) / "fixture.cpp"
        exe = Path(directory) / "fixture"
        cpp.write_text(source)
        for compiler in compilers():
            built = subprocess.run(
                [compiler, "-std=c++98", "-O2", "-Wall", "-Wextra",
                 "-Werror=return-type", str(cpp), "-o", str(exe)],
                capture_output=True, text=True, timeout=20)
            assert built.returncode == 0, built.stderr
            try:
                result = subprocess.run([str(exe)], capture_output=True,
                                        text=True, timeout=5)
            except subprocess.TimeoutExpired as error:
                pytest.fail("source control hung: " + str(error))
            assert (result.returncode == 0) is expected_success, result.stderr


def test_actual_source_owner_bodies_preserve_hdc_order_and_lfsr_rows():
    source = (ROOT / "src/graphicwin.cpp").read_text()
    assert "RECOVERY_OBJECT: v1 0x00696B6C" in source
    assert '#include "graphicwin_palette_polynomials.inc"' in source
    assert "unsigned int GraphicWinBlitPolynomialWords[33]" in source
    assert "0x005D5B80 sub_5d5b80 0x005D5B80-0x005D5C4E" in source
    assert "0x005D5B70 sub_5d5b70 0x005D5B70-0x005D5B75" in source
    assert 'GraphicWinBlitPolynomialWords[bit_count_signed(rows)]' in source
    assert "Win::release_hdc();\n        return;" in source
    assert "buffer->release_hdc(static_cast<int>(y));" in source
    bodies = actual_bodies(source)
    assert "if (rows == 0)" not in bodies and "while (y != 1)" in bodies
    compile_and_run(bodies)


def test_omitting_buffer_hdc_failure_cleanup_is_detected():
    source = (ROOT / "src/graphicwin.cpp").read_text()
    old = "Win::release_hdc();\n        return;"
    assert old in source
    mutant = source.replace(old, "return;", 1)
    compile_and_run(actual_bodies(mutant), expected_success=False)


def test_omitting_lfsr_polynomial_xor_is_detected():
    source = (ROOT / "src/graphicwin.cpp").read_text()
    old = "y = (y >> 1) ^ polynomial;"
    assert old in source
    mutant = source.replace(old, "y >>= 1;", 1)
    compile_and_run(actual_bodies(mutant), expected_success=False)
