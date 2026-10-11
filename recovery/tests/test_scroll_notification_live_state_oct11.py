"""Execute current Scroll callbacks against native reload/delta-gate observations.

Host controls do not establish Win32 ABI, matching, or full-game acceptance.
The pre-change source and intentional damages must fail the same oracle.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]
BASE = "f227548f6bc61838fb0005ef9140bae3bae1a779"
SIGNATURES = (
    "void Scroll::on_mousewheel_down(int delta)",
    "void Scroll::on_mousewheel_up(int delta)",
    "int Scroll::on_key_click(int, int key)",
    "void Scroll::on_button_clicked(int button_id)",
)


def control(damage=None):
    source = (ROOT / "src/scroll.cpp").read_text()
    if damage == "pre_change":
        source = subprocess.run(
            ["git", "show", BASE + ":src/scroll.cpp"], cwd=ROOT,
            check=True, capture_output=True, text=True).stdout
    bodies = "\n".join(extract_function(source, sig) for sig in SIGNATURES)
    if damage == "entry_parent_sound":
        assert bodies.count("win_parent_->legacy_sound_33()") == 4
        bodies = bodies.replace("win_parent_->legacy_sound_33()", "parent->legacy_sound_33()")
    elif damage == "stale_position":
        assert bodies.count("field_2144_(field_A18_, position_);") == 4
        bodies = bodies.replace("field_2144_(field_A18_, position_);", "field_2144_(17, 4);")
    elif damage == "retest_reversal":
        old = "field_2144_(field_A18_, position_);"
        assert bodies.count(old) == 4
        bodies = bodies.replace(old, "field_2144_(field_A18_, range_reversed_ ? range_maximum_ - position_ + range_minimum_ : position_);")
    elif damage == "ungated_sound":
        assert bodies.count("if (delta != 0)") == 2
        bodies = bodies.replace("if (delta != 0)", "if (true)")
    return r'''
#include <cassert>
#include <vector>
enum {VK_END=35,VK_HOME=36,VK_PRIOR=33,VK_NEXT=34};
std::vector<int> events;
struct Sound {
    int queries=0,plays=0,playing=0;
    int is_playing(){++queries;events.push_back(3);return playing;}
    void play(){++plays;events.push_back(4);}
};
struct Scroll;
struct Win {
    Sound *sound=nullptr;int calls=0,id=0,pos=0;
    bool mutate=false,remove_callback=false;Scroll *scroll=nullptr;Win *replacement=nullptr;
    void on_scrolled(int,int);
    Sound *legacy_sound_33(){return sound;}
};
Win *WinCallbackWindow=nullptr;int ListBoxGlobalDDC=99;
struct Scroll {
    Win *win_parent_=nullptr;
    int field_A38_=0,position_=3,range_minimum_=0,range_maximum_=10,field_A18_=17,paging_=2;
    unsigned range_reversed_=0,field_A3C_=0;
    void (*field_2144_)(int,int)=nullptr;
    struct {struct {int biWidth=10,biHeight=-5;} bmiHeader;} dib_;
    int redraws=0;
    void redraw(){assert(field_A3C_==0xFFFFFFFFu);++redraws;events.push_back(5);}
    void on_mousewheel_down(int);void on_mousewheel_up(int);
    int on_key_click(int,int);void on_button_clicked(int);
};
Scroll *active=nullptr;Win *callback_parent=nullptr;
int callback_id=0,callback_pos=0;
void callback(int id,int pos){callback_id=id;callback_pos=pos;events.push_back(2);active->win_parent_=callback_parent;}
void Win::on_scrolled(int i,int p){
    ++calls;id=i;pos=p;events.push_back(1);
    if(mutate){
        scroll->position_=7;scroll->range_minimum_=2;scroll->range_maximum_=20;
        scroll->field_A18_=23;scroll->range_reversed_^=1;
        scroll->win_parent_=replacement;
        scroll->field_2144_=remove_callback ? nullptr : callback;
    }
}
BODIES
void invoke(Scroll &s,int method,int delta=1){
    if(method==0)s.on_mousewheel_down(delta);
    if(method==1)s.on_mousewheel_up(delta);
    if(method==2)assert(s.on_key_click(0,VK_END)==1);
    if(method==3)s.on_button_clicked(2);
}
int main(){
    for(int method=0;method<4;++method)for(int reversed=0;reversed<2;++reversed){
        events.clear();Scroll s;Win a,b,c;Sound sa,sb,sc;
        a.sound=&sa;b.sound=&sb;c.sound=&sc;a.scroll=&s;a.replacement=&b;a.mutate=true;
        s.win_parent_=&a;s.range_reversed_=reversed;active=&s;callback_parent=&c;
        // Parent installs a previously absent function callback and changes all values.
        invoke(s,method);
        int moved=method==1?2:method==2?10:4;
        if(reversed && method<2)moved=10-moved;
        int initial=reversed?10-moved:moved;
        assert(a.calls==1 && a.id==17 && a.pos==initial);
        assert(callback_id==23 && callback_pos==(reversed?15:7));
        assert(s.win_parent_==&c && WinCallbackWindow==&a);
        assert(sa.queries==0 && sb.queries==0 && sc.queries==1 && sc.plays==1);
        assert((events==std::vector<int>{1,2,3,4,5}));
        // Removing the callback is also observable; sound now belongs to B.
        events.clear();s=Scroll();s.win_parent_=&a;s.field_2144_=callback;
        a.calls=0;a.remove_callback=true;invoke(s,method);
        assert(a.calls==1 && s.win_parent_==&b && sb.queries==1 && sb.plays==1);
        assert((events==std::vector<int>{1,3,4,5}));
    }
    for(int method=0;method<2;++method)for(int reversed=0;reversed<2;++reversed){
        events.clear();Scroll s;Win a;Sound sound;a.sound=&sound;s.win_parent_=&a;
        s.range_reversed_=reversed;s.field_2144_=callback;invoke(s,method,0);
        int moved=method==0?4:2;
        assert(s.position_==(reversed?10-moved:moved));
        assert(a.calls==0 && sound.queries==0 && s.redraws==1);
        assert((events==std::vector<int>{5}));
        // Negative delta is nonzero, not a direction or magnitude test.
        events.clear();s.field_2144_=nullptr;invoke(s,method,-3);
        assert(a.calls==1 && sound.queries==1 && sound.plays==1);
    }
    events.clear();Scroll absent;
    absent.on_mousewheel_down(1);absent.on_mousewheel_up(1);absent.on_button_clicked(1);
    assert(absent.on_key_click(0,VK_HOME)==0 && events.empty());
    Scroll s;Win a;Sound sound;a.sound=&sound;s.win_parent_=&a;
    assert(s.on_key_click(0,0)==0 && a.calls==0 && sound.queries==0 && s.redraws==0);
    events.clear();sound.playing=1;s.on_button_clicked(0);
    assert(s.position_==3 && sound.queries==1 && sound.plays==0);
    assert((events==std::vector<int>{1,3,5}));
}
'''.replace("BODIES", bodies)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "pre_change", "entry_parent_sound", "stale_position", "retest_reversal", "ungated_sound"))
def test_actual_notification_order_and_live_values(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    source = tmp_path / "scroll.cpp"
    source.write_text(control(damage))
    binary = tmp_path / "scroll"
    subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)],
                   check=True, capture_output=True, timeout=30)
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage is None), result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (False, True))
def test_actual_redraw_signed_bevel_bound(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    text = (ROOT / "src/scroll.cpp").read_text()
    body = extract_function(text, "void Scroll::on_redraw()")
    helper = extract_function(text, "inline LONG long_from_bits(uint32_t bits)")
    if damage:
        old = "int layer = 0; layer < long_from_bits(bevel_thickness_)"
        assert body.count(old) == 1
        body = body.replace(old, "unsigned int layer = 0; layer < bevel_thickness_")
    fixture = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
typedef int32_t LONG;
using std::memcpy;
HELPER
struct RECT {int left,top,right,bottom;};
struct Buffer {
    int boxes=0;unsigned *live_bound=nullptr;bool shorten=false;
    void box(RECT *rect,int,int){
        assert(boxes<4);assert(rect->left==boxes && rect->top==boxes);
        assert(rect->right==10-boxes && rect->bottom==10-boxes);
        ++boxes;if(shorten)*live_bound=1;
    }
    void tile(void *,int,int,int,int,int,int){}
    void fill(int){}void fill(RECT *,int){}
};
struct Button {void redraw(){}};
struct Sprite {
    int width(){return 0;}int height(){return 0;}int transparent_index(){return 0;}
    void draw(Buffer *,int,int,int,int,int){}
};
struct Scroll : Buffer {
    Button flat_button_left_,flat_button_right_;
    int field_A64_=0,border_color_=-1,bevel_upper_=0,bevel_lower_=0;
    RECT rect2_;void *field_A78_=nullptr;Sprite *thumb_sprite_=nullptr;
    unsigned bevel_thickness_=0;
    struct {struct {int biWidth=10,biHeight=-10;} bmiHeader;} dib_;
    void compute_thumb_rect(RECT *r){*r={0,0,10,10};}
    void on_redraw();
};
BODY
int main(){
    unsigned values[]={0,0xFFFFFFFFu,0x80000000u,1,3};
    int counts[]={0,0,0,1,3};
    for(int i=0;i<5;++i){Scroll s;s.bevel_thickness_=values[i];s.on_redraw();assert(s.boxes==counts[i]);}
    Scroll s;s.bevel_thickness_=3;s.live_bound=&s.bevel_thickness_;s.shorten=true;
    s.on_redraw();assert(s.boxes==1);
}
'''.replace("HELPER", helper).replace("BODY", body)
    source = tmp_path / "redraw.cpp"
    source.write_text(fixture)
    binary = tmp_path / "redraw"
    subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)],
                   check=True, capture_output=True, timeout=30)
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (not damage), result.stderr


def test_original_identities_claims_and_symbols_retained():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    path = ROOT / "src/scroll.cpp"
    prior = subprocess.check_output(["git", "show", BASE + ":src/scroll.cpp"],
                                    cwd=ROOT, text=True)
    before = {row.address: row for row in read_text(prior, path)}
    after = {row.address: row for row in read_text(path.read_text(), path)}
    assert before.keys() == after.keys()
    for address, row in before.items():
        assert _continuity_fields(row) == _continuity_fields(after[address])
        assert row.symbol == after[address].symbol


def test_pinned_delta_gates_signed_bound_and_live_sound_parent():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    expected = (
        (0x60637C, "je", "0x606425"),
        (0x60649C, "je", "0x606543"),
        (0x6067FC, "jle", "0x60684d"),
        (0x60684B, "jl", "0x606804"),
        (0x606402, "mov", "eax, dword ptr [esi + 0xc4]"),
        (0x606521, "mov", "eax, dword ptr [esi + 0xc4]"),
        (0x6066AE, "mov", "eax, dword ptr [esi + 0xc4]"),
        (0x606C08, "mov", "eax, dword ptr [esi + 0xc4]"),
    )
    for address, mnemonic, operands in expected:
        instruction = next(decoder.disasm(image.read(Span(address, address + 8)), address))
        assert (instruction.mnemonic, instruction.op_str) == (mnemonic, operands)
