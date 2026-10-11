"""Execute the actual bounded combat block; not full battle/game acceptance."""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
BASE = "ee413fae0760f5b846b82264bb49524d36e65bd5"


def block(text):
    start = text.index("                    LPSTR display_def")
    end = text.index("\n                }\n                if (faction_id_def", start)
    return text[start:end]


def test_only_defense_block_changes_and_original_obligations_survive():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    path = ROOT / "src/veh.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/veh.cpp"], cwd=ROOT, text=True)
    new = path.read_text()
    # Reviewed follow-up terrain/PSI modifiers have their own actual-source,
    # outside-scope and pinned controls; keep this older defense review bounded.
    from test_combat_terrain_modifiers_oct11 import modifiers_region
    new = new.replace(modifiers_region(new), modifiers_region(old))
    assert old.replace(block(old), "REVIEWED_DEFENSE_BLOCK") == new.replace(
        block(new), "REVIEWED_DEFENSE_BLOCK")
    assert [(_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(old, path)] == [
        (_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(new, path)]


HOST = r'''
#include <cassert>
#include <cstring>
typedef unsigned int uint32_t;
typedef const char *LPSTR;
enum { TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2,
       FAC_PERIMETER_DEFENSE=4, FAC_TACHYON_FIELD=5, FAC_NAVAL_YARD=28,
       FAC_AEROSPACE_COMPLEX=29, SP_CITIZENS_DEFENSE_FORCE=1,
       SP_MARITIME_CONTROL_CENTER=2, SP_CLOUDBASE_ACADEMY=3,
       ABL_BLINK_DISPLACER=0x20000, MaxVehProtoFactionNum=64,
       BSC_SPORE_LAUNCHER=15, BIT_BUNKER=0x800, BIT_FOREST=0x200000,
       BIT_FUNGUS=0x20, TERRAIN_BIT_ROCKY=0x80, ALT_BIT_OCEAN_SHELF=0x40 };
struct Scenario {
 int base, attacker, proto, faction, combat, triad, proto_triad, speed;
 int rating, facilities, projects, bits, rocky, altitude, base_pct, infantry_pct;
 bool blink, artillery;
 Scenario():base(0),attacker(0),proto(64),faction(1),combat(0),triad(0),
 proto_triad(0),speed(1),rating(1),facilities(0),projects(0),bits(0),rocky(0),
 altitude(0x60),base_pct(0),infantry_pct(0),blink(false),artillery(false) {}
};
static Scenario state;
struct VehicleRow { int proto_id,faction_id; } vehicle_backing[3];
// A legitimate preceding row models native pre-array memory without host OOB.
static VehicleRow *Vehs=vehicle_backing+1;
static int facility_calls, ability_calls, rating_calls, altitude_calls;
struct { int combat_pct_base_def, combat_pct_infantry_vs_base; } Rules;
struct FacilityRow { LPSTR name; } Facility[30];
struct { int chassis_id; } VehPrototypes[128];
struct { int speed; } Chassis[1];
static char labels[1200][1];
static LPSTR label_get(int n) { return labels[n]; }
struct Strings { LPSTR get(LPSTR p) { return p; } } StringTable;
static int bit_at(int,int) { return state.bits; }
static int rocky_at(int,int) { return state.rocky; }
static int altitude_at(int,int) { ++altitude_calls;return state.altitude; }
static int get_triad(int n) { assert(n>=0);return state.triad; }
static bool has_fac_built(int fac,int base) {
 assert(base>=0);++facility_calls;
 int mask=fac==4?1:fac==28?2:fac==29?4:8;
 return (state.facilities&mask)!=0;
}
static bool has_project(int project,int) { return (state.projects&(1<<(project-1)))!=0; }
static bool has_abil(int proto,int ability) {
 assert(proto>=0&&ability==ABL_BLINK_DISPLACER);++ability_calls;return state.blink;
}
static int get_proto_offense_rating(int) { ++rating_calls;return state.rating; }
static int get_proto_triad(int) { return state.proto_triad; }
struct Note { int side,pct;LPSTR text; } notes[8];
static int note_count;
static void add_bat(int side,int pct,LPSTR text) {
 assert(note_count<8);notes[note_count].side=side;notes[note_count].pct=pct;
 notes[note_count++].text=text;
}
struct Result { int offense,defense;unsigned multiplier;LPSTR display; };
static Result run(Scenario s) {
 state=s;facility_calls=ability_calls=rating_calls=altitude_calls=note_count=0;
 assert(s.attacker>=-1&&s.attacker<=1);
 Vehs[s.attacker].proto_id=s.proto;Vehs[s.attacker].faction_id=s.faction;
 Rules.combat_pct_base_def=s.base_pct;Rules.combat_pct_infantry_vs_base=s.infantry_pct;
 Chassis[0].speed=s.speed;Facility[28].name=labels[28];Facility[29].name=labels[29];
 const int base_id_def=s.base,veh_id_atk=s.attacker;
 // Match battle_compute's earlier guarded cached values; the infantry block
 // must not substitute them for its later unguarded row reads.
 const int proto_id_atk=s.attacker>=0?s.proto:-1;
 const int faction_id_atk=s.attacker>=0?s.faction:0;
 const int faction_id_def=2,combat_type=s.combat;
 const bool is_artillery=s.artillery;
 const int x_def=0,y_def=0;
 int offense=101,defense=101;
 // ACTUAL_BLOCK
 Result result={offense,defense,def_multi,display_def};return result;
}
int main() {
 Scenario s;Result r;
 for(int base=0;base!=3;++base) {
  s=Scenario();s.base=base;s.base_pct=25;r=run(s);
  assert(r.defense==252&&r.multiplier==2&&r.display==labels[332]);
  assert(note_count==1&&notes[0].pct==25);
  for(int triad=0;triad!=3;++triad) {
   s.triad=triad;s.facilities=1<<triad;r=run(s);
   assert(r.defense==404&&r.multiplier==4&&note_count==1&&notes[0].pct==100);
   s.facilities|=8;r=run(s);
   assert(r.defense==606&&r.multiplier==6&&notes[0].pct==200&&r.display==labels[357]);
   s.blink=true;r=run(s);assert(r.defense==202&&r.multiplier==2&&note_count==0);
   s.blink=false;s.facilities=0;s.projects=1<<triad;r=run(s);
   assert(r.defense==404&&r.multiplier==4);s.projects=0;
  }
  s.triad=0;s.facilities=8;r=run(s);assert(r.defense==404&&r.multiplier==4);
  s.facilities=0;s.blink=true;r=run(s);assert(r.defense==202&&note_count==0);
 }
 s=Scenario();s.triad=3;s.base_pct=25;r=run(s);
 assert(r.defense==252&&r.multiplier==2&&facility_calls==1);
 s.facilities=8;r=run(s);assert(r.defense==404&&r.multiplier==4&&facility_calls==1);
 s=Scenario();s.base=-1;r=run(s);
 assert(r.defense==202&&facility_calls==0&&r.display==0);
 s.bits=BIT_BUNKER;r=run(s);assert(r.defense==303&&r.display==labels[358]);
 s.blink=true;r=run(s);assert(r.defense==202&&r.display==labels[428]);
 s=Scenario();s.base=-1;s.artillery=true;r=run(s);
 assert(r.defense==303&&r.display==labels[525]&&altitude_calls==0&&facility_calls==0);
 s.bits=BIT_FUNGUS;r=run(s);assert(r.defense==202&&altitude_calls==1);
 s.altitude=0x20;r=run(s);assert(r.defense==303);
 s.bits=BIT_FOREST;r=run(s);assert(r.defense==202);
 s.bits=0;s.rocky=0x80;r=run(s);assert(r.defense==202);
 s=Scenario();s.attacker=-1;s.proto=-1;s.faction=0;s.base_pct=25;
 s.facilities=8;r=run(s);
 assert(r.defense==252&&r.multiplier==2&&facility_calls==1&&ability_calls==0);
 s=Scenario();s.infantry_pct=25;
 for(int proto=63;proto!=66;++proto) {
  s.proto=proto;s.rating=-1;r=run(s);
  assert(r.offense==(proto>=64?126:101));assert(rating_calls==(proto>=64?0:1));
 }
 s.proto=63;s.rating=1;r=run(s);assert(r.offense==126);
 s.proto=15;r=run(s);assert(r.offense==101);
 s.proto=64;s.combat=1;r=run(s);assert(r.offense==101);
 s.combat=0;s.base=-1;r=run(s);assert(r.offense==101);
 s.base=0;s.proto_triad=1;r=run(s);assert(r.offense==101);
 s.proto_triad=0;s.speed=2;r=run(s);assert(r.offense==101);
 // The image reloads the row, even for attacker -1; its contents are unknown
 // in the real game, not necessarily zero. Both possible faction gates matter.
 s=Scenario();s.attacker=-1;s.infantry_pct=25;r=run(s);assert(r.offense==126);
 s.faction=0;r=run(s);assert(r.offense==101);
 return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [None,
    ("if (base_id_def >= 0)", "if (base_id_def)"),
    ("def_multi = fac_modifier;", "def_multi = 2;"),
    ("!has_abil(proto_id_atk, ABL_BLINK_DISPLACER)", "has_abil(proto_id_atk, ABL_BLINK_DISPLACER)"),
    ("(Rules.combat_pct_base_def + 100) * defense / 100", "((Rules.combat_pct_base_def + 100) / 100) * defense"),
    ("infantry_proto >= MaxVehProtoFactionNum", "infantry_proto > MaxVehProtoFactionNum"),
    ("!(bit_def & BIT_FUNGUS)", "(bit_def & BIT_FUNGUS)"),
    ("defense *= def_multi;", "if (base_id_def >= 0) defense *= def_multi;"),
    ("veh_id_atk < 0 ? veh_id_atk : 2", "2"),
    ("Vehs[veh_id_atk].faction_id", "faction_id_atk"),
])
def test_actual_defense_block_and_damage_controls(compiler, mutation, tmp_path):
    executable_compiler=shutil.which(compiler)
    if not executable_compiler:
        pytest.skip("host compiler unavailable")
    actual=block((ROOT/"src/veh.cpp").read_text())
    if mutation:
        assert actual.count(mutation[0])==1
        actual=actual.replace(*mutation)
    source=tmp_path/"defense.cpp"
    source.write_text(HOST.replace("// ACTUAL_BLOCK",actual))
    binary=tmp_path/"defense"
    built=subprocess.run([executable_compiler,"-std=c++98",str(source),"-o",str(binary)],capture_output=True,text=True)
    assert built.returncode==0,built.stdout+built.stderr
    result=subprocess.run([str(binary)],capture_output=True,text=True,
                          preexec_fn=lambda:resource.setrlimit(resource.RLIMIT_CORE,(0,0)))
    assert (result.returncode<0 if mutation else result.returncode==0),result.stdout+result.stderr


def test_native_defense_control_flow_and_multiplier_anchors():
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path=ROOT/".opensmacx/game/terranx_original.exe"
    if not path.exists():pytest.skip("pinned executable unavailable")
    image=PeImage(path.read_bytes(),image_pin(ROOT))
    rows={i.address:(i.mnemonic,i.op_str) for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(
        image.read(Span(0x50327E,0x5037C0)),0x50327E)}
    for address,expected in {
        0x5032A8:("mov","esi, 2"),0x5032FB:("jl","0x5035bc"),
        0x5032AD:("xor","ebx, ebx"),
        0x5033CA:("mov","esi, 4"),0x503506:("add","esi, 2"),
        0x5034C6:("mov","esi, dword ptr [ebp + 8]"),
        0x503514:("mov","dword ptr [ebp + 0xc], esi"),
        0x50353F:("jne","0x5035bc"),0x5035A9:("imul","ecx, dword ptr [ebp - 8]"),
        0x5035EC:("mov","dword ptr [ebp + 0xc], eax"),
        0x503648:("cmp","ax, 0x40"),0x50364C:("jge","0x50367a"),
        0x503725:("jge","0x503780"),0x503760:("je","0x50376e"),
        0x503773:("mov","dword ptr [ebp + 0xc], 3"),
        0x503785:("imul","edi, dword ptr [ebp - 8]"),
    }.items():assert rows[address]==expected
