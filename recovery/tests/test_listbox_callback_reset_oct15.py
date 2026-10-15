"""Host controls for ListBox's recovered callback-reset receiver."""

import re
import shutil
import subprocess

import pytest

from test_application_link_recovery import ROOT


def _body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError(f"unterminated body: {signature}")


def _harness(tmp_path, mutate=None):
    cpp = (ROOT / "src/listbox.cpp").read_text()
    header = (ROOT / "src/listbox.h").read_text()
    dialog_header = (ROOT / "src/dialog.h").read_text()
    ctor = _body(cpp, "ListBox::ListBox()")
    close = _body(cpp, "void ListBox::close()")
    if mutate:
        for member, inherited in (("draw_callback_", "field_30_"),
                                  ("write_callback_", "field_34_"),
                                  ("fill_callback_", "field_38_")):
            if mutate == "constructor":
                assert f"{member} = 0;" in ctor
                ctor = ctor.replace(f"{member} = 0;", f"{inherited} = 0;")
            elif mutate == "close":
                assert f"{member} = 0;" in close
                close = close.replace(f"{member} = 0;", f"{inherited} = 0;")
            else:
                raise AssertionError("unknown mutation")

    typedefs = "\n".join(line.strip() for line in re.findall(
        r"^\s*typedef (?:void|int) \(__cdecl \*(?:DrawCallback|WriteCallback|FillCallback)\)\([^\n]+",
        header, re.M))
    assert len(typedefs.splitlines()) == 3
    assert all(re.search(rf"uint32_t field_{off}_;", dialog_header)
               for off in ("30", "34", "38"))
    assert all(f"{typ} {name}_;" in header for typ, name in (
        ("DrawCallback", "draw_callback"), ("WriteCallback", "write_callback"),
        ("FillCallback", "fill_callback")))

    source = r'''#include <cstdint>
#include <cstdlib>
#define __cdecl
struct RECT { int left, top, right, bottom; };
class GraphicWin { public: virtual ~GraphicWin() {} void close() { events[nevents++] = 1; }
  static int events[4], nevents; };
int GraphicWin::events[4] = {}; int GraphicWin::nevents = 0;
class Dialog { public: virtual ~Dialog() {} void close() { GraphicWin::events[GraphicWin::nevents++] = 2; }
  uint32_t field_30_ = 0x30303030, field_34_ = 0x34343434, field_38_ = 0x38383838;
  uint32_t field_B4_ = 0; };
''' + typedefs + r'''
uint32_t ListBoxDefault10=10, ListBoxDefault14=14, ListBoxDefault24=24,
         ListBoxDefault2C=44, ListBoxCloseDynamicDefault=28;
void draw(GraphicWin*,int,int,int,int,int) {}
int write_cb(GraphicWin*,int,int,RECT*,char*) { return 1; }
int fill_cb(GraphicWin*,int,int,RECT*) { return 1; }
class ListBox : public virtual GraphicWin, public virtual Dialog {
public:
  uint32_t field_4_=0, field_8_=0, field_C_=0, field_10_=0, field_14_=0,
    field_18_=0, field_1C_=0, field_20_=0, field_24_=0, field_28_=0, field_2C_=0;
  DrawCallback draw_callback_ = draw; WriteCallback write_callback_ = write_cb;
  FillCallback fill_callback_ = fill_cb;
  void (*selection_callback_)(int)=draw_selection;
  void (*double_click_callback_)(int)=draw_selection;
  static void draw_selection(int) {}
  ListBox(); void close();
};
''' + ctor + "\n" + close + r'''
int main() {
  ListBox box;
  if (box.draw_callback_ || box.write_callback_ || box.fill_callback_) return 1;
  if (box.field_30_ != 0x30303030 || box.field_34_ != 0x34343434 ||
      box.field_38_ != 0x38383838 || box.field_B4_ != 1) return 2;
  box.draw_callback_=draw; box.write_callback_=write_cb; box.fill_callback_=fill_cb;
  box.field_30_=0x11111111; box.field_34_=0x22222222; box.field_38_=0x33333333;
  box.field_B4_=0;
  GraphicWin::nevents=0;
  box.close();
  if (GraphicWin::nevents != 2 || GraphicWin::events[0] != 1 || GraphicWin::events[1] != 2) return 3;
  if (box.draw_callback_ || box.write_callback_ || box.fill_callback_ ||
      box.field_30_ != 0x11111111 || box.field_34_ != 0x22222222 ||
      box.field_38_ != 0x33333333 || box.field_B4_ != 1) return 4;
  return 0;
}
'''
    path = tmp_path / ("listbox_mutant.cpp" if mutate else "listbox_live.cpp")
    path.write_text(source)
    return path


@pytest.mark.parametrize("compiler", ["clang++", "g++"])
def test_actual_constructor_and_close_reset_callback_members_only(tmp_path, compiler):
    if not shutil.which(compiler):
        pytest.skip(f"{compiler} unavailable")
    source = _harness(tmp_path)
    exe = tmp_path / "live"
    built = subprocess.run([compiler, "-std=c++17", str(source), "-o", str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True)
    assert ran.returncode == 0, f"actual recovered bodies failed control {ran.returncode}"


@pytest.mark.parametrize("compiler", ["clang++", "g++"])
@pytest.mark.parametrize("mutation,expected_exit", [("constructor", 1), ("close", 4)])
def test_old_inherited_field_resets_are_rejected(tmp_path, compiler, mutation, expected_exit):
    if not shutil.which(compiler):
        pytest.skip(f"{compiler} unavailable")
    source = _harness(tmp_path, mutate=mutation)
    exe = tmp_path / "mutant"
    built = subprocess.run([compiler, "-std=c++17", str(source), "-o", str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True)
    assert ran.returncode == expected_exit, "mutation did not fail at its own callback-reset check"
