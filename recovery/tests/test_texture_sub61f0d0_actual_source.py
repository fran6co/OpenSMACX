"""Bounded host control for the actual Texture::sub_61f0d0 body.

The runtime stops the first edge setup before rasterization. It verifies guard
ordering, selected Texture members, current Buffer rectangle publication and
the common Buffer release path; it is not a pixel/raster equivalence test.
"""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/texture.cpp").is_file())
SOURCE = ROOT / "src/texture.cpp"
FIXTURE = Path(__file__).parent / "fixtures" / "texture_sub61f0d0_actual_host.cpp"
SIGNATURE = "void Texture::sub_61f0d0(Buffer *param_2, int param_3, int param_4, int param_5,"
INSERTION = "// ACTUAL_METHOD_INSERTION"
DRAW_TRANS2_INSERTION = "// ACTUAL_DRAW_TRANS2_INSERTION"
DRAW_TRANS3_INSERTION = "// ACTUAL_DRAW_TRANS3_INSERTION"


def extract_method(source: str, signature: str = SIGNATURE) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    assert masked.count(signature) == 1, "expected one installed method definition"
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    index = opening
    while index < len(masked):
        char = masked[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError("unterminated Texture::sub_61f0d0 body")


def translation_unit(method: str, draw_trans2: str | None = None,
                     draw_trans3: str | None = None) -> str:
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(INSERTION) == 1
    assert fixture.count(DRAW_TRANS2_INSERTION) == 1
    assert fixture.count(DRAW_TRANS3_INSERTION) == 1
    source = SOURCE.read_text(encoding="utf-8")
    if draw_trans2 is None:
        draw_trans2 = extract_method(
            source, "int Texture::draw_trans2(Buffer *a1, int *a2, int a3)"
        )
    if draw_trans3 is None:
        draw_trans3 = extract_method(
            source, "int Texture::draw_trans_3(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5, int a6)"
        )
    return (fixture.replace(INSERTION, method, 1)
            .replace(DRAW_TRANS2_INSERTION, draw_trans2, 1)
            .replace(DRAW_TRANS3_INSERTION, draw_trans3, 1))


def build_and_run(source: str, compiler_name: str, expect_success: bool) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    with tempfile.TemporaryDirectory(prefix="texture-61f0d0-host-") as directory:
        cpp = Path(directory) / "control.cpp"
        binary = Path(directory) / "control"
        cpp.write_text(source, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Wno-unused-parameter",
             "-Wno-unused-function", "-no-pie", str(cpp), "-o", str(binary)],
            cwd=ROOT, capture_output=True, text=True, timeout=60,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
        if expect_success:
            assert ran.returncode == 0, ran.stdout + ran.stderr
        else:
            assert ran.returncode != 0, "mutant unexpectedly passed runtime control"


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_source_setup_bindings_and_release_control(compiler: str) -> None:
    body = extract_method(SOURCE.read_text(encoding="utf-8"))
    # The raw source must retain the redundant non-null RECT pointer test and
    # short-circuit order (shade table first, then rectangle pointer).
    assert "RECT *clip = &param_2->rect1_;" in body
    assert "if (((*blit_shade_table) != 0) && (clip != 0))" in body
    assert body.index("param_3 != 0") < body.index("param_2 == 0")
    build_and_run(translation_unit(body), compiler, expect_success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_draw_trans2_and_draw_trans3_actual_source_controls(compiler: str) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    method2 = extract_method(
        source, "int Texture::draw_trans2(Buffer *a1, int *a2, int a3)"
    )
    method3 = extract_method(
        source, "int Texture::draw_trans_3(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5, int a6)"
    )
    assert "reinterpret_cast<int *>(corners_)" in method2
    assert "reinterpret_cast<int *>(cornersAndMids_)" in method2
    assert "RECT *clip = &a1->rect1_;" in method2
    assert "RECT *clip = &a1->rect1_;" in method3
    assert "if (a6 == -1)" in method3 and method3.index("if (a6 == -1)") < method3.index("a1->get_data()")
    build_and_run(translation_unit(extract_method(source), method2, method3),
                  compiler, expect_success=True)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("method,old,new", (
    (2, "reinterpret_cast<int *>(corners_)",
     "reinterpret_cast<int *>(cornersAndMids_)"),
    (2, "reinterpret_cast<int *>(cornersAndMids_)",
     "reinterpret_cast<int *>(corners_)"),
    (2, "RECT *clip = &a1->rect1_;", "RECT *clip = &a1->rect2_;"),
    (2, "*blit_texture_width = static_cast<int>(iWidth_);",
     "*blit_texture_width = static_cast<int>(iHeight_);"),
    (3, "RECT *clip = &a1->rect1_;", "RECT *clip = &a1->rect2_;"),
    (3, "*blit_texture_width = iWidth_;", "*blit_texture_width = iHeight_;"),
))
def test_trans_setup_controls_reject_wrong_members(
    compiler: str, method: int, old: str, new: str,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    signatures = {
        2: "int Texture::draw_trans2(Buffer *a1, int *a2, int a3)",
        3: "int Texture::draw_trans_3(Buffer *a1, Vert *a2, int *a3, int a4, Vert *a5, int a6)",
    }
    body = extract_method(source, signatures[method])
    assert body.count(old) == 1
    mutant = body.replace(old, new, 1)
    build_and_run(translation_unit(
        extract_method(source), mutant if method == 2 else None,
        mutant if method == 3 else None), compiler, expect_success=False)


@pytest.mark.parametrize(
    ("old", "new"),
    [
        ("reinterpret_cast<int *>(corners_)", "reinterpret_cast<int *>(cornersAndMids_)"),
        ("reinterpret_cast<int *>(cornersAndMids_)", "reinterpret_cast<int *>(corners_)"),
        ("RECT *clip = &param_2->rect1_;", "RECT *clip = &param_2->rect2_;"),
        ("static_cast<int>(iWidth_)", "static_cast<int>(iHeight_)"),
    ],
)
@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_runtime_controls_reject_member_or_clip_rectangle_mutants(
    old: str, new: str, compiler: str
) -> None:
    body = extract_method(SOURCE.read_text(encoding="utf-8"))
    mutant = body.replace(old, new, 1)
    assert mutant != body, f"mutation anchor absent: {old}"
    build_and_run(translation_unit(mutant), compiler, expect_success=False)
