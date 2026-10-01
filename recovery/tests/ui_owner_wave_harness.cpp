// Template only: Python extracts the five actual product bodies and inserts
// them at the BODY_* markers before strict host compilation.
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

static std::vector<std::string> log_events;
static void event(const char *s) { log_events.push_back(s); }
static void check(bool ok, const char *what) {
  if (!ok) { std::fprintf(stderr, "assertion: %s\n", what); std::exit(41); }
}

struct Palette {};
struct RECT { int left, top, right, bottom; };
struct Buffer {
  int result;
  int calls;
  Buffer() : result(73), calls(0) {}
  int load_pcx(const char *, Palette *, int, int) { ++calls; event("buffer-load"); return result; }
};
class Win {
 public:
  uint32_t iFlags_;
  int feedback_x_, feedback_y_;
  Win() : iFlags_(0), feedback_x_(-1), feedback_y_(-1) {}
  virtual void on_mouse_feedback(int x, int y) { feedback_x_=x; feedback_y_=y; event("feedback"); }
  virtual void release_modal() { event("release"); }
};
class GraphicWin : public Win, public Buffer {
 public:
  int load_pcx(char *, Palette *, int, int);
};

struct Spot {
  int answer;
  Spot() : answer(-1) {}
  int check(int, int, int *out_y, int) { *out_y=41; event("spot-check"); return answer; }
};
class SetupWin : public GraphicWin {
 public:
  Spot spot_;
  void on_left_click(int, int);
};

struct SocialCategory { int models_[4]; };
struct SocialEffect { int value; };
struct PlayerData {
  int energy_reserves;
  int soc_upheaval_cost_paid;
  SocialCategory soc_category_pending;
  SocialEffect soc_effect_pending;
  PlayerData() : energy_reserves(100), soc_upheaval_cost_paid(0), soc_effect_pending() {
    std::memset(soc_category_pending.models_, 0, sizeof(soc_category_pending.models_));
  }
};
struct Player { uint32_t rule_flags; Player() : rule_flags(0) {} };
enum { STATE_SCENARIO_EDITOR=0x40, STATE_OMNISCIENT_VIEW=0x80, RFLAG_ALIEN=0x80 };
uint32_t GameState=0;
PlayerData PlayersData[8];
Player Players[8];
char ScriptPopupFile[]="scripts";
int pop_result=1, social_result=30;
int social_switch_faction=-1, parse_switch_faction=-1, popup_switch_faction=-1, synch_switch_faction=-1;
int parse_values[2]={-1,-1};
int sync_values[4]={-1,-1,-1,-1};
int sync_count=0;

class SocialWin : public GraphicWin {
 public:
  uint32_t factionID_;
  SocialCategory socCategory_;
  SocialWin() : factionID_(0) { std::memset(socCategory_.models_,0,sizeof(socCategory_.models_)); }
  void reset() { event("reset"); }
  void exit();
  int on_key_click(int, int);
  void on_button_clicked(int);
};
static SocialWin *active_social=0;
class TutWin { public: int reset() { event("tut-reset"); return 1; } };
TutWin TutorialWindow;

uint32_t social_upheaval(int faction, SocialCategory *) {
  event("upheaval");
  const uint32_t gross=static_cast<uint32_t>(social_result+PlayersData[faction].soc_upheaval_cost_paid);
  if (social_switch_faction>=0 && active_social) active_social->factionID_=static_cast<uint32_t>(social_switch_faction);
  return gross;
}
int parse_num(int which, int value) {
  event(which==0 ? "parse-upheaval" : "parse-energy");
  parse_values[which]=value;
  if (which==0 && parse_switch_faction>=0 && active_social) active_social->factionID_=static_cast<uint32_t>(parse_switch_faction);
  return 0;
}
int popp(char *, const char *, int, const char *, int (*)()) {
  event("popp");
  if (popup_switch_faction>=0 && active_social) {
    active_social->factionID_=static_cast<uint32_t>(popup_switch_faction);
    for(int i=0;i<4;++i) active_social->socCategory_.models_[i]=90+i;
  }
  return pop_result;
}
int pop_wait() { return 0; }
void synch_alloc(int faction) { event("synch-alloc"); sync_values[sync_count++]=faction; if(synch_switch_faction>=0&&active_social) active_social->factionID_=static_cast<uint32_t>(synch_switch_faction); }
void synch_soc(int faction) { event("synch-soc"); sync_values[sync_count++]=faction; }
void net_energy(int faction, int, int, int, int) { event("net-energy"); sync_values[sync_count++]=faction; }
void social_calc(SocialCategory *, SocialEffect *, int faction, int, int) { event("social-calc"); sync_values[sync_count++]=faction; }

BODY_GRAPHIC
BODY_SETUP
BODY_EXIT
BODY_KEY
BODY_BUTTON

static void reset_state() {
  log_events.clear(); GameState=0; pop_result=1; social_result=30;
  social_switch_faction=parse_switch_faction=popup_switch_faction=synch_switch_faction=-1;
  parse_values[0]=parse_values[1]=-1; sync_count=0;
  for(int i=0;i<4;++i) sync_values[i]=-1;
  for(int i=0;i<8;++i) { PlayersData[i]=PlayerData(); Players[i]=Player(); }
}
static void test_graphic_dispatch() {
  reset_state(); GraphicWin g; Palette palette; char path[]="load.pcx";
  g.iFlags_=0x800000; check(g.load_pcx(path,&palette,1,2)==1 && g.calls==0,"GraphicWin guard prevents base call");
  g.iFlags_=0; g.result=19; check(g.load_pcx(path,&palette,1,2)==19 && g.calls==1,"GraphicWin delegates result through Buffer base");
}
static void test_setup_original_coordinate() {
  reset_state(); SetupWin s; s.spot_.answer=5; s.on_left_click(7,8);
  check(log_events.size()==3 && log_events[0]=="spot-check" && log_events[1]=="feedback" && log_events[2]=="release","Setup click callback order");
  check(s.feedback_x_==7 && s.feedback_y_==8,"Spot output does not replace original feedback coordinate");
  reset_state(); SetupWin miss; miss.spot_.answer=-1; miss.on_left_click(2,3);
  check(log_events.size()==1 && log_events[0]=="spot-check","Setup miss has no callback or release");
}
static void test_social_live_reloads() {
  reset_state(); SocialWin s; active_social=&s; s.factionID_=0;
  PlayersData[0].soc_upheaval_cost_paid=5; PlayersData[0].energy_reserves=60;
  PlayersData[1].energy_reserves=50; PlayersData[2].energy_reserves=40; PlayersData[3].energy_reserves=30;
  PlayersData[4].soc_upheaval_cost_paid=10;
  social_switch_faction=4; parse_switch_faction=1; popup_switch_faction=2; synch_switch_faction=3;
  s.exit();
  check(parse_values[0]==25 && parse_values[1]==50,"paid-cost and prompt-energy loads use faction after preceding callbacks");
  check(PlayersData[0].soc_upheaval_cost_paid==5 && PlayersData[4].soc_upheaval_cost_paid==10 && PlayersData[2].soc_upheaval_cost_paid==25,"popup faction reload selects commit receiver");
  check(PlayersData[2].soc_category_pending.models_[0]==90,"pending models reload after popup callback");
  check(sync_count==4 && sync_values[0]==2 && sync_values[1]==3 && sync_values[2]==3 && sync_values[3]==3,"sync stages independently reload live faction");
}
static void test_social_exit_paths_and_input() {
  reset_state(); SocialWin s; active_social=&s; s.factionID_=0; pop_result=0; s.exit();
  check(log_events.size()==4 && log_events[3]=="popp" && PlayersData[0].soc_upheaval_cost_paid==0,"cancel returns before state writes and modal release");
  reset_state(); SocialWin too_much; active_social=&too_much; too_much.factionID_=0; social_result=200; PlayersData[0].energy_reserves=10; too_much.exit();
  check(log_events.size()==4 && log_events[3]=="popp" && PlayersData[0].soc_upheaval_cost_paid==0,"too-expensive path returns after warning");
  reset_state(); SocialWin bypass; active_social=&bypass; bypass.factionID_=0; GameState=STATE_SCENARIO_EDITOR|STATE_OMNISCIENT_VIEW; bypass.exit();
  check(log_events.size()==6 && log_events[0]=="upheaval" && log_events[1]=="synch-alloc" && log_events[5]=="release","both bypass bits skip popup but commit and release");
  reset_state(); SocialWin input; active_social=&input;
  check(input.on_key_click(0,27)==1 && log_events[0]=="tut-reset" && log_events[1]=="reset" && log_events.back()=="release","Escape resets then exits");
  reset_state(); input.on_button_clicked(-2);
  check(log_events[0]=="tut-reset" && log_events[1]=="reset" && log_events.back()=="release","button -2 reset falls through to exit");
  reset_state(); input.on_button_clicked(27);
  check(log_events.size()==2 && log_events[0]=="tut-reset" && log_events[1]=="reset","button 27 resets without exit");
}
int main() { test_graphic_dispatch(); test_setup_original_coordinate(); test_social_live_reloads(); test_social_exit_paths_and_input(); }
