// Self-contained host projection of the full source-extracted 0x0048C650 body.
#include <cassert>
#include <stdint.h>
#include <string.h>

#define __cdecl
typedef const char *LPCSTR;

struct Base {
    int16_t x, y;
    uint8_t faction_id_current, former, population_size, rest[4], visibility;
    uint8_t facilities_built[12];
    uint32_t state;
    char name_string[25];
};
struct PlayerData { uint32_t flags; };

struct Sprite {
    static int constructors, destructors, init_count;
    Sprite() { ++constructors; }
    ~Sprite() { ++destructors; }
    int init(char *, int, int) { ++init_count; return 0; }
};
int Sprite::constructors = 0;
int Sprite::destructors = 0;
int Sprite::init_count = 0;

struct Buffer {
    static int get_pcx_dimensions(const char *, int *width, int *height) {
        *width = 30;
        *height = 40;
        return 0;
    }
};

struct Dialogs {
    static int constructors, destructors, total_items, total_sprites;
    static Sprite *last_sprite;
    Dialogs() { ++constructors; }
    ~Dialogs() { ++destructors; }
    int item(char *, int) { ++total_items; return 0; }
    int sprite(Sprite *value, char *, int) {
        ++total_sprites;
        last_sprite = value;
        return 0;
    }
};
int Dialogs::constructors = 0;
int Dialogs::destructors = 0;
int Dialogs::total_items = 0;
int Dialogs::total_sprites = 0;
Sprite *Dialogs::last_sprite = 0;

struct Popup {
    static int constructors, destructors, starts, buttons, execs;
    static int input_state, auto_hide, start_value, picture_set;
    static int sprite_configures, owned_picture_selected, result;
    static Sprite *selected_picture;
    Dialogs dialog;
    Sprite picture;

    Popup() { ++constructors; }
    ~Popup() { ++destructors; }
    void start(char *, char *, int value, char *, int, int) {
        ++starts;
        start_value = value;
    }
    Dialogs &dialogs() { return dialog; }
    int button(char *) { ++buttons; return 0; }
    int exec(int, int) { ++execs; return result; }
    uint32_t input_poll_state() const { return input_state; }
    Sprite &owned_picture() { return picture; }
    void select_owned_picture() {
        ++owned_picture_selected;
        selected_picture = &picture;
    }
    void set_caller_picture(Sprite *value) {
        ++picture_set;
        selected_picture = value;
        if (value == &picture) ++owned_picture_selected;
    }
    void configure_sprite_dialog(Sprite *value) {
        ++sprite_configures;
        dialog.sprite(value, 0, 0);
    }
    void set_value(int value) { start_value = value; }
    void set_auto_hide_value(uint32_t value) { auto_hide = static_cast<int>(value); }
};
int Popup::constructors = 0;
int Popup::destructors = 0;
int Popup::starts = 0;
int Popup::buttons = 0;
int Popup::execs = 0;
int Popup::input_state = 0;
int Popup::auto_hide = 0;
int Popup::start_value = -2;
int Popup::picture_set = 0;
int Popup::sprite_configures = 0;
int Popup::owned_picture_selected = 0;
int Popup::result = 0;
Sprite *Popup::selected_picture = 0;

struct FX {
    int plays, fades;
    FX() : plays(0), fades(0) {}
    void play(int) { ++plays; }
    void fade(int) { ++fades; }
};
struct Console {
    int faction_id_, focuses;
    Console() : faction_id_(0), focuses(0) {}
    void focus(int, int, int) { ++focuses; }
};

Base base_current;
Base Bases[4];
int BaseIDCurrentSelected = 1;
Base *BaseCurrent = &base_current;
PlayerData PlayersData[4];
uint32_t LockEnableMask = 0;
uint32_t GameState = 0;
uint32_t GameWarnings = 0;
uint32_t GameMorePreferences = 0;
int CurrentFactionEnergyReserves = 0;
int BaseProductionZoomRequested = 0;
int IsMultiplayerNet = 0;
const uint32_t STATE_UNK_1 = 1;
const uint32_t STATE_UNK_8 = 8;
const uint32_t STATE_OMNISCIENT_VIEW = 0x80;
const uint32_t MPREF_ADV_ZOOM_BASE_NO_RECENTER_MAP = 4;
const uint32_t BSTATE_UNK_8000000000 = 0x80000000u;
const uint32_t BSTATE_RESEARCH_DATA_STOLEN = 0x40;
int prefs_saved = 0;
int staples = 0;
int parses = 0;
int bitmask_call_count = 0;
int parse_sets_energy_to_zero = 0;
char PopupStartCaption[] = "POPUP";
FX g_FX;
Console ConsoleGlobal;
struct AlphaPrefs { uint32_t announce; } AlphaIniPrefs;

void parse_says(int, const char *, int, int) {
    ++parses;
    if (parse_sets_energy_to_zero) CurrentFactionEnergyReserves = 0;
}
char *label_get(int index) {
    static char labels[4][16] = {"zero", "one", "two", "three"};
    return labels[index & 3];
}
void bitmask_call(int, int *offset, int *mask) {
    ++bitmask_call_count;
    *offset = 0;
    *mask = 1;
}
void prefs_save(int) { ++prefs_saved; }
void action_staple(int) { ++staples; }

/*__POPUP_BODY__*/

int main() {
    strcpy(base_current.name_string, "Base");
    base_current.faction_id_current = 0;
    base_current.population_size = 2;
    base_current.x = 12;
    base_current.y = 14;
    Bases[1].faction_id_current = 2;
    Bases[1].visibility = 1;
    Bases[1].facilities_built[0] = 1;
    PlayersData[0].flags = 0xFFFFFFFFu;
    ConsoleGlobal.faction_id_ = 0;

    // Gate 1: local faction is not enabled.
    assert(popb("BODY", 1, 7, "image.pcx", 0) == 0);
    assert(Popup::constructors == 1 && Popup::destructors == 1);
    assert(Dialogs::constructors == 1 && Dialogs::destructors == 1);
    assert(Sprite::constructors == 1 && Sprite::destructors == 1);
    assert(parses == 0 && Popup::starts == 0);

    // Gate 2: state bit 1 without state bit 8 suppresses the popup.
    LockEnableMask = 1;
    GameState = 1;
    assert(popb("BODY", 1, 7, "image.pcx", 0) == 0);
    assert(parses == 0 && Popup::starts == 0 && Popup::execs == 0);

    // Gate 3: enabled non-viewer faction with no energy cannot open it.
    base_current.faction_id_current = 1;
    LockEnableMask = 3;
    GameState = 8;
    CurrentFactionEnergyReserves = 0;
    assert(popb("BODY", 1, 7, "image.pcx", 0) == 0);
    assert(parses == 0 && Popup::starts == 0 && Popup::execs == 0);
    base_current.faction_id_current = 0;

    // Facility bit contributes exactly +2 to the signed PlayerData flags
    // threshold: -1 becomes 1, so the optional third dialog row is added.
    LockEnableMask = 1;
    GameState = 8;
    GameWarnings = 1;
    AlphaIniPrefs.announce = 1;
    Popup::result = 3;
    Popup::input_state = 1;
    assert(popb("BODY", 1, 7, "image.pcx", 0) == 3);
    assert(Popup::constructors == 4 && Popup::destructors == 4);
    assert(Dialogs::constructors == 4 && Dialogs::destructors == 4);
    assert(Sprite::constructors == 4 && Sprite::destructors == 4);
    assert(Popup::starts == 1 && Popup::execs == 1 && Popup::buttons == 1);
    assert(Dialogs::total_items == 3);
    assert(Popup::owned_picture_selected == 1 && Sprite::init_count == 1);
    assert(Popup::selected_picture != 0);
    assert(Popup::auto_hide == 1002 && Popup::start_value == 0);
    assert(g_FX.plays == 1 && g_FX.fades == 1);
    assert(ConsoleGlobal.focuses == 1 && parses == 1 && staples == 1);
    assert(GameWarnings == 0 && AlphaIniPrefs.announce == 0 && prefs_saved == 1);

    // Without the facility +2, signed -1 remains below zero and the third row
    // is omitted. This distinguishes the original threshold from a no-op.
    Bases[1].facilities_built[0] = 0;
    Popup::result = 1;
    Popup::input_state = 0;
    assert(popb("BODY", 0, -1, 0, 0) == 1);
    assert(Dialogs::total_items == 5);
    Bases[1].facilities_built[0] = 1;

    // A stored warning-state value, not the exec result, controls persistence.
    GameWarnings = 1;
    AlphaIniPrefs.announce = 1;
    Popup::result = 3;
    Popup::input_state = 0;
    CurrentFactionEnergyReserves = 1;
    assert(popb("BODY", 1, 7, 0, 0) == 1); // energy overrides exec result 3
    assert(GameWarnings == 1 && AlphaIniPrefs.announce == 1 && prefs_saved == 1);
    assert(staples == 1 && g_FX.fades == 2);

    // After parse_says clears energy, selected-base visibility permits focus.
    base_current.faction_id_current = 1;
    Bases[1].faction_id_current = 2;
    Bases[1].visibility = 1;
    LockEnableMask = 3;
    GameWarnings = 1;
    CurrentFactionEnergyReserves = 1;
    parse_sets_energy_to_zero = 1;
    Popup::result = 2;
    Popup::input_state = 0;
    assert(popb("BODY", 1, -1, 0, 0) == 2);
    assert(ConsoleGlobal.focuses == 2);

    // Neither ownership nor visibility allows focus, and the preference bit
    // independently suppresses focus even when visibility does allow it.
    Bases[1].visibility = 0;
    CurrentFactionEnergyReserves = 1;
    Popup::result = 1;
    assert(popb("BODY", 1, -1, 0, 0) == 1);
    assert(ConsoleGlobal.focuses == 2);
    Bases[1].visibility = 1;
    GameMorePreferences = MPREF_ADV_ZOOM_BASE_NO_RECENTER_MAP;
    CurrentFactionEnergyReserves = 1;
    assert(popb("BODY", 1, -1, 0, 0) == 1);
    assert(ConsoleGlobal.focuses == 2);

    base_current.faction_id_current = 0;
    Bases[1].faction_id_current = 2;
    Bases[1].visibility = 1;
    CurrentFactionEnergyReserves = 0;
    parse_sets_energy_to_zero = 0;
    GameMorePreferences = 0;

    Sprite caller_picture;
    Popup::result = 2;
    Popup::input_state = 0;
    assert(popb("BODY", 0, -1, 0, &caller_picture) == 2);
    assert(Dialogs::total_items == 8);
    assert(Popup::auto_hide == 2 && Popup::sprite_configures == 1);
    assert(Dialogs::total_sprites == 1 && Dialogs::last_sprite == &caller_picture);
    assert((BaseCurrent->state & 0x80000040u) == 0x80000040u);

    Popup::result = 0;
    assert(popb("BODY", 0, -1, 0, 0) == 0);
    assert(BaseProductionZoomRequested == 1);
    assert(Dialogs::total_items == 11);

    assert(Popup::constructors == 11 && Popup::destructors == 11);
    assert(Dialogs::constructors == 11 && Dialogs::destructors == 11);
    // One caller-owned test Sprite remains alive until main exits.
    assert(Sprite::constructors == 12 && Sprite::destructors == 11);
    assert(Popup::execs == 8 && bitmask_call_count > 0);
}
