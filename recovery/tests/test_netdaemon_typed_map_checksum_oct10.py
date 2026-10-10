"""Actual Map declaration and tile walks; bounded controls, not game acceptance."""
from dataclasses import fields
from pathlib import Path
import subprocess

import pytest
from match_recovery.source import read_text, write
from test_net_hotseat_owner_actual_source_oct6 import extract_definition
from test_console_typed_owner_members_oct10 import run_control

ROOT = Path(__file__).resolve().parents[2]
BASE = "4c969d3f1357050990cb92dd08883aa4871ee241"
SIGNATURE = "void NetDaemon::game_checksum(int *results)"


def map_blocks(body):
    anchors = (
        ("\n    {\n        int countA = MapArea;", "\n    {\n        int idx2;"),
        ("\n    {\n        int rawVal = MapLatitudeBounds;", "\n    if (BaseAtDebugPopupFlag"),
        ("\n        {\n            int cnt1 = MapArea;", "\n        *out++ = checksum((char *)&MapSizePlanet"),
    )
    return tuple(body[body.index(start):body.index(end, body.index(start))]
                 for start, end in anchors)


def restore_map_checksum_migration(source):
    old = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT).decode()
    before = extract_definition(old, SIGNATURE)
    after = extract_definition(source, SIGNATURE)
    restored = after
    for now, prior in zip(map_blocks(after), map_blocks(before)):
        assert restored.count(now) == 1
        restored = restored.replace(now, prior, 1)
    return source.replace(after, restored, 1)


def test_only_map_walks_change_and_all_bindings_survive():
    path = ROOT / "src/netdaemon.cpp"
    source = path.read_text()
    from test_netdaemon_file_control_packets_oct10 import restore_file_control_packet_migration
    source = restore_file_control_packet_migration(source)
    old = subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"], cwd=ROOT).decode()
    assert restore_map_checksum_migration(source) == old
    rows = read_text(source, path)
    assert write(source, rows) == source
    previous = {r.address: r for r in read_text(old, path)}
    assert {r.address for r in rows} == set(previous)
    for row in rows:
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(previous[row.address], field.name)
        def lessons(r):
            return [(x.kind, x.key, x.prose, tuple(t for _, t in x.lines)) for x in r.origin]
        assert lessons(row) == lessons(previous[row.address])
    main, strips, debug = map_blocks(extract_definition(source, SIGNATURE))
    assert "map_tile_bytes" not in main + strips + debug
    assert main.count("Map *tile = MapTiles;") == 1
    assert debug.count("Map *tile = MapTiles;") == 6
    assert strips.index("int recIdx =") < strips.index("div = div + remainder;")


def control():
    declaration = extract_definition((ROOT / "src/map.h").read_text(), "struct Map ") + ";"
    blocks = map_blocks(extract_definition((ROOT / "src/netdaemon.cpp").read_text(), SIGNATURE))
    program = r'''
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
DECLARATION
static_assert(sizeof(Map)==0x2C,"native stride");
static_assert(offsetof(Map,climate)==0 && offsetof(Map,region)==3,"native bytes");
static_assert(offsetof(Map,bit)==8 && offsetof(Map,contour)==1,"native bytes");
static_assert(offsetof(Map,territory)==7 && offsetof(Map,bit2)==0xC,"native bytes");
static Map storage[2][2048];static Map *MapTiles;
static uint32_t MapArea,MapLongitude;
static int MapLatitudeBounds,MapLongitudeBounds;
struct Event{int length;unsigned char seed,result;std::vector<unsigned char> bytes;
 bool operator==(const Event &e)const{return length==e.length && seed==e.seed && result==e.result && bytes==e.bytes;}};
static std::vector<Event> events;static int mutation;
static unsigned char observe(char *p,int n,unsigned char seed){
 Event e{n,seed,seed,{}};
 for(int i=0;i<n;++i){unsigned char c=p[i];e.bytes.push_back(c);e.result=(e.result*33u)^c;}
 events.push_back(e);
 if(mutation && (events.size()==1 || events.size()==8)){
  // A service callback rebinds the global, not the already captured cursor.
  const bool first=events.size()==1;
  MapTiles=storage[first?1:0]+1024;
  MapArea=(mutation==1)==first?1:7;
  MapLongitude=3;MapLongitudeBounds=6;MapLatitudeBounds=99;
 }
 return e.result;
}
static unsigned char checksum(char *p,int n,unsigned char cs){return observe(p,n,cs);}
static void actual(int mode,unsigned char &cs,int *&out){
 if(mode==0){MAIN}else if(mode==1){STRIPS}else{DEBUG}
}
// Native instruction byte ranges; no member expressions or actual-loop reuse.
static void original(int mode,unsigned char &cs,int *&out){
 const int offsets[]={0,3,8,1,7,0xC};const int lengths[]={1,1,4,1,1,1};
 if(mode==0){
  int count=MapArea;char *p=reinterpret_cast<char *>(MapTiles);
  for(int row=0;row<count;++row){
   for(int field=0;field<6;++field)cs=observe(p+offsets[field],lengths[field],cs);
   count=MapArea;p+=0x2C;
  }
  *out++=cs;cs=0;
 }else if(mode==1){
  const int latitude=MapLatitudeBounds;
  int div=latitude/16;const int remainder=latitude-div*16;
  for(int strip=0;strip<16;++strip){
   const int row_start=strip*div;
   if(strip==15 && remainder)div+=remainder;
   cs=0;const int count=(MapLongitudeBounds>>1)*div;
   if(div!=0 && count>0){
    char *p=reinterpret_cast<char *>(MapTiles)+static_cast<int>(MapLongitude)*row_start*0x2C;
    for(int row=0;row<count;++row){
     for(int field=0;field<5;++field)cs=observe(p+offsets[field],lengths[field],cs);
     p+=0x2C;
    }
   }
   *out++=cs;
  }
 }else{
  for(int field=0;field<6;++field){
   int count=MapArea;char *p=reinterpret_cast<char *>(MapTiles)+offsets[field];
   for(int row=0;row<count;++row){cs=observe(p,lengths[field],cs);count=MapArea;p+=0x2C;}
   *out++=cs;if(field!=5)cs=0;
  }
 }
}
static void initialize(int area,int latitude,int longitude,int change,unsigned trial){
 uint32_t state=trial+0x76543210u;
 for(unsigned char *p=reinterpret_cast<unsigned char *>(storage),*end=p+sizeof(storage);p!=end;++p){state=state*1664525u+1013904223u;*p=state>>24;}
 MapTiles=storage[0]+1024;MapArea=area;MapLatitudeBounds=latitude;
 MapLongitudeBounds=longitude;MapLongitude=4;mutation=change;events.clear();
}
int main(){
 for(int area:{-1,0,1,5,128,256})for(int latitude:{-33,-17,-16,-1,0,1,15,16,17,31,32,33})
 for(int longitude:{-2,0,2,8})for(int change:{0,1,2})for(int mode=0;mode<3;++mode){
  int expected[18],actual_values[18];for(int i=0;i<18;++i)expected[i]=actual_values[i]=-197;
  initialize(area,latitude,longitude,change,2);unsigned char cs=0;int *out=expected+1;
  original(mode,cs,out);const auto trace=events;const auto last_cs=cs;
  const int written=out-(expected+1);const auto last_area=MapArea;
  const auto last_pointer=MapTiles;const auto last_longitude=MapLongitude;
  assert(written==(mode==0?1:mode==1?16:6));
  initialize(area,latitude,longitude,change,2);cs=0;out=actual_values+1;
  actual(mode,cs,out);
  assert(out==actual_values+1+written && cs==last_cs);
  assert(std::memcmp(expected,actual_values,sizeof(expected))==0 && events==trace);
  assert(MapArea==last_area && MapTiles==last_pointer && MapLongitude==last_longitude);
 }
}
'''.replace("DECLARATION", declaration)
    for token, block in zip(("MAIN", "STRIPS", "DEBUG"), blocks):
        program = program.replace(token, block)
    return program


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_map_walks_match_ranges_pointer_captures_and_strip_edges(tmp_path, compiler):
    program = control()
    run_control(tmp_path, compiler, program, None)
    for damage in (
        ("checksum((char *)&tile->bit2, 1, cs);\n                countA", "checksum((char *)&tile->bit2, 4, cs);\n                countA"),
        ("countA = MapArea;\n                ++tile;", "++tile;"),
        ("cnt2 = MapArea;\n                ++tile;", "++tile;"),
        ("++tile;\n                idxA++;", "tile = MapTiles + idxA + 1;\n                idxA++;"),
        ("Map *tile = MapTiles + static_cast<int>(MapLongitude) * recIdx;", "Map *tile = MapTiles + static_cast<int>(MapLongitude) * k * div;"),
        ("checksum((char *)&tile->territory, 1, cs);\n                        ++tile;", "checksum((char *)&tile->visibility, 1, cs);\n                        ++tile;"),
    ):
        run_control(tmp_path, compiler, program, damage)


def test_original_separate_map_pointer_captures_are_pinned():
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.exists():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), image_pin(ROOT))
    for address, encoding in (
        (0x52FA84,"8b1d0ca39400"),(0x52FD51,"8b1d0ca39400"),
        (0x52FFD2,"8b350ca39400"),(0x530017,"a10ca39400"),
        (0x53005C,"8b1d0ca39400"),(0x5300A3,"a10ca39400"),
        (0x5300E8,"a10ca39400"),(0x53012E,"a10ca39400"),
    ):
        expected=bytes.fromhex(encoding)
        assert image.read(Span(address,address+len(expected)))==expected
