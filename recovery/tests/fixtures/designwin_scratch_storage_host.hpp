#pragma once

#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

struct RECT { int left, top, right, bottom; };
struct Font {
    int unk_1_ = 0, line_height_ = 10, height_ = 10;
    int pixel_height() const { return height_; }
    int line_height() const { return line_height_; }
};
struct HostTrace {
    int static_draw = 0, construct = 0, render = 0, multi_render = 0;
    int x = 0, y = 0, multi_x = 0, multi_y = 0;
};
inline HostTrace Trace{};

struct Buffer {
    void set_clip(RECT *) {}
    void copy(Buffer *, RECT *) {}
    void set_font(Font *, int, int, int) {}
    void set_text_color(int, int, int, int) {}
    void set_text_color2(int, int, int, int) {}
    int wrap_cent(char *, RECT *) { return 0; }
    void write_cent_l(char *, RECT *, int) {}
    void write_cent_l(Font *, char *, RECT *, int) {}
    void write_l(char *, int, int, int) {}
    void write_l(Font *, char *, int, int, int) {}
};

using ColorTable = unsigned char[256];
struct Caviar {
    void set_scene_rotation(float, float, float) {}
    void set_scaling(float) {}
    void render(Buffer *, int x, int y, int) { ++Trace.render; Trace.x=x; Trace.y=y; }
    void render_multi_table_dest(Buffer *, int x, int y, int, ColorTable *, int) {
        ++Trace.multi_render; Trace.multi_x=x; Trace.multi_y=y;
    }
    void reset_objects() {}
    void clear_buffers(int) {}
};

// Exact source layout from src/veh.h, intentionally included rather than a
// six-scalar imitation so test offsets and the signed icon byte remain real.
struct VehPrototype {
    char veh_name[32];
    uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id;
    uint8_t carry_capacity, cost, plan;
    int8_t unk_1;
    uint8_t obsolete_factions;
    int8_t combat_factions, icon_offset, padding;
    uint16_t flags;
    int16_t preq_tech;
};
static_assert(sizeof(VehPrototype) == 0x34);
static_assert(offsetof(VehPrototype, ability_flags) == 0x20);
static_assert(offsetof(VehPrototype, chassis_id) == 0x24);
static_assert(offsetof(VehPrototype, weapon_id) == 0x25);
static_assert(offsetof(VehPrototype, armor_id) == 0x26);
static_assert(offsetof(VehPrototype, reactor_id) == 0x27);
static_assert(offsetof(VehPrototype, icon_offset) == 0x2e);
static_assert(0x009ab868 + 63 * 0x34 == 0x009ac534);
inline VehPrototype VehPrototypes[128]{};

struct WeaponRow { int icon = 1, mode = 3, offense_rating = 0, cost = 1; };
struct ArmorRow { int mode = 3, defense_rating = 1; };
struct ChassisRow { int triad = 0, speed = 1; };
struct AbilityRow { const char *name = "ABILITY"; };
struct RulesType {
    int combat_pct_infantry_vs_base = 0;
    int combat_pct_mobile_open_ground = 0;
    int move_rate_roads = 1;
};
struct StringTableDouble { char *get(int) { return const_cast<char *>("ITEM"); } };
struct ConsoleDouble { int faction_id_ = 0; };
constexpr int MaxAbilityNum = 32;
inline WeaponRow Weapon[32]{};
inline ArmorRow Armor[32]{};
inline ChassisRow Chassis[32]{};
inline AbilityRow Ability[MaxAbilityNum]{};
inline char *Triad[4] = {const_cast<char *>("LAND"), const_cast<char *>("SEA"),
                         const_cast<char *>("AIR"), const_cast<char *>("ORBIT")};
inline RulesType Rules{};
inline StringTableDouble StringTable{};
inline ConsoleDouble ConsoleGlobal{};
inline char StringTemp[4096]{};
inline int TextColourTable[256]{};
inline ColorTable ArmorTransparentColorTables[24]{};
inline int MapWinCaptionScale=0, MapWinCaptionDivisor=1;

struct DesignWin : Buffer {
    RECT vehicle_panel_rect_{11, 23, 118, 177};
    int field_CE8_ = 0, factionID_ = 0, protoID_ = 1;
    int protoChassisType_ = 3, protoWeaponType_ = 5, protoArmorType_ = 7;
    int protoReactorType_ = 2, protoAbilityID1_ = -1, protoAbilityID2_ = -1;
    uint32_t protoAbilityFlags_ = 0x20;
    float scene_rotation_ = 0.0f, mineralCostFactor_ = 1.0f;
    char prototype_name_[128] = "";
    char preset_storage_[15][32]{};
    char *preset_names_[15]{};
    RECT field_B2C_rect_{};
    int field_B3C_=0, field_B40_=0, field_B44_=0, field_B48_=0;
    Font font2_{}, font3_{};
    Buffer buffer_{};
    Caviar caviar_{};
    DesignWin() {
        for (int i=0; i<15; ++i) {
            std::snprintf(preset_storage_[i], sizeof preset_storage_[i], "P%d", i);
            preset_names_[i] = preset_storage_[i];
        }
    }
    void UNK3();
    void draw_vehicle(int redraw);
    void draw_flash();
    void construct_vehicle() { ++Trace.construct; }
    void say_stats_3(char *, int) {}
};

inline char *label_get(int) { return const_cast<char *>("LABEL"); }
inline void name_proto(char *out, int, int, int, int, int, int, int) {
    std::strcpy(out, "CUSTOM");
}
inline int has_tech(int, int) { return 1; }
inline int transport_val(int, unsigned, int) { return 1; }
inline int proto_cost(int, int, int, int, int) { return 1; }
inline int prototype_factor(int) { return 100; }
inline int speed_proto(int) { return 1; }
inline int veh_fake(int, int) { return 1; }
inline void veh_draw(Buffer *, int, int, int, int, int, int) { ++Trace.static_draw; }
inline void vehdraw_construct_armor(int, int, int, int, int, Caviar *) {}
inline char *CharUpperA(char *text) {
    if (text) for (char *p=text; *p; ++p) *p = static_cast<char>(std::toupper(static_cast<unsigned char>(*p)));
    return text;
}
inline char *_itoa(int value, char *dest, int radix) {
    if (radix == 10) std::sprintf(dest, "%d", value);
    else std::sprintf(dest, "%x", static_cast<unsigned>(value));
    return dest;
}
