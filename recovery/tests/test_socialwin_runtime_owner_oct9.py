"""Actual-source controls for SocialWin's typed runtime receivers."""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced_statement(source: str, marker: str) -> str:
    start = source.index(marker)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated statement: {marker}")


SOURCE = (ROOT / "src/socialwin.cpp").read_text()


def test_socialwin_canvas_uses_real_main_interface_buffer_and_restores_clip():
    assert "InterfaceBuffer" not in SOURCE
    assert "InterfaceClipRect" not in SOURCE
    assert "reinterpret_cast<Buffer *>(0x007AEC64)" not in SOURCE
    for name in ("on_redraw", "draw_psych_alloc", "draw_economy_alloc",
                 "draw_labs_alloc", "draw_overview", "draw_mouseover",
                 "draw_info"):
        body = braced_statement(SOURCE, f"SocialWin::{name}(")
        if name in ("on_redraw", "draw_psych_alloc", "draw_economy_alloc",
                    "draw_labs_alloc"):
            assert "MainInterfaceGlobal.Buffer::restore_clip()" in body
            assert "MainInterfaceGlobal.Buffer::box_sprite(" in body
        else:
            assert "static_cast<Buffer *>(&MainInterfaceGlobal)" in body
    for name in ("on_redraw", "draw_psych_alloc", "draw_economy_alloc",
                 "draw_labs_alloc", "draw_overview", "draw_mouseover"):
        body = braced_statement(SOURCE, f"SocialWin::{name}(")
        assert "restore_clip()" in body


def test_language_branch_uses_source_owned_word():
    assert "extern uint32_t Language;" in SOURCE
    assert "0x009BC054" not in SOURCE
    assert SOURCE.count("if (Language != 2)") == 1


def test_map_window_virtual_base_and_null_argument_are_typed(tmp_path):
    start = SOURCE.index("            MapWin *window = MapWinTable[0];")
    end = SOURCE.index("                0, 1, -1, -1);", start) + len(
        "                0, 1, -1, -1);"
    )
    selected_window = SOURCE[start:end]
    assert "reinterpret_cast" not in selected_window

    harness = r"""
#include <cassert>
#include <cstring>
#include <cstdlib>
struct Buffer { int id; };
struct GraphicWin : Buffer {};
struct MapWin : virtual GraphicWin {};
MapWin *MapWinTable[8];
char SocialWinTutorialName[]="SOCWIN";
int calls;
GraphicWin *received;
int received_a, received_b;
struct Tutorial {
  void tut_win(GraphicWin *window, const char *title, int a, int b, int, int, int, int) {
    ++calls; received=window; received_a=a; received_b=b;
    if (title != SocialWinTutorialName || std::strcmp(title, "SOCWIN") != 0) std::abort();
  }
} TutorialWindow;
void run(int out_a, int out_b) {
__SELECTION__
}
int main() {
  MapWinTable[0]=0; run(3, 5); assert(calls==1 && received==0);
  MapWin map; GraphicWin *expected=static_cast<GraphicWin *>(&map);
  MapWinTable[0]=&map; run(7, 11);
  assert(calls==2 && received==expected && received_a==7 && received_b==11);
}
""".replace("__SELECTION__", selected_window)
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "socialwin_map_receiver.cpp"
    executable = tmp_path / "socialwin_map_receiver"
    source.write_text(harness)
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True,
                   text=True, timeout=10)


PANEL_LIFETIME_FIXTURE = r'''
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <vector>
struct RECT { int left, top, right, bottom; };
bool same(RECT a, RECT b) {
    return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom;
}
std::vector<RECT> framed, boxed;
std::vector<const RECT *> framed_at;
RECT *mutate_panel;
struct Font {};
struct Buffer {
    int identity;
    Buffer(int value=2):identity(value){}
    void restore_clip() { assert(identity==1); *mutate_panel=RECT{1000,2000,3000,4000}; }
    void set_clip(RECT *) {}
    void box_sprite(RECT *r,int *) { framed.push_back(*r); framed_at.push_back(r); }
    void box(RECT *r,int,int) { assert(identity==1); boxed.push_back(*r); }
    void set_font(Font *,Font *,Font *,int) {}
    void set_text_color(int,int,int,int) {}
    int write_cent_l(char *,RECT *,int) { return 0; }
    void tile(Buffer *,int,int,int,int,int,int) {}
};
struct MainInterface:Buffer {
    MainInterface():Buffer(1){}
    void soft_update(RECT *) {}
} MainInterfaceGlobal;
struct GraphicWin:Buffer {};
struct Sprite {
    int width() const { return 5; }
    int height() const { return 4; }
    int transparent_index() const { return 0; }
    void draw(Buffer *,int,int,int,int,int) { assert(false); }
};
const int MaxSocialEffectNum=11;
struct SocialEffect { int values_[11]; };
struct PlayerData { int se_alloc_psych,se_alloc_labs; SocialEffect soc_effect_base; } PlayersData[2];
struct Player { int noun_gender,is_noun_plural; char noun_faction[16]; } Players[2];
char StringTemp[4096],BufferJoinSpace[]=" ",PercentText[]="%";
int InterfaceBoxStyles[7],GenderDefault,PluralityDefault;
char *label_get(int) { static char text[]="allocation";return text; }
void CharUpperA(char *) {}
char *_itoa(int value,char *out,int base) { assert(base==10);std::sprintf(out,"%d",value);return out; }
void do_sound() {}
struct SocialWin:GraphicWin {
    unsigned int factionID_;
    Font font1_;
    RECT tooltipPsych_,tooltipEconomy_,tooltipLabs_,tooltipFactionPower_;
    Sprite energyAllocLock_[3],effectIcons_[126];
    Buffer energyAllocSlider_[6];
    void soft_update(RECT *) {}
    void draw_psych_alloc();void draw_economy_alloc();
    void draw_labs_alloc();void draw_faction_basal();
};
ACTUAL_OPERATIONS
ACTUAL_BODIES
int main() {
    SocialWin win;
    win.factionID_=0;
    PlayersData[0].se_alloc_psych=3;PlayersData[0].se_alloc_labs=2;
    std::strcpy(Players[0].noun_faction,"faction");
    for (int which=0;which<3;++which) {
        framed.clear();boxed.clear();framed_at.clear();
        RECT *member=which==0?&win.tooltipPsych_:which==1?&win.tooltipLabs_:&win.tooltipEconomy_;
        *member=RECT{10,20,110,80};mutate_panel=member;
        if(which==0)win.draw_psych_alloc();else if(which==1)win.draw_labs_alloc();else win.draw_economy_alloc();
        assert(framed.size()==1 && same(framed[0],RECT{10,20,110,80}));
        assert(boxed.size()==1 && same(boxed[0],RECT{36,23,84,43}));
        assert(same(*member,RECT{1000,2000,3000,4000}));
    }
    framed.clear();boxed.clear();framed_at.clear();
    win.tooltipFactionPower_=RECT{10,20,110,80};
    win.draw_faction_basal();
    assert(framed.size()==3);
    assert(same(framed[0],RECT{10,20,110,80}));
    assert(same(framed[1],RECT{13,23,107,43}));
    assert(same(framed[2],RECT{13,57,107,77}));
    assert(framed_at[1]==framed_at[2]);
}
'''


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "psych_snapshot", "labs_snapshot",
                                    "economy_snapshot", "title_height"])
def test_actual_panel_snapshots_and_faction_title(tmp_path, compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    names = ("draw_psych_alloc", "draw_labs_alloc", "draw_economy_alloc",
             "draw_faction_basal")
    bodies = {name: braced_statement(SOURCE, f"void SocialWin::{name}()") for name in names}
    if damage is not None and damage.endswith("_snapshot"):
        kind = damage.removesuffix("_snapshot")
        name = f"draw_{kind}_alloc"
        local = "local_rect" if kind == "psych" else "panel"
        member = {"psych": "tooltipPsych_", "labs": "tooltipLabs_",
                  "economy": "tooltipEconomy_"}[kind]
        before = f"    RECT {local} = {member};\n    MainInterfaceGlobal.Buffer::restore_clip();"
        after = f"    MainInterfaceGlobal.Buffer::restore_clip();\n    RECT {local} = {member};"
        assert bodies[name].count(before) == 1
        bodies[name] = bodies[name].replace(before, after, 1)
    elif damage == "title_height":
        before = "title.bottom = title.top + 0x14;"
        assert bodies["draw_faction_basal"].count(before) == 1
        bodies["draw_faction_basal"] = bodies["draw_faction_basal"].replace(
            before, "title.bottom = title.top + 0x17;", 1)
    operations = "\n".join(braced_statement(SOURCE, signature) for signature in (
        "static inline RECT *social_expand_rect(",
        "static int social_effect_value(const SocialEffect &effect, int index) {",
    ))
    fixture = PANEL_LIFETIME_FIXTURE.replace("ACTUAL_OPERATIONS", operations)
    fixture = fixture.replace("ACTUAL_BODIES", "\n\n".join(bodies.values()))
    source = tmp_path / "socialwin-panels.cpp"
    executable = tmp_path / "socialwin-panels"
    source.write_text(fixture)
    built = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra",
                            str(source), "-o", str(executable)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)
    assert (ran.returncode == 0) == (damage is None), ran.stdout + ran.stderr


def test_language_branch_actual_source_statement(tmp_path):
    statement = braced_statement(SOURCE, "if (Language != 2)")
    harness = r"""
unsigned int Language;
int uppercase_calls;
char StringTemp[8];
void CharUpperA(char *) { ++uppercase_calls; }
void run() { __LANGUAGE_BRANCH__ }
int main() {
  Language=2; run(); if (uppercase_calls != 0) return 1;
  Language=1; run(); if (uppercase_calls != 1) return 2;
  Language=0; run(); return uppercase_calls == 2 ? 0 : 3;
}
""".replace("__LANGUAGE_BRANCH__", statement)
    compiler = shutil.which("c++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = tmp_path / "socialwin_language.cpp"
    executable = tmp_path / "socialwin_language"
    source.write_text(harness)
    subprocess.run([compiler, "-std=c++17", str(source), "-o", str(executable)],
                   check=True, capture_output=True, text=True, timeout=30)
    subprocess.run([str(executable)], check=True, capture_output=True,
                   text=True, timeout=10)
