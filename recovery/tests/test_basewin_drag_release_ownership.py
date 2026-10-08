"""Actual-source drag/capture controls; host doubles are not native ABI proof."""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/IMAGE_PIN").is_file())
SOURCE = ROOT / "src/basewin.cpp"
SIGNATURES = {
    "on_iface_left_up": r"void\s+BaseWin::on_iface_left_up\s*\(\s*int\s+xCoord\s*,\s*int\s+yCoord\s*\)\s*\{",
    "on_left_up": r"void\s+BaseWin::on_left_up\s*\(\s*int\s+xCoord\s*,\s*int\s+yCoord\s*\)\s*\{",
}


def extract_function(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    found = list(re.finditer(SIGNATURES[signature], masked))
    assert len(found) == 1, f"active {signature} definition missing/ambiguous"
    start = found[0].start()
    depth = 0
    for pos in range(masked.find("{", start), len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError(f"unterminated actual {signature}")


HOST = r'''#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
struct RECT { int left, top, right, bottom; };
static std::vector<std::string> events;
struct Buffer { RECT rect2_{0,0,10,10}; };
struct MainInterface;
extern MainInterface MainInterfaceGlobal;
struct Win;
Win *WinPointerOwner3=nullptr, *WinPointerOwner4=nullptr;
struct Win {
  virtual ~Win() = default;
  virtual void on_lose_mouse_capture() { events.push_back("lose");
    assert(WinPointerOwner3 == nullptr);
    if (capture_replacement) WinPointerOwner4=capture_replacement; }
  Win *capture_replacement=nullptr;
  int conversion_x=0, conversion_y=0;
  void screen_to_client(int *x, int *y) { events.push_back("tut-screen-to-client");
    *x -= 100; *y -= 200; }
};
struct GraphicWin : Win, Buffer {};
struct BaseWin : GraphicWin {
  // Distinct shadowing member: the original callback needs Buffer's rect.
  RECT rect2_{900,900,910,910};
  int field_40B54_=0, field_40B4C_=0, field_40B50_=0;
  int in_box(int x, int y, RECT *r) {
    events.push_back("hit-test");
    return x >= r->left && x < r->right && y >= r->top && y < r->bottom;
  }
  void on_left_click(int, int) { events.push_back("front-click"); }
  void on_iface_left_click(int, int) { events.push_back("iface-click"); }
  void set_cursor(int) { events.push_back("window-cursor"); }
  void client_to_screen(int *x, int *y) { events.push_back("base-client-to-screen");
    *x += 100; *y += 200; }
  void on_iface_left_up(int xCoord, int yCoord);
  void on_left_up(int xCoord, int yCoord);
};
BaseWin TutWinBaseWindow;
struct MainInterface : GraphicWin { int client_to_screen_calls=0, cursor_calls=0;
  void client_to_screen(int *x,int *y) { ++client_to_screen_calls;
    events.push_back("main-client-to-screen"); *x += 100; *y += 200; }
  void set_cursor(int) { ++cursor_calls; events.push_back("main-cursor"); }
  void screen_to_client(int *x,int *y) { events.push_back("main-screen-to-client");
    *x -= 100; *y -= 200; }
} MainInterfaceGlobal;
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "clear_after_callback", "stale_owner", "shadow_rect"))
def test_actual_full_methods_branches_and_fresh_owner_reload(tmp_path: Path,
                                                           compiler_name: str,
                                                           damage: str | None) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    text = SOURCE.read_text(encoding="utf-8")
    bodies = {name: extract_function(text, name) for name in SIGNATURES}
    for name, body in bodies.items():
        assert "WinPointerOwner3" in body and "WinPointerOwner4" in body
        assert "IfaceCaptureSlot" not in body and "IfaceEnterSlot" not in body
        assert "IfaceDropRect" not in body and "InterfaceClipRect" not in body
        fields = ("left", "top", "right", "bottom")
        owner = "TutWinBaseWindow" if name == "on_iface_left_up" else "MainInterfaceGlobal"
        copy_sites = [body.index(f"rect.{field} = {owner}.Buffer::rect2_.{field};")
                      for field in fields]
        assert copy_sites == sorted(copy_sites), f"{name} rectangle copy order changed"
    assert "TutWinBaseWindow.Buffer::rect2_" in bodies["on_iface_left_up"]
    assert "MainInterfaceGlobal.Buffer::rect2_" in bodies["on_left_up"]
    body = bodies["on_iface_left_up"]
    if damage == "clear_after_callback":
        assert body.count("WinPointerOwner3 = 0;") == 1
        body = body.replace("WinPointerOwner3 = 0;", "", 1)
        body = body.replace("on_lose_mouse_capture();",
                            "on_lose_mouse_capture(); WinPointerOwner3 = 0;", 1)
    elif damage == "stale_owner":
        opening = body.index("{")
        body = body[:opening + 1] + "\nWin *cached_owner4 = WinPointerOwner4;" + body[opening + 1:]
        assert body.count("if (WinPointerOwner4 == self)") == 1
        body = body.replace("if (WinPointerOwner4 == self)", "if (cached_owner4 == self)", 1)
    elif damage == "shadow_rect":
        body = body.replace("TutWinBaseWindow.Buffer::rect2_", "TutWinBaseWindow.rect2_")
    bodies["on_iface_left_up"] = body
    main = r'''
  BaseWin self, other;
  self.Buffer::rect2_={0,0,10,10};

  // Capture cleanup remains active when the drag flag is nonpositive.
  WinPointerOwner3=&self; WinPointerOwner4=&self;
  self.on_iface_left_up(1,1);
  assert(events.size()==1 && events[0]=="lose" && !WinPointerOwner3 && !WinPointerOwner4);

  // Interface-up: initial hit routes to interface click; miss/clip-hit routes
  // to front click. Capture-loss callback changes Owner4; following cleanup
  // must reread it and preserve the replacement.
  events.clear(); self.field_40B54_=1;
  self.on_iface_left_up(2,2);
  assert(events.size()==2 && events[0]=="hit-test" && events[1]=="iface-click");
  events.clear(); self.field_40B54_=1; self.field_40B4C_=4; self.field_40B50_=5;
  self.capture_replacement=&other;
  TutWinBaseWindow.Buffer::rect2_={0,0,30,30};
  WinPointerOwner3=&self; WinPointerOwner4=&self;
  self.on_iface_left_up(20,20);
  assert(events[0]=="hit-test" && events[1]=="main-client-to-screen"
         && events[2]=="tut-screen-to-client" && events[3]=="hit-test"
         && events[4]=="front-click" && events[5]=="lose");
  assert(!WinPointerOwner3 && WinPointerOwner4==&other);

  // Front-up has the opposite first-hit/clip-hit dispatch and uses the main
  // interface as the other coordinate space.
  events.clear(); self.field_40B54_=1;
  self.on_left_up(2,2);
  assert(events.size()==2 && events[0]=="hit-test" && events[1]=="front-click");
  events.clear(); self.field_40B54_=1; self.field_40B4C_=7; self.field_40B50_=8;
  self.capture_replacement=&other;
  MainInterfaceGlobal.Buffer::rect2_={0,0,30,30};
  WinPointerOwner3=&self; WinPointerOwner4=&self;
  self.on_left_up(20,20);
  assert(events[0]=="hit-test" && events[1]=="base-client-to-screen"
         && events[2]=="main-screen-to-client" && events[3]=="hit-test"
         && events[4]=="iface-click" && events[5]=="lose");
  assert(!WinPointerOwner3 && WinPointerOwner4==&other);

  // Both clip misses preserve cursor and drag cleanup behavior.
  events.clear(); MainInterfaceGlobal.cursor_calls=0;
  self.field_40B54_=1; self.field_40B4C_=9; self.field_40B50_=10;
  TutWinBaseWindow.Buffer::rect2_={30,40,50,60};
  self.on_iface_left_up(20,20);
  assert(self.field_40B54_==-1 && self.field_40B4C_==-1 && self.field_40B50_==-1);
  assert(MainInterfaceGlobal.cursor_calls==1);
  events.clear(); MainInterfaceGlobal.cursor_calls=0;
  self.field_40B54_=1; self.field_40B4C_=9; self.field_40B50_=10;
  MainInterfaceGlobal.Buffer::rect2_={30,40,50,60};
  self.on_left_up(20,20);
  assert(self.field_40B54_==-1 && self.field_40B4C_==-1 && self.field_40B50_==-1);
  assert(MainInterfaceGlobal.cursor_calls==1);
'''
    source = HOST + bodies["on_iface_left_up"] + "\n" + bodies["on_left_up"] + "\nint main(){" + main + "}\n"
    cpp, exe = tmp_path / "actual.cpp", tmp_path / "actual"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++14", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True,
                           text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    assert (ran.returncode == 0) == (damage is None), ran.stdout + ran.stderr
