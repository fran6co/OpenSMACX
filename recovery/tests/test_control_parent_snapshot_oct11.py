"""Actual control bodies retain native parent snapshots across external calls.

Host doubles do not prove original Win32 layout or full executable matching.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function
from test_texture_raster_actual_source import _continuity_fields
from match_recovery.source import read_text

ROOT = Path(__file__).resolve().parents[2]
BASE = "f35836fc519023ddf0f89da5e8db1b9a2894f686"
OWNERS = ("CheckBox", "RadioButton", "CheckButton")
FILES = ("checkbox", "radiobutton", "checkbutton")


def source_bodies(baseline=False):
    output = []
    for owner, filename in zip(OWNERS, FILES):
        path = ROOT / f"src/jackal/{filename}.cpp"
        text = (subprocess.check_output(["git", "show", BASE + ":" + str(path.relative_to(ROOT))],
                                       cwd=ROOT, text=True) if baseline else path.read_text())
        signature = (f"void {owner}::on_left_click(int, int)" if owner == "CheckButton"
                     else f"void {owner}::on_redraw()")
        output.append(extract_function(text, signature))
        if owner == "CheckBox":
            output.append(extract_function(text, "void CheckBox::on_mouse_move(int x_coord, int y_coord)"))
    return "\n".join(output)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "baseline", "live_copy", "stale_copy_geometry", "live_click", "cached_argument", "wrong_axes", "lost_noop"))
def test_actual_parent_snapshots(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(compiler + " unavailable")
    bodies = source_bodies(damage == "baseline")
    if damage == "live_copy":
        old = "static_cast<GraphicWin *>(parent)->copy("
        assert bodies.count(old) == 2
        bodies = bodies.replace(old, "static_cast<GraphicWin *>(win_parent_)->copy(")
    if damage == "live_click":
        assert bodies.count("parent->on_button_clicked(toggle_argument_);") == 1
        bodies = bodies.replace("parent->on_button_clicked(toggle_argument_);", "win_parent_->on_button_clicked(toggle_argument_);")
    if damage == "stale_copy_geometry":
        old = "this, Dialog::field_24_, Dialog::field_28_, 0, 0,\n            static_cast<int>(dib_.bmiHeader.biWidth),\n            -static_cast<int>(dib_.bmiHeader.biHeight)"
        assert bodies.count(old) == 2
        bodies = bodies.replace(old, "this, 7, 9, 0, 0, 20, 30")
    if damage == "cached_argument":
        assert bodies.count("checked_ = checked_ == 0;") == 1
        bodies = bodies.replace("checked_ = checked_ == 0;", "const int saved = toggle_argument_; checked_ = checked_ == 0;")
        bodies = bodies.replace("parent->on_button_clicked(toggle_argument_);", "parent->on_button_clicked(saved);")
    if damage == "wrong_axes":
        old = "static_cast<uint32_t>(x_coord / cell_width)\n                              * Dialog::field_58_\n                              + static_cast<uint32_t>(y_coord / cell_height)"
        assert bodies.count(old) == 1
        bodies = bodies.replace(old, "static_cast<uint32_t>(x_coord / cell_width + (y_coord * Dialog::field_58_) / cell_height)")
    if damage == "lost_noop":
        old = "if (Dialog::field_F0_ == 0xFFFFFFFFu) return;"
        assert bodies.count(old) == 1
        bodies = bodies.replace(old, "if (false) return;")
    fixture = r'''
#include <cassert>
#include <cstdint>
#include <vector>
std::vector<int> events;
struct GraphicWin;
struct Win {
    int backs=0,clicks=0,last=0,handled=0;Win **child_parent=nullptr;Win *replacement=nullptr;
    int *x=nullptr,*y=nullptr,*width=nullptr,*height=nullptr;
    int on_dialog_back_draw(GraphicWin *){
        ++backs;events.push_back(1);*child_parent=replacement;
        *x=17;*y=19;*width=22;*height=-33;return handled;
    }
    void on_button_clicked(int value){++clicks;last=value;events.push_back(4);}
};
Win *WinCallbackWindow=nullptr;
struct GraphicWin : Win {
    Win *win_parent_=nullptr;int copies=0,updates=0;
    struct {struct {int biWidth=20,biHeight=-30;} bmiHeader;} dib_;
    void copy(GraphicWin *,int x,int y,int,int,int width,int height){
        assert(x==17 && y==19 && width==22 && height==33);++copies;events.push_back(2);
    }
    void soft_update(){++updates;events.push_back(3);}
};
struct Dialog {
    struct {int entry_count_=0;} item_list_;
    int field_24_=7,field_28_=9,selected_position_=0;unsigned field_F0_=0;
    unsigned field_44_=2,field_64_=8,field_68_=8,field_58_=3;
    int pos_to_id(int p){return p;}
};
struct CheckBox : GraphicWin,Dialog {
    int draws=0,redraws=0;void draw_item(int,int,int){++draws;events.push_back(6);}
    void redraw(){++redraws;}
    int get_state_id(int){return 1;}void on_redraw();void on_mouse_move(int,int);
};
struct RadioButton : GraphicWin,Dialog {
    int draws=0;void draw_item(int,int){++draws;events.push_back(6);}
    void on_redraw();
};
struct CheckButton : GraphicWin {
    int checked_=0,toggle_argument_=17,redraws=0;
    void (*click_callback_)(int)=nullptr;
    Win *draw_parent=nullptr;
    void redraw(){++redraws;events.push_back(5);win_parent_=draw_parent;toggle_argument_=23;}
    void on_left_click(int,int);
};
CheckButton *active=nullptr;Win *callback_parent=nullptr;int callback_value=0;
void callback(int value){callback_value=value;events.push_back(7);active->win_parent_=callback_parent;active->toggle_argument_=31;}
BODIES
template<class Control>void check_redraw(bool radio){
    for(int handled=0;handled<2;++handled){
        events.clear();Control c;GraphicWin a,b;
        c.win_parent_=&a;a.child_parent=&c.win_parent_;a.replacement=&b;a.handled=handled;
        a.x=&c.field_24_;a.y=&c.field_28_;
        a.width=&c.dib_.bmiHeader.biWidth;a.height=&c.dib_.bmiHeader.biHeight;
        c.on_redraw();
        assert(a.backs==1 && b.backs==0 && a.copies==1-handled && b.copies==0);
        assert(c.win_parent_==&b && WinCallbackWindow==&a);
        assert(c.updates==(radio?1:0));
        assert(events[0]==1);if(!handled)assert(events[1]==2);
        // Original CheckBox leaves the global untouched for null parent;
        // RadioButton publishes null before its guard.
        events.clear();c.win_parent_=nullptr;WinCallbackWindow=&a;c.on_redraw();
        assert(events.empty() && WinCallbackWindow==(radio?nullptr:&a));
    }
}
int main(){
    check_redraw<CheckBox>(false);check_redraw<RadioButton>(true);
    for(int initial=0;initial<2;++initial){
        events.clear();CheckButton c;GraphicWin a,b,d;
        c.win_parent_=&a;c.draw_parent=&b;c.checked_=initial;
        c.click_callback_=callback;active=&c;callback_parent=&d;
        c.on_left_click(0,0);
        assert(c.checked_==!initial && callback_value==23);
        assert(a.clicks==1 && a.last==31 && b.clicks==0 && d.clicks==0);
        assert(c.redraws==2 && (events==std::vector<int>{5,7,4,5}));
    }
    events.clear();CheckButton c;c.on_left_click(0,0);
    assert(c.checked_==0 && c.redraws==0 && events.empty());
    CheckBox box;box.on_mouse_move(31,24);
    assert(box.field_F0_==11 && box.redraws==1);
    box.selected_position_=1<<11;box.field_F0_=0xFFFFFFFFu;
    box.on_mouse_move(31,24);assert(box.redraws==1);
    box.field_F0_=10;box.on_mouse_move(31,24);
    assert(box.field_F0_==0xFFFFFFFFu && box.redraws==2);
    box.selected_position_=0;box.on_mouse_move(31,24);
    assert(box.field_F0_==11 && box.redraws==3);
    box.on_mouse_move(31,24);assert(box.redraws==4);
    box.on_mouse_move(-31,24);assert(box.field_F0_==0xFFFFFFF9u);
    box.field_58_=0x80000000u;box.on_mouse_move(20,0);
    assert(box.field_F0_==0);
}
'''.replace("BODIES", bodies)
    source = tmp_path / "controls.cpp"
    source.write_text(fixture)
    binary = tmp_path / "controls"
    subprocess.run([executable, "-std=c++11", str(source), "-o", str(binary)],
                   check=True, capture_output=True, timeout=30)
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage is None), result.stderr


def test_original_identity_span_claim_and_symbol_continuity():
    for filename in FILES:
        path = ROOT / f"src/jackal/{filename}.cpp"
        prior = subprocess.check_output(["git", "show", BASE + ":" + str(path.relative_to(ROOT))],
                                        cwd=ROOT, text=True)
        before, after = read_text(prior, path), read_text(path.read_text(), path)
        assert [(r.address, _continuity_fields(r), r.symbol) for r in before] == [
            (r.address, _continuity_fields(r), r.symbol) for r in after]


def test_pinned_receivers_and_hover_control_flow():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    expected = {
        0x60FAEB: ("mov", "ecx, ebx"),
        0x60FB31: ("lea", "ecx, [ebx + 0x444]"),
        0x60FAC7: ("je", "0x60fb89"),
        0x60FAD3: ("mov", "dword ptr [0x9b7ab8], ebx"),
        0x60E25B: ("mov", "ecx, ebx"),
        0x60E2A1: ("lea", "ecx, [ebx + 0x444]"),
        0x60E237: ("mov", "dword ptr [0x9b7ab8], ebx"),
        0x60E23D: ("je", "0x60e2fa"),
        0x633924: ("mov", "edi, dword ptr [esi + 0xc4]"),
        0x633965: ("mov", "edx, dword ptr [edi]"),
        0x633968: ("mov", "ecx, edi"),
        0x60FBD3: ("idiv", "ecx"),
        0x60FBDC: ("imul", "ecx, dword ptr [esi + 0x3c]"),
        0x60FBE0: ("idiv", "ebp"),
        0x60FBE7: ("add", "ecx, eax"),
        0x60FC00: ("je", "0x60fc24"),
    }
    for address, value in expected.items():
        instruction = next(decoder.disasm(image.read(Span(address, address + 8)), address))
        assert (instruction.mnemonic, instruction.op_str) == value
    # EDI is root+1C; ESI adds root's Dialog displacement to that adjusted
    # receiver. Native +3C is consequently Dialog+58, not Dialog+3C.
    assert image.read(Span(0x670718, 0x670724)) == bytes.fromhex("000000001c000000340a0000")
