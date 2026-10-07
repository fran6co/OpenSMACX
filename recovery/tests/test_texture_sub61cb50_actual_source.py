"""C++98 host controls for the installed Texture::sub_61cb50 body."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/texture.cpp").is_file())
SOURCE = ROOT / "src/texture.cpp"
FIXTURE = Path(__file__).with_name("fixtures") / "texture_sub61cb50_actual_host.cpp"
HELPER_MARKER = "/* ACTUAL_HELPERS */"
SETUP_MARKER = "/* ACTUAL_SETUP_EDGE */"
OWNER_MARKER = "/* ACTUAL_OWNER */"


def _definition(source: str, name: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    matches = []
    for match in re.finditer(r"\b" + re.escape(name) + r"\s*\(", masked):
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
    assert len(matches) == 1, f"expected one live definition of {name}, found {len(matches)}"
    start = masked.rfind("\n", 0, matches[0][0]) + 1
    depth = 0
    for end in range(matches[0][1], len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated definition for {name}")


def _translation_unit(source: str, refresh_helper: str | None = None) -> str:
    fixture = FIXTURE.read_text(encoding="utf-8")
    for marker in (HELPER_MARKER, SETUP_MARKER, OWNER_MARKER):
        assert fixture.count(marker) == 1
    helpers = _definition(source, "texture_span_step")
    helper = (refresh_helper if refresh_helper is not None else
              _definition(source, "texture_sub61cb50_advance_edge"))
    setup = _definition(source, "Texture::setup_edge")
    owner = _definition(source, "Texture::sub_61cb50")
    return (fixture.replace(HELPER_MARKER, helpers + "\n\n" + helper, 1)
            .replace(SETUP_MARKER, setup, 1)
            .replace(OWNER_MARKER, owner, 1))


def _compile_and_run(source: str, compiler_name: str, tmp_path: Path,
                     stem: str) -> subprocess.CompletedProcess[str]:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    cpp = tmp_path / (stem + ".cpp")
    exe = tmp_path / stem
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-no-pie",
         str(cpp), "-o", str(exe)], cwd=ROOT, capture_output=True,
        text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], cwd=ROOT, capture_output=True,
                          text=True, timeout=30)


def test_installed_owner_is_unique_typed_void_definition() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    owner = _definition(source, "Texture::sub_61cb50")
    assert owner.startswith("void Texture::sub_61cb50(")
    assert "Buffer *buffer" in owner and "Vert *vertices" in owner
    assert "Texture *shade" in owner and "unsigned char *remap" in owner
    assert "mapped == 0 || mapped == 1" in owner
    assert "buffer->free_data(1)" in owner
    assert "texture_reset_fpu();" in owner
    assert "return 1" not in owner


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_source_remap_clip_page_wrap_and_fractional_uv(
    compiler_name: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    result = _compile_and_run(_translation_unit(source), compiler_name,
                              tmp_path, "texture_sub61cb50_actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_edge_refresh_mutant_fails_expiry_control(
    compiler_name: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    helper = _definition(source, "texture_sub61cb50_advance_edge")
    old = "return Texture::setup_edge(&edge, edge.id);"
    mutant = helper.replace(old, "return 1;", 1)
    assert mutant != helper
    result = _compile_and_run(_translation_unit(source, mutant), compiler_name,
                              tmp_path, "texture_sub61cb50_no_refresh")
    assert result.returncode != 0, "counter-expiry refresh mutant escaped the host control"
