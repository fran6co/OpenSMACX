"""Actual farm_compute with original-derived bounded state/callback controls.

Uses real Base/Map/Veh declarations, the real PlayerData prefix through its
treaties, actual geometry helpers and radius tables, and the complete producer.
Expected flags use independently listed original farm coordinates and explicit
scenarios. Callees/map storage are stateful doubles, not working game services.
Host layout checks do not establish native ABI, initialized game state, or the
unrecovered consumers. Fixtures keep all faction/vehicle/map indices legal.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def braced(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if not depth:
            return source[start:end + 1]
    raise AssertionError("unterminated actual declaration")


def actual_source():
    declarations = []
    for filename, name in (("base.h", "Base"), ("map.h", "Map"), ("veh.h", "Veh")):
        declarations.append(braced((ROOT / "src" / filename).read_text(), "struct " + name + " {") + ";")
    player = (ROOT / "src/faction.h").read_text()
    start = player.index("struct PlayerData {")
    end = player.index("uint32_t diplo_treaties[8];", start) + len("uint32_t diplo_treaties[8];")
    declarations.append(player[start:end] + "\n};")
    for filename,names in (("faction.h",("add_goal",)),("map.h",("goody_at","whose_territory")),
                           ("veh.h",("veh_at","stack_fix","stack_check"))):
        public=(ROOT/"src"/filename).read_text()
        for name in names:
            prototype,=re.findall(r"(?:int|void) __cdecl "+name+r"\([^;{}]+\);",public)
            declarations.append(prototype)
    header = (ROOT / "src/map.h").read_text()
    for filename,names in (("map.h",("MapLongitude","MapLongitudeBounds","MapLatitudeBounds","MapIsFlat")),
                           ("base.h",("BaseUpkeepStage",))):
        public=(ROOT/"src"/filename).read_text()
        for name in names:
            declaration,=re.findall(r"extern (?:int|uint32_t|BOOL) "+name+r";",public)
            declarations.append(declaration.removeprefix("extern "))
    for name in ("RadiusOffsetX", "RadiusOffsetY"):
        table, = re.findall(r"const int " + name + r"\[\]\s*=\s*\{[^}]*\};", header)
        declarations.append(table)
    helpers = "\n".join(braced(header, signature) for signature in (
        "MEASURED inline BOOL __cdecl on_map(", "MEASURED inline int __cdecl xrange(",
        "MEASURED inline int __cdecl x_dist(", "MEASURED inline int __cdecl vector_dist("))
    source = (ROOT / "src/base.cpp").read_text()
    for pattern in (r"uint32_t FarmStatus\[25\];", r"uint32_t FarmUnavailableMask;"):
        definition, = re.findall(pattern, source)
        declarations.append(definition)
    return "\n".join(declarations), helpers, braced(source, "void __cdecl farm_compute()")


HARNESS = r'''
#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <type_traits>
#include <initializer_list>
#define MEASURED
#define __cdecl
typedef int BOOL;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d case %d\n",line,0);std::exit(97);}}
#define require(x) check((x),__LINE__)
ACTUAL_DECLARATIONS
Base Bases[8],*BaseCurrent;
int BaseCurrentCount,BaseIDCurrentSelected;
PlayerData PlayersData[8];Veh Vehs[16];
Map grids[3][4096];
const int offset[21][2]={{0,0},{1,-1},{2,0},{1,1},{0,2},{-1,1},{-2,0},{-1,-1},{0,-2},
    {2,-2},{2,2},{-2,2},{-2,-2},{1,-3},{3,-1},{3,1},{1,3},{-1,3},{-3,1},{-3,-1},{-1,-3}};
int entry_x,entry_y,entry_faction,active_grid,active_stride;
int goody[21],raw_vehicle[21],fixed_vehicle[21],stack_answer[21],territory[21];
uint32_t wanted[25];
int active_position,goody_calls,veh_calls,fix_calls,stack_calls,territory_calls,goal_calls,map_reads;
int mutate_goody,mutate_goals,mutate_territory,change_current,change_selected,change_count;
std::vector<std::string> events;
int cx(int position){
    int x=entry_x+offset[position][0];
    if(!(MapIsFlat&1)){x=(x+int(MapLongitudeBounds))%int(MapLongitudeBounds);}
    return x;
}
int cy(int position){return entry_y+offset[position][1];}
bool valid(int position){return cx(position)>=0 && cx(position)<int(MapLongitudeBounds) && cy(position)>=0 && cy(position)<int(MapLatitudeBounds);}
int position_at(int x,int y){
    for(int p=0;p<21;++p)if(cx(p)==x && cy(p)==y)return p;
    require(false);return -1;
}
void event(const char *name,int position){events.push_back(std::string(name)+std::to_string(position));}
Map *map_tiles(){++map_reads;return grids[active_grid];}
int goody_at(int x,int y){
    int p=position_at(x,y);require(valid(p));event("G",p);++goody_calls;
    if(mutate_goody && p==0){active_grid=1;MapLongitude=40;active_stride=40;}
    return goody[p];
}
void add_goal(int faction,int type,int priority,int x,int y,int base){
    require(faction==entry_faction && priority==1 && base==-1 && x==cx(0) && y==cy(0));
    const int types[3]={3,2,0};require(type==types[goal_calls%3]);++goal_calls;event("A",type);
    if(mutate_goals && goal_calls==3){active_grid=2;MapLongitude=48;active_stride=48;}
}
int veh_at(int x,int y){
    int p=position_at(x,y);require(valid(p));active_position=p;event("V",p);++veh_calls;return raw_vehicle[p];
}
int stack_fix(int vehicle){
    require(vehicle==raw_vehicle[active_position]);event("F",active_position);++fix_calls;return fixed_vehicle[active_position];
}
int stack_check(int vehicle,int type,int c1,int c2,int c3){
    require(vehicle==fixed_vehicle[active_position] && type==9 && c1==3 && c2==-1 && c3==-1);
    event("S",active_position);++stack_calls;return stack_answer[active_position];
}
int whose_territory(int faction,int x,int y,int *base,BOOL flag){
    int p=position_at(x,y);require(faction==entry_faction && base==nullptr && flag==0);
    require(active_position==p);event("T",p);++territory_calls;
    if(mutate_territory && p==0){active_grid=1;MapLongitude=40;active_stride=40;}
    if(change_current && p==0)BaseCurrent=&Bases[2];
    if(change_selected && p==0)BaseIDCurrentSelected=1;
    if(change_count && p==0)BaseCurrentCount=2;
    return territory[p];
}
ACTUAL_HELPERS
ACTUAL_BODY
void reset(int x=20,int y=12,int faction=3,bool flat=false){
    std::memset(Bases,0,sizeof(Bases));std::memset(PlayersData,0,sizeof(PlayersData));
    std::memset(Vehs,0,sizeof(Vehs));std::memset(grids,0,sizeof(grids));
    BaseCurrent=&Bases[0];Bases[0].x=x;Bases[0].y=y;Bases[0].faction_id_current=faction;
    BaseIDCurrentSelected=0;BaseCurrentCount=1;BaseUpkeepStage=0;
    entry_x=x;entry_y=y;entry_faction=faction;MapLongitude=32;MapLongitudeBounds=64;MapLatitudeBounds=32;MapIsFlat=flat;
    active_grid=0;active_stride=32;active_position=-1;
    goody_calls=veh_calls=fix_calls=stack_calls=territory_calls=goal_calls=map_reads=0;
    mutate_goody=mutate_goals=mutate_territory=change_current=change_selected=change_count=0;events.clear();
    for(int p=0;p<25;++p){FarmStatus[p]=0xabad1234;wanted[p]=p>=21?1:valid(p)?0:2;}
    FarmUnavailableMask=0xffffffff;
    for(int p=0;p<21;++p){goody[p]=0;raw_vehicle[p]=fixed_vehicle[p]=territory[p]=-1;stack_answer[p]=0;}
    for(auto &grid:grids)for(auto &tile:grid){tile.visibility=uint8_t(1u<<faction);tile.val2=15;tile.climate=0x60;}
    for(auto &v:Vehs){v.faction_id=faction;v.prev_veh_id_stack=v.next_veh_id_stack=-1;}
}
Map &tile(int position,int grid=0,int stride=32){require(valid(position));return grids[grid][cy(position)*stride+cx(position)/2];}
void verify(){
    farm_compute();uint32_t mask=0;
    for(int p=0;p<25;++p){
        if(FarmStatus[p]!=wanted[p])std::fprintf(stderr,"position%d got%u wanted%u\n",p,FarmStatus[p],wanted[p]);
        require(FarmStatus[p]==wanted[p]);if(p<21 && wanted[p])mask|=1u<<p;
    }
    require(FarmUnavailableMask==mask && (FarmUnavailableMask&0xffe00000u)==0);
}
void chain(){
    raw_vehicle[0]=10;fixed_vehicle[0]=5;
    Vehs[5].prev_veh_id_stack=3;Vehs[3].next_veh_id_stack=5;
    Vehs[5].next_veh_id_stack=7;Vehs[7].prev_veh_id_stack=5;
}
void geometry(){
    for(int faction:{0,3,7})for(bool flat:{false,true})
      for(auto center:{std::vector<int>{0,0},std::vector<int>{63,31},std::vector<int>{-1,1},std::vector<int>{20,-2},std::vector<int>{20,12}}){
        reset(center[0],center[1],faction,flat);verify();int count=0;
        for(int p=0;p<21;++p)count+=valid(p);
        require(veh_calls==count && fix_calls==count && territory_calls==count && goody_calls==0);
        std::vector<std::string> expected_events;
        for(int p=0;p<21;++p)if(valid(p))for(const char *name:{"V","F","T"})expected_events.push_back(std::string(name)+std::to_string(p));
        require(events==expected_events);
    }
    reset();for(int p=0;p<21;++p){tile(p).visibility=0;wanted[p]=2;}verify();
    require(veh_calls==0 && territory_calls==0 && map_reads==21);
    reset();PlayersData[3].flags=0x200;for(int p=0;p<21;++p)tile(p).visibility=0;verify();
    require(veh_calls==21 && territory_calls==21);
    reset();tile(0).visibility=0;tile(0).val2=8;wanted[0]=2;verify(); // visibility is +4, not +2
}
void flags(){
    for(int owner:{0,7,8,15})for(int bits:{0,1,2}){
        reset();tile(0).val2=uint8_t(0xA0|owner);tile(0).bit=bits;
        if((bits&1) && owner<8)wanted[0]=4;verify();
    }
    for(int location:{3,5,7})for(bool visible:{false,true})for(int treaty:{0,1,2,4}){
        reset();chain();Vehs[location].faction_id=5;Vehs[location].visibility=visible?8:0;
        PlayersData[3].diplo_treaties[5]=treaty;PlayersData[5].diplo_treaties[3]=treaty?0:3;
        if(visible && !(treaty&3))wanted[0]=8;verify();require(stack_calls==1);
    }
    reset();chain();stack_answer[0]=5;wanted[0]=8;verify();
    for(int owner:{-1,3,5})for(int treaty:{0,1,2})for(bool vehicles:{false,true}){
        reset();if(vehicles)chain();territory[0]=owner;PlayersData[3].diplo_treaties[5]=treaty;
        if(owner==5)wanted[0]=32;verify();
    }
    reset();chain();stack_answer[0]=1;tile(0).bit=1;tile(0).val2=3;territory[0]=5;
    wanted[0]=4|8|32;verify();
}
void callback_reloads(){
    reset();BaseUpkeepStage=1;goody[0]=1;mutate_goody=mutate_goals=1;
    for(auto &grid:grids)for(auto &t:grid)t.visibility=0;
    tile(0).climate=0x40; // pre-goody tile says ocean; post-goody says land
    tile(0,1,40).climate=0x60;
    tile(0,2,48).visibility=8;tile(0,2,48).bit=1;tile(0,2,48).val2=3;
    for(int p=0;p<21;++p)wanted[p]=2;wanted[0]=4;
    verify();require(goal_calls==3 && goody_calls==21 && territory_calls==1);
    require((std::vector<std::string>(events.begin(),events.begin()+7)==std::vector<std::string>{"G0","A3","A2","A0","V0","F0","T0"}));
    reset();BaseUpkeepStage=1;goody[0]=1;tile(0).climate=0x40;verify();require(goal_calls==0 && goody_calls==21);
    reset();BaseUpkeepStage=1;mutate_goody=1;tile(0,1,40).bit=1;tile(0,1,40).val2=3;wanted[0]=4;
    verify();require(goody_calls==21 && goal_calls==0); // false goody still refreshes stride
    reset();BaseUpkeepStage=2;goody[0]=1;verify();require(goody_calls==0 && goal_calls==0);
    reset();mutate_territory=1;tile(1,1,40).bit=1;tile(1,1,40).val2=3;wanted[1]=4;verify();
    require(MapLongitude==40 && active_grid==1);
}
int independent_distance(int x1,int y1,int x2,int y2){
    int dx=std::abs(x1-x2);if(!(MapIsFlat&1))dx=std::min(dx,int(MapLongitudeBounds)-dx);
    int dy=std::abs(y1-y2);return std::max(dx,dy)-(std::abs(dx-dy)+2)/4;
}
void expect_neighbor(int id,int live_x,int live_y){
    const Base &other=Bases[id];
    if(independent_distance(live_x,live_y,other.x,other.y)>5)return;
    for(int slot=0;slot<21;++slot){
        if(slot && !(other.resource_sqr_radius&(1u<<slot)))continue;
        int x=other.x+offset[slot][0],y=other.y+offset[slot][1];
        if(!(MapIsFlat&1))x=(x+int(MapLongitudeBounds))%int(MapLongitudeBounds);
        if(x<0 || x>=int(MapLongitudeBounds) || y<0 || y>=int(MapLatitudeBounds))continue;
        if(other.faction_id_current!=entry_faction && (grids[active_grid][y*active_stride+x/2].climate&0xE0)>=0x60)continue;
        // Set intersection of explicitly enumerated original farm positions;
        // no source Radius arrays or vector_dist call is used by the oracle.
        for(int p=0;p<21;++p)if(cx(p)==x && cy(p)==y)wanted[p]|=16;
    }
}
void neighbors(){
    for(bool flat:{false,true})for(int x:{0,20,63})for(int slot:{0,1,7,13,20})for(bool foreign:{false,true})for(bool ocean:{false,true}){
        reset(x,12,3,flat);BaseCurrentCount=2;Bases[1].x=(x+2)%64;Bases[1].y=12;
        Bases[1].faction_id_current=foreign?5:3;Bases[1].resource_sqr_radius=slot?1u<<slot:0;
        Bases[1].nutrients_accumulated=~Bases[1].resource_sqr_radius;
        for(auto &t:grids[0])t.climate=ocean?0x40:0x60;
        expect_neighbor(1,x,12);verify();
    }
    // Query-time pointer changes affect outer distance only. Inner matching
    // must still use the entry center, not the replacement BaseCurrent.
    for(int shift:{7,8}){
        reset();BaseCurrentCount=3;Bases[1]=Bases[0];Bases[2]=Bases[0];
        Bases[2].x=20+shift;Bases[2].y=12+(shift==7);Bases[2].faction_id_current=5;change_current=1;
        expect_neighbor(1,Bases[2].x,Bases[2].y);expect_neighbor(2,Bases[2].x,Bases[2].y);
        verify();require(BaseCurrent==&Bases[2]);
        require((FarmStatus[0]&16)==(shift==7?16:0));
    }
    reset();BaseCurrentCount=2;Bases[1]=Bases[0];Bases[0].resource_sqr_radius=1u<<1;change_selected=1;
    expect_neighbor(0,20,12);verify();require((FarmStatus[1]&16)!=0);
    reset();Bases[1]=Bases[0];change_count=1;expect_neighbor(1,20,12);verify();require(BaseCurrentCount==2);
}
int main(){
    require(sizeof(Base)==0x134 && sizeof(Map)==44 && sizeof(Veh)==52);
    require(offsetof(Base,resource_sqr_radius)==0x78 && offsetof(Base,nutrients_accumulated)==0x3C);
    require(offsetof(PlayerData,diplo_treaties)==0x18 && offsetof(Map,visibility)==4);
    require((std::is_same<decltype(Base::x),int16_t>::value));
    require((std::is_same<decltype(Base::y),int16_t>::value));
    require((std::is_same<decltype(Base::faction_id_current),uint8_t>::value));
    require(offsetof(Veh,next_veh_id_stack)==0x30 && offsetof(Veh,prev_veh_id_stack)==0x32);
    require((std::is_same<decltype(&add_goal),void (*)(int,int,int,int,int,int)>::value));
    geometry();flags();callback_reloads();neighbors();
}
'''


def run_host(tmp_path, declarations, helpers, body):
    compiler=shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host compiler required")
    source=tmp_path/"farm.cpp"
    source.write_text(HARNESS.replace("ACTUAL_DECLARATIONS",declarations)
                      .replace("ACTUAL_HELPERS",helpers).replace("ACTUAL_BODY",body))
    binary=tmp_path/"farm"
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=address,undefined",
                             "-fno-sanitize-recover=all",str(source),"-o",str(binary)],
                            capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,
                          env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_farm_compute(tmp_path):
    result=run_host(tmp_path,*actual_source())
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("before,after", [
    ("position >= 21", "position >= 20"),
    ("FarmStatus[position] = 1;", "FarmStatus[position] = 0;"),
    ("FarmStatus[position] = 2;", "FarmStatus[position] = 1;"),
    ("FarmStatus[position] = 0;", "FarmStatus[position] |= 0;"),
    ("xrange(center_x + RadiusOffsetX[position])", "center_x + RadiusOffsetX[position]"),
    ("if (!on_map(x, y))", "if (false)"),
    ("BaseUpkeepStage == 1", "BaseUpkeepStage != 0"),
    ("(PlayersData[faction].flags & 0x200)", "(PlayersData[faction].flags & 0x100)"),
    (".visibility\n                & (1u << faction)", ".val2\n                & (1u << faction)"),
    ("(tile->bit & 1)", "(tile->bit & 2)"),
    ("(tile->val2 & 0xF) < 8", "(tile->val2 & 0xF) <= 8"),
    ("stack_check(vehicle, 9, 3, -1, -1)", "stack_check(vehicle, 9, 2, -1, -1)"),
    ("int vehicle = stack_fix(veh_at(x, y));", "int vehicle = veh_at(x, y);"),
    ("while (Vehs[vehicle].prev_veh_id_stack >= 0)", "while (false)"),
    ("(Vehs[vehicle].visibility & (1u << faction)) != 0", "true"),
    ("PlayersData[faction].diplo_treaties[other_faction]", "PlayersData[other_faction].diplo_treaties[faction]"),
    ("diplo_treaties[other_faction] & 3", "diplo_treaties[other_faction] & 1"),
    ("const int territory = whose_territory", "vehicle = fixed_vehicle[active_position];\n        const int territory = whose_territory"),
    ("territory >= 0 && territory != faction", "territory >= 0"),
    ("FarmStatus[position] |= 0x20;", "FarmStatus[position] |= 0x10;"),
    ("other->resource_sqr_radius", "other->nutrients_accumulated"),
    ("worked != 0 &&", "true &&"),
    ("other->faction_id_current != faction", "other->faction_id_current == faction"),
    ("Base *const current = BaseCurrent;", "Base *const current = base;"),
    ("x_dist(center_x, x), abs(center_y - y)", "x_dist(BaseCurrent->x, x), abs(BaseCurrent->y - y)"),
    ("if (distance > 2)", "if (distance >= 2)"),
    ("other_id == BaseIDCurrentSelected", "other_id == 0"),
    ("uint32_t unavailable = 0;", "uint32_t unavailable = 1;"),
    ("if (FarmStatus[position] != 0)", "if ((FarmStatus[position] & 1) != 0)"),
    ("for (position = 0; position < 21; ++position) {\n        if (FarmStatus",
     "for (position = 0; position < 25; ++position) {\n        if (FarmStatus"),
    ("add_goal(faction, 2, 1, x, y, -1);", "add_goal(faction, 0, 1, x, y, -1);"),
    ("row_stride = static_cast<int>(MapLongitude);", "row_stride = 32;"),
])
def test_farm_compute_damage_is_observed(tmp_path,before,after):
    declarations,helpers,body=actual_source()
    assert before in body
    result=run_host(tmp_path,declarations,helpers,body.replace(before,after))
    assert result.returncode!=0
    assert result.returncode==97 or "runtime error:" in result.stderr or "AddressSanitizer" in result.stderr,result.stderr


@pytest.mark.parametrize("mutation", ["cache_map", "before_goody", "after_goals", "after_territory", "cache_count", "cache_selection"])
def test_farm_callback_reload_damage_is_observed(tmp_path,mutation):
    declarations,helpers,body=actual_source()
    reload="row_stride = static_cast<int>(MapLongitude);"
    if mutation=="cache_map":
        body=body.replace("map_tiles()","cached_map")
        body=body.replace("void __cdecl farm_compute() {","void __cdecl farm_compute() {\nMap *const cached_map=map_tiles();")
    elif mutation=="before_goody":
        before="const int goody = goody_at(x, y);\n            "+reload
        assert before in body
        body=body.replace(before,reload+"\n            const int goody = goody_at(x, y);")
    elif mutation=="after_goals":
        before="add_goal(faction, 0, 1, x, y, -1);\n                "+reload
        assert before in body
        body=body.replace(before,"add_goal(faction, 0, 1, x, y, -1);")
    elif mutation=="after_territory":
        before="        "+reload+"\n    }\n\n    for (int other_id"
        assert before in body
        body=body.replace(before,"    }\n\n    for (int other_id")
    else:
        name="BaseCurrentCount" if mutation=="cache_count" else "BaseIDCurrentSelected"
        body=body.replace(name,"cached_value")
        body=body.replace("void __cdecl farm_compute() {","void __cdecl farm_compute() {\nconst int cached_value="+name+";")
    result=run_host(tmp_path,declarations,helpers,body)
    assert result.returncode==97,result.stderr
