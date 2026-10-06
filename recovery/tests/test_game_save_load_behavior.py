"""Execute actual owning-TU save/load bodies against external-service doubles.

This does not implement or prove the unfinished game_data serializer or game runtime.
"""
from pathlib import Path
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/game.cpp"
FIXTURE = Path(__file__).with_name("game_save_load_host_control.cpp")

def extract(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError(f"unterminated actual body: {signature}")

SIGNATURES = ('int __cdecl sub_5a6510', 'int __cdecl game_io(MapWin *record', 'static int map_data_plain_block', 'static int map_data_block', 'int __cdecl map_data(FILE *file', 'int __cdecl load_map_daemon(char *path)', 'int __cdecl save_map_daemon(char *path)', 'int __cdecl load_daemon(LPCSTR path', 'void __cdecl save_map()')

@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_save_load_bodies(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    bodies = "\n\n".join(extract(SOURCE.read_text(), signature) for signature in SIGNATURES)
    fixture = FIXTURE.read_text()
    marker = "/* ACTUAL_SAVE_LOAD_BODIES */"
    assert fixture.count(marker) == 1
    cpp = tmp_path / "actual.cpp"
    binary = tmp_path / "control"
    fixture = fixture.replace(marker, bodies)
    state_header = ROOT / "src/game_data_state_window.h"
    state_marker = "/* ACTUAL_GAME_DATA_STATE_HEADER */"
    assert fixture.count(state_marker) == 1
    fixture = fixture.replace(state_marker, f'#include "{state_header}"')
    cpp.write_text(fixture)
    built = subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_c_linkage_declaration_matches_product_header():
    declaration = 'extern "C" int __cdecl sub_5a6510(int x, int y);'
    assert declaration in (ROOT / "src/game.h").read_text()
    assert 'extern "C" int __cdecl sub_5a6510(int x, int y) {' in SOURCE.read_text()
