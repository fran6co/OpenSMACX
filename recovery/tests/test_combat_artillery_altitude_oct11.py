"""Actual artillery tail over bounded terrain altitudes; not whole combat proof."""
from pathlib import Path
import resource
import shutil
import subprocess

import pytest

from test_mapwin_cache import braced

ROOT = Path(__file__).resolve().parents[2]
BASE = "ae160dcc3"


def artillery_region(text):
    return braced(text, "            if (!(combat_type & 0x18)) {")


def test_only_artillery_tail_changes_with_original_obligations_preserved():
    from match_recovery.source import read_text
    from test_texture_raster_actual_source import _continuity_fields
    path = ROOT / "src/veh.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/veh.cpp"], cwd=ROOT, text=True)
    new = path.read_text()
    assert new.replace(artillery_region(new), artillery_region(old), 1) == old
    assert [(_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(old, path)] == [
        (_continuity_fields(r), r.symbol, r.function_fragment) for r in read_text(new, path)]


HOST = r'''
#include <cassert>
typedef unsigned int uint32_t;
enum {TRIAD_LAND=0,TRIAD_SEA=1,TRIAD_AIR=2};
struct {int combat_pct_land_gun_vs_ship_art,combat_pct_art_bonus_lvl_alt;} Rules;
struct {int x,y;} Vehs[2]={{10,20},{30,40}};
static int attacker_altitude,defender_altitude,attacker_triad,defender_triad;
static int altitude_calls,note_count,note_side,note_percent,note_label;
static char labels[600];
static int get_proto_triad(int id){assert(id==1||id==2);return id==1?attacker_triad:defender_triad;}
static int alt_at(int x,int y){++altitude_calls;if(x==10&&y==20)return attacker_altitude;assert(x==30&&y==40);return defender_altitude;}
static const char *label_get(int id){return labels+id;}
static void add_bat(int side,int pct,const char *label){
 assert(note_count++==0);note_side=side;note_percent=pct;note_label=(int)(label-labels);
}
struct Result {int offense,defense;};
static Result run(int combat_type){
 int offense=101,defense=101;altitude_calls=note_count=0;
 const int proto_id_atk=1,proto_id_def=2,veh_id_atk=0,veh_id_def=1;
 // ACTUAL_TAIL
 Result r={offense,defense};return r;
}
static int scale(int value,int factor){return (int)((long long)value*factor/100);}
int main(){
 const int pcts[]={-100,-50,-25,0,25,50,125,1000};
 const int modes[]={2,10,18,26};
 for(attacker_triad=0;attacker_triad<3;++attacker_triad)
 for(defender_triad=0;defender_triad<3;++defender_triad)
 for(attacker_altitude=0;attacker_altitude<8;++attacker_altitude)
 for(defender_altitude=0;defender_altitude<8;++defender_altitude)
 for(unsigned p=0;p<sizeof(pcts)/sizeof(pcts[0]);++p)
 for(unsigned m=0;m<sizeof(modes)/sizeof(modes[0]);++m)
 for(int guns=0;guns<=1;++guns){
  Rules.combat_pct_land_gun_vs_ship_art=guns?25:0;
  Rules.combat_pct_art_bonus_lvl_alt=pcts[p];
  int side=-1,pct=0,label=0,expected_calls=0;
  if(!(modes[m]&0x18)){
   if(attacker_triad==TRIAD_LAND&&defender_triad==TRIAD_SEA){if(guns){side=0;pct=25;label=435;}}
   else if(attacker_triad==TRIAD_SEA&&defender_triad==TRIAD_LAND){if(guns){side=1;pct=25;label=435;}}
   else if(pcts[p]){
    expected_calls=2;int delta=attacker_altitude-defender_altitude;
    if(delta){side=delta>0?0:1;pct=(delta>0?delta:-delta)*pcts[p];label=576;}
   }
  }
  Result r=run(modes[m]);
  assert(r.offense==(side==0?scale(101,100+pct):101));
  assert(r.defense==(side==1?scale(101,100+pct):101));
  assert(altitude_calls==expected_calls&&note_count==(side>=0));
  if(side>=0)assert(note_side==side&&note_percent==pct&&note_label==label);
 }
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutation", [None,
    ("(alt_modifier_def + 100) * defense / 100", "((alt_modifier_def + 100) / 100) * defense"),
    ("(alt_mod_atk + 100) * offense / 100", "((alt_mod_atk + 100) / 100) * offense"),
    ("if (alt_def >= alt_atk)", "if (alt_def > alt_atk)"),
    ("if (!(combat_type & 0x18))", "if (!(combat_type & 0x08))"),
    ("add_bat(1, alt_modifier_def,", "add_bat(0, alt_modifier_def,"),
])
def test_actual_tail_and_damage_controls(compiler, mutation, tmp_path):
    compiler = shutil.which(compiler)
    if not compiler:
        pytest.skip("host compiler unavailable")
    actual = HOST.replace("// ACTUAL_TAIL", artillery_region((ROOT / "src/veh.cpp").read_text()))
    if mutation:
        assert actual.count(mutation[0]) == 1
        actual = actual.replace(*mutation)
    source = tmp_path / "tail.cpp"
    source.write_text(actual)
    binary = tmp_path / "tail"
    compiled = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True,
                            preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
    assert (result.returncode < 0 if mutation else result.returncode == 0), result.stdout + result.stderr


def test_pinned_artillery_fallback_and_both_altitude_products():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    rows = {i.address: (i.mnemonic, i.op_str)
            for i in decoder.disasm(image.read(Span(0x502726, 0x502A51)), 0x502726)}
    for address, expected in {
        0x502726: ("test", "byte ptr [ebp + 0x18], 2"),
        0x50272A: ("je", "0x502a51"),
        0x502735: ("jl", "0x5044a3"),
        0x502744: ("push", "1"),
        0x502748: ("push", "esi"),
        0x502749: ("push", "eax"),
        0x50274A: ("call", "0x5015b0"),
        0x50275A: ("test", "al, 0x18"),
        0x50275C: ("jne", "0x5044a3"),
        0x5028F4: ("mov", "edx, dword ptr [0x9497f0]"),
        0x502930: ("shr", "edi, 5"),
        0x502952: ("shr", "esi, 5"),
        0x502955: ("cmp", "esi, edi"),
        0x502957: ("jge", "0x5029d2"),
        0x502959: ("sub", "edi, esi"),
        0x502962: ("imul", "ecx, edx"),
        0x502965: ("add", "ecx, 0x64"),
        0x50297D: ("mov", "dword ptr [ebp - 4], edx"),
        0x502992: ("imul", "edi, dword ptr [0x9497f0]"),
        0x5029A1: ("mov", "edx, dword ptr [0x915614]"),
        0x5029BA: ("mov", "dword ptr [ecx*4 + 0x9155f0], edi"),
        0x5029D2: ("jle", "0x5044a3"),
        0x5029D8: ("sub", "esi, edi"),
        0x5029E1: ("imul", "ecx, edx"),
        0x5029E4: ("add", "ecx, 0x64"),
        0x5029FD: ("mov", "dword ptr [ebp - 8], edx"),
        0x502A11: ("imul", "esi, dword ptr [0x9497f0]"),
        0x502A20: ("mov", "edx, dword ptr [0x915618]"),
        0x502A44: ("mov", "dword ptr [ecx*4 + 0x915600], esi"),
    }.items():
        assert rows[address] == expected
    # Operands/targets are asserted below from independently decoded native
    # control flow; the two accumulators are offense -4 and defense -8.
    for lo, hi, accumulator in ((0x502959, 0x502980, "dword ptr [ebp - 4]"),
                                (0x5029D2, 0x5029FD, "dword ptr [ebp - 8]")):
        block = [(a, m, o) for a, (m, o) in rows.items() if lo <= a < hi]
        product = next(a for a, m, o in block if m == "imul" and accumulator in o)
        divide = next(a for a, m, o in block if m == "imul" and o == "ecx")
        assert product < divide
        assert any(m == "sar" and o == "edx, 5" for a, m, o in block)
    assert any(m == "call" and o == "0x5015b0" for a, (m, o) in rows.items() if a < 0x50275A)
    assert any("0x9497f0" in operand for _, operand in rows.values())
    assert not any("0x949824" in operand for _, operand in rows.values())
