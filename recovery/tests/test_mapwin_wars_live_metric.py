"""Actual diplomacy-key body and callback metric phases, not target ABI proof."""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_statuswin_draw_active_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]


def product(old_cache=False):
    owner = (ROOT / "src/mapwin.cpp").read_text()
    method = extract_function(owner, "void MapWin::draw_wars()")
    if old_cache:
        original = method
        method = method.replace("int y = font_line_advance(*font);",
                                "const int saved_metric = font_line_advance(*font);\n"
                                "    int y = saved_metric;", 1)
        method = method.replace("y += font_line_advance(*font);", "y += saved_metric;")
        method = method.replace("y = font_line_advance(*font);", "y = saved_metric;")
        assert method != original
    header = (ROOT / "src/jackal/font.h").read_text()
    accessors = "\n".join(extract_function(header, signature) for signature in
                          ("int pixel_height() const", "int line_height() const"))
    helper = extract_function((ROOT / "src/jackal/font_metrics.h").read_text(),
                              "inline int font_line_advance(")
    return HARNESS.replace("ACCESSORS", accessors).replace("HELPER", helper).replace("METHOD", method)


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <vector>
int mode, calls, reads[3], GenderDefault, PluralityDefault;
struct ReadWord {
 int value, slot;
 ReadWord(int v, int s): value(v), slot(s) {}
 operator int() const { ++reads[slot]; return value; }
};
struct Font {
 ReadWord unk_1_, height_, line_height_;
 Font(): unk_1_(0,0), height_(10,1), line_height_(7,2) {}
 ACCESSORS
 int width(const char *s) { assert(!std::strcmp(s,"FUNGUS")); return 5; }
};
HELPER
struct Player { int noun_gender, is_noun_plural; const char *noun_faction; };
Player Players[8];
struct Relations { uint32_t diplo_treaties[8]; } PlayersData[8];
const int MaxPlayerNum=8;
int InfoFactionTextColors[8];
char StringTemp[128];
struct Buffer {
 void set_font(Font *, int, int, int);
 void set_text_color(int, int, int, int) {}
 int write_l(char *, int, int, int);
};
struct MapWin : Buffer { Font font_b_; void draw_wars(); };
MapWin *active;
std::vector<int> xs, ys;
void Buffer::set_font(Font *font, int a, int b, int c) {
 assert(font==&active->font_b_ && !a && !b && !c);
 font->height_.value=20;
 font->line_height_.value=20;
 if(mode) font->unk_1_.value=-1;
}
int Buffer::write_l(char *text, int x, int y, int length) {
 assert(length==static_cast<int>(std::strlen(text)));
 assert(!std::strcmp(text, calls<7 ? "A: " : "T"));
 xs.push_back(x); ys.push_back(y); ++calls;
 // Drawing callbacks can mutate the owner font between original metric reads.
 ++active->font_b_.height_.value;
 ++active->font_b_.line_height_.value;
 return calls<8 ? 7+calls : 0;
}
METHOD
int main(int argc,char **argv) {
 assert(argc==2); mode=std::atoi(argv[1]);
 MapWin window; active=&window;
 for(int row=0;row<8;++row) {
  Players[row].noun_faction="A";
  for(int col=0;col<8;++col) PlayersData[row].diplo_treaties[col]=10;
 }
 window.draw_wars();
 assert(calls==56);
 // Pinned4698EF starts y;4699C9 advances labels after write_l;
 // 469A06 resets matrix y;469AF3 advances each completed matrix row.
 const int labels[]={10,31,53,76,100,125,151};
 const int matrix[]={27,61,102,150,205,267,336};
 for(int i=0;i<7;++i) { assert(xs[i]==5); assert(ys[i]==labels[i]); }
 for(int row=0;row<7;++row) for(int col=0;col<7;++col) {
  const int i=7+7*row+col;
  assert(xs[i]==14+5*col); assert(ys[i]==matrix[row]);
 }
 assert(reads[0]==16);
 assert(reads[1]==(mode ? 1 : 16));
 assert(reads[2]==(mode ? 15 : 0));
}
'''


def build(tmp_path, compiler, old_cache=False):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "wars.cpp"
    source.write_text(product(old_cache))
    output = tmp_path / "wars"
    result = subprocess.run([executable, "-std=c++17", "-O0", str(source), "-o", str(output)],
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return output


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_metric_reads_follow_render_callbacks(tmp_path, compiler):
    executable = build(tmp_path, compiler)
    for mode in (0, 1):
        result = subprocess.run([str(executable), str(mode)], capture_output=True, text=True)
        assert result.returncode == 0, result.stderr


def test_old_cached_metric_mutant_compiles_and_fails_positions(tmp_path):
    executable = build(tmp_path, "g++", old_cache=True)
    for mode in (0, 1):
        result = subprocess.run([str(executable), str(mode)], capture_output=True, text=True)
        assert result.returncode != 0
