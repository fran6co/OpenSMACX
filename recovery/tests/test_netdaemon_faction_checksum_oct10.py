"""Actual faction loops and declarations; bounded controls, not game acceptance."""
from dataclasses import fields
from pathlib import Path
import re
import subprocess

import pytest
from match_recovery.source import read_text, write
from test_net_hotseat_owner_actual_source_oct6 import extract_definition
from test_console_typed_owner_members_oct10 import run_control

ROOT = Path(__file__).resolve().parents[2]
BASE = "4f8119d85ccd3cc1e3082637a07729a7e5e6bff9"
SIGNATURE = "void NetDaemon::game_checksum(int *results)"
OFFSETS = {
    "tech_accumulated": 0x3C8, "tech_id_researching": 0x3CC,
    "planet_ecology": 0x464, "energy_reserves": 0x220,
    "soc_category_pending": 0x228, "satellites_": 0x808,
    "diplo_treaties": 0x18, "diplo_agenda": 0x38,
    "integrity_blemishes": 0x124, "diplo_friction": 0x58,
    "diplo_patience": 0xB8, "diplo_merc": 0x98,
    "diplo_spoke": 0x78, "diplo_wrongs": 0x14C,
}


def faction_blocks(body, *, previous=False):
    starts = (
        "\n    {\n        char *p;" if previous else "\n    {\n        // The original cursor",
        "\n        {\n            char *p;" if previous else "\n        {\n            for (int faction",
    )
    ends = ("\n    *out++ = cs;", "\n        cs = 0;")
    return tuple(body[body.index(start):body.index(end, body.index(start))]
                 for start, end in zip(starts, ends))


def test_only_two_walks_change_and_all_original_obligations_survive():
    path = ROOT / "src/netdaemon.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT).decode()
    current = path.read_text()
    from test_netdaemon_typed_map_checksum_oct10 import restore_map_checksum_migration
    current = restore_map_checksum_migration(current)
    # Later authorized packet migration has its own whole-source continuity
    # and actual-source controls; reverse only that bounded step here.
    from test_netdaemon_typed_bulk_landmark_oct10 import restore_packet_migration
    from test_netdaemon_vehicle_base_checksum_oct10 import restore_vehicle_base_checksum_migration
    current = restore_vehicle_base_checksum_migration(current)
    current = restore_packet_migration(current)
    before = extract_definition(old, SIGNATURE)
    after = extract_definition(current, SIGNATURE)
    restored = after
    for current_block, old_block in zip(faction_blocks(after), faction_blocks(before, previous=True)):
        restored = restored.replace(current_block, old_block, 1)
    assert restored == before
    assert current.replace(after, before, 1) == old
    rows = read_text(current, path)
    assert write(current, rows) == current
    prior = {r.address: r for r in read_text(old, path)}
    assert {r.address for r in rows} == set(prior)
    for row in rows:
        previous = prior[row.address]
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(previous, field.name)
        def lessons(r):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines)) for x in r.origin]
        assert lessons(row) == lessons(previous)
    ordinary, debug = faction_blocks(after)
    assert "Bases" not in debug
    assert "(char *)Bases" not in ordinary
    assert ordinary.index("masked1[idx4] =") < ordinary.index("masked2[idx4] =")
    assert "PlayerData &player = PlayersData[faction];" in ordinary
    assert "PlayerData &player = PlayersData[faction];" in debug


def control():
    header = (ROOT / "src/faction.h").read_text()
    declarations = "\n".join(extract_definition(header, "struct " + name)
                             + ";" for name in ("SocialCategory", "SocialEffect", "Goal", "PlayerData"))
    bound = re.search(r"(?m)^static const int MaxPlayerNum = 8;", header)[0]
    assertions = "\n".join("static_assert(offsetof(PlayerData, %s)==%s, \"native anchor\");"
                           % (name, hex(offset)) for name, offset in OFFSETS.items())
    ordinary, debug = faction_blocks(extract_definition((ROOT / "src/netdaemon.cpp").read_text(), SIGNATURE))
    return r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
typedef int32_t BOOL;
DECLARATIONS
BOUND
ASSERTIONS
static_assert(sizeof(PlayerData)==0x20CC,"native row stride");
static_assert(sizeof(PlayerData::diplo_patience)==8,"cross-member range is deliberate");
PlayerData PlayersData[MaxPlayerNum];
struct Event {
    int length;unsigned char seed,result;std::vector<unsigned char> bytes;
    bool operator==(const Event &b)const{
        return length==b.length && seed==b.seed && result==b.result && bytes==b.bytes;
    }
};
static std::vector<Event> events;
static bool mutate;
static unsigned char observe(char *p,int n,unsigned char seed){
    Event e{n,seed,seed,{}};
    for(int i=0;i<n;++i){
        const unsigned char c=static_cast<unsigned char>(p[i]);
        e.bytes.push_back(c);e.result=static_cast<unsigned char>((e.result*33u)^c);
    }
    events.push_back(e);
    if(mutate){
        const size_t at=events.size();
        if(at==1)PlayersData[1].energy_reserves=0x10203040;
        if(at==4){PlayersData[1].sanction_turns=0x11223344;PlayersData[1].loan_balance[4]=0x55667788;}
        if(at==6){PlayersData[1].diplo_treaties[2]=0xFEDCBA98u;PlayersData[1].diplo_agenda[3]=-17;}
        if(at==7)PlayersData[2].tech_accumulated=-91;
        if(at==14)PlayersData[3].planet_ecology=-79;
    }
    return e.result;
}
static unsigned char checksum(char *p,int n,unsigned char seed){return observe(p,n,seed);}
static void actual_ordinary(unsigned char &cs,int *&out){
ORDINARY
    *out++=cs;cs=0;
}
static void actual_debug(unsigned char &cs,int *&out){
DEBUG
    cs=0;
}
// Independent original offset/range oracle; it has no host Bases allocation.
static void original(bool debug,unsigned char &cs,int *&out){
    for(int row=1;row<8;++row){
        char *p=reinterpret_cast<char *>(PlayersData)+row*0x20CC;
        cs=observe(p+0x3C8,4,debug?0:cs);
        cs=observe(p+0x3CC,4,cs);
        cs=observe(p+0x464,4,cs);
        cs=observe(p+0x220,4,cs);
        cs=observe(p+0x228,16,cs);
        cs=observe(p+0x808,16,cs);
        if(debug){
            for(int offset:{0x18,0x38,0x58,0xB8,0x98,0x78,0x14C})cs=observe(p+offset,32,cs);
        }else{
            unsigned int a[8],b[8];
            for(int i=0;i<8;++i){
                unsigned int x,y;std::memcpy(&x,p+0x18+i*4,4);std::memcpy(&y,p+0x38+i*4,4);
                a[i]=x&0x0B1D3C7Fu;b[i]=y&0x1E68u;
            }
            cs=observe(reinterpret_cast<char *>(a),32,cs);
            cs=observe(reinterpret_cast<char *>(b),32,cs);
        }
        cs=observe(p+0x124,4,cs);
        if(debug)*out++=cs;
    }
    if(!debug)*out++=cs;
    cs=0;
}
static void initialize(unsigned trial){
    unsigned char *p=reinterpret_cast<unsigned char *>(PlayersData);
    uint32_t state=0x12345678u+trial;
    for(size_t i=0;i<sizeof(PlayersData);++i){state=state*1664525u+1013904223u;p[i]=state>>24;}
    events.clear();
}
int main(){
    for(unsigned trial=0;trial<8;++trial)for(bool debug:{false,true})for(bool live:{false,true}){
        mutate=live;initialize(trial);
        int expected[10],actual[10];
        for(int i=0;i<10;++i)expected[i]=actual[i]=-197;
        unsigned char cs=0;int *out=expected+1;
        original(debug,cs,out);
        const auto trace=events;const size_t written=out-(expected+1);
        assert(written==(debug?7u:1u));
        initialize(trial);cs=0;out=actual+1;
        if(debug)actual_debug(cs,out);else actual_ordinary(cs,out);
        assert(out==actual+1+written && cs==0);
        assert(std::memcmp(actual,expected,sizeof(actual))==0);
        assert(events==trace); // Every original input byte, seed, range and order.
    }
}
'''.replace("DECLARATIONS", declarations).replace("BOUND", bound).replace(
        "ASSERTIONS", assertions).replace("ORDINARY", ordinary).replace("DEBUG", debug)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_loops_match_original_ranges_seeds_outputs_and_live_reads(tmp_path, compiler):
    program = control()
    run_control(tmp_path, compiler, program, None)
    for damage in (
        ("for (int faction = 1; faction < MaxPlayerNum; ++faction) {\n            PlayerData",
         "for (int faction = 0; faction < MaxPlayerNum; ++faction) {\n            PlayerData"),
        ("for (int faction = 1; faction < MaxPlayerNum; ++faction) {\n                PlayerData",
         "for (int faction = 1; faction < MaxPlayerNum - 1; ++faction) {\n                PlayerData"),
        ("player.diplo_treaties[idx4] & 0xb1d3c7f", "player.diplo_treaties[idx4] & 0xb1d3c7e"),
        ("player.diplo_agenda[idx4] & 0x1e68", "player.diplo_agenda[idx4] & 0x1e60"),
        ("checksum((char *)&player.tech_accumulated, 4, 0)", "checksum((char *)&player.tech_accumulated, 4, cs)"),
        ("offsetof(PlayerData, diplo_patience), 0x20, cs", "offsetof(PlayerData, diplo_patience), 8, cs"),
        ("checksum((char *)player.diplo_wrongs, 0x20, cs)", "checksum((char *)player.diplo_spoke, 0x20, cs)"),
        ("*out++ = cs;", "*out = cs;"),
    ):
        run_control(tmp_path, compiler, program, damage)


def test_original_seven_row_bounds_are_pinned_not_inferred_from_host_order():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source.asm import Listing
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    for start, end in ((0x52F975, 0x52FA6D), (0x52FE80, 0x52FFCD)):
        lines = "\n".join(Listing(image.read(Span(start, end)), start).lines).lower()
        assert "0x96ee78" in lines
        assert "0x20cc" in lines
        assert "0x97d40c" in lines
        assert "jl " in lines
    assert 0x96EE78 == 0x96C9E0 + 0x20CC + OFFSETS["tech_id_researching"]
    assert 0x97D40C == 0x96C9E0 + 8 * 0x20CC + OFFSETS["tech_id_researching"]
