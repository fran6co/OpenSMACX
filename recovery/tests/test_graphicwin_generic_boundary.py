"""Keep GraphicWin independent of unrelated application window owners."""

from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def test_graphicwin_translation_unit_does_not_import_concrete_app_windows() -> None:
    source = (ROOT / "src/graphicwin.cpp").read_text()
    for header in ("statuswin.h", "net_class.h", "basepop.h", "original_seam.h"):
        assert f'#include "{header}"' not in source

    # These concrete-owner APIs and classes had been reachable only through
    # broad includes. Their names are not part of GraphicWin's implementation.
    for name in ("StatusWin", "NetClass", "BasePop", "OriginalSeam"):
        assert name not in source


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_graphicwin_caption_callback_contract_compiles(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    header = (ROOT / "src/graphicwin.h").read_text()
    declaration = "typedef int (__cdecl *GraphicWinCaptionDraw)(GraphicWin *, char *, Buffer *);"
    assert header.count(declaration) == 1
    fixture = """#include <cassert>
#define __cdecl
class GraphicWin;
class Buffer;
DECLARATION
int draw(GraphicWin *, char *, Buffer *) { return 23; }
int main() {
    GraphicWinCaptionDraw callback = &draw;
    assert(callback(0, 0, 0) == 23);
}
""".replace("DECLARATION", declaration)
    translation_unit = tmp_path / "graphicwin-caption-contract.cpp"
    executable = tmp_path / "graphicwin-caption-contract"
    translation_unit.write_text(fixture)
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(translation_unit), "-o", str(executable)],
        check=True, capture_output=True, text=True, timeout=30,
    )
    subprocess.run([str(executable)], check=True, capture_output=True,
                   text=True, timeout=10)
