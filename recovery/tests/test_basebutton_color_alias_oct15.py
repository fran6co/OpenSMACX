"""Exercise BaseButton's actual default-colour producers and init reads.

The host projection compiles the unchanged setter bodies and contiguous colour
call fragment extracted from init. It validates slot mapping, guards, call
order, and that later calls reload values after a callback mutates the table;
it is not a VC6 ABI or full-init test.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/basebutton.cpp").read_text()
HEADER = (ROOT / "src/basebutton.h").read_text()


def body(signature):
    start = SOURCE.index(signature)
    opening = SOURCE.index("{", start)
    depth = 0
    for end in range(opening, len(SOURCE)):
        depth += (SOURCE[end] == "{") - (SOURCE[end] == "}")
        if depth == 0:
            return SOURCE[start:end + 1]
    raise AssertionError("unterminated " + signature)


SETTERS = "\n".join(
    body("void BaseButton::" + name + "(")
    for name in ("set_def_text_color", "set_def_text_color2", "set_def_text_color3")
)

INIT_START = SOURCE.index("int BaseButton::init(")
COLOR_START = SOURCE.index("    Buffer::set_text_color(\n", INIT_START)
COLOR_END_TEXT = "        static_cast<int>(DialogDefaults[13]), static_cast<int>(DialogDefaults[16]));"
COLOR_END = SOURCE.index(COLOR_END_TEXT, COLOR_START) + len(COLOR_END_TEXT)
INIT_COLOR_CALLS = SOURCE[COLOR_START:COLOR_END]


def setter_program(setters, damage):
    if damage == "wrong_setter_slot":
        damaged = setters.replace("DialogDefaults[8] =", "DialogDefaults[7] =", 1)
        assert damaged != setters
        setters = damaged
    return r'''#include <cassert>
#include <climits>
#include <stdint.h>
uint32_t DialogDefaults[37];
class BaseButton {
public:
  static void set_def_text_color(int, int, int, int);
  static void set_def_text_color2(int, int, int, int);
  static void set_def_text_color3(int, int, int, int);
};
SETTERS
int main() {
  for (int i=0; i<37; ++i) DialogDefaults[i]=0x51000000u+(uint32_t)i;
  BaseButton::set_def_text_color(-1, INT_MIN, 23, -17);
  assert(DialogDefaults[5]==0xffffffffu && DialogDefaults[8]==0x80000000u);
  assert(DialogDefaults[11]==23u && DialogDefaults[14]==0xffffffefu);
  for (int i=5; i<=16; ++i) {
    if (i!=5 && i!=8 && i!=11 && i!=14)
      assert(DialogDefaults[i]==0x51000000u+(uint32_t)i);
  }
  BaseButton::set_def_text_color2(101, 102, 103, 104);
  assert(DialogDefaults[6]==101u && DialogDefaults[9]==102u);
  assert(DialogDefaults[12]==103u && DialogDefaults[15]==104u);
  BaseButton::set_def_text_color3(201, 202, 203, 204);
  assert(DialogDefaults[7]==201u && DialogDefaults[10]==202u);
  assert(DialogDefaults[13]==203u && DialogDefaults[16]==204u);
  for (int i=0; i<5; ++i) assert(DialogDefaults[i]==0x51000000u+(uint32_t)i);
  for (int i=17; i<37; ++i) assert(DialogDefaults[i]==0x51000000u+(uint32_t)i);
}
'''.replace("SETTERS", setters)


def init_program(calls, damage):
    if damage == "wrong_init_slot":
        damaged = calls.replace("DialogDefaults[8]", "DialogDefaults[7]", 1)
        assert damaged != calls
        calls = damaged
    elif damage == "reordered_init_calls":
        first = calls.index("Buffer::set_text_color(\n")
        second = calls.index("Buffer::set_text_color2(\n")
        third = calls.index("Buffer::set_text_color3(\n")
        blocks = [calls[first:second], calls[second:third], calls[third:]]
        calls = blocks[1] + blocks[0] + blocks[2]
    return r'''#include <cassert>
#include <stdint.h>
uint32_t DialogDefaults[37];
struct Buffer {
  static int calls;
  static uint32_t seen[3][4];
  static void mutate(uint32_t marker) {
    for (int i=5; i<=16; ++i) DialogDefaults[i]=marker+(uint32_t)i;
  }
  static void set_text_color(int a,int b,int c,int d) {
    seen[calls][0]=a; seen[calls][1]=b; seen[calls][2]=c; seen[calls][3]=d;
    ++calls; mutate(0x200u);
  }
  static void set_text_color2(int a,int b,int c,int d) {
    seen[calls][0]=a; seen[calls][1]=b; seen[calls][2]=c; seen[calls][3]=d;
    ++calls; mutate(0x300u);
  }
  static void set_text_color3(int a,int b,int c,int d) {
    seen[calls][0]=a; seen[calls][1]=b; seen[calls][2]=c; seen[calls][3]=d;
    ++calls;
  }
};
int Buffer::calls=0;
uint32_t Buffer::seen[3][4]={{0}};
void init_color_fragment() {
CALLS
}
int main() {
  for (int i=0; i<37; ++i) DialogDefaults[i]=0x100u+(uint32_t)i;
  init_color_fragment();
  assert(Buffer::calls==3);
  const uint32_t expected[3][4]={{0x105,0x108,0x10b,0x10e},
                                 {0x206,0x209,0x20c,0x20f},
                                 {0x307,0x30a,0x30d,0x310}};
  for (int r=0; r<3; ++r)
    for (int c=0; c<4; ++c) assert(Buffer::seen[r][c]==expected[r][c]);
  for (int i=0; i<5; ++i) assert(DialogDefaults[i]==0x100u+(uint32_t)i);
  for (int i=17; i<37; ++i) assert(DialogDefaults[i]==0x100u+(uint32_t)i);
}
'''.replace("CALLS", calls)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "wrong_setter_slot"])
def test_setters_write_only_their_four_table_slots(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source, executable = tmp_path / "setters.cpp", tmp_path / "setters"
    source.write_text(setter_program(SETTERS, damage))
    compiled = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                               str(source), "-o", str(executable)],
                              capture_output=True, text=True, timeout=45)
    assert compiled.returncode == 0, compiled.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
    assert (run.returncode == 0) == (damage is None), run.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "wrong_init_slot", "reordered_init_calls"])
def test_init_fragment_reloads_each_tier_between_buffer_calls(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    source, executable = tmp_path / "init.cpp", tmp_path / "init"
    source.write_text(init_program(INIT_COLOR_CALLS, damage))
    compiled = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                               str(source), "-o", str(executable)],
                              capture_output=True, text=True, timeout=45)
    assert compiled.returncode == 0, compiled.stderr
    run = subprocess.run([str(executable)], capture_output=True, text=True, timeout=15)
    assert (run.returncode == 0) == (damage is None), run.stderr


def test_header_does_not_reintroduce_pointer_aliases():
    assert "BaseButtonDefaultTextColors" not in HEADER
    assert "BaseButtonStaticDefaults" not in HEADER
    assert re.search(r"extern\s+uint32_t\s+DialogDefaults\[37\]", HEADER)
