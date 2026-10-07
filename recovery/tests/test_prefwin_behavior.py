"""Host behavior controls over the current PrefWin production bodies.

These tests extract member/helper definitions from src/prefwin.cpp and compile
those exact bodies against small side-effect doubles. They check callback and
preference-control behavior only; they do not prove native ABI, game runtime,
or the unrelated UI services represented by the doubles.
"""

from pathlib import Path
import shutil
import subprocess

import pytest
from current_game_storage_fixture import current_game_storage_fixture


ROOT = Path(__file__).resolve().parents[2]
UNIT = ROOT / "src/prefwin.cpp"


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated production function: {signature}")


def extract_struct(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1] + ";"
    raise AssertionError(f"unterminated production declaration: {signature}")


def compile_and_run(tmp_path, text: str, stem: str) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    source = tmp_path / f"{stem}.cpp"
    executable = tmp_path / stem
    source.write_text(text)
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(executable)],
        check=True,
        capture_output=True,
        text=True,
    )
    return subprocess.run([str(executable)], capture_output=True, text=True)


REDRAW_FIXTURE = r'''
#include <cassert>
#include <vector>
std::vector<int> events;
struct CheckBox {
    int event;
    void on_redraw() { events.push_back(event); }
};
struct PrefWin {
    CheckBox bscPrefCheckBox_{5}, advPrefCheckBox_{6}, warnPrefCheckBox_{7};
    CheckBox warnContPrefCheckBox_{8}, autoPrefCheckBox_{9};
    CheckBox audioVisPrefCheckBox_{10}, mapPrefCheckBox_{11};
    void redraw_option() { events.push_back(1); }
    void redraw_volume() { events.push_back(2); }
    void hide_option_buttons() { events.push_back(3); }
    void update_pref_buttons() { events.push_back(4); }
    void on_redraw();
};
BODY
int main() {
    PrefWin window;
    window.on_redraw();
    const std::vector<int> expected{1,2,3,4,5,6,7,8,9,10,11};
    assert(events == expected);
}
'''


def test_on_redraw_calls_every_owned_control_in_original_order(tmp_path):
    body = extract_function(UNIT.read_text(), "void PrefWin::on_redraw()")
    result = compile_and_run(tmp_path, REDRAW_FIXTURE.replace("BODY", body), "prefwin-redraw")
    assert result.returncode == 0, result.stderr


def test_on_redraw_damage_control_detects_dropped_map_callback(tmp_path):
    body = extract_function(UNIT.read_text(), "void PrefWin::on_redraw()")
    omitted = "  mapPrefCheckBox_.on_redraw();\n"
    assert omitted in body
    damaged = body.replace(omitted, "", 1)
    result = compile_and_run(tmp_path, REDRAW_FIXTURE.replace("BODY", damaged), "prefwin-redraw-damaged")
    assert result.returncode != 0


PREFERENCE_FIXTURE = r'''
#include <cassert>
#include <cstdint>
struct PrefWin {
    uint32_t prefsBasicPending_, morePrefsBasicPending_;
    uint32_t prefsAdvPending_, morePrefsAdvPending_;
    uint32_t prefsAutoPending_, morePrefsAutoPending_;
    uint32_t warningsPending_;
    uint32_t prefsAVPending_, morePrefsAVPending_;
    uint32_t prefsMapPending_, morePrefsMapPending_;
};
struct CheckBox {
    int values[16]{};
    void set_state_id(int id, int value) { values[id] = value; }
    int get_state_id(int id) { return values[id]; }
};
ENTRY
HELPERS
int main() {
    const CheckEntry entries[] = {
        {7, 0x0001, 0, false}, {8, 0x0004, 1, false},
        {9, 0x0008, 0, true},  {10, 0x0010, 1, true},
    };
    PrefWin prefs{};
    CheckBox controls{};
    prefs.prefsBasicPending_ = 0x0009;
    prefs.morePrefsBasicPending_ = 0x0004;
    read_controls(&prefs, controls, entries, 4);
    assert(controls.values[7] == 1);
    assert(controls.values[8] == 1);
    assert(controls.values[9] == 0);  // inverted bit is set
    assert(controls.values[10] == 1); // inverted bit is clear

    PrefWin edited{};
    CheckBox edited_controls{};
    edited_controls.values[7] = 1;
    edited_controls.values[8] = 1;
    edited_controls.values[9] = 0;
    edited_controls.values[10] = 1;
    write_controls(&edited, edited_controls, entries, 4);
    assert(edited.prefsBasicPending_ == 0x0009);
    assert(edited.morePrefsBasicPending_ == 0x0004);
}
'''


def preference_source(*, damage_inversion=False):
    source = UNIT.read_text()
    entry = extract_struct(source, "struct CheckEntry")
    helpers = "\n".join(
        extract_function(source, signature)
        for signature in (
            "uint32_t &pref_word(PrefWin *self, unsigned int group)",
            "void read_controls(PrefWin *self, CheckBox &box, const CheckEntry *items,",
            "void write_controls(PrefWin *self, CheckBox &box, const CheckEntry *items,",
        )
    )
    if damage_inversion:
        original = "(set != items[i].inverted) ? 1 : 0"
        assert original in helpers
        helpers = helpers.replace(original, "(set == items[i].inverted) ? 1 : 0", 1)
    return ("#include <cstdint>\n" + PREFERENCE_FIXTURE
            .replace("ENTRY", entry)
            .replace("HELPERS", helpers))


def test_preference_bit_controls_round_trip_with_inverted_options(tmp_path):
    result = compile_and_run(tmp_path, preference_source(), "prefwin-preferences")
    assert result.returncode == 0, result.stderr


def test_preference_bit_damage_control_catches_inversion_error(tmp_path):
    result = compile_and_run(
        tmp_path, preference_source(damage_inversion=True), "prefwin-preferences-damaged"
    )
    assert result.returncode != 0


SCROLL_FIXTURE = r'''
#include <cassert>
#include <vector>
struct WaveDevice {
    std::vector<int> calls;
    void set_volume(int p) { calls.push_back(0x50000000 | p); }
    void set_group_volume(int g, int p) { calls.push_back(0x51000000 | (g << 16) | p); }
} WaveDeviceGlobal;
struct Music { std::vector<int> *calls; void set_volume(int p) { calls->push_back(0x53000000 | p); } };
Music *FactionMusic;
struct PrefWin { void on_scrolling(int control_id, int position); };
BODY
int main() {
    PrefWin window;
    Music music{&WaveDeviceGlobal.calls};
    FactionMusic = &music;
    window.on_scrolling(0x50, 0x61);
    window.on_scrolling(0x51, 0x62);
    window.on_scrolling(0x52, 0x63);
    window.on_scrolling(0x53, 0x64);
    window.on_scrolling(0x54, 0x65);
    const std::vector<int> expected{
        0x50000061, 0x51000062, 0x51010063, 0x53000064
    };
    assert(WaveDeviceGlobal.calls == expected);
    FactionMusic = nullptr;
    window.on_scrolling(0x53, 0x66);
    assert(WaveDeviceGlobal.calls == expected);
}
'''


def test_scrolling_routes_each_control_to_its_volume_owner(tmp_path):
    body = extract_function(UNIT.read_text(), "void PrefWin::on_scrolling(int control_id, int position)")
    result = compile_and_run(tmp_path, SCROLL_FIXTURE.replace("BODY", body), "prefwin-scrolling")
    assert result.returncode == 0, result.stderr


ALPHA = ROOT / "src/alpha.h"


def extract_braced(source: str, marker: str, *, suffix: str = "") -> str:
    start = source.index(marker)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1] + suffix
    raise AssertionError(f"unterminated production region: {marker}")


def extract_enum(alpha: str, name: str) -> str:
    return extract_braced(alpha, f"enum {name}", suffix=";")


def extract_table(source: str, name: str) -> str:
    declaration = f"const CheckEntry {name}[] ="
    return extract_braced(source, declaration, suffix=";")


def audio_harness(*, add_ghost_ids: bool = False) -> str:
    production = UNIT.read_text()
    alpha = ALPHA.read_text()

    # These are the actual declarations and numeric values in alpha.h, not a
    # second copy of the preference-bit algorithm.
    enums = "\n".join(
        extract_enum(alpha, enum_name)
        for enum_name in (
            "GameBaseWarningsBitfield",
            "GamePreferencesBitfield",
            "GameMorePreferencesBitfield",
        )
    )
    check_entry = extract_braced(production, "struct CheckEntry", suffix=";")
    table_names = (
        "kBasic", "kAdvanced", "kAutomation", "kAudioVisual", "kMap",
        "kWarnings", "kWarningContinue",
    )
    tables = {name: extract_table(production, name) for name in table_names}
    if add_ghost_ids:
        marker = "const CheckEntry kAudioVisual[] = {"
        ghosts = (
            "\n    {0x3D, PREF_AV_VOLUME_MUSIC_TOGGLE, 6, false},"
            "\n    {0x3E, PREF_AV_VOLUME_SFX_TOGGLE, 6, false},"
            "\n    {0x3F, MPREF_AV_VOICEOVER_TECH_FAC, 7, false},"
        )
        tables["kAudioVisual"] = tables["kAudioVisual"].replace(
            marker, marker + ghosts, 1
        )

    helpers = "\n\n".join(
        extract_function(production, signature)
        for signature in (
            "uint32_t &pref_word(PrefWin *self, unsigned int group)",
            "void read_controls(PrefWin *self, CheckBox &box, const CheckEntry *items,",
            "void write_controls(PrefWin *self, CheckBox &box, const CheckEntry *items,",
        )
    )
    methods = "\n\n".join(
        extract_function(production, signature)
        for signature in (
            "void PrefWin::set_preferences()",
            "void PrefWin::get_preferences()",
        )
    )

    return r'''
#include <cassert>
#include <cstdint>
#include <map>
#include <set>
#include <utility>
#include <vector>

ENUMS

CHECK_ENTRY
TABLES

struct CheckBox {
    std::set<int> allowed;
    std::map<int, int> state;

    void allow_entries(const CheckEntry *entries, unsigned int count) {
        for (unsigned int i = 0; i < count; ++i)
            allowed.insert(entries[i].id);
    }
    void set_state_id(int id, int value) {
        // These three absent IDs were the former volume-toggle ghost entries.
        assert(id != 0x3D && id != 0x3E && id != 0x3F);
        assert(allowed.count(id) == 1);
        state[id] = value;
    }
    int get_state_id(int id) {
        assert(id != 0x3D && id != 0x3E && id != 0x3F);
        assert(allowed.count(id) == 1);
        return state[id];
    }
};

struct Toggle { int field_A18_ = 0; };
struct TextEvent { int button; int state; };
struct PrefWin {
    uint32_t prefsBasicPending_ = 0, morePrefsBasicPending_ = 0;
    uint32_t prefsAdvPending_ = 0, morePrefsAdvPending_ = 0;
    uint32_t prefsAutoPending_ = 0, morePrefsAutoPending_ = 0;
    uint32_t prefsAVPending_ = 0, morePrefsAVPending_ = 0;
    uint32_t prefsMapPending_ = 0, morePrefsMapPending_ = 0;
    uint32_t warningsPending_ = 0;
    CheckBox bscPrefCheckBox_, advPrefCheckBox_, autoPrefCheckBox_;
    CheckBox audioVisPrefCheckBox_, mapPrefCheckBox_;
    CheckBox warnPrefCheckBox_, warnContPrefCheckBox_;
    Toggle volumeVoiceToggle_, volumeSFXToggle_, volumeMusicToggle_;
    std::vector<TextEvent> text_events_;

    void change_volume_button_text(int button) {
        int value = -1;
        if (button == 0x57) value = volumeVoiceToggle_.field_A18_;
        if (button == 0x56) value = volumeSFXToggle_.field_A18_;
        if (button == 0x58) value = volumeMusicToggle_.field_A18_;
        text_events_.push_back({button, value});
    }
    void set_preferences();
    void get_preferences();

    void permit_production_ids() {
        bscPrefCheckBox_.allow_entries(kBasic, sizeof(kBasic) / sizeof(kBasic[0]));
        advPrefCheckBox_.allow_entries(kAdvanced, sizeof(kAdvanced) / sizeof(kAdvanced[0]));
        autoPrefCheckBox_.allow_entries(kAutomation, sizeof(kAutomation) / sizeof(kAutomation[0]));
        audioVisPrefCheckBox_.allow_entries(kAudioVisual, sizeof(kAudioVisual) / sizeof(kAudioVisual[0]));
        mapPrefCheckBox_.allow_entries(kMap, sizeof(kMap) / sizeof(kMap[0]));
        warnPrefCheckBox_.allow_entries(kWarnings, sizeof(kWarnings) / sizeof(kWarnings[0]));
        warnPrefCheckBox_.allowed.insert(0x35);
        warnContPrefCheckBox_.allow_entries(kWarningContinue, sizeof(kWarningContinue) / sizeof(kWarningContinue[0]));
    }
};

HELPERS
METHODS

int main() {
    PrefWin prefs{};
    prefs.permit_production_ids();
    assert(prefs.audioVisPrefCheckBox_.allowed.count(0x3D) == 0);
    assert(prefs.audioVisPrefCheckBox_.allowed.count(0x3E) == 0);
    assert(prefs.audioVisPrefCheckBox_.allowed.count(0x3F) == 0);

    // Voice enabled, music/SFX disabled: load through production, then save
    // through production and ensure unrelated UI checkboxes do not invent bits.
    prefs.morePrefsAVPending_ = MPREF_AV_VOICEOVER_TECH_FAC;
    prefs.prefsAVPending_ = 0;
    prefs.get_preferences();
    assert(prefs.volumeVoiceToggle_.field_A18_ == 1);
    assert(prefs.volumeSFXToggle_.field_A18_ == 0);
    assert(prefs.volumeMusicToggle_.field_A18_ == 0);
    const std::vector<TextEvent> expected_text{
        {0x57, 1}, {0x56, 0}, {0x58, 0}
    };
    assert(prefs.text_events_.size() == expected_text.size());
    for (unsigned int i = 0; i < expected_text.size(); ++i) {
        assert(prefs.text_events_[i].button == expected_text[i].button);
        assert(prefs.text_events_[i].state == expected_text[i].state);
    }

    prefs.set_preferences();
    assert((prefs.morePrefsAVPending_ & MPREF_AV_VOICEOVER_TECH_FAC) != 0);
    assert((prefs.prefsAVPending_ & PREF_AV_VOLUME_SFX_TOGGLE) == 0);
    assert((prefs.prefsAVPending_ & PREF_AV_VOLUME_MUSIC_TOGGLE) == 0);

    // The restore-default branch calls get_preferences after setting the
    // volume controls. Exercise that actual production read path from stale
    // (all-off) UI state and require its ordered label callbacks to refresh.
    prefs.text_events_.clear();
    prefs.volumeVoiceToggle_.field_A18_ = 0;
    prefs.volumeSFXToggle_.field_A18_ = 0;
    prefs.volumeMusicToggle_.field_A18_ = 0;
    prefs.morePrefsAVPending_ = MPREF_AV_VOICEOVER_TECH_FAC;
    prefs.prefsAVPending_ = PREF_AV_VOLUME_SFX_TOGGLE | PREF_AV_VOLUME_MUSIC_TOGGLE;
    prefs.get_preferences();
    assert(prefs.volumeVoiceToggle_.field_A18_ == 1);
    assert(prefs.volumeSFXToggle_.field_A18_ == 1);
    assert(prefs.volumeMusicToggle_.field_A18_ == 1);
    assert(prefs.text_events_.size() == 3);
    assert(prefs.text_events_[0].button == 0x57 && prefs.text_events_[0].state == 1);
    assert(prefs.text_events_[1].button == 0x56 && prefs.text_events_[1].state == 1);
    assert(prefs.text_events_[2].button == 0x58 && prefs.text_events_[2].state == 1);
}
'''.replace("ENUMS", enums).replace("CHECK_ENTRY", check_entry).replace(
        "TABLES", "\n".join(tables.values())
    ).replace("HELPERS", helpers).replace("METHODS", methods)


def test_audio_volume_round_trip_and_restore_refreshes_labels(tmp_path):
    harness = audio_harness()
    result = compile_and_run(tmp_path, harness, "prefwin-audio-behavior")
    assert result.returncode == 0, result.stderr


def test_audio_table_ghost_id_damage_control_fails(tmp_path):
    # Inject the former volume entries into the extracted production table.
    # The MockCheckBox rejects IDs 0x3D/0x3E/0x3F even if a damaged table lists
    # them, so the regression cannot silently make the mock accept the bug.
    harness = audio_harness(add_ghost_ids=True)
    result = compile_and_run(tmp_path, harness, "prefwin-audio-ghost-damaged")
    assert result.returncode != 0


DISPLAY_FIXTURE = r'''
#include <cassert>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
struct Style { int index; };
struct Event { std::string name; int a, b; const void *p; };
std::vector<Event> events;
std::vector<RECT> drawn_rects;
std::vector<int> drawn_styles;
void event(const char *name, int a = 0, int b = 0, const void *p = nullptr) {
    events.push_back({name, a, b, p});
}

struct Win {
    static const Win *focus;
    static void set_def_focus(Win *target) { focus = target; event("focus", 0, 0, target); }
    void show(int mode) { event("win_show", mode); }
};
const Win *Win::focus = nullptr;
struct GraphicWin : Win {};
struct Menu { void hide() { event("menu_hide"); } };
struct Console { Menu alpha_menu_; };
Console ConsoleGlobal;

struct Buffer {
    static int box_sprite(RECT *rect, Style *style) {
        drawn_rects.push_back(*rect);
        drawn_styles.push_back(style->index);
        event("box", style->index);
        return 0;
    }
};
Style InterfaceBoxStyles[19] = {
    {0},{1},{2},{3},{4},{5},{6},{7},{8},{9},
    {10},{11},{12},{13},{14},{15},{16},{17},{18}
};

/* ACTUAL_GAME_STORAGE */
int DialogDefaultStyle = 0;
int GamePreferencesAfterModal = -1;
const int PrefWinDisplayWidth = 700, PrefWinDisplayHeight = 390;
const int PrefWinButtonWidth = 220, PrefWinNarrowDisplayVerticalOffset = 50;
int PrefWinGeometryValue_857888 = 10, PrefWinGeometryValue_857884 = 20;
int PrefWinGeometryValue_8578C0 = 100;
int PrefWinGeometryValue_85789C = 40, PrefWinGeometryValue_857898 = 30;
int PrefWinGeometryValue_8578A8 = 50, PrefWinGeometryValue_8578A4 = 60;
int PrefWinGeometryValue_8577D8 = 70, PrefWinGeometryValue_8577DC = 80;
int PrefWinGeometryValue_8577E4 = 90, PrefWinGeometryValue_8577E0 = 100;

int pop_wait() { return 0; }
void slide_show(GraphicWin *, int mode) { event("slide_show", mode); }

struct Toggle { int field_A18_ = 0; };
struct Button {
    const char *name;
    void show(int mode) { event(name, mode); }
};
struct ButtonGroup { void button_click(int page) { event("page_click", page); } };
ButtonGroup PrefWinButtonGroup;

struct PrefWin : GraphicWin {
    int prefsFinal_ = 0, morePrefsFinal_ = 0, warningsFinal_ = 0;
    int prefsBasicPending_ = 0, prefsAdvPending_ = 0, prefsAutoPending_ = 0;
    int prefsAVPending_ = 0, prefsMapPending_ = 0;
    int morePrefsBasicPending_ = 0, morePrefsAdvPending_ = 0;
    int morePrefsAutoPending_ = 0, morePrefsAVPending_ = 0, morePrefsMapPending_ = 0;
    int warningsPending_ = 0;
    int field_A14_ = 4, field_A18_ = 0;
    Toggle volumeMainToggle_, volumeVoiceToggle_, volumeSFXToggle_, volumeMusicToggle_;
    Button restoreDefaultButton_{"show_restore"}, okButton_{"show_ok"}, cancelButton_{"show_cancel"};
    void get_preferences() { event("get_preferences"); }
    void change_volume_button_text(int button) {
        event("volume_text", button, volumeMainToggle_.field_A18_);
    }
    void redraw_option();
    void redraw_volume();
    void redraw() { event("redraw"); }
    void show(int mode) { Win::show(mode); }
    int set_modal(int mode, int (*callback)(), Win *owner) {
        event("modal", mode, callback == pop_wait, owner);
        if (GamePreferencesAfterModal != -1)
            GameDataStateStorage.fields_.game_preferences_ = GamePreferencesAfterModal;
        return 0;
    }
    void display(int page);
};
PrefWin ConsolePrefWin;

PREFERENCE_WINDOW_HELPER
REDRAW_OPTION
REDRAW_VOLUME
DISPLAY_BODY

void assert_tags(const std::vector<std::string> &expected) {
    assert(events.size() == expected.size());
    for (unsigned int i = 0; i < expected.size(); ++i)
        assert(events[i].name == expected[i]);
}
void run_case(int page, int preferences, int post_modal_preferences,
              int initial_style, int expected_style) {
    events.clear(); drawn_rects.clear(); drawn_styles.clear();
    GameDataStateStorage.fields_.game_preferences_ = preferences;
    GamePreferencesAfterModal = post_modal_preferences;
    DialogDefaultStyle = initial_style;
    ConsolePrefWin.field_A14_ = 4;
    ConsolePrefWin.volumeVoiceToggle_.field_A18_ = 1;
    ConsolePrefWin.volumeSFXToggle_.field_A18_ = 0;
    ConsolePrefWin.volumeMusicToggle_.field_A18_ = 1;
    ConsolePrefWin.display(page);

    const std::vector<int> expected_styles{9, 9, 18, 18, 9};
    assert(drawn_styles == expected_styles);
    const std::vector<RECT> expected_rects{
        {0x17, 0, 723, 390},
        {0x1B, 4, 719, 386},
        {10, 20, 230, 120},
        {40, 30, 90, 90},
        {70, 30, 160, 130},
    };
    assert(drawn_rects.size() == expected_rects.size());
    for (unsigned int i = 0; i < expected_rects.size(); ++i) {
        assert(drawn_rects[i].left == expected_rects[i].left);
        assert(drawn_rects[i].top == expected_rects[i].top);
        assert(drawn_rects[i].right == expected_rects[i].right);
        assert(drawn_rects[i].bottom == expected_rects[i].bottom);
    }

    std::vector<std::string> expected_tags{
        "menu_hide", "box", "box", "box", "get_preferences", "volume_text",
        "box", "box", "show_restore", "show_ok", "show_cancel"
    };
    if (page >= 0) expected_tags.push_back("page_click");
    expected_tags.insert(expected_tags.end(), {
        "redraw", "slide_show", "win_show", "focus", "modal"
    });
    assert_tags(expected_tags);
    assert(events[5].a == 0x55 && events[5].b == 1); // OR of voice/SFX/music
    assert(events[8].a == 0 && events[9].a == 0 && events[10].a == 0);
    if (page >= 0) assert(events[11].a == page);
    const unsigned int focus_index = page >= 0 ? 15 : 14;
    const unsigned int modal_index = page >= 0 ? 16 : 15;
    assert(events[focus_index].p == static_cast<const Win *>(&ConsolePrefWin));
    assert(events[modal_index].a == 0 && events[modal_index].b == 1 && events[modal_index].p == nullptr);
    assert(DialogDefaultStyle == expected_style);
}

int main() {
    // A negative page skips group selection; absent preference bit sets style bit 4.
    run_case(-1, 0, 0x20000000, 0x20, 0x20);
    // A real page selects before redraw. Modal processing changes the live
    // bit, and the post-modal branch must observe that new value.
    run_case(3, 0x20000000, 0, 0x24, 0x24);
}
'''


def display_source(*, mutations=()):
    source = UNIT.read_text()
    display = extract_function(source, "void PrefWin::display(int page)")
    mutations = dict(mutations)
    for old, new in mutations.items():
        assert old in display, f"mutant anchor not found: {old}"
        display = display.replace(old, new, 1)
    return (DISPLAY_FIXTURE.replace(
            "/* ACTUAL_GAME_STORAGE */", current_game_storage_fixture())
            .replace("PREFERENCE_WINDOW_HELPER", extract_function(source, "Win *preference_window()"))
            .replace("REDRAW_OPTION", extract_function(source, "void PrefWin::redraw_option()"))
            .replace("REDRAW_VOLUME", extract_function(source, "void PrefWin::redraw_volume()"))
            .replace("DISPLAY_BODY", display))


def test_display_draw_order_focus_modal_and_postmodal_preferences(tmp_path):
    result = compile_and_run(tmp_path, display_source(), "prefwin-display")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("mutation", [
    ("Buffer::box_sprite(&outer, &InterfaceBoxStyles[9]);",
     "Buffer::box_sprite(&outer, &InterfaceBoxStyles[18]);"),
    ("restoreDefaultButton_.show(0);", "restoreDefaultButton_.show(1);"),
    ("Win::set_def_focus(preference_window());", "Win::set_def_focus(nullptr);"),
    ("  show(3);\n  Win::set_def_focus(preference_window());",
     "  Win::set_def_focus(preference_window());\n  show(3);"),
    ("if (!(GameDataStateStorage.fields_.game_preferences_ & 0x20000000))",
     "if (GameDataStateStorage.fields_.game_preferences_ & 0x20000000)"),
])
def test_display_behavior_mutants_are_detected(tmp_path, mutation):
    result = compile_and_run(
        tmp_path, display_source(mutations=(mutation,)), "prefwin-display-mutant"
    )
    assert result.returncode != 0


DESTROY_FIXTURE = r'''
#include <cassert>
#include <string>
#include <utility>
#include <vector>
struct Event { std::string name; int a, b; const void *p; };
std::vector<Event> events;
void event(const char *name, int a = 0, int b = 0, const void *p = nullptr) {
    events.push_back({name, a, b, p});
}
struct Win {
    static const Win *focus;
    static void set_def_focus(Win *target) { focus = target; event("focus", 0, 0, target); }
    void hide() { event("hide"); }
};
const Win *Win::focus = nullptr;
struct GraphicWin : Win {};
struct Console : GraphicWin {};
Console ConsoleGlobal;
struct IniPrefs { int preferences = 0, more_preferences = 0, announce = 0; } AlphaIniPrefs;
int UnkBitfield1 = 0;
void draw_map(int value) { event("draw_map", value); }
void slide_hide(GraphicWin *window, int mode) { event("slide_hide", mode, 0, window); }
void prefs_use() { event("prefs_use"); }
void prefs_save(int value) { event("prefs_save", value); }
std::vector<std::pair<std::string, int>> stored_preferences;
void prefs_put(const char *key, int value, int flags) {
    assert(flags == 0);
    stored_preferences.push_back({key, value});
    event("prefs_put", value);
}
Win *console_window() { return static_cast<Win *>(static_cast<GraphicWin *>(&ConsoleGlobal)); }
struct Scroll { int value; int position() const { return value; } };
struct Button { int field_A9C_ = 1; };
struct PrefWin : GraphicWin {
    int field_A5C_ = 0;
    int prefsFinal_ = 0, morePrefsFinal_ = 0, warningsFinal_ = 0;
    int prefsBasicPending_ = 0, prefsAdvPending_ = 0, prefsAutoPending_ = 0;
    int prefsAVPending_ = 0, prefsMapPending_ = 0;
    int morePrefsBasicPending_ = 0, morePrefsAdvPending_ = 0;
    int morePrefsAutoPending_ = 0, morePrefsAVPending_ = 0, morePrefsMapPending_ = 0;
    int warningsPending_ = 0;
    Scroll volumeMainScroll_{101}, volumeVoiceScroll_{102};
    Scroll volumeMusicScroll_{103}, volumeSFXScroll_{104};
    Button okButton_, cancelButton_;
    void set_preferences() { event("set_preferences"); }
    void release_modal() { event("release_modal"); }
    void destroy();
};
DESTROY_BODY
void assert_tags(const std::vector<std::string> &expected) {
    assert(events.size() == expected.size());
    for (unsigned int i = 0; i < expected.size(); ++i)
        assert(events[i].name == expected[i]);
}
int main() {
    PrefWin accepted{};
    accepted.field_A5C_ = 1;
    accepted.prefsFinal_ = 0x10; accepted.morePrefsFinal_ = 0x20; accepted.warningsFinal_ = 0x40;
    accepted.prefsBasicPending_ = 1; accepted.prefsAdvPending_ = 2;
    accepted.morePrefsBasicPending_ = 4; accepted.warningsPending_ = 8;
    accepted.destroy();
    assert(accepted.field_A5C_ == 0);
    assert(accepted.prefsFinal_ == 0x13 && AlphaIniPrefs.preferences == 0x13);
    assert(accepted.morePrefsFinal_ == 0x24 && AlphaIniPrefs.more_preferences == 0x24);
    assert(accepted.warningsFinal_ == 0x48 && AlphaIniPrefs.announce == 0x48);
    assert(accepted.okButton_.field_A9C_ == 0 && accepted.cancelButton_.field_A9C_ == 0);
    assert(UnkBitfield1 == 4);
    const std::vector<std::pair<std::string, int>> expected_stored{
        {"Main Volume", 101}, {"Voice Volume", 102},
        {"Music Volume", 103}, {"SFX Volume", 104}
    };
    assert(stored_preferences == expected_stored);
    assert_tags({"set_preferences", "prefs_use", "prefs_save", "prefs_put", "prefs_put",
        "prefs_put", "prefs_put", "draw_map", "slide_hide", "hide", "focus", "release_modal"});
    assert(events[8].p == static_cast<const GraphicWin *>(&accepted));
    assert(events[10].p == static_cast<const Win *>(&ConsoleGlobal));

    events.clear(); stored_preferences.clear();
    PrefWin cancelled{};
    cancelled.destroy();
    assert(stored_preferences.empty());
    assert_tags({"draw_map", "slide_hide", "hide", "focus", "release_modal"});
    assert(events[1].p == static_cast<const GraphicWin *>(&cancelled));
    assert(events[3].p == static_cast<const Win *>(&ConsoleGlobal));
}
'''


def destroy_source(*, mutations=()):
    source = UNIT.read_text()
    body = extract_function(source, "void PrefWin::destroy()")
    for old, new in mutations:
        assert old in body, f"mutant anchor not found: {old}"
        body = body.replace(old, new, 1)
    return DESTROY_FIXTURE.replace("DESTROY_BODY", body)


def test_destroy_persists_accepted_preferences_then_closes_in_order(tmp_path):
    result = compile_and_run(tmp_path, destroy_source(), "prefwin-destroy")
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("mutation", [
    ("    field_A5C_ = 0;", "    field_A5C_ = 1;"),
    ("prefsFinal_ |=", "prefsFinal_ ="),
    ("    prefs_put(\"SFX Volume\", volumeSFXScroll_.position(), 0);\n", ""),
])
def test_destroy_persistence_mutants_are_detected(tmp_path, mutation):
    result = compile_and_run(
        tmp_path, destroy_source(mutations=(mutation,)), "prefwin-destroy-mutant"
    )
    assert result.returncode != 0
