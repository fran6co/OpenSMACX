"""Execute recovered source paths with bounded doubles, not whole-game proof."""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
BASE = "602b8d5b4"


def regions(text):
    pairs = (
        ("    BOOL is_artillery =", "    uint32_t psi_combat_type"),
        ("                    if (!has_abil(proto_id_def, ABL_DISSOCIATIVE_WAVE) &&", "                if (Players[faction_id_atk].rule_flags"),
        ("                int bonus_count = Players[faction_id_atk].faction_bonus_count;", "                if (psi_combat_type && faction_id_atk)"),
        ("            int bonus_count = Players[faction_id_def].faction_bonus_count;", "            if (veh_id_atk >= 0 && VehPrototypes[proto_id_def].plan"),
        ("                if (!is_ocean(x_def, y_def)) {\n                    uint32_t sensor_def", "\n            }\n        } else if (veh_id_def >= 0)"),
    )
    result = []
    for start, end in pairs:
        lo = text.index(start)
        hi = text.index(end, lo)
        result.append(text[lo:hi])
    return result


def restore_regions(current, prior):
    for new, old in zip(regions(current), regions(prior)):
        assert current.count(new) == 1
        current = current.replace(new, old, 1)
    from test_combat_artillery_altitude_oct11 import artillery_region
    current = current.replace(artillery_region(current), artillery_region(prior), 1)
    return current


def test_only_reviewed_ability_regions_change_and_claims_survive():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    path = ROOT / "src/veh.cpp"
    before = subprocess.check_output(["git", "show", BASE + ":src/veh.cpp"], cwd=ROOT, text=True)
    after = path.read_text()
    # The reviewed subsequent artillery-tail closure has a separate whole-diff guard.
    from test_combat_artillery_altitude_oct11 import artillery_region
    after = after.replace(artillery_region(after), artillery_region(before), 1)
    assert restore_regions(after, before) == before
    assert [(_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(before, path)] == [
        (_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(after, path)]


HOST = r'''
#include <cassert>
#include <cstring>
typedef unsigned int uint32_t;
typedef int BOOL;
enum {TRIAD_LAND=0,TRIAD_SEA=1,TRIAD_AIR=2,
 ABL_DISSOCIATIVE_WAVE=1,ABL_EMPATHIC=2,ABL_DROP_POD=4,ABL_TRANCE=8,
 ABL_AIR_SUPERIORITY=16,ABL_COMM_JAMMER=32,ABL_AAA=64,
 ARM_RESONANCE_3_ARMOR=11,ARM_RESONANCE_8_ARMOR=13,
 ARM_PULSE_3_ARMOR=10,ARM_PULSE_8_ARMOR=12,VSTATE_MADE_AIRDROP=32,
 RULE_OFFENSE=18,RULE_DEFENSE=17};
struct RuleState {
 int combat_pct_emp_song_atk_vs_psi,combat_pen_pct_atk_airdrop;
 int max_airdrop_sans_orb_insert,combat_pct_def_range_sensor,combat_pct_trance_def_vs_psi;
 int combat_pen_pct_air_supr_vs_grnd,combat_pct_air_supr_vs_air;
 int combat_pen_pct_non_cbt_def_vs_cbt,combat_pct_bonus_vs_ship_port;
 int combat_pct_com_jam_def_vs_mobl,combat_pct_aaa_bonus_vs_air;
};
static RuleState Rules;
struct Scenario {
 RuleState rules;int atk_abil,def_abil,atk_triad,def_triad,armor,speed,psi;
 int attack_bonus,defense_bonus,base,attacker,sensor_status,base_result,base_faction,territory;
 int defender_ocean,sensor_ocean,offrating,defrating,drop_state,drop_range;
 bool attack_present,defense_present;
 Scenario():atk_abil(0),def_abil(0),atk_triad(0),def_triad(0),armor(0),speed(1),psi(0),
 attack_bonus(0),defense_bonus(0),base(-1),attacker(0),sensor_status(0),base_result(0),
 base_faction(2),territory(2),defender_ocean(0),sensor_ocean(0),offrating(2),defrating(2),
 drop_state(0),drop_range(1),attack_present(false),defense_present(false) {memset(&rules,0,sizeof(rules));rules.max_airdrop_sans_orb_insert=2;}
};
static Scenario state;
struct Vehicle {int proto_id,state; } backing[3];
static Vehicle *Vehs=backing+1; // valid preceding storage models native pre-array read
struct Proto {int armor_id,chassis_id;} VehPrototypes[3];
struct ChassisRow {int speed;} Chassis[3];
struct Player {int faction_bonus_count,faction_bonus_id[1],faction_bonus_val1[1];} Players[3];
struct Base {uint32_t faction_id_current;} base_backing[3];
static Base *Bases=base_backing+1; // likewise retain the unchecked base_find(-1) effect
struct AbilityRow {const char *name;} Ability[3]={{"ability"},{"ability"},{"ability"}};
static int RadiusOffsetX[25],RadiusOffsetY[25];
static int base_calls,territory_calls,missile_proto;
static bool has_abil(int id,int ability){return ((id==1?state.atk_abil:state.def_abil)&ability)!=0;}
static bool is_proto_missile(int id){missile_proto=id;return id==0;}
static int get_proto_triad(int id){return id==1?state.atk_triad:state.def_triad;}
static int get_proto_offense_rating(int){return state.offrating;}
static int get_proto_defense_rating(int){return state.defrating;}
static int abil_index(int){return 0;}
static int drop_range(int){return state.drop_range;}
static bool is_ocean(int x,int){return x==100?state.defender_ocean:state.sensor_ocean;}
static int xrange(int x){return x;}
static bool on_map(int x,int){return x==101;}
static uint32_t is_sensor(int,int){return state.sensor_status;}
static int base_find(int,int){++base_calls;return state.base_result;}
static int whose_territory(int,int,int,void *,bool){++territory_calls;return state.territory;}
static char labels[1200];
static const char *label_get(int n){return labels+n;}
struct Note {int side,pct,label,offense,defense;} notes[32];
static int note_count,*observed_offense,*observed_defense;
static void add_bat(int side,int pct,const char *name){
 assert(note_count<32);Note &n=notes[note_count++];n.side=side;n.pct=pct;
 n.label=name==Ability[0].name?-1:(int)(name-labels);
 n.offense=*observed_offense;n.defense=*observed_defense;
}
struct Result {int offense,defense,bombard;};
static Result run(Scenario s){
 state=s;Rules=s.rules;note_count=base_calls=territory_calls=0;missile_proto=-99;
 memset(Players,0,sizeof(Players));
 if(s.attack_present){Players[1].faction_bonus_count=1;Players[1].faction_bonus_id[0]=RULE_OFFENSE;Players[1].faction_bonus_val1[0]=s.attack_bonus;}
 if(s.defense_present){Players[2].faction_bonus_count=1;Players[2].faction_bonus_id[0]=RULE_DEFENSE;Players[2].faction_bonus_val1[0]=s.defense_bonus;}
 Vehs[-1].proto_id=0;Vehs[0].proto_id=1;Vehs[0].state=s.drop_state;
 VehPrototypes[2].armor_id=s.armor;VehPrototypes[1].chassis_id=1;
 Chassis[1].speed=s.speed;Bases[s.base_result].faction_id_current=s.base_faction;
 for(int i=0;i<25;++i){RadiusOffsetX[i]=i+1;RadiusOffsetY[i]=0;}
 int offense=101,defense=101;
 observed_offense=&offense;observed_defense=&defense;
 const int veh_id_atk=s.attacker,veh_id_def=1,proto_id_atk=1,proto_id_def=2;
 const uint32_t faction_id_atk=1,faction_id_def=2,psi_combat_type=s.psi;
 const int base_id_def=s.base,x_def=100,y_def=200;
 const int combat_type=0;
 // ACTUAL_BOMBARD
 { // ACTUAL_EMPATH_DROP
 }
 { // ACTUAL_ATTACK_BONUS
 }
 { // ACTUAL_DEFENSE_BONUS
 }
 // ACTUAL_DEFENSE_ABILITIES
 Result r={offense,defense,is_bombard};return r;
}
static int scale(int value,int factor){return (int)((long long)value*factor/100);}
static void note(int label,int side,int pct){
 for(int i=0;i<note_count;++i)if(notes[i].label==label){assert(notes[i].side==side&&notes[i].pct==pct);return;}
 assert(false);
}
int main(){
 const int pcts[]={-100,-50,-25,0,25,50,100,125};
 for(int family=0;family<12;++family)for(unsigned i=0;i<sizeof(pcts)/sizeof(pcts[0]);++i){
  Scenario s;int pct=pcts[i],factor=100+pct,side=1,label=0;
  switch(family){
   case 0:s.psi=2;s.atk_abil=ABL_EMPATHIC;s.rules.combat_pct_emp_song_atk_vs_psi=pct;side=0;label=-1;break;
   case 1:s.drop_state=VSTATE_MADE_AIRDROP;s.atk_abil=ABL_DROP_POD;s.rules.combat_pen_pct_atk_airdrop=pct;factor=100-pct;side=0;label=437;break;
   case 2:s.attack_present=true;s.attack_bonus=100+pct;side=0;label=1108;break;
   case 3:s.defense_present=true;s.defense_bonus=100+pct;label=1109;break;
   case 4:s.sensor_status=1;s.rules.combat_pct_def_range_sensor=pct;label=613;break;
   case 5:s.psi=1;s.def_abil=ABL_TRANCE;s.rules.combat_pct_trance_def_vs_psi=pct;label=329;break;
   case 6:s.atk_triad=TRIAD_AIR;s.atk_abil=ABL_AIR_SUPERIORITY;s.rules.combat_pen_pct_air_supr_vs_grnd=pct;factor=100-pct;side=0;label=448;break;
   case 7:s.atk_triad=s.def_triad=TRIAD_AIR;s.atk_abil=ABL_AIR_SUPERIORITY;s.rules.combat_pct_air_supr_vs_air=pct;side=0;label=449;break;
   case 8:s.offrating=0;s.defrating=1;s.rules.combat_pen_pct_non_cbt_def_vs_cbt=pct;factor=100-pct;label=439;break;
   case 9:s.atk_triad=s.def_triad=TRIAD_SEA;s.base=0;s.rules.combat_pct_bonus_vs_ship_port=pct;side=0;label=335;break;
   case 10:s.speed=2;s.def_abil=ABL_COMM_JAMMER;s.rules.combat_pct_com_jam_def_vs_mobl=pct;label=336;break;
   case 11:s.atk_triad=TRIAD_AIR;s.def_abil=ABL_AAA;s.rules.combat_pct_aaa_bonus_vs_air=pct;label=337;break;
  }
  Result r=run(s);assert(r.offense==(side?101:scale(101,factor)));
  assert(r.defense==(side?scale(101,factor):101));
  if(pct)note(label,side,(family==1||family==6||family==8)?-pct:pct);
 }
 Scenario s;s.psi=1;s.atk_abil=ABL_EMPATHIC;s.rules.combat_pct_emp_song_atk_vs_psi=50;
 assert(run(s).offense==101&&note_count==0);s.psi=2;s.def_abil=ABL_DISSOCIATIVE_WAVE;
 assert(run(s).offense==101&&note_count==0);
 s=Scenario();s.drop_state=VSTATE_MADE_AIRDROP;s.atk_abil=ABL_DROP_POD;s.rules.combat_pen_pct_atk_airdrop=50;s.rules.combat_pct_emp_song_atk_vs_psi=25;
 assert(run(s).offense==50);note(437,0,-50);s.drop_range=3;assert(run(s).offense==50);note(438,0,-50);
 for(int ocean=0;ocean<2;++ocean)for(int owner=-1;owner<=3;++owner){
  s=Scenario();s.sensor_status=1;s.sensor_ocean=ocean;s.base_faction=owner;s.territory=owner;s.rules.combat_pct_def_range_sensor=25;
  Result r=run(s);bool eligible=ocean?(owner<0||owner==2):owner==2;
  assert(r.defense==(eligible?126:101));assert(base_calls==!ocean&&territory_calls==ocean);
 }
 s=Scenario();s.sensor_status=2;s.rules.combat_pct_def_range_sensor=25;
 assert(run(s).defense==157);note(613,1,25);note(1123,1,25);
 s.base_result=-1;assert(run(s).defense==157&&base_calls==1); // retain unchecked lookup
 s.defender_ocean=1;assert(run(s).defense==101&&base_calls==0&&territory_calls==0);
 for(int armor=10;armor<=12;armor+=2)for(int triad=0;triad<3;++triad)for(int speed=1;speed<=2;++speed){
  s=Scenario();s.armor=armor;s.atk_triad=triad;s.speed=speed;
  assert(run(s).defense==(triad==TRIAD_LAND&&speed>1?126:101));
 }
 s=Scenario();s.atk_triad=s.def_triad=TRIAD_AIR;s.atk_abil=ABL_AIR_SUPERIORITY;s.def_abil=ABL_AIR_SUPERIORITY;s.rules.combat_pct_air_supr_vs_air=25;
 assert(run(s).offense==101); // defender suppresses attacker's air-to-air bonus
 s.atk_abil=0;assert(run(s).defense==126);note(449,1,25);
 s=Scenario();s.psi=3;s.atk_abil=ABL_EMPATHIC|ABL_DROP_POD;s.def_abil=ABL_TRANCE;
 s.drop_state=VSTATE_MADE_AIRDROP;s.rules.combat_pct_emp_song_atk_vs_psi=50;
 s.rules.combat_pen_pct_atk_airdrop=25;s.attack_present=s.defense_present=true;
 s.attack_bonus=s.defense_bonus=125;s.sensor_status=2;
 s.rules.combat_pct_def_range_sensor=s.rules.combat_pct_trance_def_vs_psi=25;
 Result combined=run(s);assert(combined.offense==141&&combined.defense==245);
  s=Scenario();s.attacker=-1;Result r=run(s);assert(r.bombard&&missile_proto==0);
}
'''


MUTATIONS = [None,
    ("is_proto_missile(Vehs[veh_id_atk].proto_id)", "(veh_id_atk >= 0 && is_proto_missile(proto_id_atk))"),
    ("&& (psi_combat_type & 2)", "&& psi_combat_type"),
    ("(100 - Rules.combat_pen_pct_atk_airdrop) * offense", "(100 - Rules.combat_pct_emp_song_atk_vs_psi) * offense"),
    ("add_bat(0, -Rules.combat_pen_pct_atk_airdrop,", "add_bat(0, Rules.combat_pen_pct_atk_airdrop,"),
    ("add_bat(0, rule_off_bonus - 100,", "add_bat(0, rule_off_bonus,"),
    ("add_bat(1, rule_def_bonus - 100,", "add_bat(1, rule_def_bonus,"),
    ("if (!is_ocean(x_radius, y_radius))", "if (is_ocean(x_radius, y_radius))"),
    ("if (Bases[base_id_find].faction_id_current", "if (base_id_find >= 0 && Bases[base_id_find].faction_id_current"),
    ("if (get_proto_triad(proto_id_def) != TRIAD_AIR)", "if (get_proto_triad(proto_id_def) == TRIAD_AIR)"),
    ("&& !has_abil(proto_id_atk, ABL_AIR_SUPERIORITY) && !psi_combat_type",
     "&& has_abil(proto_id_atk, ABL_AIR_SUPERIORITY) && !psi_combat_type"),
    ("(armor_id_def == ARM_PULSE_3_ARMOR || armor_id_def == ARM_PULSE_8_ARMOR)",
     "armor_id_def == ARM_PULSE_3_ARMOR || armor_id_def == ARM_PULSE_8_ARMOR"),
]
for expression, target in [
    ("Rules.combat_pct_emp_song_atk_vs_psi + 100", "offense"),
    ("Rules.combat_pct_def_range_sensor + 100", "defense"),
    ("Rules.combat_pct_trance_def_vs_psi + 100", "defense"),
    ("100 - ground_strike_pen", "offense"), ("air_to_air + 100", "offense"),
    ("Rules.combat_pct_air_supr_vs_air + 100", "defense"),
    ("100 - Rules.combat_pen_pct_non_cbt_def_vs_cbt", "defense"),
    ("Rules.combat_pct_bonus_vs_ship_port + 100", "offense"),
    ("Rules.combat_pct_com_jam_def_vs_mobl + 100", "defense"),
    ("Rules.combat_pct_aaa_bonus_vs_air + 100", "defense"),
]:
    MUTATIONS.append((f"({expression}) * {target} / 100", f"(({expression}) / 100) * {target}"))
for bonus, target in (("rule_off_bonus", "offense"), ("rule_def_bonus", "defense")):
    MUTATIONS.append((f"{bonus} * {target} / 100", f"({bonus} / 100) * {target}"))


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", MUTATIONS)
def test_actual_source_and_damage_controls(compiler, mutation, tmp_path):
    compiler = shutil.which(compiler)
    if not compiler:
        pytest.skip("host compiler unavailable")
    blocks = regions((ROOT / "src/veh.cpp").read_text())
    # The Empath body is nested in surrounding native gates that are unchanged;
    # extract its complete if, then the adjacent airdrop if without closing that parent.
    from test_mapwin_cache import braced
    empath = blocks[1]
    blocks[1] = braced(empath, "if (!has_abil") + "\n" + braced(empath, "if (Vehs[veh_id_atk].state")
    actual = HOST
    for marker, code in zip(("BOMBARD", "EMPATH_DROP", "ATTACK_BONUS", "DEFENSE_BONUS", "DEFENSE_ABILITIES"), blocks):
        actual = actual.replace("// ACTUAL_" + marker, code)
    if mutation:
        assert mutation[0] in actual
        actual = actual.replace(*mutation)
    source = tmp_path / "abilities.cpp"
    source.write_text(actual)
    binary = tmp_path / "abilities"
    built = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True,
                            preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
    assert (result.returncode < 0 if mutation else result.returncode == 0), result.stdout + result.stderr


def test_original_gates_products_reports_and_unchecked_reads():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    rows = {}
    for lo, hi in ((0x501DD5, 0x501E08), (0x502282, 0x502289),
                   (0x502335, 0x502381), (0x502409, 0x50243F),
                   (0x5024BE, 0x5024C8), (0x5025B9, 0x50265A),
                   (0x502A51, 0x502B6C), (0x503ABB, 0x503B11),
                   (0x503B56, 0x503B79), (0x503BCE, 0x503BF1),
                   (0x503CA8, 0x503CCB), (0x503E28, 0x503E61),
                   (0x503EBE, 0x503ED4), (0x503F7A, 0x504031),
                   (0x5040DB, 0x5040FC), (0x5041B3, 0x5041CB),
                   (0x504220, 0x50427C), (0x50435B, 0x504378),
                   (0x504437, 0x50444E)):
        rows.update({i.address: (i.mnemonic, i.op_str)
                     for i in decoder.disasm(image.read(Span(lo, hi)), lo)})
    for address, expected in {
        0x501DDB: ("movsx", "eax, word ptr [eax*4 + 0x952832]"),
        0x501E00: ("test", "dl, dl"),
        0x502285: ("and", "esi, 2"),
        0x50236C: ("imul", "ecx, dword ptr [ebp - 4]"),
        0x502409: ("mov", "eax, dword ptr [0x9497e4]"),
        0x50241B: ("sub", "ecx, eax"),
        0x502422: ("imul", "ecx, dword ptr [ebp - 4]"),
        0x5024BE: ("mov", "esi, dword ptr [0x9497e4]"),
        0x5024C4: ("neg", "esi"),
        0x5025DD: ("imul", "ecx, dword ptr [ebp - 4]"),
        0x502609: ("sub", "ecx, 0x64"),
        0x502AC7: ("imul", "ecx, dword ptr [ebp - 8]"),
        0x502B10: ("sub", "ecx, 0x64"),
        0x503ABD: ("call", "0x4e3b80"),
        0x503AD1: ("mov", "cl, byte ptr [eax*4 + 0x97d044]"),
        0x503AF6: ("call", "0x4e3ef0"),
        0x503B00: ("jl", "0x503b0e"),
        0x503B64: ("imul", "ecx, edi"),
        0x503BDC: ("imul", "ecx, edi"),
        0x503CB6: ("imul", "ecx, edi"),
        0x503E28: ("cmp", "byte ptr [ecx + 0x94a379], 2"),
        0x503E2F: ("je", "0x503e93"),
        0x503E4A: ("imul", "ecx, dword ptr [ebp - 4]"),
        0x503F7A: ("push", "0x20"),
        0x503F7D: ("call", "0x5bf1f0"),
        0x503F87: ("je", "0x50407a"),
        0x503F94: ("push", "0x20"),
        0x503F97: ("call", "0x5bf1f0"),
        0x503FA1: ("jne", "0x50407a"),
        0x504014: ("imul", "ecx, edi"),
        0x504019: ("sar", "edx, 5"),
        0x50401E: ("shr", "ecx, 0x1f"),
        0x504239: ("je", "0x504243"),
        0x504237: ("cmp", "al, 0xa"),
        0x50423B: ("cmp", "al, 0xc"),
        0x50423D: ("jne", "0x5042e1"),
        0x504262: ("mov", "cl, byte ptr [eax + 0x94a379]"),
        0x504268: ("test", "cl, cl"),
        0x50426A: ("jne", "0x5042e4"),
        0x50426C: ("cmp", "byte ptr [eax + 0x94a378], 1"),
        0x504273: ("jbe", "0x5042e4"),
    }.items():
        assert rows[address] == expected
    assert any(mnemonic == "and" and operand.endswith(", 2")
               for address, (mnemonic, operand) in rows.items() if 0x502282 <= address < 0x502289)
    # Every additional percentage path performs its accumulator product before
    # the signed reciprocal /100 operation, rather than dividing its factor first.
    for lo, hi in ((0x503EBE, 0x503ED4), (0x5040DB, 0x5040FC),
                   (0x5041B3, 0x5041CB), (0x50435B, 0x504378), (0x504437, 0x50444E)):
        instructions = [(a, m, o) for a, (m, o) in rows.items() if lo <= a < hi]
        product = next(a for a, m, o in instructions if m == "imul" and o != "ecx")
        divide = next(a for a, m, o in instructions if m == "imul" and o == "ecx")
        assert product < divide
