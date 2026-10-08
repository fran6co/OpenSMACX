"""Actual psych row/panel bodies with original-derived bounded observations.

Real Base/faction field declarations, stage storage and Sprite read accessors
are extracted unchanged. Random, font, labels and graphics services are explicit
recording doubles; their native ABI, pixels, asset lifetime and game integration
are not certified. In particular the original borrowed font is not restored:
the test records its numeric identity without dereferencing it after destruction.
The Font double represents successful initialization only; failed OS font
creation, exception unwinding and subsequent use of the borrowed font remain
outside these controls. Valid specialist types and finite game populations only.
"""

from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest


ROOT=Path(__file__).resolve().parents[2]


def braced(source,signature):
    start=source.index(signature);opening=source.index("{",start);depth=0
    for end in range(opening,len(source)):
        depth+=(source[end]=="{")-(source[end]=="}")
        if not depth:return source[start:end+1]
    raise AssertionError("unterminated actual definition")


def actual_source():
    cpp=(ROOT/"src/basewin.cpp").read_text()
    base=(ROOT/"src/base.h").read_text();faction=(ROOT/"src/faction.h").read_text()
    sprite=(ROOT/"src/sprite.h").read_text();storage=(ROOT/"src/sprite.cpp").read_text()
    declarations=[]
    for source,names in ((base,("Base",)),(faction,("SocialCategory","SocialEffect","Goal","Player","PlayerData"))):
        for name in names:declarations.append(braced(source,"struct "+name+" {")+";")
    for name in ("BasePsychTalents","BasePsychDrones","BasePsychSuperDrones"):
        definition,=re.findall(r"^int "+name+r"\[5\];",(ROOT/"src/base.cpp").read_text(),re.M)
        declarations.append(definition)
    for name in ("g_CITIZEN_SM_CURSOR_SPRITES","g_SPECIALIST_SM_CURSOR_SPRITES","g_AL_CITIZEN_SM_CURSOR_SPRITES","g_AL_SPECIALIST_SM_CURSOR_SPRITES"):
        definition,=re.findall(r"^Sprite "+name+r"\[\d+\];",storage,re.M)
        declarations.append(definition)
    for header,name in (("game.h","ExpansionEnabled"),("mapwin.h","MapWinCaptionScale"),("mapwin.h","MapWinCaptionDivisor")):
        definition,=re.findall(r"^extern (?:int|uint32_t|BOOL) "+name+r";",(ROOT/"src"/header).read_text(),re.M)
        declarations.append(definition)
    accessors="\n".join(braced(sprite,signature) for signature in
                         ("unsigned char transparent_index() const","int width() const","int height() const"))
    fields="\n".join(re.search(r"^  (?:unsigned char|int) "+name+r";",sprite,re.M).group() for name in
                      ("cTransparentIndex_","iWidth_","iHeight_"))
    draw,=re.findall(r"  int draw\(Buffer \*destination, int transparent, int x, int y\);",sprite)
    helpers="\n".join(braced(cpp,"int psych_display_"+name+"(") for name in ("signed","add","sub"))
    bodies="\n".join(braced(cpp,"void BaseWin::"+name+"(") for name in ("psych_row","draw_psych"))
    return "\n".join(declarations),fields+"\n"+accessors+"\n"+draw,helpers,bodies


HARNESS=r'''
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl
typedef int BOOL;typedef char *LPSTR;
void check(bool ok,int line){if(!ok){std::fprintf(stderr,"assertion line %d\n",line);std::exit(92);}}
#define require(x) check((x),__LINE__)
struct RECT {int32_t left,top,right,bottom;};
bool same(const RECT&a,const RECT&b){return a.left==b.left && a.top==b.top && a.right==b.right && a.bottom==b.bottom;}
struct Font;struct Buffer;
struct Sprite {ACTUAL_SPRITE_READS};
ACTUAL_DECLARATIONS
Base Bases[8],*BaseCurrent;Player Players[8];PlayerData PlayersData[8];
int BaseIDCurrentSelected,MapWinCaptionScale,MapWinCaptionDivisor,TextColourTable[195];
BOOL ExpansionEnabled;
const char *DefaultFontFace="fixture-face";
char StringTemp[1032];
enum {FAC_BROOD_PIT=35};
std::vector<std::string> events;
struct Draw {Sprite *sprite;Buffer *buffer;int transparent,x,y;};std::vector<Draw> draws;
struct Text {int label,x,y,width,len;};std::vector<Text> texts;
int random_calls,random_births,random_deaths,best_calls,label_current,clip_calls;
int change_population_after_draw,change_specialists_after_draw,change_selected,change_base_on_font,change_width_on_text,change_population_on_time;
uintptr_t installed_font,dead_font;
bool panel_active;
RECT initial_clip;
struct Font {
    int height_;
    Font():height_(17){events.push_back("font-create");}
    int pixel_height() const{return height_;}
    int init(const char *face,int height,int flags){require(face==DefaultFontFace && height==12 && flags==1);events.push_back("font-init");height_=11;return 0;}
    ~Font();
};
struct Buffer {
    RECT rect1_;
    int set_clip(RECT *r){events.push_back("clip");rect1_=*r;++clip_calls;return 0;}
    int set_font(Font *font,Font *b,Font *c,Font *d){
        require(!b&&!c&&!d && MapWinCaptionScale==30 && MapWinCaptionDivisor==38);
        events.push_back("font-set");installed_font=reinterpret_cast<uintptr_t>(font);
        if(change_base_on_font>=0){BaseCurrent=&Bases[change_base_on_font];change_base_on_font=-1;}
        return 0;
    }
    void set_text_color(int a,int b,int c,int d){require(a==7114 && b==7115 && c==1 && d==1);events.push_back("text-color");}
    int write_cent_l(char *text,int x,int y,int width,int len){
        require(std::string(text)=="label-"+std::to_string(label_current) && len==int(std::strlen(text)));
        require(MapWinCaptionScale==30 && MapWinCaptionDivisor==38);events.push_back("text");
        texts.push_back({label_current,x,y,width,len});
        if(change_width_on_text>=0){g_CITIZEN_SM_CURSOR_SPRITES[0].iWidth_=change_width_on_text;change_width_on_text=-1;}
        return 17;
    }
};
struct GraphicWin:Buffer {void fill(int color){require(color==0);events.push_back("fill");}};
struct BaseWin:GraphicWin {
    RECT rect1_;
    void psych_row(int,int,int,int,int,int,int);void draw_psych();
};
BaseWin window;
Font::~Font(){
    require(MapWinCaptionScale==-1 && MapWinCaptionDivisor==-1 && same(window.Buffer::rect1_,initial_clip));
    events.push_back("font-destroy");dead_font=reinterpret_cast<uintptr_t>(this);
}
struct Random {
    Random(){events.push_back("random-create");++random_births;}
    ~Random(){events.push_back("random-destroy");++random_deaths;}
    void reseed(uint32_t value){require(value==0x98765432u);events.push_back("random-seed");}
    uint32_t get(int low,int high){require(low==0 && high==2);events.push_back("random-get");return random_calls++%2;}
};
unsigned long timeGetTime(){events.push_back("time");if(change_population_on_time>=0){BaseCurrent->population_size=change_population_on_time;ExpansionEnabled=1;Players[3].rule_flags=0x80;change_population_on_time=-1;}return 0x98765432u;}
int best_specialist(){++best_calls;events.push_back("best");return 6;}
char *label_get(int label){
    static char text[32];label_current=label;events.push_back("label");
    std::snprintf(text,sizeof(text),"label-%d",label);return text;
}
void bitmask(int id,int *offset,int *mask){
    require(id==35);events.push_back("bitmask");*offset=4;*mask=8;
    if(change_selected>=0){BaseIDCurrentSelected=change_selected;change_selected=-1;}
}
int Sprite::draw(Buffer *buffer,int transparent,int x,int y){
    require(buffer==static_cast<Buffer*>(&window));require(transparent==cTransparentIndex_);
    draws.push_back({this,buffer,transparent,x,y});events.push_back("draw");
    if(change_population_after_draw>=0){BaseCurrent->population_size=change_population_after_draw;change_population_after_draw=-1;}
    if(change_specialists_after_draw>=0){BaseCurrent->specialist_total=change_specialists_after_draw;change_specialists_after_draw=-1;}
    return 29;
}
ACTUAL_HELPERS
ACTUAL_BODIES
void reset(){
    std::memset(Bases,0,sizeof(Bases));std::memset(Players,0,sizeof(Players));std::memset(PlayersData,0,sizeof(PlayersData));
    BaseCurrent=&Bases[0];BaseCurrent->faction_id_current=3;BaseCurrent->faction_id_former=3;BaseCurrent->population_size=8;
    BaseIDCurrentSelected=0;ExpansionEnabled=0;MapWinCaptionScale=101;MapWinCaptionDivisor=103;
    events.clear();draws.clear();texts.clear();random_calls=random_births=random_deaths=best_calls=clip_calls=0;
    change_population_after_draw=change_specialists_after_draw=change_selected=change_base_on_font=change_width_on_text=change_population_on_time=-1;installed_font=dead_font=0;
    initial_clip={-9,-7,401,503};window.Buffer::rect1_=initial_clip;window.rect1_={10,20,210,400};
    for(int i=0;i<195;++i)TextColourTable[i]=7000+i;
    for(int i=0;i<5;++i)BasePsychTalents[i]=BasePsychDrones[i]=BasePsychSuperDrones[i]=0;
    Sprite *arrays[]={g_CITIZEN_SM_CURSOR_SPRITES,g_SPECIALIST_SM_CURSOR_SPRITES,g_AL_CITIZEN_SM_CURSOR_SPRITES,g_AL_SPECIALIST_SM_CURSOR_SPRITES};
    int sizes[]={8,7,4,7};for(int a=0;a<4;++a)for(int i=0;i<sizes[a];++i){arrays[a][i].cTransparentIndex_=128+20*a+i;arrays[a][i].iWidth_=30;arrays[a][i].iHeight_=38;}
}
void expect_draw(int index,Sprite *sprite,int x,int y){require(index<int(draws.size()));const Draw &d=draws[index];require(d.sprite==sprite && d.x==x && d.y==y);}
void row_controls(){
    for(int alien=0;alien<2;++alien){reset();ExpansionEnabled=alien;Players[3].rule_flags=0x80;
        BaseCurrent->specialist_total=2;BaseCurrent->spec[0]=0x52;
        window.psych_row(11,23,101,31,1,3,1);require(draws.size()==8);
        int moods[]={0,1,1,2,2,3};Sprite *citizens=alien?g_AL_CITIZEN_SM_CURSOR_SPRITES:g_CITIZEN_SM_CURSOR_SPRITES;
        for(int i=0;i<6;++i)expect_draw(i,&citizens[moods[i]+(alien?0:4*(i%2))],11+10*i,23);
        Sprite *specialists=alien?g_AL_SPECIALIST_SM_CURSOR_SPRITES:g_SPECIALIST_SM_CURSOR_SPRITES;
        expect_draw(6,&specialists[2],71,23);expect_draw(7,&specialists[5],81,23);
        require(random_calls==(alien?0:6) && random_births==1 && random_deaths==1 && best_calls==0);
        require(events[0]=="random-create" && events[1]=="time" && events[2]=="random-seed" && events.back()=="random-destroy");
    }
    const int widths[]={1000,1,-20,100};const int sprite_widths[]={31,31,0,-3};const int steps[]={31,1,1,1};
    for(int n=0;n<4;++n){reset();BaseCurrent->population_size=3;window.psych_row(7,9,widths[n],sprite_widths[n],0,0,0);
        require(draws.size()==3);for(int i=0;i<3;++i)require(draws[i].x==7+i*steps[n]);}
    reset();BaseCurrent->population_size=1;window.psych_row(7,9,1,-3,2,0,0);require(draws.size()==2);require(draws[1].x==4);
    reset();ExpansionEnabled=1;Players[3].rule_flags=0;BaseCurrent->population_size=1;window.psych_row(7,9,31,31,0,0,0);
    require(draws.size()==1 && draws[0].sprite==&g_CITIZEN_SM_CURSOR_SPRITES[1]);
    reset();BaseCurrent->population_size=18;BaseCurrent->specialist_total=18;
    BaseCurrent->spec[0]=0x01234560;BaseCurrent->spec[1]=0x65432106;
    window.psych_row(0,0,600,31,0,0,0);require(draws.size()==18 && best_calls==2 && random_calls==0);
    const int types[18]={0,6,5,4,3,2,1,0,6,0,1,2,3,4,5,6,6,6};
    for(int i=0;i<18;++i)expect_draw(i,&g_SPECIALIST_SM_CURSOR_SPRITES[types[i]],31*i,0);
    reset();BaseCurrent->population_size=4;change_population_after_draw=2;
    window.psych_row(0,0,124,31,0,0,0);require(draws.size()==2);
    reset();BaseCurrent->population_size=4;change_population_on_time=2;
    window.psych_row(0,0,61,31,0,0,0);require(draws.size()==2 && random_calls==0);
    expect_draw(1,&g_AL_CITIZEN_SM_CURSOR_SPRITES[1],10,0);
    reset();BaseCurrent->population_size=0;ExpansionEnabled=1;Players[3].rule_flags=0x80;
    window.psych_row(0,0,61,31,0,0,0);require(draws.empty() && random_births==1 && random_deaths==1 && events[1]=="time");
    reset();BaseCurrent->population_size=2;window.psych_row(2147483640,0,100,31,2,0,0);
    require(draws.size()==2 && draws[1].x==-2147483625);
    reset();BaseCurrent->population_size=4;BaseCurrent->specialist_total=4;BaseCurrent->spec[0]=0x5432;
    change_specialists_after_draw=1;window.psych_row(0,0,124,31,0,0,0);
    require(draws.size()==1 && draws[0].sprite==&g_SPECIALIST_SM_CURSOR_SPRITES[2]);
}
void panel_controls(){
    reset();BaseCurrent->population_size=6;
    const int t[5]={0,1,2,2,3},d[5]={3,3,2,2,1},s[5]={0,0,0,1,0};
    for(int i=0;i<5;++i){BasePsychTalents[i]=t[i];BasePsychDrones[i]=d[i];BasePsychSuperDrones[i]=s[i];}
    PlayersData[3].soc_effect_pending.values_[SocialEffect::POLICE]=-2;window.draw_psych();
    require(texts.size()==5 && draws.size()==30 && clip_calls==2);
    const int labels[]={322,323,324,326,327};
    const int moods[5][6]={{1,1,1,2,2,2},{0,1,1,2,2,2},{0,0,1,1,2,2},{0,0,1,1,2,3},{0,0,0,1,1,2}};
    for(int row=0;row<5;++row){require(texts[row].label==labels[row] && texts[row].x==112 && texts[row].y==34+39*row && texts[row].width==96);
        for(int i=0;i<6;++i)expect_draw(row*6+i,&g_CITIZEN_SM_CURSOR_SPRITES[moods[row][i]+4*(i%2)],12+13*i,22+39*row);}
    require(events[0]=="font-create" && events[1]=="clip" && events[2]=="fill" && events[3]=="font-init" && events[4]=="font-set");
    require(events[events.size()-2]=="clip" && events.back()=="font-destroy" && installed_font==dead_font && dead_font!=0);
    require(MapWinCaptionScale==-1 && MapWinCaptionDivisor==-1 && same(window.Buffer::rect1_,initial_clip));
    reset();BaseCurrent->population_size=2;BasePsychTalents[2]=BasePsychTalents[3]=1;BasePsychTalents[4]=2;
    window.draw_psych();require(texts.size()==3 && texts[0].label==322 && texts[1].label==324 && texts[2].label==327);
    require(texts[1].y==73 && texts[2].y==112); // skipped states do not consume a visible row.
    reset();BaseCurrent->population_size=1;window.draw_psych();require(texts.size()==1 && random_births==1);
    for(int kind=0;kind<3;++kind){reset();BaseCurrent->population_size=1;BaseCurrent->assimilation_turns_left=kind?4:0;
        BaseCurrent->faction_id_former=kind==1?7:3;BaseCurrent->nerve_staple_turns_left=1;
        window.draw_psych();require(texts[0].label==(kind==1?970:971));}
    reset();BaseCurrent->population_size=1;BasePsychTalents[3]=BasePsychTalents[4]=1;
    PlayersData[3].soc_effect_pending.values_[SocialEffect::POLICE]=-3;Bases[0].facilities_built[4]=8;change_selected=1;
    window.draw_psych();require(texts.size()==2 && texts[1].label==325);
    reset();Bases[1]=Bases[0];Bases[1].faction_id_current=7;Bases[1].faction_id_former=3;Bases[1].assimilation_turns_left=5;
    BasePsychTalents[3]=BasePsychTalents[4]=1;PlayersData[3].soc_effect_pending.values_[SocialEffect::POLICE]=-3;PlayersData[7].soc_effect_pending.values_[SocialEffect::POLICE]=3;
    change_base_on_font=1;window.draw_psych();require(texts[0].label==970 && texts[1].label==326);
    reset();BaseCurrent->population_size=6;change_width_on_text=10;window.draw_psych();
    require(draws.size()==6 && draws[1].x==23); // width is read after the caption callback.
}
int main(){row_controls();panel_controls();}
'''


def run_host(tmp_path,declarations,sprite,helpers,bodies):
    compiler=shutil.which("clang++") or shutil.which("g++")
    if not compiler:pytest.skip("host compiler required")
    source=tmp_path/"display.cpp";binary=tmp_path/"display"
    source.write_text(HARNESS.replace("ACTUAL_DECLARATIONS",declarations).replace("ACTUAL_SPRITE_READS",sprite).replace("ACTUAL_HELPERS",helpers).replace("ACTUAL_BODIES",bodies))
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=address,undefined","-fno-sanitize-recover=all",str(source),"-o",str(binary)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(binary)],capture_output=True,text=True,timeout=10,env=dict(os.environ,ASAN_OPTIONS="detect_leaks=0"))


def test_actual_psych_display(tmp_path):
    result=run_host(tmp_path,*actual_source());assert result.returncode==0,result.stderr


MUTANTS=[
    ("population-spacing","bodies","(population - 1)","population"),
    ("spacing-minimum","bodies","? 1\n            :", "? 0\n            :"),
    ("spacing-maximum","bodies","available > sprite_width ? sprite_width : available","available > sprite_width ? available : available"),
    ("alien-requires-expansion","bodies","ExpansionEnabled != 0","true"),
    ("alien-flag","bodies","rule_flags & 0x80","rule_flags & 0x40"),
    ("human-variant-stride","bodies","variant * 4","variant * 2"),
    ("superdrone-threshold","bodies","drone >= ordinary_drones","drone > ordinary_drones"),
    ("specialist-second-word","bodies","spec[specialist >> 3]","spec[0]"),
    ("specialist-fallback","bodies","specialist < 16","specialist < 15"),
    ("specialist-live-bound","bodies","for (int specialist = 0; specialist < BaseCurrent->specialist_total; ++specialist)",
        "const int cached_specialists = BaseCurrent->specialist_total; for (int specialist = 0; specialist < cached_specialists; ++specialist)"),
    ("specialist-nibble","bodies","((specialist & 7) * 4)","((specialist & 7) * 3)"),
    ("content-live-population","bodies","psych_display_sub(BaseCurrent->population_size, BaseCurrent->specialist_total)","psych_display_sub(population, BaseCurrent->specialist_total)"),
    ("half-panel-width","bodies","const int half_width = psych_display_sub(panel_width / 2, 4);","const int half_width = y;"),
    ("row-stage-talents","bodies","BasePsychTalents[stage], BasePsychDrones[stage]","BasePsychTalents[0], BasePsychDrones[stage]"),
    ("row-stage-superdrones","bodies","BasePsychSuperDrones[stage]);","0);"),
    ("duplicate-super-stage","bodies","&& BasePsychSuperDrones[stage] == BasePsychSuperDrones[stage - 1]",""),
    ("duplicate-stage-skip","bodies","&& BasePsychTalents[stage] == BasePsychTalents[stage - 1]","&& false"),
    ("caption-width","bodies","text_y, half_width,","text_y, y,"),
    ("caption-scale-order","bodies","MapWinCaptionScale = 30;","MapWinCaptionScale = 31;"),
    ("caption-reset","bodies","MapWinCaptionDivisor = -1;","MapWinCaptionDivisor = 0;"),
    ("restore-original-clip","bodies","canvas->set_clip(&saved_clip);","canvas->set_clip(&rect1_);"),
    ("former-faction-caption","bodies","base->faction_id_former != base->faction_id_current","base->faction_id_former == base->faction_id_current"),
    ("police-cached-faction","bodies","PlayersData[faction].soc_effect_pending.values_[SocialEffect::POLICE]","PlayersData[BaseCurrent->faction_id_current].soc_effect_pending.values_[SocialEffect::POLICE]"),
    ("selected-before-bitmask","bodies","Bases[selected].facilities_built","Bases[BaseIDCurrentSelected].facilities_built"),
    ("brood-police-caption","bodies","brood_pit ? 2 : 0","brood_pit ? 0 : 0"),
    ("sprite-width-accessor","sprite","return iWidth_;","return iHeight_;"),
    ("sprite-height-accessor","sprite","return iHeight_;","return iWidth_;"),
    ("palette-key-unsigned","sprite","unsigned char transparent_index()","signed char transparent_index()"),
    ("coordinate-wrap","helpers","return psych_display_signed(static_cast<uint32_t>(left) + static_cast<uint32_t>(right));","return left + right;"),
]


@pytest.mark.parametrize("name,part,before,after",MUTANTS,ids=[x[0] for x in MUTANTS])
def test_compiling_display_damage(tmp_path,name,part,before,after):
    extracted=list(actual_source());index={"sprite":1,"helpers":2,"bodies":3}[part]
    assert before in extracted[index],(name,"missing mutation anchor")
    extracted[index]=extracted[index].replace(before,after)
    result=run_host(tmp_path,*extracted)
    assert result.returncode!=0,(name,"behavior damage escaped controls")
