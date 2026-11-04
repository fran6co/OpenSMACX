"""Actual-source host controls for TutWin::tour1 (stand-ins are not ABI proof)."""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import shutil
import subprocess

import pytest


# When moved to recovery/tests, parents[2] is the worker root.
ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/tutwin.cpp"
CALLOUT_SOURCE = SOURCE
PINNED_IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")
))
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
HOST_COMPILERS = tuple(
    path for name in ("g++", "clang++") if (path := shutil.which(name)) is not None
)


def definition(path: Path, signature: str) -> str:
    source = path.read_text()
    start = source.index(signature)
    end = source.index("\n}", start) + 2
    return source[start:end]


def compile_run(tmp_path: Path, compiler: str, code: str, name: str) -> subprocess.CompletedProcess:
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(code)
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        check=True,
        capture_output=True,
        text=True,
        timeout=30,
    )
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


HARNESS = r'''#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <vector>

struct RECT { int left, top, right, bottom; };
struct GraphicWin { int tag; };
struct Prefix { int tag; };
struct Sprite {};
extern int callCount;
extern std::vector<int> screen_calls;
// Deliberately nonempty prefix plus virtual base: the delivered owner must be
// the actual adjusted GraphicWin subobject, not a zero-offset stand-in.
struct MapWin : Prefix, virtual GraphicWin {
  int id;
  MapWin(int p, int g, int i) : GraphicWin{g}, Prefix{p}, id(i) {}
};
MapWin primaryMap(11, 21, 1), secondMap(12, 22, 2), thirdMap(13, 23, 3);
MapWin *MapWinTable[1] = {&primaryMap};

struct Button { bool shown = false; bool is_visible() { return shown; } };
struct Window {
  int id = 1;
  bool visible = true;
  bool zoom_sets_visible = true;
  int zoom_count = 0, zoom_base = -1, visible_reads = 0;
  RECT rect1_, rect2_, rect5_, rect6_, rect7_, rect8_, rect9_, rect10_, rect11_, rect12_, rect13_,
       rect14_, rect15_, rect16_, rect18_, rect19_, info_title_rect_, population_rect_,
       flash_rect_a_, flash_rect_b_, queue_help_rect_, queue_content_rect_, garrison_rect_,
       commerce_report_rect_, nutrient_content_rect_, supported_content_rect_, previous_button_rect_, next_button_rect_,
       expenses_help_rects_[6];
  Button flatButtons_[18];
  std::vector<int> ops;
  void record(int code) { ops.push_back(callCount * 1000 + code); }
  bool is_visible() { ++visible_reads; return visible; }
  void zoom(int base, int mode) {
    if (mode != 0) fail("zoom-mode");
    ++zoom_count; zoom_base = base; record(900 + base);
    if (zoom_sets_visible) visible = true;
  }
  void focus() { record(100); }
  void client_to_screen(int *x, int *y) {
    screen_calls.push_back(id);
    *x += id == 1 ? 10001 : 20002;
    *y += id == 1 ? 1001 : 2002;
  }
  void draw_expenses(int value) { record(200 + value); }
  void draw_farm(int value) { record(300 + value); }
  void draw_energy_alloc(int value) { record(400 + value); }
  static void fail(const char *why) { std::fprintf(stderr, "witness:%s\n", why); std::exit(71); }
};
Window TutWinBaseWindow, MainInterfaceGlobal;
struct BaseRecord { int faction_id_current; } Bases[4];
struct GameData { struct Fields { int base_current_count_; } fields_; } GameDataStateStorage;
struct ConsoleState { int faction_id_; } ConsoleGlobal;
struct BaseName { char name_string[40]; } baseCurrent;
BaseName *BaseCurrent = &baseCurrent;
struct RuleSet { int nutrient_req_citizen; } Rules;

struct Expected { const char *label; int rect_id; int route; } expected[36];
int expectedCount, callCount, stopAt;
int parseSaysCount, parseNumCount;
std::vector<int> ownerSequence;
std::vector<int> screen_calls;
int last_x, last_y, lastScreenCount, mutateOwner = 1;

RECT rect_for(int id) {
  int left = -1001 + id * 79;
  int top = -901 + id * 31;
  return RECT{left, top, left + 21, top + 29};
}
void set_rect(RECT &rect, int id) { rect = rect_for(id); }
void expect(const char *label, int id, int route) {
  expected[expectedCount++] = Expected{label, id, route};
}
void build_expected(bool energy) {
  expectedCount = 0;
  expect("TOUR1GOVERNOR", 8, 1); expect("TOUR1EXPLORE", 14, 1);
  expect("TOUR1DISCOVER", 15, 1); expect("TOUR1BUILD", 18, 1);
  expect("TOUR1CONQUER", 19, 1); expect("TOUR1GOVERNOR2", 8, 1);
  expect("TOUR1ADVANCED", 16, 1); expect("TOUR1CAPTION", 101, 2);
  expect("TOUR1ECO", 5, 1); expect("TOUR1POP", 102, 1);
  expect("TOUR1POP2", 102, 0); expect("TOUR1STAPLE", 12, 2);
  expect("TOUR1PRODUCTION", 104, 2); expect("TOUR1QUEUE", 105, 2);
  expect("TOUR1CHANGE", 103, 2); expect("TOUR1HURRY", 106, 2);
  expect("TOUR1GARRISON", 107, 2); expect("TOUR1SUPPORTED", 120, 2);
  expect("TOUR1FACILITIES", 2, 1); expect("TOUR1COMMERCE", 108, 1);
  expect("TOUR1NUTRIENTS", 109, 1); expect("TOUR1AREA", 1, 1);
  expect("TOUR1NUT", 201, 1); expect("TOUR1MIN", 202, 1);
  expect("TOUR1ENERGY", 200, 1);
  if (energy) expect("TOUR1ALLOC2", 13, 1);
  else expect("TOUR1ALLOC", 205, 1);
  expect("TOUR1ECON", 205, 1); expect("TOUR1PSYCH", 203, 1);
  expect("TOUR1LABS", 204, 1); expect("TOUR1SUPPORTBUTTON", 9, 1);
  expect("TOUR1PSYCHBUTTON", 11, 1); expect("TOUR1RESOURCEBUTTON", 10, 1);
  expect("TOUR1PREV", 110, 2); expect("TOUR1NEXT", 111, 2);
  expect("TOUR1RENAME", 7, 1); expect("TOUR1OK", 6, 1);
}
void initialize(bool energy) {
  TutWinBaseWindow = Window{}; MainInterfaceGlobal = Window{};
  TutWinBaseWindow.id = 1; MainInterfaceGlobal.id = 2;
  TutWinBaseWindow.visible = true; TutWinBaseWindow.flatButtons_[17].shown = energy;
  int rect_id = 1;
  RECT *all[] = {&TutWinBaseWindow.rect1_, &TutWinBaseWindow.rect2_, &TutWinBaseWindow.rect5_,
    &TutWinBaseWindow.rect6_, &TutWinBaseWindow.rect7_, &TutWinBaseWindow.rect8_, &TutWinBaseWindow.rect12_,
    &TutWinBaseWindow.rect9_, &TutWinBaseWindow.rect10_, &TutWinBaseWindow.rect11_,
    &TutWinBaseWindow.rect13_, &TutWinBaseWindow.rect14_, &TutWinBaseWindow.rect15_,
    &TutWinBaseWindow.rect16_, &TutWinBaseWindow.rect18_, &TutWinBaseWindow.rect19_,
    &TutWinBaseWindow.info_title_rect_, &TutWinBaseWindow.population_rect_,
    &TutWinBaseWindow.flash_rect_a_, &TutWinBaseWindow.flash_rect_b_,
    &TutWinBaseWindow.queue_help_rect_, &TutWinBaseWindow.queue_content_rect_,
    &TutWinBaseWindow.garrison_rect_, &TutWinBaseWindow.commerce_report_rect_,
    &TutWinBaseWindow.nutrient_content_rect_, &TutWinBaseWindow.previous_button_rect_,
    &TutWinBaseWindow.next_button_rect_};
  for (RECT *r : all) set_rect(*r, rect_id++);
  for (int i = 0; i != 6; ++i) set_rect(TutWinBaseWindow.expenses_help_rects_[i], 200 + i);
  // Give source fields distinct tagged rectangles so each call's choice is observable.
  set_rect(TutWinBaseWindow.rect1_, 1); set_rect(TutWinBaseWindow.rect2_, 2);
  set_rect(TutWinBaseWindow.rect5_, 5); set_rect(TutWinBaseWindow.rect6_, 6);
  set_rect(TutWinBaseWindow.rect7_, 7); set_rect(TutWinBaseWindow.rect8_, 8);
  set_rect(TutWinBaseWindow.rect9_, 9); set_rect(TutWinBaseWindow.rect10_, 10);
  set_rect(TutWinBaseWindow.rect11_, 11); set_rect(TutWinBaseWindow.rect13_, 13);
  set_rect(TutWinBaseWindow.rect12_, 12);
  set_rect(TutWinBaseWindow.rect14_, 14); set_rect(TutWinBaseWindow.rect15_, 15);
  set_rect(TutWinBaseWindow.rect16_, 16); set_rect(TutWinBaseWindow.rect18_, 18);
  set_rect(TutWinBaseWindow.rect19_, 19); set_rect(TutWinBaseWindow.info_title_rect_, 101);
  set_rect(TutWinBaseWindow.population_rect_, 102); set_rect(TutWinBaseWindow.flash_rect_a_, 103);
  set_rect(TutWinBaseWindow.queue_help_rect_, 104); set_rect(TutWinBaseWindow.queue_content_rect_, 105);
  set_rect(TutWinBaseWindow.flash_rect_b_, 106); set_rect(TutWinBaseWindow.garrison_rect_, 107);
  set_rect(TutWinBaseWindow.commerce_report_rect_, 108); set_rect(TutWinBaseWindow.nutrient_content_rect_, 109);
  set_rect(TutWinBaseWindow.supported_content_rect_, 120);
  set_rect(TutWinBaseWindow.previous_button_rect_, 110); set_rect(TutWinBaseWindow.next_button_rect_, 111);
  build_expected(energy);
  callCount = parseSaysCount = parseNumCount = 0;
  screen_calls.clear(); lastScreenCount = 0; mutateOwner = 1;
  GameDataStateStorage.fields_.base_current_count_ = 3;
  ConsoleGlobal.faction_id_ = 7; Bases[0].faction_id_current = 2;
  Bases[1].faction_id_current = 7; Bases[2].faction_id_current = 7;
  Rules.nutrient_req_citizen = 14;
  std::strcpy(baseCurrent.name_string, "BASE-NAME");
  ownerSequence.clear(); ownerSequence.push_back(0); ownerSequence.push_back(-1);
  ownerSequence.push_back(2); ownerSequence.push_back(0);
  MapWinTable[0] = &primaryMap;
}
void parse_says(int slot, char *name, int x, int y) {
  if (slot != 0 || name != baseCurrent.name_string || x != -1 || y != -1)
    Window::fail("parse-says-arguments");
  ++parseSaysCount; TutWinBaseWindow.record(101);
}
void parse_num(int slot, int number) {
  if (slot != 0 || number != Rules.nutrient_req_citizen) Window::fail("parse-num-arguments");
  ++parseNumCount; TutWinBaseWindow.record(500);
}
struct TutWin {
  void tour1(); void iface_rect(RECT *, int *, int *); void base_rect(RECT *, int *, int *);
  int tut_win(GraphicWin *, const char *, int, int, Sprite *, int, int, int);
  int do_base(RECT *, const char *, int); int do_iface(RECT *, const char *, int);
};
int TutWin::tut_win(GraphicWin *owner, const char *text, int x, int y,
                    Sprite *unused, int flags, int arrow_x, int arrow_y) {
  (void)unused;
  if (callCount >= expectedCount) Window::fail("extra-callout");
  const Expected &e = expected[callCount];
  if (std::strcmp(text, e.label) != 0) Window::fail("label-order");
  MapWin *live = MapWinTable[0];
  GraphicWin *want = live ? static_cast<GraphicWin *>(live) : 0;
  if (owner != want) Window::fail("owner-reloaded-and-virtual-adjusted-per-call");
  if (flags != 2 || arrow_x != -1 || arrow_y != -1) Window::fail("callout-flags");
  RECT r = rect_for(e.rect_id);
  int want_x = r.left + (r.right - r.left) / 2;
  int want_y = r.top + (r.bottom - r.top) / 2;
  if (e.route == 1) { want_x += 10001; want_y += 1001; }
  else if (e.route == 2) { want_x += 20002; want_y += 2002; }
  else { want_x = last_x; want_y = last_y; }
  if (x != want_x || y != want_y) {
    std::fprintf(stderr, "coordinate %s got %d,%d wanted %d,%d route %d rect %d\\n",
                 text, x, y, want_x, want_y, e.route, e.rect_id);
    Window::fail("rectangle-route-coordinate");
  }
  if (e.route != 0) {
    int seen = screen_calls.back();
    int want_window = e.route == 1 ? 1 : 2;
    if (seen != want_window || (int)screen_calls.size() != lastScreenCount + 1)
      Window::fail("fixed-helper-window-route");
  } else if (callCount == 0 ||
             (x != last_x || y != last_y) || (int)screen_calls.size() != lastScreenCount)
    Window::fail("POP2-coordinate-reuse");
  last_x = x; last_y = y;
  lastScreenCount = (int)screen_calls.size();
  ++callCount;
  if (mutateOwner) {
    int next = ownerSequence[callCount % ownerSequence.size()];
    MapWinTable[0] = next == 0 ? &primaryMap : next == 1 ? &secondMap :
                     next == 2 ? &thirdMap : nullptr;
  }
  return callCount - 1 != stopAt;
}
BODY_TOUR
BODY_IFACE
BODY_BASE
BODY_DO_IFACE
BODY_DO_BASE
'''


def make_unit(tour: str, iface: str, base: str, do_iface: str, do_base: str,
              damage: str = "") -> str:
    body = tour
    if damage == "wrong_first_helper_window":
        old = "iface_rect(&TutWinBaseWindow.info_title_rect_, &x, &y);"
        assert old in body
        body = body.replace(old, "base_rect(&TutWinBaseWindow.info_title_rect_, &x, &y);", 1)
    elif damage == "wrong_first_rectangle":
        old = "rect = TutWinBaseWindow.rect14_;"
        assert old in body
        body = body.replace(old, "rect = TutWinBaseWindow.rect15_;", 1)
    elif damage == "missing_pop2":
        old = 'if (tut_win(MapWinTable[0], "TOUR1POP2", x, y, 0, 2, -1, -1) == 0) return;'
        if old not in body:
            old = next(line.strip() for line in body.splitlines()
                       if '"TOUR1POP2"' in line and '== 0) return;' in line)
        assert old in body
        body = body.replace(old, "(void)0;", 1)
    elif damage == "skip_cancel_return":
        old = 'if (tut_win(MapWinTable[0], "TOUR1GOVERNOR", x, y, 0, 2, -1, -1) == 0) return;'
        if old not in body:
            old = next(line.strip() for line in body.splitlines()
                       if '"TOUR1GOVERNOR"' in line and '== 0) return;' in line)
        assert old in body
        call = old[4:].rsplit(" == 0) return;", 1)[0] + ";"
        body = body.replace(old, call, 1)
    elif damage == "wrong_branch":
        old = "if (TutWinBaseWindow.flatButtons_[17].is_visible()) {"
        assert old in body
        body = body.replace(old, "if (!TutWinBaseWindow.flatButtons_[17].is_visible()) {", 1)
    elif damage == "null_owner":
        old = 'tut_win(MapWinTable[0], "TOUR1GOVERNOR",'
        assert old in body
        body = body.replace(old, 'tut_win(static_cast<GraphicWin *>(0), "TOUR1GOVERNOR",', 1)
    elif damage == "stale_owner_snapshot":
        # Keep this damage control behavior-specific: force every call to use
        # the entry-time owner snapshot, which is wrong when a callout callback
        # changes MapWinTable[0].
        assert "MapWinTable[0]" in body
        body = body.replace("int x;\n    int y;", "MapWin *const snapshot = MapWinTable[0];\n    int x;\n    int y;", 1)
        body = body.replace("MapWinTable[0]", "snapshot")
        body = body.replace("MapWin *const snapshot = snapshot;", "MapWin *const snapshot = MapWinTable[0];", 1)
    harness = (HARNESS.replace("BODY_TOUR", body)
            .replace("BODY_IFACE", iface).replace("BODY_BASE", base)
            .replace("BODY_DO_IFACE", do_iface).replace("BODY_DO_BASE", do_base)
            + r'''
int main() {
  for (int energy = 0; energy != 2; ++energy) {
    initialize(energy != 0);
    TutWin w;
    stopAt = -1;
    w.tour1();
    if (callCount != expectedCount || parseSaysCount != 1 || parseNumCount != 1)
      Window::fail("complete-callout-sequence");
    const std::vector<int> expected_ops = {
      100, 101, 304, 22202, 22500, 25402, 29201, 29303, 30302, 31304, 36301
    };
    if (TutWinBaseWindow.ops != expected_ops) {
      std::fprintf(stderr, "ops:");
      for (int value : TutWinBaseWindow.ops) std::fprintf(stderr, " %d", value);
      std::fprintf(stderr, "\\n");
      Window::fail("focus-dialog-draw-and-lifecycle-order");
    }
    for (int cut = 0; cut < expectedCount; ++cut) {
      initialize(energy != 0); stopAt = cut; TutWin w2; w2.tour1();
      if (callCount != cut + 1) Window::fail("cancellation-position");
      if (parseNumCount != (cut >= 22 ? 1 : 0)) Window::fail("parse-num-cancellation-order");
      std::vector<int> expected_prefix = {100, 101, 304};
      if (cut >= 22) { expected_prefix.push_back(22202); expected_prefix.push_back(22500); }
      if (cut >= 25) expected_prefix.push_back(25402);
      if (cut >= 29) { expected_prefix.push_back(29201); expected_prefix.push_back(29303); }
      if (cut >= 30) expected_prefix.push_back(30302);
      if (cut >= 31) expected_prefix.push_back(31304);
      if (TutWinBaseWindow.ops != expected_prefix)
        Window::fail("cancellation-side-effect-boundary");
    }
  }
  // Not-visible search: first matching base only. A failed zoom then exits
  // before focus/dialog/callouts; a successful zoom continues normally.
  initialize(false); TutWinBaseWindow.visible = false; TutWinBaseWindow.zoom_sets_visible = false;
  MapWinTable[0] = &primaryMap; stopAt = -1; TutWin search; search.tour1();
  if (TutWinBaseWindow.zoom_count != 1 || TutWinBaseWindow.zoom_base != 1 ||
      callCount != 0 || TutWinBaseWindow.ops.size() != 1 || TutWinBaseWindow.ops[0] != 901)
    Window::fail("zoom-search-and-invisible-return");
  initialize(false); TutWinBaseWindow.visible = false; TutWinBaseWindow.zoom_sets_visible = true;
  MapWinTable[0] = &primaryMap; stopAt = 0; TutWin visible; visible.tour1();
  if (TutWinBaseWindow.zoom_count != 1 || TutWinBaseWindow.zoom_base != 1 ||
      callCount != 1 || TutWinBaseWindow.ops[0] != 901 || TutWinBaseWindow.ops[1] != 100)
    Window::fail("zoom-search-before-normal-sequence");
  return 0;
}
''')
    return harness


@pytest.mark.parametrize("compiler", HOST_COMPILERS)
def test_tour1_full_actual_body_helpers_routes_zoom_and_all_cancellations(tmp_path, compiler):
    assert "class MapWin : public virtual GraphicWin" in (ROOT / "src/mapwin.h").read_text()
    result = compile_run(
        tmp_path, compiler,
        make_unit(definition(SOURCE, "void TutWin::tour1() {"),
                  definition(CALLOUT_SOURCE, "void TutWin::iface_rect(RECT *rect, int *x, int *y) {"),
                  definition(CALLOUT_SOURCE, "void TutWin::base_rect(RECT *rect, int *x, int *y) {"),
                  definition(CALLOUT_SOURCE, "int TutWin::do_iface(RECT *rect, const char *text, int flag) {"),
                  definition(CALLOUT_SOURCE, "int TutWin::do_base(RECT *rect, const char *text, int flag) {")),
        "tour1_actual",
    )
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", HOST_COMPILERS)
@pytest.mark.parametrize("damage", [
    "wrong_first_helper_window", "wrong_first_rectangle", "missing_pop2",
    "skip_cancel_return", "wrong_branch", "null_owner", "stale_owner_snapshot",
])
def test_compiling_tour1_damage_controls_fail_named_behavior(tmp_path, compiler, damage):
    result = compile_run(
        tmp_path, compiler,
        make_unit(definition(SOURCE, "void TutWin::tour1() {"),
                  definition(CALLOUT_SOURCE, "void TutWin::iface_rect(RECT *rect, int *x, int *y) {"),
                  definition(CALLOUT_SOURCE, "void TutWin::base_rect(RECT *rect, int *x, int *y) {"),
                  definition(CALLOUT_SOURCE, "int TutWin::do_iface(RECT *rect, const char *text, int flag) {"),
                  definition(CALLOUT_SOURCE, "int TutWin::do_base(RECT *rect, const char *text, int flag) {"),
                  damage),
        f"tour1_{damage}",
    )
    assert result.returncode == 0x47, f"{damage} did not reach its expected behavior witness: {result.stderr}"
    expected_witness = {
        "wrong_first_helper_window": "rectangle-route-coordinate",
        "wrong_first_rectangle": "rectangle-route-coordinate",
        "missing_pop2": "label-order",
        "skip_cancel_return": "cancellation-position",
        "wrong_branch": "label-order",
        "null_owner": "owner-reloaded-and-virtual-adjusted-per-call",
        "stale_owner_snapshot": "owner-reloaded-and-virtual-adjusted-per-call",
    }[damage]
    assert f"witness:{expected_witness}" in result.stderr


def test_pinned_original_proves_per_call_null_checked_virtual_owner_and_labels():
    if not PINNED_IMAGE.is_file():
        pytest.skip("pinned original image is unavailable")
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    raw = PINNED_IMAGE.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    from match_recovery.literal_source import read_literals
    ok_literal, = [binding for binding in read_literals(ROOT, PIN)
                   if binding.unit == "src/tutwin.cpp" and binding.origin.address == 0x0068790C]
    assert ok_literal.span == Span(0x0068790C, 0x00687914)
    assert ok_literal.symbol == "??_C@_07PKHB@TOUR1OK?$AA@"
    assert ok_literal.evidence.address == 0x004BB7A0
    assert image.read(ok_literal.span) == b"TOUR1OK\0"
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = list(decoder.disasm(image.read(Span(0x004BB7A0, 0x004BC591)), 0x004BB7A0))
    calls = [i for i, ins in enumerate(instructions)
             if ins.mnemonic == "call" and ins.op_str == "0x4bdfe0"]
    assert len(calls) == 33
    def pushed_label(call_index):
        for ins in reversed(instructions[max(0, call_index - 18):call_index]):
            if ins.mnemonic != "push" or not ins.op_str.startswith("0x"):
                continue
            address = int(ins.op_str, 16)
            try:
                value = image.read(Span(address, address + 40)).split(b"\0", 1)[0]
            except ValueError:
                continue
            if value.startswith(b"TOUR1"):
                return value.decode("ascii")
        return None
    direct_labels = [pushed_label(i) for i in calls]
    assert direct_labels == [
        "TOUR1GOVERNOR", "TOUR1EXPLORE", "TOUR1DISCOVER", "TOUR1BUILD",
        "TOUR1CONQUER", "TOUR1GOVERNOR2", "TOUR1ADVANCED", "TOUR1CAPTION",
        "TOUR1ECO", "TOUR1POP", "TOUR1POP2", "TOUR1STAPLE", "TOUR1PRODUCTION",
        "TOUR1QUEUE", "TOUR1CHANGE", "TOUR1HURRY", "TOUR1GARRISON", "TOUR1SUPPORTED",
        "TOUR1FACILITIES", "TOUR1COMMERCE", "TOUR1NUTRIENTS", "TOUR1AREA", "TOUR1NUT",
        "TOUR1MIN", "TOUR1ENERGY", "TOUR1ALLOC", "TOUR1ALLOC2", "TOUR1ECON",
        "TOUR1PSYCH", "TOUR1LABS", "TOUR1SUPPORTBUTTON", "TOUR1PSYCHBUTTON",
        "TOUR1RESOURCEBUTTON",
    ]
    previous_call = -1
    for call_index in calls:
        # At every call site the original freshly reads global 0x7D3C3C,
        # null-tests it, then adds its vbtable-derived GraphicWin adjustment.
        preceding = instructions[previous_call + 1:call_index]
        owner_load = [i for i, ins in enumerate(preceding)
                      if ins.mnemonic == "mov" and ins.op_str == "ecx, dword ptr [0x7d3c3c]"]
        assert owner_load, hex(instructions[call_index].address)
        i = owner_load[-1]
        tail = preceding[i:]
        rendered = [(ins.mnemonic, ins.op_str) for ins in tail]
        assert ("test", "ecx, ecx") in rendered
        assert any(mn in {"jne", "jnz"} for mn, _ in rendered)
        base_load = next(op.split(",")[0] for mn, op in rendered
                         if mn == "mov" and op.endswith("dword ptr [ecx]"))
        adjusted_load = next(
            op.split(",")[0] for mn, op in rendered
            if mn == "mov" and op.endswith(f"dword ptr [{base_load} + 4]")
        )
        assert ("add", f"{adjusted_load}, ecx") in rendered
        assert ("xor", f"{adjusted_load}, {adjusted_load}") in rendered
        assert ("push", adjusted_load) in rendered
        previous_call = call_index
    helper_calls = [(i, ins.op_str) for i, ins in enumerate(instructions)
                    if ins.mnemonic == "call" and ins.address > instructions[calls[-1]].address
                    and ins.op_str in {"0x4ba8f0", "0x4ba870"}]
    assert [target for _, target in helper_calls] == ["0x4ba8f0", "0x4ba8f0", "0x4ba870", "0x4ba870"]
    assert [pushed_label(i) for i, _ in helper_calls] == [
        "TOUR1PREV", "TOUR1NEXT", "TOUR1RENAME", "TOUR1OK"
    ]
    # The test oracle's labels must be present in the pinned image as literals.
    labels = [
        "TOUR1GOVERNOR", "TOUR1EXPLORE", "TOUR1DISCOVER", "TOUR1BUILD",
        "TOUR1CONQUER", "TOUR1GOVERNOR2", "TOUR1ADVANCED", "TOUR1CAPTION",
        "TOUR1ECO", "TOUR1POP", "TOUR1POP2", "TOUR1STAPLE", "TOUR1PRODUCTION",
        "TOUR1QUEUE", "TOUR1CHANGE", "TOUR1HURRY", "TOUR1GARRISON", "TOUR1SUPPORTED",
        "TOUR1FACILITIES", "TOUR1COMMERCE", "TOUR1NUTRIENTS", "TOUR1AREA", "TOUR1NUT",
        "TOUR1MIN", "TOUR1ENERGY", "TOUR1ALLOC", "TOUR1ALLOC2", "TOUR1ECON",
        "TOUR1PSYCH", "TOUR1LABS", "TOUR1SUPPORTBUTTON", "TOUR1PSYCHBUTTON",
        "TOUR1RESOURCEBUTTON", "TOUR1PREV", "TOUR1NEXT", "TOUR1RENAME", "TOUR1OK",
    ]
    image_bytes = raw
    for label in labels:
        needle = label.encode("ascii") + b"\0"
        assert needle in image_bytes, label

