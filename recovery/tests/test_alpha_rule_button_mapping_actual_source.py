"""Original fixed rule-button mapping, projected from read_rules' actual tail.

The instrumented row owner stops at an invalid subscript before host UB. This
observes the attempted index; production keeps the original unchecked access.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def block(text, start):
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated actual source")


def actual_tail():
    source = (ROOT / "src/alpha.cpp").read_text()
    body = block(source, source.index("BOOL __cdecl read_rules(BOOL tgl_all_rules)"))
    start = body.index('// Buttons used by "Edit Map" menus.')
    return body[start:body.rindex("return false;") + len("return false;")]


def terraforming_declaration():
    source = (ROOT / "src/terraforming.h").read_text()
    return block(source, source.index("struct RulesTerraforming")) + ";"


PRELUDE = r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
typedef char *LPSTR;
static const int MaxTerrainNum=20;
static const int LM_JUNGLE=2, LM_DUNES=6, LM_URANIUM=3, LM_GEOTHERMAL=10;
@@TERRAFORMING@@
struct BadRead { int index; };
struct TerrainRows {
    RulesTerraforming rows[MaxTerrainNum];
    std::vector<int> reads;
    RulesTerraforming &operator[](int index) {
        reads.push_back(index);
        if(index<0 || index>=MaxTerrainNum) throw BadRead{index};
        return rows[index];
    }
} Terraforming;
struct NaturalRow { LPSTR name, name_short; } Natural[16];
struct Write { int button; std::string text; };
static std::vector<Write> writes;
static std::vector<char> events;
static int scenario;
static char injected_name[]="changed-dunes";
struct StringTableMock {
    char *get(char *name) {
        events.push_back('g');
        return name;
    }
} StringTable;
struct BaseButton {
    int id;
    int set_bubble_text(const char *text) {
        events.push_back('s');
        writes.push_back(Write{id,text});
        if(scenario==3 && id==17) Terraforming.rows[0].bit=0;
        if(scenario==4 && id==32) Natural[LM_DUNES].name_short=injected_name;
        return 0;
    }
};
struct FlatButton : BaseButton {};
struct Interface {
    FlatButton buttons[48];
    FlatButton &flat_button(int index) { return buttons[index]; }
    int set_bubble_text(uint32_t index, const char *text) {
        return buttons[index].set_bubble_text(text);
    }
} MainInterfaceGlobal;
#define require(x) do { if(!(x)) { std::fprintf(stderr,"failed:%d\n",__LINE__); std::exit(1); } } while(0)
bool populate_rule_buttons() {
    int i=0; (void)i;
@@TAIL@@
}
'''

SUFFIX = r'''
static char terrain_names[20][12], natural_names[16][12];
void reset(int which) {
    scenario=which;
    writes.clear(); events.clear(); Terraforming.reads.clear();
    for(int i=0;i<20;++i) {
        std::snprintf(terrain_names[i],12,"terrain-%d",i);
        Terraforming.rows[i].name=terrain_names[i];
        Terraforming.rows[i].bit=1;
    }
    for(int i=0;i<16;++i) {
        std::snprintf(natural_names[i],12,"natural-%d",i);
        Natural[i].name=Natural[i].name_short=natural_names[i];
    }
    for(int i=0;i<48;++i) MainInterfaceGlobal.buttons[i].id=i;
}
void check_result(const int *expected, bool changed_dunes=false) {
    require(writes.size()==19);
    for(int i=0;i<15;++i) {
        require(writes[i].button==i+17);
        require(writes[i].text==terrain_names[expected[i]]);
    }
    const int natural_indices[]={2,6,3,10};
    for(int i=0;i<4;++i) {
        require(writes[i+15].button==i+32);
        require(writes[i+15].text==(changed_dunes && i==1
                ? injected_name : natural_names[natural_indices[i]]));
    }
    require(events.size()==38);
    for(int i=0;i<19;++i) {
        require(events[i*2]=='g' && events[i*2+1]=='s');
    }
    // Pin5881E3..1F7 scans all20bit fields before each selected name read.
    require(Terraforming.reads.size()==315);
    for(int i=0;i<15;++i) {
        for(int row=0;row<20;++row) require(Terraforming.reads[i*21+row]==row);
        require(Terraforming.reads[i*21+20]==expected[i]);
    }
}
int main() {
    // Five distributed holes: later zero rows must not shift earlier ordinals.
    reset(0);
    const int holes[]={1,4,8,12,18};
    for(int hole:holes) Terraforming.rows[hole].bit=0;
    const int distributed[]={0,2,3,5,6,7,9,10,11,13,14,15,16,17,19};
    require(!populate_rule_buttons()); check_result(distributed);
    // With20nonzero rows the original still updates only15terraform buttons.
    reset(1);
    const int no_holes[]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14};
    require(!populate_rule_buttons()); check_result(no_holes);
    reset(2);
    for(int row=0;row<5;++row) Terraforming.rows[row].bit=0;
    const int leading[]={5,6,7,8,9,10,11,12,13,14,15,16,17,18,19};
    require(!populate_rule_buttons()); check_result(leading);
    // A completed button callback changes an already-passed row. The next
    // button recomputes from entry0; a single filtered walk would choose row1.
    reset(3);
    const int callback[]={0,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    require(!populate_rule_buttons()); check_result(callback);
    // The four Natural source fields remain live across preceding callbacks.
    reset(4);
    require(!populate_rule_buttons()); check_result(no_holes,true);
    // Six leading holes select row20 for button31. Stop at the attempted
    // subscript; do not execute undefined host memory access or add a product guard.
    reset(5);
    for(int row=0;row<6;++row) Terraforming.rows[row].bit=0;
    int bad=-1;
    try { populate_rule_buttons(); } catch(BadRead hit) { bad=hit.index; }
    require(bad==20);
    require(writes.size()==14 && events.size()==28);
    for(int i=0;i<14;++i) {
        require(writes[i].button==i+17);
        require(writes[i].text==terrain_names[i+6]);
    }
    require(Terraforming.reads.back()==20);
    return 0;
}
'''


def damage_tail(tail, damage):
    if damage == "old_filtered_walk":
        start = tail.index("for (int button = 17;")
        current = block(tail, start)
        old = '''for (i = 0; i < MaxTerrainNum; ++i) {
            if (Terraforming[i].bit) {
                MainInterfaceGlobal.set_bubble_text(j++ + 17,
                    StringTable.get(Terraforming[i].name));
            }
        }'''
        return tail[:start] + "int j=0;\n" + old + tail[start + len(current):]
    if damage == "skip_late_hole_test":
        old = "Terraforming[entry].bit == 0 && terrain >= entry"
        assert old in tail
        return tail.replace(old, "Terraforming[entry].bit == 0")
    if damage == "omit_last_slot":
        assert "button <= 31" in tail
        return tail.replace("button <= 31", "button <= 30")
    if damage == "cache_next_natural":
        anchor = "MainInterfaceGlobal.flat_button(32).set_bubble_text"
        assert anchor in tail
        return tail.replace(anchor, "char *cached_dunes = Natural[LM_DUNES].name_short;\n    " + anchor).replace(
            "StringTable.get(Natural[LM_DUNES].name_short)", "StringTable.get(cached_dunes)")
    raise AssertionError(damage)


def run_tail(tmp_path, compiler_name, tail):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    cpp = tmp_path / "rule_buttons.cpp"
    exe = tmp_path / "rule_buttons"
    cpp.write_text(PRELUDE.replace("@@TERRAFORMING@@", terraforming_declaration()).replace("@@TAIL@@", tail) + SUFFIX)
    built = subprocess.run([compiler,"-std=c++11","-O0","-Wall","-Wextra","-Werror",str(cpp),"-o",str(exe)], capture_output=True,text=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_fixed_mapping_holes_counts_and_callback_reads(tmp_path, compiler_name):
    result = run_tail(tmp_path, compiler_name, actual_tail())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["old_filtered_walk", "skip_late_hole_test", "omit_last_slot", "cache_next_natural"])
def test_rule_button_mapping_damage_is_observed(tmp_path, compiler_name, damage):
    tail = actual_tail()
    damaged = damage_tail(tail,damage)
    assert tail != damaged
    result = run_tail(tmp_path,compiler_name,damaged)
    assert result.returncode == 1, result.stderr


def test_pinned_fixed_mapping_and_actual_button_calls():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path=Path(os.environ.get("OPENSMACX_IMAGE", ROOT/".opensmacx/game/terranx_original.exe"))
    image=PeImage(path.read_bytes(),PIN)
    ins=list(Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x5881CF,0x588293)),0x5881CF))
    rows={i.address:(i.mnemonic,i.op_str) for i in ins}
    assert rows[0x5881CF] == ("mov","edi, 0x11")
    assert rows[0x5881D4] == ("mov","esi, 0x7bb62c")
    assert rows[0x5881D9] == ("lea","eax, [edi - 0x11]")
    assert rows[0x5881DE] == ("mov","ecx, 0x691888")
    assert rows[0x5881E3] == ("cmp","dword ptr [ecx], 0")
    assert rows[0x5881E6] == ("jne","0x5881ed")
    assert rows[0x5881E8] == ("cmp","eax, edx")
    assert rows[0x5881EA] == ("jl","0x5881ed")
    assert rows[0x5881EC] == ("inc","eax")
    assert rows[0x5881ED] == ("add","ecx, 0x20")
    assert rows[0x5881F1] == ("cmp","ecx, 0x691b08")
    assert (0x691B08-0x691888)//0x20 == 20
    assert rows[0x5881F9] == ("shl","eax, 5")
    assert rows[0x588201] == ("mov","eax, dword ptr [eax + 0x691878]")
    assert rows[0x588215] == ("add","esi, 0xb4c")
    assert rows[0x58821C] == ("cmp","esi, 0x7c5454")
    assert rows[0x588222] == ("jle","0x5881d9")
    assert (0x7C5454-0x7BB62C)//0xB4C+1 == 15
    calls=[i.address for i in ins if i.mnemonic=="call" and i.op_str=="0x607550"]
    assert calls == [0x588210,0x58823B,0x588257,0x588272,0x58828E]
    for name,value in (("LM_JUNGLE",2),("LM_DUNES",6),("LM_URANIUM",3),("LM_GEOTHERMAL",10)):
        assert re.search(rf"\b{name}\s*=\s*{value}\s*[,}}]",(ROOT/"src/map.h").read_text())
