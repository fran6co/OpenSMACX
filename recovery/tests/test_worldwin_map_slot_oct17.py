"""Actual-source control for WorldWin's embedded MapWin slot assignment.

This is a bounded host fixture, not a native binary or runtime/game proof.
It compiles the extracted WorldWin bodies against small dependency spies so
the slot and active fields remain distinguishable at the call boundaries.
"""

from hashlib import sha256
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.binding_source import parse as binding_rationales
from match_recovery.source import read_text


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
INIT_SPAN_SHA256 = "d1a929bb30ec7a73d4ec29780b827f9c21f0b349775cf6aa165e978d08927b9f"
INIT_SPAN = Span(0x004C49A0, 0x004C4A66)
INIT_STORE = bytes.fromhex("C7 86 A4 F2 01 00 01 00 00 00")
CLEAR_SLOT_STORE = bytes.fromhex("89 BE 78 DD 01 00")
CLEAR_ACTIVE_STORE = bytes.fromhex("89 8E 74 DD 01 00")
MAP_INIT_ACTIVE_STORE = bytes.fromhex("C7 86 74 DD 01 00 01 00 00 00")


def _function_body(source, signature):
    """Extract one definition while ignoring braces in comments and strings."""
    match = re.search(r"(?m)^\s*" + re.escape(signature) + r"\s*\{", source)
    assert match is not None, "missing actual source definition: " + signature
    opening = source.find("{", match.start())
    depth = 0
    state = "code"
    i = opening
    while i < len(source):
        char = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line_comment"
                i += 1
            elif char == "/" and nxt == "*":
                state = "block_comment"
                i += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[match.start():i + 1]
        elif state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and nxt == "/":
                state = "code"
                i += 1
        elif state == "string" or state == "character":
            if char == "\\":
                i += 1
            elif (state == "string" and char == '"') or (
                    state == "character" and char == "'"):
                state = "code"
        i += 1
    raise AssertionError("unterminated source definition: " + signature)


def _source_parts():
    source = (ROOT / "src/worldwin.cpp").read_text()
    header = (ROOT / "src/mapwin.h").read_text()
    return (
        _function_body(source, "void WorldWin::UNK1()"),
        _function_body(source, "void WorldWin::init(int reset_map)"),
        header,
    )


def _fixture(world_init_body, unk1_body):
    return r'''#include <stdint.h>
#include <assert.h>
#include <string.h>

struct RECT { int left, top, right, bottom; };
class MainInterface;

enum { EVENT_CLEAR = 1, EVENT_MAP_INIT = 2, EVENT_GRAPHIC_INIT = 3,
       EVENT_SHOW = 4, EVENT_WRONG_WIN_INIT = 5 };
int Events[16];
int EventCount = 0;
int GraphicInitResult = 0;

static void observe_map_entry();

struct MapWin {
  unsigned int field_1DD74_;
  unsigned int field_1DD78_;
  unsigned int active_at_clear;
  unsigned int slot_at_clear;
  unsigned int clear_argument;
  unsigned int active_at_init;
  unsigned int slot_at_init;
  int mode_at_init;
  int update_at_init;

  MapWin() : field_1DD74_(0), field_1DD78_(0), active_at_clear(0),
      slot_at_clear(0), clear_argument(0), active_at_init(0),
      slot_at_init(0), mode_at_init(-1), update_at_init(-1) {}

  void clear(int index) {
    observe_map_entry();
    active_at_clear = field_1DD74_;
    slot_at_clear = field_1DD78_;
    clear_argument = (unsigned int)index;
    Events[EventCount++] = EVENT_CLEAR;
    field_1DD78_ = (unsigned int)index;
    field_1DD74_ = (index == 0);
  }

  void init(int mode, int update) {
    observe_map_entry();
    active_at_init = field_1DD74_;
    slot_at_init = field_1DD78_;
    mode_at_init = mode;
    update_at_init = update;
    Events[EventCount++] = EVENT_MAP_INIT;
    // The actual MapWin init sets the active field itself; this spy only
    // models that declared call contract, not its full rendering behavior.
    field_1DD74_ = 1;
  }
};

class Win {
 public:
  int win_init_count;
  int show_count;
  RECT *rect_argument;
  int x_argument, y_argument, a_argument, b_argument;
  Win *parent_argument;
  int show_argument;

  Win() : win_init_count(0), show_count(0), rect_argument(0),
      x_argument(-1), y_argument(-1), a_argument(-1), b_argument(-1),
      parent_argument(0), show_argument(-1) {}

  void capture_init_arguments(RECT *rect, int x, int y, Win *parent, int a, int b) {
    rect_argument = rect;
    x_argument = x;
    y_argument = y;
    parent_argument = parent;
    a_argument = a;
    b_argument = b;
  }

  int init(RECT *rect, int x, int y, Win *parent, int a, int b) {
    ++win_init_count;
    capture_init_arguments(rect,x,y,parent,a,b);
    Events[EventCount++] = EVENT_WRONG_WIN_INIT;
    return 19;
  }

  virtual void show(int value) {
    ++show_count;
    show_argument = value;
    Events[EventCount++] = EVENT_SHOW;
  }
};

class GraphicWin : public Win {
 public:
  int graphic_init_count;
  GraphicWin() : graphic_init_count(0) {}
  int init(RECT *rect, int x, int y, Win *parent, int a, int b) {
    ++graphic_init_count;
    capture_init_arguments(rect,x,y,parent,a,b);
    Events[EventCount++] = EVENT_GRAPHIC_INIT;
    return GraphicInitResult;
  }
};

class MainInterface : public GraphicWin {
 public:
  RECT tutorial_bounds_;
  MainInterface() {
    tutorial_bounds_.left = 101;
    tutorial_bounds_.top = 202;
    tutorial_bounds_.right = 303;
    tutorial_bounds_.bottom = 404;
  }
};

unsigned int StatusFlags = 0;
int MapInputEnabled = 0;
RECT WorldMapBounds = { -10, -20, -30, -40 };
MainInterface MainInterfaceGlobal;

class WorldWin : public GraphicWin {
 public:
  unsigned int field_239BC_;
  unsigned int field_239C0_;
  unsigned int detail_map_selected_;
  unsigned int viewport_outline_enabled_;
  MapWin mapWin_;

  WorldWin() : field_239BC_(9), field_239C0_(9),
      detail_map_selected_(9), viewport_outline_enabled_(9) {}
  void UNK1();
  void init(int reset_map);
};

static WorldWin *ObservedWorld;
static unsigned int ExpectedStatus;
static RECT ExpectedBounds;
static void observe_map_entry() {
  assert(ObservedWorld->field_239BC_ == 0);
  assert(ObservedWorld->field_239C0_ == 0);
  assert(ObservedWorld->detail_map_selected_ == 0);
  assert(ObservedWorld->viewport_outline_enabled_ == 0);
  assert(MapInputEnabled == 1 && StatusFlags == ExpectedStatus);
  assert(WorldMapBounds.left == ExpectedBounds.left);
  assert(WorldMapBounds.top == ExpectedBounds.top);
  assert(WorldMapBounds.right == ExpectedBounds.right);
  assert(WorldMapBounds.bottom == ExpectedBounds.bottom);
}

@@UNK1_BODY@@
@@INIT_BODY@@

static int same_rect(const RECT &a, const RECT &b) {
  return a.left == b.left && a.top == b.top &&
         a.right == b.right && a.bottom == b.bottom;
}

static int run_case(int reset_map, unsigned int stale_slot,
                    unsigned int initial_active, unsigned int initial_flags,
                    int expect_copy, int graphic_result = 0) {
  WorldWin world;
  world.mapWin_.field_1DD74_ = initial_active;
  world.mapWin_.field_1DD78_ = stale_slot;
  StatusFlags = initial_flags;
  MapInputEnabled = 0;
  WorldMapBounds.left = -10; WorldMapBounds.top = -20;
  WorldMapBounds.right = -30; WorldMapBounds.bottom = -40;
  EventCount = 0;
  GraphicInitResult = graphic_result;
  ObservedWorld = &world;
  ExpectedStatus = initial_flags | 8U;
  ExpectedBounds = expect_copy ? MainInterfaceGlobal.tutorial_bounds_ : WorldMapBounds;

  world.init(reset_map);

  if (world.mapWin_.slot_at_init != 1 || world.mapWin_.mode_at_init != 1 ||
      world.mapWin_.update_at_init != 0 ||
      world.mapWin_.active_at_init != (reset_map == 0 ? initial_active : 0))
    return 10;
  if (world.field_239BC_ != 8 || world.field_239C0_ != 0 ||
      world.detail_map_selected_ != 0 ||
      world.viewport_outline_enabled_ != 0) return 11;
  if (MapInputEnabled != 1) return 12;
  if (StatusFlags != (initial_flags | 8U)) return 13;
  if (expect_copy) {
    if (!same_rect(WorldMapBounds, MainInterfaceGlobal.tutorial_bounds_))
      return 14;
  } else if (WorldMapBounds.left != -10 || WorldMapBounds.top != -20 ||
             WorldMapBounds.right != -30 || WorldMapBounds.bottom != -40) {
    return 15;
  }
  if (world.graphic_init_count != 1 || world.win_init_count != 0 ||
      world.rect_argument != &WorldMapBounds ||
      world.x_argument != 0 || world.y_argument != 0x20 ||
      world.parent_argument != static_cast<Win *>(&MainInterfaceGlobal) ||
      world.a_argument != 0 || world.b_argument != 0) return 16;
  if (world.show_count != 1 || world.show_argument != 0) return 17;

  if (reset_map != 0) {
    if (world.mapWin_.clear_argument != 1 ||
        world.mapWin_.active_at_clear != initial_active) return 18;
    if (EventCount != 4 || Events[0] != EVENT_CLEAR ||
        Events[1] != EVENT_MAP_INIT || Events[2] != EVENT_GRAPHIC_INIT ||
        Events[3] != EVENT_SHOW) return 19;
  } else {
    if (EventCount != 3 || Events[0] != EVENT_MAP_INIT ||
        Events[1] != EVENT_GRAPHIC_INIT || Events[2] != EVENT_SHOW) return 20;
  }
  if (world.mapWin_.field_1DD78_ != 1 || world.mapWin_.field_1DD74_ != 1)
    return 21;
  return 0;
}

int main() {
  const int resets[] = {0,1,-1};
  const unsigned int slots[] = {0,1,6};
  const unsigned int statuses[] = {0,8,0x40,0x48};
  for (int r=0;r<3;++r)
    for (int s=0;s<3;++s)
      for (unsigned int active=0;active<2;++active)
        for (int f=0;f<4;++f)
          for (int failed=0;failed<2;++failed)
            if (run_case(resets[r],slots[s],active,statuses[f],
                         (statuses[f]&8U)==0,failed?17:0) != 0) return 5;
  // Both zero and a nonzero stale slot are decisive with reset_map == 0;
  // clear(1) must not be allowed to mask the WorldWin pre-store contract.
  if (run_case(0, 0, 0, 8, 0) != 0) return 1;
  if (run_case(0, 6, 0, 8, 0) != 0) return 2;
  // reset_map uses truthiness, so both positive and negative values clear.
  if (run_case(1, 6, 0, 0, 1) != 0) return 3;
  if (run_case(-1, 0, 1, 8, 0) != 0) return 4;
  return 0;
}
'''.replace("@@UNK1_BODY@@", unk1_body).replace(
        "@@INIT_BODY@@", world_init_body)


def _compilers():
    return [(name, path) for name in ("g++", "clang++")
            if (path := shutil.which(name)) is not None]


def _compile_and_run(compiler, fixture, directory, stem):
    cpp = directory / (stem + ".cpp")
    exe = directory / stem
    cpp.write_text(fixture)
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


def test_pinned_worldwin_init_span_and_member_targets():
    path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not path.is_file():
        pytest.skip("pinned game unavailable")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    span = image.read(INIT_SPAN)
    assert len(span) == 198
    assert sha256(span).hexdigest() == INIT_SPAN_SHA256
    assert image.read(Span(0x004C4A11, 0x004C4A11 + len(INIT_STORE))) == INIT_STORE
    assert image.read(Span(0x0046287C, 0x0046287C + len(CLEAR_SLOT_STORE))) == CLEAR_SLOT_STORE
    assert image.read(Span(0x004628AF, 0x004628AF + len(CLEAR_ACTIVE_STORE))) == CLEAR_ACTIVE_STORE
    assert image.read(Span(0x00470AB1, 0x00470AB1 + len(MAP_INIT_ACTIVE_STORE))) == MAP_INIT_ACTIVE_STORE
    # The two RECT overloads have compatible call shapes but different owners
    # and lifecycle effects. Masked CALL bytes cannot distinguish them.
    call = image.read(Span(0x4C4A53, 0x4C4A58))
    assert call[0] == 0xE8
    assert 0x4C4A58 + int.from_bytes(call[1:], "little", signed=True) == 0x5D50A0

    _, _, header = _source_parts()
    world_header = (ROOT / "src/worldwin.h").read_text()
    assert "MapWin mapWin_;  // 0x152C" in world_header
    assert "class WorldWin : public GraphicWin" in world_header
    assert 0x152C + 0x1DD78 == 0x1F2A4
    assert 0x152C + 0x1DD74 == 0x1F2A0
    world_source = (ROOT / "src/worldwin.cpp").read_text()
    records = read_text(world_source, ROOT / "src/worldwin.cpp")
    entry, = (row for row in records if row.address == INIT_SPAN.start)
    rationale = binding_rationales(world_source, records)[entry.line]
    assert "GraphicWin" in rationale["reason"]
    assert "uint32_t field_1DD74_;  // 0x1DD74" in header
    assert "uint32_t field_1DD78_;  // 0x1DD78" in header
    map_source = (ROOT / "src/mapwin.cpp").read_text()
    clear_body = _function_body(map_source, "void MapWin::clear(int index)")
    map_init_body = _function_body(map_source, "void MapWin::init(int mode, int update)")
    assert "field_1DD78_ = index;" in clear_body
    assert "field_1DD74_ = (index == 0);" in clear_body
    assert "field_1DD74_ = 1;" in map_init_body
    assert "field_1DD78_" in map_init_body


def test_actual_worldwin_init_preserves_embedded_map_slot(tmp_path):
    compilers = _compilers()
    if not compilers:
        pytest.skip("host GCC/Clang unavailable")
    unk1, init, header = _source_parts()
    assert "mapWin_.field_1DD78_ = 1;" in init
    assert "GraphicWin::init(&WorldMapBounds, 0, 0x20," in init
    assert "uint32_t field_1DD74_;  // 0x1DD74" in header
    assert "uint32_t field_1DD78_;  // 0x1DD78" in header
    fixture = _fixture(init, unk1)
    for name, compiler in compilers:
        result = _compile_and_run(compiler, fixture, tmp_path, "worldwin_" + name)
        assert result.returncode == 0, name + ": " + result.stdout + result.stderr


def test_worldwin_slot_fixture_rejects_wrong_member_and_order_mutants(tmp_path):
    compilers = _compilers()
    if not compilers:
        pytest.skip("host GCC/Clang unavailable")
    unk1, init, _ = _source_parts()
    mutants = {
        "write_active_flag": (
            "mapWin_.field_1DD78_ = 1;", "mapWin_.field_1DD74_ = 1;"),
        "drop_slot_seed": ("mapWin_.field_1DD78_ = 1;", ""),
        "invert_reset_test": ("if (reset_map != 0)", "if (reset_map == 0)"),
        "wrong_clear_slot": ("mapWin_.clear(1);", "mapWin_.clear(0);"),
        "wrong_map_init_args": ("mapWin_.init(1, 0);", "mapWin_.init(0, 1);"),
        "wrong_base_route": ("GraphicWin::init(&WorldMapBounds", "Win::init(&WorldMapBounds"),
        "skip_status_copy": ("        UNK1();", "        ;"),
        "skip_show": ("    show(0);", "    ;"),
        "wrong_base_height": ("GraphicWin::init(&WorldMapBounds, 0, 0x20,", "GraphicWin::init(&WorldMapBounds, 0, 0x10,"),
        "invert_status_guard": ("if ((StatusFlags & 8) == 0)",
                                "if ((StatusFlags & 8) != 0)"),
        "clear_after_init": (
            "if (reset_map != 0) {\n        mapWin_.clear(1);\n    }\n"
            "    mapWin_.init(1, 0);",
            "mapWin_.init(1, 0);\n    if (reset_map != 0) {\n"
            "        mapWin_.clear(1);\n    }"),
    }
    for label, (old, new) in mutants.items():
        assert init.count(old) == 1, "mutant anchor changed: " + label
        mutated = init.replace(old, new, 1)
        for compiler_name, compiler in compilers:
            result = _compile_and_run(
                compiler, _fixture(mutated, unk1), tmp_path,
                "mutant_" + label + "_" + compiler_name)
            assert result.returncode != 0, (
                label + " unexpectedly survived under " + compiler_name)
