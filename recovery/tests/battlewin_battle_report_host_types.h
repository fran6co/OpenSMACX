#ifndef BATTLE_REPORT_HOST_TYPES_H
#define BATTLE_REPORT_HOST_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#define __cdecl
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef int BOOL;

struct RECT { int left, top, right, bottom; };
struct BoxSpriteParams { int unused; };
struct Events {
    std::vector<std::string> values;
    void add(const char *s) { values.push_back(s); }
};
static Events test_events;
static int font_ctor_count;
static int font_dtor_count;
static int stats_call_count;
static int morale_call_count;
static int morale_faction_args[4];

struct Font {
    int style;
    Font() : style(-1) { ++font_ctor_count; }
    ~Font() { ++font_dtor_count; }
    int init(const char *, int size, int style) {
        this->style = style;
        char buf[64];
        sprintf(buf, "font:%d:%d", size, style);
        test_events.add(buf);
        return 1;
    }
};
struct Time {
    void pulse(void (__cdecl *)(int), int, int, int) {
        test_events.add("time_pulse");
    }
};

struct Buffer {
    void box_sprite(RECT *, BoxSpriteParams *) { test_events.add("box"); }
    int set_font(Font *a, Font *, Font *, Font *) {
        char buf[32];
        sprintf(buf, "set_font:%d", a ? a->style : -1);
        test_events.add(buf); return 1;
    }
    void set_text_color(int, int, int, int) { test_events.add("color"); }
    void set_clip(RECT *) { test_events.add("clip"); }
    void restore_clip() { test_events.add("restore_clip"); }
    int write_cent_l(char *s, int, int, int, int) {
        test_events.values.push_back(std::string("write:") + s); return 0;
    }
    int wrap_cent(char *s, RECT *r) {
        char buf[128];
        sprintf(buf, "wraprect:%d:%d:%d:%d:%s", r->left, r->top,
                r->right, r->bottom, s);
        test_events.add(buf); return 0;
    }
    int wrap_cent(char *s, int, int, int) {
        test_events.values.push_back(std::string("wrapcent:") + s); return 0;
    }
    int wrap(char *s, int x, int y, int width) {
        char buf[256];
        sprintf(buf, "wrap:%d:%d:%d:%s", x, y, width, s);
        test_events.add(buf); return 0;
    }
};
struct MainInterface : Buffer {
    void redraw_complete() { test_events.add("redraw_complete"); }
    void soft_update() { test_events.add("soft_update"); }
    void update(void *) { test_events.add("update"); }
};
static MainInterface MainInterfaceGlobal;

struct Sprite {
    const char *tag;
    Sprite() : tag("mindworm") {}
    int draw(Buffer *, int transparent, int x, int y, int, int) {
        char buf[80];
        sprintf(buf, "sprite_draw:%s:%d:%d:%d", tag, transparent, x, y);
        test_events.add(buf); return 1;
    }
    int width() const { return 12; }
    int height() const { return 10; }
    unsigned char transparent_index() const { return 0; }
};
struct FactionArt { Sprite council_small; };
static Sprite g_BATTLE_MIND_WORM_SPRITE, g_BATTLE_ISLE_DEEP_SPRITE,
    g_BATTLE_LOCUSTS_CHIRON_SPRITE, g_BATTLE_FUNGAL_TOWER_SPRITE,
    g_BATTLE_SPORE_LAUNCHER_SPRITE, g_BATTLE_SEALURK_SPRITE;
static FactionArt FactionsArt[8];

struct Veh { short proto_id; unsigned char faction_id; unsigned char year_end_lurking; };
struct VehPrototype {
    char veh_name[32];
    unsigned char chassis_id, weapon_id, armor_id, reactor_id;
    unsigned char plan;
};
struct RulesWeapon { char *name; signed char offense_rating; };
struct RulesArmor { char *name; signed char defense_rating; };
struct RulesChassis { unsigned char triad; };
struct RulesBasic { unsigned int psi_combat_ratio_atk[3], psi_combat_ratio_def[3]; };
struct Player {
    int noun_gender;
    BOOL is_noun_plural;
    char adj_name_faction[128];
    char formal_name_faction[40];
    char noun_faction[64];
};
struct Console { int faction_id_; };

static int IfaceIdToSlotStorage[30];
static int ActiveIfaceIdStorage;
static int WinScreenWidth = 800;
static int MessageWinTextColor = 7;
static int ChatItemIdTable[8];
static int ChatShadowColors[8];
static const char *DefaultFontFace = "Arial";
static BoxSpriteParams InterfaceBoxStyles[1];
static Veh Vehs[2049];
static VehPrototype VehPrototypes[512];
static RulesWeapon Weapon[26];
static RulesArmor Armor[14];
static RulesChassis Chassis[9];
static RulesBasic Rules;
static Player Players[8];
static Console ConsoleGlobal;
static unsigned int GameState;
static unsigned int Language;
static int GenderDefault;
static BOOL PluralityDefault;
static char StringTemp[1032];
static char BufferJoinSpace[] = " ";
static unsigned int VehBasicBattleMorale[2] = {2, 2};
static unsigned int VehBattleModCount[2];
static int VehBattleModifier[8];
static char VehBattleDisplay[8][80];
static int MapWinCaptionScale = 4;
static int MapWinCaptionDivisor = 5;
static char *MessageName = "message";

struct StringTableType {
    char *get(int id) {
        if (id == 101) return const_cast<char *>("WeaponLabel");
        if (id == 202) return const_cast<char *>("ArmorLabel");
        return MessageName;
    }
};
static StringTableType StringTable;

int range(int x, int low, int high) {
    return x < low ? low : (x > high ? high : x);
}
Font *find_font(int, int) { static Font cached; return &cached; }
void say_stats_3(char *out, int) { ++stats_call_count; strcpy(out, "stats"); }
void say_morale(int, int faction_id_vs_native) {
    if (morale_call_count < 4)
        morale_faction_args[morale_call_count] = faction_id_vs_native;
    ++morale_call_count;
    strcpy(StringTemp, "morale");
}
int offense_proto(int, int, BOOL) { return 3; }
int armor_proto(int, int, BOOL) { return 4; }
int veh_draw(Buffer *, int id, int x, int y, int flags, int, int) {
    char buf[96];
    sprintf(buf, "veh_draw:%d:%d:%d:%d", id, x, y, flags);
    test_events.add(buf); return 1;
}
void do_non_input() { test_events.add("do_non_input"); }
void __cdecl teardown_00422ec0() {}

char *label_get(int offset) {
    static char labels[512][16];
    sprintf(labels[offset & 511], "L%d", offset * 4);
    return labels[offset & 511];
}
int _itoa(int value, char *out, int radix) {
    if (radix == 10) sprintf(out, "%d", value);
    else sprintf(out, "%x", static_cast<unsigned int>(value));
    return 0;
}

static int union_rect_calls;
BOOL UnionRect(RECT *out, const RECT *a, const RECT *b) {
    ++union_rect_calls;
    out->left = a->left < b->left ? a->left : b->left;
    out->top = a->top < b->top ? a->top : b->top;
    out->right = a->right > b->right ? a->right : b->right;
    out->bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
    return 1;
}

class BattleWin {
public:
    void battle_report(int, int, int, int, int);
    Time time_;
    RECT battle_rects_[4];
};

#endif
