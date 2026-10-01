/* Standalone host declarations/scenarios. Pytest replaces the extraction marker
 * with four helpers and Console::give_hints copied from src/console.cpp. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum { PLAN_RECONNAISANCE=3, PLAN_COLONIZATION=8, PLAN_TERRAFORMING=9,
       PLAN_DEFENSIVE=2, PLAN_ALIEN_ARTIFACT=12 };
enum { TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2 };
struct Sprite { int tag; };
struct Map { uint32_t bit, val2, val3, visibility; uint8_t climate; int region; };
struct Veh { uint32_t state; int x,y,proto_id,home_base_id,dmg_incurred,faction_id; };
struct VehPrototype { int plan,chassis_id,weapon_id,armor_id; const char *veh_name; };
struct ChassisRule { int triad,speed; };
struct WeaponRule { int offense_rating; };
struct ArmorRule { int defense_rating; };
struct Base { int x,y,faction_id_current,population_size; const char *name_string; };
struct PlayerData { uint32_t flags; int total_mil_veh,proto_id_active[128],unk_82[256]; };
struct Player { uint32_t rule_flags; int is_leader_female,noun_gender,is_noun_plural; const char *title_leader,*name_leader,*noun_faction; };
struct MapWin { int field_1DDA4_,field_1DDA8_,field_1DDCC_,field_1DDD0_,field_1DDD4_,field_1DDD8_; };
struct WindowStub { int is_visible() const { return 0; } };
struct TutorialStub {
    int count,flag,x,y,game_state_at_map,tour_stage; const char *text; Sprite *sprite;
    int tutorial_blocked() const { return 0; }
    void set_tour_stage(int s) { tour_stage=s; }
    int tut_map(const char *,int,int,int,Sprite *,int,int,int);
};
struct ConsoleGlobalStub { int faction_id_; void focus(int,int,int); };
class Console { public: int selected_vehicle_,faction_id_; void give_hints(); };

Veh Vehs[64]; VehPrototype VehPrototypes[128]; ChassisRule Chassis[64];
WeaponRule Weapon[64]; ArmorRule Armor[64]; Base Bases[64]; int BaseCurrentCount;
Base *BaseCurrent; PlayerData PlayersData[16]; Player Players[16];
MapWin HostMapWin; MapWin *MapWinTable[1]; WindowStub TutWinBaseWindow,TutWinSocWindow;
TutorialStub TutorialWindow; ConsoleGlobalStub ConsoleGlobal; Map MapTiles[64][64];
int RadiusBaseX[9]={1,2,1,0,-1,-2,-1,0,0},RadiusBaseY[9]={-1,0,1,2,1,0,-1,-2,0};
int RadiusOffsetX[289],RadiusOffsetY[289]; int MapLongitudeBounds=64,MapLatitudeBounds=64,MapIsFlat=1;
int IsMultiplayerNet,TurnCurrentNum,GameState,GamePreferences,TutWinFlags9A64BC,DesignTutorState;
int TutorialFlags,TutorialFlags2,GenderDefault,PluralityDefault,BaseFindDist; char StringTemp[1024];
Sprite g_TER1_FARM_SPRITES[5],g_TER1_MINE_SPRITE,g_TER1_SOLAR_COLLECTOR_SPRITE;
Sprite g_TER1_SEA_LAND_RESOURCE_SPRITES[12],g_TER1_UNITY_POD_SPRITES[6];
int calls[32],focus_count,map_count,map_flag,map_x,map_y,map_state,map_arrow_x,map_arrow_y; const char *map_text; Sprite *map_sprite;
int reject_mask,mutate_mask,mutate_proto,mutate_triad,goody_on,goody_x,goody_y;
int enemy_vehicle_on,enemy_vehicle_x,enemy_vehicle_y,enemy_base_on,enemy_base_x,enemy_base_y;
int bonus_value,site_value=1,territory_result=-999;

Map *map_loc(int x,int y) { return &MapTiles[y][x]; }
void say_base(char *p,int) { p[0]='b';p[1]=0; }
void say_stats_3(char *p,int) { p[0]='s';p[1]=0; }
void parse_says(int,const char *,int,int) {}
int can_arty(int,int) { return 0; } int enemy_move(int) { return 0; }
int veh_at(int x,int y) { return enemy_vehicle_on&&x==enemy_vehicle_x&&y==enemy_vehicle_y?1:-1; }
int base_at(int x,int y) { return enemy_base_on&&x==enemy_base_x&&y==enemy_base_y?0:-1; }
int tut_check(int f) { int i=0;while(i<32&&static_cast<unsigned int>(f)!=(1u<<i))++i;if(i<32)++calls[i];
    if(f==mutate_mask){Vehs[0].proto_id=mutate_proto;Chassis[VehPrototypes[mutate_proto].chassis_id].triad=mutate_triad;return 0;}
    if(f==reject_mask||(TutorialFlags&f)) return 0;
    TutorialFlags|=f;return 1; }
int tut_check2(int f) { if(TutorialFlags2&f)return 0;TutorialFlags2|=f;return 1; }
int whose_territory(int f,int,int,int *,int *) { return territory_result==-999?f:territory_result; }
int world_site(int,int,int) { return site_value; } void site_set(int x,int y,int s){MapTiles[y][x].val2=s<<4;}
int bonus_at(int,int,int){return bonus_value;} int goody_at(int x,int y){return goody_on&&x==goody_x&&y==goody_y;}
int base_find(int,int,int){BaseFindDist=0;BaseCurrent=&Bases[0];return 0;}
int can_terraform(int,int,int,int,int){return 0;} int at_goal(int,int,int,int){return 0;} int good_sensor(int,int,int){return 0;}
int TutorialStub::tut_map(const char *s,int x,int y,int,Sprite *p,int f,int ax,int ay){++count;++map_count;text=s;map_text=s;sprite=p;map_sprite=p;flag=f;map_flag=f;map_x=x;map_y=y;map_arrow_x=ax;map_arrow_y=ay;game_state_at_map=GameState;map_state=GameState;return 0;}
void ConsoleGlobalStub::focus(int,int,int){++focus_count;}

/* __EXTRACTED_GIVE_HINTS_SOURCE__ */

static void reset_case(Console &c) {
    memset(Vehs,0,sizeof(Vehs));memset(VehPrototypes,0,sizeof(VehPrototypes));memset(Chassis,0,sizeof(Chassis));
    memset(Weapon,0,sizeof(Weapon));memset(Armor,0,sizeof(Armor));memset(Bases,0,sizeof(Bases));
    memset(PlayersData,0,sizeof(PlayersData));memset(Players,0,sizeof(Players));memset(MapTiles,0,sizeof(MapTiles));
    memset(RadiusOffsetX,0,sizeof(RadiusOffsetX));memset(RadiusOffsetY,0,sizeof(RadiusOffsetY));memset(calls,0,sizeof(calls));
    MapLongitudeBounds=MapLatitudeBounds=64;MapIsFlat=1;IsMultiplayerNet=TurnCurrentNum=GameState=0;GamePreferences=0x20;
    TutWinFlags9A64BC=TutorialFlags=TutorialFlags2=0;DesignTutorState=77;TutorialWindow.tour_stage=0;
    BaseCurrentCount=0;BaseCurrent=&Bases[0];BaseFindDist=0;ConsoleGlobal.faction_id_=0;map_count=focus_count=map_flag=map_x=map_y=map_state=map_arrow_x=map_arrow_y=0;
    map_text=0;map_sprite=0;reject_mask=mutate_mask=mutate_proto=mutate_triad=0;goody_on=enemy_vehicle_on=enemy_base_on=0;
    goody_x=goody_y=enemy_vehicle_x=enemy_vehicle_y=enemy_base_x=enemy_base_y=0;bonus_value=0;site_value=1;territory_result=-999;
    HostMapWin.field_1DDA4_=HostMapWin.field_1DDA8_=0;HostMapWin.field_1DDCC_=64;HostMapWin.field_1DDD0_=64;
    HostMapWin.field_1DDD4_=HostMapWin.field_1DDD8_=0;MapWinTable[0]=&HostMapWin;
    c.selected_vehicle_=0;c.faction_id_=1;Vehs[0].x=Vehs[0].y=10;Vehs[0].proto_id=0;Vehs[0].home_base_id=0;Vehs[0].faction_id=1;
    VehPrototypes[0].chassis_id=0;VehPrototypes[0].weapon_id=VehPrototypes[0].armor_id=0;VehPrototypes[0].veh_name="test";
    Chassis[0].triad=TRIAD_LAND;PlayersData[1].flags=0x200;
}
static int eq(const char *a,const char *b){return a&&strcmp(a,b)==0;}
static void run_case(const char *name) {
    Console c;reset_case(c);
    if(eq(name,"state-clear")){VehPrototypes[0].plan=PLAN_COLONIZATION;Vehs[0].dmg_incurred=1;c.give_hints();assert(focus_count==0&&map_count==1&&eq(map_text,"DAMAGED"));}
    else if(eq(name,"colonist")){Vehs[0].state=0x2000;VehPrototypes[0].plan=PLAN_COLONIZATION;Players[0].rule_flags=0x100;Vehs[0].dmg_incurred=1;c.give_hints();assert(focus_count==1&&map_count==0&&calls[21]==0);}
    else if(eq(name,"stage-owner")){Vehs[0].state=0x2000;VehPrototypes[0].plan=PLAN_TERRAFORMING;PlayersData[1].proto_id_active[0]=1;c.give_hints();assert(map_count==1&&eq(map_text,"NEWTERRAFORMER")&&TutorialWindow.tour_stage==2&&DesignTutorState==77);}
    else if(eq(name,"game-state-order")){Vehs[0].state=0x2000;VehPrototypes[0].plan=PLAN_TERRAFORMING;PlayersData[1].proto_id_active[0]=1;c.give_hints();assert(map_count==1&&map_state==0&&(GameState&0x400));}
    else if(eq(name,"site-owner")){Vehs[0].state=0x2000;VehPrototypes[0].plan=PLAN_COLONIZATION;Chassis[0].triad=TRIAD_LAND;RadiusOffsetX[49]=1;RadiusOffsetY[49]=0;territory_result=2;c.give_hints();assert(map_count==1&&eq(map_text,"NEWCOLONIST")&&map_arrow_x==-1&&map_arrow_y==-1);}
    else if(eq(name,"no-base-check")){Vehs[1].faction_id=2;Vehs[1].proto_id=1;Chassis[0].triad=TRIAD_LAND;MapTiles[10][10].climate=0x40;enemy_vehicle_on=1;enemy_vehicle_x=11;enemy_vehicle_y=9;c.give_hints();assert(map_count==1&&eq(map_text,"ENEMYVEH")&&calls[13]==0);}
    else if(eq(name,"valid-unity")){
        const int tx[6]={11,12,11,10,9,8},ty[6]={9,10,11,12,11,10};
        for(int sea=0;sea<2;++sea)for(int rem=0;rem<3;++rem){
            reset_case(c);Chassis[0].triad=sea?TRIAD_SEA:TRIAD_LAND;
            if(sea)TutorialFlags|=0x20;
            const int k=sea*3+rem;Vehs[0].x=10;Vehs[0].y=10;
            goody_on=1;goody_x=tx[k];goody_y=ty[k];
            MapTiles[ty[k]][tx[k]].climate=sea?0:0x60;
            c.give_hints();
            const int expected=sea+2*((5*ty[k]+7*tx[k])%3);
            assert(map_count==1&&eq(map_text,"EXPLORATION")&&map_sprite==&g_TER1_UNITY_POD_SPRITES[expected]);
        }
    }
    else {fprintf(stderr,"unknown host scenario: %s\n",name);abort();}
}
int main(int argc,char **argv){if(argc!=2)return 2;run_case(argv[1]);return 0;}
