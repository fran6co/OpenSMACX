"""Actual Faction bodies: live treaty phases and named-array field accesses.

The append double models a controlled external-call effect to distinguish read
phases; it does not claim historical strcat invokes a callback. Layout and VC6
emission are qualified separately by native objects, not by these host models.
"""
from pathlib import Path
import shutil
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[2]


def definition(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == '{') - (source[end] == '}')
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError('unterminated actual body')


HARNESS = r'''
#include <cassert>
#include <cstring>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>
#define __cdecl
const uint32_t DTREATY_PACT=1,DTREATY_VENDETTA=16,DTREATY_HAVE_SURRENDERED=0x2000000;
struct Player { int is_leader_female,noun_gender,is_noun_plural;
 char title_leader[32],name_leader[32],noun_faction[32],adj_name_faction[32]; } Players[8];
struct PlayerData { uint32_t diplo_treaties[8];int traded_maps; } PlayersData[8];
int GenderDefault,PluralityDefault,DiploFriction,DiploScratchFA98;
int append_count,effect,mood_seen,special_seen,visible,draw_calls,update_calls;
std::string caption_seen;
char* Mood[9];
struct Strings { char* get(char* p) { return p; } } StringTable;
char special[]="special",mood_label[]="mood",hood_label[]="hood";
char* label_get(int i) { assert(i==0x1e8);++special_seen;return special; }
void say_fac_special(char* out,char* text,int who) {
 assert(who==2 && text==mood_label);++mood_seen;std::strcat(out,text);
}
char* append_text(char* out,const char* text) {
 char* result=std::strcat(out,text);
 if(++append_count==2) {
  if(effect==1) PlayersData[1].diplo_treaties[2]&=~DTREATY_PACT;
  if(effect==2) PlayersData[1].diplo_treaties[2]|=DTREATY_PACT;
 }
 return result;
}
struct Rect { int left,top,right,bottom; };
struct DiploPop { Rect rect1_;int is_visible(){return visible;}
 void draw_info(){++draw_calls;}void soft_update(Rect* p){assert(p==&rect1_);++update_calls;}
} DiplomacyPopup;
std::vector<int> parse_ids,genders,pluralities;
std::vector<std::string> parse_texts;
int break_mode,popup_calls,net_calls,popup_result;
char *ScriptPopupFile=(char*)"script";
char DiploLabelBREAKINGPACT[]="pact",DiploLabelBreakSmPcx[]="pactpic";
char DiploLabelBREAKINGTREATY[]="treaty",DiploLabelBreakSmPcx_7C0[]="treatypic";
char DiploLabelBREAKINGTRUCE[]="truce",DiploLabelBreakSmPcx_7E0[]="trucepic";
char DiploLabelBEGINVENDETTA[]="vendetta",DiploLabelBreakSmPcx_800[]="vendettapic";
int parse_says(int id,const char* text,int a,int b) {
 assert(a==-1 && b==-1);
 if(id==7){caption_seen=text;return 0;}
 parse_ids.push_back(id);parse_texts.push_back(text);
 genders.push_back(GenderDefault);pluralities.push_back(PluralityDefault);
 if(break_mode && id==0){Players[2].is_leader_female=22;std::strcpy(Players[2].name_leader,"fresh-name");}
 if(break_mode && id==1){Players[2].noun_gender=33;Players[2].is_noun_plural=44;std::strcpy(Players[2].noun_faction,"fresh-noun");}
 return 0;
}
char* get_pact_hood(int a,int b) {
 assert(a==1&&b==2);std::strcpy(Players[2].adj_name_faction,"fresh-adjective");
 if(break_mode==2)PlayersData[1].diplo_treaties[2]=1;
 return hood_label;
}
struct NetMsg {void pop(const char* key,int x,int y,const char* pic){
 assert(key==DiploLabelBREAKINGPACT&&x==-5000&&y==0&&pic==DiploLabelBreakSmPcx);++net_calls;}
} NetMsgPrimary;
int popp(const char* file,const char* key,int a,const char* pic,int(*)()) {
 assert(file==ScriptPopupFile&&a==0);++popup_calls;
 assert((key==DiploLabelBREAKINGTREATY&&pic==DiploLabelBreakSmPcx_7C0)||
        (key==DiploLabelBREAKINGTRUCE&&pic==DiploLabelBreakSmPcx_7E0)||
        (key==DiploLabelBEGINVENDETTA&&pic==DiploLabelBreakSmPcx_800));
 return popup_result;
}
struct Map {uint8_t visibility;uint32_t bit,bit_visible[8];} tiles[3];
Map* MapTiles=tiles;int MapArea,DiplomacyMapTransferChanged;
struct Base {uint8_t visibility;int faction_id_current;} Bases[3];
struct State {struct Fields {int base_current_count_;}fields_;}GameDataStateStorage;
struct Console {int faction_id_;}ConsoleGlobal;
unsigned UnkBitfield1;int redraws,map_draws;
struct GraphicWin {void redraw(){++redraws;}};
struct WorldWin:GraphicWin{}WorldClimateWorldWin;
std::vector<std::pair<int,int> > spots;
void spot_base(int base,int who){spots.push_back({base,who});if(base==0)Bases[1].faction_id_current=2;}
void draw_map(int arg){assert(arg==1);++map_draws;}
#define strcat append_text
ACTUAL
#undef strcat
void reset() {
 std::memset(Players,0,sizeof Players);std::memset(PlayersData,0,sizeof PlayersData);
 std::strcpy(Players[2].noun_faction,"Faction");std::strcpy(Players[2].title_leader,"title");
 std::strcpy(Players[2].name_leader,"name");std::strcpy(Players[2].adj_name_faction,"adjective");
 Players[2].is_leader_female=11;Players[2].noun_gender=12;Players[2].is_noun_plural=13;
 DiploFriction=DiploScratchFA98=append_count=effect=mood_seen=special_seen=0;
 visible=draw_calls=update_calls=break_mode=popup_calls=net_calls=popup_result=0;
 parse_ids.clear();parse_texts.clear();genders.clear();pluralities.clear();caption_seen.clear();
 for(int i=0;i<9;++i)Mood[i]=mood_label;
}
void captions() {
 reset();DiploScratchFA98=1;effect=1;PlayersData[1].diplo_treaties[2]=1;
 diplomacy_caption(1,2);
 assert(caption_seen=="Faction - special" && special_seen==1 && mood_seen==0 && "caption must reload treaty after append");
 reset();DiploScratchFA98=1;effect=2;diplomacy_caption(1,2);
 assert(caption_seen=="Faction - mood" && mood_seen==1 && special_seen==0 && "post-append pact suppresses special");
 const int frictions[]={0,1,3,5,9,13,16,18,20};
 for(int f:frictions){reset();DiploFriction=f;visible=1;diplomacy_caption(1,2);
 assert(caption_seen=="Faction - mood"&&draw_calls==1&&update_calls==1);}
 reset();DiploFriction=1;PlayersData[1].diplo_treaties[2]=16;diplomacy_caption(1,2);assert(DiploFriction==15);
 const uint32_t reverse[]={1,0x2000000,0x2000001};
 for(uint32_t flags:reverse){reset();DiploFriction=1;PlayersData[2].diplo_treaties[1]=flags;diplomacy_caption(1,2);
 assert(special_seen==(flags==0x2000001));}
}
void breaks() {
 reset();break_mode=2;PlayersData[1].diplo_treaties[2]=2;
 assert(break_treaty(1,2,2)==1 && net_calls==1 && popup_calls==0 && "break must reload post-parse treaty");
 assert((parse_ids==std::vector<int>{0,1,2,3,4}));
 assert(parse_texts[0]=="title"&&parse_texts[1]=="fresh-name"&&parse_texts[2]=="fresh-noun"&&parse_texts[4]=="fresh-adjective");
 assert(genders[0]==11&&genders[1]==22&&genders[2]==33&&pluralities[0]==0&&pluralities[1]==0&&pluralities[2]==44);
 for(int result:{0,5})for(uint32_t flags:{2u,4u,8u}){
 reset();PlayersData[1].diplo_treaties[2]=flags;popup_result=result;
 assert(break_treaty(1,2,flags)==(result==0)&&popup_calls==1);}
 reset();PlayersData[1].diplo_treaties[2]=16;assert(break_treaty(1,2,16)==0&&parse_ids.empty());
 reset();PlayersData[1].diplo_treaties[2]=0x10004;assert(break_treaty(1,2,0x10004)==0&&popup_calls==0);
}
void maps() {
 reset();MapArea=0;GameDataStateStorage.fields_.base_current_count_=0;ConsoleGlobal.faction_id_=7;
 PlayersData[1].traded_maps=0x20;PlayersData[2].traded_maps=0x80;trade_maps(1,2);
 assert(PlayersData[1].traded_maps==0xa4&&PlayersData[2].traded_maps==0xa6&&"ordered map-mask stores");
 PlayersData[1].traded_maps=0x20;trade_maps(1,1);assert(PlayersData[1].traded_maps==0x22);
 std::memset(tiles,0,sizeof tiles);std::memset(Bases,0,sizeof Bases);spots.clear();
 MapArea=3;GameDataStateStorage.fields_.base_current_count_=3;ConsoleGlobal.faction_id_=1;
 tiles[0].visibility=2;tiles[0].bit=0xf;tiles[0].bit_visible[0]=0x1a;
 tiles[1].visibility=4;tiles[1].bit=0xf;tiles[1].bit_visible[1]=0x2b;
 tiles[2].visibility=0;Bases[0].faction_id_current=1;
 redraws=map_draws=0;UnkBitfield1=0x10;trade_maps(1,2);
 assert(tiles[0].visibility==6&&tiles[0].bit_visible[1]==0xa);
 assert(tiles[1].visibility==6&&tiles[1].bit_visible[0]==0xb&&tiles[2].visibility==0&&DiplomacyMapTransferChanged==1);
 assert((spots==std::vector<std::pair<int,int>>{{0,2},{1,1}}));
 assert(UnkBitfield1==0x14&&redraws==1&&map_draws==1);
}
int main(){captions();breaks();maps();}
'''


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', ['none', 'cached_caption_treaty', 'stale_break_flags', 'reverse_map_store'])
def test_actual_shared_structure_and_live_phases(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + ' unavailable')
    source = (ROOT / 'src/faction.cpp').read_text()
    bodies = '\n'.join(definition(source, signature) for signature in (
        'void __cdecl diplomacy_caption(', 'int __cdecl break_treaty(', 'void __cdecl trade_maps('))
    diagnostic = ''
    if damage == 'cached_caption_treaty':
        start = 'void __cdecl diplomacy_caption(int faction_id, int faction_id_with) {'
        assert start in bodies
        bodies = bodies.replace(start, start + '\n uint32_t cached_treaty=PlayersData[faction_id].diplo_treaties[faction_id_with];', 1)
        operand = 'PlayersData[faction_id].diplo_treaties[faction_id_with]'
        # Leave the snapshot initializer intact, then substitute both actual reads.
        prefix, tail = bodies.split('uint32_t cached_treaty=', 1)
        initializer, tail = tail.split(';', 1)
        bodies = prefix + 'uint32_t cached_treaty=' + initializer + ';' + tail.replace(operand, 'cached_treaty')
        diagnostic = 'caption must reload treaty after append'
    elif damage == 'stale_break_flags':
        before = 'flags = PlayersData[faction_id].diplo_treaties[target_faction_id];'
        assert bodies.count(before) == 2
        at = bodies.rfind(before)
        bodies = bodies[:at] + 'flags = flags;' + bodies[at + len(before):]
        diagnostic = 'break must reload post-parse treaty'
    elif damage == 'reverse_map_store':
        before = 'PlayersData[faction_id].traded_maps = static_cast<int>(combined);'
        assert before in bodies
        bodies = bodies.replace(before, 'PlayersData[faction_id].traded_maps = static_cast<int>(combined) ^ 0x40;', 1)
        diagnostic = 'ordered map-mask stores'
    cpp, binary = tmp_path / 'actual.cpp', tmp_path / 'actual'
    cpp.write_text(HARNESS.replace('ACTUAL', bodies))
    compiled = subprocess.run([executable, '-std=c++11', '-O2', '-Wall', '-Wextra', str(cpp), '-o', str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    if damage == 'none':
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0
        assert diagnostic in result.stderr, result.stderr


# This projection uses the actual Goal declaration and actual add_site body.
# Its add_goal seam records the already-published fields and ordinary arguments;
# it does not claim host compilation qualifies original offsets or VC6 emission.
SITE_HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <climits>
#define __cdecl
ACTUAL_GOAL
static_assert(sizeof(Goal)==16,"actual Goal record");
const int MaxSitesNum=25,MaxGoalsNum=75,STATE_DEBUG_MODE=0x1000;
struct PlayerData {Goal goals[MaxGoalsNum],sites[MaxSitesNum];}PlayersData[8];
struct State {struct Fields {int game_state_;}fields_;}GameDataStateStorage;
int calls,dangers,expected[6],chosen;
void danger(const char*,const char*,int,int,int){++dangers;}
void add_goal(int f,int type,int priority,int x,int y,int base){
 const Goal &site=PlayersData[f].sites[chosen];
 assert(type==expected[1] && "stored signed type passed to add_goal");
 assert(priority==expected[2] && "stored signed priority passed to add_goal");
 assert(f==expected[0]&&x==expected[3]&&y==expected[4]&&base==expected[5]);
 assert(site.type==type&&site.priority==priority&&site.x==x&&site.y==y&&"complete site published before call");
 ++calls;
}
ACTUAL_SITE
void reset(){
 std::memset(PlayersData,0,sizeof PlayersData);
 for(int f=0;f<8;++f)for(int i=0;i<25;++i){
  Goal &s=PlayersData[f].sites[i];s.type=-1;s.priority=0;s.x=-200-i;s.y=-100-i;s.base_id=1000+i;
 }
 calls=dangers=0;GameDataStateStorage.fields_.game_state_=0;
 expected[0]=2;expected[3]=10;expected[4]=8;expected[5]=-1;chosen=0;
}
int main(){
 const int inputs[][4]={{0x10001,0x10002,1,2},{0x8000,0xffff,-32768,-1},
  {-65537,-32769,-1,32767},{INT_MIN,INT_MAX,0,-1}};
 for(const auto &v:inputs){reset();expected[1]=v[2];expected[2]=v[3];
  add_site(2,v[0],v[1],10,8);assert(calls==1&&dangers==0&&"signed unused site insertion");
  assert(PlayersData[2].sites[0].base_id==1000);
  for(int i=1;i<25;++i)assert(PlayersData[2].sites[i].type==-1&&PlayersData[2].sites[i].x==-200-i);
 }
 // An unused signed type must outrank an ordinary priority-zero record.
 reset();PlayersData[2].sites[0].type=7;chosen=1;expected[1]=3;expected[2]=1;
 add_site(2,3,1,10,8);assert(calls==1&&PlayersData[2].sites[0].type==7&&"signed unused type wins selection");
 // A duplicate never calls add_goal, and the selected maximum narrows once.
 reset();Goal &same=PlayersData[2].sites[4];same.type=3;same.priority=32000;same.x=10;same.y=8;
 add_site(2,3,0x10001,10,8);assert(calls==0&&same.priority==1);
 same.priority=32000;add_site(2,3,-20,10,8);assert(calls==0&&same.priority==32000);
 // Existing negative priority may outrank the unused score of 1000.
 reset();Goal &low=PlayersData[2].sites[2];low.type=4;low.priority=-32768;
 chosen=2;expected[1]=5;expected[2]=1;add_site(2,5,1,10,8);assert(calls==1);
 // Native TEST AH,0x10 is bit0x1000; bit0x10 must not enable debug.
 reset();GameDataStateStorage.fields_.game_state_=0x10;expected[1]=3;expected[2]=1;expected[3]=9;
 add_site(2,3,1,9,8);assert(calls==1&&dangers==0);
 // Preserve the debug-only bad parity callback without rejecting the site.
 reset();GameDataStateStorage.fields_.game_state_=0x1000;expected[1]=3;expected[2]=1;expected[3]=9;
 add_site(2,3,1,9,8);assert(calls==1&&dangers==1);
}
"""


@pytest.mark.parametrize('compiler', ['g++', 'clang++'])
@pytest.mark.parametrize('damage', ['none', 'input_type', 'input_priority', 'unsigned_type', 'swap_coordinates'])
def test_actual_site_signed_fields_and_goal_arguments(tmp_path, compiler, damage):
    executable = shutil.which(compiler)
    if not executable:
        pytest.skip(compiler + ' unavailable')
    body = definition((ROOT / 'src/faction.cpp').read_text(), 'void __cdecl add_site(')
    # VC6's original for-scope leaves i live for the second loop. Transport
    # only that declaration for standard host C++; all operations stay actual.
    vc6_for = 'for (int i = 0; i < MaxSitesNum; i++) {'
    assert body.count(vc6_for) == 1
    body = body.replace(vc6_for, 'int i; for (i = 0; i < MaxSitesNum; i++) {')
    original = body
    diagnostic = ''
    if damage in ('input_type', 'input_priority'):
        before = 'add_goal(faction_id, sites.type, sites.priority, sites.x, sites.y, -1);'
        assert body.count(before) == 1
        if damage == 'input_type':
            after = 'add_goal(faction_id, type, sites.priority, sites.x, sites.y, -1);'
            diagnostic = 'stored signed type passed to add_goal'
        else:
            after = 'add_goal(faction_id, sites.type, priority, sites.x, sites.y, -1);'
            diagnostic = 'stored signed priority passed to add_goal'
        body = body.replace(before, after)
    elif damage == 'unsigned_type':
        before = 'const int16_t type_cmp = sites.type;'
        assert body.count(before) == 1
        body = body.replace(before, 'const uint16_t type_cmp = sites.type;')
        diagnostic = 'signed unused site insertion'
    elif damage == 'swap_coordinates':
        before = 'sites.x = x;\n        sites.y = y;'
        assert body.count(before) == 1
        body = body.replace(before, 'sites.x = y;\n        sites.y = x;')
        diagnostic = 'x==expected[3]'
    if damage != 'none':
        assert body != original
    goal = definition((ROOT / 'src/faction.h').read_text(), 'struct Goal {') + ';'
    cpp, binary = tmp_path / 'actual.cpp', tmp_path / 'actual'
    cpp.write_text(SITE_HARNESS.replace('ACTUAL_GOAL', goal).replace('ACTUAL_SITE', body))
    compiled = subprocess.run([executable, '-std=c++11', '-O2', '-Wall', '-Wextra', str(cpp), '-o', str(binary)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    if damage == 'none':
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0
        assert diagnostic in result.stderr, result.stderr
