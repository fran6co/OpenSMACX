#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct RECT { int left, top, right, bottom; };
typedef char *LPSTR;
typedef const char *LPCSTR;

static char events[8192];
static int event_count;
static char label_storage[1024][48];
static char string_temp[1024];
#define StringTemp string_temp
static int throw_at_box;
static int box_calls;
static char rendered[8][1024];
static int rendered_count;
static char wrapped[1024];
static int updated_original_rect;
static RECT original_panel = { 10, 20, 210, 140 };

static void event(const char *name) {
    if (event_count && event_count < (int)sizeof(events)-1) events[event_count++] = '|';
    while (*name && event_count < (int)sizeof(events)-1) events[event_count++] = *name++;
    events[event_count] = 0;
}

struct BoxSpriteParams { int id; };
BoxSpriteParams InterfaceBoxStyles[8];

class Font {
public:
    Font() { event("font.ctor"); }
    ~Font() { event("font.dtor"); }
    int init(LPCSTR face, int height, int style) {
        if (!face || height != 0x12 || style != 1) abort();
        event("font.init"); return 1;
    }
    int line_advance() const { return 12; }
};
static const char *DefaultFontFace = "default";

class Buffer {
    RECT clip_;
public:
    Buffer() { clip_.left=1; clip_.top=2; clip_.right=300; clip_.bottom=200; }
    const RECT &current_clip() const { return clip_; }
    void set_clip(RECT *r) { if (!r) abort(); event("clip"); clip_ = *r; }
    int box_sprite(RECT *, BoxSpriteParams *p) {
        event(p == &InterfaceBoxStyles[3] ? "box3" : "box0");
        ++box_calls;
        if (throw_at_box == box_calls) throw 17;
        return 1;
    }
    void set_text_color(int a,int b,int c,int d) {
        if (a != 0xE0 || b != -1 || c != 1 || d != 1) abort();
        event("color");
    }
    void set_font(Font *, int a,int b,int c) {
        if (a||b||c) abort();
        event("setfont");
    }
    int write_cent_l(char *s, RECT *, int) {
        if (rendered_count < 8) {
            strncpy(rendered[rendered_count], s, sizeof(rendered[rendered_count])-1);
            rendered[rendered_count][sizeof(rendered[rendered_count])-1]=0;
            ++rendered_count;
        }
        event("write"); return 1;
    }
    int wrap_height(char *, int) { event("wrap_height"); return 18; }
    int wrap_cent(char *s, RECT *) {
        strncpy(wrapped,s,sizeof(wrapped)-1); wrapped[sizeof(wrapped)-1]=0;
        event("wrap_cent"); return 1;
    }
};

struct MainInterfaceMock : Buffer {
    void soft_update(RECT *r) {
        updated_original_rect = r && r->left==original_panel.left &&
            r->top==original_panel.top && r->right==original_panel.right &&
            r->bottom==original_panel.bottom;
        event("update");
    }
};
MainInterfaceMock MainInterfaceGlobal;

struct VehPrototype {
    uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id;
    int8_t unk_1;
    uint8_t obsolete_factions;
    uint16_t flags;
};
enum { PROTO_ACTIVE = 1 };
VehPrototype VehPrototypes[512];
struct PlayerData { uint8_t proto_id_active[512]; uint8_t proto_id_queue[512]; };
PlayerData PlayersData[8];

static char *label_get(int index) {
    if (index < 0 || index >= 1024) abort();
    return label_storage[index];
}
static char *CharUpperA(char *s) {
    for (char *p=s; *p; ++p) if (*p >= 'a' && *p <= 'z') *p -= 'a' - 'A';
    event("upper"); return s;
}
static char *_itoa(int n, char *out, int radix) {
    if (radix != 10) abort();
    sprintf(out, "%d", n);
    return out;
}

class DesignWin {
public:
    RECT info_panel_rect_;
    uint32_t factionID_, protoID_;
    uint8_t protoChassisType_, protoWeaponType_, protoArmorType_, protoReactorType_;
    uint32_t protoAbilityFlags_;
    void draw_info();
};

// @@ACTUAL_DESIGN_INFO@@

static void reset() {
    event_count=0; events[0]=0; rendered_count=0; wrapped[0]=0; box_calls=0;
    throw_at_box=0; updated_original_rect=0;
    for (int i=0;i<512;++i) {
        memset(&VehPrototypes[i], 0, sizeof(VehPrototypes[i]));
        memset(&PlayersData[0].proto_id_active[i],0,1);
        memset(&PlayersData[0].proto_id_queue[i],0,1);
    }
    for (int i=0;i<1024;++i) sprintf(label_storage[i], "L%03d", i);
    strcpy(label_storage[0xE4C/4], "title");
    strcpy(label_storage[0xE44/4], "exact");
    strcpy(label_storage[0xE50/4], "selected");
    strcpy(label_storage[0xE54/4], "changed");
    strcpy(label_storage[0xC38/4], "none");
    strcpy(label_storage[0xC34/4], "unit");
    strcpy(label_storage[0xE48/4], "design");
    strcpy(string_temp, "");
    InterfaceBoxStyles[0].id=0; InterfaceBoxStyles[3].id=3;
}
static DesignWin make_win(unsigned id) {
    DesignWin w;
    w.info_panel_rect_=original_panel; w.factionID_=0; w.protoID_=id;
    w.protoChassisType_=1; w.protoWeaponType_=2; w.protoArmorType_=3;
    w.protoReactorType_=4; w.protoAbilityFlags_=5;
    return w;
}
static void activate(int id, int category=0) {
    VehPrototype &p=VehPrototypes[id]; p.flags=PROTO_ACTIVE;
    p.chassis_id=1; p.weapon_id=2; p.armor_id=3; p.reactor_id=4;
    p.ability_flags=5; p.unk_1=(int8_t)category;
}
static int contains(const char *haystack, const char *needle) {
    return strstr(haystack,needle)!=0;
}

int main() {
    reset();
    DesignWin low=make_win(2); activate(2); low.draw_info();
    if (rendered_count!=1 || strcmp(rendered[0],"title") || strcmp(wrapped,"exact") ||
        !updated_original_rect || !contains(events,"font.ctor|clip|font.init|box3|box0") ||
        !contains(events,"write|wrap_height|wrap_cent|update|clip|font.dtor")) {
        fprintf(stderr,"case1 render=%d [%s][%s] update=%d events=%s\n",rendered_count,
            rendered[0],rendered[1],updated_original_rect,events); return 1;
    }

    reset();
    DesignWin high=make_win(0x80); activate(0x80); high.draw_info();
    if (rendered_count!=1 || strcmp(wrapped,"selected")) { fprintf(stderr,"case2 [%s]\n",wrapped); return 2; }

    reset();
    DesignWin scan=make_win(0x80); activate(0x80, 1);
    // Force selected-field mismatch. Two eligible faction slots with matching category
    // and 0/0 counts must both store, leaving the later id selected.
    scan.protoWeaponType_=9;
    for (int i=0;i<64;++i) activate(i, 2);
    activate(0,1); activate(1,1);
    VehPrototypes[0].obsolete_factions=1; VehPrototypes[1].obsolete_factions=1;
    scan.draw_info();
    if (scan.protoID_!=1 || rendered_count!=1 || strcmp(wrapped,"changed")) { fprintf(stderr,"case3 id=%u [%s]\n",scan.protoID_,wrapped); return 3; }

    reset();
    DesignWin fallback=make_win(0x80); // inactive selected id
    fallback.draw_info();
    if (rendered_count!=1 || strcmp(wrapped,"UNIT (DESIGN #1)") ||
        !contains(events,"write|upper|wrap_height|wrap_cent|update|clip|font.dtor")) {
        fprintf(stderr,"case4 [%s] events=%s\n",wrapped,events); return 4;
    }

    reset();
    DesignWin unwind=make_win(2); activate(2); throw_at_box=1;
    try { unwind.draw_info(); return 5; }
    catch (int) { if (!contains(events,"font.dtor")) return 6; }

    puts("draw_info body host control: PASS");
    return 0;
}
