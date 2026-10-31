"""Actual-source keyboard and coordinate callback controls, with pinned table evidence."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]


def body(signature, address):
    path = ROOT / 'src/setupwin.cpp'
    record, = (r for r in read_file(path) if r.address == address)
    assert record.image_spans
    text = path.read_text()
    start = text.index(signature)
    opening = text.index('{', start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == '{') - (text[end] == '}')
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError('unclosed actual method')


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <vector>
struct Event { int kind, row; uint32_t selected; };
struct Spot {
 int result, incoming_x, incoming_y;
 int check(int x, int y, int *position, int *) {
  incoming_x=x; incoming_y=y; *position=777; return result;
 }
};
struct SetupWin {
 uint32_t selected_row_, row_count_;
 std::vector<Event> events;
 Spot spot_;
 void release_modal() { events.push_back(Event{3,0,selected_row_}); }
 void on_mouse_move(int x,int y) { events.push_back(Event{4,y,static_cast<uint32_t>(x)}); }
 int draw_item(int row,int selected) {
  events.push_back(Event{selected,row,selected_row_});
  if (!selected) { selected_row_=999; row_count_=42; }
  return 0;
 }
 void soft_update() { events.push_back(Event{2,0,selected_row_}); }
 int on_key_click(int,int);
 void on_left_click(int,int);
};
// ACTUAL_METHODS
int main() {
 for (int count=0;count<=3;++count) {
  for (int old=-1;old<=6;++old) {
   for (int key=-2;key<128;++key) {
    SetupWin w; w.selected_row_=old; w.row_count_=count;
    int answer=w.on_key_click(27,key);
    if(key==38 || key==40) {
     int next= key==38 ? (old==0 ? count-1:old-1) : (old==count-1 ? 0:old+1);
     assert(answer==1 && w.events.size()==3);
     assert(w.events[0].kind==0 && w.events[0].row==old);
     assert(w.events[0].selected==static_cast<uint32_t>(old));
     assert(w.events[1].kind==1 && w.events[1].row==next);
     assert(w.events[1].selected==static_cast<uint32_t>(next));
     assert(w.events[2].kind==2 && w.selected_row_==static_cast<uint32_t>(next));
    } else if(key==13 || key==27) {
     assert(answer==1 && w.events.size()==1 && w.events[0].kind==3);
     assert(w.selected_row_==static_cast<uint32_t>(key==27 ? -1:old));
     assert(w.events[0].selected==w.selected_row_);
    } else {
     assert(answer==0 && w.events.empty());
     assert(w.selected_row_==static_cast<uint32_t>(old));
    }
   }
  }
 }
 for(int hit=-1;hit<=1;++hit) {
  SetupWin w; w.spot_.result=hit; w.selected_row_=5;
  w.on_left_click(11,23);
  assert(w.spot_.incoming_x==11 && w.spot_.incoming_y==23);
  if(hit==-1) assert(w.events.empty());
  else {
   assert(w.events.size()==2 && w.events[0].kind==4 && w.events[0].row==23);
   assert(w.events[0].selected==11 && w.events[1].kind==3);
  }
 }
}
'''


@pytest.mark.parametrize('damage', ['none', 'lost_escape', 'wrong_wrap', 'clobbered_coordinate'])
def test_keyboard_mapping_and_callbacks_preserve_original_state(tmp_path, damage):
    key=body('int SetupWin::on_key_click(',0x4AE3E0)
    click=body('void SetupWin::on_left_click(',0x4AE340)
    if damage=='lost_escape':
        old='selected_row_ = static_cast<uint32_t>(-1);'
        assert key.count(old)==1
        key=key.replace(old,'')
    elif damage=='wrong_wrap':
        old='current == 0 ? static_cast<int>(row_count_) - 1 : current - 1'
        assert key.count(old)==1
        key=key.replace(old,'current == 0 ? 0 : current - 1')
    elif damage=='clobbered_coordinate':
        old='on_mouse_move(x, original_y);'
        assert click.count(old)==1
        click=click.replace(old,'on_mouse_move(x, y);')
    compiler=shutil.which('clang++') or shutil.which('g++')
    assert compiler
    cpp=tmp_path/'actual.cpp';exe=tmp_path/'actual'
    cpp.write_text(HARNESS.replace('// ACTUAL_METHODS',key+'\n'+click))
    built=subprocess.run([compiler,'-std=c++11','-fsanitize=address,undefined',
                          '-fno-sanitize-recover=all',str(cpp),'-o',str(exe)],text=True,capture_output=True)
    assert built.returncode==0,built.stdout+built.stderr
    result=subprocess.run([str(exe)],text=True,capture_output=True,
                          env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})
    assert (result.returncode==0)==(damage=='none'),result.stdout+result.stderr


def test_pinned_dispatch_table_has_five_targets_and_28_selector_cells():
    import struct
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    pin='01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
    path=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
    if not path.is_file():pytest.skip('pinned image unavailable')
    image=PeImage(path.read_bytes(),pin)
    assert struct.unpack('<5I',image.read(Span(0x4AE4A0,0x4AE4B4)))==(
        0x4AE480,0x4AE461,0x4AE405,0x4AE41D,0x4AE495)
    indices=image.read(Span(0x4AE4B4,0x4AE4D0))
    assert list(indices)==[{13:0,27:1,38:2,40:3}.get(key,4) for key in range(13,41)]
    assert all(address in image.relocations for address in range(0x4AE4A0,0x4AE4B4,4))
