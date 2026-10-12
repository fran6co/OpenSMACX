"""Full actual kill body with allocated host owners and dependency controls.

Pinned instructions establish the native branches, capture widths and ordering.
Host dependency shims exercise the complete source operation, not native ABI,
the native memcpy provider, linked preservation, or game acceptance.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest

from test_netdaemon_early_routes_oct12 import declaration, PIN
from test_netdaemon_synch_dispatch import _definition

ROOT = Path(__file__).resolve().parents[2]


def material():
    source = (ROOT / "src/veh.cpp").read_text()
    body = _definition(source, r"void\s+__cdecl\s+kill", "kill")
    header = (ROOT / "src/veh.h").read_text()
    records = "\n".join(declaration(header, name) for name in ("Veh", "VehPrototype", "RulesWeapon"))
    map_header = (ROOT / "src/map.h").read_text()
    records += "\n" + declaration(map_header, "Map")
    map_loc = _definition(map_header, r"inline\s+Map\s*\*\s*__cdecl\s+map_loc", "map_loc")
    return records, map_loc, body


HARNESS = r'''
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <string>
#include <vector>
#define __cdecl
typedef char *LPSTR;
/* ACTUAL RECORDS */
static_assert(sizeof(Veh)==0x34 && sizeof(VehPrototype)==0x34 && sizeof(Map)==0x2c,"passive widths");
static Veh Vehs[8];
static VehPrototype VehPrototypes[128];
static RulesWeapon Weapon[26];
static Map tiles[32];
static int MapLatitudeBounds=8, MapLongitudeBounds=8, MapLongitude=4;
static int VehBattleUnkTgl[2];
static struct { int faction_id_; } ConsoleGlobal;
static std::vector<std::string> events;
static std::function<void(const std::string&)> callback;
static int ability_result, stack_result, tile_reads;
static int ability_proto, base_arg, at_x, at_y, checked_faction, draw_x, draw_y;
static void event(const std::string&s) { events.push_back(s); if(callback)callback(s); }
static Map *map_tiles() { ++tile_reads; return tiles; }
/* ACTUAL MAP LOC */
static int has_abil(int prototype,int ability) {
    assert(ability==2); ability_proto=prototype; event("ability"); return ability_result;
}
static void veh_kill(int id) { assert(id==2); event("delete"); }
static void stack_veh(int id,int mode) { assert(id==2 && mode==2); event("stack"); }
static void stack_kill(int id) { assert(id==2); event("stack_delete"); }
static int veh_at(int x,int y) { at_x=x; at_y=y; event("at"); return 6; }
static int stack_check(int id,int mode,int faction,int a,int b) {
    assert(id==6 && mode==1 && a==-1 && b==-1); checked_faction=faction;
    event("check"); return stack_result;
}
static void draw_radius(int x,int y,int mode,int detail) {
    assert(mode==2 && detail==2); draw_x=x; draw_y=y; event("radius");
}
static void draw_tile(int x,int y,int detail) {
    assert(detail==2); draw_x=x; draw_y=y; event("tile");
}
static struct Tutorial {
    void check_base(int base) { base_arg=base; event("tutorial"); }
} TutWinBaseWindow;
/* ACTUAL KILL */
static void reset() {
    callback={}; events.clear(); std::memset(Vehs,0,sizeof Vehs);
    std::memset(VehPrototypes,0,sizeof VehPrototypes); std::memset(Weapon,0,sizeof Weapon);
    std::memset(tiles,0,sizeof tiles); VehBattleUnkTgl[0]=VehBattleUnkTgl[1]=0;
    MapLatitudeBounds=8; MapLongitudeBounds=8; MapLongitude=4;
    Vehs[2].x=2; Vehs[2].y=2; Vehs[2].proto_id=9; Vehs[2].faction_id=3;
    Vehs[2].home_base_id=5; Vehs[2].morale=255;
    VehPrototypes[9].weapon_id=4; Weapon[4].offense_rating=1;
    ConsoleGlobal.faction_id_=3; ability_result=0; stack_result=0; tile_reads=0;
    ability_proto=base_arg=at_x=at_y=checked_faction=draw_x=draw_y=-99;
}
static void expect(std::initializer_list<const char*> list) {
    assert(events.size()==list.size()); size_t i=0;
    for(const char*s:list)assert(events[i++]==s);
}
int main() {
    // Zero capacity never reads terrain and joins the common tutorial/draw tail.
    reset(); kill(2); expect({"ability","delete","tutorial","at","check","radius"});
    assert(tile_reads==0 && base_arg==5 && at_x==2 && at_y==2 && checked_faction==3);
    // All nonzero-capacity paths at/above the native mask threshold use the
    // same ordinary deletion and common tail, rather than returning early.
    for(int climate : {0x60,0x63,0x80,0xe0}) {
        reset(); VehPrototypes[9].carry_capacity=1; tiles[9].climate=climate;
        kill(2); expect({"ability","delete","tutorial","at","check","radius"});
        assert(tile_reads==1 && base_arg==5);
    }
    for(int climate : {0,0x20,0x40,0x5f}) {
        reset(); VehPrototypes[9].carry_capacity=1; tiles[9].climate=climate;
        kill(2); expect({"ability","stack","stack_delete","tutorial","at","check","radius"});
    }
    // Prototype64 bypasses the basic-unit weapon special case. Prototype15,
    // negative signed weapon offense and unsigned morale retain their native
    // nonzero cargo route; no clamp/normalization is introduced.
    for(int prototype : {9,15,64}) {
        reset(); Vehs[2].proto_id=prototype; VehPrototypes[prototype].carry_capacity=255;
        VehPrototypes[prototype].weapon_id=4; Weapon[4].offense_rating=-1;
        Vehs[2].morale=255; kill(2);
        expect({"ability","stack","stack_delete","tutorial","at","check","radius"});
    }
    // Off-map x=-2 is the sole cargo-stack return before tutorial/draw.
    reset(); VehPrototypes[9].carry_capacity=1; Vehs[2].x=-2;
    kill(2); expect({"ability","stack_delete"}); assert(tile_reads==0 && base_arg==-99);
    // The same sentinel with zero capacity still follows ordinary deletion.
    reset(); Vehs[2].x=-2; kill(2); expect({"ability","delete","tutorial"});
    // Other invalid coordinates still stack/delete and run the tutorial tail.
    for(int axis : {0,1}) for(int coordinate : {-1,8}) {
        reset(); VehPrototypes[9].carry_capacity=1;
        if(axis==0)Vehs[2].x=coordinate;else Vehs[2].y=coordinate;
        kill(2); expect({"ability","stack","stack_delete","tutorial"}); assert(tile_reads==0);
    }
    // The ability argument uses the old prototype, while dispatch and tutorial
    // use the prototype/home reloaded afterward. Coordinates/faction stay old.
    reset(); VehPrototypes[9].carry_capacity=1;
    callback=[](const std::string&s) {
        if(s=="ability") { Vehs[2].proto_id=64; Vehs[2].home_base_id=7;
            Vehs[2].x=4; Vehs[2].y=4; Vehs[2].faction_id=6; }
        if(s=="delete")Vehs[2].home_base_id=1;
    };
    kill(2); expect({"ability","delete","tutorial","at","check","radius"});
    assert(ability_proto==9 && base_arg==7 && at_x==2 && at_y==2 && checked_faction==3);
    assert(draw_x==2 && draw_y==2);
    // Tail guards use the state after tutorial callback, even when the cargo
    // branch initially had valid coordinates.
    reset(); callback=[](const std::string&s) { if(s=="tutorial")VehBattleUnkTgl[0]=1; };
    kill(2); expect({"ability","delete","tutorial"});
    reset(); callback=[](const std::string&s) { if(s=="tutorial")MapLongitudeBounds=0; };
    kill(2); expect({"ability","delete","tutorial"});
    // Changing bounds after an initially invalid cargo route can enable draw.
    reset(); VehPrototypes[9].carry_capacity=1; Vehs[2].x=8;
    callback=[](const std::string&s) { if(s=="tutorial")MapLongitudeBounds=10; };
    kill(2); expect({"ability","stack","stack_delete","tutorial","at","check","radius"});
    assert(at_x==8 && draw_x==8);
    // Remote faction draws one tile; local stack-check/ability outcomes select
    // radius or tile. The saved faction is not reread from the deleted row.
    reset(); ConsoleGlobal.faction_id_=4; kill(2); expect({"ability","delete","tutorial","tile"});
    for(int ability : {0,1}) for(int checked : {0,1}) {
        reset(); ability_result=ability; stack_result=checked; kill(2);
        expect({"ability","delete","tutorial","at","check", checked && !ability?"tile":"radius"});
    }
    reset(); callback=[](const std::string&s) { if(s=="tutorial")ConsoleGlobal.faction_id_=4; };
    kill(2); expect({"ability","delete","tutorial","tile"});
    return 0;
}
'''


def run(tmp_path, compiler, records, map_loc, body):
    cpp=tmp_path/"kill.cpp"; exe=tmp_path/"kill"
    cpp.write_text(HARNESS.replace("/* ACTUAL RECORDS */",records)
                   .replace("/* ACTUAL MAP LOC */",map_loc).replace("/* ACTUAL KILL */",body))
    built=subprocess.run([compiler,"-std=c++11","-fsanitize=undefined","-fno-sanitize-recover=all",str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=60)
    assert built.returncode==0,built.stdout+built.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True,timeout=20)


@pytest.mark.parametrize("compiler",["g++","clang++"])
def test_complete_actual_kill_branches(tmp_path,compiler):
    compiler=shutil.which(compiler)
    if not compiler:pytest.skip("host compiler unavailable")
    result=run(tmp_path,compiler,*material())
    assert result.returncode==0,result.stdout+result.stderr


@pytest.mark.parametrize("damage",["old_prototype","old_home","omit_common_draw","invert_terrain","omit_stack","sentinel_tutorial","capture_late_coords","skip_ability_radius","omit_tutorial","tail_before_tutorial"])
def test_complete_kill_damage_controls(tmp_path,damage):
    compiler=shutil.which("g++")
    if not compiler:pytest.skip("host compiler unavailable")
    records,map_loc,body=material()
    changes={
        "old_prototype":("const int proto_id = Vehs[veh_id].proto_id;","const int proto_id = initial_proto_id;"),
        "old_home":("    const int home_base_id = Vehs[veh_id].home_base_id;", ""),
        "omit_common_draw":("            veh_kill(veh_id);\n        } else {", "            veh_kill(veh_id);\n            TutWinBaseWindow.check_base(home_base_id);\n            return;\n        } else {"),
        "invert_terrain":("climate & 0xe0) >= 0x60","climate & 0xe0) < 0x60"),
        "omit_stack":("            stack_veh(veh_id, 2);",""),
        "sentinel_tutorial":("                stack_kill(veh_id);\n                return;","                stack_kill(veh_id);\n                TutWinBaseWindow.check_base(home_base_id);\n                return;"),
        "capture_late_coords":("    const int x = Vehs[veh_id].x;\n    const int y = Vehs[veh_id].y;", ""),
        "skip_ability_radius":("checked == 0 || worm_abil != 0","checked == 0"),
        "omit_tutorial":("    TutWinBaseWindow.check_base(home_base_id);", "    (void)home_base_id;"),
        "tail_before_tutorial":("    TutWinBaseWindow.check_base(home_base_id);", "    const bool before_tutorial = VehBattleUnkTgl[0] == 0;\n    TutWinBaseWindow.check_base(home_base_id);"),
    }
    before,after=changes[damage];assert before in body
    body=body.replace(before,after,1)
    if damage=="old_home":
        body=body.replace("    const int worm_abil =", "    const int home_base_id = Vehs[veh_id].home_base_id;\n    const int worm_abil =",1)
    elif damage=="capture_late_coords":
        body=body.replace("    const int proto_id = Vehs[veh_id].proto_id;", "    const int x = Vehs[veh_id].x;\n    const int y = Vehs[veh_id].y;\n    const int proto_id = Vehs[veh_id].proto_id;",1)
    elif damage=="tail_before_tutorial":
        body=body.replace("VehBattleUnkTgl[0] == 0 &&", "before_tutorial &&",1)
    result=run(tmp_path,compiler,records,map_loc,body)
    assert result.returncode!=0,"failed to detect "+damage


def test_pinned_complete_kill_and_status_owner_evidence():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path=Path(os.environ.get("OPENSMACX_IMAGE","/home/fran6co/code/OpenSMACX/.opensmacx/game/terranx_original.exe"))
    image=PeImage(path.read_bytes(),PIN);decoder=Cs(CS_ARCH_X86,CS_MODE_32)
    decoded=list(decoder.disasm(image.read(Span(0x5C0B00,0x5C0CB0)),0x5C0B00))
    assert decoded[-1].address+decoded[-1].size==0x5C0CB0
    ins={i.address:(i.mnemonic,i.op_str) for i in decoded}
    expected={
        0x5C0B19:("movsx","eax, word ptr [esi + 0x952832]"),
        0x5C0B20:("movsx","ebx, word ptr [esi + 0x952828]"),
        0x5C0B27:("movsx","ecx, word ptr [esi + 0x95282a]"),
        0x5C0B2E:("mov","dl, byte ptr [esi + 0x952836]"),
        0x5C0B3E:("call","0x5bf1f0"),
        0x5C0B43:("mov","dx, word ptr [esi + 0x952832]"),
        0x5C0B4D:("movsx","ecx, word ptr [esi + 0x952856]"),
        0x5C0B66:("mov","cl, byte ptr [eax + 0x9ab890]"),
        0x5C0B6E:("je","0x5c0ba2"),0x5C0B74:("jge","0x5c0ba2"),
        0x5C0B87:("test","al, al"),0x5C0B89:("jl","0x5c0b91"),
        0x5C0B8B:("cmp","dx, 0xf"),0x5C0B8F:("jne","0x5c0b9f"),
        0x5C0B96:("mov","cl, byte ptr [esi + 0x95284c]"),
        0x5C0BAD:("je","0x5c0c1e"),
        0x5C0BB1:("jl","0x5c0c09"),0x5C0BB9:("jge","0x5c0c09"),
        0x5C0BBD:("jl","0x5c0c09"),0x5C0BC5:("jge","0x5c0c09"),
        0x5C0BE8:("and","al, 0xe0"),0x5C0BEA:("cmp","eax, 0x60"),
        0x5C0BED:("setl","dl"),0x5C0BF4:("je","0x5c0c1e"),
        0x5C0BF9:("call","0x5b8ee0"),0x5C0BFF:("call","0x5b9510"),
        0x5C0C07:("jmp","0x5c0c27"),0x5C0C09:("cmp","ebx, -2"),
        0x5C0C0C:("jne","0x5c0bf6"),0x5C0C0F:("call","0x5b9510"),
        0x5C0C1D:("ret",""),0x5C0C1F:("call","0x5c08c0"),
        0x5C0C27:("mov","eax, dword ptr [ebp - 8]"),
        0x5C0C30:("call","0x41a7a0"),0x5C0C35:("mov","eax, dword ptr [0x91561c]"),
        0x5C0C3C:("jne","0x5c0ca9"),
        0x5C0C61:("jne","0x5c0c9d"),0x5C0C6C:("call","0x5bfe90"),
        0x5C0C75:("call","0x5b9580"),0x5C0C7F:("je","0x5c0c88"),
        0x5C0C81:("mov","eax, dword ptr [ebp - 0x10]"),
        0x5C0C86:("je","0x5c0c9d"),0x5C0C8E:("call","0x46aef0"),
        0x5C0CA1:("call","0x46af40"),0x5C0CAF:("ret",""),
    }
    for address,value in expected.items():assert ins[address]==value,hex(address)
    # No constructed local/unwind path belongs to this plain scalar operation.
    assert [i.op_str for i in decoded if i.mnemonic=="call"]==[
        "0x5bf1f0","0x5b8ee0","0x5b9510","0x5b9510","0x5c08c0",
        "0x41a7a0","0x5bfe90","0x5b9580","0x46aef0","0x46af40"]
    other={}
    for start,end in ((0x5C0A71,0x5C0A8A),(0x4B6530,0x4B6569)):
        other.update({i.address:(i.mnemonic,i.op_str) for i in decoder.disasm(image.read(Span(start,end)),start)})
    assert other[0x5C0A71]==("mov","eax, dword ptr [0x8c6b34]")
    assert other[0x5C0A7A]==("mov","dword ptr [0x8c6b34], ecx")
    assert other[0x5C0A85]==("mov","dword ptr [0x8c6b34], eax")
    assert other[0x4B6544]==("mov","edx, dword ptr [ecx + 0x15cc]")
    assert 0x8C5568+0x15CC==0x8C6B34
    header=(ROOT/"src/statuswin.h").read_text()
    assert "friend void __cdecl veh_kill(int veh_id);" in header
    assert "int32_t field_15CC_;" in header
    source=(ROOT/"src/veh.cpp").read_text()
    assert "StatusHoverVehIdAlt" not in source
    assert "ConsoleStatusWin.field_15CC_" in _definition(source,r"void\s+__cdecl\s+veh_kill","veh_kill")
