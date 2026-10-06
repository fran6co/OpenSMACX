// Ignored host-only projection of the proposed owner bodies. Product source
// changes remain coordinator-owned. This fixture preserves the function logic
// while replacing external services with observable test doubles.
#include <cassert>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#define __cdecl
using uint32_t = unsigned int;
using uint8_t = unsigned char;
using BOOL = int;
using LPCSTR = const char *;
struct RECT { int left, top, right, bottom; };

static std::vector<std::string> events;
static int next_base_id = 0;
static int territory_target = -1;
static int base_init_result = 0;
static int rocky_value = 0;
static int is_human_values[8] = {};
static int zoomed_value = 0;
static int show_window_mode = -1;
static int is_zoomed_calls = 0;
static int last_focus_x = -1;
static int last_focus_y = -1;
static int last_focus_faction = -1;
static int focus_calls = 0;
static int mutate_faction_after_draws = 0;
static int capture_lost_calls = 0;
static int slot52_calls = 0;
static int display_hook_calls = 0;
static int sound_play_calls = 0;
static int sound_is_playing = 0;
static int box_calls = 0;
static int close_sprite_draws = 0;
static int color_a = -1;
static int color_b = -1;
struct Draw { int slot, x, y, radius, faction; };
static std::vector<Draw> draws;

static const unsigned int MPREF_MAP_SHOW_FOG_WAR = 1;
static const unsigned int STATE_OMNISCIENT_VIEW = 0x80;
static const unsigned int SW_MAXIMIZE = 3;
static const unsigned int SW_MINIMIZE = 6;

struct DataFields {
    unsigned int game_more_preferences_ = 0;
    unsigned int game_state_ = 0;
    int turn_current_num_ = 0;
    int base_current_count_ = 0;
} fields;
struct GameDataState { DataFields fields_; } GameDataStateStorage;

struct PlayerData {
    int current_num_bases = 0;
    unsigned int diplo_treaties[8] = {};
    int last_turn_new_base = -1;
} PlayersData[8];

struct Veh {
    int x = 0;
    int y = 0;
    uint8_t faction_id = 0;
    unsigned int flags = 0;
    unsigned int visibility = 0;
    unsigned int state = 0;
    short home_base_id = -1;
} Vehs[256];

struct Base {
    uint32_t governor = 0;
    char name_string[64] = {};
    unsigned int visibility = 0;
    unsigned int faction_id_current = 0;
} Bases[256];

static void mutate_faction_if_ready();
struct Console {
    int faction_id_ = 0;
    void focus(int x, int y, int faction) {
        ++focus_calls;
        last_focus_x = x;
        last_focus_y = y;
        last_focus_faction = faction;
        events.push_back("focus");
        (void)x; (void)y; (void)faction;
    }
} ConsoleGlobal;
static void mutate_faction_if_ready() {
    if (mutate_faction_after_draws != 0 &&
        static_cast<int>(draws.size()) == mutate_faction_after_draws) {
        ConsoleGlobal.faction_id_ = 9;
    }
}

struct MapWin {
    int field_1DD74_ = 0;
    int slot = 0;
    void draw_radius(int x, int y, int radius, int faction) {
        draws.push_back({slot, x, y, radius, faction});
        mutate_faction_if_ready();
    }
} MapWinStorage[8];
static const unsigned int MapWinTableSlots = 8;
static MapWin *MapWinTable[MapWinTableSlots] = {};

static char ActionBuildLogCaption[12] = "Action Base";
static char ActionBuildTurnLogCaption[17] = "Action Base Turn";

void log_say(LPCSTR caption, int, int, int) {
    events.push_back(caption == ActionBuildLogCaption ? "log-base" : "log-turn");
}
int rocky_at(int, int) { events.push_back("rocky-read"); return rocky_value; }
void rocky_set(int, int, int) { events.push_back("rocky-write"); }
int whose_territory(int, int, int, int, int) {
    events.push_back("territory"); return territory_target;
}
int is_human(int faction) { return is_human_values[faction]; }
void treaty_on(int, int, int) { events.push_back("treaty-on"); }
void double_cross(int, int, int) { events.push_back("double-cross"); }
int base_init(int, int, int) { events.push_back("base-init"); return base_init_result; }
void make_base_unique(int) { events.push_back("unique-name"); }
void veh_kill(int) { events.push_back("veh-kill"); }
void draw_map(int) { events.push_back("draw-map"); }
void del_site(int, int, int, int, int) { events.push_back("del-site"); }
void mon_colony_founded(int, char *) { events.push_back("monument"); }
void veh_skip(int) { events.push_back("veh-skip"); }

void __cdecl draw_tile_fixup(int x_coord, int y_coord, int wider_radius,
                             int draw_type) {
    if (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_MAP_SHOW_FOG_WAR) {
        int const faction = ConsoleGlobal.faction_id_;
        for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
            MapWin *const window = MapWinTable[slot];
            if (window == 0) {
                continue;
            }
            if (slot != 0 && window->field_1DD74_ == 0) {
                continue;
            }
            window->draw_radius(x_coord, y_coord, (wider_radius != 0) + 1,
                                faction);
        }
    } else {
        for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
            MapWin *const window = MapWinTable[slot];
            if (window == 0) {
                continue;
            }
            if (slot != 0 && window->field_1DD74_ == 0) {
                continue;
            }
            window->draw_radius(x_coord, y_coord, 0, draw_type);
        }
    }
}

void __cdecl draw_tile_fixup(int x_coord, int y_coord, int second_x_coord,
                             int second_y_coord, int wider_radius,
                             int draw_type) {
    if (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_MAP_SHOW_FOG_WAR) {
        int const faction = ConsoleGlobal.faction_id_;
        for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
            MapWin *const window = MapWinTable[slot];
            if (window == 0) {
                continue;
            }
            if (slot != 0 && window->field_1DD74_ == 0) {
                continue;
            }
            window->draw_radius(x_coord, y_coord, (wider_radius != 0) + 1,
                                faction);
        }
        if (second_x_coord != -1) {
            int const faction = ConsoleGlobal.faction_id_;
            for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
                MapWin *const window = MapWinTable[slot];
                if (window == 0) {
                    continue;
                }
                if (slot != 0 && window->field_1DD74_ == 0) {
                    continue;
                }
                window->draw_radius(second_x_coord, second_y_coord,
                                    (wider_radius != 0) + 1, faction);
            }
        }
    } else {
        for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
            MapWin *const window = MapWinTable[slot];
            if (window == 0) {
                continue;
            }
            if (slot != 0 && window->field_1DD74_ == 0) {
                continue;
            }
            window->draw_radius(x_coord, y_coord, 0, draw_type);
        }
    }
}

int __cdecl action_build(int veh_id, char *base_name) {
    const int x = Vehs[veh_id].x;
    const int y = Vehs[veh_id].y;
    const unsigned char owner = Vehs[veh_id].faction_id;

    log_say(ActionBuildLogCaption, x, y, owner);
    log_say(ActionBuildTurnLogCaption, GameDataStateStorage.fields_.turn_current_num_, 0,
            0);

    if (rocky_at(x, y) > 0x40) {
        rocky_set(x, y, 1);
    }

    if (owner == ConsoleGlobal.faction_id_ ||
        (Vehs[veh_id].faction_id == ConsoleGlobal.faction_id_ &&
         (Vehs[veh_id].flags & 0x440) != 0x440) ||
        ((1 << (ConsoleGlobal.faction_id_ & 0x1f)) &
         Vehs[veh_id].visibility) != 0 ||
        (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0) {
        ConsoleGlobal.focus(x, y, owner);
    }

    int target = whose_territory(owner, x, y, 0, 0);

    if (PlayersData[owner].current_num_bases != 0 && target >= 0 &&
        static_cast<unsigned int>(target) != owner &&
        (PlayersData[owner].diplo_treaties[target] & 0x11) == 0 &&
        (is_human(owner) || is_human(target))) {
        if ((PlayersData[owner].diplo_treaties[target] & 2) == 0) {
            treaty_on(owner, target, 0x10);
        } else {
            double_cross(owner, target, -1);
        }
    }

    int base_id = base_init(owner, x, y);
    if (base_id >= 0) {
        if (is_human(owner)) {
            const short other_base = Vehs[veh_id].home_base_id;
            if (other_base >= 0) {
                Bases[base_id].governor = Bases[other_base].governor;
            }
            if ((Vehs[veh_id].state & 0x200) != 0) {
                Bases[base_id].governor |= 0x80000000;
            }
        }
        PlayersData[owner].last_turn_new_base = GameDataStateStorage.fields_.turn_current_num_;

        if (base_name != 0 && strlen(base_name) != 0) {
            strcpy(Bases[base_id].name_string, base_name);
            make_base_unique(base_id);
        }

        veh_kill(veh_id);

        if (owner == ConsoleGlobal.faction_id_ ||
            Bases[base_id].faction_id_current == ConsoleGlobal.faction_id_ ||
            (Bases[base_id].visibility &
             (1 << (ConsoleGlobal.faction_id_ & 0x1f))) != 0 ||
            (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0) {
            draw_map(1);
        }

        for (int i = 1; i < 8; i++) {
            del_site(i, 8, x, y, 2);
        }

        mon_colony_founded(owner, Bases[base_id].name_string);
        return base_id;
    }

    veh_skip(veh_id);
    return base_id;
}

struct Sound {
    int is_playing() { events.push_back("sound-query"); return sound_is_playing; }
    void play() { ++sound_play_calls; events.push_back("sound-play"); }
};
typedef void (__cdecl *WinDisplayActionHookFn)();
struct AutoSound { Sound *val_35_ = 0; };
struct Win : AutoSound {
    WinDisplayActionHookFn field_438_ = 0;
    virtual void on_lose_mouse_capture() { ++capture_lost_calls; }
    virtual void vslot_52() { ++slot52_calls; events.push_back("slot52"); }
    static void __cdecl display_action(int action);
};
static Win *WinCallbackWindow = 0;
static Win *WinPointerOwner1 = 0;
static Win *WinPointerOwner2 = 0;
static Win *WinPointerOwner3 = 0;
static Win *WinPointerOwner4 = 0;
static void *HandleMain = reinterpret_cast<void *>(1);
static int IsZoomed(void *) { ++is_zoomed_calls; return zoomed_value; }
static int ShowWindow(void *, int mode) { show_window_mode = mode; return 0; }

struct BoxSpriteParams {};
static BoxSpriteParams InterfaceBoxStyles[64];
static int TextColourTable[195];
struct Buffer {
    void restore_clip() { events.push_back("restore-clip"); }
    void box_sprite(RECT *, BoxSpriteParams *style) {
        assert(style >= InterfaceBoxStyles && style < InterfaceBoxStyles + 64);
        ++box_calls;
        events.push_back("box");
    }
    void set_text_color(int a, int b, int, int) {
        color_a = a; color_b = b; events.push_back("color");
    }
};
struct GraphicWin : Buffer {};
struct Sprite {
    int height() const { return 10; }
    int transparent_index() const { return 3; }
    void draw(Buffer *, int, int, int, int, int) {
        ++close_sprite_draws;
    }
};
static Sprite g_IFACE_CLOSE_X_SPRITES[32];
struct DialogsState { int kind_ = 0; };
struct Popup : Win {
    unsigned int field_30AC_ = 0;
    DialogsState dialogs_;
    Win *external_window_ = 0;
    int on_dialog_item_back_draw(GraphicWin *, int, int, RECT *);
};
static Popup DiplomacyPopup;

int Popup::on_dialog_item_back_draw(GraphicWin *a1, int, int a3, RECT *a4) {
    if (a1 == 0) {
        return 0;
    }
    if (a4 == 0) {
        return 0;
    }
    if (static_cast<int>(field_30AC_) / 10000 == 1000) {
        return 0;
    }

    Buffer *const buffer = static_cast<Buffer *>(a1);
    buffer->restore_clip();

    RECT local_rect;
    local_rect.left = a4->left;
    local_rect.top = a4->top;
    local_rect.right = a4->right;
    local_rect.bottom = a4->bottom;

    if (dialogs_.kind_ == 1) {
        Sprite *const spr = &g_IFACE_CLOSE_X_SPRITES[a3];
        local_rect.left += 3;
        const int y = local_rect.top +
                      (local_rect.bottom - spr->height() - local_rect.top) / 2;
        spr->draw(buffer, spr->transparent_index(), local_rect.left, y, 1, 1);
        local_rect.left += 0x15;
    }

    buffer->box_sprite(&local_rect, &InterfaceBoxStyles[27 + a3]);

    if (external_window_ == static_cast<Win *>(&DiplomacyPopup)) {
        buffer->set_text_color(TextColourTable[192 - a3 * 2],
                               TextColourTable[193 - a3 * 2], 1, 1);
        return 1;
    }
    buffer->set_text_color(TextColourTable[80 - a3 * 2],
                           TextColourTable[81 - a3 * 2], 1, 1);
    return 1;
}

void __cdecl Win::display_action(int action) {
    Win *self = WinCallbackWindow;
    if (action == 0) {
        int zoomed = IsZoomed(HandleMain);
        WinPointerOwner1 = nullptr;
        WinPointerOwner2 = nullptr;
        if (zoomed != 0) {
            if (WinPointerOwner3 == self) {
                WinPointerOwner3 = nullptr;
                self->on_lose_mouse_capture();
            }
        } else {
            if (WinPointerOwner3 == self) {
                WinPointerOwner3 = nullptr;
                self->on_lose_mouse_capture();
            }
        }
        if (WinPointerOwner4 == self) {
            WinPointerOwner4 = nullptr;
        }
        ShowWindow(HandleMain, SW_MAXIMIZE);
    } else if (action == 1) {
        WinPointerOwner1 = nullptr;
        WinPointerOwner2 = nullptr;
        if (WinPointerOwner3 == self) {
            WinPointerOwner3 = nullptr;
            self->on_lose_mouse_capture();
        }
        if (WinPointerOwner4 == self) {
            WinPointerOwner4 = nullptr;
        }
        ShowWindow(HandleMain, SW_MINIMIZE);
        return;
    } else if (action == 2) {
        self->vslot_52();
        WinDisplayActionHookFn hook = self->field_438_;
        if (hook != 0) {
            hook();
        }
        Sound *child = self->val_35_;
        if (child != 0) {
            if (child->is_playing() == 0) {
                child->play();
            }
        }
        return;
    }
}

static void reset_observations() {
    events.clear(); draws.clear(); box_calls = close_sprite_draws = 0;
    color_a = color_b = -1; focus_calls = capture_lost_calls = 0;
    slot52_calls = display_hook_calls = sound_play_calls = 0;
    show_window_mode = -1; is_zoomed_calls = 0;
}

static void reset_map_slots() {
    draws.clear();
    for (int i = 0; i < 8; ++i) {
        MapWinStorage[i].field_1DD74_ = 0;
        MapWinStorage[i].slot = i;
        MapWinTable[i] = &MapWinStorage[i];
    }
    MapWinTable[1] = nullptr;
    MapWinStorage[2].field_1DD74_ = 1;
    MapWinStorage[4].field_1DD74_ = 1;
    MapWinStorage[5].field_1DD74_ = 0;
    MapWinTable[6] = nullptr;
    MapWinStorage[7].field_1DD74_ = 1;
}

static void test_draw_tile_fixup() {
    reset_map_slots();
    ConsoleGlobal.faction_id_ = 5;
    GameDataStateStorage.fields_.game_more_preferences_ = 0;
    draw_tile_fixup(4, 7, 1, 9);
    assert(draws.size() == 4);
    for (const Draw &draw : draws) {
        assert(draw.x == 4 && draw.y == 7 && draw.radius == 0 && draw.faction == 9);
    }
    assert(draws[0].slot == 0 && draws[1].slot == 2 &&
           draws[2].slot == 4 && draws[3].slot == 7);

    draws.clear();
    GameDataStateStorage.fields_.game_more_preferences_ = MPREF_MAP_SHOW_FOG_WAR;
    draw_tile_fixup(4, 7, 0, 9);
    assert(draws.size() == 4);
    for (const Draw &draw : draws) {
        assert(draw.radius == 1 && draw.faction == 5);
    }

    draws.clear();
    draw_tile_fixup(4, 7, 8, 9, 1, 6);
    assert(draws.size() == 8);
    for (int i = 0; i < 4; ++i) {
        assert(draws[i].x == 4 && draws[i].y == 7);
        assert(draws[i + 4].x == 8 && draws[i + 4].y == 9);
        assert(draws[i].radius == 2 && draws[i + 4].radius == 2);
        assert(draws[i].faction == 5 && draws[i + 4].faction == 5);
    }

    draws.clear();
    ConsoleGlobal.faction_id_ = 5;
    mutate_faction_after_draws = 4;
    draw_tile_fixup(4, 7, 8, 9, 1, 6);
    assert(draws.size() == 8);
    for (int i = 0; i < 4; ++i) assert(draws[i].faction == 5);
    for (int i = 4; i < 8; ++i) assert(draws[i].faction == 9);
    mutate_faction_after_draws = 0;

    draws.clear();
    draw_tile_fixup(4, 7, -1, 9, 1, 6);
    assert(draws.size() == 4);
    for (const Draw &draw : draws) assert(draw.x == 4);

    draws.clear();
    GameDataStateStorage.fields_.game_more_preferences_ = 0;
    draw_tile_fixup(4, 7, 8, 9, 1, 6);
    assert(draws.size() == 4);
    for (const Draw &draw : draws) assert(draw.x == 4 && draw.faction == 6);
}

static void reset_action_build() {
    reset_observations();
    rocky_value = 0x41;
    territory_target = 1;
    base_init_result = 4;
    std::memset(Vehs, 0, sizeof(Vehs));
    std::memset(Bases, 0, sizeof(Bases));
    std::memset(PlayersData, 0, sizeof(PlayersData));
    std::memset(is_human_values, 0, sizeof(is_human_values));
    Vehs[3].x = 10; Vehs[3].y = 14; Vehs[3].faction_id = 0;
    Vehs[3].home_base_id = 2; Vehs[3].state = 0x200;
    ConsoleGlobal.faction_id_ = 0;
    GameDataStateStorage.fields_.game_state_ = 0;
    GameDataStateStorage.fields_.turn_current_num_ = 23;
    PlayersData[0].current_num_bases = 1;
    is_human_values[0] = 1;
    is_human_values[1] = 1;
    Bases[2].governor = 7;
    Bases[4].faction_id_current = 0;
    Bases[4].visibility = 1;
}

static void test_action_build() {
    reset_action_build();
    int result = action_build(3, const_cast<char *>("Gaia"));
    assert(result == 4);
    assert(std::string(Bases[4].name_string) == "Gaia");
    assert(Bases[4].governor == 0x80000007U);
    assert(PlayersData[0].last_turn_new_base == 23);
    assert(focus_calls == 1 && last_focus_x == 10 && last_focus_y == 14 &&
           last_focus_faction == 0);
    assert(events[0] == "log-base" && events[1] == "log-turn");
    assert(events[2] == "rocky-read" && events[3] == "rocky-write");
    assert(events[4] == "focus" && events[5] == "territory");
    assert(events[6] == "treaty-on" && events[7] == "base-init");
    assert(events[8] == "unique-name" && events[9] == "veh-kill");
    assert(events[10] == "draw-map");
    int sites = 0;
    for (const std::string &event : events) if (event == "del-site") ++sites;
    assert(sites == 7 && events.back() == "monument");

    reset_action_build();
    ConsoleGlobal.faction_id_ = 1;
    territory_target = 1;
    PlayersData[0].current_num_bases = 1;
    PlayersData[0].diplo_treaties[1] = 2;
    is_human_values[1] = 1;
    base_init_result = 4;
    assert(action_build(3, const_cast<char *>("Gaia")) == 4);
    assert(std::find(events.begin(), events.end(), "double-cross") != events.end());
    assert(focus_calls == 0);

    reset_action_build();
    territory_target = -1;
    base_init_result = -1;
    result = action_build(3, nullptr);
    assert(result == -1 && events.back() == "veh-skip");
    assert(std::find(events.begin(), events.end(), "veh-kill") == events.end());
}

static void test_popup_item_painter() {
    reset_observations();
    Popup popup;
    GraphicWin window;
    RECT rect = {10, 20, 90, 60};
    for (int i = 0; i < 195; ++i) TextColourTable[i] = i + 1000;
    assert(popup.on_dialog_item_back_draw(nullptr, 0, 2, &rect) == 0);
    assert(popup.on_dialog_item_back_draw(&window, 0, 2, nullptr) == 0);
    popup.field_30AC_ = 1000U * 10000U;
    assert(popup.on_dialog_item_back_draw(&window, 0, 2, &rect) == 0);
    assert(events.empty());

    popup.field_30AC_ = 0;
    popup.dialogs_.kind_ = 1;
    popup.external_window_ = static_cast<Win *>(&DiplomacyPopup);
    assert(popup.on_dialog_item_back_draw(&window, 0, 2, &rect) == 1);
    assert(close_sprite_draws == 1 && box_calls == 1);
    assert(color_a == TextColourTable[188] && color_b == TextColourTable[189]);
    assert(events[0] == "restore-clip");

    reset_observations();
    popup.dialogs_.kind_ = 0;
    popup.external_window_ = nullptr;
    assert(popup.on_dialog_item_back_draw(&window, 0, 0, &rect) == 1);
    assert(color_a == TextColourTable[80] && color_b == TextColourTable[81]);

    reset_observations();
    assert(popup.on_dialog_item_back_draw(&window, 0, 3, &rect) == 1);
    assert(close_sprite_draws == 0 && box_calls == 1);
    assert(color_a == TextColourTable[74] && color_b == TextColourTable[75]);
}

static void test_win_display_action() {
    reset_observations();
    Win win;
    WinCallbackWindow = &win;
    WinPointerOwner1 = WinPointerOwner2 = WinPointerOwner3 = WinPointerOwner4 = &win;
    zoomed_value = 1;
    Win::display_action(0);
    assert(is_zoomed_calls == 1 && show_window_mode == SW_MAXIMIZE);
    assert(WinPointerOwner1 == nullptr && WinPointerOwner2 == nullptr);
    assert(WinPointerOwner3 == nullptr && WinPointerOwner4 == nullptr);
    assert(capture_lost_calls == 1);

    reset_observations();
    WinPointerOwner1 = WinPointerOwner2 = WinPointerOwner4 = &win;
    WinPointerOwner3 = nullptr;
    zoomed_value = 0;
    Win::display_action(0);
    assert(is_zoomed_calls == 1 && show_window_mode == SW_MAXIMIZE);
    assert(capture_lost_calls == 0 && WinPointerOwner4 == nullptr);

    reset_observations();
    WinPointerOwner1 = WinPointerOwner2 = WinPointerOwner3 = WinPointerOwner4 = &win;
    Win::display_action(1);
    assert(is_zoomed_calls == 0 && show_window_mode == SW_MINIMIZE);
    assert(capture_lost_calls == 1 && WinPointerOwner3 == nullptr);

    reset_observations();
    Sound sound;
    win.val_35_ = &sound;
    win.field_438_ = []() { ++display_hook_calls; events.push_back("display-hook"); };
    sound_is_playing = 0;
    Win::display_action(2);
    assert(slot52_calls == 1 && display_hook_calls == 1);
    assert(sound_play_calls == 1);
    assert(events[0] == "slot52" && events[1] == "display-hook" &&
           events[2] == "sound-query" && events[3] == "sound-play");

    reset_observations();
    sound_is_playing = 1;
    Win::display_action(2);
    assert(slot52_calls == 1 && display_hook_calls == 1);
    assert(sound_play_calls == 0);
    assert(events[0] == "slot52" && events[1] == "display-hook" &&
           events[2] == "sound-query");

    reset_observations();
    win.val_35_ = nullptr;
    Win::display_action(2);
    assert(slot52_calls == 1 && display_hook_calls == 1 &&
           sound_play_calls == 0);
    assert(events[0] == "slot52" && events[1] == "display-hook");

    reset_observations();
    Win::display_action(9);
    assert(slot52_calls == 0 && display_hook_calls == 0 && sound_play_calls == 0);
}

int main() {
    reset_map_slots();
    test_draw_tile_fixup();
    test_action_build();
    test_popup_item_painter();
    test_win_display_action();
}
