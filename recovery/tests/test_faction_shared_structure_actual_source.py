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
