"""Compile the complete production button-placement block with typed C++98 controls.

These are bounded host behavior and pinned-original instruction controls, not
VC6 ABI or game runtime acceptance. All button setup calls stay in the block.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_basepop_dialog_attach_oct17 import _function_body
from test_popup_negative_position_oct18 import _decode_original


ROOT = Path(__file__).resolve().parents[2]
MARKER = "/* ACTUAL_BUTTON_BLOCK */"


HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
struct BasePop;
struct Font {};
struct Event { int id, x, y, width, height, style; const char *text; BasePop *parent; };
static std::vector<Event> events;
static std::vector<unsigned long> allocations;
static void *mem_get(unsigned long n) { allocations.push_back(n); return std::malloc(n); }
static const char *BasePopDefaultOkText = 0;
static const char *BasePopDefaultCancelText = 0;
struct FlatButton {
    BasePop *parent; int id;
    FlatButton() : parent(0), id(0) {}
    void init(char *text, int identifier, int x, int y, int w, int h, BasePop *p, int style) {
        parent=p; id=identifier;
        Event e={identifier,x,y,w,h,style,text,p}; events.push_back(e);
    }
    void set_font(Font *,Font *,Font *,int) {}
    void redraw() {}
    void set_text_color(int,int,int,int) {}
    void set_text_color2(int,int,int,int) {}
    void set_text_color3(int,int,int,int) {}
    void show(int);
};
struct StringStructPayload { char *text; };
struct ItemList {
    int entry_count_, cursor; bool null_payload;
    StringStructPayload payload;
    ItemList() : entry_count_(0), cursor(-1), null_payload(false) { payload.text=(char *)"TOUR"; }
    void seek_pos(int n) { cursor=n; }
    StringStructPayload *next_entry() { ++cursor; return null_payload ? 0 : &payload; }
};
struct BasePop {
    int field_30F4_, field_30C0_, field_30EC_, field_30E0_, field_30D0_;
    int field_30E4_, field_30CC_, field_30A8_, field_30B0_, field_20F4_;
    BasePop *owner_window_;
    char *ok_text_, *cancel_text_;
    Font *button_font1_, *button_font2_, *button_font3_;
    int button_color_a_,button_color_b_,button_color_c_,button_color_d_;
    int button_color_2a_,button_color_2b_,button_color_2c_,button_color_2d_;
    int button_color_3a_,button_color_3b_,button_color_3c_,button_color_3d_;
    FlatButton flat_button1_,flat_button2_,*item_buttons_[16];
    ItemList item_ids_;
    int provider_x, provider_y, provider_style, provider_calls;
    bool mutate_pitch;
    BasePop() : field_30F4_(12),field_30C0_(211),field_30EC_(8),field_30E0_(4),
        field_30D0_(20),field_30E4_(5),field_30CC_(60),field_30A8_(0),
        field_30B0_(3),field_20F4_(2),owner_window_(this),
        ok_text_((char *)"OKAY"),cancel_text_((char *)"CANCEL"),
        button_font1_(0),button_font2_(0),button_font3_(0),
        button_color_a_(0),button_color_b_(0),button_color_c_(0),button_color_d_(0),
        button_color_2a_(0),button_color_2b_(0),button_color_2c_(0),button_color_2d_(0),
        button_color_3a_(0),button_color_3b_(0),button_color_3c_(0),button_color_3d_(0),
        provider_x(0),provider_y(0),provider_style(0),provider_calls(0),mutate_pitch(false) {
        for(int i=0;i<16;++i) item_buttons_[i]=0;
        item_ids_.entry_count_=2;
    }
    ~BasePop() { for(int i=0;i<16;++i) delete item_buttons_[i]; }
    void on_adjust_button_placement(int *x,int *y,int *style) {
        ++provider_calls; *x+=provider_x; *y+=provider_y; *style=provider_style;
    }
    void apply_button_config(FlatButton *) {}
    void apply_button_bevel(FlatButton *) {}
    void place() {
/* ACTUAL_BUTTON_BLOCK */
        ;
    }
};
void FlatButton::show(int) {
    // Model a legitimate reentrant virtual callback mutating popup spacing.
    if(parent->mutate_pitch) {
        if(id==-1) { parent->field_30E4_=7; parent->field_30CC_=70; }
        if(id==1) { parent->field_30E4_=9; parent->field_30CC_=80; }
        if(id==2) { parent->field_30E4_=11; parent->field_30CC_=90; }
    }
}
static void require(bool condition,const char *witness) {
    if(!condition) { std::fprintf(stderr,"witness: %s\n",witness); std::exit(1); }
}
static void check(const Event &e,int id,int x,int y,int style,BasePop *p) {
    require(e.id==id,"button-identity");
    require(e.y==y,"shared-Y-baseline");
    require(e.x==x,"horizontal-X-placement");
    require(e.style==style && e.parent==p,"provider-style-parent");
    require(e.height==20,"button-height");
}
int main() {
    const int origins[]={-12,0,18};
    const int modes[]={0,0x40,0x10000,0x10040};
    for(int o=0;o<3;++o) for(int m=0;m<4;++m) {
        BasePop p; events.clear();
        // Exercise the provider's X, Y, and style outputs, not initial values.
        p.provider_x=origins[o]+12; p.provider_y=13; p.provider_style=0x123;
        p.field_30A8_=modes[m];
        bool only=(modes[m]&0x10000)!=0, cancel=(modes[m]&0x40)!=0;
        p.field_30B0_=2+(!only)+cancel;
        p.place();
        require(p.provider_calls==1,"provider-called-once");
        require(events.size()==static_cast<unsigned>(2+(!only)+cancel),"all-button-families");
        int n=0,origin=origins[o],pitch=65,y=192;
        if(!only) check(events[n++],-1,cancel ? (origin<0 ? origin-pitch : origin+2*pitch) : origin,y,0x123,&p);
        int x=origin<0 ? origin-pitch*(p.field_30B0_-1) : origin;
        check(events[n++],1,x,y,0x123,&p);
        check(events[n++],2,x+pitch,y,0x123,&p);
        if(cancel) check(events[n++],-2,origin<0 ? origin : origin+pitch*(p.field_30B0_-1),y,0x123,&p);
    }
    // Fresh reloads after OK, first item and second item callbacks.
    for(int negative=0;negative<2;++negative) {
        BasePop p; events.clear(); p.mutate_pitch=true; p.field_30A8_=0x40;
        p.provider_x=negative ? 0 : 30; p.field_30B0_=4;
        p.place(); int origin=negative ? -12 : 18;
        check(events[0],-1,negative ? origin-65 : origin+130,179,0,&p);
        int x=negative ? origin-77*3 : origin;
        check(events[1],1,x,179,0,&p);
        check(events[2],2,x+89,179,0,&p);
        check(events[3],-2,negative ? origin : origin+101*3,179,0,&p);
        require(events[1].width==70 && events[2].width==80 && events[3].width==90,"fresh-button-width");
    }
    const int skips[]={0x100,0x80000,0x80100};
    for(int i=0;i<3;++i) { BasePop p; events.clear();p.field_30A8_=skips[i];p.place();require(events.empty(),"flag-skip");require(p.provider_calls==1,"provider-before-skip"); }
    { BasePop p,other;events.clear();p.owner_window_=&other;p.place();require(events.empty(),"owner-skip"); }
    { BasePop p;events.clear();p.item_ids_.entry_count_=0;p.place();require(events.size()==1,"empty-list-OK"); }
    { BasePop p;events.clear();p.item_ids_.entry_count_=1;p.item_ids_.null_payload=true;p.place();require(events[1].text==0,"null-payload-forwarded"); }
    // Preserve original fallback quirk: allocation uses Cancel length, text OK.
    { BasePop p;events.clear();allocations.clear();p.field_30A8_=0x40;p.item_ids_.entry_count_=0;
      p.cancel_text_=0;p.ok_text_=0;p.place();
      require(allocations.size()==2 && allocations[0]==3 && allocations[1]==7,"fallback-allocation-sizes");
      require(std::strcmp(events[0].text,"OK")==0 && std::strcmp(events[1].text,"OK")==0,"original-cancel-OK-quirk");
      std::free(p.cancel_text_);std::free(p.ok_text_); }
}
'''


def _block():
    source = (ROOT / "src/basepop.cpp").read_text()
    body = _function_body(source, "int BasePop::exec(int flags,")
    start = body.index("    // THE ROW ORIGIN.")
    end = body.index("pump:", start) + len("pump:")
    return body[start:end]


def _run(tmp_path, compiler, block):
    cpp, exe = tmp_path / "buttons.cpp", tmp_path / "buttons"
    cpp.write_text(HARNESS.replace(MARKER, block))
    built = subprocess.run([compiler, "-std=c++98", "-O0", str(cpp), "-o", str(exe)],
                           capture_output=True, text=True, timeout=40)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_button_block_cpp98(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    result = _run(tmp_path, compiler, _block())
    assert result.returncode == 0, result.stderr


def test_compiling_prior_Y_damage_fails_named_witness(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    block, count = re.subn(r"button_x = origin_x", "button_y = origin_x", _block())
    assert count >= 4, "damage must affect the actual placement assignments"
    result = _run(tmp_path, compiler, block)
    assert result.returncode != 0
    assert "witness: shared-Y-baseline" in result.stderr


def test_compiling_cached_pitch_damage_fails_named_witness(tmp_path):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host C++ compiler unavailable")
    block, count = re.subn(
        r"static_cast<int>\(field_30E4_\)\s*\+\s*static_cast<int>\(field_30CC_\)",
        "cached_pitch", _block(),
    )
    assert count >= 5
    block = block.replace(
        "const int origin_x = button_x;",
        "const int cached_pitch = field_30E4_ + field_30CC_;\n"
        "    const int origin_x = button_x;", 1,
    )
    result = _run(tmp_path, compiler, block)
    assert result.returncode != 0
    assert "witness: horizontal-X-placement" in result.stderr


def test_pinned_original_contiguous_button_coordinate_and_fresh_pitch_anchors():
    decoded = _decode_original()  # whole 602600..6037F5, no guessed mid-instruction starts
    expected = {
        0x602C49: ("mov", "dword ptr [esp + 0x14], eax"),
        0x602C53: ("mov", "dword ptr [esp + 0x10], ecx"),
        0x602C64: ("call", "dword ptr [edx + 0x178]"),
        0x602C76: ("test", "eax, 0x80100"),
        0x602C85: ("cmp", "dword ptr [esi + 0xa3c], esi"),
        0x602CA4: ("mov", "edx, dword ptr [esi + 0x30e4]"),
        0x602CAA: ("mov", "edi, dword ptr [esi + 0x30cc]"),
        0x602D48: ("mov", "edx, dword ptr [esp + 0x18]"),
        0x602D4E: ("mov", "eax, dword ptr [esp + 0x1c]"),
        0x602F9F: ("mov", "dword ptr [esp + 0x10], ebp"),
        0x602FA5: ("mov", "ecx, dword ptr [esi + 0x30e4]"),
        0x602FAB: ("mov", "ebx, dword ptr [esi + 0x30cc]"),
        0x6030CC: ("mov", "edx, dword ptr [esp + 0x20]"),
        0x6030D1: ("mov", "ecx, dword ptr [esp + 0x20]"),
        0x6030D6: ("push", "edx"),
        0x6030D7: ("push", "ecx"),
        0x603334: ("mov", "ecx, dword ptr [esi + 0x30cc]"),
        0x60333A: ("mov", "ebp, dword ptr [esi + 0x30e4]"),
        0x603353: ("mov", "dword ptr [esp + 0x10], edx"),
        0x603378: ("mov", "eax, dword ptr [esi + 0x30e4]"),
        0x60337E: ("mov", "ebx, dword ptr [esi + 0x30cc]"),
        0x603578: ("mov", "eax, dword ptr [esi + 0x30e4]"),
        0x60357E: ("mov", "edx, dword ptr [esi + 0x30cc]"),
    }
    for address, pair in expected.items():
        instruction = decoded[address]
        assert (instruction.mnemonic, instruction.op_str) == pair, hex(address)
