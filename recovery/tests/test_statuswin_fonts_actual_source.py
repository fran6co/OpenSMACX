"""Actual-source host controls for StatusWin's two font/layout bodies."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name("statuswin_fonts_host.cpp")
SIGNATURES = ("void StatusWin::UNK1()", "void StatusWin::on_redraw()")


def extract(source: str, signature: str) -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    code = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)
    assert code.count(signature) == 1, f"expected exactly one product definition for {signature}"
    start = code.index(signature)
    opening = code.index("{", start)
    depth = 0
    for end in range(opening, len(code)):
        if code[end] == "{":
            depth += 1
        elif code[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated product definition: {signature}")


def build_unit(tmp_path: Path, bodies: tuple[str, str], name: str) -> Path:
    text = FIXTURE.read_text(encoding="utf-8")
    for marker, body in zip(("/* PRODUCT_UNK1 */", "/* PRODUCT_ON_REDRAW */"), bodies):
        assert text.count(marker) == 1
        text = text.replace(marker, body)
    source = tmp_path / f"{name}.cpp"
    source.write_text(text, encoding="utf-8")
    return source


def compile_run(tmp_path: Path, compiler: str, bodies: tuple[str, str], name: str) -> subprocess.CompletedProcess:
    source = build_unit(tmp_path, bodies, name)
    exe = tmp_path / name
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(exe)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)


def product_bodies() -> tuple[str, str]:
    source = (ROOT / "src/statuswin.cpp").read_text(encoding="utf-8")
    return tuple(extract(source, signature) for signature in SIGNATURES)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_statuswin_font_and_redraw_bodies(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    result = compile_run(tmp_path, compiler, product_bodies(), "statuswin-fonts")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_statuswin_behavioral_damage_controls(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    unk1, redraw = product_bodies()
    good = compile_run(tmp_path, compiler, (unk1, redraw), "statuswin-good")
    assert good.returncode == 0, good.stdout + good.stderr

    mutants = (
        (unk1.replace("font2_.init((char *)DefaultFontFace, size, 2);",
                      "font2_.init((char *)DefaultFontFace, size, 1);")
         if "font2_.init((char *)DefaultFontFace, size, 2);" in unk1 else
         unk1.replace("font2_.init(DefaultFontFace, size, 2);",
                      "font2_.init(DefaultFontFace, size, 1);"), redraw),
        (unk1.replace("if (WinScreenWidth != 800) {", "if (WinScreenWidth == 800) {", 1), redraw),
        (unk1, redraw.replace("draw_status(-1, -1, -1, -1, 0);",
                              "draw_status(-1, -1, -1, -1, 1);", 1)),
        (unk1, redraw.replace("MainInterfaceGlobal.status_left_rect()",
                              "MainInterfaceGlobal.status_right_rect()", 1)),
    )
    for index, mutant in enumerate(mutants):
        assert mutant != (unk1, redraw), f"damage anchor missing for mutant {index}"
        result = compile_run(tmp_path, compiler, mutant, f"statuswin-mutant-{index}")
        assert result.returncode != 0, f"behavioral control missed mutant {index}"


def test_source_keeps_global_reload_and_call_order_anchors():
    unk1, redraw = product_bodies()
    assert unk1.count("WinScreenWidth") >= 2
    assert unk1.index("font1_.init") < unk1.index("font2_.init") < unk1.index("font3_.init")
    assert unk1.rindex("WinScreenWidth") > unk1.index("font3_.init")
    assert redraw.index("draw_status") < redraw.index("status_hit_rect")
    assert redraw.index("status_hit_rect") < redraw.index("status_right_rect")
    assert redraw.index("status_left_rect") < redraw.index("soft_update")
