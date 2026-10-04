#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

struct RECT { int left, top, right, bottom; };
struct Font { int unk_1_, line_height_, height_; int width(const char *s) const { return (int)strlen(s) * 8; } };
struct RulesWeapon { const char *name; const char *name_short; int8_t offense_rating;
    int8_t icon; uint8_t mode, cost; int16_t preq_tech, padding; };
struct RulesArmor { const char *name; const char *name_short; int8_t defense_rating;
    uint8_t mode, cost, padding1; int16_t preq_tech, padding2; };
struct RulesReactor { const char *name; const char *name_short;
    int16_t preq_tech; uint16_t power; };
struct VehPrototype { int chassis_id, weapon_id, armor_id, reactor_id; uint32_t ability_flags; uint16_t flags; char veh_name[32]; };
struct RulesChassis { const char *name; };
struct CaviarData {};
CaviarData g_VW00_CAVIARDATA;
static CaviarData *last_model_definition;
struct DesignWin;
static DesignWin *active_window;
static int selection_after_constructor;
static int selection_during_detail_label;
static void mutate_detail_selection(int);
struct Console { int faction_id_; };
static Console ConsoleGlobal;
static int DesignAutoBuildActive;
static const int MaxVehProtoNum = 512;
static const uint16_t PROTO_ACTIVE = 1;
static int MapWinCaptionScale = -1, MapWinCaptionDivisor = -1;

static int clip_calls, copy_calls, render_calls, multi_render_calls;
static int weapon_construct_calls, armor_construct_calls, reactor_construct_calls;
static int fake_calls, vehicle_draw_calls, name_proto_calls;
static int vehicle_construct_calls, stats_calls;
static int last_scale, last_render_kind, last_vehicle, last_vehicle_x, last_vehicle_y;
static int last_render_x, last_render_y;
static int last_weapon, last_armor, last_reactor, last_ability;
static int last_written_length, last_wrapped, written_calls;
static int last_color_first, last_color_second, color_calls;
static int color_first[16], color_second[16];
static RECT last_write_rect, last_wrap_rect;
static RECT font_write_rects[16];
static int font_write_count;
static int last_weapon_chassis, last_weapon_faction, last_weapon_id;
static int last_component_faction, last_component_chassis, last_component_weapon;
static int mutate_weapon_mode_in_constructor;
static int mutate_armor_after_constructor;
static int mutate_palette_in_constructor;
static char last_text[256], all_text[2048];
static char scratch[1032];
static char label_text[256];
static unsigned char ArmorTransparentColorTables[24][256];
int TextColourTable[195];

struct Strings { char *get(int); };
static Strings StringTable;
static char event_log[128];

static void note_event(char event) {
    const size_t length = strlen(event_log);
    if (length + 1 < sizeof(event_log)) {
        event_log[length] = event;
        event_log[length + 1] = '\0';
    }
}

static void reset_observations() {
    clip_calls = copy_calls = render_calls = multi_render_calls = 0;
    weapon_construct_calls = armor_construct_calls = reactor_construct_calls = 0;
    fake_calls = vehicle_draw_calls = name_proto_calls = 0;
    vehicle_construct_calls = stats_calls = 0;
    last_scale = last_render_kind = last_vehicle = last_vehicle_x = last_vehicle_y = -1;
    last_render_x = last_render_y = -1;
    last_weapon = last_armor = last_reactor = last_ability = -1;
    last_written_length = last_wrapped = written_calls = 0;
    last_color_first = last_color_second = color_calls = 0;
    font_write_count = 0;
    last_weapon_chassis = last_weapon_faction = last_weapon_id = -1;
    last_component_faction = last_component_chassis = last_component_weapon = -1;
    mutate_weapon_mode_in_constructor = 0;
    selection_after_constructor = selection_during_detail_label = 0;
    last_model_definition = 0;
    mutate_armor_after_constructor = mutate_palette_in_constructor = 0;
    last_text[0] = all_text[0] = scratch[0] = label_text[0] = event_log[0] = '\0';
}

static void record_text(const char *text) {
    strncpy(last_text, text, sizeof(last_text) - 1);
    last_text[sizeof(last_text) - 1] = '\0';
    if (all_text[0] != '\0') strcat(all_text, "|");
    strcat(all_text, text);
    ++written_calls;
}

static char *label_get(int id) {
    mutate_detail_selection(id);
    strcpy(label_text, id == 0x200 / 4 ? "weapon-label-a" :
           id == 0x204 / 4 ? "weapon-label-b" :
           id == 0x1f8 / 4 ? "armor-caption" :
           id == 0x13c / 4 ? "reactor-caption" :
           id == 0x584 / 4 ? "planet-buster" :
           id == 0x64 / 4 ? "none" :
           id == 0xA6C / 4 ? "defense" : "label");
    return label_text;
}

static char *CharUpperA(char *text) {
    for (char *p = text; *p != '\0'; ++p) *p = (char)toupper((unsigned char)*p);
    return text;
}

static char *_itoa(int value, char *out, int radix) {
    if (radix == 10) sprintf(out, "%d", value);
    else sprintf(out, "%x", (unsigned)value);
    return out;
}

struct Buffer {
    void set_clip(RECT *) { note_event('L'); ++clip_calls; }
    void copy(Buffer *, RECT *) { note_event('P'); ++copy_calls; }
    void set_text_color(int first, int second, int, int) {
        last_color_first = first; last_color_second = second; ++color_calls;
        if (color_calls <= 16) {
            color_first[color_calls - 1] = first;
            color_second[color_calls - 1] = second;
        }
        note_event('T');
    }
    void set_font(Font *, int, int, int) { note_event('F'); }
    void write_cent_l(Font *, char *text, RECT *area, int length) {
        note_event('W');
        last_write_rect = *area;
        if (font_write_count < 16) font_write_rects[font_write_count++] = *area;
        ++last_written_length; last_written_length = length;
        record_text(text);
    }
    void write_cent_l(char *text, RECT *area, int length) {
        note_event('W');
        last_write_rect = *area;
        ++last_written_length; last_written_length = length;
        record_text(text);
    }
    int wrap_cent(char *text, RECT *area) {
        note_event('w');
        last_wrap_rect = *area;
        ++last_wrapped;
        record_text(text);
        return area->top + ((int)strlen(text) / 12 + 1) * 12;
    }
    int wrap_height(char *text, int) {
        note_event('h'); return (int)strlen(text) > 24 ? 24 : 12;
    }
};

struct Caviar {
    void add_object(CaviarData *definition, int) { last_model_definition = definition; }
    void reset_objects() { note_event('R'); }
    void set_scaling(float value) { last_scale = (int)(value * 1000.0f + 0.5f); }
    void set_scene_rotation(float, float, float) {}
    void render(Buffer *, int x, int y, int) {
        note_event('V');
        ++render_calls; last_render_kind = 1; last_render_x = x; last_render_y = y;
    }
    void render_multi_table_dest(Buffer *, int x, int y, int, unsigned char *, int) {
        note_event('V');
        ++multi_render_calls; last_render_kind = 2;
        last_render_x = x; last_render_y = y;
    }
};

struct DesignWin : Buffer {
    RECT weapon_panel_rect_, armor_panel_rect_, reactor_panel_rect_, vehicle_panel_rect_;
    Buffer buffer_;
    Font font1_, font2_;
    Caviar caviar_;
    uint32_t factionID_, protoID_, protoChassisType_, protoWeaponType_;
    uint32_t protoArmorType_, protoAbilityFlags_, protoReactorType_;
    uint32_t protoAbilityID1_, protoAbilityID2_, field_CE8_;
    float scene_rotation_;
    char prototype_name_[32];
    RECT field_B2C_rect_;
    uint32_t field_B3C_, field_B40_, field_B44_, field_B48_;
    DesignWin() : factionID_(0), protoID_(0), protoChassisType_(0),
        protoWeaponType_(0), protoArmorType_(0), protoAbilityFlags_(0),
        protoReactorType_(0), protoAbilityID1_(0xffffffffu),
        protoAbilityID2_(0xffffffffu), field_CE8_(0), scene_rotation_(0.0f),
        field_B3C_(0), field_B40_(0), field_B44_(0),
        field_B48_(0) { prototype_name_[0] = '\0'; }
    void draw_weapon(int); void draw_armor(int); void draw_reactor(int); void draw_vehicle(int);
};

static RulesWeapon Weapon[26];
static RulesArmor Armor[14];
static RulesReactor Reactor[4];
static RulesChassis Chassis[64];
static VehPrototype VehPrototypes[512];
static void mutate_detail_selection(int id) {
    if (selection_during_detail_label && color_calls == 3 && id == 0x204 / 4) {
        active_window->protoWeaponType_ = selection_during_detail_label;
        selection_during_detail_label = 0;
    }
}
char *Strings::get(int id) {
    note_event('S');
    for (int index = 0; index < 26; ++index) {
        if (Weapon[index].name != 0 && id == static_cast<int>(
                reinterpret_cast<uintptr_t>(Weapon[index].name)))
            return const_cast<char *>(Weapon[index].name);
    }
    for (int index = 0; index < 14; ++index) {
        if (Armor[index].name != 0 && id == static_cast<int>(
                reinterpret_cast<uintptr_t>(Armor[index].name)))
            return const_cast<char *>(Armor[index].name);
    }
    for (int index = 0; index < 4; ++index) {
        if (Reactor[index].name != 0 && id == static_cast<int>(
                reinterpret_cast<uintptr_t>(Reactor[index].name)))
            return const_cast<char *>(Reactor[index].name);
    }
    return label_text;
}
static int vehdraw_construct_weapon(int faction, int chassis, int weapon, Caviar *) {
    note_event('c'); ++weapon_construct_calls; last_weapon = weapon;
    last_weapon_id = weapon;
    last_weapon_faction = faction; last_weapon_chassis = chassis;
    if (mutate_weapon_mode_in_constructor) Weapon[weapon].mode = 9;
    if (selection_after_constructor) {
        active_window->protoWeaponType_ = selection_after_constructor;
        active_window->weapon_panel_rect_.right = 200;
    }
    if (mutate_palette_in_constructor) TextColourTable[160] = 4242;
    return 0;
}
static int vehdraw_construct_armor(int faction, int chassis, int weapon, int armor, int, Caviar *) {
    note_event('c'); ++armor_construct_calls; last_armor = armor;
    last_component_faction = faction; last_component_chassis = chassis;
    last_component_weapon = weapon;
    if (mutate_armor_after_constructor) {
        Armor[armor].mode = 3;
        Armor[armor].defense_rating = 77;
    }
    if (mutate_palette_in_constructor) TextColourTable[160] = 4242;
    return 0;
}
static int vehdraw_construct_reactor(int faction, int chassis, int weapon, int reactor, uint32_t abilities, Caviar *) {
    note_event('c'); ++reactor_construct_calls; last_reactor = reactor; last_ability = (int)abilities;
    last_component_faction = faction; last_component_chassis = chassis;
    last_component_weapon = weapon;
    if (mutate_palette_in_constructor) TextColourTable[160] = 4343;
    return 0;
}
static int veh_fake(int prototype, int) { ++fake_calls; last_vehicle = prototype; return 2048; }
static int veh_draw(Buffer *, int vehicle, int x, int y, int, int, int) {
    ++vehicle_draw_calls; last_vehicle = vehicle; last_vehicle_x = x; last_vehicle_y = y; return 0;
}
static void vehdraw_construct_vehicle(int, int, int, int, int, uint32_t, Caviar *) {
    ++vehicle_construct_calls;
}
static void say_stats_3(char *text, int) { ++stats_calls; strcat(text, "STATS"); }
static void name_proto(char *text, int, int, int, int, int, int, int) {
    ++name_proto_calls; strcpy(text, "GENERATED VEHICLE");
}

#define StringTemp scratch
// @@ACTUAL_DESIGN_WEAPON@@

static int assertion_number;
static void require(bool condition) {
    ++assertion_number;
    if (!condition) { fprintf(stderr, "host control assertion %d failed; text=%s events=%s\n", assertion_number, all_text, event_log); abort(); }
}
static RECT rect(int left, int top, int right, int bottom) {
    RECT value; value.left = left; value.top = top;
    value.right = right; value.bottom = bottom; return value;
}

static void initialize(DesignWin &window) {
    reset_observations();
    active_window = &window;
    DesignAutoBuildActive = 0;
    ConsoleGlobal.faction_id_ = 7;
    for (int i = 0; i < 195; ++i) TextColourTable[i] = 1000 + i;
    window.weapon_panel_rect_ = rect(0, 0, 100, 100);
    window.armor_panel_rect_ = rect(0, 0, 100, 100);
    window.reactor_panel_rect_ = rect(0, 0, 100, 100);
    window.vehicle_panel_rect_ = rect(0, 0, 200, 140);
    window.font1_.unk_1_ = -1; window.font1_.line_height_ = 12; window.font1_.height_ = 10;
    window.font2_.unk_1_ = -1; window.font2_.line_height_ = 14; window.font2_.height_ = 12;
    window.factionID_ = 2; window.protoID_ = 3; window.protoChassisType_ = 4;
    window.protoWeaponType_ = 2; window.protoArmorType_ = 5;
    window.protoAbilityFlags_ = 0x8000; window.protoReactorType_ = 2;
    window.protoAbilityID1_ = 4; window.protoAbilityID2_ = 8;
    window.prototype_name_[0] = '\0';
    Weapon[2].name = "laser"; Weapon[2].offense_rating = 4; Weapon[2].mode = 1; Weapon[2].cost = 6;
    Weapon[3].name = "missile"; Weapon[3].offense_rating = 12; Weapon[3].mode = 9; Weapon[3].cost = 8;
    Armor[0].name = "none"; Armor[0].defense_rating = 0; Armor[0].padding1 = 0;
    Armor[5].name = "silksteel"; Armor[5].defense_rating = 3;
    Armor[5].mode = 1; Armor[5].padding1 = 1;
}

int main() {
    DesignWin window;
    initialize(window);
    window.draw_weapon(0);
    require(strcmp(all_text, "WEAPON-LABEL-B|LASER|weapon-label-b 4") == 0);
    require(last_scale == 150 && last_render_x == 50 && last_render_y == 50);
    require(last_write_rect.top == 88 && last_write_rect.bottom == 98);
    require(color_calls == 3 && color_first[0] == 1156 &&
            color_first[1] == 1158 && color_first[2] == 1160);

    initialize(window); Weapon[2].offense_rating = 99;
    window.draw_weapon(0);
    require(strcmp(last_text, "weapon-label-b *") == 0);
    initialize(window); Weapon[2].offense_rating = 98;
    window.draw_weapon(0);
    require(strcmp(last_text, "weapon-label-b 98") == 0);
    initialize(window); Weapon[2].offense_rating = -128;
    window.draw_weapon(0);
    require(strcmp(last_text, "weapon-label-b label") == 0);

    initialize(window); window.protoWeaponType_ = 0;
    window.draw_weapon(0);
    require(last_model_definition == &g_VW00_CAVIARDATA);
    require(weapon_construct_calls == 0 && last_scale == 150);

    initialize(window); selection_after_constructor = 3;
    window.draw_weapon(0);
    require(last_weapon_id == 2 && last_scale == 60);
    require(last_render_x == 100 && last_render_y == 50);
    require(strstr(all_text, "MISSILE") != 0);
    require(strcmp(last_text, "weapon-label-a 8") == 0);

    initialize(window); selection_during_detail_label = 3;
    window.draw_weapon(0);
    require(last_scale == 150);
    require(strcmp(last_text, "weapon-label-b 12") == 0);

    initialize(window); window.protoWeaponType_ = 21;
    window.draw_weapon(0);
    require(last_weapon_chassis == -1);
    initialize(window); window.protoWeaponType_ = 21;
    DesignAutoBuildActive = 1;
    window.draw_weapon(0);
    require(last_weapon_chassis == 4);

    initialize(window); Weapon[2].name = "a very long weapon name";
    window.font1_.line_height_ = 13;
    window.draw_weapon(0);
    require(last_wrapped == 1 && last_wrap_rect.top == 62 &&
            last_wrap_rect.bottom == 72);
    require(last_write_rect.top == 88 && last_write_rect.bottom == 98);

    initialize(window); window.weapon_panel_rect_ = rect(-4, -6, 1, 1);
    window.draw_weapon(0);
    require(last_render_x == -1 && last_render_y == -2);

    initialize(window);
    window.weapon_panel_rect_ = rect(2147483647, 0, 2147483647, 100);
    window.draw_weapon(0);
    require(last_render_x == -1);

    initialize(window); Weapon[2].mode = 3; Weapon[2].cost = 255;
    window.draw_weapon(0);
    require(last_scale == 80 && strcmp(last_text, "weapon-label-a 255") == 0);
    return 0;
}
