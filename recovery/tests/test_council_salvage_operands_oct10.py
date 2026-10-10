"""Actual proposal case and PlayerData layout; no whole-game acceptance."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import read_text, write
from test_net_hotseat_owner_actual_source_oct6 import extract_definition
from test_console_typed_owner_members_oct10 import run_control

ROOT = Path(__file__).resolve().parents[2]
BASE = "a80ca6ae5e9b307ecaed6188d0dbe68d8899431f"


def case(source):
    start = source.index("    case PROP_SALVAGE_UNITY_CORE: {")
    return source[start:source.index("    case PROP_UNITE_SUPREME_LEADER:", start)]


def test_existing_case_and_all_original_evidence_survives():
    path=ROOT / "src/council.cpp"
    now=path.read_text()
    old=subprocess.check_output(["git","show",BASE+":src/council.cpp"],cwd=ROOT).decode()
    assert now==old
    rows=read_text(now,path)
    assert write(now,rows)==now
    before={r.address:r for r in read_text(old,path)}
    assert {r.address for r in rows}==set(before)
    for row in rows:
        for field in fields(row):
            if field.name not in {"line","region","origin"}:
                assert getattr(row,field.name)==getattr(before[row.address],field.name)
        def lessons(r):
            return [(x.kind,x.key,x.prose,tuple(t for _,t in x.lines)) for x in r.origin]
        assert lessons(row)==lessons(before[row.address])


def control():
    header=(ROOT / "src/faction.h").read_text()
    declarations="\n".join(extract_definition(header,"struct "+name+" ")+";"
                           for name in ("SocialCategory","SocialEffect","Goal","PlayerData"))
    source=(ROOT / "src/council.cpp").read_text()
    helper=extract_definition(source,"static int wants_prop_signed_difference(")
    body=case(source)
    return r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <climits>
typedef int32_t BOOL;
const int MaxPlayerNum=8,PROP_SALVAGE_UNITY_CORE=2;
DECLARATIONS
static_assert(sizeof(PlayerData)==0x20CC,"native stride");
static_assert(offsetof(PlayerData,stolen_data_count)==0x200,"rejected hypothesis field");
static_assert(offsetof(PlayerData,energy_reserves)==0x220,"owner and rival energy");
static_assert(offsetof(PlayerData,soc_effect_base)+3*4==0x2DC,"rejected talent hypothesis");
static_assert(offsetof(PlayerData,soc_effect_base)+4*4==0x2E0,"rejected morale hypothesis");
static_assert(offsetof(PlayerData,unk_13)==0x2FC && offsetof(PlayerData,maint_cost_total)==0x300,"owner and rival net");
PlayerData PlayersData[8];
HELPER
static int actual(int faction){int score=0;switch(PROP_SALVAGE_UNITY_CORE){BODY}return -197;}
static int word(const PlayerData &p,int offset){int v;std::memcpy(&v,reinterpret_cast<const char *>(&p)+offset,4);return v;}
static int subtract(int a,int b){uint32_t bits=uint32_t(a)-uint32_t(b);int v;std::memcpy(&v,&bits,4);return v;}
// Independently decoded native operands and signed strict CMP branches.
static int original(int faction){
 const auto &owner=PlayersData[faction];int score=0;
 if(word(owner,0x3EC))++score;
 if(word(owner,0x3DC)<0)--score;
 const int energy=word(owner,0x220);
 if(energy>=500)--score;if(energy<100)++score;if(energy<200)++score;
 int relative=0;
 for(int peer=1;peer<8;++peer){if(peer==faction)continue;
  relative+=word(PlayersData[peer],0x220)>energy?1:-1;
  relative+=subtract(word(PlayersData[peer],0x2FC),word(PlayersData[peer],0x300))>
             subtract(word(owner,0x2FC),word(owner,0x300))?1:-1;
 }
 return relative>0?score+relative/2:score;
}
int main(){
 const int edge[]={INT_MIN,-501,-1,0,99,100,199,200,499,500,INT_MAX};
 for(int faction=0;faction<8;++faction)for(int energy:edge)for(int rival:edge)for(int net:edge){
  std::memset(PlayersData,0,sizeof(PlayersData));
  auto &owner=PlayersData[faction];owner.ai_power=1;owner.ai_fight=-1;
  owner.energy_reserves=energy;owner.unk_13=net;owner.maint_cost_total=0xFFFFFFFFu;
  for(int i=1;i<8;++i){if(i==faction)continue;
   auto &p=PlayersData[i];p.energy_reserves=rival;
   p.unk_13=net;p.maint_cost_total=i%2?INT_MIN:INT_MAX;
   // Decouple the rejected hypothesis from the real original operands.
   p.stolen_data_count[0]=uint32_t(energy==INT_MAX?INT_MIN:INT_MAX);
   p.soc_effect_base.values_[SocialEffect::TALENT]=net==INT_MAX?INT_MIN:INT_MAX;
   p.soc_effect_base.values_[SocialEffect::MORALE]=0;
  }
  assert(actual(faction)==original(faction));
 }
 // Equality takes both original decrement branches, not the increment side.
 std::memset(PlayersData,0,sizeof(PlayersData));
 assert(actual(1)==original(1));
 // Positive energy comparisons expose a wrong net equality increment.
 for(int i=2;i<8;++i)PlayersData[i].energy_reserves=1;
 assert(actual(1)==original(1));
}
'''.replace("DECLARATIONS",declarations).replace("HELPER",helper).replace("BODY",body)


@pytest.mark.parametrize("compiler",("g++","clang++"))
def test_actual_case_matches_original_signed_operands(tmp_path,compiler):
    program=control()
    run_control(tmp_path,compiler,program,None)
    for damage in (
        ("peer.energy_reserves > owner.energy_reserves", "static_cast<int>(peer.stolen_data_count[0]) > owner.energy_reserves"),
        ("peer.energy_reserves > owner.energy_reserves", "static_cast<uint32_t>(peer.energy_reserves) > static_cast<uint32_t>(owner.energy_reserves)"),
        ("peer.unk_13", "peer.soc_effect_base.values_[SocialEffect::TALENT]"),
        ("peer.maint_cost_total", "peer.soc_effect_base.values_[SocialEffect::MORALE]"),
        ("if (peer_net > owner_net)", "if (peer_net >= owner_net)"),
    ):
        run_control(tmp_path,compiler,program,damage)


def test_pinned_peer_operand_encodings_and_row_cursor():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    path=ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():pytest.skip("pinned original unavailable")
    image=PeImage(path.read_bytes(),image_pin(ROOT))
    for address,encoding in ((0x52B77D,"b9aced9600"),(0x52B78C,"8b9920ffffff"),
                             (0x52B7A8,"8b59fc"),(0x52B7AB,"2b19"),(0x52B7B8,"81c1cc200000")):
        expected=bytes.fromhex(encoding)
        assert image.read(Span(address,address+len(expected)))==expected
    assert 0x96EDAC==0x96C9E0+0x20CC+0x300
    assert 0x96EDAC-0xE0-0x20CC-0x96C9E0==0x220
    assert 0x96EDAC-4-0x20CC-0x96C9E0==0x2FC
