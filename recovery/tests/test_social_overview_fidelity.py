"""Actual overview body with bounded service doubles, not native layout proof.

Expected rectangles, snapshot timing and the last-description gate come from
4B0410..4B089A. The real Font selector is compiled too. Parser/render services
are doubles; these tests do not establish complete rendering or byte equality.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated body")


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
struct RECT { int left,top,right,bottom; };
struct Font { int unk_1_,height_,line_height_; int line_advance() const; };
FONT_BODY
struct SocialEffect { int values_[11]; };
struct Player { SocialEffect soc_effect_pending; } PlayersData[2];
struct Rule { const char *set1; } SocialEffects[11];
const int MaxSocialEffectNum=11;
int InterfaceBoxStyles[7];
char StringTemp[1024], line_buffer[32];
char *TextBufferGetPtr=line_buffer;
const char *AlphaxFileID="alpha", *SocialOverviewSection="overview";
const char *SocialWinOverviewSocText="SOC", *SocialWinOverviewPlusText="+";
int scenario, opened, read_count, closed, sounds, fonts, refreshes;
std::vector<RECT> boxes;
std::vector<int> ys;
std::vector<std::string> descriptions_seen;
struct SocialWin;
SocialWin *active;
struct Buffer {
 void restore_clip() {}
 void set_wrap_width(int) {}
 void box_sprite(RECT *r,int*) { boxes.push_back(*r); }
 void set_font(Font*,int,int,int) { ++fonts; }
 void set_text_color(int,int,int,int) {}
 void write_cent_l(char*,RECT *r,int) { assert(r->top==23 && r->bottom==43); }
 int wrap_height(char *s,int width) { assert(width==171); return *s ? 10 : 0; }
 void write_right_l(char*,int,int,int);
 int wrap(char *s,RECT *r) {
   assert(r->left==33 && r->right==204 && r->bottom==317);
   descriptions_seen.push_back(s); return r->top+7;
 }
} MainInterfaceGlobal;
struct SocialWin {
 int effect_hover_button_id_,effectMaximize_;
 int factionID_;
 RECT tooltipEffects_;
 SocialEffect socEffect_,socEffectOrig_;
 Font font1_,font7_;
 void soft_update(RECT *r) { assert(r==&tooltipEffects_); ++refreshes; }
 void draw_overview();
};
void Buffer::write_right_l(char*,int x,int y,int) {
 assert(x==28); ys.push_back(y);
 // A callback changes the live owner before the subsequent snapshot read.
 active->factionID_=1;
}
int social_effect_value(const SocialEffect &e,int i) { return e.values_[i]; }
char *label_get(int n) { assert(n==0x2e4); static char title[]="title"; return title; }
void do_sound() { ++sounds; }
void text_close() { ++closed; }
int text_open(const char *file,const char *section) {
 if(file) return 0;
 assert(section==StringTemp);
 ++opened; read_count=0;
 return scenario==2 && opened==3;
}
char *text_get() { ++read_count; std::strcpy(line_buffer,"line"); return 0; }
int text_item_number() { return scenario==3 && opened==1 && read_count==1 ? -100 : 100; }
char *text_item() {
 static char row[]="row", updated[]="updated", empty[]="";
 if(scenario==1 && opened==10) return empty;
 return read_count>1 ? updated : row;
}
char *_itoa(int value,char *out,int radix) { assert(radix==10); std::sprintf(out,"%d",value); return out; }
OVERVIEW_BODY
int main(int argc,char **argv) {
 scenario=argc>1 ? argv[1][0]-'0' : 0;
 SocialWin w={}; active=&w;
 w.effect_hover_button_id_=w.effectMaximize_=-1;
 w.tooltipEffects_={10,20,210,320};
 w.font7_.unk_1_=scenario==3 ? -1 : 3;
 w.font7_.height_=11; w.font7_.line_height_=17;
 for(int i=0;i<11;++i) {
  SocialEffects[i].set1="effect";
  w.socEffect_.values_[i]=i-4;
  w.socEffectOrig_.values_[i]=-999;
  PlayersData[0].soc_effect_pending.values_[i]=20+i;
  PlayersData[1].soc_effect_pending.values_[i]=100+i;
 }
 w.draw_overview();
 assert(closed==1);
 assert(boxes.size()==3);
 assert(boxes[0].top==20 && boxes[0].bottom==320);
 assert(boxes[1].top==23 && boxes[1].bottom==43);
 assert(boxes[2].top==45 && boxes[2].bottom==317);
 if(scenario==2) {
  assert(ys.empty() && !sounds && !refreshes);
  return 0;
 }
 assert(opened==10 && sounds==1 && refreshes==1 && ys.size()==10);
 assert(fonts==(scenario==1 ? 12 : 22));
 for(int i=0;i<10;++i) assert(ys[i]==(scenario==1 ? 136 : 131+i*7));
 assert(descriptions_seen.size()==(scenario==1 ? 0u : 10u));
 if(scenario==3) assert(descriptions_seen[0]=="updated");
 for(int i=0;i<11;++i) {
  assert(w.socEffect_.values_[i]==i-4);
  assert(w.socEffectOrig_.values_[i]==(i==3 ? -999 : 100+i));
 }
}
'''


def run(tmp_path, scenario, change=None):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    overview = body((ROOT / "src/socialwin.cpp").read_text(), "void SocialWin::draw_overview()")
    if change:
        before, after = change
        assert overview.count(before) == 1
        overview = overview.replace(before, after, 1)
    font = body((ROOT / "src/jackal/font.cpp").read_text(), "int Font::line_advance() const")
    source = tmp_path / "overview.cpp"
    source.write_text(HARNESS.replace("FONT_BODY", font).replace("OVERVIEW_BODY", overview))
    exe = tmp_path / "overview"
    built = subprocess.run([compiler, "-std=c++11", "-O0", str(source), "-o", str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe), str(scenario)], capture_output=True, text=True)


@pytest.mark.parametrize("scenario", [0, 1, 2, 3])
def test_actual_overview_native_layout_snapshot_and_parser(tmp_path, scenario):
    result = run(tmp_path, scenario)
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("scenario,before,after", [
    (0, "panel.top = title_rect.bottom + 2;", "panel.top = title_rect.bottom + 5;"),
    (0, "panel.top = buffer->wrap(descriptions[effect], &panel);", "panel.top = buffer->wrap(descriptions[effect], &panel) + font7_.line_advance();"),
    (1, "if (last_description_length != 0)", "if (descriptions[effect][0] != 0)"),
    (0, "socEffectOrig_.values_[effect] =", "socEffect_.values_[effect] ="),
    (0, "PlayersData[factionID_].soc_effect_pending.values_[effect];", "PlayersData[0].soc_effect_pending.values_[effect];"),
    (3, "text_get();\n            if (strlen(TextBufferGetPtr) == 0)", "char *line = text_get();\n            if (line == 0 || strlen(TextBufferGetPtr) == 0)"),
])
def test_controls_reject_wrong_recovery(tmp_path, scenario, before, after):
    assert run(tmp_path, scenario, (before, after)).returncode != 0
