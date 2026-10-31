"""Actual palette bubbles and picker selection read-boundary controls.

The six original table entries remain distinct although4/5 share a label.
Host services observe calls and mutate live state; this is not GUI/ABI proof.
"""
from pathlib import Path
import shutil
import subprocess
import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]


def definition(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError('unterminated actual body')


HARNESS = r'''
#include <cassert>
#include <string>
#include <vector>
#include <utility>
struct RECT { int left,top,right,bottom; };
std::string events;
int category,spot_result,expected_label;
char first_text[]="retained",later_text[]="changed";
char *lookup_output;
struct Tutorial { void reset() { events+="T"; } } TutorialWindow;
struct LabelTable { char *strings_ptr[1024]; } Labels;
struct Strings {
 char *get(char *label) {
  events+="L";assert(label==Labels.strings_ptr[expected_label]);return lookup_output;
 }
} StringTable;
struct Spot {
 int check(int x,int y,int *position,int *type) {
  events+="S";assert(x==10&&y==20);*position=-17;*type=category;return spot_result;
 }
};
struct DesignWin {
 Spot spot2_;
 void client_to_screen(int *x,int *y) {
  events+="C";*x+=100;*y-=200;lookup_output=later_text;
 }
 void set_bubble_text(char *text,RECT *r) {
  events+="B";assert(text==first_text);
  assert(r->left==110&&r->right==110&&r->top==-180&&r->bottom==-180);
 }
 void bubble_me_elmo2(char *,int,int);
 void on_right_click(int,int);
};
struct SelectPartWin {
 int field_2B6C_,field_2B70_,field_2B60_,field_2B74_,field_2B80_,field_4648_;
 bool mutate_high,mutate_low,mutate_update;
 int updates;
 std::vector<std::pair<int,int> > draws;
 SelectPartWin():field_2B6C_(2),field_2B70_(2),field_2B60_(0),field_2B74_(4),
  field_2B80_(1),field_4648_(17),mutate_high(false),mutate_low(false),
  mutate_update(false),updates(0) {}
 int draw_part(int id,int selected) {
  draws.push_back(std::make_pair(id,selected));
  if(selected&&mutate_high)field_2B80_=9;
  if(!selected&&mutate_low){field_2B80_=44;field_4648_=80;}
  return 57;
 }
 virtual void on_update() {
  ++updates;if(mutate_update)field_4648_=88;
 }
 void on_mouse_move(int,int);
};
ACTUAL
void bubbles() {
 char labels[5][2];
 for(int i=0;i<5;++i)Labels.strings_ptr[0x335+i]=labels[i];
 const int ids[]={0x335,0x336,0x337,0x338,0x339,0x339};
 for(category=-2;category<=7;++category) {
  for(int result=-1;result<=0;++result) {
   DesignWin w;spot_result=result;events.clear();lookup_output=first_text;
   expected_label=(category>=0&&category<=5)?ids[category]:0;
   w.on_right_click(10,20);
   assert(events==(result!=-1&&category>=0&&category<=5?"TSLCB":"TS"));
  }
 }
}
void selection() {
 {
  SelectPartWin w;w.on_mouse_move(4,4);
  assert((w.draws==std::vector<std::pair<int,int> >{{0,1},{1,0}}));
  assert(w.field_2B80_==0&&w.field_4648_==57&&w.updates==1);
 }
 {
  SelectPartWin w;w.mutate_high=true;w.mutate_update=true;w.on_mouse_move(4,4);
  assert((w.draws==std::vector<std::pair<int,int> >{{0,1},{9,0}}));
  assert(w.field_2B80_==0&&w.field_4648_==88&&w.updates==1);
 }
 {
  SelectPartWin w;w.field_2B80_=0;w.on_mouse_move(4,4);
  assert(w.draws.empty()&&w.updates==0&&w.field_4648_==17);
 }
 {
  SelectPartWin w;w.on_mouse_move(3,4);
  assert((w.draws==std::vector<std::pair<int,int> >{{1,0}}));
  assert(w.field_2B80_==-1&&w.field_4648_==0&&w.updates==0);
 }
 {
  SelectPartWin w;w.field_2B74_=1;w.field_2B80_=0;w.mutate_low=true;
  w.on_mouse_move(112,4);
  assert((w.draws==std::vector<std::pair<int,int> >{{0,0}}));
  assert(w.field_2B80_==-1&&w.field_4648_==0&&w.updates==0);
 }
 {
  SelectPartWin w;w.field_2B60_=-5;w.on_mouse_move(4,4);
  assert((w.draws==std::vector<std::pair<int,int> >{{-5,1},{1,0}}));
  assert(w.field_2B80_==-5&&w.field_4648_==57&&w.updates==1);
 }
 {
  SelectPartWin w;w.field_2B80_=-1;w.on_mouse_move(300,4);
  assert(w.draws.empty()&&w.field_4648_==0&&w.field_2B80_==-1&&w.updates==0);
 }
}
int main() { bubbles();selection(); }
'''


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
def test_actual_bubble_dispatch_and_picker_read_boundaries(tmp_path, compiler):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + ' unavailable')
    path = ROOT / 'src/designwin.cpp'
    records = read_file(path)
    for address in (0x439670, 0x439700, 0x433820):
        assert len([row for row in records if row.address == address]) == 1
    source = path.read_text()
    bodies = '\n'.join(definition(source, signature) for signature in (
        'void DesignWin::bubble_me_elmo2(', 'void DesignWin::on_right_click(',
        'void SelectPartWin::on_mouse_move('))

    def run(candidate, name):
        cpp = tmp_path / (name + '.cpp')
        binary = tmp_path / name
        cpp.write_text(HARNESS.replace('ACTUAL', candidate))
        built = subprocess.run([executable, '-std=c++11', '-O2', str(cpp), '-o', str(binary)],
                               capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(binary)], capture_output=True, text=True)

    assert run(bodies, 'actual').returncode == 0
    mutants = {
        'wrong_ability_label': ('Labels.strings_ptr[0xCE4 / 4]', 'Labels.strings_ptr[0xCE0 / 4]'),
        'swapped_coordinates': ('client_to_screen(&a2, &a3);', 'client_to_screen(&a3, &a2);'),
        'lost_label_snapshot': ('set_bubble_text(a1, &anchor);', 'set_bubble_text(lookup_output, &anchor);'),
        'stale_selection_after_draw': ('draw_part(field_2B80_, 0);', 'draw_part(previous, 0);'),
    }
    for name, (before, after) in mutants.items():
        assert before in bodies
        assert run(bodies.replace(before, after, 1), name).returncode != 0
