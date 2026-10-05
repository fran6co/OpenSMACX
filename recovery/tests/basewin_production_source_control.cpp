#include <assert.h>
#include <stdio.h>
#include <string.h>

typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
struct RECT { int left, top, right, bottom; };
struct Sprite;
struct Font {
    int height, style;
    int init(const char *, int value, int style_value) {
        height = value; style = style_value; return 0;
    }
    int line_advance() const { return height + style + 2; }
    int pixel_height() const { return height; }
};
struct Sprite {
    int create_blank(int, int, int) { return 0; }
    int draw_mono(void *, int, int, int, int) { return 0; }
    int draw_mono(void *, int, int, int, int color, int numerator, int denominator) {
        ++mono_calls; mono_color = color; mono_numerator = numerator;
        mono_denominator = denominator; return 0;
    }
    int transparent_index() const { return 0; }
    int width() const { return 20; }
    static int mono_calls, mono_color, mono_numerator, mono_denominator;
};
int Sprite::mono_calls = 0;
int Sprite::mono_color = 0;
int Sprite::mono_numerator = 0;
int Sprite::mono_denominator = 0;
static int MapWinCaptionScale = -1;
static int MapWinCaptionDivisor = -1;
struct Caviar {
    int reset_calls;
    int render_calls;
    float scale;
    float rotation;
    int caption_scale_during_scale;
    int caption_divisor_during_scale;
    int caption_scale_during_rotation;
    int caption_divisor_during_rotation;
    int caption_scale_during_render;
    int caption_divisor_during_render;
    Caviar() : reset_calls(0) {}
    void reset_objects() { ++reset_calls; }
    void set_scaling(float value) {
        scale = value;
        caption_scale_during_scale = MapWinCaptionScale;
        caption_divisor_during_scale = MapWinCaptionDivisor;
    }
    void set_scene_rotation(float value, float, float) {
        rotation = value;
        caption_scale_during_rotation = MapWinCaptionScale;
        caption_divisor_during_rotation = MapWinCaptionDivisor;
    }
    int render(void *, int, int, int) {
        ++render_calls;
        caption_scale_during_render = MapWinCaptionScale;
        caption_divisor_during_render = MapWinCaptionDivisor;
        return 0;
    }
};
struct Spot {
    void kill_type(int) {}
    int add(int, int, int, int, int, int) { return 0; }
};
struct VehPrototype {
    char veh_name[32]; unsigned char faction_id; unsigned char cost;
    unsigned char weapon_id;
};
struct Veh { unsigned char faction_id; unsigned char morale; };
struct WeaponRecord { signed char offense_rating; };
struct RulesFacility { char *name; int cost; };
struct BoxSpriteParams {
    Sprite *corner0_, *corner1_, *corner2_, *corner3_;
    Sprite *edge0_, *edge1_, *edge2_, *edge3_;
    void *mask_;
    int mask_color_;
};
struct Base {
    unsigned char faction_id_current;
    int minerals_accumulated;
    int mineral_surplus;
    int queue_production_id[10];
    unsigned char facilities_built[12];
};
struct BufferMock {
    int tile_calls;
    int box_calls;
    int fills;
    int clip_calls;
    int update_calls;
    int soft_update_calls;
    int text_writes;
    int explicit_text_writes;
    int border42, border6c;
    int first_tile_x, first_tile_y, first_tile_width, first_tile_height;
    Sprite *first_tile_sprite;
    Sprite *last_tile_sprite;
    int fill_calls, fill_42_calls, fill_6c_calls, fill_df_calls, last_fill_color;
    RECT last_fill_rect;
    RECT first_fill_rect;
    int set_font_calls, text_color_calls;
    Font *current_font;
    int wrap_cent_calls;
    int wrap_font_height[64], wrap_font_style[64];
    int last_wrap_x, last_wrap_y, last_wrap_width;
    int last_explicit_font_height, last_explicit_font_style;
    char last_text[128];
    int color_a[32], color_b[32];
    RECT clip;
    RECT clip_rect() { return clip; }
    int box_sprite(RECT *, void *) { return 0; }
    int set_clip(RECT *) { ++clip_calls; return 0; }
    int set_font(Font *font, Font *, Font *, Font *) {
        ++set_font_calls; current_font = font; return 0;
    }
    void set_text_color(int a, int b, int, int) {
        color_a[text_color_calls] = a; color_b[text_color_calls] = b;
        ++text_color_calls;
    }
    int wrap_cent(Font *font, char *, RECT *area) {
        wrap_font_height[wrap_cent_calls] = font->height;
        wrap_font_style[wrap_cent_calls] = font->style;
        ++wrap_cent_calls;
        return area->top + font->line_advance();
    }
    int wrap_cent(char *, RECT *area) {
        wrap_font_height[wrap_cent_calls] = current_font->height;
        wrap_font_style[wrap_cent_calls] = current_font->style;
        ++wrap_cent_calls;
        return area->top + current_font->line_advance();
    }
    int wrap_cent(char *, int x, int y, int width) {
        wrap_font_height[wrap_cent_calls] = current_font->height;
        wrap_font_style[wrap_cent_calls] = current_font->style;
        ++wrap_cent_calls;
        last_wrap_x = x; last_wrap_y = y; last_wrap_width = width;
        return y + current_font->line_advance();
    }
    int write_cent_l(char *, RECT *, int) { return 0; }
    int write_cent_l(char *, int, int, int, int) { return 0; }
    int write_cent_l(Font *font, char *text, RECT *, int) {
        ++text_writes; ++explicit_text_writes;
        last_explicit_font_height = font->height;
        last_explicit_font_style = font->style;
        strncpy(last_text, text, sizeof(last_text) - 1);
        last_text[sizeof(last_text) - 1] = 0;
        return 0;
    }
    int tile(Sprite *sprite, int x, int y, int, int, int width, int height) {
        last_tile_sprite = sprite;
        if (tile_calls == 0) {
            first_tile_sprite = sprite;
            first_tile_x = x; first_tile_y = y;
            first_tile_width = width; first_tile_height = height;
        }
        ++tile_calls;
        return 0;
    }
    int box(RECT *, int c, int) {
        ++box_calls;
        if (c == 0x42) ++border42;
        if (c == 0x6C) ++border6c;
        return 0;
    }
    int fill(RECT *rect, int color) {
        if (fill_calls == 0) first_fill_rect = *rect;
        last_fill_rect = *rect;
        ++fill_calls;
        last_fill_color = color;
        if (color == 0x42) ++fill_42_calls;
        if (color == 0x6C) ++fill_6c_calls;
        if (color == 0xDF) ++fill_df_calls;
        return 0;
    }
    int update(RECT *, void *) { ++update_calls; return 0; }
    void soft_update(RECT *) { ++soft_update_calls; }
};
struct ProdPicker { int edbc_fac(int) { return 0; } };
struct BaseWin {
    RECT queue_help_rect_, queue_outer_rect_, flash_rect_a_, flash_rect_b_;
    Spot spot1_;
    Sprite sprites2_[2], sprite5_;
    Caviar caviar_;
    ProdPicker prodPicker_;
    uint32_t field_1E670_;
    void draw_production(int);
};

static BufferMock MainInterfaceGlobal;
static Base CurrentBase;
static Base AlternateBase;
static Base *BaseCurrent = &CurrentBase;
static int BaseIDCurrentSelected = 0;
static int WinScreenWidth = 800;
static const char *DefaultFontFace = "font";
static const int WinFillColour = 0;
static char StringTemp[1024];
static VehPrototype VehPrototypes[300];
static Veh Vehs[2049];
static WeaponRecord Weapon[26];
static RulesFacility Facility[134];
static Sprite g_FACILITY_ICON_SPRITES[70], g_SECRET_PROJECT_ICON_SPRITES[64];
static BoxSpriteParams InterfaceBoxStyles[4];
static int TextColourTable[195];
static int VehDrawSuppressedVehicle;
static int rendered_cells;
static int fill_42_calls, fill_6c_calls, fill_df_calls;
static int veh_draw_result;
static int veh_draw_calls;
static int veh_fake_calls;
static int veh_fake_faction = -1;
static int veh_construct_calls;
static int veh_draw_x, veh_draw_y, veh_draw_zoom, veh_draw_flags, veh_draw_frame;
static int veh_draw_caption_scale, veh_draw_caption_divisor;
static int bitmask_calls, base_project_calls;
static int breed_mod_calls;
static int breed_mod_faction = -1;
static int cost_factor_faction = -1;
static bool switch_base_from_veh_fake;
static bool switch_base_from_veh_cost;
static bool switch_base_from_veh_draw;
static bool mutate_minerals_from_cost_factor;
static bool mutate_surplus_from_cost_factor;

static char *label_get(int) { return const_cast<char *>("label"); }
static char *CharUpperA(char *s) { return s; }
static void UnionRect(RECT *out, const RECT *a, const RECT *b) {
    out->left = a->left < b->left ? a->left : b->left;
    out->top = a->top < b->top ? a->top : b->top;
    out->right = a->right > b->right ? a->right : b->right;
    out->bottom = a->bottom > b->bottom ? a->bottom : b->bottom;
}
static char * _itoa(int value, char *out, int) { sprintf(out, "%d", value); return out; }
struct Strings { char *get(int) { return const_cast<char *>("display-name"); } };
static Strings StringTable;
static int cost_factor(int faction, int, int) {
    cost_factor_faction = faction;
    if (mutate_minerals_from_cost_factor) BaseCurrent->minerals_accumulated = 32;
    if (mutate_surplus_from_cost_factor) BaseCurrent->mineral_surplus = 3;
    return 4;
}
static int veh_cost(int id, int, int *has) {
    *has = 1;
    if (switch_base_from_veh_cost) BaseCurrent = &AlternateBase;
    return id ? 8 : 0;
}
static int veh_fake(int id, int faction) {
    ++veh_fake_calls; Vehs[2048].faction_id = 1;
    veh_fake_faction = faction;
    if (switch_base_from_veh_fake) BaseCurrent = &AlternateBase;
    return 2048 + id * 0;
}
static int mandate_color(int) { return 7; }
static int veh_draw(BufferMock *, int, int x, int y, int zoom, int flags, int frame) {
    ++veh_draw_calls;
    veh_draw_x = x; veh_draw_y = y; veh_draw_zoom = zoom;
    veh_draw_flags = flags; veh_draw_frame = frame;
    veh_draw_caption_scale = MapWinCaptionScale;
    veh_draw_caption_divisor = MapWinCaptionDivisor;
    if (switch_base_from_veh_draw) BaseCurrent = &AlternateBase;
    return veh_draw_result;
}
static void vehdraw_construct_vehicle(int, Caviar *renderer) {
    ++veh_construct_calls;
    renderer->reset_objects();
}
static int prototype_factor(int) { return 25; }
static int breed_mod(int, int faction) {
    ++breed_mod_calls; breed_mod_faction = faction; return 0;
}
static void bitmask(int id, int *offset, int *mask) { ++bitmask_calls; *offset = id / 8; *mask = 1 << (id % 8); }
static int base_project(int) { ++base_project_calls; return -1; }
static const int FacilitySPStart = 70;
struct GraphicWin {
    static int fill(RECT *, int color) {
        ++rendered_cells;
        if (color == 0x42) ++fill_42_calls;
        if (color == 0x6C) ++fill_6c_calls;
        if (color == 0xDF) ++fill_df_calls;
        return 0;
    }
};

#include "basewin_production_body.inc"

int main() {
    BaseWin win;
    for (int color_index = 0; color_index < 195; ++color_index)
        TextColourTable[color_index] = 1000 + color_index;
    RECT queue_rect = {0, 0, 250, 240};
    RECT outer_rect = {260, 0, 400, 180};
    RECT flash_a = {0, 190, 50, 210};
    RECT flash_b = {60, 190, 100, 210};
    win.queue_help_rect_ = queue_rect;
    win.queue_outer_rect_ = outer_rect;
    win.flash_rect_a_ = flash_a;
    win.flash_rect_b_ = flash_b;
    win.field_1E670_ = 0;
    CurrentBase.faction_id_current = 1;
    CurrentBase.queue_production_id[0] = 3;
    CurrentBase.minerals_accumulated = 4;
    CurrentBase.mineral_surplus = 2;
    VehPrototypes[3].veh_name[0] = 'u';
    VehPrototypes[3].cost = 8;
    VehPrototypes[3].weapon_id = 0;
    Weapon[0].offense_rating = -1;
    veh_draw_result = 1;
    win.draw_production(0);
    assert(MainInterfaceGlobal.clip_calls >= 3);
    assert(MainInterfaceGlobal.tile_calls == 32);
    assert(MainInterfaceGlobal.fill_calls == 0);
    assert(MainInterfaceGlobal.box_calls == 2);
    assert(MainInterfaceGlobal.first_tile_x == 10 && MainInterfaceGlobal.first_tile_y == 63);
    assert(MainInterfaceGlobal.first_tile_width == 55 && MainInterfaceGlobal.first_tile_height == 15);
    assert(MainInterfaceGlobal.soft_update_calls == 1);
    assert(veh_fake_calls == 1 && veh_draw_calls == 1);
    assert(breed_mod_calls == 1 && Vehs[2048].morale == 1);
    assert(veh_draw_zoom == 1 && veh_draw_flags == 0xCC && veh_draw_frame == 0);
    assert(veh_draw_x == 36 && veh_draw_y == -20);
    assert(veh_draw_caption_scale == 178 && veh_draw_caption_divisor == 100);
    assert(MapWinCaptionScale == -1 && MapWinCaptionDivisor == -1);
    assert(win.caviar_.reset_calls == 2 && win.caviar_.render_calls == 1);
    assert(win.caviar_.scale > 0.1245f && win.caviar_.scale < 0.1247f);
    assert(win.caviar_.rotation > 0.5232f && win.caviar_.rotation < 0.5235f);
    assert(win.caviar_.caption_scale_during_scale == 178);
    assert(win.caviar_.caption_divisor_during_scale == 100);
    assert(win.caviar_.caption_scale_during_rotation == 178);
    assert(win.caviar_.caption_divisor_during_rotation == 100);
    assert(win.caviar_.caption_scale_during_render == 178);
    assert(win.caviar_.caption_divisor_during_render == 100);
    assert(MainInterfaceGlobal.set_font_calls == 2);
    assert(MainInterfaceGlobal.wrap_cent_calls == 2);
    assert(MainInterfaceGlobal.wrap_font_height[0] == 12 &&
           MainInterfaceGlobal.wrap_font_style[0] == 1);
    assert(MainInterfaceGlobal.wrap_font_height[1] == 10 &&
           MainInterfaceGlobal.wrap_font_style[1] == 0);
    assert(MainInterfaceGlobal.color_a[0] == TextColourTable[82]);
    assert(MainInterfaceGlobal.color_b[0] == TextColourTable[83]);
    assert(MainInterfaceGlobal.color_a[1] == TextColourTable[118]);
    assert(MainInterfaceGlobal.color_b[1] == TextColourTable[119]);
    assert(MainInterfaceGlobal.color_a[2] == TextColourTable[104]);
    assert(MainInterfaceGlobal.color_b[2] == TextColourTable[105]);
    assert(MainInterfaceGlobal.text_writes == 1 &&
           MainInterfaceGlobal.explicit_text_writes == 1);
    assert(MainInterfaceGlobal.last_explicit_font_height == 10 &&
           MainInterfaceGlobal.last_explicit_font_style == 0);
    assert(MainInterfaceGlobal.last_wrap_x == 125 &&
           MainInterfaceGlobal.last_wrap_y == 46 &&
           MainInterfaceGlobal.last_wrap_width == 0);
    assert(BaseCurrent->facilities_built[0] == 0);
    assert(bitmask_calls == 0 && base_project_calls == 0);

    veh_draw_result = 0;
    const int resets_before_failed_draw = win.caviar_.reset_calls;
    const int renders_before_failed_draw = win.caviar_.render_calls;
    win.draw_production(0);
    assert(breed_mod_calls == 2);
    assert(win.caviar_.reset_calls == resets_before_failed_draw);
    assert(win.caviar_.render_calls == renders_before_failed_draw);
    assert(MapWinCaptionScale == -1 && MapWinCaptionDivisor == -1);

    MainInterfaceGlobal.tile_calls = MainInterfaceGlobal.box_calls = 0;
    rendered_cells = 0;
    MainInterfaceGlobal.clip_calls = 0;
    MainInterfaceGlobal.update_calls = 0;
    CurrentBase.queue_production_id[0] = -2;
    Facility[2].name = const_cast<char *>("facility");
    Facility[2].cost = 8;
    const int prior_fake_calls = veh_fake_calls;
    const int prior_vehicle_draws = veh_draw_calls;
    const int prior_resets = win.caviar_.reset_calls;
    const int prior_font_sets = MainInterfaceGlobal.set_font_calls;
    const int prior_wraps = MainInterfaceGlobal.wrap_cent_calls;
    const int prior_colors = MainInterfaceGlobal.text_color_calls;
    win.draw_production(1);
    assert(MainInterfaceGlobal.clip_calls >= 3);
    assert(MainInterfaceGlobal.update_calls == 1);
    assert(MainInterfaceGlobal.tile_calls == 32);
    assert(MainInterfaceGlobal.box_calls == 2);
    assert(veh_fake_calls == prior_fake_calls && veh_draw_calls == prior_vehicle_draws);
    assert(win.caviar_.reset_calls == prior_resets);
    assert(MainInterfaceGlobal.set_font_calls == prior_font_sets);
    assert(MainInterfaceGlobal.wrap_cent_calls == prior_wraps + 1);
    assert(MainInterfaceGlobal.wrap_font_height[prior_wraps] == 12 &&
           MainInterfaceGlobal.wrap_font_style[prior_wraps] == 1);
    assert(MainInterfaceGlobal.color_a[prior_colors] == TextColourTable[116]);
    assert(MainInterfaceGlobal.color_b[prior_colors] == TextColourTable[117]);
    assert(MainInterfaceGlobal.color_a[prior_colors + 1] == TextColourTable[104]);
    assert(MainInterfaceGlobal.color_b[prior_colors + 1] == TextColourTable[105]);
    assert(Sprite::mono_calls == 1 && Sprite::mono_color == 7);
    assert(Sprite::mono_numerator == 0x28 && Sprite::mono_denominator == 20);
    assert(bitmask_calls == 1);

    CurrentBase.facilities_built[0] = 4;
    const int colors_before_owned = MainInterfaceGlobal.text_color_calls;
    const int writes_before_owned = MainInterfaceGlobal.text_writes;
    win.draw_production(0);
    assert(MainInterfaceGlobal.text_writes == writes_before_owned + 1);
    assert(MainInterfaceGlobal.color_a[colors_before_owned + 1] == TextColourTable[120]);
    assert(MainInterfaceGlobal.color_b[colors_before_owned + 1] == TextColourTable[121]);
    assert(MainInterfaceGlobal.last_explicit_font_height == 12 &&
           MainInterfaceGlobal.last_explicit_font_style == 1);

    BaseCurrent = &CurrentBase;
    CurrentBase.queue_production_id[0] = 3;
    CurrentBase.faction_id_current = 1;
    CurrentBase.minerals_accumulated = 32;
    AlternateBase.faction_id_current = 5;
    AlternateBase.minerals_accumulated = 4;
    AlternateBase.mineral_surplus = 3;
    switch_base_from_veh_cost = true;
    win.draw_production(0);
    switch_base_from_veh_cost = false;
    assert(veh_fake_faction == 5 && breed_mod_faction == 5);
    assert(cost_factor_faction == 5);
    BaseCurrent = &CurrentBase;

    CurrentBase.queue_production_id[0] = 3;
    CurrentBase.faction_id_current = 1;
    CurrentBase.minerals_accumulated = 32;
    CurrentBase.mineral_surplus = 0;
    AlternateBase.faction_id_current = 5;
    AlternateBase.minerals_accumulated = 4;
    AlternateBase.mineral_surplus = 3;
    Weapon[0].offense_rating = -1;
    veh_draw_result = 0;
    switch_base_from_veh_fake = true;
    const int prior_borders = MainInterfaceGlobal.box_calls;
    MainInterfaceGlobal.tile_calls = 0;
    win.draw_production(0);
    assert(BaseCurrent == &AlternateBase);
    assert(breed_mod_faction == 5 && cost_factor_faction == 5);
    assert(MainInterfaceGlobal.first_tile_sprite == &win.sprites2_[1]);
    assert(MainInterfaceGlobal.last_tile_sprite == &win.sprites2_[0]);
    assert(MainInterfaceGlobal.box_calls == prior_borders + 3);
    BaseCurrent = &CurrentBase;
    switch_base_from_veh_fake = false;

    CurrentBase.queue_production_id[0] = 3;
    CurrentBase.faction_id_current = 1;
    CurrentBase.minerals_accumulated = 32;
    CurrentBase.mineral_surplus = 0;
    AlternateBase.faction_id_current = 5;
    AlternateBase.minerals_accumulated = 4;
    AlternateBase.mineral_surplus = 3;
    switch_base_from_veh_draw = true;
    const int prior_draw_switch_borders = MainInterfaceGlobal.box_calls;
    MainInterfaceGlobal.tile_calls = 0;
    win.draw_production(0);
    switch_base_from_veh_draw = false;
    assert(BaseCurrent == &AlternateBase);
    assert(breed_mod_faction == 1 && cost_factor_faction == 5);
    assert(MainInterfaceGlobal.first_tile_sprite == &win.sprites2_[1]);
    assert(MainInterfaceGlobal.last_tile_sprite == &win.sprites2_[0]);
    assert(MainInterfaceGlobal.box_calls == prior_draw_switch_borders + 3);
    BaseCurrent = &CurrentBase;

    CurrentBase.queue_production_id[0] = 3;
    CurrentBase.faction_id_current = 1;
    CurrentBase.minerals_accumulated = 4;
    CurrentBase.mineral_surplus = 0;
    mutate_minerals_from_cost_factor = true;
    MainInterfaceGlobal.tile_calls = 0;
    win.draw_production(0);
    mutate_minerals_from_cost_factor = false;
    assert(MainInterfaceGlobal.first_tile_sprite == &win.sprites2_[1]);
    assert(MainInterfaceGlobal.last_tile_sprite == &win.sprites2_[0]);
    CurrentBase.minerals_accumulated = 4;
    CurrentBase.mineral_surplus = 0;
    MainInterfaceGlobal.box_calls = 0;
    mutate_surplus_from_cost_factor = true;
    win.draw_production(0);
    mutate_surplus_from_cost_factor = false;
    assert(MainInterfaceGlobal.box_calls == 3);
    CurrentBase.mineral_surplus = 200000;
    win.draw_production(0);
    assert(strcmp(MainInterfaceGlobal.last_text, "label: 2") == 0);
    CurrentBase.mineral_surplus = 0;

    Facility[2].cost = 0;
    CurrentBase.queue_production_id[0] = -2;
    const int writes_before_zero_cost = MainInterfaceGlobal.text_writes;
    MainInterfaceGlobal.update_calls = 0;
    win.draw_production(1);
    assert(MainInterfaceGlobal.text_writes == writes_before_zero_cost);
    assert(MainInterfaceGlobal.update_calls == 1);
    CurrentBase.facilities_built[0] = 0;

    MainInterfaceGlobal.tile_calls = MainInterfaceGlobal.box_calls = 0;
    MainInterfaceGlobal.fill_calls = MainInterfaceGlobal.fill_42_calls = 0;
    MainInterfaceGlobal.fill_6c_calls = MainInterfaceGlobal.fill_df_calls = 0;
    rendered_cells = fill_42_calls = fill_6c_calls = fill_df_calls = 0;
    CurrentBase.queue_production_id[0] = -2;
    CurrentBase.minerals_accumulated = 4;
    CurrentBase.mineral_surplus = 0;
    Facility[2].cost = 25;
    CurrentBase.minerals_accumulated = 101;
    win.draw_production(0);
    assert(MainInterfaceGlobal.tile_calls == 0);
    assert(MainInterfaceGlobal.fill_calls == 102);
    assert(MainInterfaceGlobal.last_fill_color == 0x6C);
    assert(MainInterfaceGlobal.fill_42_calls == 100);
    assert(MainInterfaceGlobal.fill_6c_calls == 2 && MainInterfaceGlobal.fill_df_calls == 0);
}
