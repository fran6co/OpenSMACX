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
static void set_rect(RECT &rect, LONG left, LONG top, LONG right, LONG bottom) {
    rect.left=left; rect.top=top; rect.right=right; rect.bottom=bottom;
}
struct Font { int id; Font() : id(0) {} };
struct BoxSpriteParams { int id; };
struct ConsoleType { int faction_id_; };
static ConsoleType ConsoleGlobal={3};
struct Event {
    std::string name; int a, b, c, d; std::string text; const RECT *rect;
};
static std::vector<Event> events;
class NewTechWin;
static NewTechWin *callback_win=0;
static int callback_mutations=0;
static int font_calls=0, color_calls=0, category_calls=0, mandate_calls=0;
static void after_set_font(Font *font);
static void after_set_text_color(int color);
static void log(const char *name, int a=0, int b=0, int c=0, int d=0,
                const char *text="", const RECT *rect=0) {
    Event e; e.name=name; e.a=a; e.b=b; e.c=c; e.d=d; e.text=text; e.rect=rect;
    events.push_back(e);
}

class Buffer {
public:
    RECT work;
    void clear_links() { log("clear"); ConsoleGlobal.faction_id_=2; }
    void box_sprite(RECT *r, BoxSpriteParams *style) {
        log("box", style->id, r->left, r->top, r->right-r->left, "", r);
    }
    void set_font(Font *font, int, int, int) {
        log("font", font->id); ++font_calls; after_set_font(font);
    }
    void set_text_color(int c, int, int, int) {
        log("color", c); ++color_calls; after_set_text_color(c);
    }
    void write_cent_l(char *s, RECT *r, unsigned int n) {
        char text[1200]; std::memcpy(text, s, n); text[n]=0;
        log("text", r->left, r->top, r->right, r->bottom, text, r);
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
    void draw_mono(Buffer *, int, int x, int y, int color, int w, int h);
};
static Sprite g_TECH_ICON_SPRITES[MaxTechnologyNum];
class NewTechWin : public GraphicWin {
public:
    uint32_t field_A14_, field_A18_, field_A1C_, field_A20_;
    RECT main_rect_, title_rect_, category_rect_, row_rects_[4];
    RECT label_rects_[4], bottom_rect_;
    uint32_t field_AF4_,field_AF8_,field_AFC_,field_B00_;
    uint32_t field_B24_,field_B28_;
    void on_redraw();
};
static BoxSpriteParams InterfaceStandardPopupBoxStyle={9};
static BoxSpriteParams InterfaceBoxStyles[7]={{0},{1},{2},{3},{4},{5},{6}};
static char string_temp_storage[1032];
static char *StringTemp = string_temp_storage;
static const char NewTechCentempSuffix[] = " ";
static const int TECH_CENTEMP=58;
struct PlayerData { int theory_of_everything; };
static PlayerData PlayersData[8]={{0},{0},{9},{7},{0},{0},{0},{0}};
static Font fake_font;
static Font *find_font(int id, int) { fake_font.id=id; return &fake_font; }
static char *label_get(int id) {
    static char texts[4][32];
    std::sprintf(texts[id & 3], "label-%d", id);
    return texts[id & 3];
}
static char *CharUpperA(char *text) {
    for (char *p=text; *p; ++p) *p=(char)std::toupper((unsigned char)*p);
    return text;
}
static int tech_category(int id) {
    log("category", id); ++category_calls; return id % 5;
}
static int mandate_color(int c) {
    ++mandate_calls;
    if(callback_mutations && mandate_calls==2 && callback_win) callback_win->field_B24_=59;
    return 100+c;
}
static void say_tech(char *out, int id, int) {
    log("say", id);
    if(callback_mutations && callback_win) callback_win->field_B24_=58;
    std::sprintf(out, "tech-%d", id);
}
static char * _itoa(int v, char *out, int) { std::sprintf(out, "%d", v); return out; }

static void after_set_font(Font *font) {
    if(callback_mutations && font->id==0x28 && callback_win)
        callback_win->field_B24_=22;
}
static void after_set_text_color(int color) {
    if(callback_mutations && color!=0xE1 && callback_win)
        callback_win->field_B24_=57;
}
void Sprite::draw_mono(Buffer *, int, int x, int y, int color, int w, int h) {
    log("icon", static_cast<int>(this-g_TECH_ICON_SPRITES), x, y, color, "");
    events.back().text = std::string("size-") + char('0'+w/100) + "-" + char('0'+h/100);
}

/* ACTUAL OWNER BODY */

static int count(const char *name) {
    int n=0; for (size_t i=0;i<events.size();++i) if(events[i].name==name)++n;
    return n;
}
static void initialize(NewTechWin &w, int tech, int other_faction) {
    w.work.left=1; w.work.top=2; w.work.right=300; w.work.bottom=400;
    set_rect(w.main_rect_,10,20,290,380);
    set_rect(w.title_rect_,13,23,287,45);
    set_rect(w.category_rect_,20,80,280,180);
    set_rect(w.row_rects_[0],13,50,287,75);
    set_rect(w.row_rects_[1],13,80,287,105);
    set_rect(w.row_rects_[2],13,110,287,135);
    set_rect(w.row_rects_[3],13,140,287,165);
    set_rect(w.label_rects_[0],13,30,287,60);
    set_rect(w.label_rects_[1],13,60,287,90);
    set_rect(w.label_rects_[2],13,90,287,120);
    set_rect(w.label_rects_[3],13,120,287,150);
    set_rect(w.bottom_rect_,13,150,287,180);
    w.field_AF4_=20; w.field_AF8_=40; w.field_AFC_=280; w.field_B00_=60;
    w.field_B24_=static_cast<uint32_t>(tech);
    w.field_B28_=static_cast<uint32_t>(other_faction);
}
int main(int argc, char **argv) {
    if(argc>1) {
        if(std::strcmp(argv[1], "reload")!=0) return 20;
        NewTechWin live; initialize(live, 11, 1);
        callback_win=&live; callback_mutations=1;
        live.on_redraw();
        if(count("category")!=2 || count("say")!=1 || count("icon")!=1) return 21;
        bool category_after_font=false, say_after_color=false;
        bool category_after_say=false, centemp_after_say=false;
        for(size_t i=0;i<events.size();++i) {
            if(events[i].name=="category" && events[i].a==22) category_after_font=true;
            if(events[i].name=="category" && events[i].a==58) category_after_say=true;
            if(events[i].name=="say" && events[i].a==57) say_after_color=true;
            if(events[i].name=="text" && events[i].text=="tech-57 7")
                centemp_after_say=true;
        }
        bool icon_after_color=false;
        for(size_t i=0;i<events.size();++i)
            if(events[i].name=="icon" && events[i].a==59 && events[i].d==103)
                icon_after_color=true;
        if(!category_after_font || !say_after_color || !category_after_say ||
           !centemp_after_say || !icon_after_color || live.field_B24_!=59)
            return 22;
        return 0;
    }
    (void)argv;
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
    const RECT *boxes[14]; const RECT *texts[6];
    int box_index=0, text_index=0;
    for(size_t i=0;i<events.size();++i) {
        if(events[i].name=="box") boxes[box_index++]=events[i].rect;
        if(events[i].name=="text") texts[text_index++]=events[i].rect;
    }
    if(box_index!=14 || text_index!=6) return 5;
    if(boxes[0]!=boxes[2] || boxes[0]==&win.work) return 6;
    if(boxes[1]!=&win.main_rect_ || boxes[3]!=&win.title_rect_) return 7;
    for(int i=0;i<4;++i) {
        if(boxes[4+i]!=&win.label_rects_[i] || texts[1+i]!=&win.label_rects_[i]) return 8;
        if(boxes[8+i]!=&win.row_rects_[i]) return 9;
    }
    if(texts[0]!=&win.title_rect_ || texts[5]!=boxes[0]) return 10;
    if(boxes[12]!=&win.category_rect_ || boxes[13]!=&win.bottom_rect_) return 11;
    return 0;
}
