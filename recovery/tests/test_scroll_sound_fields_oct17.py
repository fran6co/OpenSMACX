"""Keep Scroll's two adjacent AutoSound words distinct and live at callbacks.

Host controls exercise extracted product bodies. They do not establish the
Win32 ABI, byte matching, ownership, lifetime, or full-game acceptance.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from test_listbox_rendering_actual_source import extract_function

ROOT = Path(__file__).resolve().parents[2]
MOUSE_MOVE = "void Scroll::on_mouse_move(int x, int y)"


def mouse_move_source(damage=None):
    source = (ROOT / "src/scroll.cpp").read_text(encoding="utf-8")
    body = extract_function(source, MOUSE_MOVE)
    if damage == "stale_parent":
        old = "Sound *const target = win_parent_->legacy_sound_33();"
        assert body.count(old) == 1
        body = body.replace(old, "Sound *const target = parent->legacy_sound_33();", 1)
    elif damage == "retest_reversal":
        old = "field_2148_(field_A18_, position_);"
        assert body.count(old) == 1
        new = (
            "field_2148_(field_A18_, range_reversed_ != 0U "
            "? (range_minimum_ - position_) + range_maximum_ : position_);"
        )
        body = body.replace(old, new, 1)
    elif damage == "unconditional_sound":
        old = (
            "        Sound *const target = win_parent_->legacy_sound_33();\n"
            "        if (target != nullptr && target->is_playing() == 0) {\n"
            "            target->play();\n"
            "        }\n"
            "    }\n"
            "    redraw();"
        )
        new = (
            "    }\n"
            "    Sound *const target = win_parent_->legacy_sound_33();\n"
            "    if (target != nullptr && target->is_playing() == 0) {\n"
            "        target->play();\n"
            "    }\n"
            "    redraw();"
        )
        assert body.count(old) == 1
        body = body.replace(old, new, 1)
    elif damage is not None:
        raise AssertionError(f"unknown damage: {damage}")
    return body


MOUSE_MOVE_HARNESS = r'''#include <cassert>
#include <cstdint>

typedef uint32_t DWORD;
typedef intptr_t LPARAM;
struct RECT { int left, top, right, bottom; };
struct Sound {
    int queries=0, plays=0, playing=0;
    int is_playing(){ ++queries; return playing; }
    void play(){ ++plays; }
};
struct Scroll;
struct Win {
    Sound *sound=nullptr;
    int calls=0, id=0, position=0;
    void on_scrolling(int value_id, int value_position);
    Sound *legacy_sound_33(){ return sound; }
};
typedef void (*ScrollNotification)(int,int);
Win *WinCallbackWindow=nullptr;
Win *WinPointerOwner1=nullptr;
Win *WinPointerOwner2=nullptr;
int WinSizingFlag=0;
int ListBoxGlobalDDC=0;
uintptr_t HandleMain=0;
const int WM_MOUSEMOVE=0x200;
DWORD timeGetTime(){ return 1; }
int get_lbutton_state(){ return 0; }
void get_mouse_pos(int *,int *){}
int PostMessageA(uintptr_t,int,int,LPARAM){ return 1; }

struct Scroll : Win {
    Win *win_parent_=nullptr;
    RECT rect2_{0,0,100,100}, thumb_rect_{0,0,10,10};
    int field_A44_=0, border_color_=-1, position_=3;
    int range_minimum_=0, range_maximum_=10, paging_=2, field_A18_=17;
    int field_A30_=1;
    DWORD field_A38_=0;
    unsigned field_A3C_=0;
    unsigned range_reversed_=0;
    ScrollNotification field_2148_=nullptr;
    struct { struct { int biWidth=100, biHeight=-10; } bmiHeader; } dib_;
    int redraws=0;
    bool in_box(int,int,RECT *){ return true; }
    void redraw(){ ++redraws; }
    void on_mouse_move(int,int);
};

static Scroll *active=nullptr;
static Win *replacement_after_parent=nullptr;
static Win *replacement_after_callback=nullptr;
static bool mutate_parent=false;
static bool mutate_callback=false;
static int callback_calls=0, callback_id=0, callback_position=0;

void Win::on_scrolling(int value_id, int value_position){
    ++calls; id=value_id; position=value_position;
    if(mutate_parent){
        active->position_=7;
        active->range_minimum_=2;
        active->range_maximum_=20;
        active->field_A18_=23;
        active->range_reversed_^=1U;
        active->win_parent_=replacement_after_parent;
    }
}
static void optional_callback(int value_id, int value_position){
    ++callback_calls; callback_id=value_id; callback_position=value_position;
    if(mutate_callback){
        active->position_=9;
        active->range_minimum_=3;
        active->range_maximum_=30;
        active->field_A18_=31;
        active->range_reversed_^=1U;
        active->win_parent_=replacement_after_callback;
    }
}

/* BODY */

static void setup(Scroll &scroll, Win &entry, Win &middle, Win &last,
                  Sound &entry_sound, Sound &middle_sound, Sound &last_sound,
                  int x){
    scroll.win_parent_=&entry;
    scroll.field_2148_=optional_callback;
    entry.sound=&entry_sound; middle.sound=&middle_sound; last.sound=&last_sound;
    active=&scroll; WinPointerOwner1=&scroll; WinPointerOwner2=nullptr;
    WinSizingFlag=1; mutate_parent=true; mutate_callback=true;
    replacement_after_parent=&middle; replacement_after_callback=&last;
    callback_calls=callback_id=callback_position=0;
    scroll.on_mouse_move(x,4);
}

int main(){
    // Equal saved/current position bypasses both notifications and sound.
    {
        Scroll scroll; Win entry; Sound sound; entry.sound=&sound;
        active=&scroll; scroll.win_parent_=&entry; scroll.field_2148_=optional_callback;
        WinPointerOwner1=&scroll; WinSizingFlag=1;
        scroll.on_mouse_move(39,4); // computes position 3, its entry value
        assert(scroll.position_==3 && entry.calls==0 && callback_calls==0);
        assert(sound.queries==0 && sound.plays==0 && scroll.redraws==1);
        assert(WinCallbackWindow==&entry);
    }

    // The reversal arm is chosen before callbacks. Both callbacks reload the
    // live id/position/ranges within that arm, and sound reloads the final
    // parent after the optional callback changes it again.
    for(unsigned reversed=0; reversed!=2; ++reversed){
        Scroll scroll; Win entry, middle, last; Sound a,b,c;
        scroll.range_reversed_=reversed;
        setup(scroll,entry,middle,last,a,b,c,60);
        assert(scroll.position_==9 && entry.calls==1);
        assert(entry.id==17 && entry.position==(reversed ? 4 : 6));
        assert(callback_calls==1 && callback_id==23);
        assert(callback_position==(reversed ? 15 : 7));
        assert(scroll.win_parent_==&last && WinCallbackWindow==&entry);
        assert(a.queries==0 && b.queries==0 && c.queries==1 && c.plays==1);
        assert(scroll.redraws==1);
    }

    // No optional callback means sound follows the parent changed by the
    // virtual notification. Playing and null targets retain their native gate.
    {
        Scroll scroll; Win entry,middle,last; Sound a,b,c;
        scroll.field_2148_=nullptr;
        scroll.win_parent_=&entry; entry.sound=&a; middle.sound=&b; last.sound=&c;
        active=&scroll; WinPointerOwner1=&scroll; WinSizingFlag=1;
        mutate_parent=true; mutate_callback=false; replacement_after_parent=&middle;
        scroll.on_mouse_move(60,4);
        assert(entry.calls==1 && scroll.win_parent_==&middle);
        assert(a.queries==0 && b.queries==1 && b.plays==1 && c.queries==0);
    }
    {
        Scroll scroll; Win entry,middle,last; Sound a,b,c;
        c.playing=1;
        setup(scroll,entry,middle,last,a,b,c,60);
        assert(c.queries==1 && c.plays==0);
    }
    {
        Scroll scroll; Win entry,middle,last; Sound a,b;
        entry.sound=&a; middle.sound=&b; last.sound=nullptr;
        active=&scroll; WinPointerOwner1=&scroll; WinSizingFlag=1;
        mutate_parent=true; mutate_callback=true;
        replacement_after_parent=&middle; replacement_after_callback=&last;
        callback_calls=callback_id=callback_position=0;
        scroll.win_parent_=&entry; scroll.field_2148_=optional_callback;
        scroll.on_mouse_move(60,4);
        assert(entry.calls==1 && callback_calls==1 && scroll.win_parent_==&last);
        assert(a.queries==0 && b.queries==0);
    }
}
'''


def source_for_control(damage=None):
    body = mouse_move_source(damage)
    assert "/* BODY */" in MOUSE_MOVE_HARNESS
    return MOUSE_MOVE_HARNESS.replace("/* BODY */", body, 1)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "stale_parent", "retest_reversal", "unconditional_sound"))
def test_actual_mouse_move_tail_and_damage_controls(tmp_path, compiler, damage):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} unavailable")
    source = tmp_path / "scroll_mouse_move.cpp"
    source.write_text(source_for_control(damage), encoding="utf-8")
    binary = tmp_path / "scroll_mouse_move"
    built = subprocess.run(
        [compiler_path, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary)], capture_output=True, timeout=10)
    assert (result.returncode == 0) == (damage is None), result.stdout + result.stderr


def test_source_uses_distinct_sound_words_only_in_the_pinned_paths():
    auto = (ROOT / "src/autosound.h").read_text(encoding="utf-8")
    defaults = (ROOT / "src/autosound_defaults.h").read_text(encoding="utf-8")
    assert "Sound *legacy_sound_32() const { return val_32_; }" in auto
    assert "Sound *legacy_sound_33() const { return val_33_; }" in auto
    assert "RECOVERY_MEMBER: v3 AutoSound val_32_ Sound* 0x80 0x00606320" in auto
    assert "Sound *val_32_;" in auto and "int val_32_;" not in auto
    assert "Sound *val_32_;" in defaults and "int val_32_;" not in defaults

    source = (ROOT / "src/scroll.cpp").read_text(encoding="utf-8")
    selected = (
        "void Scroll::on_mousewheel_down(int delta)",
        "void Scroll::on_mousewheel_up(int delta)",
        "int Scroll::on_key_click(int, int key)",
        "void Scroll::on_button_clicked(int button_id)",
    )
    for signature in selected:
        body = extract_function(source, signature)
        assert "legacy_sound_32()" in body and "legacy_sound_33()" not in body
    for signature in (MOUSE_MOVE, "int Scroll::on_button_pressed(int button_id)"):
        body = extract_function(source, signature)
        assert "legacy_sound_33()" in body and "legacy_sound_32()" not in body


def test_pinned_sound_offsets_and_mouse_move_gate_reload_order():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    def instruction(address):
        code = image.read(Span(address, address + 8))
        return next(decoder.disasm(code, address))

    for address in (0x606409, 0x606527, 0x6066B4, 0x606C0E):
        item = instruction(address)
        assert item.mnemonic == "mov"
        assert item.op_str.endswith("[eax + 0x80]")
    mouse_sound = instruction(0x6061AB)
    assert mouse_sound.mnemonic == "mov"
    assert mouse_sound.op_str == "edi, dword ptr [eax + 0x84]"
    assert (instruction(0x606120).mnemonic, instruction(0x606120).op_str) == ("cmp", "ecx, eax")
    assert (instruction(0x606122).mnemonic, instruction(0x606122).op_str) == ("je", "0x6061c7")
    assert (instruction(0x606130).mnemonic, instruction(0x606130).op_str) == ("mov", "ecx, dword ptr [esi + 0xc4]")
    assert (instruction(0x6061A5).mnemonic, instruction(0x6061A5).op_str) == ("mov", "eax, dword ptr [esi + 0xc4]")
