"""Actual raster controls for address-domain shade storage.

High host addresses expose truncation; this is not proof of native ABI,
complete raster equivalence, or executable acceptance.
"""
import shutil
import subprocess

import pytest

from match_recovery.source import read_text

from test_texture_raster_actual_source import _continuity_fields
import test_texture_raster_edge_members_oct10 as edge


BASELINE = "98dfa254c7389a11f966d636012b9f99842592ed"


def test_shade_cursor_preserves_all_texture_original_obligations():
    before = subprocess.run(
        ["git", "show", f"{BASELINE}:src/texture.cpp"], cwd=edge.ROOT,
        check=True, capture_output=True, text=True,
    ).stdout
    old = read_text(before, edge.TEXTURE)
    current = read_text(edge.TEXTURE.read_text(), edge.TEXTURE)
    assert len(old) == len(current) == 96
    assert [_continuity_fields(row) for row in current] == [
        _continuity_fields(row) for row in old
    ]
    assert [(row.symbol, row.function_fragment) for row in current] == [
        (row.symbol, row.function_fragment) for row in old
    ]


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("truncate", (False, True))
def test_actual_shade_readers_retain_high_plane_addresses(
    compiler, truncate, tmp_path, monkeypatch,
):
    edge._require_low_address_host()
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is unavailable")
    fixture = edge.FIXTURE.replace(
        "unsigned char *shade_table = arena + 0x800;",
        "unsigned char high_shade[256] = {};\n"
        "    assert(reinterpret_cast<uintptr_t>(high_shade) > UINT32_MAX);\n"
        "    unsigned char *shade_table = high_shade;",
        1,
    ).replace("shade_table = arena + 0x800;", "shade_table = high_shade;")
    assert fixture != edge.FIXTURE and "shade_table = arena + 0x800;" not in fixture
    monkeypatch.setattr(edge, "FIXTURE", fixture)
    source = edge.TEXTURE.read_text()
    if truncate:
        old = "return reinterpret_cast<uintptr_t>(pointer);"
        assert source.count(old) == 1
        source = source.replace(
            old, "return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pointer));",
            1,
        )
    result = edge._compile_and_run(
        source, compiler_path, tmp_path, f"shade_{compiler}_{truncate}",
    )
    if truncate:
        assert result.returncode != 0, "truncation damage must not pass pixel controls"
    else:
        assert result.returncode == 0, result.stdout + result.stderr
