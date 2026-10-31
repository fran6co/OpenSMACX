"""Actual callbacks against pinned table dispatch and post-set_range member reload.

Host behavior controls do not establish native ABI or linked acceptance.
"""
from pathlib import Path
import shutil
import subprocess
import pytest
ROOT=Path(__file__).resolve().parents[2]
def definition(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if not depth:
            return source[start:end + 1]
    raise AssertionError('unterminated actual definition')


HARNESS=r'''
#include <cassert>
#include <climits>
#include <string>
struct RECT { int value; };
std::string events;
struct Buffer {
 RECT working; RECT *last;
 void set_clip(RECT *r) { last=r;events+=r==&working?"C":"c"; }
 void restore_clip() { set_clip(&working); }
};
struct Win { int distinct_base[16]; };
struct GraphicWin:Win,Buffer {
 int field_C0FC_,field_C108_;
 RECT *buffer_working_rect() { return &working; }
 virtual void redraw() { events+="V"; }
 void soft_update() { events+="U"; }
};
struct Interface { int field_8_; } ReportIfGlobal;
struct Scroll {
 GraphicWin *owner;int paging_,position,lo,hi;
 void set_range(int a,int b) {
  events+="R";lo=a;hi=b;owner->field_C108_=73;owner->field_C0FC_=91;
 }
 void set_paging(int p) { paging_=p; }
 void set_pos(int p) { events+="P";position=p; }
};
struct ReportWin:GraphicWin {
 int field_C100_,field_C104_,field_E270_,field_59AC_;
 RECT energy_panel_rect_;Scroll scroll_;
 ReportWin() { scroll_.owner=this; }
 void reset() { events+="r"; }
 void draw_intel() { events+="I"; }
 void draw_labs() { events+="L"; }
 void draw_energy() { events+="E"; }
 void draw_ops() { events+="O"; }
 void draw_projects() { events+="J"; }
 void draw_sat() { events+="S"; }
 void draw_security() { events+="Q"; }
 void draw_score() { events+="T"; }
 void on_scrolled(int,int);void on_scrolling(int,int);void on_redraw();
 void UNK1(int,int,int,int);
};
ACTUAL
int main() {
 ReportWin w;
 const int pages[]={INT_MIN,-1,0,1,2,3,4,5,6,7,8,9,INT_MAX};
 const char *drawers[]={"I","","L","E","O","J","S","Q","T"};
 for(unsigned n=0;n<sizeof(pages)/sizeof(pages[0]);++n) {
  const int p=pages[n];ReportIfGlobal.field_8_=p;events.clear();w.on_redraw();
  assert(events==std::string("rc")+(p>=0&&p<=8?drawers[p]:"")+"C");
  assert(w.field_59AC_==-1&&w.last==&w.working);
  for(int mode=0;mode!=2;++mode) {
   ReportIfGlobal.field_8_=p;events.clear();
   if(mode)w.on_scrolling(42,-17);else w.on_scrolled(42,-17);
   assert(w.field_C0FC_==-17);
   const std::string expected=p==5?"rJU":p>=0&&p<=8&&p!=1?"V":"";
   assert(events==expected);
  }
 }
 events.clear();w.field_E270_=12;w.UNK1(11,-4,97,6);
 assert(events=="RP");assert(w.field_C100_==-4&&w.field_C104_==97);
 assert(w.scroll_.lo==-4&&w.scroll_.hi==97);
 assert(w.scroll_.paging_==73&&w.scroll_.position==91);assert(w.field_E270_==0);
}
'''
@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
def test_actual_report_callbacks_and_live_range_state(tmp_path, compiler):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(f'{compiler} unavailable')
    source = (ROOT / 'src/reportwin.cpp').read_text()
    signatures = ('void ReportWin::on_scrolled(', 'void ReportWin::on_scrolling(',
                  'void ReportWin::on_redraw()', 'void ReportWin::UNK1(')
    bodies = '\n'.join(definition(source, signature) for signature in signatures)

    def run(body, label):
        cpp = tmp_path / (label + '.cpp')
        binary = tmp_path / label
        cpp.write_text(HARNESS.replace('ACTUAL', body))
        subprocess.run([executable, '-std=c++11', '-O2', str(cpp), '-o', str(binary)],
                       check=True, capture_output=True, text=True)
        return subprocess.run([str(binary)], capture_output=True, text=True)

    assert run(bodies, 'actual').returncode == 0
    old = 'scroll_.set_paging(field_C108_);'
    assert old in bodies
    assert run(bodies.replace(old, 'scroll_.set_paging(page_size);', 1),
               'stale_argument').returncode != 0
    # Independent dispatch and clipping expectations must detect real damage.
    assert run(bodies.replace('draw_projects();', 'draw_labs();', 1),
               'wrong_page').returncode != 0
    assert run(bodies.replace('Buffer::restore_clip();', '', 1),
               'missing_restore').returncode != 0
