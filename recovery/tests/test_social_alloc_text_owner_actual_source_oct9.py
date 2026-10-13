"""Execute the three live SocialWin allocation renderers with typed fixtures."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = (ROOT / "src/socialwin.cpp").read_text()


def actual_function(name: str) -> str:
    marker = f"void SocialWin::{name}() {{"
    start = SOURCE.index(marker)
    opening = SOURCE.index("{", start)
    depth = 0
    for end in range(opening, len(SOURCE)):
        depth += (SOURCE[end] == "{") - (SOURCE[end] == "}")
        if depth == 0:
            return SOURCE[start:end + 1]
    raise AssertionError(f"unterminated actual source function {name}")


def percent_owner() -> str:
    marker = "static const char PercentText[] = \"%\";"
    start = SOURCE.index(marker)
    return SOURCE[start:SOURCE.index("\n", start)]


CPP_PREFIX = r'''#include <cassert>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

struct RECT { int32_t left, top, right, bottom; };
struct Sprite { int w=8, h=10; int width() const { return w; }
  int height() const { return h; } };
struct PlayerData { int se_alloc_psych=0, se_alloc_labs=0; };
PlayerData PlayersData[8];
char StringTemp[256];
__PERCENT_OWNER__
const char BufferJoinSpace[] = " ";
int InterfaceBoxStyles[7] = {0,1,2,3,4,5,6};
int font1_;
std::vector<std::string> events;
std::vector<std::string> tile_calls;
std::string drawn_text;
bool mutate_alloc_on_label;

void event(const char *name) { events.emplace_back(name); }
struct Buffer {
  int id;
  explicit Buffer(int value=0):id(value) {}
  void restore_clip() { event("restore"); }
  void box_sprite(RECT *r, int *style) {
    (void)r; assert(style == &InterfaceBoxStyles[6]); event("box_sprite");
  }
  void box(RECT *r, int a, int b) {
    (void)r; assert(a==0xbc && b==0xbc); event("box");
  }
  void set_font(int *, int a, int b, int c) {
    assert(a==0 && b==0 && c==0); event("font");
  }
  void set_text_color(int a, int b, int c, int d) {
    assert(a==0x9f && b==-1 && c==1 && d==1); event("color");
  }
  void write_cent_l(char *s, RECT *r, int n) {
    (void)r; assert(n == static_cast<int>(std::strlen(s)));
    drawn_text = s; event("write");
  }
  void tile(Buffer *texture, int sx, int sy, int dx, int dy, int w, int h) {
    char row[160];
    std::snprintf(row, sizeof(row), "%d:%d,%d,%d,%d,%d,%d",
                  texture->id,sx,sy,dx,dy,w,h);
    tile_calls.emplace_back(row); event("tile");
  }
  void soft_update(RECT *r) { (void)r; event("update"); }
};
Buffer MainInterfaceGlobal;

char *label_get(int id) {
  char row[24]; std::snprintf(row,sizeof(row),"label%d",id); event(row);
  if (mutate_alloc_on_label) {
    PlayersData[0].se_alloc_psych=8; PlayersData[0].se_alloc_labs=5;
    mutate_alloc_on_label=false;
  }
  if (id==318) return const_cast<char *>("Psych");
  if (id==366) return const_cast<char *>("Economy");
  if (id==60) return const_cast<char *>("Labs");
  return const_cast<char *>("wrong");
}
char *_itoa(int value, char *out, int radix) {
  assert(radix==10); char row[32];
  std::snprintf(row,sizeof(row),"itoa%d",value); event(row);
  std::snprintf(out,80,"%d",value); return out;
}
void CharUpperA(char *s) {
  event("upper"); for (;*s;++s) *s=static_cast<char>(std::toupper(
      static_cast<unsigned char>(*s)));
}
void do_sound() { event("sound"); }

struct SocialWin {
  uint32_t factionID_=0;
  RECT tooltipPsych_{10,20,210,80};
  RECT tooltipEconomy_{30,40,230,100};
  RECT tooltipLabs_{50,60,250,120};
  Sprite energyAllocLock_[2];
  Buffer energyAllocSlider_[6] = {Buffer(0),Buffer(1),Buffer(2),
                                  Buffer(3),Buffer(4),Buffer(5)};
  int font1_=0;
  void draw_psych_alloc();
  void draw_economy_alloc();
  void draw_labs_alloc();
};
'''

CPP_MAIN = r'''
void reset_case() {
  std::memset(PlayersData,0,sizeof(PlayersData)); StringTemp[0]='\0';
  events.clear(); tile_calls.clear(); drawn_text.clear(); mutate_alloc_on_label=false;
}
void expect_order(std::initializer_list<const char *> expected) {
  assert(events.size()==expected.size()); size_t i=0;
  for (const char *name: expected) assert(events[i++]==name);
}
int main() {
  SocialWin w;

  // Psych: 2/10 is rendered as 20%; the source uses the native ten-unit width scale and
  // its two ordered tile calls are observable, not replaced by a percent bar.
  reset_case(); PlayersData[0].se_alloc_psych=2; mutate_alloc_on_label=true;
  w.draw_psych_alloc();
  assert(drawn_text=="PSYCH 20%" && std::strcmp(StringTemp,drawn_text.c_str())==0);
  assert(PlayersData[0].se_alloc_psych==8);
  assert(tile_calls.size()==2);
  assert(tile_calls[0]=="4:58,67,58,67,149,10");
  assert(tile_calls[1]=="5:21,67,21,67,38,-24");
  expect_order({"restore","box_sprite","box","label318","itoa20",
    "upper","font","color","write","tile","tile","update","sound"});

  // Zero and a signed out-of-domain value retain native formatting/arithmetic;
  // this harness deliberately does not turn them into clamped allocations.
  reset_case(); PlayersData[0].se_alloc_psych=0; w.draw_psych_alloc();
  assert(drawn_text=="PSYCH 0%" && tile_calls.size()==1);
  assert(tile_calls[0]=="4:21,67,21,67,186,10");
  reset_case(); PlayersData[0].se_alloc_psych=5; w.draw_psych_alloc();
  assert(drawn_text=="PSYCH 50%" && tile_calls.size()==2);
  assert(tile_calls[0]=="4:114,67,114,67,93,10");
  assert(tile_calls[1]=="5:21,67,21,67,94,-24");
  reset_case(); PlayersData[0].se_alloc_psych=10; w.draw_psych_alloc();
  assert(drawn_text=="PSYCH 100%" && tile_calls.size()==1);
  assert(tile_calls[0]=="5:21,67,21,67,163,-24");
  reset_case(); PlayersData[0].se_alloc_psych=-1; w.draw_psych_alloc();
  assert(drawn_text=="PSYCH -10%" && tile_calls.size()==1);
  assert(tile_calls[0]=="4:3,67,3,67,204,10");

  // Economy subtracts psych, then labs, and divides the signed remainder by 10.
  reset_case(); PlayersData[0].se_alloc_psych=2; PlayersData[0].se_alloc_labs=3;
  mutate_alloc_on_label=true;
  w.draw_economy_alloc();
  assert(drawn_text=="ECONOMY 50%" && tile_calls.size()==2);
  assert(PlayersData[0].se_alloc_psych==8 && PlayersData[0].se_alloc_labs==5);
  assert(tile_calls[0]=="0:134,87,134,87,93,10");
  assert(tile_calls[1]=="1:41,87,41,87,94,10");
  expect_order({"restore","box_sprite","box","label366","itoa50",
    "upper","font","color","write","tile","tile","update","sound"});
  reset_case(); PlayersData[0].se_alloc_psych=8; PlayersData[0].se_alloc_labs=5;
  w.draw_economy_alloc();
  assert(drawn_text=="ECONOMY -30%" && tile_calls.size()==1);

  // Labs uses its own allocation and slider pair; zero and fractional shares
  // follow the same original order through update and sound.
  reset_case(); PlayersData[0].se_alloc_labs=3; mutate_alloc_on_label=true;
  w.draw_labs_alloc();
  assert(drawn_text=="LABS 30%" && tile_calls.size()==2);
  assert(PlayersData[0].se_alloc_labs==5);
  assert(tile_calls[0]=="2:116,107,116,107,131,10");
  assert(tile_calls[1]=="3:61,107,61,107,56,10");
  expect_order({"restore","box_sprite","box","label60","itoa30",
    "upper","font","color","write","tile","tile","update","sound"});
  reset_case(); PlayersData[0].se_alloc_labs=0; w.draw_labs_alloc();
  assert(drawn_text=="LABS 0%" && tile_calls.size()==1);
  return 0;
}
'''


def build_and_run(tmp_path: Path, bodies: dict[str, str], owner: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "social_alloc_actual_source.cpp"
    executable = tmp_path / "social_alloc_actual_source"
    rendered = CPP_PREFIX.replace("__PERCENT_OWNER__", owner)
    rendered += "\n" + "\n".join(bodies[name] for name in (
        "draw_psych_alloc", "draw_economy_alloc", "draw_labs_alloc"))
    rendered += CPP_MAIN
    source.write_text(rendered)
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True,
                   text=True, timeout=10)


def test_actual_three_renderers_use_typed_percent_owner_and_preserve_behavior(tmp_path):
    assert "static int *const PercentText" not in SOURCE
    assert "reinterpret_cast<char *>(PercentText)" not in SOURCE
    assert SOURCE.count("strcat(StringTemp, PercentText);") == 3
    assert "ItoaFn" not in SOURCE
    assert "reinterpret_cast<ItoaFn>(_itoa)" not in SOURCE
    bodies = {name: actual_function(name) for name in (
        "draw_psych_alloc", "draw_economy_alloc", "draw_labs_alloc")}
    for body in bodies.values():
        assert "_itoa(" in body and "reinterpret_cast<ItoaFn>" not in body
    build_and_run(tmp_path, bodies, percent_owner())


@pytest.mark.parametrize("name, old, new", [
    ("draw_psych_alloc", "strcat(StringTemp, PercentText);",
     "strcat(StringTemp, \"\");"),
    ("draw_psych_alloc", "label_get(318)", "label_get(319)"),
    ("draw_psych_alloc", "allocation * 10", "allocation"),
    ("draw_psych_alloc", "int scaled = (avail_width * allocation) / 10;",
     "int scaled = (avail_width * allocation) / 5;"),
    ("draw_economy_alloc", "10 - player.se_alloc_psych - player.se_alloc_labs",
     "10 - player.se_alloc_psych + player.se_alloc_labs"),
    ("draw_labs_alloc", "CharUpperA(StringTemp);", "/* uppercase omitted */"),
    ("draw_labs_alloc", "    do_sound();", "    /* sound omitted */"),
])
def test_actual_source_controls_reject_semantic_mutants(tmp_path, name, old, new):
    bodies = {function: actual_function(function) for function in (
        "draw_psych_alloc", "draw_economy_alloc", "draw_labs_alloc")}
    assert old in bodies[name]
    bodies[name] = bodies[name].replace(old, new, 1)
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "social_alloc_mutant.cpp"
    executable = tmp_path / "social_alloc_mutant"
    rendered = CPP_PREFIX.replace("__PERCENT_OWNER__", percent_owner())
    rendered += "\n" + "\n".join(bodies[function] for function in (
        "draw_psych_alloc", "draw_economy_alloc", "draw_labs_alloc"))
    rendered += CPP_MAIN
    source.write_text(rendered)
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    result = subprocess.run([str(executable)], capture_output=True, text=True,
                            timeout=10)
    assert result.returncode != 0, f"mutant survived: {name}: {old} -> {new}"
