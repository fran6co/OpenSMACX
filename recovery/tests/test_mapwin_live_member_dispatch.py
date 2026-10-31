"""Actual global broadcasters: member gates and callback-visible table changes."""

import os
from pathlib import Path
import re
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]


def broadcaster(name):
    text = (ROOT / "src/mapwin.cpp").read_text()
    match = re.search(r"^void __cdecl " + name + r"\([^;{}]*\)\s*\{", text, re.M)
    assert match, name
    start = match.start()
    brace = text.index("{", start)
    depth = 0
    for end in range(brace, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated broadcaster")


def test_global_broadcasters_are_declared_in_header_and_owned_by_mapwin_cpp():
    header = (ROOT / "src/mapwin.h").read_text()
    source = (ROOT / "src/mapwin.cpp").read_text()
    for name, address in (("draw_tile", "0x0046AF40"), ("draw_tiles", "0x0046B140")):
        assert f"void __cdecl {name}(int x_coord, int y_coord, int draw_type);" in header
        assert f"MEASURED inline void __cdecl {name}(" not in header
        marker = source.index(f"// ORIGINAL: {address} ?{name}@@YAXHHH@Z")
        body = source.index(f"void __cdecl {name}(", marker)
        assert body > marker
        assert broadcaster(name).startswith(f"void __cdecl {name}(")


def test_pinned_broadcaster_entries_dispatch_through_mapwin_radius():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    records = {row.address: row for row in read_file(ROOT / "src/mapwin.cpp")}
    for entry, pc in ((0x0046AF40, 0x0046AF71), (0x0046B140, 0x0046B171)):
        record = records[entry]
        spans = record.image_spans
        assert spans
        instructions = {
            instruction.address: instruction
            for start, end in spans
            for instruction in decoder.disasm(image.read(Span(start, end)), start)
        }
        assert any(start <= pc < end for start, end in spans)
        assert instructions[pc].mnemonic == "call"
        assert instructions[pc].op_str == "0x46a2a0"


PREFIX = r'''
#include <stdint.h>
#include <stddef.h>
#define MEASURED
#define __cdecl
struct MapWin {
    uint32_t view_active_;
    int id;
    void draw_radius(int, int, int, int);
};
MapWin *MapWinTable[8];
const size_t MapWinTableSlots = 8;
MapWin replacement = { 0xffffffffu, 4 };
int calls[8], total, expected_radius;
void MapWin::draw_radius(int x, int y, int radius, int flags) {
    if (x != 12 || y != 34 || radius != expected_radius || flags != 7)
        total = -100;
    ++calls[id]; ++total;
    if (id == 1) MapWinTable[3] = &replacement;
}
'''


def run_control(tmp_path, compiler, name, body, null_primary=False):
    primary = "nullptr" if null_primary else "&first"
    expected = "0" if null_primary else "1"
    main = f'''
int main() {{
    MapWin first = {{ 0, 0 }}, active = {{ 2, 1 }};
    MapWin inactive = {{ 0, 2 }}, displaced = {{ 1, 3 }};
    MapWinTable[0] = {primary};
    MapWinTable[1] = &active;
    MapWinTable[2] = &inactive;
    MapWinTable[3] = &displaced;
    expected_radius = {int(name == 'draw_tiles')};
    {name}(12, 34, 7);
    return calls[0] == {expected} && calls[1] == 1 && calls[2] == 0
        && calls[3] == 0 && calls[4] == 1 && total == {2 + int(not null_primary)}
        ? 0 : 1;
}}
'''
    cpp = tmp_path / "control.cpp"
    exe = tmp_path / "control"
    cpp.write_text(PREFIX + body + main)
    subprocess.run([compiler, "-std=c++11", "-O2", str(cpp), "-o", str(exe)],
                   check=True, capture_output=True)
    return subprocess.run([str(exe)], capture_output=True).returncode


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("name", ["draw_tile", "draw_tiles"])
def test_actual_broadcaster_primary_gate_and_live_table(tmp_path, compiler, name):
    body = broadcaster(name)
    assert "static_cast<const volatile MapWin *>(window)->view_active_" in body
    assert run_control(tmp_path, compiler, name, body) == 0
    assert run_control(tmp_path, compiler, name, body, null_primary=True) == 0


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("name", ["draw_tile", "draw_tiles"])
def test_controls_reject_activity_and_primary_gate_damage(tmp_path, compiler, name):
    body = broadcaster(name)
    for old, new in [("if (slot != 0)", "if (true)"),
                     ("if (active == 0)", "if (active != 0)")]:
        mutant = body.replace(old, new)
        assert mutant != body
        assert run_control(tmp_path, compiler, name, mutant) != 0


def test_pinned_activation_reloads_slot_after_callbacks():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original required")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    instructions = {i.address: i for i in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
        image.read(Span(0x470FA0, 0x47102D)), 0x470FA0)}
    for address in (0x470FD0, 0x470FE3, 0x470FF3):
        assert instructions[address].mnemonic == "mov"
        assert instructions[address].op_str == "ecx, dword ptr [esi*4 + 0x7d3c3c]"
    for address, target in ((0x470FDE, "0x462870"), (0x470FEE, "0x470a90")):
        assert instructions[address].mnemonic == "call"
        assert instructions[address].op_str == target


ACTIVATION_PREFIX = r'''
#include <cassert>
#include <cstddef>
#define __cdecl
struct MapWin;
MapWin *MapWinTable[8];
const size_t MapWinTableSlots=8;
MapWin *old_window;
int selected_slot, events;
struct MapWin {
    int view_active_, map_view_index_, center_x_, center_y_, id;
    void clear(int index);
    void init(int mode,int update);
    void show(int mode);
};
MapWin replacement, final_window;
void MapWin::clear(int index) {
    assert(events==0 && this==old_window && index==map_view_index_);
    ++events; MapWinTable[selected_slot]=&replacement;
}
void MapWin::init(int mode,int update) {
    assert(events==1 && this==&replacement && mode==0 && update==1);
    ++events; MapWinTable[selected_slot]=&final_window;
}
void MapWin::show(int mode) {
    assert(events==2 && this==&final_window && mode==0);
    assert(center_x_==-17 && center_y_==29);
    ++events;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "cached_init", "cached_coordinates", "active_gate"])
def test_activation_observes_slot_replacement_after_callbacks(tmp_path, compiler, damage):
    body = broadcaster("mapwin_activate_free_slot")
    replacements = {
        "cached_init": ("MapWinTable[slot]->init(0, 1);", "old_window->init(0, 1);"),
        "cached_coordinates": ("MapWinTable[slot]->center_x_ = tile_x;", "old_window->center_x_ = tile_x;"),
        "active_gate": ("->view_active_ != 0", "->view_active_ == 0"),
    }
    if damage is not None:
        before, after = replacements[damage]
        assert body.count(before) == 1
        body = body.replace(before, after)
    main = r'''
int main() {
    MapWin active={1,5,0,0,1}, available={0,6,0,0,2}, later={0,7,0,0,3};
    for(int slot=1;slot<8;++slot) {
        for(int i=0;i<8;++i) MapWinTable[i]=0;
        for(int i=1;i<slot;++i) if(i%2) MapWinTable[i]=&active;
        MapWinTable[slot]=&available;
        if(slot+1<8) MapWinTable[slot+1]=&later;
        selected_slot=slot; old_window=&available; events=0;
        final_window.center_x_=final_window.center_y_=0;
        mapwin_activate_free_slot(-17,29);
        assert(events==3 && MapWinTable[slot]==&final_window);
        assert(available.center_x_==0 && replacement.center_x_==0);
    }
    for(int i=0;i<8;++i) MapWinTable[i]=i%2 ? &active : 0;
    events=0; mapwin_activate_free_slot(-17,29); assert(events==0);
}
'''
    cpp = tmp_path / "activation.cpp"
    exe = tmp_path / "activation"
    cpp.write_text(ACTIVATION_PREFIX + body + main)
    built = subprocess.run([compiler, "-std=c++11", "-O2", str(cpp), "-o", str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(exe)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)


def actual_member(signature):
    source = (ROOT / 'src/mapwin.cpp').read_text()
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(signature)


BUTTON_PREFIX = r'''
#include <cassert>
#include <cstring>
#include <vector>
typedef char *LPSTR;
std::vector<int> events;
char StringTemp[80], world[]="world", detail[]="detail";
struct LabelsOwner { char *strings_ptr[512]; } Labels;
struct Strings { char *get(char *text) { return text; } } StringTable;
struct Panel { int id; void reset() { events.push_back(id); } };
Panel ConsoleInfoWin={0},ConsoleStatusWin={1};
int zoom_code, zoom_index, closes, captions;
struct Console { void zoom(int code,int index) { zoom_code=code;zoom_index=index;events.push_back(3); } } ConsoleGlobal;
struct Win { void set_caption(const char *text) { ++captions; events.push_back(4); assert(text==StringTemp); } };
struct MapWin : Win {
    unsigned field_1DD70_;
    int map_view_index_, view_active_;
    void close() { ++closes; events.push_back(2); }
    void standard_caption();
    void on_button_clicked(int button);
};
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [None, "caption_omitted", "toggle", "caption_polarity"])
def test_actual_button_caption_reuses_original_label_behavior(tmp_path, compiler, damage):
    body = actual_member('void MapWin::standard_caption()') + '\n' + actual_member(
        'void MapWin::on_button_clicked(int button)')
    mutations = {
        'caption_omitted': ('standard_caption();', '(void)0;'),
        'toggle': ('field_1DD70_ ^= 0x40000000U;', 'field_1DD70_ ^= 0x20000000U;'),
        'caption_polarity': ('(field_1DD70_ & 0x40000000U) != 0', '(field_1DD70_ & 0x40000000U) == 0'),
    }
    if damage is not None:
        before, after = mutations[damage]
        assert body.count(before) == 1
        body = body.replace(before, after)
    main = r'''
int main() {
    Labels.strings_ptr[0x100]=world; Labels.strings_ptr[0x101]=detail;
    for(int index=0;index<2;++index) for(int flag=0;flag<2;++flag)
    for(int button=-1;button<=4;++button) {
        unsigned initial=flag ? 0x40000000U : 0;
        MapWin window; window.field_1DD70_=initial; window.map_view_index_=index; window.view_active_=1;
        events.clear(); closes=captions=0; zoom_code=zoom_index=-1;
        std::strcpy(StringTemp,"stale"); window.on_button_clicked(button);
        assert(events.size()>=2 && events[0]==0 && events[1]==1);
        assert(closes==(button==0 && index!=0));
        assert(window.view_active_==((button==0 && index!=0) ? 0 : 1));
        if(button==1 || button==2) { assert(zoom_code==0x202+button && zoom_index==index); }
        else assert(zoom_code==-1);
        unsigned expected=(button==3 && index!=0) ? initial^0x40000000U : initial;
        assert(window.field_1DD70_==expected);
        assert(captions==(button==3 && index!=0));
        if(captions) assert(std::strcmp(StringTemp,(expected & 0x40000000U) ? "world" : "detail")==0);
        else assert(std::strcmp(StringTemp,"stale")==0);
        assert(events.size()==((button==1 || button==2 || ((button==0 || button==3) && index!=0)) ? 3u : 2u));
    }
}
'''
    cpp = tmp_path / 'buttons.cpp'
    exe = tmp_path / 'buttons'
    cpp.write_text(BUTTON_PREFIX + body + main)
    built = subprocess.run([compiler, '-std=c++11', '-O2', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(exe)], capture_output=True)
    assert (result.returncode == 0) == (damage is None)
