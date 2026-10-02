// Host external component/API doubles and branch controls only.
// PickTech declarations and bodies are injected from current product source.
// This does not model native ABI, layout, real callees or game runtime.
#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <string>
#include <vector>
#define __cdecl
typedef char *LPSTR;
const int MaxTechnologyNum=89, MaxVehProtoFactionNum=64, MaxAbilityNum=29;
const int MaxArmorNum=14, MaxWeaponNum=26, MaxReactorNum=4, MaxChassisNum=9;
const int MaxTerrainNum=20, MaxFacilityNum=134, FacilitySPStart=70;
const int TECH_TRANT=88, FALSE=0;
struct RECT { int left, top, right, bottom; };
int popup_ctor=0,popup_dtor=0,list_ctor=0,list_dtor=0;
int list_load_result=1,list_remove_calls=0,list_final_cursor=-1;
int draw_calls=0,close_calls=0,modal_calls=0,ai_calls=0,random_calls=0;
int modal_selection=0,fx_calls=0,tutorial_calls=0,console_tech_calls=0;
int random_bounds[200]={};
std::vector<std::string> displayed;
std::vector<std::string> candidate_captions;
char StringTemp[1032]={}, PopupStartCaption[256]="popup";
char tutorial_caption_storage[16]="tutorial";
char *TutorialPopupCaption=tutorial_caption_storage;
const char *DefaultFontFace="font";
int WinScreenWidth=800,WinScreenHeight=600,MessageWinTextColor=7;
int MapWinCaptionScale=0,MapWinCaptionDivisor=1;
int SpriteScaleNumerator=1,SpriteScaleDenominator=1;
uint32_t GameRules=0,GamePreferences=0,LockEnableMask=0;
int TurnCurrentNum=20,TechAchievedGate=0,IsMultiplayerNet=0,ConsoleControlTurnActive=0;
struct Font { int height=12; void init(const char *,int h,int){height=h;}
  int line_advance()const{return height;} };
struct Heap{};
Heap *StringAllocationHeap=0;
struct StringList { Heap *saved;
 int current_position_=0;
 StringList():saved(StringAllocationHeap){StringAllocationHeap=0;++list_ctor;}
 // Observe the cursor on entry to external destructor cleanup, not after
 // pretending that external cleanup establishes the owner's missing store.
 ~StringList(){list_final_cursor=current_position_;StringAllocationHeap=saved;++list_dtor;}
 int load(char *,char *,int,int){current_position_=17;return list_load_result;}
 void remove_all(){++list_remove_calls;} };
struct Style{}; Style InterfaceBoxStyles[10];
struct Buffer {
 RECT working={0,0,700,500};
 void set_font(Font*,Font*,Font*,int){}
 void set_text_color(int,int,int,int){} void set_text_color2(int,int,int,int){}
 int pixel_width(){return working.right;} int pixel_height(){return working.bottom;}
 void set_clip(RECT*){} void restore_clip(){} void box_sprite(RECT*,Style*){}
 const RECT &clip_rect() const {return working;}
 int text_width(const char *s){return int(strlen(s))*6;}
 int wrap_height(char*,int){return 12;}
 int wrap_cent(char *s,int,int y,int){displayed.push_back(s);return y+12;}
 int write_strings(StringList*,int,int y,int,int){return y+12;}
 int write_cent_l(char*,RECT*,int){return 0;}
};
struct Win {
 int caption_height_=-9;
 virtual ~Win(){} virtual void show(int){} virtual void hide(){}
 virtual void on_mouse_move(int,int){} virtual int on_key_click(int,int){return 0;}
 virtual void on_left_click_event(int,int){} virtual void vslot_22(int,int){}
 virtual int set_modal(int,int(*)(),Win*);
 void center(){} void set_caption_height(int){} void on_lose_mouse_capture(){}
 void release_modal(){} };
Win *WinPointerOwner3=0,*WinPointerOwner4=0;
struct GraphicWin:Win,Buffer {
 virtual ~GraphicWin(){} unsigned close(){++close_calls;return 0;}
 void soft_update(){} virtual void on_update(){}
 int init(int,int,int w,int h,char*,int,Win*,void*,void*){working={0,0,w,h};return 0;}
};
struct Dialogs {void item(char *s,int){candidate_captions.push_back(s);}}
;
struct Popup {int value=-1; Popup(){++popup_ctor;}~Popup(){++popup_dtor;}
 int start(char*,const char*,int,char*,int,GraphicWin*){return 0;}
 Dialogs &dialogs(){static Dialogs d;return d;} void set_value(int v){value=v;}
 const char *caption_text()const{return "technology";} };
struct RulesTechnology {LPSTR name=nullptr;int preq_tech_1=-100,preq_tech_2=-100;
 int power_value=0,tech_value=0,wealth_value=0,growth_value=0;};
RulesTechnology Technology[89];
struct RulesFacility {LPSTR name=nullptr,effect=nullptr;int preq_tech=-100,free_tech=-100;};
RulesFacility Facility[134];
struct RulesCitizen {LPSTR singular_name=nullptr;int preq_tech=-100;}; RulesCitizen Citizen[10];
struct RulesAbility {LPSTR name=nullptr;int preq_tech=-100;};RulesAbility Ability[29];
struct RulesTerraforming {LPSTR name=nullptr,name_sea=nullptr;int preq_tech=-100,preq_tech_sea=-100;};
RulesTerraforming Terraforming[20];
struct RulesOrder {LPSTR order=nullptr,order_sea=nullptr;};RulesOrder Order[40];
struct RulesChassis {LPSTR offsv_1_name=nullptr;int preq_tech=-100,triad=0;uint8_t speed=1;};
RulesChassis Chassis[9];
struct RulesArmor {LPSTR name=nullptr;int preq_tech=-100;int8_t defense_rating=1;};RulesArmor Armor[14];
struct RulesWeapon {LPSTR name=nullptr;int preq_tech=-100;int8_t offense_rating=1;uint8_t cost=1;};
RulesWeapon Weapon[26];
struct RulesReactor {LPSTR name=nullptr;int preq_tech=-100;};RulesReactor Reactor[4];
struct VehPrototype {char veh_name[32]={};int flags=0,preq_tech=-100;};VehPrototype VehPrototypes[64];
struct RulesSocialCategory {LPSTR name[4]={};int preq_tech[4]={-100,-100,-100,-100};};
RulesSocialCategory SocialCategories[4];
struct RulesMandate {LPSTR name_caps=nullptr;};RulesMandate Mandate[4];
LPSTR Triad[3]={}; int MandateColors[8]={};
struct RulesBasic {int tech_three_energy_sqr=-100,tech_three_minerals_sqr=-100,tech_three_nutrients_sqr=-100;}Rules;
struct PlayerData {int tech_ranking=0,theory_of_everything=0;};PlayerData PlayersData[8];
struct Console {int faction_id_=1;void set_tech(int){++console_tech_calls;}}ConsoleGlobal;
struct FX {void play(int){++fx_calls;}}g_FX;
struct Sprite {int transparent_index(){return 0;}void draw_mono(Buffer*,int,int,int,int){++draw_calls;}};
Sprite g_TECH_ICON_SPRITES[89];
int string_get_calls=0, label_get_calls=0;
std::vector<std::string> lookup_events;
struct StringStore {const char *get(int){++string_get_calls;lookup_events.push_back("name");return "name";}}StringTable;
int datalink_calls=0,datalink_opcode=-1,datalink_tech=-1;
struct Datalink {int exec(int opcode,int tech){++datalink_calls;datalink_opcode=opcode;datalink_tech=tech;return 0;}}DatalinkMain;
int owned[8][89]={}, available[89]={},category_id[89]={},values[89]={};
int project_bases[64];
int base_project(int i){return project_bases[i];}
bool has_tech(int tech,int faction){return tech>=0&&tech<89&&owned[faction][tech];}
int tech_avail(int tech,int){return available[tech];}
int tech_category(int tech){return tech<89 ? category_id[tech] : 0;}
int tech_ai(int){++ai_calls;return 77;}
int tech_val(int tech,int,int){return values[tech];}
int tech_mil(int t){return Technology[t].power_value;}
int tech_tech(int t){return Technology[t].tech_value;}
int tech_infra(int t){return Technology[t].wealth_value;}
int tech_colonize(int t){return Technology[t].growth_value;}
int tech_recurse(int,int){return 3;}
int mandate_color(int category){return category;}
int veh_fake(int,int){++draw_calls;return 2048;}
int veh_draw(Buffer*,int,int,int,int,int,int){++draw_calls;return 0;}
unsigned random(int,int high){random_bounds[random_calls++]=high;return 0;}
char *label_get(int index){++label_get_calls;lookup_events.push_back("label");static char l[400][32];sprintf(l[index],"label%d",index);return l[index];}
Font *find_font(int,int){static Font f;return &f;}
void say_tech(char *buffer,int tech,int){sprintf(buffer+strlen(buffer),"T%d",tech);}
void help_any(){} void do_all_draws(){}
int tut_check2(int){return 1;}
int popt(char*,const char*,int,char*,int){++tutorial_calls;return 0;}
char *_itoa(int n,char *b,int){sprintf(b,"%d",n);return b;}
char *CharUpperA(char *s){for(char *p=s;*p;++p)*p=char(toupper(*p));return s;}
// Current product declarations and bodies are inserted by the source-extraction test.









/* PICKTECH_PRODUCT_DECLARATION */
/* PICKTECH_PRODUCT_IMPLEMENTATION */
int Win::set_modal(int flag,int(*)(),Win*) {
    ++modal_calls;
    if (flag) static_cast<PickTech *>(this)->selected_index_=modal_selection;
    return 0;
}
static void reset_state() {
    string_get_calls=label_get_calls=0;
    lookup_events.clear();
    memset(owned,0,sizeof owned);memset(available,0,sizeof available);
    memset(category_id,0,sizeof category_id);memset(values,0,sizeof values);
    popup_ctor=popup_dtor=ai_calls=random_calls=modal_calls=draw_calls=0;
    fx_calls=tutorial_calls=console_tech_calls=0;
    list_ctor=list_dtor=0;displayed.clear();candidate_captions.clear();
    GameRules=GamePreferences=LockEnableMask=0;TurnCurrentNum=20;
    IsMultiplayerNet=ConsoleControlTurnActive=TechAchievedGate=0;
    WinPointerOwner3=WinPointerOwner4=0;modal_selection=0;
    StringAllocationHeap=0;
    list_load_result=1;list_remove_calls=0;list_final_cursor=-1;
    datalink_calls=0;datalink_opcode=datalink_tech=-1;
    for(int i=0;i<64;++i)project_bases[i]=-1;
    for(int i=0;i<89;++i)Technology[i]=RulesTechnology();
    for(int i=0;i<8;++i)PlayersData[i]=PlayerData();
    Rules=RulesBasic();
    for(int i=0;i<134;++i)Facility[i]=RulesFacility();
    for(int i=0;i<10;++i)Citizen[i]=RulesCitizen();
    for(int i=0;i<29;++i)Ability[i]=RulesAbility();
    for(int i=0;i<20;++i)Terraforming[i]=RulesTerraforming();
    for(int i=0;i<9;++i)Chassis[i]=RulesChassis();
    for(int i=0;i<14;++i)Armor[i]=RulesArmor();
    for(int i=0;i<26;++i)Weapon[i]=RulesWeapon();
    for(int i=0;i<4;++i)Reactor[i]=RulesReactor();
    for(int i=0;i<64;++i)VehPrototypes[i]=VehPrototype();
}
static void initialize_draw(PickTech &p) {
    p.item_count_=1;p.cell_width_=240;p.cell_height_=300;p.row_pitch_=10;
    p.faction_id_=1;p.column_count_=1;p.row_count_=1;
}
int main() {
    reset_state();
    {
        PickTech p;
        assert(p.column_count_==0&&p.row_count_==0&&p.selected_index_==0&&p.selection_active_==0);
        assert(p.pick(1,0,-1,0)==77);
        assert(ai_calls==1&&random_calls==0&&popup_ctor==4&&popup_dtor==4);
    }
    reset_state();
    {PickTech p;assert(p.pick(1,1,-1,0)==-1);assert(popup_ctor==4&&popup_dtor==4);}
    reset_state();
    available[4]=available[5]=1;values[4]=values[5]=20;
    Technology[4].power_value=3;Technology[4].tech_value=6;
    Technology[4].growth_value=3;Technology[4].wealth_value=6;
    {PickTech p;assert(p.pick(1,1,-1,0)==5);
     assert(p.item_count_==2&&p.technology_ids_[0]==4&&p.technology_ids_[1]==5);
     assert(random_calls==4&&random_bounds[0]==9&&random_bounds[1]==17);
     assert(modal_calls==0&&popup_dtor==4);}
    reset_state();
    LockEnableMask=2;available[1]=available[2]=available[8]=1;
    modal_selection=1;
    {PickTech p;assert(p.pick(1,0,-1,0)==2);
     assert(p.item_count_==2&&p.technology_ids_[0]==1&&p.technology_ids_[1]==2);
     assert(modal_calls==1&&p.row_pitch_==10&&WinPointerOwner3==&p);
     assert(p.caption_height_==29);}
    reset_state();
    owned[2][5]=owned[2][6]=1;Technology[5].preq_tech_1=3;
    Technology[6].preq_tech_2=3;owned[1][3]=1;
    Technology[5].preq_tech_1=4;
    {PickTech p;assert(p.pick(1,1,2,0)==6);assert(p.item_count_==1);
     assert(random_calls==2&&p.technology_ids_[0]==6);}
    reset_state();
    LockEnableMask=2;modal_selection=41;
    for(int i=0;i<89;++i)available[i]=1;
    {PickTech p;assert(p.pick(1,1,-1,0)==41);
     assert(p.column_count_==10&&p.row_count_==9&&p.item_count_==89);
     assert(random_calls==178&&p.row_pitch_==10&&popup_dtor==4);}
    reset_state();
    GameRules=0x200;GamePreferences=0x20;PlayersData[1].tech_ranking=2;
    {PickTech p;assert(p.pick(1,0,-1,0)==77);assert(tutorial_calls==1&&console_tech_calls==1);}
    reset_state();
    GamePreferences=0x20;IsMultiplayerNet=ConsoleControlTurnActive=1;
    {PickTech p;assert(p.pick(1,1,-1,0)==-1);assert(tutorial_calls==1&&fx_calls==1);}
    reset_state();
    {PickTech p;initialize_draw(p);RECT rect={0,0,240,300};
     Heap original_heap;StringAllocationHeap=&original_heap;
     assert(p.draw_tech(5,&rect,0)==1);
     assert(list_ctor==1&&list_dtor==1&&StringAllocationHeap==&original_heap);
     assert(displayed.size()>=2&&displayed[1]=="name 3");
     const int drawn=draw_calls;
     assert(p.draw_tech(97,&rect,0)==1&&draw_calls==drawn);
     assert(list_ctor==1&&list_dtor==1);
     assert(p.draw_tech(-1,&rect,0)==0);}
    reset_state();
    available[5]=1;
    Rules.tech_three_nutrients_sqr=Rules.tech_three_minerals_sqr=Rules.tech_three_energy_sqr=5;
    VehPrototypes[0].flags=1;VehPrototypes[0].preq_tech=5;strcpy(VehPrototypes[0].veh_name,"prototype");
    Reactor[0].preq_tech=Weapon[0].preq_tech=Armor[0].preq_tech=Chassis[0].preq_tech=5;
    Facility[1].preq_tech=Facility[70].preq_tech=5;
    Terraforming[0].preq_tech=Terraforming[0].preq_tech_sea=5;
    Ability[0].preq_tech=Citizen[0].preq_tech=5;Technology[0].preq_tech_1=5;
    {PickTech p;assert(p.pick(1,1,-1,0)==5);
     assert(candidate_captions[0]=="T5|(label278)++++++++++++");
     // Native later-benefit arms do not look up unused labels or rule names.
     assert(label_get_calls==1 && string_get_calls==0);}
    reset_state();available[5]=1;Facility[70].preq_tech=5;project_bases[0]=0;
    Technology[0].preq_tech_1=Technology[1].preq_tech_1=5;
    {PickTech p;assert(p.pick(1,1,-1,0)==5);
     assert(candidate_captions[0]=="T5|(label157: name)");}
    reset_state();available[5]=1;Terraforming[0].preq_tech=Terraforming[0].preq_tech_sea=5;
    {PickTech p;assert(p.pick(1,1,-1,0)==5);
     assert(candidate_captions[0]=="T5|(name)");}
    reset_state();available[5]=1;Weapon[0].preq_tech=5;Weapon[0].offense_rating=0;
    {PickTech p;assert(p.pick(1,1,-1,0)==5);
     assert(candidate_captions[0]=="T5|(label128: name)");
     assert(lookup_events.size()==2&&lookup_events[0]=="label"&&lookup_events[1]=="name");}
    reset_state();available[5]=1;Weapon[0].preq_tech=5;Weapon[0].offense_rating=-1;
    {PickTech p;assert(p.pick(1,1,-1,0)==5);
     assert(candidate_captions[0]=="T5|(label129: name)");}
    reset_state();
    {PickTech p;initialize_draw(p);p.item_count_=4;p.column_count_=2;p.row_count_=2;
     for(int i=0;i<4;++i)p.technology_ids_[i]=i;
     p.selected_index_=0;p.on_key_click(0,0x25);assert(p.selected_index_==1);
     p.on_key_click(0,0x26);assert(p.selected_index_==3);
     p.on_mouse_move(-1,0);assert(p.selected_index_==-1&&p.selection_active_==0);
     p.on_mouse_move(1,30);assert(p.selected_index_==0&&p.selection_active_==1);
     WinPointerOwner3=WinPointerOwner4=&p;static_cast<Win *>(&p)->on_left_click_event(1,30);
     assert(WinPointerOwner3==0&&WinPointerOwner4==0);}
    reset_state();
    {PickTech p;initialize_draw(p);p.cell_height_=2000;RECT rect={0,0,240,2000};
     Chassis[0].preq_tech=5;Chassis[0].speed=200;
     p.draw_tech(5,&rect,0);bool found=false;
     for(size_t k=0;k<displayed.size();++k)if(displayed[k]=="{name (name 200)}")found=true;
     assert(found);}
    reset_state();
    {PickTech p;initialize_draw(p);RECT rect={0,0,240,2000};
     Weapon[0].preq_tech=5;Weapon[0].offense_rating=0;Weapon[0].cost=250;
     p.draw_tech(5,&rect,0);bool found=false;
     for(size_t k=0;k<displayed.size();++k)if(displayed[k]=="{name (l 250)}")found=true;
     assert(found);}
    reset_state();
    {PickTech p;initialize_draw(p);RECT rect={0,0,240,2000};
     Reactor[0].preq_tech=5;
     p.draw_tech(5,&rect,0);bool found=false;
     for(size_t k=0;k<displayed.size();++k)if(displayed[k]=="name (l: 1)")found=true;
     assert(found);}
    reset_state();
    {PickTech p;p.column_count_=p.row_count_=p.selected_index_=p.selection_active_=99;
     p.close();assert(p.column_count_==0&&p.row_count_==0&&p.selected_index_==0&&p.selection_active_==0);}
    reset_state();
    {PickTech p;initialize_draw(p);RECT rect={0,0,240,300};
     Heap original_heap;StringAllocationHeap=&original_heap;list_load_result=0;
     assert(p.draw_tech(5,&rect,0)==1);
     assert(list_remove_calls==1&&list_final_cursor==0);
     assert(list_ctor==1&&list_dtor==1&&StringAllocationHeap==&original_heap);}
    reset_state();
    {struct MouseOverride : PickTech {
         int calls=0;
         void on_mouse_move(int,int){++calls;selection_active_=1;selected_index_=0;}
     } p;
     initialize_draw(p);p.technology_ids_[0]=7;
     WinPointerOwner3=WinPointerOwner4=&p;
     static_cast<Win *>(&p)->on_left_click_event(-500,-500);
     assert(p.calls==1&&WinPointerOwner3==0&&WinPointerOwner4==0);
     static_cast<Win *>(&p)->vslot_22(-500,-500);
     assert(p.calls==2&&datalink_calls==1&&datalink_opcode==14&&datalink_tech==7);}
    puts("PickTech current-product controls passed (external APIs are doubles)");
}
