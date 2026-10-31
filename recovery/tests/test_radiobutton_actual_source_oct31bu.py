"""Actual RadioButton controls. Host models check behavior, not native ABI.

Pinned: 60E220..60E2FE retains parent and reloads draw state;
60D5A2..60D5BD faults on malformed payloads, 60D5D9..60D5FF reloads after text_width.
"""
from pathlib import Path
import os
import shutil
import subprocess
import pytest
SOURCE=Path(__file__).resolve().parents[2]/"src/jackal/radiobutton.cpp"
def body(text,name,result):
    start=text.index(result+" RadioButton::"+name+"("); end=text.index("{",start)+1; depth=1
    while depth:
        depth+=(text[end]=="{")-(text[end]=="}");end+=1
    return text[start:end]
HARNESS=r'''
#include <cassert>
#include <cstdint>
#include <cstring>
struct RadioButton; struct GraphicWin;
static RadioButton *active;
static int callback_mode,copies,draws,updates,handled,states[8];
struct StringStructPayload {char *text;int field_8_;};
struct StringStructEntry {StringStructEntry *next;StringStructPayload *payload;
StringStructPayload *string_payload(){return payload;}};
struct ItemList {StringStructEntry *head_,*current_;int current_position_,entry_count_;};
struct Sprite {int width(){return 3;}int height(){return 4;}};
struct Win {virtual int on_dialog_back_draw(GraphicWin *);};
static Win *WinCallbackWindow,*copied_parent,*replacement_parent;
struct Buffer {
struct {struct {int biWidth,biHeight;} bmiHeader;} dib_;
void set_text_color(int,int,int,int){} void set_text_color2(int,int,int,int){}
void set_text_color3(int,int,int,int){} void set_font(int,int,int,int){}
int text_width(char *);int text_height(){return 12;}
void copy(Buffer *,int,int,int,int,int,int);
};
struct GraphicWin:Win,Buffer {Win *win_parent_;};
struct Dialog {
ItemList item_list_;int selected_position_,window_x_,window_y_;
int field_34_,field_48_,field_68_,field_50_,row_count_,field_4C_,field_20_;
int field_2C_,field_40_,field_44_,field_64_,field_5C_,column_count_;
int field_30_,field_60_,font1_,font2_,font3_;
};
struct RadioButton:virtual GraphicWin,virtual Dialog {
int text_color_a_,text_color_b_,text_color_c_,text_color_d_;
int text_color_2a_,text_color_2b_,text_color_2c_,text_color_2d_;
int text_color_3a_,text_color_3b_,text_color_3c_,text_color_3d_;
Sprite *unchecked_sprite_,*checked_sprite_;
void on_redraw();int calculate_dimensions();
void draw_item(int p,int s){assert(p==draws);states[draws++]=s;
if(p==0){item_list_.entry_count_=4;selected_position_=2;}}
void soft_update(){++updates;}
};
static StringStructEntry *replacement_entry;
int Win::on_dialog_back_draw(GraphicWin *self){
assert(self==static_cast<GraphicWin *>(active));active->win_parent_=replacement_parent;
active->window_x_=31;active->window_y_=41;
active->dib_.bmiHeader.biWidth=51;active->dib_.bmiHeader.biHeight=-61;return handled;
}
void Buffer::copy(Buffer *source,int x,int y,int sx,int sy,int w,int h){
assert(source==static_cast<Buffer *>(active));
assert(x==31&&y==41&&sx==0&&sy==0&&w==51&&h==61);
copied_parent=static_cast<GraphicWin *>(this);++copies;
}
int Buffer::text_width(char *text){
if(callback_mode==1){assert(text&&std::strcmp(text,"before")==0);active->item_list_.current_=replacement_entry;}
else if(callback_mode==2)active->item_list_.head_=0;
else if(callback_mode==3)active->item_list_.current_=0;
return 200001;
}
/* DIMENSIONS */
/* REDRAW */
int main(int argc,char **argv){
RadioButton b={};active=&b;GraphicWin parent={},other={};replacement_parent=&other;
if(argc>1&&!std::strcmp(argv[1],"--redraw")){
b.on_redraw();assert(!copies&&!draws&&!updates&&!WinCallbackWindow);
b.win_parent_=&parent;b.item_list_.entry_count_=3;b.selected_position_=1;b.on_redraw();
assert(WinCallbackWindow==&parent&&copied_parent==&parent);
assert(copies==1&&draws==4&&updates==1);
assert(states[0]==0&&states[1]==0&&states[2]==1&&states[3]==0);
copies=draws=updates=0;handled=1;b.win_parent_=&parent;b.on_redraw();
assert(!copies&&draws==4&&updates==1);return 0;}
StringStructPayload first={(char*)"before",71},second={(char*)"after",81};
StringStructEntry a={},z={};a.next=&a;a.payload=&first;z.next=&z;z.payload=&second;
replacement_entry=&z;b.item_list_.head_=&a;b.item_list_.current_=&a;b.item_list_.entry_count_=1;
b.field_48_=10;b.field_50_=1;b.field_4C_=1;callback_mode=1;
if(argc>1){
if(!std::strcmp(argv[1],"--head-fault"))b.item_list_.head_=0;
if(!std::strcmp(argv[1],"--payload-fault"))a.payload=0;
if(!std::strcmp(argv[1],"--post-head-fault"))callback_mode=2;
if(!std::strcmp(argv[1],"--post-current-fault"))callback_mode=3;
}
assert(b.calculate_dimensions()==0);
assert(first.field_8_==71&&second.field_8_==3);
assert(b.field_68_==12&&b.field_60_==12&&b.column_count_==1);return 0;
}
'''
def compile_actual(tmp_path,compiler,text=None):
    text=SOURCE.read_text() if text is None else text
    fixture=HARNESS.replace("/* DIMENSIONS */",body(text,"calculate_dimensions","int"))
    fixture=fixture.replace("/* REDRAW */",body(text,"on_redraw","void"))
    cpp=tmp_path/"radio.cpp";exe=tmp_path/"radio";cpp.write_text(fixture)
    result=subprocess.run([compiler,"-std=c++11","-Wall","-Wextra","-Werror","-fwrapv",
        "-fsanitize=undefined","-fno-sanitize-recover=all",str(cpp),"-o",str(exe)],text=True,capture_output=True)
    assert result.returncode==0,result.stdout+result.stderr
    return exe
def run(exe,mode=None):
    return subprocess.run([str(exe)]+([mode] if mode else []),text=True,capture_output=True,
        timeout=20,env={**os.environ,"UBSAN_OPTIONS":"halt_on_error=1"})
@pytest.mark.parametrize("compiler",("g++","clang++"))
def test_actual_dimensions_and_redraw(compiler,tmp_path):
    if not shutil.which(compiler):pytest.skip(compiler+" unavailable")
    exe=compile_actual(tmp_path,compiler)
    for mode in (None,"--redraw"):
        result=run(exe,mode);assert result.returncode==0,result.stdout+result.stderr
    for mode in ("--head-fault","--payload-fault","--post-head-fault","--post-current-fault"):
        result=run(exe,mode);assert result.returncode!=0
        assert ("runtime error: member call on null pointer" if mode == "--post-current-fault"
                else "runtime error: member access within null pointer") in result.stderr
@pytest.mark.parametrize("damage",("cached-payload","safe-store","selected-state","live-parent"))
def test_callback_and_fault_mutants_are_rejected(damage,tmp_path):
    if not shutil.which("g++"):pytest.skip("g++ unavailable")
    text=SOURCE.read_text();mode=None
    if damage=="cached-payload":
        old="        const int width = Buffer::text_width(text);"
        assert text.count(old)==1
        text=text.replace(old,"        StringStructEntry *cached = Dialog::item_list_.current_;\n"+old)
        old="? Dialog::item_list_.current_->string_payload() : 0;\n        payload->field_8_"
        assert text.count(old)==1
        text=text.replace(old,"? cached->string_payload() : 0;\n        payload->field_8_")
    elif damage=="safe-store":
        old="        payload->field_8_ = width / max_wrap_width + 1;";assert text.count(old)==1
        text=text.replace(old,"        if (payload) payload->field_8_ = width / max_wrap_width + 1;")
        mode="--post-head-fault"
    elif damage=="selected-state":
        old="        if (Dialog::selected_position_ == position) {";assert text.count(old)==1
        text=text.replace(old,old.replace(" == "," != "));mode="--redraw"
    else:
        start=text.index("void RadioButton::on_redraw()");prefix,text=text[:start],text[start:]
        old="static_cast<GraphicWin *>(parent)->copy(";assert text.count(old)==1
        text=prefix+text.replace(old,"static_cast<GraphicWin *>(win_parent_)->copy(");mode="--redraw"
    result=run(compile_actual(tmp_path,"g++",text),mode);assert result.returncode!=0
    if damage=="safe-store":assert "runtime error:" not in result.stderr
