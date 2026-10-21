"""Focused host controls for two mapped MainInterface input bodies.

These tests extract the bodies from the source records selected by their
guarded original identities. The fake owners exercise source-level call order
and state effects only; they do not claim VC6 ABI/layout or original-runtime
proof.
"""

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import function_line, read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/maininterface.cpp"
MAPPINGS = {
    0x45CCB0: (
        "?on_iface_group_clicked@MainInterface@@QAEXHHH@Z",
        "?on_iface_group_clicked@MainInterface@@UAEXHHH@Z",
        ((0x45CCB0, 0x45CD75),),
        "void MainInterface::on_iface_group_clicked(int group, int, int selected)",
    ),
    0x45CF80: (
        "?on_iface_right_down@MainInterface@@QAEXHH@Z",
        "?on_iface_right_down@MainInterface@@UAEXHH@Z",
        ((0x45CF80, 0x45CFFD),),
        "void MainInterface::on_iface_right_down(int a1, int a2)",
    ),
}


def _mapped_definition(address):
    catalogue_name, emitted_symbol, spans, signature = MAPPINGS[address]
    rows = read_file(SOURCE)
    matches = [row for row in rows if row.address == address]
    assert len(matches) == 1
    row = matches[0]
    assert row.name == catalogue_name
    assert row.symbol == emitted_symbol
    assert row.image_spans == spans
    assert row.body == ""
    assert not row.extract_error
    assert row.region

    # Use the native source reader's mapped region and code-only definition
    # locator, then balance the actual braces to pass the extracted C++ body
    # into the fixture. The implementation is never copied into this test.
    definition_name = signature.split("(", 1)[0].split()[-1]
    start_line = function_line(row.region, definition_name)
    lines = row.region.splitlines(keepends=True)
    start = sum(len(line) for line in lines[:start_line - 1])
    brace = row.region.find("{", start)
    assert brace >= start
    depth = 0
    for pos in range(brace, len(row.region)):
        if row.region[pos] == "{":
            depth += 1
        elif row.region[pos] == "}":
            depth -= 1
            if depth == 0:
                return row.region[start:pos + 1]
    raise AssertionError("unterminated mapped body at 0x%08X" % address)


GROUP = _mapped_definition(0x45CCB0)
RIGHT_DOWN = _mapped_definition(0x45CF80)


HARNESS = r'''#include <cassert>
#include <cstring>
#include <vector>

struct RECT { int left, top, right, bottom; };

enum EventCode {
  SPOT_CHECK = 1,
  COORDINATE_CONVERSION = 2,
  STRING_LOOKUP = 3,
  BUBBLE_SET = 4,
  REDRAW = 5,
  EDITOR_PAINT = 6,
  EDIT_PANE = 7,
  WORLD_MAP = 8,
  DETAIL_MAP = 9
};

static std::vector<int> events;
static int IfaceIdToSlotStorage[30];
static int ActiveIfaceIdStorage;
static const int MaxTerrainNum = 20;
struct TerrainRule { unsigned int bit; };
static TerrainRule Terraforming[MaxTerrainNum];
static int MapEditorBrushId;

struct LabelOwner { char **strings_ptr; };
static LabelOwner Labels;
static char *expected_label;
static int last_string_key;

struct StringTableOwner {
  char *get(char *key) {
    events.push_back(STRING_LOOKUP);
    last_string_key = -1;
    assert(key == expected_label);
    return key;
  }
} StringTable;

struct Spot {
  int result;
  int position;
  int type;
  int write_position;
  int write_type;
  int last_x, last_y;
  int calls;
  Spot() : result(-1), position(0), type(0), write_position(0),
           write_type(0), last_x(0), last_y(0), calls(0) {}
  int check(int x, int y, int *spot_pos, int *spot_type) {
    ++calls;
    events.push_back(SPOT_CHECK);
    last_x = x; last_y = y;
    // The mapped body passes its local address as spot_pos and a null
    // spot_type. A miss returns -1 without writing through that address;
    // the caller's short-circuit must avoid reading the uninitialized local.
    assert(spot_pos != 0);
    assert(spot_type == 0);
    if (result != -1) {
      if (write_position) *spot_pos = position;
      if (write_type && spot_type != 0) *spot_type = type;
    }
    return result;
  }
};

static const void *last_win_receiver;
static const void *expected_win_receiver;
static int last_screen_x, last_screen_y;
static int last_bubble_x, last_bubble_y;
static char *last_bubble_text;

struct Win {
  void client_to_screen(int *x, int *y) {
    events.push_back(COORDINATE_CONVERSION);
    last_win_receiver = this;
    assert(this == expected_win_receiver);
    *x += 100;
    *y += 200;
    last_screen_x = *x;
    last_screen_y = *y;
  }
  void set_bubble_text(char *text, RECT *rect) {
    events.push_back(BUBBLE_SET);
    last_win_receiver = this;
    assert(this == expected_win_receiver);
    last_bubble_text = text;
    last_bubble_x = rect->left;
    last_bubble_y = rect->top;
    assert(rect->right == rect->left && rect->bottom == rect->top);
  }
};

static const void *last_redraw_receiver;
static const void *last_pane_receiver;
static int redraw_count;
static int pane_count;
static int paint_count;
static int last_paint;
static int world_map_count;
static int detail_map_count;

struct ConsoleOwner {
  void editor_paints(int paint) {
    events.push_back(EDITOR_PAINT);
    ++paint_count;
    last_paint = paint;
  }
} ConsoleGlobal;

struct WorldClimateOwner {
  void set_world_map() { events.push_back(WORLD_MAP); ++world_map_count; }
  void set_detail_map() { events.push_back(DETAIL_MAP); ++detail_map_count; }
} WorldClimateWorldWin;

struct MainInterface : Win {
  Spot spot_;
  void on_iface_group_clicked(int group, int unused, int selected);
  void on_iface_right_down(int x, int y);
  void on_redraw() {
    events.push_back(REDRAW);
    ++redraw_count;
    last_redraw_receiver = this;
  }
  void draw_edit_pane() {
    events.push_back(EDIT_PANE);
    ++pane_count;
    last_pane_receiver = this;
  }
};
static MainInterface MainInterfaceGlobal;

__GROUP__
__RIGHT_DOWN__

static void reset_observers() {
  events.clear();
  last_win_receiver = 0;
  expected_win_receiver = 0;
  last_screen_x = last_screen_y = 0;
  last_bubble_x = last_bubble_y = 0;
  last_bubble_text = 0;
  last_redraw_receiver = last_pane_receiver = 0;
  redraw_count = pane_count = paint_count = 0;
  last_paint = -999;
  world_map_count = detail_map_count = 0;
  last_string_key = -1;
}

static void clear_group_guards() {
  memset(IfaceIdToSlotStorage, 0, sizeof(IfaceIdToSlotStorage));
  ActiveIfaceIdStorage = 0;
}

int main() {
  MainInterface receiver;
  clear_group_guards();
  reset_observers();

  // Group 0 redraws on the dispatched receiver, not the global owner.
  receiver.on_iface_group_clicked(0, 777, 1000);
  assert(events.size() == 1 && events[0] == REDRAW);
  assert(redraw_count == 1 && last_redraw_receiver == &receiver);
  assert(pane_count == 0 && paint_count == 0);

  // A nonzero active slot suppresses every dispatch; negative selections do
  // likewise, including group 3's global brush update and redraw.
  reset_observers();
  ActiveIfaceIdStorage = 2;
  IfaceIdToSlotStorage[2] = 1;
  MapEditorBrushId = 314;
  receiver.on_iface_group_clicked(3, 0, 1036);
  assert(events.empty() && MapEditorBrushId == 314 && pane_count == 0);
  clear_group_guards();
  receiver.on_iface_group_clicked(3, 0, -1);
  assert(events.empty() && MapEditorBrushId == 314 && pane_count == 0);

  // Group 2's ordinary action uses value-11. The higher actions traverse all
  // 20 terrain rules and compare each index against the evolving tool value.
  reset_observers();
  receiver.on_iface_group_clicked(2, 0, 1011);
  assert(events.size() == 1 && events[0] == EDITOR_PAINT);
  assert(paint_count == 1 && last_paint == 0);
  reset_observers();
  for (int i = 0; i < MaxTerrainNum; ++i) Terraforming[i].bit = 0;
  receiver.on_iface_group_clicked(2, 0, 1017);
  assert(events.size() == 1 && events[0] == EDITOR_PAINT);
  assert(paint_count == 1 && last_paint == 26);
  reset_observers();
  for (int i = 0; i < MaxTerrainNum; ++i) Terraforming[i].bit = 0;
  Terraforming[3].bit = 1;
  Terraforming[11].bit = 1;
  receiver.on_iface_group_clicked(2, 0, 1017);
  assert(events.size() == 1 && events[0] == EDITOR_PAINT);
  assert(last_paint == 9);

  // Group 3 stores the brush globally and invokes the global MainInterface
  // pane, even when the method receiver is a distinct object.
  reset_observers();
  MapEditorBrushId = -1;
  receiver.on_iface_group_clicked(3, 0, 1039);
  assert(MapEditorBrushId == 3);
  assert(events.size() == 1 && events[0] == EDIT_PANE);
  assert(pane_count == 1 && last_pane_receiver == &MainInterfaceGlobal);
  assert(last_pane_receiver != &receiver);

  // Other group ids choose between the two world map targets by value==6.
  reset_observers();
  receiver.on_iface_group_clicked(1, 0, 1006);
  assert(events.size() == 1 && events[0] == WORLD_MAP);
  assert(world_map_count == 1 && detail_map_count == 0);
  reset_observers();
  receiver.on_iface_group_clicked(1, 0, 1005);
  assert(events.size() == 1 && events[0] == DETAIL_MAP);
  assert(world_map_count == 0 && detail_map_count == 1);

  // A miss leaves the output unwritten; result==-1 must short-circuit before
  // reading it. This intentionally does not initialize Spot's output local.
  reset_observers();
  receiver.spot_.result = -1;
  receiver.spot_.write_position = 0;
  expected_win_receiver = static_cast<Win *>(&receiver);
  receiver.on_iface_right_down(7, 11);
  assert(events.size() == 1 && events[0] == SPOT_CHECK);
  assert(receiver.spot_.calls == 1 && receiver.spot_.last_x == 7
         && receiver.spot_.last_y == 11);
  assert(last_win_receiver == 0 && last_bubble_text == 0);

  // A successful hit with another Spot position does nothing after the hit
  // test. The type output is null in this call; position 5 is the trigger.
  reset_observers();
  receiver.spot_.result = 2;
  receiver.spot_.position = 4;
  receiver.spot_.write_position = 1;
  receiver.spot_.write_type = 0;
  receiver.on_iface_right_down(13, 17);
  assert(events.size() == 1 && events[0] == SPOT_CHECK);
  assert(last_win_receiver == 0 && last_bubble_text == 0);

  // Position 5 follows Spot -> coordinate conversion -> exact label-table
  // lookup -> bubble call, with the same point RECT and inherited Win receiver.
  reset_observers();
  char labels[0x314];
  char *label_slots[0x314];
  for (int i = 0; i < 0x314; ++i) label_slots[i] = labels + i;
  Labels.strings_ptr = label_slots;
  expected_label = label_slots[0x313];
  receiver.spot_.result = 2;
  receiver.spot_.position = 5;
  receiver.spot_.write_position = 1;
  receiver.spot_.write_type = 0;
  expected_win_receiver = static_cast<Win *>(&receiver);
  receiver.on_iface_right_down(-9, 21);
  assert(events.size() == 4);
  assert(events[0] == SPOT_CHECK && events[1] == COORDINATE_CONVERSION
         && events[2] == STRING_LOOKUP && events[3] == BUBBLE_SET);
  assert(last_screen_x == 91 && last_screen_y == 221);
  assert(last_bubble_x == 91 && last_bubble_y == 221);
  assert(last_bubble_text == expected_label);
  assert(last_win_receiver == static_cast<Win *>(&receiver));
}
'''


def compile_and_run(compiler, group=GROUP, right_down=RIGHT_DOWN,
                    expected_success=True):
    source = HARNESS.replace("__GROUP__", group).replace("__RIGHT_DOWN__", right_down)
    with tempfile.TemporaryDirectory(prefix="maininterface-call-order-") as temp:
        path = Path(temp) / "control.cpp"
        path.write_text(source)
        exe = Path(temp) / "control"
        subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", str(path), "-o", str(exe)],
            check=True,
        )
        result = subprocess.run(
            [str(exe)], timeout=5, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        )
        if expected_success and result.returncode != 0:
            raise AssertionError(
                compiler + " actual-source control failed: "
                + result.stderr.decode(errors="replace")
            )
        if not expected_success and result.returncode == 0:
            raise AssertionError(compiler + " failed to reject a damage mutant")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_mapped_bodies_and_compile_success_damage(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host C++ compiler unavailable")

    compile_and_run(compiler)
    mutants = [
        (GROUP.replace("if (group == 0)", "if (group == 1)", 1), RIGHT_DOWN),
        (GROUP.replace("index <= tool", "index < tool", 1), RIGHT_DOWN),
        (GROUP.replace("MainInterfaceGlobal.draw_edit_pane()",
                       "draw_edit_pane()", 1), RIGHT_DOWN),
        (GROUP, RIGHT_DOWN.replace("0x313", "0x312", 1)),
        (GROUP, RIGHT_DOWN.replace("Win::client_to_screen(&a1, &a2);", "", 1)),
    ]
    for group, right_down in mutants:
        assert (group, right_down) != (GROUP, RIGHT_DOWN)
        compile_and_run(compiler, group, right_down, expected_success=False)
