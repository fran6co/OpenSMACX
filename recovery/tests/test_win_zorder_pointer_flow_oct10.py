"""Actual-source controls for Win z-order pointer identity and compaction.

These host controls exercise extracted product bodies; they do not establish
the native VC6 ABI, matching, or linked-image preservation.
"""

from pathlib import Path
import re
import shutil
import subprocess

from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/win.cpp").is_file())
SOURCE = ROOT / "src/win.cpp"


def extract(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    matches = list(re.finditer(re.escape(signature), masked))
    assert len(matches) == 1, (signature, len(matches))
    opening = masked.find("{", matches[0].end())
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[matches[0].start():end + 1]
    raise AssertionError(f"unterminated {signature}")


def close_compaction(source: str) -> str:
    close = extract(source, "void Win::close()")
    declaration = "Win **node2 = WinRootWindows + i;"
    assert close.count(declaration) == 1
    declaration_at = close.index(declaration)
    opening = close.rfind("if (i < WinRootCount) {", 0, declaration_at)
    assert opening >= 0
    depth = 0
    for end in range(close.index("{", opening), len(close)):
        if close[end] == "{":
            depth += 1
        elif close[end] == "}":
            depth -= 1
            if depth == 0:
                return close[opening:end + 1]
    raise AssertionError("unterminated root-list compaction")


FIXTURE = r'''
#include <cassert>
#include <cstdint>
#include <vector>
#define __cdecl
struct RECT { int left, top, right, bottom; };
typedef RECT tagRECT;
struct Win;
struct Sprite {};
struct Buffer {
  int fills, copies;
  Buffer() : fills(0), copies(0) {}
  void fill(RECT *, int) { ++fills; }
  void fill(int) { ++fills; }
  int copy(Buffer *, int, int, int, int, int, int) { ++copies; return 0; }
};
static std::vector<Win *> paints;
static std::vector<Win *> nc_paints;
static Win *WinZOrderWindow;
static Win *WinZOrderArray[512];
static int WinZOrderCount, WinZOrderFlag;
static Win *WinRootWindows[512];
static int WinRootCount;
static Buffer *WinBackBuffer;
static int WinViewOriginX, WinViewOriginY;
static int WinFlipSpriteY, WinFlipSpriteX, WinClipWidth, WinClipHeight;
static Sprite *WinFlipSprite;
// These tests cover borrower identity, not the production lifetime guards.
struct BufferFixtureOwner {
  Buffer value_;
  Buffer &buffer() { return value_; }
};
static BufferFixtureOwner ScreenBufferOwner, WinDrawingBufferOwner;
static Win *visibility_callback_target;
static Win *visibility_callback_owner;
static bool callback_armed;

struct Win {
  unsigned int iFlags_, iSomeFlag_;
  Win *poWinBase_, *win_parent_;
  Win *children_[8];
  int child_count_;
  RECT client_rect_, outer_rect_;
  bool visible;
  Win() : iFlags_(0), iSomeFlag_(0), poWinBase_(this), win_parent_(0),
          child_count_(0), client_rect_{0,0,0,0}, outer_rect_{0,0,0,0},
          visible(true) { for (int i=0; i<8; ++i) children_[i]=0; }
  int is_visible() {
    if (callback_armed && this == visibility_callback_owner) {
      WinZOrderWindow = visibility_callback_target;
    }
    return visible;
  }
  void client_to_screen(int *x, int *y) { *x += outer_rect_.left; *y += outer_rect_.top; }
  void on_nc_paint(RECT *, int) { nc_paints.push_back(this); }
  int on_paint(RECT *) { paints.push_back(this); return 0; }
  static int update_screen(RECT *, Win *);
  void update_window_to_buffer(Buffer *);
};
static int IntersectRect(RECT *out, const RECT *a, const RECT *b) {
  out->left = a->left > b->left ? a->left : b->left;
  out->top = a->top > b->top ? a->top : b->top;
  out->right = a->right < b->right ? a->right : b->right;
  out->bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
  return out->left < out->right && out->top < out->bottom;
}
// ACTUAL_WIN_BODIES
static void reset_globals() {
  WinZOrderWindow=0; WinZOrderCount=0; WinZOrderFlag=0;
  WinRootCount=0; WinBackBuffer=0;
  WinViewOriginX=WinViewOriginY=0;
  WinFlipSprite=0; WinFlipSpriteY=WinFlipSpriteX=0;
  WinClipWidth=WinClipHeight=0;
  paints.clear(); nc_paints.clear();
  callback_armed=false; visibility_callback_target=0; visibility_callback_owner=0;
  for (int i=0;i<512;++i) { WinZOrderArray[i]=0; WinRootWindows[i]=0; }
}

int main(int argc, char **argv) {
  reset_globals();
  if (argc > 1 && argv[1][0] == 'r') {
    Win first, second, target, alias;
    first.iSomeFlag_=1; second.iSomeFlag_=3; target.iSomeFlag_=3;
    first.poWinBase_=&alias; second.poWinBase_=&target; target.poWinBase_=&target;
    WinZOrderCount=3; WinZOrderWindow=&target;
    WinZOrderArray[0]=&first; WinZOrderArray[1]=&second; WinZOrderArray[2]=&target;
    assert(Win::update_screen(0, &target)==0);
    // The start index is the real target's table slot; paints walk backwards.
    assert(nc_paints.size()==2);
    assert(nc_paints[0]==&target && nc_paints[1]==&second);
    assert(paints.size()==2 && paints[0]==&target && paints[1]==&target);
    return 0;
  }
  if (argc > 1 && argv[1][0] == 'x') {
    Win a,b,c,d;
    WinRootWindows[0]=&a; WinRootWindows[1]=&b;
    WinRootWindows[2]=&c; WinRootWindows[3]=&d;
    WinRootCount=3; int i=1;
    // ACTUAL_CLOSE_COMPACTION
    assert(WinRootWindows[0]==&a && WinRootWindows[1]==&c && WinRootWindows[2]==&d);
    return 0;
  }

  Win subject, first_root, second_root, child, parent_probe;
  subject.iSomeFlag_=3;
  first_root.iSomeFlag_=3; second_root.iSomeFlag_=3;
  first_root.poWinBase_=&first_root; second_root.poWinBase_=&second_root;
  first_root.child_count_=1; first_root.children_[0]=&child;
  child.iSomeFlag_=1; child.win_parent_=&parent_probe;
  WinRootWindows[0]=&first_root; WinRootWindows[1]=&second_root;
  WinRootWindows[2]=&subject;
  WinRootCount=3;
  visibility_callback_owner=&parent_probe;
  visibility_callback_target=&second_root;
  callback_armed=true;
  subject.update_window_to_buffer(&WinDrawingBufferOwner.buffer());
  // Visibility reenters the traversal and changes the active window. Both
  // root passes must reload that pointer after recursion; the second root
  // then resets the list before appending itself.
  assert(WinBackBuffer==0);
  assert(WinZOrderWindow==&second_root);
  assert(WinZOrderCount==2 && WinZOrderArray[0]==&second_root
         && WinZOrderArray[1]==&subject);
  assert((WinZOrderFlag & 1)==0);
  assert(nc_paints.size()==2 && nc_paints[0]==&subject
         && nc_paints[1]==&second_root);
  return 0;
}
'''


def translation(mutant=None):
    source = SOURCE.read_text(encoding="utf-8")
    zorder = extract(source, "void __cdecl recurse_zorder(Win * window)")
    update_screen = extract(source, "int __cdecl Win::update_screen(RECT *area, Win *window)")
    update_window = extract(source, "void Win::update_window_to_buffer(Buffer * buffer)")
    compaction = close_compaction(source)
    assert "Win *cur;" in zorder and "reinterpret_cast<int>" not in zorder
    assert "Win *target;" in update_screen
    assert "Win *w = WinZOrderArray[i];" in update_screen
    assert "Win **node2 = WinRootWindows + i;" in compaction
    bodies = [zorder, update_screen, update_window]
    if mutant == "wrong_valid_render_entry":
        changed = update_screen.replace(
            "Win *w = WinZOrderArray[i];",
            "Win *w = WinZOrderArray[i == 2 ? 1 : i];", 1)
        assert changed != update_screen
        bodies[1] = changed
    elif mutant == "stale_first_reload":
        changed = update_window.replace(
            "recurse_zorder(*piVar4);\n                    iVar2 = WinZOrderWindow;",
            "recurse_zorder(*piVar4);\n                    iVar2 = 0;", 1)
        assert changed != update_window
        bodies[2] = changed
    fixture = FIXTURE.replace("// ACTUAL_WIN_BODIES", "\n".join(bodies))
    return fixture.replace("// ACTUAL_CLOSE_COMPACTION", compaction)


def run_fixture(tmp_path, source, *args):
    compiler = shutil.which("g++")
    if not compiler:
        import pytest
        pytest.skip("g++ unavailable")
    cpp, binary = tmp_path / "win_zorder.cpp", tmp_path / "win_zorder"
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra",
                            "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
                          capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary), *args], capture_output=True,
                          text=True, timeout=30)


def test_actual_pointer_flow_render_reload_and_close_compaction(tmp_path):
    for mode in ("render", "callback", "close"):
        run = run_fixture(tmp_path, translation(),
                          "render" if mode == "render" else
                          "xclose" if mode == "close" else "callback")
        assert run.returncode == 0, run.stdout + run.stderr


def test_actual_pointer_flow_controls_reject_valid_wrong_entry_and_stale_reload(tmp_path):
    wrong_entry = run_fixture(tmp_path, translation("wrong_valid_render_entry"), "render")
    assert wrong_entry.returncode != 0
    stale_reload = run_fixture(tmp_path, translation("stale_first_reload"), "callback")
    assert stale_reload.returncode != 0


def test_close_root_compaction_uses_typed_slots_and_rejects_wrong_valid_slot(tmp_path):
    actual = translation()
    run = run_fixture(tmp_path, actual, "xclose")
    assert run.returncode == 0, run.stdout + run.stderr

    source = SOURCE.read_text(encoding="utf-8")
    compaction = close_compaction(source)
    mutant = compaction.replace("WinRootWindows + i", "WinRootWindows + i + 1", 1)
    assert mutant != compaction
    changed = actual.replace(compaction, mutant, 1)
    run = run_fixture(tmp_path, changed, "xclose")
    assert run.returncode != 0
