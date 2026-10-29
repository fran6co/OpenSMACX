from pathlib import Path
import shutil
import subprocess

from match_recovery.source import function_line


WORKTREE = Path(__file__).resolve().parents[2]
SOURCE = WORKTREE / "src/infowin.cpp"


def production_definition():
    text = SOURCE.read_text(encoding="utf-8")
    line = function_line(text, "InfoWin::right_menu")
    assert line > 0
    lines = text.splitlines(keepends=True)
    start = sum(map(len, lines[: line - 1]))
    opening = text.index("{", start)
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                body = text[start : pos + 1]
                assert "BODY GOES HERE." not in body
                return body
    raise AssertionError("InfoWin::right_menu definition is not brace-balanced")


def test_actual_right_menu_preserves_menu_and_selection_effect_order(tmp_path):
    compiler = shutil.which("g++") or shutil.which("c++")
    if not compiler:
        return
    body = production_definition()
    harness = r'''
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
using std::string;
using std::vector;

static vector<string> events;
static int chosen_slot = 4;
static int chosen_id = 9;
static int throw_on_conversion = 0;
static int state9_calls = 0;
static int box_pane = -1;
static int box_visible0 = -1;
static int box_visible1 = -1;
static int draw_pane = -1;
static int *observed_visible0 = 0;
static int *observed_visible1 = 0;
static const int pop_expire = 77;
static const int STATE_UNK_800 = 0x800;
static int InterfaceBoxStyles[32];
struct RECT { int identity; };
struct GameFields { unsigned game_state_; };
struct GameDataState { GameFields fields_; };
static GameDataState GameDataStateStorage = {{0}};

static void event(const string &s) { events.push_back(s); }
static int label_get(int id) {
    event("label:" + std::to_string(id));
    return id;
}
struct PullDown {
    PullDown() { event("ctor"); }
    ~PullDown() { event("dtor"); }
    void init(int n) { event("init:" + std::to_string(n)); }
    void add_item(int id, int label) {
        event("item:" + std::to_string(id) + ":" + std::to_string(label));
    }
    void add_separator(int id) { event("separator:" + std::to_string(id)); }
    void show(int x, int y, int expiry) {
        event("show:" + std::to_string(x) + ":" + std::to_string(y) + ":" + std::to_string(expiry));
    }
    int get_selected() { event("selected"); return chosen_slot; }
    int item_id(int slot) {
        event("item_id:" + std::to_string(slot));
        return chosen_id;
    }
};

class MainInterface {
public:
    void client_to_screen(int *x, int *y) {
        event("convert");
        if (throw_on_conversion) throw std::runtime_error("conversion");
        *x += 10;
        *y += 20;
    }
    void box_sprite(RECT *rect, int *style) {
        (void)style;
        box_pane = rect->identity;
        box_visible0 = observed_visible0 ? *observed_visible0 : -1;
        box_visible1 = observed_visible1 ? *observed_visible1 : -1;
        event("box:" + std::to_string(box_pane));
    }
};
static MainInterface MainInterfaceGlobal;

class InfoWin {
public:
    void right_menu(int x, int y);
    int state_valid(int state) {
        event("state:" + std::to_string(state));
        if (state == 9) {
            ++state9_calls;
            if (state9_calls == 2) {
                active_pane_ = 1;
                return 0;
            }
            return 1;
        }
        return state == 0 || state == 2 || state == 5;
    }
    void setup_text() {
        event("setup");
        active_pane_ = 2;
    }
    void draw_text(int pane) {
        draw_pane = pane;
        event("draw:" + std::to_string(pane));
    }
    void help_any() { event("help"); }
    int active_pane_ = 0;
    int pane_states_[2] = {0, 6};
    RECT pane_rects_[2] = {{0}, {1}};
    int text_lengths_[2] = {12, 30};
    int visible_characters_[2] = {0, 0};
    int field_94_ = 1;
    int field_A8_ = 1;
};
''' + body + r'''

static void require(bool ok, const char *message) {
    if (!ok) throw std::runtime_error(message);
}
static bool has_menu_prefix(const vector<string> &expected) {
    if (events.size() < expected.size()) return false;
    for (size_t i = 0; i < expected.size(); ++i)
        if (events[i] != expected[i]) return false;
    return true;
}
static int at(const string &s) {
    for (int i = 0; i < static_cast<int>(events.size()); ++i)
        if (events[i] == s) return i;
    return -1;
}
static void normal_case() {
    events.clear(); state9_calls = 0; chosen_slot = 4; chosen_id = 9;
    GameDataStateStorage.fields_.game_state_ = 0;
    InfoWin w;
    observed_visible0 = w.visible_characters_;
    observed_visible1 = w.visible_characters_ + 1;
    w.right_menu(2, 3);
    const vector<string> menu_prefix = {
        "ctor", "convert", "init:0",
        "state:0", "label:587", "item:0:587",
        "state:1", "state:2", "label:589", "item:2:589",
        "state:3", "state:4", "state:5", "label:595", "item:5:595",
        "state:6", "state:7", "separator:99", "label:575", "item:20:575",
        "show:12:23:77", "selected", "item_id:4"
    };
    require(has_menu_prefix(menu_prefix), "constructor, conversion, state/label/item and selection order");
    require(at("state:0") < at("label:587") && at("label:587") < at("item:0:587"), "first state/label/item order");
    require(at("state:2") < at("label:589") && at("label:589") < at("item:2:589"), "second state/label/item order");
    require(at("item:5:595") < at("separator:99") && at("separator:99") < at("label:575") && at("label:575") < at("item:20:575") && at("item:20:575") < at("show:12:23:77"), "remaining menu order");
    require(at("show:12:23:77") < at("selected") && at("selected") < at("item_id:4"), "selection order");
    require(w.pane_states_[0] == 2 && w.pane_states_[1] == 6, "cached pane normalized in place");
    require(box_pane == 1, "box sprite uses live active pane after state validation");
    require(box_visible0 == 13 && box_visible1 == 31, "box sees expanded text budgets");
    require(w.visible_characters_[0] == 0 && w.visible_characters_[1] == 0, "visible budgets cleared before setup");
    require(draw_pane == 2 && at("setup") < at("draw:2"), "draw rereads pane changed by setup");
    require(at("box:1") < at("setup") && at("draw:2") < at("dtor"), "render and dtor order");
}
static void negative_slot_case() {
    events.clear(); state9_calls = 0; chosen_slot = -3; chosen_id = 20;
    GameDataStateStorage.fields_.game_state_ = 0;
    InfoWin w; w.right_menu(0, 0);
    require(at("item_id:-3") >= 0, "negative cancellation slot still reaches item_id");
    require(at("item_id:-3") < at("help") && at("help") < at("dtor"), "help action follows item translation");
}
static void upper_bound_case() {
    events.clear(); chosen_slot = 64; chosen_id = 20;
    GameDataStateStorage.fields_.game_state_ = 0;
    InfoWin w; w.right_menu(0, 0);
    require(at("selected") >= 0 && at("item_id:64") < 0 && at("dtor") >= 0, "slot 64 skips table lookup");
}
static void state_guard_case() {
    events.clear(); chosen_slot = 0;
    GameDataStateStorage.fields_.game_state_ = STATE_UNK_800;
    InfoWin w; w.right_menu(0, 0);
    require(at("show:10:20:77") < at("dtor"), "menu is shown before game-state early return");
    require(at("selected") < 0 && at("item_id:0") < 0, "blocked state skips selection");
}
static void exception_case() {
    events.clear(); throw_on_conversion = 1;
    GameDataStateStorage.fields_.game_state_ = 0;
    InfoWin w;
    bool caught = false;
    try { w.right_menu(0, 0); }
    catch (const std::runtime_error &) { caught = true; }
    throw_on_conversion = 0;
    require(caught, "conversion exception not propagated");
    require(events.size() == 3 && events[0] == "ctor" && events[1] == "convert" && events[2] == "dtor", "menu lifetime unwinds on conversion exception");
}
int main() {
    try { normal_case(); negative_slot_case(); upper_bound_case(); state_guard_case(); exception_case(); }
    catch (const std::exception &) { return 1; }
    return 0;
}
'''
    source = tmp_path / "infowin_right_menu.cpp"
    executable = tmp_path / "infowin_right_menu"
    source.write_text(harness, encoding="utf-8")
    subprocess.run(
        # The original checks the pane array's address, rather than its
        # contents. Keep that quirk in the production body while allowing the
        # modern host compiler's expected diagnostic; other warnings stay fatal.
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-error=address", str(source), "-o", str(executable)],
        check=True,
        capture_output=True,
        text=True,
    )
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
