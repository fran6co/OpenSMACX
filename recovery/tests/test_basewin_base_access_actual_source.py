"""Actual-source controls for BaseWin inherited width and Win-base calls.

Storage and Win32 services are projected host doubles; these controls do not
establish native object layout, vtable ABI, linked preservation or GUI behavior.
The actual product bodies and width accessor are compiled without rewriting.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/IMAGE_PIN").is_file())


def extract(text, signature):
    assert text.count(signature) == 1, signature
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for i in range(opening, len(text)):
        if text[i] == "{":
            depth += 1
        elif text[i] == "}":
            depth -= 1
            if depth == 0:
                return text[start:i + 1]
    raise AssertionError("unbalanced body: " + signature)


def actual_bodies():
    source = (ROOT / "src/basewin.cpp").read_text(encoding="utf-8")
    hittest = extract(source, "int BaseWin::on_nc_hittest(int a1, int a2)")
    bubble = extract(source, "void BaseWin::bubble_me_elmo2(char *text, int x, int y)")
    assert "RECT rect = { x, y, x, y };" in bubble
    return hittest, bubble


PREAMBLE = r'''#include <assert.h>
struct RECT { int left, top, right, bottom; };
struct BitmapHeader { int biWidth, biHeight; };
struct Dib { BitmapHeader bmiHeader; };
struct Buffer { Dib dib_; WIDTH_ACCESSOR };
struct Win {
  int base_calls;
  int observed_x, observed_y;
  char *observed_text;
  RECT observed_rect;
  Win() : base_calls(0), observed_x(0), observed_y(0), observed_text(0),
          observed_rect() {}
  virtual int on_nc_hittest(int x, int y) { ++base_calls; return x + y + 700; }
  void client_to_screen(int *x, int *y) { *x += 101; *y -= 37; }
  void set_bubble_text(char *text, RECT *r) {
    observed_text = text; observed_rect = *r;
  }
};
struct GraphicWin : Win, Buffer {};
struct BaseWin : GraphicWin {
  int on_nc_hittest(int, int);
  void bubble_me_elmo2(char *, int, int);
};
'''


MAIN = r'''struct FurtherOverride : BaseWin {
  int on_nc_hittest(int, int) { return -999; }
};
int main() {
  FurtherOverride b;
  b.dib_.bmiHeader.biWidth = 100;
  b.dib_.bmiHeader.biHeight = -400;
  assert(b.BaseWin::on_nc_hittest(22, 1) == 2);
  assert(b.BaseWin::on_nc_hittest(23, 1) == 724);
  assert(b.base_calls == 1);
  assert(b.BaseWin::on_nc_hittest(45, 1) == 746); // width - 55 is inclusive
  assert(b.BaseWin::on_nc_hittest(46, 1) == 2);
  assert(b.base_calls == 2);
  char message[] = "bubble";
  b.bubble_me_elmo2(message, 12, -8);
  assert(b.observed_text == message);
  assert(b.observed_rect.left == 113 && b.observed_rect.top == -45);
  assert(b.observed_rect.right == 113 && b.observed_rect.bottom == -45);
  return 0;
}
'''


def translation(hittest, bubble):
    accessor = extract((ROOT / "src/buffer.h").read_text(),
                       "int pixel_width() const")
    return PREAMBLE.replace("WIDTH_ACCESSOR", accessor) + hittest + "\n" + bubble + "\n" + MAIN


def compile_run(compiler, code, should_pass):
    with tempfile.TemporaryDirectory(prefix="basewin-owner-host-") as td:
        cpp, exe = Path(td) / "controls.cpp", Path(td) / "controls"
        cpp.write_text(code, encoding="utf-8")
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                "-Werror", str(cpp), "-o", str(exe)],
                               capture_output=True, text=True, timeout=30)
        assert built.returncode == 0, built.stdout + built.stderr
        run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=5)
        assert (run.returncode == 0) == should_pass, run.stdout + run.stderr


def test_source_relationships_and_host_controls():
    bw, gw, buf, win = [ (ROOT / path).read_text(encoding="utf-8") for path in
                         ("src/basewin.h", "src/graphicwin.h", "src/buffer.h", "src/win.h") ]
    assert "class BaseWin : public GraphicWin, public SubInterface" in bw
    assert "class GraphicWin : public Win, public Buffer" in gw
    assert "int pixel_width() const { return dib_.bmiHeader.biWidth; }" in buf
    assert "virtual int on_nc_hittest(int x, int y);" in win
    assert "void client_to_screen(int *x, int *y);" in win
    assert "void set_bubble_text(char * text, RECT * rect);" in win
    hittest, bubble = actual_bodies()
    assert "pixel_width() - 0x37" in hittest
    assert "Win::on_nc_hittest(a1, a2)" in hittest
    assert "reinterpret_cast" not in hittest + bubble
    assert "RECT rect = { x, y, x, y };" in bubble
    compilers = (shutil.which("g++"), shutil.which("clang++"))
    assert all(compilers), "GCC and Clang are required"
    for compiler in compilers:
        compile_run(compiler, translation(hittest, bubble), True)


def test_mutated_controls_reject_wrong_threshold_width_dispatch_and_rect():
    hittest, bubble = actual_bodies()
    mutations = (
        (hittest.replace("a1 >= 0x17", "a1 > 0x17", 1), bubble),
        (hittest.replace("pixel_width() - 0x37", "pixel_width() - 0x36", 1), bubble),
        (hittest.replace("pixel_width()", "0", 1), bubble),
        (hittest.replace("Win::on_nc_hittest(a1, a2)",
                         "on_nc_hittest(a1, a2)", 1), bubble),
        (hittest, bubble.replace("RECT rect = { x, y, x, y };",
                                 "RECT rect = { x, y, x + 1, y };", 1)),
    )
    for compiler_name in ("g++", "clang++"):
        compiler = shutil.which(compiler_name)
        assert compiler, compiler_name + " is required"
        for bad_hittest, bad_bubble in mutations:
            compile_run(compiler, translation(bad_hittest, bad_bubble), False)

