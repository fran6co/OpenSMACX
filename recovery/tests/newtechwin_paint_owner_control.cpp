#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

typedef long LONG;
typedef unsigned int uint32_t;
static const int MaxTechnologyNum=89;
typedef char *LPSTR;
struct RECT { LONG left, top, right, bottom; };
struct Font {};
struct BoxSpriteParams { int id; };
struct ConsoleType { int faction_id_; };
static ConsoleType ConsoleGlobal={3};
struct Event { std::string name; int a, b, c, d; std::string text; };
static std::vector<Event> events;
static void log(const char *name, int a=0, int b=0, int c=0, int d=0,
                const char *text="") {
    Event e; e.name=name; e.a=a; e.b=b; e.c=c; e.d=d; e.text=text;
    events.push_back(e);
}

class Buffer {
public:
    RECT work;
    void clear_links() { log("clear"); ConsoleGlobal.faction_id_=2; }
    void box_sprite(RECT *r, BoxSpriteParams *style) {
        log("box", style->id, r->left, r->top, r->right-r->left);
    }
    void set_font(Font *, int, int, int) { log("font"); }
    void set_text_color(int c, int, int, int) { log("color", c); }
    void write_cent_l(char *s, RECT *r, unsigned int n) {
        char text[1200]; std::memcpy(text, s, n); text[n]=0;
        log("text", r->left, r->top, r->right, r->bottom, text);
    }
    int text_line_height() { return 12; }
};
class Win {};
class GraphicWin : public Win, public Buffer {
public:
    RECT *buffer_working_rect() { return &work; }
};
struct Sprite {
    int transparent_index() const { return 13; }
    void draw_mono(Buffer *, int, int x, int y, int color, int w, int h) {
        log("icon", x, y, color, w, "");
        events.back().d = h;
    }
};
class NewTechWin : public GraphicWin {
public:
    uint32_t field_A14_, field_A18_, field_A1C_, field_A20_;
    uint32_t field_A24_,field_A28_,field_A2C_,field_A30_;
    uint32_t field_A34_,field_A38_,field_A3C_,field_A40_;
    uint32_t field_A44_,field_A48_,field_A4C_,field_A50_;
    uint32_t field_A54_,field_A58_,field_A5C_,field_A60_;
    uint32_t field_A64_,field_A68_,field_A6C_,field_A70_;
    uint32_t field_A74_,field_A78_,field_A7C_,field_A80_;
    uint32_t field_A84_,field_A88_,field_A8C_,field_A90_;
    uint32_t field_A94_,field_A98_,field_A9C_,field_AA0_;
    uint32_t field_AA4_,field_AA8_,field_AAC_,field_AB0_;
    uint32_t field_AB4_,field_AB8_,field_ABC_,field_AC0_;
    uint32_t field_AC4_,field_AC8_,field_ACC_,field_AD0_;
    uint32_t field_AD4_,field_AD8_,field_ADC_,field_AE0_;
    uint32_t field_AF4_,field_AF8_,field_AFC_,field_B00_;
    uint32_t field_B24_,field_B28_;
    void on_redraw();
};
static BoxSpriteParams InterfaceStandardPopupBoxStyle={9};
static BoxSpriteParams InterfaceBoxStyles[7]={{0},{1},{2},{3},{4},{5},{6}};
static Sprite g_TECH_ICON_SPRITES[MaxTechnologyNum];
static char string_temp_storage[1032];
static char *StringTemp = string_temp_storage;
static const char NewTechCentempSuffix[] = " ";
static const int TECH_CENTEMP=58;
struct PlayerData { int theory_of_everything; };
static PlayerData PlayersData[8]={{0},{0},{9},{7},{0},{0},{0},{0}};
static Font fake_font;
static Font *find_font(int, int) { return &fake_font; }
static char *label_get(int id) {
    static char texts[4][32];
    std::sprintf(texts[id & 3], "label-%d", id);
    return texts[id & 3];
}
static char *CharUpperA(char *text) {
    for (char *p=text; *p; ++p) *p=(char)std::toupper((unsigned char)*p);
    return text;
}
static int tech_category(int id) { return id % 5; }
static int mandate_color(int c) { return 100+c; }
static void say_tech(char *out, int id, int) { std::sprintf(out, "tech-%d", id); }
static char * _itoa(int v, char *out, int) { std::sprintf(out, "%d", v); return out; }

/* ACTUAL OWNER BODY */

static int count(const char *name) {
    int n=0; for (size_t i=0;i<events.size();++i) if(events[i].name==name)++n;
    return n;
}
static void initialize(NewTechWin &w, int tech, int other_faction) {
    w.work.left=1; w.work.top=2; w.work.right=300; w.work.bottom=400;
    w.field_A24_=10; w.field_A28_=20; w.field_A2C_=290; w.field_A30_=380;
    w.field_A34_=13; w.field_A38_=23; w.field_A3C_=287; w.field_A40_=45;
    w.field_A44_=20; w.field_A48_=80; w.field_A4C_=280; w.field_A50_=180;
    w.field_A54_=13; w.field_A58_=50; w.field_A5C_=287; w.field_A60_=75;
    w.field_A64_=13; w.field_A68_=80; w.field_A6C_=287; w.field_A70_=105;
    w.field_A74_=13; w.field_A78_=110; w.field_A7C_=287; w.field_A80_=135;
    w.field_A84_=13; w.field_A88_=140; w.field_A8C_=287; w.field_A90_=165;
    w.field_A94_=13; w.field_A98_=30; w.field_A9C_=287; w.field_AA0_=60;
    w.field_AA4_=13; w.field_AA8_=60; w.field_AAC_=287; w.field_AB0_=90;
    w.field_AB4_=13; w.field_AB8_=90; w.field_ABC_=287; w.field_AC0_=120;
    w.field_AC4_=13; w.field_AC8_=120; w.field_ACC_=287; w.field_AD0_=150;
    w.field_AD4_=13; w.field_AD8_=150; w.field_ADC_=287; w.field_AE0_=180;
    w.field_AF4_=20; w.field_AF8_=40; w.field_AFC_=280; w.field_B00_=60;
    w.field_B24_=static_cast<uint32_t>(tech);
    w.field_B28_=static_cast<uint32_t>(other_faction);
}
int main(int argc, char **argv) {
    (void)argc; (void)argv;
    NewTechWin win; initialize(win, 58, 1); win.on_redraw();
    if (events.empty() || events[0].name!="clear") return 1;
    if (count("box")!=14 || count("text")!=6 || count("icon")!=1) return 2;
    bool faction_label=false, centemp=false;
    for(size_t i=0;i<events.size();++i) {
        if(events[i].name=="text" && events[i].text=="LABEL-542") faction_label=true;
        if(events[i].name=="text" && events[i].text=="tech-58 7") centemp=true;
    }
    if(!faction_label || !centemp) return 3;
    if(events.back().name!="box" || events.back().a!=0) return 4;
    return 0;
}
