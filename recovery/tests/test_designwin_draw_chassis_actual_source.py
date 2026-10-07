"""Host behavior controls for the installed DesignWin chassis renderer."""

from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/designwin.cpp").is_file():
            return parent
    raise AssertionError("cannot locate repository root")


def _definition(source: str, signature: str) -> str:
    matches = list(re.finditer(rf"(?m)^{re.escape(signature)}\s*\{{", source))
    assert len(matches) == 1, f"expected exactly one installed definition: {signature}"
    start = matches[0].start()
    opening = source.index("{", start)
    depth, state, i = 0, "code", opening
    while i < len(source):
        ch = source[i]
        nxt = source[i + 1] if i + 1 < len(source) else ""
        if state == "line":
            if ch == "\n": state = "code"
        elif state == "block":
            if ch == "*" and nxt == "/": state = "code"; i += 1
        elif state in ("string", "char"):
            if ch == "\\": i += 1
            elif (state == "string" and ch == '"') or (state == "char" and ch == "'"):
                state = "code"
        elif ch == "/" and nxt == "/": state = "line"; i += 1
        elif ch == "/" and nxt == "*": state = "block"; i += 1
        elif ch == '"': state = "string"
        elif ch == "'": state = "char"
        elif ch == "{": depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                body = source[start:i + 1]
                assert "BODY GOES HERE." not in body
                return body
        i += 1
    raise AssertionError(f"unclosed definition: {signature}")


SUPPORT = r'''#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
struct TraceState {
  std::vector<std::string> events;
  std::vector<int> colors;
  std::string font_text, name_text, detail_text;
  RECT model_rect{}, wrapped_rect{};
  int width_result = 1;
  int render_x = 0, render_y = 0;
} trace;
struct Font {
  int unk_1_ = -1, line_height_ = 14, height_ = 10;
  int width(char *text) { trace.events.push_back("width:" + std::string(text)); return trace.width_result; }
};
struct Buffer {
  void set_clip(RECT *) { trace.events.push_back("clip"); }
  void copy(Buffer *, RECT *) { trace.events.push_back("copy"); }
  void set_text_color(int a, int b, int, int) {
    trace.events.push_back("color"); trace.colors.push_back(a); trace.colors.push_back(b);
  }
  void write_cent_l(Font *, char *text, RECT *rect, int) {
    trace.events.push_back("title:" + std::string(text)); trace.font_text = text;
    trace.model_rect.left = rect->left; trace.model_rect.right = rect->right;
    trace.model_rect.top = rect->top;
  }
  void write_cent_l(char *text, RECT *rect, int) {
    std::string value(text);
    trace.events.push_back("write:" + value);
    if (value == "LANDCRAFT") trace.name_text = value;
    else trace.detail_text = value;
    trace.model_rect = *rect;
  }
  void set_font(Font *, int, int, int) { trace.events.push_back("font"); }
  int wrap_cent(char *text, RECT *rect) {
    trace.events.push_back("wrap:" + std::string(text)); trace.wrapped_rect = *rect;
    return 0;
  }
};
struct Caviar {
  void reset_objects() { trace.events.push_back("reset"); }
  void set_scaling(float value) { assert(value == 0.05f); trace.events.push_back("scale"); }
  void set_scene_rotation(float x, float y, float z) {
    assert(x == 0.5233333706855774f && y == 0.0f && z == 0.0f);
    trace.events.push_back("rotation");
  }
  void render(Buffer *, int x, int y, int mode) {
    assert(mode == 2); trace.events.push_back("render"); trace.render_x=x; trace.render_y=y;
  }
};
struct ChassisRow { char *offsv_1_name; unsigned char speed, triad; };
inline ChassisRow Chassis[9]{};
inline char *Triad[5]{}; // fifth sentinel observes the original unchecked byte index
inline int TextColourTable[195]{};
inline char StringTemp[1032]{};
inline char SpaceText[] = " ";
struct StringTableDouble {
  char *get(int address) {
    trace.events.push_back("get:" + std::to_string(address));
    if (address == 0x2000) return const_cast<char *>("landcraft");
    if (address == 0x3000) return const_cast<char *>("land");
    if (address == 0x3004) return const_cast<char *>("unbounded");
    return const_cast<char *>("unknown");
  }
} StringTable;
inline char *label_get(int index) {
  trace.events.push_back("label:" + std::to_string(index));
  return const_cast<char *>("Chassis");
}
inline char *CharUpperA(char *text) {
  trace.events.push_back("upper");
  for (char *p=text; *p; ++p) *p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
  return text;
}
inline char *_itoa(int value, char *out, int radix) {
  assert(radix == 10); std::sprintf(out, "%d", value); return out;
}
void vehdraw_construct_chassis(int faction, int chassis, int weapon, int armor,
                               int reactor, Caviar *) {
  assert(faction == 2 && chassis == 3 && weapon == 5 && armor == 0 && reactor == 4);
  trace.events.push_back("construct");
}
struct DesignWin : Buffer {
  RECT chassis_panel_rect_{10, 20, 210, 180};
  Font font1_{-1, 9, 10}, font2_{-1, 14, 12};
  Buffer buffer_{};
  Caviar caviar_{};
  uint32_t factionID_ = 2, protoChassisType_ = 3, protoWeaponType_ = 5,
           protoReactorType_ = 4;
  void draw_chassis(int redraw);
};
'''


def test_draw_chassis_actual_method_preserves_render_order_wrap_and_unchecked_triad(tmp_path):
    root = _root()
    source = (root / "src/designwin.cpp").read_text(encoding="utf-8")
    method = _definition(source, "void DesignWin::draw_chassis(int redraw)")
    helpers = "\n".join(_definition(source, signature) for signature in (
        "static int designwin_panel_signed(uint32_t bits)",
        "static int designwin_panel_add(int left, int right)",
        "static int designwin_panel_subtract(int left, int right)",
        "static int designwin_panel_font_height(int extra, int line_height, int height)",
        "static void designwin_panel_append_label(char *output, int label_id)",
    ))
    test_main = r'''int main(int argc, char **) {
  Chassis[3].offsv_1_name = reinterpret_cast<char *>(0x2000);
  Chassis[3].speed = 4; Chassis[3].triad = 0;
  Triad[0] = reinterpret_cast<char *>(0x3000);
  Triad[4] = reinterpret_cast<char *>(0x3004);
  TextColourTable[156]=156; TextColourTable[157]=157;
  TextColourTable[158]=158; TextColourTable[159]=159;
  TextColourTable[160]=160; TextColourTable[161]=161;
  DesignWin win;
  trace.width_result = argc > 1 ? 500 : 80;
  win.draw_chassis(0);
  assert(trace.font_text == "CHASSIS");
  assert(trace.render_x == 110 && trace.render_y == 94);
  assert(trace.colors == std::vector<int>({156,157,158,159,160,161}));
  assert(trace.events.front() == "clip" && trace.events[1] == "copy");
  assert(trace.events[trace.events.size()-1] == "write:land 4");
  const auto construct = std::find(trace.events.begin(), trace.events.end(), "construct");
  const auto reset = std::find(trace.events.begin(), trace.events.end(), "reset");
  const auto model = std::find(trace.events.begin(), trace.events.end(), "render");
  assert(reset < construct && construct < model);
  if (argc > 1) {
    assert(std::find_if(trace.events.begin(), trace.events.end(), [](const std::string &s) { return s.find("wrap:LANDCRAFT") == 0; }) != trace.events.end());
  } else {
    assert(std::find(trace.events.begin(), trace.events.end(), "write:LANDCRAFT") != trace.events.end());
  }
  trace = TraceState{};
  Chassis[3].triad = 4;
  win.draw_chassis(1);
  assert(trace.events.back() == "write:unbounded 4");
  return 0;
}
'''
    unit = tmp_path / "designwin_chassis.cpp"
    exe = tmp_path / "designwin_chassis"
    unit.write_text(SUPPORT + "\n" + helpers + "\n" + method + "\n" + test_main,
                    encoding="utf-8")
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang required for source-extracted host test"
    for compiler in compilers:
        built = subprocess.run([compiler, "-std=c++17", "-O0", str(unit), "-o", str(exe)],
                               text=True, capture_output=True)
        assert built.returncode == 0, built.stderr
        for args in ([], ["wrap"]):
            ran = subprocess.run([str(exe), *args], text=True, capture_output=True)
            assert ran.returncode == 0, ran.stderr


def test_design_it_and_preview_use_source_owned_vehicle_apis():
    root = _root()
    source = (root / "src/designwin.cpp").read_text(encoding="utf-8")
    vehicle_header = (root / "src/veh.h").read_text(encoding="utf-8")
    faction_header = (root / "src/faction.h").read_text(encoding="utf-8")
    faction_source = (root / "src/faction.cpp").read_text(encoding="utf-8")
    design_it = _definition(source, "int DesignWin::design_it()")
    flash = _definition(source, "void DesignWin::draw_flash()")
    assert "retire_proto(" in design_it and "prune_protos(" in design_it
    assert design_it.count("name_proto(") == 2
    assert "name_proto(" in flash
    for declaration in ("void __cdecl name_proto(", "void __cdecl retire_proto(",
                        "void __cdecl prune_protos("):
        assert declaration in vehicle_header
    assert "int __cdecl diplo_lock(int patience);" in faction_header
    assert "void __cdecl diplo_unlock();" in faction_header
    assert "int __cdecl diplo_lock(int patience)" in faction_source
    assert "void __cdecl diplo_unlock()" in faction_source
    multiplayer_path = design_it[design_it.index("if (!IsMultiplayerNet)"):]
    lock_order = ("diplo_lock(0x32)",
                  "message_data(0x2415, 0, protoID_, factionID_, 0, 0)",
                  "NetDaemonNet.await_exec(0)",
                  "diplo_unlock()")
    lock_positions = [multiplayer_path.index(token) for token in lock_order]
    assert lock_positions == sorted(lock_positions)
    for body in (design_it, flash):
        assert not any(token in body for token in (
            "name_proto_seam", "retire_proto_seam", "prune_protos_seam",
            "0x0057E040", "0x0057F1D0", "0x0057F4B0"))
    assert not any(token in design_it for token in (
        "diplo_lock_seam", "diplo_unlock_seam", "0x00539820", "0x005398C0"))
