"""The original 0x61B780 keys borrowed Gouraud texels, but not owned texels.

The host control extracts the real span helpers and Texture entry from
src/texture.cpp. Only Buffer storage, edge setup, and the host surface are
controlled doubles; this is not a native ABI or pixel-exactness claim.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

import test_texture_raster_actual_source as raster


ENTRY_SIGNATURE = "void Texture::sub_61b780("
KEY_CHECK = "if (texel != key) {"
UNKEYED_CHECK = "if (true) {"


def _run(unit: str, compiler: str, tmp_path: Path, stem: str) -> subprocess.CompletedProcess:
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is unavailable")
    cpp = tmp_path / f"{stem}.cpp"
    binary = tmp_path / stem
    cpp.write_text(unit, encoding="utf-8")
    built = subprocess.run(
        [compiler_path, "-std=c++11", "-Wall", "-Wextra",
         "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
        cwd=raster.ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], check=False, capture_output=True,
                          text=True, timeout=45)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_61b780_skips_key_valued_borrowed_texel(compiler, tmp_path):
    source = raster.TEXTURE.read_text(encoding="utf-8")
    entry = raster._extract_definition(source, ENTRY_SIGNATURE)
    assert entry.count(KEY_CHECK) == 1
    borrowed = entry.index("if (borrowed) {")
    owned = entry.index(
        "            } else {\n                int row_offset = TextureRasterRowOffset;\n"
        "                const int pitch = TextureRowPitchScratch;",
        borrowed,
    )
    assert KEY_CHECK in entry[borrowed:owned]
    assert KEY_CHECK not in entry[owned:]

    unit = raster._translation_unit(source)
    ran = _run(unit, compiler, tmp_path, "texture_gouraud_unkeyed")
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_removing_borrowed_key_skip_in_61b780_fails_pixel_control(compiler, tmp_path):
    source = raster.TEXTURE.read_text(encoding="utf-8")
    entry = raster._extract_definition(source, ENTRY_SIGNATURE)
    assert entry.count(KEY_CHECK) == 1
    unkeyed_entry = entry.replace(KEY_CHECK, UNKEYED_CHECK, 1)
    assert unkeyed_entry != entry
    keyed_source = source.replace(entry, unkeyed_entry, 1)

    unit = raster._translation_unit(keyed_source)
    ran = _run(unit, compiler, tmp_path, "texture_gouraud_missing_borrowed_key_mutant")
    assert ran.returncode != 0, "removing the borrowed key skip escaped pixel assertions"

