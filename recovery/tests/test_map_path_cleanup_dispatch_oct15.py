"""Actual-source controls for mapped shutdown calls and inline cleanup."""

from pathlib import Path
import os
import shutil
import subprocess

import pytest

from match_recovery.source import read_file
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/path.h").is_file())
HOST_COMPILERS = [path for path in (shutil.which("g++"), shutil.which("clang++")) if path]


@pytest.fixture(params=HOST_COMPILERS or [None], ids=[Path(p).name for p in HOST_COMPILERS] or ["no-host-compiler"])
def host_compiler(request):
    if request.param is None:
        pytest.skip("no GCC or Clang host compiler available")
    return request.param


def _function(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unclosed function: {signature}")


def _ordered(body: str, fragments: tuple[str, ...]) -> bool:
    positions = [body.index(fragment) for fragment in fragments]
    return positions == sorted(positions)


def test_path_real_and_inline_cleanup_have_native_order_and_null_guards():
    header = (ROOT / "src/path.h").read_text()
    source = (ROOT / "src/path.cpp").read_text()
    helper = _function(header, "void release_storage()")
    mapped = _function(source, "void Path::shutdown()")
    contract = (
        "if (map_table_)", "free(map_table_)",
        "if (x_table_)", "free(x_table_)",
        "if (y_table_)", "free(y_table_)",
        "map_table_ = nullptr", "y_table_ = nullptr", "x_table_ = nullptr",
    )
    assert _ordered(helper, contract)
    assert _ordered(mapped, contract)
    assert "release_storage();" in _function(source, "void Path::init()")
    assert "release_storage();" in _function(header, "MEASURED ~Path()")
    assert "Paths.shutdown();" in _function(
        (ROOT / "src/game.cpp").read_text(), "void __cdecl system_close()")


def test_map_real_and_inline_cleanup_routes_are_distinct_and_ordered():
    header = (ROOT / "src/map.h").read_text()
    source = (ROOT / "src/map.cpp").read_text()
    helper = _function(header, "void __cdecl release_map_storage()")
    mapped = _function(source, "void __cdecl map_shutdown()")
    contract = (
        "if (map_tiles())", "free(map_tiles())",
        "if (MapAbstract())", "free(MapAbstract())",
        "map_tiles() = 0", "MapAbstract() = 0",
    )
    assert _ordered(helper, contract)
    assert _ordered(mapped, contract)
    assert "release_map_storage();" in _function(source, "BOOL __cdecl map_init()")
    assert "map_shutdown();" not in _function(source, "BOOL __cdecl map_init()")
    assert "release_map_storage();" in _function(source, "BOOL __cdecl map_read(FILE *map_file)")
    assert "map_shutdown();" not in _function(source, "BOOL __cdecl map_read(FILE *map_file)")
    assert "map_shutdown();" in _function(
        (ROOT / "src/console.cpp").read_text(), "void Console::editor_clear()")
    assert "map_shutdown();" in _function(
        (ROOT / "src/game.cpp").read_text(), "void __cdecl system_close()")
    sync = _function((ROOT / "src/netdaemon.cpp").read_text(), "void NetDaemon::synch(")
    reset = sync[sync.index("} else if (opcode == 0x02"):]
    assert _ordered(reset, ("map_shutdown();", "map_init();", "Paths.init();"))


def test_editor_clear_uses_the_original_qualified_direct_redraw():
    source = (ROOT / "src/console.cpp").read_text()
    body = _function(source, "void Console::editor_clear()")
    assert "WorldClimateWorldWin.GraphicWin::redraw();" in body
    assert "WorldClimateWorldWin.redraw();" not in body


def test_zoom_uses_the_original_qualified_direct_redraw():
    source = (ROOT / "src/console.cpp").read_text()
    body = _function(source, "void Console::zoom(")
    assert "WorldClimateWorldWin.GraphicWin::redraw();" in body
    assert "WorldClimateWorldWin.redraw();" not in body


def test_pinned_zoom_redraw_receiver_and_direct_target():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x5153CC, 0x5153D6)), 0x5153CC))
    assert [(i.mnemonic, i.op_str) for i in instructions] == [
        ("mov", "ecx, 0x8e9f60"), ("call", "0x5d5a70")]


def test_body_locations_now_match_out_of_line_definitions():
    path_rows = {row.address: row for row in read_file(ROOT / "src/path.cpp")}
    map_rows = {row.address: row for row in read_file(ROOT / "src/map.cpp")}
    assert path_rows[0x0059A2D0].body == "src/path.cpp"
    assert map_rows[0x00590E90].body == "src/map.cpp"
    assert "// body src/path.h" not in (ROOT / "src/path.cpp").read_text()
    assert "// body src/map.h" not in (ROOT / "src/map.cpp").read_text()


def test_extracted_cleanup_bodies_execute_null_and_free_order_controls(tmp_path: Path, host_compiler: str):
    header = (ROOT / "src/path.h").read_text()
    path_class = _function(header, "class Path {")
    path_class = path_class.replace("class Path {", "class Path { friend struct PathTestAccess;", 1)
    path_shutdown = _function((ROOT / "src/path.cpp").read_text(), "void Path::shutdown()")
    map_header = (ROOT / "src/map.h").read_text()
    map_helper = _function(map_header, "inline void __cdecl release_map_storage()")
    map_shutdown = _function((ROOT / "src/map.cpp").read_text(), "void __cdecl map_shutdown()")
    source = r"""
#include <stdint.h>
#include <stdlib.h>
static void* released[16]; static int release_count;
static void tracked_free(void* p) { released[release_count++] = p; }
#define MEASURED
#define BOOL int
#define __cdecl
#define free tracked_free
""" + path_class + ";\n" + path_shutdown + r"""
struct PathTestAccess {
  static void set(Path& p, int* map, int16_t* x, int16_t* y) {
    p.map_table_ = map; p.x_table_ = x; p.y_table_ = y;
  }
  static bool empty(const Path& p) {
    return p.map_table_ == 0 && p.x_table_ == 0 && p.y_table_ == 0;
  }
};
struct Map { int token; };
static Map* map_pointer; static uint8_t* abstract_pointer;
static Map*& map_tiles() { return map_pointer; }
static uint8_t*& MapAbstract() { return abstract_pointer; }
""" + map_helper + "\n" + map_shutdown + r"""
int main() {
  static int map_token, abstract_token; static int16_t x_token, y_token;
      static Path p;
  p.shutdown();
  if (release_count != 0 || !PathTestAccess::empty(p)) return 1;
  PathTestAccess::set(p, &map_token, &x_token, &y_token);
  p.shutdown();
  if (release_count != 3 || released[0] != &map_token ||
      released[1] != &x_token || released[2] != &y_token ||
      !PathTestAccess::empty(p)) return 2;
  PathTestAccess::set(p, 0, &x_token, 0);
  p.shutdown();
  if (release_count != 4 || released[3] != &x_token ||
      !PathTestAccess::empty(p)) return 3;

      static Path destructor_path;
      PathTestAccess::set(destructor_path, &map_token, &x_token, &y_token);
      destructor_path.~Path();
      if (release_count != 7 || released[4] != &map_token ||
          released[5] != &x_token || released[6] != &y_token ||
          !PathTestAccess::empty(destructor_path)) return 7;

  release_count = 0; map_pointer = 0; abstract_pointer = 0;
  release_map_storage(); map_shutdown();
  if (release_count != 0 || map_pointer || abstract_pointer) return 4;
  map_pointer = reinterpret_cast<Map*>(&map_token);
  abstract_pointer = reinterpret_cast<uint8_t*>(&abstract_token);
  release_map_storage();
  if (release_count != 2 || released[0] != &map_token ||
      released[1] != &abstract_token || map_pointer || abstract_pointer) return 5;
  abstract_pointer = reinterpret_cast<uint8_t*>(&abstract_token);
  map_shutdown();
  if (release_count != 3 || released[2] != &abstract_token || abstract_pointer) return 6;
  return 0;
}
"""
    cpp = tmp_path / "cleanup_actual.cpp"
    exe = tmp_path / "cleanup_actual"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [host_compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        text=True, capture_output=True, check=False, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], text=True, capture_output=True, check=False, timeout=30)
    assert result.returncode == 0, result.stdout + result.stderr
