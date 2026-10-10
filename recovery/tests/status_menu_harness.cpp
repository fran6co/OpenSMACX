// Collaborator doubles for the actual StatusWin handler inserted by pytest.
// These are test-only objects, not a replacement implementation of the game.
#include <assert.h>
#include <stdio.h>
#include <string.h>

enum { STATE_SCENARIO_EDITOR = 0x40, STATE_OMNISCIENT_VIEW = 0x80 };
enum { DRAW = 1, REDRAW, ACTIVE, ZOOM, SOFT_ZOOM, PRODUCTION, HURRY,
       GOVERNOR, LOCK, WAKE, SYNC, AWAIT, UNLOCK, MOVE, HELP, EDIT, DATALINK };
struct Event { int code, a, b, c; } events[32];
int event_count, menu_count, separator_count, spy_subject;
int menu_ids[16];
char menu_text[16][128], labels[1200][16];
int hit_result, hit_menu, hit_index, selected_action, display_choice, lock_result, selectable;
int spying_result, live_menus, live_displays;
struct GameDataState { struct Fields { int game_state_; } fields_; } GameDataStateStorage;
int &GameState = GameDataStateStorage.fields_.game_state_;
int IsMultiplayerNet, ConsoleControlTurnActive;
char StringTemp[1032];

void event(int code, int a = 0, int b = 0, int c = 0) {
    assert(event_count < 32);
    Event &value = events[event_count++];
    value.code = code; value.a = a; value.b = b; value.c = c;
}
int count(int code) {
    int result = 0;
    for (int i = 0; i < event_count; ++i) result += events[i].code == code;
    return result;
}
Event &only(int code) {
    assert(count(code) == 1);
    for (int i = 0; i < event_count; ++i) if (events[i].code == code) return events[i];
    return events[0];
}
bool offered(int id) {
    for (int i = 0; i < menu_count; ++i) if (menu_ids[i] == id) return true;
    return false;
}
char *label_get(int id) { assert(id >= 0 && id < 1200); return labels[id]; }
struct Base { int faction_id_current, queue_production_id[10]; } Bases[8];
struct Faction { int diplo_treaties[8]; } PlayersData[8];
struct FacilityRecord { char *name; } Facility[8];
struct Prototype { char veh_name[32]; } VehPrototypes[8];
struct Strings {
    char *get(int id) { assert(id == 17); return const_cast<char *>("Facility"); }
    char *get(char *id) {
        assert(id == reinterpret_cast<char *>(17));
        return const_cast<char *>("Facility");
    }
} StringTable;
int spying(int subject) { spy_subject = subject; return spying_result; }
int veh_selectable(int index) { assert(index == hit_index); return selectable; }
int veh_wake(int index) { event(WAKE, index); return 0; }
void synch_veh(int index) { event(SYNC, index); }
void help_any() { event(HELP); }
struct PullDown {
    PullDown() { ++live_menus; }
    ~PullDown() { --live_menus; }
    void init(void *parent) { assert(parent == 0); }
    void add_item(int id, char *text) {
        assert(menu_count < 16);
        menu_ids[menu_count] = id;
        strcpy(menu_text[menu_count++], text);
    }
    void add_separator(int id) { assert(id == 99); ++separator_count; }
    void show(int x, int y, int (*poll)()) { assert(x == 110 && y == 220 && poll == 0); }
    // The double gives the handler an action, not an index. Native compiler
    // controls separately check the real PullDown implementation and layout.
    int get_selected_id() { return selected_action; }
};
struct Dialogs {
    int items;
    void item(char *text, int id) {
        assert(id == items++ && strcmp(text, labels[0xFEC / 4 + id]) == 0);
    }
};
struct PopMenu {
    Dialogs rows;
    PopMenu() { ++live_displays; rows.items = 0; }
    ~PopMenu() { --live_displays; }
    void init() {}
    Dialogs &dialogs() { return rows; }
    int exec(int x, int y, int (*poll)()) {
        assert(rows.items == 3 && x == 130 && y == 240 && poll == 0);
        return display_choice;
    }
};
struct Interface {
    void get_mouse_pos(int *x, int *y) { *x = 30; *y = 40; }
    void client_to_screen(int *x, int *y) { *x += 100; *y += 200; }
} MainInterfaceGlobal;
struct Datalink { void exec(int topic, int id) { event(DATALINK, topic, id); } } DatalinkMain;
struct BaseWin {
    void zoom(int id, int wait) { event(ZOOM, id, wait); }
    void soft_zoom(int id) { event(SOFT_ZOOM, id); }
    void production(int a, int b) { event(PRODUCTION, a, b); }
    void hurry() { event(HURRY); }
    void gov_on(int flags) { event(GOVERNOR, flags); }
} TutWinBaseWindow;
struct Daemon {
    struct PrimaryState { uint32_t leading_words_[3]; uint32_t flags_; uint32_t trailing_words_[10]; } primary_state_{};
    struct ChannelState { uint32_t leading_words_[4]; union Payload { uint32_t flags; int *vehicle_selection_follow; } payload_; uint32_t trailing_words_[10]; } channels_[2]{};
    int lock_veh(int *id, int target, int a, int b, int c) {
        assert(target == 0 && a == -1 && b == -1 && c == 0);
        event(LOCK, *id); return lock_result;
    }
    void await_synch() { event(AWAIT); }
    void unlock_veh() { event(UNLOCK); }
} NetDaemonNet;
struct Console {
    int faction_id_, selected_vehicle_;
    void set_move(int force) { event(MOVE, force); }
    void editor_edit_veh(int id) { event(EDIT, id); }
} ConsoleGlobal;
struct Spot {
    int check(int x, int y, int *index, int *menu) {
        assert(x == 10 && y == 20); *index = hit_index; *menu = hit_menu; return hit_result;
    }
};
struct StatusWin {
    Spot spot_;
    int field_15A8_, iUnitIndex_, iCityIndex_, hover_vehicle_id_, hover_base_id_, field_15CC_, field_15D0_;
    void on_right_click(int, int, int);
    void draw_status(int a, int b, int c, int d, int e) {
        assert(a == -1 && b == -1 && c == -1 && d == -1 && e == 0); event(DRAW);
    }
    void redraw() { event(REDRAW); }
    void draw_active(int x, int y, int mode) { event(ACTIVE, x, y, mode); }
} status;

// RECOVERED_HANDLER is replaced with read_file(...).region, never a copy.

void reset() {
    assert(live_menus == 0 && live_displays == 0);
    event_count = menu_count = separator_count = 0;
    spy_subject = -1;
    hit_result = 0; hit_menu = 1; hit_index = 3;
    selected_action = -1; display_choice = -1;
    selectable = 1; lock_result = spying_result = 0;
    GameState = IsMultiplayerNet = ConsoleControlTurnActive = 0;
    ConsoleGlobal.faction_id_ = 2; ConsoleGlobal.selected_vehicle_ = 7;
    memset(Bases, 0, sizeof(Bases));
    memset(PlayersData, 0, sizeof(PlayersData));
    memset(&NetDaemonNet, 0, sizeof(NetDaemonNet));
    memset(&status, 0, sizeof(status));
    Bases[3].faction_id_current = ConsoleGlobal.faction_id_;
    Bases[3].queue_production_id[0] = 1;
    strcpy(VehPrototypes[1].veh_name, "Scout");
    Facility[2].name = reinterpret_cast<char *>(17);
    status.hover_vehicle_id_ = 4; status.hover_base_id_ = 8;
    status.field_15CC_ = 11; status.field_15D0_ = 12;
}
void click() {
    status.on_right_click(10, 20, 999);
    assert(live_menus == 0 && live_displays == 0);
}
int main() {
    for (int i = 0; i < 1200; ++i) sprintf(labels[i], "L%d", i);
    reset(); hit_result = -1; click(); assert(event_count == 0 && menu_count == 0);
    for (int choice = -1; choice <= 2; ++choice) {
        reset(); hit_menu = 3; display_choice = choice; click();
        assert(count(ACTIVE) == 0 && menu_count == 0);
        assert(count(DRAW) == (choice == 0 || choice == 1));
        assert(count(DATALINK) == (choice == 2));
        if (choice == 1) assert(status.field_15A8_ == -1);
        if (choice == 2) assert(only(DATALINK).a == 9 && only(DATALINK).b == 0);
    }
    reset(); hit_menu = 2; selected_action = 20; click();
    assert(menu_count == 1 && offered(20) && separator_count == 0 && count(HELP) == 1);
    reset(); hit_menu = 0; GameState = STATE_SCENARIO_EDITOR; selected_action = 90; click();
    assert(offered(6) && offered(7) && offered(90) && separator_count == 1 && only(EDIT).a == 3);
    reset(); hit_menu = 10; selected_action = 7; click();
    assert(status.iUnitIndex_ == 7 && count(REDRAW) == 1 && !offered(90));
    reset(); Bases[3].faction_id_current = 4; spying_result = 1; click();
    assert(spy_subject == 4 && offered(5) && !offered(51) && separator_count == 0);
    reset(); Bases[3].faction_id_current = 4; PlayersData[2].diplo_treaties[4] = 1; click();
    assert(offered(5) && !offered(51));
    reset(); Bases[3].faction_id_current = 4; click(); assert(menu_count == 1 && offered(20));
    reset(); Bases[3].faction_id_current = 4; GameState = STATE_OMNISCIENT_VIEW; click();
    assert(offered(51) && offered(56));
    reset(); click();
    assert(menu_count == 8 && separator_count == 1 && strcmp(menu_text[2], " L551 Scout") == 0);
    assert(strcmp(menu_text[6], " L555") == 0); // final row must start with an empty scratch buffer
    reset(); Bases[3].queue_production_id[0] = -2; click();
    assert(strcmp(menu_text[2], " L551 Facility") == 0);
    for (int action = 1; action <= 2; ++action) {
        reset(); selected_action = action; click(); assert(count(REDRAW) == 1);
        assert(status.iCityIndex_ == (action == 1 ? 3 : -1));
        assert(status.iUnitIndex_ == (action == 2 ? 3 : -1));
    }
    reset(); selected_action = 5; click(); assert(only(ZOOM).a == 3 && only(ZOOM).b == 0 && !count(ACTIVE));
    for (int failure = 0; failure <= 1; ++failure) {
        reset(); hit_menu = 0; selected_action = 6; lock_result = failure; click();
        assert(ConsoleGlobal.selected_vehicle_ == 3 && only(LOCK).a == 3 && only(MOVE).a == 1 && !count(ACTIVE));
        assert(count(WAKE) == !failure && count(SYNC) == !failure);
        assert(count(AWAIT) == !failure && count(UNLOCK) == !failure);
        if (!failure) {
            assert(events[0].code == LOCK && events[1].code == WAKE && events[2].code == SYNC);
            assert(events[3].code == AWAIT && events[4].code == UNLOCK && events[5].code == MOVE);
        }
    }
    reset(); hit_menu = 0; selected_action = 6; selectable = 0; click(); assert(event_count == 0);
    for (int command = 51; command <= 56; ++command) {
        reset(); selected_action = command; click(); assert(only(SOFT_ZOOM).a == 3);
        if (command == 51) assert(count(PRODUCTION) == 1);
        else if (command == 52) assert(count(HURRY) == 1);
        else assert(only(GOVERNOR).a == (0x1000000 << (command - 53)));
        assert(only(ACTIVE).a == 11 && only(ACTIVE).b == 12 && only(ACTIVE).c == 0);
        assert(status.hover_vehicle_id_ == -1 && status.hover_base_id_ == -1);
        for (int block = 0; block < 4; ++block) {
            reset(); selected_action = command; IsMultiplayerNet = 1;
            if (block == 0) ConsoleControlTurnActive = 1;
            if (block == 1) NetDaemonNet.channels_[1].payload_.flags = 1;
            if (block == 2) NetDaemonNet.primary_state_.leading_words_[2] = 1;
            if (block == 3) NetDaemonNet.primary_state_.trailing_words_[7] = 1 << ConsoleGlobal.faction_id_;
            click(); assert(!count(SOFT_ZOOM) && count(ACTIVE) == 1);
            IsMultiplayerNet = 0; event_count = 0; click(); assert(count(SOFT_ZOOM) == 1);
        }
    }
    reset(); status.hover_vehicle_id_ = status.hover_base_id_ = -1; click(); assert(!count(ACTIVE));
    return 0;
}
