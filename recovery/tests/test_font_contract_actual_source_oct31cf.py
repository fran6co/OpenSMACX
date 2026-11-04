"""Actual metric/client snippets and damage controls; not full game/layout proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_statuswin_draw_active_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]


def product():
    header = (ROOT / 'src/jackal/font.h').read_text()
    owner = (ROOT / 'src/buffer.cpp').read_text()
    signature = 'int Font::line_advance() const'
    getter = extract_function(header if signature in header else owner, signature)
    helper = extract_function((ROOT / 'src/jackal/font_metrics.h').read_text(), 'inline int font_line_advance(')
    accessors = '\n'.join(extract_function(header, signature) for signature in
                          ('int pixel_height() const', 'int line_height() const'))
    getter = helper + '\n' + getter
    status = extract_function((ROOT / 'src/statuswin.cpp').read_text(),
                              'void StatusWin::draw_active(int vehicle_id, int base_id, int add_spot)')
    entry = re.search(r'const int row_pitch = font1_\.line_advance\(\);', status)
    assert entry and entry.start() < status.index('screen->set_font(')
    assert status.count('font1_.line_advance()') == 1
    advances = re.findall(r'line_y = static_cast<int32_t>\(static_cast<uint32_t>\(line_y\)\s*\+ static_cast<uint32_t>\(row_pitch\)\);', status)
    assert len(advances) == 4
    cargo_start = status.index('            const int capacity = veh_cargo(')
    cargo_end = status.index('        if (vehicle.dmg_incurred', cargo_start)
    cargo = status[cargo_start:cargo_end]
    assert 'screen->wrap(' not in cargo
    call = re.search(r'if \(StringTemp != 0\) \{\s*screen->write_l\(StringTemp, content_x, line_y, strlen\(StringTemp\)\);\s*\}', cargo)
    assert call
    commerce = extract_function((ROOT / 'src/basewin.cpp').read_text(),
                                'void BaseWin::draw_commerce(int update_mode)')
    detail = re.search(r'const int detail_y = static_cast<int32_t>\(\s*static_cast<uint32_t>\(row_y\) \+\s*static_cast<uint32_t>\(font.pixel_height\(\)\) \+ 1U\);', commerce)
    assert detail
    return getter, accessors, entry.group(), advances, call.group(), detail.group()


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <climits>
using std::strlen;
static int reads[3],writes,wraps;
struct ReadWord {
 int value,slot;
 ReadWord(int v=0,int s=0):value(v),slot(s){}
 operator int() const { ++reads[slot];return value; }
};
struct Font {
 ReadWord unk_1_,height_,line_height_;
 Font(int a,int h,int l):unk_1_(a,0),height_(h,1),line_height_(l,2){}
 int line_advance() const;
 ACCESSORS
};
GETTER
struct Buffer {
 int write_l(char *text,int x,int y,int len) {
  assert(!std::strcmp(text,"cargo")&&x==7&&y==23&&len==5);
  ++writes;return -900;
 }
 int wrap(char*,int,int,int) {++wraps;return 900;}
};
int main() {
 const int a[]={-1,INT_MIN,0,4,1,INT_MAX};
 const int h[]={9,9,9,9,INT_MAX,1};
 const int expected[]={17,17,9,13,INT_MIN,INT_MIN};
 for(int i=0;i<6;++i) {
  Font f(a[i],h[i],17);reads[0]=reads[1]=reads[2]=0;
  assert(f.line_advance()==expected[i]);
  assert(reads[0]==1&&reads[1]==(a[i]>=0)&&reads[2]==(a[i]<0));
 }
 Font font1_(4,9,17);
 ENTRY
 // A renderer callback changes the live font after the actual entry snapshot.
 font1_.height_.value=99;font1_.unk_1_.value=3;
 int line_y=10;int content_x=7;Buffer storage;Buffer *screen=&storage;
 char StringTemp[16]="cargo";
 ADVANCE0
 CARGO
 ADVANCE1
 ADVANCE2
 ADVANCE3
 assert(line_y==62&&writes==1&&wraps==0&&row_pitch==13);
 for(int mode=0;mode<3;++mode) {
  Font font(mode==0?-1:4,9,17);
  int row_y=mode==2?INT_MAX:100;
  DETAIL
  assert(detail_y==(mode==2?INT_MIN+9:110));
 }
}
'''


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', [None, 'late_metric', 'wrong_commerce_metric', 'cargo_wrap', 'eager_metric_read'])
def test_actual_font_and_client_contracts(tmp_path, compiler, damage):
    compiler = shutil.which(compiler)
    assert compiler, 'required host compiler unavailable'
    getter, accessors, entry, advances, cargo, detail = product()
    if damage == 'late_metric':
        advances = [x.replace('row_pitch', 'font1_.line_advance()') for x in advances]
    elif damage == 'wrong_commerce_metric':
        detail = detail.replace('font.pixel_height()', 'font.line_advance()')
    elif damage == 'cargo_wrap':
        cargo = 'line_y = screen->wrap(StringTemp, content_x, line_y, 100);'
    elif damage == 'eager_metric_read':
        getter = getter.replace('int Font::line_advance() const {',
                                'int Font::line_advance() const { const int eager = height_; (void)eager;', 1)
    generated = HARNESS.replace('GETTER', getter).replace('ACCESSORS', accessors).replace('ENTRY', entry)
    generated = generated.replace('CARGO', cargo).replace('DETAIL', detail)
    for index, advance in enumerate(advances):
        generated = generated.replace('ADVANCE' + str(index), advance)
    cpp, exe = tmp_path / 'contract.cpp', tmp_path / 'contract'
    cpp.write_text(generated)
    built = subprocess.run([compiler, '-std=c++11', '-O2', '-Wall', '-Wextra',
                            str(cpp), '-o', str(exe)], capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True)
    assert (result.returncode == 0) == (damage is None), result.stderr
