from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


# Installed destination: recovery/tests/test_texture_draw_trans61c090_actual_source.py
ROOT = next(
    parent for parent in Path(__file__).resolve().parents
    if (parent / "src/texture.cpp").is_file()
)
SOURCE = ROOT / "src/texture.cpp"
SIGNATURE = (
    "void Texture::draw_trans(Buffer *buffer, Vert *vertices, int *zvals,\n"
    "                         int count, Vert *uv) {"
)
FIXTURE = ROOT / "recovery/tests/fixtures/texture_draw_trans_host.cpp"
METHOD_INSERTION = "// ACTUAL_METHOD_INSERTION"
HELPER_INSERTION = "// ACTUAL_HELPER_INSERTION"
HELPER_NAMES = (
    "texture_draw_gouraud_edge_step",
    "texture_draw_source_step",
    "texture_draw_source_index",
    "texture_draw_next_source_index",
    "texture_draw_lit_span_mode",
    "texture_draw_lit_span",
    "texture_draw_keyed_lit_span",
)


def _method_body(source: str) -> str:
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("Texture::draw_trans body is unterminated")


def _definition(source: str, marker: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    matches = []
    for match in re.finditer(r"\b" + re.escape(marker) + r"\s*\(", masked):
        depth = 1
        end = match.end()
        while end < len(masked) and depth:
            if masked[end] == "(":
                depth += 1
            elif masked[end] == ")":
                depth -= 1
            end += 1
        while end < len(masked) and masked[end].isspace():
            end += 1
        if depth == 0 and end < len(masked) and masked[end] == "{":
            matches.append((match.start(), end))
    if len(matches) != 1:
        raise AssertionError(f"expected one source definition for {marker}, got {len(matches)}")
    _, opening = matches[0]
    start = masked.rfind("\n", 0, matches[0][0]) + 1
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated source definition for {marker}")


def _actual_translation_unit(source: str, method: str | None = None) -> str:
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(METHOD_INSERTION) == 1
    assert fixture.count(HELPER_INSERTION) == 1
    selected_method = _method_body(source) if method is None else method
    helpers = "\n\n".join(_definition(source, name) for name in HELPER_NAMES)
    return (fixture.replace(HELPER_INSERTION, helpers, 1)
            .replace(METHOD_INSERTION, selected_method, 1))


def _compile_and_run(source: str, compiler: str, tmp_path: Path, stem: str) -> None:
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-no-pie", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def _assert_owner_contract(body: str) -> None:
    required = (
        "buffer->get_data()",
        "TextureLightCeiling",
        "TextureLightFloor",
        "setup_edge_gouraud(&left_edge, top_index)",
        "setup_edge_gouraud(&right_edge, top_index)",
        "texture_span_step(",
        "texture_light_step(",
        "static_cast<const unsigned char *>(pixels_)",
        "texture_draw_keyed_lit_span(",
        "span_light, light_step, pixel_count, key)",
        "buffer->free_data(1)",
        "texture_reset_fpu();",
    )
    missing = [token for token in required if token not in body]
    assert not missing, f"missing native draw behavior: {missing}"
    assert "dst[x] = dst[x]" not in body
    assert "destination[0] = destination[0]" not in body


def test_installed_draw_trans_samples_texture_and_preserves_keyed_lit_path() -> None:
    body = _method_body(SOURCE.read_text(encoding="utf-8"))
    _assert_owner_contract(body)


@pytest.mark.parametrize(
    "old,new",
    [
        ("texture_draw_keyed_lit_span(", "texture_draw_lit_span("),
        ("static_cast<const unsigned char *>(pixels_)", "destination"),
        ("pixel_count, key)", "pixel_count, 0)"),
    ],
)
def test_actual_source_controls_reject_sample_key_or_threshold_regression(
    old: str, new: str,
) -> None:
    body = _method_body(SOURCE.read_text(encoding="utf-8"))
    mutant = body.replace(old, new, 1)
    assert mutant != body
    with pytest.raises(AssertionError):
        _assert_owner_contract(mutant)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_installed_draw_trans_executes_clipped_keyed_uv_and_light_path(
    compiler: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = _method_body(source)
    assert "span_light, light_step, pixel_count, key)" in body
    _compile_and_run(_actual_translation_unit(source), compiler, tmp_path,
                     "texture_draw_trans_actual")


@pytest.mark.parametrize("mutation", ("light_phase", "left_clip", "refresh_order"))
def test_actual_draw_trans_runtime_controls_reject_mutated_behavior(
    mutation: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = _method_body(source)
    if mutation == "light_phase":
        old, new = "span_light, light_step, pixel_count, key)", \
            "span_light >> 8, light_step, pixel_count, key)"
    elif mutation == "left_clip":
        old, new = "if (*blit_clip_right <= right_x)", "if (false)"
    else:
        old, new = "if (!left_failed) {", "if (true) {"
    mutant = body.replace(old, new, 1)
    assert mutant != body
    tu = _actual_translation_unit(source, mutant)
    tool = shutil.which("g++")
    if tool is None:
        pytest.skip("g++ unavailable")
    cpp = tmp_path / f"texture_draw_trans_{mutation}_mutant.cpp"
    binary = tmp_path / f"texture_draw_trans_{mutation}_mutant"
    cpp.write_text(tu, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-no-pie", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=30)
    assert ran.returncode != 0
