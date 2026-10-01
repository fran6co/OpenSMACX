#include <cassert>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#define __cdecl

struct Player { int is_leader_female = 0; char name_leader[16] = "leader"; char title_leader[16] = "title"; };
struct Data {
    struct { int economy = 0; } soc_effect_base;
    int unk_13 = 0, maint_cost_total = 0, current_num_bases = 1;
    int energy_reserves = 0;
    uint32_t diplo_treaties[8]{};
    int unk_29[8]{};
    int unk_30[8]{};
};
struct Sprite {};
struct SpriteBox { void sprite(Sprite *, const char *, int) {} };
struct Dialogs : SpriteBox { std::vector<std::pair<std::string, int>> rows; int list_count=0; void item(const char *s, int i) { rows.emplace_back(s, i); } };
struct Popup {
    Dialogs d;
    static int result;
    static int exec_calls;
    static int close_calls;
    static int scroll_close_calls;
    static int base_close_calls;
    static std::vector<std::pair<std::string,int>> last_rows;
    void close() { ++close_calls; ++scroll_close_calls; ++base_close_calls; }
    ~Popup() { close(); ++scroll_close_calls; ++base_close_calls; }
    void start(const char *, char *, int, void *, int, void *) {}
    Dialogs &dialogs() { return d; }
    bool picture_enabled() const { return d.list_count != 0; }
    void set_caller_picture(Sprite *) {}
    int exec(int, int (*)(void)) { ++exec_calls; last_rows=d.rows; return result; }
};
int Popup::result = 0; int Popup::exec_calls = 0; int Popup::close_calls = 0;
int Popup::scroll_close_calls = 0; int Popup::base_close_calls = 0;
std::vector<std::pair<std::string,int>> Popup::last_rows;

static constexpr int MaxPlayerNum = 8;
Player Players[8]; Data PlayersData[8];
int PopupDialogFactionID, TurnRedrawPending = 1, PluralityDefault, GenderDefault;
int IsMultiplayerNet, GovernorFaction = 1, TurnCurrentNum = 8;
int ExpansionEnabled = 0;
int DiploScratchFA98, DiploScratchFAA8 = -1, DiploScratchFA18 = -1, DiploScratchFA1C = -1, DiploScratchFA28 = -1;
int DiploScratchFA1C_unused;
int CouncWindowLeader[8]{};
std::vector<int> council_vote_args;
struct Council { int leader_label_kind_[8]{}; int votes[8]{}; int votes_for(int p) { council_vote_args.push_back(p); return votes[p]; } void set_vote(int p, int c) { last_set = {p,c}; } std::pair<int,int> last_set{-1,-1}; } CouncilWindow;
struct Tech { char *name = nullptr; } Technology[256];
Sprite portraits[8]; Sprite *FactionDiplomacyPortraits[8] = {&portraits[0],&portraits[1],&portraits[2],&portraits[3],&portraits[4],&portraits[5],&portraits[6],&portraits[7]};
char StringTemp[1032], PopupStartCaption[256], ScriptPopupFile[256], TextBufferGetPtr[1032];
int pop_wait() { return 0; }
char DiploLabelBUYNOTALK[]="notalk", DiploLabelBUYPROMISED[]="promised", DiploLabelBUYALREADY[]="already", DiploLabelBUYNO[]="no", DiploLabelBUYNO_6D8[]="no6", DiploLabelBUYVOTE[]="vote", DiploLabelGOV[]="gov", DiploLabelABSTAIN[]="abstain", DiploLabelNAY[]="nay", DiploLabelYEA[]="yea", DiploLabel1[]="one", DiploLabel2[]="two", DiploLabel3[]="three", DiploLabel4[]="four", DiploLabelA[]="a", DiploLabelBUYVOTEMENU[]="menu", DiploLabelSendingENERGY[]="energy", DiploLabelSendingTECH[]="tech";

struct Event { std::string name; int a[6]{}; std::string text; Event() = default; Event(const char *n) : name(n) {} };
std::vector<Event> events;
struct ConsoleMock { void update_data(int v) { Event e{"update"}; e.a[0]=v; events.push_back(e); } } ConsoleGlobal;
struct NetDaemonMock { void await_diplo(int opcode) { Event e{"await_diplo"}; e.a[0]=opcode; events.push_back(e); } } NetDaemonNet;
int force_text_open = 0, mutate_treaty_on_xpop = 0;
int mutate_gift_on_parse = 0, gift_parse_count = 0;
int want_result = 0;
int eligible_mask = 0;
std::vector<int> wants_results;
std::vector<std::array<int, 3>> wants_args;
size_t wants_cursor = 0;
std::vector<int> tech_values;
std::vector<std::array<int, 3>> tech_args;
std::vector<int> turn_after_tech;
size_t tech_cursor = 0;
static int diplo_add(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a) + static_cast<uint32_t>(b)); }
static int diplo_sub(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a) - static_cast<uint32_t>(b)); }
static int diplo_mul(int a, int b) { return static_cast<int>(static_cast<uint32_t>(a) * static_cast<uint32_t>(b)); }
char *_itoa(int n, char *out, int) { std::sprintf(out, "%d", n); return out; }
void diplomacy_check(int,int,int) { events.push_back({"check"}); }
void diplomacy_caption(int,int) { events.push_back({"caption"}); }
void tech_analysis(int,int) { events.push_back({"analysis"}); }
int is_alien_faction(int) { return 0; }
void parse_says(int slot, const char *text, int, int) {
    Event e{"parse_says"}; e.a[0]=slot; e.text=text; events.push_back(e);
    if (mutate_gift_on_parse && gift_parse_count++ == 0)
        PlayersData[2].unk_30[1] = 3;
}
void X_pop(const char *, int (*)(void)) { events.push_back({"xpop"}); if (mutate_treaty_on_xpop) PlayersData[2].diplo_treaties[1] = 0x20; }
void x_pops_short(const char *, int, Sprite *, int (*)(void)) { events.push_back({"xpops"}); }
int wants_prop(int a, int b, int c) {
    wants_args.push_back({a, b, c});
    if (wants_cursor < wants_results.size()) return wants_results[wants_cursor++];
    ++wants_cursor;
    return want_result;
}
int eligible(int player) { return (eligible_mask & (1 << player)) != 0; }
int council_votes(int p) { return CouncilWindow.votes_for(p); }
int tech_val(int technology, int faction, int simple) {
    tech_args.push_back({technology, faction, simple});
    const int value = tech_cursor < tech_values.size() ? tech_values[tech_cursor] : 10000;
    if (tech_cursor < turn_after_tech.size()) TurnCurrentNum = turn_after_tech[tech_cursor];
    ++tech_cursor;
    return value;
}
void parse_string(const char *s, char *out) { if (s != out) std::strncpy(out, s, 1020); out[1020]=0; }
void parse_num(int, int) {}
void parse_say(int, int, int, int) {}
void message_data(int a,int b,int c,int d,int e,int f) { Event ev{"message"}; ev.a[0]=a;ev.a[1]=b;ev.a[2]=c;ev.a[3]=d;ev.a[4]=e;ev.a[5]=f; events.push_back(ev); }
int X_text_open(const char *, const char *) { return force_text_open; }
void text_get() { std::strcpy(TextBufferGetPtr, "row"); }
void log_say(const char *, int a, int b, int c) { Event ev{"log"};ev.a[0]=a;ev.a[1]=b;ev.a[2]=c;events.push_back(ev); }
void synch_leader(int a) { Event ev{"sync"};ev.a[0]=a;events.push_back(ev); }
void tech_achieved(int a,int b,int c,int d) { Event ev{"tech_local"};ev.a[0]=a;ev.a[1]=b;ev.a[2]=c;ev.a[3]=d;events.push_back(ev); }

// PRODUCT_BODY_INSERTION_POINT

static void reset() {
    for (auto &p : PlayersData) p = Data{};
    for (auto &p : Players) p = Player{};
    for (auto &v : CouncilWindow.leader_label_kind_) v = 0;
    for (auto &v : CouncilWindow.votes) v = 0;
    events.clear(); council_vote_args.clear(); Popup::result=0; Popup::exec_calls=0; Popup::close_calls=0;
    Popup::scroll_close_calls=0; Popup::base_close_calls=0;
    Popup::last_rows.clear();
    TurnRedrawPending=1; IsMultiplayerNet=0; GovernorFaction=1; TurnCurrentNum=8;
    ExpansionEnabled=0; DiploScratchFA98=0; DiploScratchFAA8=10; DiploScratchFA18=11; DiploScratchFA1C=12; DiploScratchFA28=13;
    for (int i=0;i<8;++i) CouncilWindow.leader_label_kind_[i]=7;
    PlayersData[1].energy_reserves=10000; PlayersData[2].energy_reserves=10000;
    PlayersData[1].unk_13=20; PlayersData[2].unk_13=20;
    PlayersData[1].current_num_bases=1; PlayersData[2].current_num_bases=1;
    want_result=0; eligible_mask=0; wants_results.clear(); wants_args.clear(); wants_cursor=0;
    tech_values.clear(); tech_args.clear(); tech_cursor=0;
    force_text_open=0; mutate_treaty_on_xpop=0;
    mutate_gift_on_parse=0; gift_parse_count=0; turn_after_tech.clear();
}
static int count(const char *name) { int n=0; for (auto &e:events) if(e.name==name) ++n; return n; }
static void assert_native_popup_dtor() { assert(Popup::close_calls==1 && Popup::scroll_close_calls==2 && Popup::base_close_calls==2); }
static std::vector<int> row_ids() { std::vector<int> ids; for (auto &r:Popup::last_rows) ids.push_back(r.second); return ids; }
static Event find(const char *name, int nth=0) { for(auto &e:events) if(e.name==name && nth--==0) return e; assert(false); return {}; }
static void menu_body(int result, int net, int candidate_count=4) {
    reset(); Popup::result=result; IsMultiplayerNet=net;
    if (candidate_count == 1) tech_values = {1560};
    else if (candidate_count == 2) tech_values = {800,800};
    else if (candidate_count == 3) tech_values = {600,600,600};
    else tech_values = {400,400,400,400};
    if (candidate_count < 4) DiploScratchFA28=-1;
    if (candidate_count < 3) DiploScratchFA1C=-1;
    if (candidate_count < 2) DiploScratchFA18=-1;
    if (candidate_count < 1) DiploScratchFAA8=-1;
    buy_council_vote(1,2,1,0);
    assert(Popup::exec_calls==1);
    assert_native_popup_dtor();
}
int main() {
    // X_text_open aborts after popup construction; no modal exec or transaction.
    reset(); force_text_open=1; buy_council_vote(1,2,1,0);
    assert(Popup::exec_calls==0 && count("message")==0 && count("sync")==0);
    assert_native_popup_dtor();

    // A cancel result exits without transaction or council redraw.
    menu_body(0,0); assert(count("message")==0 && count("tech_local")==0 && count("sync")==0);
    assert((row_ids()==std::vector<int>{0,1,5}));

    // Item availability gates: no reserve means omit energy; no candidates
    // means omit all four research choices.
    reset(); Popup::result=0; PlayersData[1].energy_reserves=0;
    buy_council_vote(1,2,1,0); assert((row_ids()==std::vector<int>{0,2}));
    reset(); Popup::result=0; DiploScratchFAA8=DiploScratchFA18=DiploScratchFA1C=DiploScratchFA28=-1;
    buy_council_vote(1,2,1,0); assert((row_ids()==std::vector<int>{0,1}));

    // The native proposal tally reads every eligible peer against OTHER,
    // not FACTION, and never queries OTHER as a candidate.
    reset(); Popup::result=0; want_result=-2; eligible_mask=0;
    buy_council_vote(1,2,0,5);
    assert((council_vote_args==std::vector<int>{1,2,3,2,4,2,5,2,6,2,7,2}));

    // OTHER being governor doubles the threshold price; FACTION being
    // governor does not. One controlled tech score straddles those prices.
    reset(); Popup::result=0; DiploScratchFAA8=10;
    DiploScratchFA18=DiploScratchFA1C=DiploScratchFA28=-1; tech_values={2000};
    GovernorFaction=1; buy_council_vote(1,2,1,0);
    assert((row_ids()==std::vector<int>{0,1,2}));
    reset(); Popup::result=0; DiploScratchFAA8=10;
    DiploScratchFA18=DiploScratchFA1C=DiploScratchFA28=-1; tech_values={2000};
    GovernorFaction=2; buy_council_vote(1,2,1,0);
    assert((row_ids()==std::vector<int>{0,1}));

    // A computed zero price takes the short BUYVOTE0 route even for a
    // non-neutral OTHER, then reaches the common commit tail.
    reset(); PlayersData[1].energy_reserves=0; PlayersData[1].unk_13=0;
    PlayersData[1].maint_cost_total=0; PlayersData[1].current_num_bases=0;
    PlayersData[2].unk_13=0; PlayersData[2].maint_cost_total=0;
    PlayersData[2].current_num_bases=0;
    buy_council_vote(1,2,1,0);
    assert(Popup::exec_calls==0 && count("xpops")==1);
    assert(PlayersData[2].unk_29[1]==0 && PlayersData[2].unk_30[1]==1);

    // Energy purchase local mutation and multiplayer wire/log/sync payload.
    menu_body(1,0); assert(PlayersData[1].energy_reserves < 10000 && PlayersData[2].energy_reserves > 10000);
    assert(count("message")==0 && find("update").a[0]==1 && count("tech_local")==0);
    menu_body(1,1); { auto e=find("message"); assert(e.a[0]==0x2447 && e.a[2]==1 && e.a[3]<0 && e.a[4]==2 && e.a[5]==-e.a[3]); }
    assert(find("await_diplo").a[0]==0x447 && count("await_diplo")==1 && count("log")==1);
    assert(find("message",1).a[0]==0x2601 && find("sync").a[0]==2);
    assert(PlayersData[2].unk_30[1]==1);

    // Tech rows 1..4: every offered result maps to the matching candidate.
    for (int n=1;n<=4;++n) {
        menu_body(n+1,0,n); assert((row_ids()==std::vector<int>{0,1,n+1})); assert(count("tech_local")==n);
        assert(CouncilWindow.last_set==std::make_pair(2,0));
        for(int i=0;i<n;++i) { auto e=find("tech_local",i); assert(e.a[0]==2 && e.a[1]==10+i && e.a[2]==1 && e.a[3]==0); }
        menu_body(n+1,1,n); assert(count("message")==n+1 && count("await_diplo")==n);
        for(int i=0;i<n;++i) { auto e=find("message",i); assert(e.a[0]==0x244b && e.a[2]==2 && e.a[3]==10+i && e.a[4]==1 && e.a[5]==0); }
        for(int i=0;i<n;++i) assert(find("await_diplo",i).a[0]==0x44b);
        assert(find("message",n).a[0]==0x2601);
        assert(find("sync").a[0]==2);
    }

    // Each tech valuation is followed by a live current-turn reload. The
    // first callback advances the turn, raising the second score's scale.
    reset(); Popup::result=3; tech_values={800,400}; turn_after_tech={8,16};
    DiploScratchFAA8=10; DiploScratchFA18=11; DiploScratchFA1C=-1; DiploScratchFA28=-1;
    buy_council_vote(1,2,1,0);
    assert(Popup::exec_calls==1 && (row_ids()==std::vector<int>{0,1,3}));
    assert(tech_args.size()==2 && TurnCurrentNum==16);
    assert(count("tech_local")==2);

    // Already-voted callback mutates the receiving treaty word after the
    // popup; the function must reload it before deciding whether to sync.
    reset(); PlayersData[1].diplo_treaties[2]=2; PlayersData[2].diplo_treaties[1]=0;
    PlayersData[2].unk_29[1]=55; PlayersData[2].unk_30[1]=4;
    PlayersData[2].diplo_treaties[4]=0x30;
    CouncilWindow.leader_label_kind_[2]=4; IsMultiplayerNet=1;
    // choice equals the current leader label; X_pop establishes the 0x20 bit.
    mutate_treaty_on_xpop=1; buy_council_vote(1,2,1,4);
    assert(count("xpop")==1 && count("sync")==0);
    assert(PlayersData[2].unk_29[1]==55 && PlayersData[2].unk_30[1]==4);

    // Promised-vote callbacks reload the gift faction after parsing the name;
    // a callback-side storage mutation affects the following title lookup.
    reset(); PlayersData[2].unk_29[1]=55; PlayersData[2].unk_30[1]=4;
    std::strcpy(Players[4].name_leader, "gift4name");
    std::strcpy(Players[4].title_leader, "gift4title");
    std::strcpy(Players[3].name_leader, "gift3name");
    std::strcpy(Players[3].title_leader, "gift3title");
    mutate_gift_on_parse=1; buy_council_vote(1,2,1,0);
    assert(Popup::exec_calls==0 && count("xpops")==1);
    assert(find("parse_says",0).text=="gift4name");
    assert(find("parse_says",1).text=="gift3title");
    assert(PlayersData[2].unk_30[1]==3);

    // A successful network BUYALREADY callback records the original faction
    // as gift faction for a non-government proposal, then synchronizes OTHER.
    reset(); PlayersData[1].diplo_treaties[2]=2; PlayersData[2].diplo_treaties[1]=0;
    CouncilWindow.leader_label_kind_[2]=4; IsMultiplayerNet=1;
    buy_council_vote(1,2,1,4);
    assert(count("xpop")==1 && count("sync")==1 && find("sync").a[0]==2);
    assert(PlayersData[2].unk_29[1]==4 && PlayersData[2].unk_30[1]==1);

    // Treaty refusal and too-poor margin are independent early exits.
    reset(); PlayersData[2].diplo_treaties[1]=0x10; buy_council_vote(1,2,1,0);
    assert(Popup::exec_calls==0 && count("message")==0); assert_native_popup_dtor();
    reset(); want_result=-5; buy_council_vote(1,2,1,0);
    assert(Popup::exec_calls==0 && count("xpops")==1); assert_native_popup_dtor();

    // The treaty override zeroes the vote-weight component but still reaches
    // the modal price/selection flow; cancel leaves stored vote untouched.
    reset(); PlayersData[2].diplo_treaties[1]=0x2000001; Popup::result=0;
    PlayersData[2].unk_29[1]=44; PlayersData[2].unk_30[1]=4;
    PlayersData[2].diplo_treaties[4]=0x30; buy_council_vote(1,2,1,3);
    assert(Popup::exec_calls==1 && PlayersData[2].unk_29[1]==44);

    // Neutral faction uses the short popup but then follows the shared vote
    // commit/redraw tail without constructing the menu.
    reset(); PlayersData[1].energy_reserves=0; PlayersData[1].unk_13=0;
    PlayersData[1].current_num_bases=0; PlayersData[0].unk_13=0;
    PlayersData[0].current_num_bases=0; buy_council_vote(1,0,0,4);
    assert(Popup::exec_calls==0 && count("xpops")==1);
    assert(PlayersData[0].unk_29[0]==4 && PlayersData[0].unk_30[0]==4);
    assert(CouncilWindow.last_set==std::make_pair(0,4));

    // Proposal 0's self-choice refusal is separate from the generic -3 gate.
    reset(); want_result=-1; buy_council_vote(1,2,0,2);
    assert(Popup::exec_calls==0 && count("xpops")==1 && count("message")==0);

    // The proposal-zero min-margin path evaluates an accepted/tied candidate
    // twice. A changing result from the second native call controls the gate.
    reset(); eligible_mask=(1 << 3); wants_results={10,0,20};
    buy_council_vote(1,2,0,5);
    assert(wants_args.size()==3);
    assert((wants_args[0]==std::array<int,3>{2,0,5}));
    assert((wants_args[1]==std::array<int,3>{2,0,3}));
    assert((wants_args[2]==std::array<int,3>{2,0,3}));
    assert(count("xpops")==1 && Popup::exec_calls==0);
    return 0;
}
