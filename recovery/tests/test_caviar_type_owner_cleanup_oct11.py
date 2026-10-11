"""Retain the active bounded views after deleting superseded proposals."""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
BASE = "6fd43a4ab"
HEADER = "src/caviar_render_types.h"


def test_live_views_and_their_layout_guards_preserved_verbatim():
    prior = subprocess.check_output(["git", "show", BASE + ":" + HEADER], cwd=ROOT, text=True)
    retained = prior[prior.index("// One variable record"):prior.index("// Proposed owners")].rstrip()
    current = (ROOT / HEADER).read_text()
    assert current[current.index("// One variable record"):].rstrip() == retained
    # The source-owned raster and scratch types retain their actual symbol
    # names/layouts; removal of dead proposals is not a type promotion.
    assert current[:current.index("// One variable record")] == (
        "// Bounded raster-stream and projection-clip views used by the Caviar renderer.\n"
        "// The scene context and callback record are owned in caviar.h. These remaining\n"
        "// views do not establish complete serialized allocations or original type names.\n"
        "#pragma once\n\n#include <stddef.h>\n#include <stdint.h>\n\n"
    )


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_view_header_compiles_with_its_existing_field_contract(compiler, tmp_path):
    compiler = shutil.which(compiler)
    if not compiler:
        pytest.skip("host compiler unavailable")
    source = tmp_path / "views.cpp"
    source.write_text(r'''
#include "caviar_render_types.h"
#include <cassert>
int main() {
 assert(sizeof(CvrRasterItemHeaderCandidate)==0x21);
 assert(offsetof(CvrRasterItemHeaderCandidate,item_count_)==0x18);
 assert(offsetof(CvrRasterItemHeaderCandidate,flags_)==0x1C);
 assert(offsetof(CvrRasterItemHeaderCandidate,inline_payload_)==0x20);
 assert(sizeof(CaviarRenderClipWindowCandidate)==0x18);
 CaviarRenderClipWindowCandidate clip={1,2,3,4,5,6};
 assert(clip.left_==1&&clip.top_==2&&clip.right_==3&&clip.bottom_==4);
 assert(clip.width_==5&&clip.height_==6);
}
''')
    executable = tmp_path / "views"
    result = subprocess.run([compiler, "-std=c++98", "-I", str(ROOT / "src"),
                             str(source), "-o", str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stdout + result.stderr
    assert subprocess.run([str(executable)], capture_output=True).returncode == 0
