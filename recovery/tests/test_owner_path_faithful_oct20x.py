import os
from pathlib import Path
import shutil
import subprocess

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"
))
SOURCE = ROOT / "src/path.cpp"
SIGNATURE = "void Path::territory(int x, int y, int UNUSED(region), int faction_id)"


def actual_body(*, unconditional_ocean_tail=False):
    records = read_file(SOURCE)
    record, = [row for row in records if row.address == 0x0059C3C0]
    assert Path(record.path).resolve() == SOURCE.resolve()
    assert record.region and SIGNATURE in record.region
    text = SOURCE.read_text()
    start = text.index(SIGNATURE)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            body = text[start:end + 1]
            break
    else:
        raise AssertionError("unterminated actual Path::territory definition")
    if unconditional_ocean_tail:
        body = body.replace(
            "if (is_ocean(x, y)) {\n        return; // skip ocean terrain\n    }",
            "if (is_ocean(x, y)) { }",
            1,
        )
    return body


HARNESS = r'''#include <cassert>
typedef unsigned int uint32_t;
typedef short int16_t;
#define UNUSED(name)
static const int ALT_BIT_SHORE_LINE = 0x60;
static const int MapLatitudeBounds = 64;
struct Map { int climate; int unk_1; int territory; };
static int VehMoveDeltaX[8] = {0};
static int VehMoveDeltaY[8] = {0};
static int ocean;
static int dispatches;
static int map_reads;
static int is_ocean(int, int) { return ocean; }
static int xrange(int x) { return x; }
static int on_map(int, int) { return 0; }
static Map *map_loc(int, int) { ++map_reads; return 0; }
static void do_all_non_input() { ++dispatches; }
class Path {
public:
  int index1_, index2_;
  int16_t *x_table_, *y_table_;
  void territory(int x, int y, int, int faction_id);
};
/* ACTUAL_PATH_TERRITORY */
int main() {
  int16_t xs[8] = {91}, ys[8] = {92};
  Path path; path.index1_ = 17; path.index2_ = 19;
  path.x_table_ = xs; path.y_table_ = ys;
  ocean = 1; dispatches = map_reads = 0;
  path.territory(4, 6, -77, 2);
  assert(dispatches == 0);
  assert(map_reads == 0);
  assert(path.index1_ == 17 && path.index2_ == 19);
  assert(xs[0] == 91 && ys[0] == 92);

  ocean = 0; dispatches = map_reads = 0;
  path.territory(4, 6, -77, 2);
  assert(dispatches == 1);
  assert(map_reads == 0);
  assert(path.index1_ == 1 && path.index2_ == 1);
  assert(xs[0] == 4 && ys[0] == 6);
  return 0;
}
'''


def compile_and_run(tmp_path, compiler, *, unconditional_ocean_tail=False, expect_success=True):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / ("path-unconditional.cpp" if unconditional_ocean_tail else "path-actual.cpp")
    binary = tmp_path / source.stem
    source.write_text(HARNESS.replace(
        "/* ACTUAL_PATH_TERRITORY */",
        actual_body(unconditional_ocean_tail=unconditional_ocean_tail),
    ))
    built = subprocess.run(
        [executable, "-std=c++03", "-O1", "-Wall", "-Wextra",
         "-Werror", "-Wno-unused-parameter", "-pedantic", str(source),
         "-o", str(binary)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True,
                            timeout=30)
    if expect_success:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "unconditional-ocean-tail mutant survived"
        assert "dispatches == 0" in result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_territory_body_skips_ocean_dispatch(compiler, tmp_path):
    compile_and_run(tmp_path, compiler)


def test_unconditional_ocean_tail_mutant_changes_original_dispatch(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler")
    compile_and_run(tmp_path, compiler, unconditional_ocean_tail=True,
                    expect_success=False)


def test_pinned_territory_ocean_branch_bypasses_dispatch_call():
    import hashlib
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    assert IMAGE.is_absolute() and IMAGE.is_file()
    data = IMAGE.read_bytes()
    assert hashlib.sha256(data).hexdigest() == IMAGE_SHA256
    image = PeImage(data, image_pin(ROOT))
    span = Span(0x0059C3C0, 0x0059C51B)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    rows = list(decoder.disasm(image.read(span), span.start))
    assert rows and rows[0].address == span.start
    assert rows[-1].address + rows[-1].size == span.end
    by_address = {row.address: (row.mnemonic, row.op_str) for row in rows}
    assert by_address[0x0059C3FA] == ("jne", "0x59c514")
    assert by_address[0x0059C50F] == ("call", "0x5fcb20")
    assert 0x0059C3FA < 0x0059C50F < 0x0059C514
    assert by_address[0x0059C514][0] == "pop"
