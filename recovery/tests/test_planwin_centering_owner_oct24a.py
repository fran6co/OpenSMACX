from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/planwin.cpp"
CONTROL = ROOT / "recovery/tests/planwin_centering_owner_control.cpp"


def _definition(text: str, signature: str) -> str:
    start = text.find(signature)
    assert start >= 0, f"missing current source definition: {signature}"
    opening = text.find("{", start)
    assert opening >= 0, f"missing body: {signature}"
    depth = 0
    quote: str | None = None
    escaped = False
    line_comment = False
    block_comment = False
    index = opening
    while index < len(text):
        char = text[index]
        next_char = text[index + 1] if index + 1 < len(text) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
        elif block_comment:
            if char == "*" and next_char == "/":
                block_comment = False
                index += 1
        elif quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
        elif char == "/" and next_char == "/":
            line_comment = True
            index += 1
        elif char == "/" and next_char == "*":
            block_comment = True
            index += 1
        elif char in ('"', "'"):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[start : index + 1]
        index += 1
    raise AssertionError(f"unterminated C++ definition: {signature}")


def _actual_bodies() -> tuple[str, str]:
    text = SOURCE.read_text(encoding="utf-8")
    return (
        _definition(text, "void PlanWin::opticenter("),
        _definition(text, "void PlanWin::monocenter("),
    )


def _program(damage: tuple[str, str, str] | None = None) -> str:
    control = CONTROL.read_text(encoding="utf-8")
    optical, mono = _actual_bodies()
    if damage is not None:
        target, old, new = damage
        if target == "monocenter_cached_dims":
            old_snapshot = """        ++zoom_offset_;
        calculate_dimensions();

        const bool flat = (MapIsFlat & 1) != 0;"""
            new_snapshot = """        ++zoom_offset_;
        const int cached_left = field_1DDA4_;
        const int cached_top = field_1DDA8_;
        const int cached_width = field_1DDCC_ + field_1DDD4_;
        const int cached_height = field_1DDD0_ + field_1DDD8_;
        calculate_dimensions();

        const bool flat = (MapIsFlat & 1) != 0;"""
            assert old_snapshot in mono
            mono = mono.replace(old_snapshot, new_snapshot, 1)
            replacements = (
                ("const int left = field_1DDA4_;", "const int left = cached_left;"),
                ("const int top = field_1DDA8_;", "const int top = cached_top;"),
                ("const int right = left + field_1DDCC_ + field_1DDD4_;", "const int right = left + cached_width;"),
                ("const int bottom = top + field_1DDD0_ + field_1DDD8_;", "const int bottom = top + cached_height;"),
            )
            for old_field, new_field in replacements:
                assert old_field in mono
                mono = mono.replace(old_field, new_field, 1)
        else:
            body = optical if target == "opticenter" else mono
            assert old in body, f"mutation anchor missing from {target}: {old}"
            body = body.replace(old, new, 1)
        if target == "opticenter":
            optical = body
        elif target == "monocenter":
            mono = body
    assert control.count("// ACTUAL_OPTICENTER") == 1
    assert control.count("// ACTUAL_MONOCENTER") == 1
    return control.replace("// ACTUAL_OPTICENTER", optical).replace(
        "// ACTUAL_MONOCENTER", mono
    )


COMPILERS = [name for name in ("g++", "clang++") if shutil.which(name)]


def _compile_run(tmp_path: Path, compiler: str, source_text: str) -> subprocess.CompletedProcess[bytes]:
    source = tmp_path / "planwin_centering.cpp"
    binary = tmp_path / "planwin_centering"
    source.write_text(source_text, encoding="utf-8")
    built = subprocess.run(
        [shutil.which(compiler) or compiler, "-std=c++17", "-O0", str(source), "-o", str(binary)],
        capture_output=True,
        text=True,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, timeout=10)


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_centering_methods_preserve_scan_and_callback_behavior(tmp_path, compiler):
    assert "static bool plan_tiles_fit" not in SOURCE.read_text(encoding="utf-8")
    result = _compile_run(tmp_path, compiler, _program())
    assert result.returncode == 0, result.stderr.decode(errors="replace")


MUTATIONS = [
    ("opticenter", "index < 25", "index < 24"),
    ("opticenter", "index < 25", "index < 26"),
    ("opticenter", "int x = x1 + RadiusOffsetX[index];", "int x = x2 + RadiusOffsetX[index];"),
    ("opticenter", "(void)xrange(middle_x);", "x1 = xrange(middle_x);"),
    ("opticenter", "zoom_offset_ = -14;", "zoom_offset_ = -15;"),
    ("opticenter", "--zoom_offset_;", "/* omitted failed-scan decrement */"),
    ("monocenter", "index < 81", "index < 80"),
    ("monocenter", "if (force_redraw || old_x != center_x_ ||", "if (old_x != center_x_ ||"),
    ("monocenter", "Buffer::copy(&buffer_, 0, 0, dib_.bmiHeader.biWidth,\n                     -dib_.bmiHeader.biHeight);", "Buffer::copy(&buffer_, 0, 0, field_1DDCC_,\n                     -dib_.bmiHeader.biHeight);"),
    (
        "monocenter",
        "GraphicWin::fill(0);\n        draw_map(0);\n        Buffer::copy(&buffer_, 0, 0, dib_.bmiHeader.biWidth,\n                     -dib_.bmiHeader.biHeight);",
        "const int copy_width_before_draw = dib_.bmiHeader.biWidth;\n        const int copy_height_before_draw = dib_.bmiHeader.biHeight;\n        GraphicWin::fill(0);\n        draw_map(0);\n        Buffer::copy(&buffer_, 0, 0, copy_width_before_draw,\n                     -copy_height_before_draw);",
    ),
    ("monocenter_cached_dims", "", ""),
]


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("damage", MUTATIONS)
def test_centering_damage_controls_compile_and_fail(tmp_path, compiler, damage):
    result = _compile_run(tmp_path, compiler, _program(damage))
    assert result.returncode != 0, f"damage escaped the actual-source behavior cases: {damage}"
