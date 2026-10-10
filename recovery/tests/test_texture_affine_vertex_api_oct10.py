"""Actual typed API, complete mapping continuity and bounded argument controls."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.source import read_text
from test_texture_edge_cache_actual_source import _masked, _run
from test_texture_raster_actual_source import _continuity_fields, _translation_unit
from test_texture_vertex_api_family_oct10 import SYMBOLS as FAMILY_SYMBOLS

ROOT = Path(__file__).resolve().parents[2]
BASE = "2a85756a748863fcae5fc85538ca0df025032139"
SYMBOLS = {
    0x620A10: "?sub_620a10@Texture@@QAEXPAVBuffer@@PAUVert@@H1PAVTextureStore@@@Z",
    0x624CA0: "?sub_624ca0@Texture@@QAEXPAVBuffer@@PAUVert@@H1PAVTextureStore@@H@Z",
}


def test_all_texture_original_identities_spans_claims_and_lessons_retained():
    path = ROOT / "src/texture.cpp"
    prior = subprocess.check_output(["git", "show", BASE + ":src/texture.cpp"],
                                    cwd=ROOT, text=True)
    before = {row.address: row for row in read_text(prior, path)}
    after = {row.address: row for row in read_text(path.read_text(), path)}
    assert set(after) == set(before)
    for address in before:
        assert _continuity_fields(after[address]) == _continuity_fields(before[address])
        if address in SYMBOLS:
            assert after[address].symbol == SYMBOLS[address]
        elif address in FAMILY_SYMBOLS:
            assert after[address].symbol == FAMILY_SYMBOLS[address]
        else:
            assert after[address].symbol == before[address].symbol
    assert after[0x624CA0].byte_exact


def test_actual_header_declares_pointer_contracts(tmp_path):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    header = _masked((ROOT / "src/texture.h").read_text())
    declarations = []
    for name in ("sub_620a10", "sub_624ca0"):
        matches = re.findall(r"void\s+" + name + r"\([^;]*\);", header)
        assert len(matches) == 1
        declarations.append(matches[0])
    source = """
#include <type_traits>
class Buffer;
class TextureStore;
struct Vert;
class Texture { public:
""" + "\n".join(declarations) + """
};
static_assert(std::is_same<decltype(&Texture::sub_620a10),
    void (Texture::*)(Buffer *, Vert *, int, Vert *, TextureStore *)>::value,
    "callee pointer contract");
static_assert(std::is_same<decltype(&Texture::sub_624ca0),
    void (Texture::*)(Buffer *, Vert *, int, Vert *, TextureStore *, int)>::value,
    "wrapper pointer contract");
"""
    path = tmp_path / "actual_api.cpp"
    path.write_text(source)
    result = subprocess.run([compiler, "-std=c++11", "-fsyntax-only", str(path)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr


def test_swapping_wrapper_vertex_arguments_fails_actual_source_control(tmp_path):
    source = (ROOT / "src/texture.cpp").read_text()
    old = "sub_620a10(a1, a2, a3, a4, a5);"
    assert source.count(old) == 1
    mutated = source.replace(old, "sub_620a10(a1, a4, a3, a2, a5);", 1)
    result = _run(tmp_path, source=mutated)
    assert result.returncode != 0, "swapped vertex channels escaped wrapper control"


def test_wrong_shared_vertex_pointer_fails_actual_raster_control(tmp_path):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    source = (ROOT / "src/texture.cpp").read_text()
    old = "*blit_edge_uvs_a = edge_vertices_arg;"
    assert source.count(old) == 1
    changed = source.replace(old, "*blit_edge_uvs_a = vertices_arg;", 1)
    path, executable = tmp_path / "raster.cpp", tmp_path / "raster"
    path.write_text(_translation_unit(changed))
    built = subprocess.run([compiler, "-std=c++11", "-Wno-unused-parameter",
                            str(path), "-o", str(executable)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(executable)], capture_output=True, timeout=30)
    assert result.returncode != 0, "wrong shared pointer escaped raster control"
