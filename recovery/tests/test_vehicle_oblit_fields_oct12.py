"""Bounded actual-source obliteration selectors and call timing, not game runtime proof."""
from __future__ import annotations

import hashlib
import os
import shutil
import subprocess
from pathlib import Path

import pytest
from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SPAN = Span(0x004CD4E0, 0x004CD69D)


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _body() -> str:
    return _definition((ROOT / "src/veh.cpp").read_text(),
                       "void __cdecl action_oblit(int veh_id, int base_id)")


PREFIX = r'''
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#define __cdecl
typedef int BOOL;
ACTUAL_DECLARATIONS
static_assert(sizeof(Player)==0x59c, "actual Player row");
static_assert(offsetof(Player,is_leader_female)==0, "leader gender");
static_assert(offsetof(Player,name_leader)==0x34, "leader name");
static_assert(offsetof(Player,title_leader)==0x4c, "leader title");
static_assert(offsetof(Player,adj_name_faction)==0x304, "faction adjective");
static_assert(offsetof(Player,faction_bonus_count)==0x50c, "distinct bonus count");
static_assert(sizeof(Veh)==0x34 && offsetof(Veh,faction_id)==0xe, "vehicle row");
static_assert(sizeof(Base)==0x134 && offsetof(Base,name_string)==0x13, "base row");
static_assert(offsetof(RulesBasic,tgl_oblit_base_atrocity)==0x128, "rule field");
Player Players[8]; Veh Vehs[8]; Base Bases[8]; RulesBasic Rules;
// The console and external callees are behavioral doubles, not ABI/layout mocks.
struct ConsoleDouble { int faction_id_; } ConsoleGlobal;
int GenderDefault, PluralityDefault, scenario;
const char *ScriptPopupFile="script";
const char VehLabelOBLITTED[]="OBLITTED", VehLabelOBLITTED2[]="OBLITTED2";
const char VehIconBASEOBL_SM_PCX[]="baseobl_sm.pcx";
const char VehIconBASEOBL2_SM_PCX[]="baseobl2_sm.pcx";
struct Event { int kind, index, gender, plural; const char *text; };
std::vector<Event> events;
int failed, popup_kind, atrocity_count;
void check(bool condition) { if (!condition) failed=1; }
void parse_num(int index, int number) {
    check(index==0 && number==-60);
    events.push_back({0,index,number,0,0});
    if (scenario==1) {
        Vehs[2].faction_id=4; Bases[1].faction_id_former=4;
        Bases[1].population_size=7;
    }
}
void parse_says(int index, const char *text, int gender, int plural) {
    check(gender==-1 && plural==-1);
    events.push_back({1,index,GenderDefault,PluralityDefault,text});
    if (scenario==1) {
        if (index==0) ConsoleGlobal.faction_id_=2;
        if (index==1) {
            ConsoleGlobal.faction_id_=6;
            GenderDefault=901; PluralityDefault=902;
        }
        if (index==3) Players[3].is_leader_female=33;
        if (index==4) Bases[1].faction_id_current=6;
    }
}
void base_kill(int index) {
    check(index==1);
    events.push_back({2,index,0,0,0});
    // Clear the source record after the native audible snapshot, as a timing
    // control. Callback mutations assert source ordering, not native reentry.
    Bases[1].faction_id_current=7; Bases[1].visibility=0;
    Bases[1].faction_id_former=7;
    if (scenario==1) ConsoleGlobal.faction_id_=4;
}
void draw_map(int mode) {
    check(mode==1); events.push_back({3,mode,0,0,0});
    if (scenario==1) ConsoleGlobal.faction_id_=3;
    if (scenario==2) ConsoleGlobal.faction_id_=6;
    if (scenario==3) Rules.tgl_oblit_base_atrocity=-1;
}
void popp(const char *file, const char *label, int value, const char *icon, int end) {
    check(file==ScriptPopupFile && value==0 && end==0);
    popup_kind=label==VehLabelOBLITTED ? 1 : label==VehLabelOBLITTED2 ? 2 : -1;
    check(icon==(popup_kind==1 ? VehIconBASEOBL_SM_PCX : VehIconBASEOBL2_SM_PCX));
    events.push_back({4,popup_kind,0,0,0});
    if (scenario==1) Rules.tgl_oblit_base_atrocity=-1;
}
void atrocity(int actor, int former, int kind, int value) {
    check(actor==3 && former==5 && kind==2 && value==0);
    ++atrocity_count; events.push_back({5,actor,former,kind,0});
}
'''

MAIN = r'''
void reset(int mode, int console, int current, int visibility, int rule) {
    std::memset(Players,0,sizeof Players); std::memset(Vehs,0,sizeof Vehs);
    std::memset(Bases,0,sizeof Bases); std::memset(&Rules,0,sizeof Rules);
    for (int i=0;i<8;++i) {
        Players[i].is_leader_female=20+i;
        Players[i].faction_bonus_count=100+i;
        std::snprintf(Players[i].name_leader,24,"name-%d",i);
        std::snprintf(Players[i].title_leader,24,"title-%d",i);
        std::snprintf(Players[i].adj_name_faction,128,"adjective-%d",i);
    }
    Vehs[2].faction_id=3; Bases[1].faction_id_former=5;
    Bases[1].faction_id_current=current; Bases[1].visibility=visibility;
    Bases[1].population_size=-6; std::strcpy(Bases[1].name_string,"base-name");
    ConsoleGlobal.faction_id_=console; Rules.tgl_oblit_base_atrocity=rule;
    GenderDefault=77; PluralityDefault=88; scenario=mode;
    events.clear(); failed=popup_kind=atrocity_count=0;
}
void verify(int console, bool callback, int popup, int atrocity_expected) {
    check(events.size()==static_cast<unsigned>(8+(popup!=0)+atrocity_expected));
    for (int i=0;i<5;++i) check(events[i+1].kind==1 && events[i+1].index==i);
    check(events[1].text==Bases[1].name_string && events[1].gender==77
          && events[1].plural==88);
    check(events[2].text==Players[console].title_leader
          && events[2].gender==20+console && events[2].plural==0);
    check(events[3].text==Players[5].adj_name_faction
          && events[3].gender==(callback?901:20+console)
          && events[3].plural==(callback?902:0));
    check(events[4].text==Players[3].title_leader
          && events[4].gender==23 && events[4].plural==0);
    check(events[5].text==Players[3].name_leader
          && events[5].gender==(callback?33:23) && events[5].plural==0);
    check(events[6].kind==2 && events[7].kind==3);
    check(popup_kind==popup && atrocity_count==atrocity_expected);
    if(popup) check(events[8].kind==4);
    if(atrocity_expected) check(events.back().kind==5);
}
int main() {
    // Actor sees OBLITTED; original signed population and nonzero rule survive.
    reset(0,3,7,0,-1); action_oblit(2,1); verify(3,false,1,1); if(failed) return 1;
    // Former owner, visible observer, current owner, and silent observer.
    reset(0,5,7,0,0); action_oblit(2,1); verify(5,false,2,0); if(failed) return 2;
    reset(0,1,7,2,0); action_oblit(2,1); verify(1,false,2,0); if(failed) return 3;
    reset(0,1,1,0,0); action_oblit(2,1); verify(1,false,2,0); if(failed) return 4;
    reset(0,1,7,0,0); action_oblit(2,1); verify(1,false,0,0); if(failed) return 5;
    // Selection IDs captured early; grammar reloads; popup uses post-draw console;
    // the atrocity rule is read after the popup callback.
    reset(1,1,7,0,0); action_oblit(2,1); verify(2,true,1,1); if(failed) return 6;
    // An audible snapshot survives base deletion and a changed popup console.
    reset(2,1,7,2,0); action_oblit(2,1); verify(1,false,2,0); if(failed) return 7;
    // No popup, but the atrocity rule still observes the preceding draw call.
    reset(3,1,7,0,0); action_oblit(2,1); verify(1,false,0,1); if(failed) return 8;
    return 0;
}
'''


def _fixture(body: str) -> str:
    declarations = []
    for file, signature in (("faction.h", "struct Player {"),
                            ("veh.h", "struct Veh {"),
                            ("base.h", "struct Base {"),
                            ("alpha.h", "struct RulesBasic {")):
        declarations.append(_definition((ROOT / "src" / file).read_text(), signature) + ";")
    return PREFIX.replace("ACTUAL_DECLARATIONS", "\n".join(declarations)) + body + MAIN


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("optimization", ("-O0", "-O2"))
@pytest.mark.parametrize("damage", (None, "gender", "title", "actor", "former",
                                    "grammar_reload", "audible", "popup_faction", "rule"))
def test_actual_action_oblit_selectors_and_compiling_mutants(
        tmp_path: Path, compiler: str, optimization: str, damage: str | None) -> None:
    if not shutil.which(compiler):
        pytest.skip(f"{compiler} unavailable")
    body = _body()
    if damage == "gender":
        body = body.replace(".is_leader_female", ".faction_bonus_count")
    elif damage == "title":
        body = body.replace("Players[ConsoleGlobal.faction_id_].title_leader",
                            "Players[ConsoleGlobal.faction_id_].name_leader")
    elif damage == "actor":
        body = body.replace("Players[veh_faction]", "Players[Vehs[2].faction_id]")
    elif damage == "former":
        body = body.replace("Players[base_former].adj_name_faction",
                            "Players[Bases[base_id].faction_id_former].adj_name_faction")
    elif damage == "grammar_reload":
        needle = "    GenderDefault = Players[veh_faction].is_leader_female;"
        assert body.count(needle) == 2
        at = body.rindex(needle)
        body = body[:at] + body[at:].replace(needle, "", 1)
    elif damage == "audible":
        start = body.index("    const bool audible =")
        end = body.index("\n\n", start)
        capture = body[start:end]
        body = body[:start] + body[end:]
        body = body.replace("    draw_map(1);", "    draw_map(1);\n" + capture)
    elif damage == "popup_faction":
        body = body.replace("    base_kill(base_id);",
                            "    const int stale_console = ConsoleGlobal.faction_id_;\n    base_kill(base_id);")
        body = body.replace("== ConsoleGlobal.faction_id_) {", "== stale_console) {")
    elif damage == "rule":
        body = body.replace("    base_kill(base_id);",
                            "    const int stale_rule = Rules.tgl_oblit_base_atrocity;\n    base_kill(base_id);")
        body = body.replace("if (Rules.tgl_oblit_base_atrocity != 0)", "if (stale_rule != 0)")
    if damage:
        assert body != _body(), f"mutant did not change source: {damage}"
    cpp, exe = tmp_path / "oblit.cpp", tmp_path / "oblit"
    cpp.write_text(_fixture(body))
    subprocess.run([compiler, "-std=c++11", optimization, "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True)
    result = subprocess.run([str(exe)], check=False)
    assert (result.returncode != 0) if damage else (result.returncode == 0)


def test_optional_pinned_oblit_fields_and_bounded_call_order() -> None:
    image_value = os.environ.get("OPENSMACX_IMAGE")
    if not image_value:
        pytest.skip("set OPENSMACX_IMAGE explicitly for pinned-image control")
    path = Path(image_value)
    assert path.is_absolute() and path.is_file()
    data = path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    image = PeImage(data, PIN)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(SPAN), SPAN.start))
    assert sum(i.size for i in instructions) == SPAN.end - SPAN.start
    native = {i.address: (i.mnemonic, i.op_str) for i in instructions}
    for address, expected in {
        0x004CD4FA: ("mov", "bl, byte ptr [edx*4 + 0x952836]"),
        0x004CD509: ("movsx", "eax, byte ptr [edi + 0x97d046]"),
        0x004CD510: ("mov", "dl, byte ptr [edi + 0x97d045]"),
        0x004CD55C: ("mov", "edx, dword ptr [eax + 0x946a50]"),
        0x004CD562: ("lea", "eax, [eax + 0x946a9c]"),
        0x004CD585: ("lea", "edx, [ecx*4 + 0x946d54]"),
        0x004CD5B0: ("mov", "eax, dword ptr [esi + 0x946a50]"),
        0x004CD5B6: ("lea", "ecx, [esi + 0x946a9c]"),
        0x004CD5C9: ("mov", "edx, dword ptr [esi + 0x946a50]"),
        0x004CD5D2: ("lea", "eax, [esi + 0x946a84]"),
        0x004CD5F9: ("mov", "cl, byte ptr [edi + 0x97d044]"),
        0x004CD609: ("mov", "al, byte ptr [edi + 0x97d04a]"),
        0x004CD635: ("mov", "eax, dword ptr [0x939284]"),
        0x004CD67E: ("mov", "eax, dword ptr [0x949860]"),
        0x004CD69C: ("ret", ""),
    }.items():
        assert native[address] == expected
    calls = [(i.address, i.op_str) for i in instructions if i.mnemonic == "call"]
    assert calls == [(0x004CD523, "0x625e30"), (0x004CD535, "0x625ec0"),
                     (0x004CD571, "0x625ec0"), (0x004CD58F, "0x625ec0"),
                     (0x004CD5C4, "0x625ec0"), (0x004CD5EF, "0x625ec0"),
                     (0x004CD629, "0x4e5250"), (0x004CD630, "0x46b190"),
                     (0x004CD676, "0x48c0a0"), (0x004CD690, "0x55eee0")]
