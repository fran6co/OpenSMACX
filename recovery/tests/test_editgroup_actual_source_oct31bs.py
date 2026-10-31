"""Actual EditGroup sizing/redraw callbacks and original unsafe-list controls."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.source import read_file

ROOT = Path(__file__).resolve().parents[2]


def definition(signature, address):
    path = ROOT / 'src/editgroup.cpp'
    row, = (r for r in read_file(path) if r.address == address)
    assert row.image_spans
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
#include <cstring>
#include <string>
#include <vector>
struct RECT { int left,top,right,bottom; };
struct EditGroup;
static EditGroup *active;
static std::string scenario;
static int measurements, copies;
static std::vector<RECT> rectangles;
struct StringStructPayload { char *text; };
struct StringStructEntry {
 StringStructPayload *payload;
 StringStructEntry *next,*previous;
 StringStructPayload *string_payload() { return payload; }
};
struct StringList {
 int entry_count_,current_position_;
 StringStructEntry *head_,*current_;
 void seek_pos(int value) {
  assert(value == -1 && head_);
  current_=head_->previous; current_position_=entry_count_-1;
 }
};
struct Win { Win *win_parent_; };
static Win *WinCallbackWindow;
struct Buffer {
 struct { struct { int biWidth,biHeight; } bmiHeader; } dib_;
 void set_text_color(int,int,int,int) {}
 void set_text_color2(int,int,int,int) {}
 void set_text_color3(int,int,int,int) {}
 void set_font(void*,void*,void*,void*) {}
 int text_width(const char *);
 int text_line_height() { return 12; }
 void copy(Buffer *,int,int,int,int,int,int) { ++copies; }
 void write_l(const char *,RECT *,int);
};
struct GraphicWin : Win,Buffer {};
struct Dialog {
 StringList item_list_;
 int row_count_,column_count_,field_50_,field_40_,field_34_,field_44_,field_64_,field_68_;
 int field_20_,field_2C_,field_4C_,field_5C_,field_30_,field_60_,window_x_,window_y_;
 int text_color_a_,text_color_b_,text_color_c_,text_color_d_;
 int text_color_2a_,text_color_2b_,text_color_2c_,text_color_2d_;
 int text_color_3a_,text_color_3b_,text_color_3c_,text_color_3d_;
 void *font1_,*font2_,*font3_;
};
struct EditGroup : virtual GraphicWin,virtual Dialog {
 int label_width_,edit_width_;
 char *text_buffers_[10];
 int calculate_dimensions();
 void on_redraw();
};
int Buffer::text_width(const char *text) {
 ++measurements;
 int answer=text ? static_cast<int>(std::strlen(text)):0;
 if (measurements==1) {
  if (scenario=="shrink") active->item_list_.entry_count_=1;
  if (scenario=="grow") active->item_list_.entry_count_=3;
  if (scenario=="head") active->item_list_.head_=0;
  if (scenario=="clear") active->item_list_.head_=active->item_list_.current_=0;
 }
 return answer;
}
void Buffer::write_l(const char *text,RECT *rect,int length) {
 assert(text && length==static_cast<int>(std::strlen(text)));
 assert(this==static_cast<Buffer *>(static_cast<GraphicWin *>(active)));
 rectangles.push_back(*rect);
 if (rectangles.size()==1) {
  if(scenario=="redraw-stop") active->row_count_=1;
  if(scenario=="redraw-step") active->field_68_=24;
 }
}
// ACTUAL_METHODS
int main(int argc,char **argv) {
 assert(argc==2); scenario=argv[1];
 EditGroup w={}; active=&w;
 char first[]="A",second[]="BB",edit1[]="abcdefgh",edit2[]="abcdefghi";
 StringStructPayload p1={first},p2={second};
 StringStructEntry one={&p1,0,0},two={&p2,0,0};
 one.next=one.previous=&two; two.next=two.previous=&one;
 w.item_list_.entry_count_=2; w.item_list_.head_=&one;
 w.text_buffers_[0]=edit1;w.text_buffers_[1]=edit2;
 w.field_50_=2;w.field_40_=2;w.field_34_=3;w.field_44_=4;w.field_4C_=1;
 if(scenario=="null_payload") one.payload=0;
 if(scenario.find("redraw")==0) {
  GraphicWin parent={}; w.win_parent_=&parent;
  w.row_count_=2;w.column_count_=1;w.field_64_=20;w.field_68_=16;
  w.on_redraw();
  assert(WinCallbackWindow==&parent && copies==1);
  assert(rectangles.size()==(scenario=="redraw-stop" ? 1:2));
  assert(rectangles[0].left==5 && rectangles[0].top==2 && rectangles[0].right==25 && rectangles[0].bottom==14);
  if(rectangles.size()==2) assert(rectangles[1].top==(scenario=="redraw-step" ? 26:18));
  return 0;
 }
 assert(w.calculate_dimensions()==0);
 // Unsafe scenarios must fail inside the actual member read, not a later assertion.
 if(scenario=="head" || scenario=="null_payload") return 0;
 assert(measurements==4);
 assert(w.label_width_==(scenario=="clear" ? 1:2));
 // Original compares measured text width before adding the inner margins.
 assert(w.edit_width_==12);
 assert(w.field_64_==(scenario=="clear" ? 28:29));
 assert(w.field_60_==36);
 if(scenario=="shrink") assert(w.item_list_.current_position_==0);
 if(scenario=="clear") assert(w.item_list_.current_==0);
}
'''


def run_host(tmp_path, scenario, *, normalized=False, live_count=False):
    sizing=definition('int EditGroup::calculate_dimensions()',0x612150)
    redraw=definition('void EditGroup::on_redraw()',0x612450)
    if normalized:
        old='label = payload->text;'
        assert sizing.count(old)==1
        sizing=sizing.replace(old,'label = payload != 0 ? payload->text : 0;')
    if live_count:
        old='while (remaining != 0);'
        assert sizing.count(old)==1
        sizing=sizing.replace(old,'while (remaining != 0 && Dialog::item_list_.entry_count_ > 1);')
    compiler=shutil.which('clang++') or shutil.which('g++')
    assert compiler
    cpp=tmp_path/'actual.cpp';exe=tmp_path/'actual'
    cpp.write_text(HARNESS.replace('// ACTUAL_METHODS',sizing+'\n'+redraw))
    built=subprocess.run([compiler,'-std=c++11','-O1','-fsanitize=address,undefined',
                          '-fno-sanitize-recover=all',str(cpp),'-o',str(exe)],text=True,capture_output=True)
    assert built.returncode==0,built.stdout+built.stderr
    return subprocess.run([str(exe),scenario],text=True,capture_output=True,
                          env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})


@pytest.mark.parametrize('scenario',['normal','shrink','grow','clear','redraw','redraw-stop','redraw-step'])
def test_actual_count_snapshot_cursor_and_redraw_callbacks(tmp_path,scenario):
    result=run_host(tmp_path,scenario)
    assert result.returncode==0,result.stdout+result.stderr


@pytest.mark.parametrize('scenario',['head','null_payload'])
def test_actual_unsafe_paths_and_rejected_normalization(tmp_path,scenario):
    result=run_host(tmp_path,scenario)
    assert result.returncode!=0
    assert 'null pointer' in result.stderr or 'AddressSanitizer' in result.stderr
    normalized=run_host(tmp_path,scenario,normalized=True)
    assert normalized.returncode==0,normalized.stdout+normalized.stderr


def test_live_count_mutant_loses_original_callback_snapshot(tmp_path):
    result=run_host(tmp_path,'shrink',live_count=True)
    assert result.returncode!=0


def test_pinned_snapshot_and_unsafe_read_edges():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    path=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
    if not path.is_file():pytest.skip('pinned original unavailable')
    image=PeImage(path.read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    instructions={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(
        image.read(Span(0x612150,0x61244A)),0x612150)}
    assert instructions[0x6122AC]==('mov','ebx, eax')
    assert instructions[0x612344]==('dec','ebx')
    assert instructions[0x612345]==('jne','0x6122ae')
    assert instructions[0x6122E4]==('cmp','ecx, ebp')
    assert instructions[0x6122E6]==('je','0x6122f0')
    assert instructions[0x6122F0]==('xor','eax, eax')
    assert instructions[0x6122F2]==('mov','eax, dword ptr [eax + 4]')
    assert instructions[0x6122E8]==('mov','eax, dword ptr [eax + 8]')
    assert instructions[0x6122EB]==('mov','eax, dword ptr [eax + 4]')


ATTACH_HARNESS = r'''
#include <cassert>
#include <cstring>
#include <vector>
struct EditGroup;
static EditGroup *active;
static int initialized, shown, deleted, failure;
struct Buffer {
 void set_text_color(int,int,int,int){} void set_text_color2(int,int,int,int){}
 void set_text_color3(int,int,int,int){} void set_font(void*,void*,void*,void*){}
};
struct GraphicWin : Buffer {
 int iFlags_;
 int init(int x,int y,int width,int height,int,int flags,GraphicWin*,int,int) {
  assert(x==7 && y==9 && width==40 && height==36 && flags==0x100000);
  return failure;
 }
};
struct EditBox : Buffer {
 int field_B3C_=0,field_B14_,field_B1C_,field_B28_,field_B2C_,field_B30_,field_B34_,field_B40_,field_B44_;
 char text_[16];
 virtual ~EditBox(){++deleted;}
 int init(int,int,int,int,GraphicWin*,int);
 void set_text(char *p){std::strcpy(text_,p);}
 void redraw(); void show(int x){assert(x==0);++shown;}
};
struct Dialog {
 int window_x_,window_y_,field_5C_=40,field_60_=36,field_44_=4,field_68_=16,field_34_=3,field_64_=29;
 int column_count_=1,row_count_=2,field_20_=0,field_6C_=6,field_40_=2;
 int field_AC_=10,field_B0_=11,field_B4_=12;
 int text_color_a_,text_color_b_,text_color_c_,text_color_d_;
 int text_color_2a_,text_color_2b_,text_color_2c_,text_color_2d_;
 int text_color_3a_,text_color_3b_,text_color_3c_,text_color_3d_;
 void *font1_,*font2_,*font3_;
};
struct EditGroup:virtual GraphicWin,virtual Dialog {
 EditBox *boxes_[10]={};char *text_buffers_[10]={};int limits_[10]={};int label_width_=2;
 int calculate_dimensions(){return 0;}
 int attach(GraphicWin*,int,int,int);
};
int EditBox::init(int left,int top,int width,int height,GraphicWin *parent,int one){
 assert(parent==static_cast<GraphicWin*>(active) && one==1);
 assert(left==13 && top==2 && width==19 && height==12);
 ++initialized;return 0;
}
void EditBox::redraw(){
 assert(field_B14_==5 && field_B1C_==6 && field_B28_==10 && field_B2C_==11 && field_B30_==12);
 assert(field_B34_==2 && field_B40_==0 && field_B44_==3 && std::strcmp(text_,"abc")==0);
 active->row_count_=1; // callback must stop the live row loop before a second allocation.
}
// ACTUAL_ATTACH
int main(int argc,char **argv){
 assert(argc==2);EditGroup w;GraphicWin parent={};active=&w;
 w.field_20_=std::strcmp(argv[1],"set")==0 ? 0x1000:0x100000;
 char text[]="abc";w.text_buffers_[0]=text;w.limits_[0]=5;
 assert(w.attach(0,7,9,0)==3 && initialized==0);
 failure=17;assert(w.attach(&parent,7,9,0)==17 && initialized==0);
 failure=0;assert(w.attach(&parent,7,9,0)==0 && initialized==1 && shown==1);
 assert(w.window_x_==7 && w.window_y_==9);
 assert(w.boxes_[0]->field_B3C_==(w.field_20_==0x1000 ? 2:0));
 delete w.boxes_[0];assert(deleted==1);
}
'''


@pytest.mark.parametrize('scenario',['set','different_bit'])
def test_attach_original_flag_geometry_and_live_callback(tmp_path,scenario):
    method=definition('int EditGroup::attach(',0x611AF0)
    compiler=shutil.which('clang++') or shutil.which('g++')
    assert compiler
    def execute(body,suffix):
        cpp=tmp_path/(suffix+'.cpp');exe=tmp_path/suffix
        cpp.write_text(ATTACH_HARNESS.replace('// ACTUAL_ATTACH',body))
        built=subprocess.run([compiler,'-std=c++11','-O1',str(cpp),'-o',str(exe)],text=True,capture_output=True)
        assert built.returncode==0,built.stdout+built.stderr
        return subprocess.run([str(exe),scenario],text=True,capture_output=True)
    actual=execute(method,'actual_attach')
    assert actual.returncode==0,actual.stdout+actual.stderr
    old='Dialog::field_20_ & 0x1000'
    assert method.count(old)==1
    mutant=execute(method.replace(old,'Dialog::field_20_ & 0x100000'),'wrong_flag')
    assert mutant.returncode!=0


def test_pinned_attach_flag_member_receiver():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    path=Path(os.environ.get('OPENSMACX_IMAGE',ROOT/'.opensmacx/game/terranx_original.exe'))
    if not path.is_file():pytest.skip('pinned original unavailable')
    image=PeImage(path.read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    instructions={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(
        image.read(Span(0x611DC6,0x611DE0)),0x611DC6)}
    assert instructions[0x611DC8]==('mov','ecx, dword ptr [eax + 8]')
    assert instructions[0x611DCB]==('mov','eax, dword ptr [ecx + edi - 0xa84]')
    assert instructions[0x611DD2]==('test','ah, 0x10')
    assert instructions[0x611DD9]==('or','dword ptr [eax + 0xb3c], 2')
