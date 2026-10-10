"""Actual owner loops against original byte ranges; not full-game acceptance."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import read_text, write
from test_net_hotseat_owner_actual_source_oct6 import extract_definition
from test_console_typed_owner_members_oct10 import run_control

ROOT = Path(__file__).resolve().parents[2]
BASE = "f21886f1e9606f78fe10f59774c07cc977e7b984"
SIGNATURE = "void NetDaemon::game_checksum(int *results)"


def checksum_blocks(body):
    anchors = (
        ("\n    {\n        int idx2;", "\n    {\n        int countC ="),
        ("\n    {\n        int countC =", "\n    {\n        int rawVal ="),
        ("\n        {\n            int idx2b;", "\n        cs = 0;\n        {\n            int countC2 ="),
        ("\n        {\n            int countC2 =", "\n    }\n}"),
    )
    return tuple(body[body.index(start):body.index(end, body.index(start))]
                 for start, end in anchors)


def restore_vehicle_base_checksum_migration(source):
    """Reverse only four authorized segments for earlier continuity controls."""
    old = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT).decode()
    before = extract_definition(old, SIGNATURE)
    after = extract_definition(source, SIGNATURE)
    restored = after
    for now, previous in zip(checksum_blocks(after), checksum_blocks(before)):
        assert restored.count(now) == 1
        restored = restored.replace(now, previous, 1)
    return source.replace(after, restored, 1)


def test_only_four_segments_change_and_original_obligations_survive():
    path = ROOT / "src/netdaemon.cpp"
    current = path.read_text()
    old = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT).decode()
    assert restore_vehicle_base_checksum_migration(current) == old
    rows = read_text(current, path)
    assert write(current, rows) == current
    prior = {r.address: r for r in read_text(old, path)}
    assert {r.address for r in rows} == set(prior)
    for row in rows:
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(prior[row.address], field.name)
        def lessons(r):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines)) for x in r.origin]
        assert lessons(row) == lessons(prior[row.address])
    normal, _, debug, _ = checksum_blocks(extract_definition(current, SIGNATURE))
    assert normal.index("int countB =") < normal.index("for (idx2")
    assert normal.index("countB =", normal.index("do {")) < normal.index("\n                    }", normal.index("do {"))
    assert debug.index("for (idx2b") < debug.index("int countB2 =")
    assert "}\n                        countB2 =" in debug
    assert "*(char *)&GameDataStateStorage" not in normal + debug


def control():
    declarations = "\n".join(extract_definition((ROOT / ("src/" + header)).read_text(), "struct " + name + " ") + ";"
                             for header, name in (("veh.h", "Veh"), ("base.h", "Base")))
    blocks = checksum_blocks(extract_definition((ROOT / "src/netdaemon.cpp").read_text(), SIGNATURE))
    program = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
DECLARATIONS
static_assert(sizeof(Veh)==0x34 && sizeof(Base)==0x134,"native strides");
static_assert(offsetof(Veh,proto_id)==0xA && offsetof(Veh,unk_1)==0xC,"partial field");
static_assert(offsetof(Veh,faction_id)==0xE && offsetof(Veh,home_base_id)==0x2E,"vehicle anchors");
static_assert(offsetof(Base,nutrients_accumulated)==0x3C && offsetof(Base,spec)==0x84,"base anchors");
static_assert(offsetof(Base,facilities_built)==0x8C,"nine-byte facility range");
static Veh Vehs[300];static Base Bases[300];
struct {struct {int vehicle_current_count_,base_current_count_;} fields_;} GameDataStateStorage;
struct Event {int length;unsigned char seed,result;std::vector<unsigned char> bytes;
 bool operator==(const Event &b)const{return length==b.length && seed==b.seed && result==b.result && bytes==b.bytes;}};
static std::vector<Event> events;
static int mutation,initial_count;
static unsigned char observe(char *p,int n,unsigned char seed){
 Event e{n,seed,seed,{}};
 for(int i=0;i<n;++i){unsigned char c=p[i];e.bytes.push_back(c);e.result=(e.result*33u)^c;}
 events.push_back(e);
 if(mutation && events.size()==1){
  int next=mutation==1?1:initial_count+2;
  GameDataStateStorage.fields_.vehicle_current_count_=next;
  GameDataStateStorage.fields_.base_current_count_=next;
 }
 return e.result;
}
static unsigned char checksum(char *p,int n,unsigned char cs){return observe(p,n,cs);}
static void actual_vehicle(bool debug,unsigned char &cs,int *&out){
 if(debug){DEBUG_VEHICLE}else{NORMAL_VEHICLE}
}
static void actual_base(bool debug,unsigned char &cs,int *&out){
 if(debug){DEBUG_BASE}else{NORMAL_BASE}
}
// Independent original instruction ranges and loop schedules, not old source.
static void original_vehicle(bool debug,unsigned char &cs,int *&out){
 int count=GameDataStateStorage.fields_.vehicle_current_count_;
 for(int faction=0;faction<8;++faction){
  if(debug){count=GameDataStateStorage.fields_.vehicle_current_count_;cs=0;}
  for(int row=0;row<count;++row){
   char *p=reinterpret_cast<char *>(Vehs)+row*0x34;
   bool match=static_cast<unsigned char>(p[0xE])==faction;
   if(match){
    const int normal_offsets[]={0,2,0xA,0xE,0x10,0x24,0x28,0x2E};
    const int normal_lengths[]={2,2,2,1,1,1,1,2};
    const int debug_offsets[]={0,2,0xA,0x24,0x28,0x2E};
    const int debug_lengths[]={2,2,3,1,1,2};
    for(int i=0;i<(debug?6:8);++i)
     cs=observe(p+(debug?debug_offsets[i]:normal_offsets[i]),debug?debug_lengths[i]:normal_lengths[i],cs);
   }
   if(debug||match)count=GameDataStateStorage.fields_.vehicle_current_count_;
  }
  if(debug)*out++=cs;
 }
 if(!debug){*out++=cs;cs=0;}
}
static void original_base(unsigned char &cs,int *&out){
 int count=GameDataStateStorage.fields_.base_current_count_;
 const int offsets[]={0,0x3C,0x40,0x4C,0x50,0x78,0x84,0x8C};
 const int lengths[]={7,4,4,4,4,4,8,9};
 for(int row=0;row<count;++row){
  char *p=reinterpret_cast<char *>(Bases)+row*0x134;
  for(int i=0;i<8;++i)cs=observe(p+offsets[i],lengths[i],cs);
  count=GameDataStateStorage.fields_.base_current_count_;
 }
 *out++=cs;
}
static void initialize(int count,int mode,unsigned trial){
 uint32_t state=trial+0x10203040u;
 for(unsigned char *p=reinterpret_cast<unsigned char *>(Vehs),*e=p+sizeof(Vehs);p!=e;++p){state=state*1664525u+1013904223u;*p=state>>24;}
 for(unsigned char *p=reinterpret_cast<unsigned char *>(Bases),*e=p+sizeof(Bases);p!=e;++p){state=state*1664525u+1013904223u;*p=state>>24;}
 for(int i=0;i<300;++i)Vehs[i].faction_id=i%8;
 initial_count=count;mutation=mode;events.clear();
 GameDataStateStorage.fields_.vehicle_current_count_=count;
 GameDataStateStorage.fields_.base_current_count_=count;
}
int main(){
 for(int count:{-1,0,1,127,128,255,256})for(int mode:{0,1,2})
 for(unsigned trial=0;trial<3;++trial)for(bool debug:{false,true})for(bool vehicles:{false,true}){
  int expected[10],actual[10];for(int i=0;i<10;++i)expected[i]=actual[i]=-197;
  initialize(count,mode,trial);unsigned char cs=vehicles?0:17;int *out=expected+1;
  if(vehicles)original_vehicle(debug,cs,out);else original_base(cs,out);
  const auto trace=events;const int written=out-(expected+1);const auto final_cs=cs;
  const int vehicle_count=GameDataStateStorage.fields_.vehicle_current_count_;
  const int base_count=GameDataStateStorage.fields_.base_current_count_;
  assert(written==(vehicles&&debug?8:1));
  initialize(count,mode,trial);cs=vehicles?0:17;out=actual+1;
  if(vehicles)actual_vehicle(debug,cs,out);else actual_base(debug,cs,out);
  // Ordinary base resets after its output; debug base leaves the final seed.
  if(!vehicles&&!debug)assert(cs==0);else assert(cs==final_cs);
  assert(out==actual+1+written && std::memcmp(actual,expected,sizeof(actual))==0);
  assert(events==trace);
  assert(GameDataStateStorage.fields_.vehicle_current_count_==vehicle_count);
  assert(GameDataStateStorage.fields_.base_current_count_==base_count);
 }
}
'''.replace("DECLARATIONS", declarations)
    for token, block in zip(("NORMAL_VEHICLE", "NORMAL_BASE", "DEBUG_VEHICLE", "DEBUG_BASE"), blocks):
        program = program.replace(token, block)
    return program


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_loops_ranges_outputs_full_counts_and_live_bounds(tmp_path, compiler):
    program = control()
    run_control(tmp_path, compiler, program, None)
    for damage in (
        ("int countB = GameDataStateStorage.fields_.vehicle_current_count_;", "int countB = *(signed char *)&GameDataStateStorage.fields_.vehicle_current_count_;"),
        ("int countB2 = GameDataStateStorage.fields_.vehicle_current_count_;", "int countB2 = *(signed char *)&GameDataStateStorage.fields_.vehicle_current_count_;"),
        ("countC = GameDataStateStorage.fields_.base_current_count_;\n                idxC++;", "idxC++;"),
        ("countC2 = GameDataStateStorage.fields_.base_current_count_;\n                    idxC2++;", "idxC2++;"),
        ("offsetof(Veh, proto_id), 3, cs", "offsetof(Veh, proto_id), 2, cs"),
        ("checksum((char *)base->facilities_built, 9, cs);\n                countC", "checksum((char *)base->facilities_built, 12, cs);\n                countC"),
        ("                *out++ = cs;\n            }", "                *out = cs;\n            }"),
    ):
        run_control(tmp_path, compiler, program, damage)


def test_original_counter_loads_backedges_and_debug_outputs():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    # These bytes independently distinguish dword MOVs from signed byte loads.
    for address, encoding in (
        (0x52FB2D, "a1c8649a00"), (0x52FBEC, "a1c8649a00"),
        (0x53018B, "8b0dc8649a00"), (0x53021C, "8b0dc8649a00"),
        (0x52FCCD, "a1cc649a00"), (0x5302F5, "a1cc649a00"),
    ):
        expected = bytes.fromhex(encoding)
        assert image.read(Span(address, address + len(expected))) == expected
    from match_recovery.source.asm import Listing
    ordinary = "\n".join(Listing(image.read(Span(0x52FBF7, 0x52FC13)), 0x52FBF7).lines).lower()
    debug = "\n".join(Listing(image.read(Span(0x53021C, 0x53024A)), 0x53021C).lines).lower()
    assert "0x52fb39" in ordinary
    assert "0x53018b" in debug and "cmp eax, 8" in debug
    assert image.read(Span(0x530231, 0x530234)) == bytes.fromhex("83c704")
    assert image.read(Span(0x53023B, 0x53023E)) == bytes.fromhex("8957fc")
