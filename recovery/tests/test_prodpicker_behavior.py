"""Host controls for the recovered ProdPicker keyboard navigation body.

The test compiles the production member definition extracted from src/prodpicker.cpp
against small behavior doubles. It is a regression check for the translated
selection/scroll/redraw sequence, not ABI or game-runtime evidence.
"""

from pathlib import Path
import shutil
import subprocess
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
UNIT = ROOT / "src/prodpicker.cpp"


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


def compile_and_run(tmp_path, body: str, *, damaged: bool = False,
                    damaged_right_edge: bool = False,
                    damaged_signed_division: bool = False):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    if damaged:
        token = "            field_B64_ = 0;\n"
        assert token in body
        body = body.replace(token, "", 1)
    if damaged_right_edge:
        corrected = """        if (next < first || last < first) {
            field_A774_ = first;
        } else {
            field_A774_ = next < last ? next : last;
        }
"""
        original_wrap = """        field_A774_ = next;
        if (first <= next && next <= last) {
            if (next > last) field_A774_ = last;
        } else {
            field_A774_ = first;
        }
"""
        assert corrected in body
        body = body.replace(corrected, original_wrap, 1)
    if damaged_signed_division:
        token = "oldSelection / static_cast<int>(field_A810_)"
        assert body.count(token) == 2
        body = body.replace(token, "oldSelection / field_A810_")
    fixture = r'''
#include <cassert>
#include <cstdint>
#include <vector>
struct Scroll {
    int pos = 0;
    int page = 2;
    int position() const { return pos; }
    int paging() const { return page; }
    void set_pos(int value) { pos = value; }
};
struct ProdPicker {
    int field_A77C_ = 0, field_A774_ = 4;
    uint32_t field_A810_ = 4;
    int field_A818_ = 11, field_A82C_ = 8, field_B64_ = 7;
    Scroll scroll_;
    std::vector<int> events;
    void draw_item(int item, int update) { events.push_back(1000 + item * 10 + update); }
    void redraw() { events.push_back(2); }
    void soft_update() { events.push_back(3); }
    int on_key_click(int repeat, int key);
};
BODY
int main() {
    ProdPicker p;
    assert(p.on_key_click(0, 0x27) == 1); // right selects adjacent item
    assert(p.field_A774_ == 5 && p.field_A77C_ == 0 && p.field_B64_ == 0);
    assert((p.events == std::vector<int>{1040, 1050, 3}));

    p.events.clear();
    p.field_B64_ = 9;
    assert(p.on_key_click(0, 0x28) == 1); // down crosses the visible row boundary
    assert(p.field_A774_ == 9 && p.field_A77C_ == 4 && p.scroll_.pos == 1);
    assert(p.field_B64_ == 0 && (p.events == std::vector<int>{2, 3}));

    p.events.clear();
    p.field_B64_ = 6;
    p.scroll_.pos = 1;
    assert(p.on_key_click(0, 0x22) == 1); // page-down uses Scroll's live paging
    assert(p.scroll_.pos == 3 && p.field_A77C_ == 12 && p.field_A774_ == 9);
    assert(p.field_B64_ == 0 && (p.events == std::vector<int>{2, 3}));

    p.events.clear();
    p.field_A818_ = 8;
    p.field_A774_ = 7; // last cell of a complete row
    p.field_A77C_ = 0;
    p.field_A82C_ = 8;
    p.field_B64_ = 6;
    p.scroll_.pos = 0;
    assert(p.on_key_click(0, 0x27) == 1);
    assert(p.field_A774_ == 7 && p.field_A77C_ == 0 && p.field_B64_ == 6);
    assert((p.events == std::vector<int>{3}));

    p.events.clear();
    p.field_A818_ = 10;
    p.field_A774_ = 9; // last cell of a short final row
    p.field_A77C_ = 8;
    p.field_A82C_ = 8;
    p.field_B64_ = 5;
    p.scroll_.pos = 2;
    assert(p.on_key_click(0, 0x27) == 1);
    assert(p.field_A774_ == 9 && p.field_A77C_ == 8 && p.field_B64_ == 5);
    assert((p.events == std::vector<int>{3}));

    p.events.clear();
    p.field_A810_ = 5;
    p.field_A818_ = 10;
    p.field_A774_ = -1; // original no-selection sentinel; IDIV rounds toward zero
    p.field_A77C_ = 0;
    assert(p.on_key_click(0, 0x25) == 1);
    assert(p.field_A774_ == 0);
    assert((p.events == std::vector<int>{990, 1000, 3}));

    p.events.clear();
    p.field_A774_ = -1;
    assert(p.on_key_click(0, 0x27) == 1);
    assert(p.field_A774_ == 0);
    assert((p.events == std::vector<int>{990, 1000, 3}));
}
'''.replace("BODY", body)
    source = tmp_path / ("prodpicker-key.cpp" if not damaged else "prodpicker-key-damaged.cpp")
    executable = source.with_suffix("")
    source.write_text(fixture)
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_keyboard_navigation_preserves_selection_scroll_and_redraw_order(tmp_path):
    body = extract_function(UNIT.read_text(), "int ProdPicker::on_key_click(int repeat, int key)")
    result = compile_and_run(tmp_path, body)
    assert result.returncode == 0, result.stderr


def test_navigation_damage_control_detects_omitted_rotation_reset(tmp_path):
    body = extract_function(UNIT.read_text(), "int ProdPicker::on_key_click(int repeat, int key)")
    result = compile_and_run(tmp_path, body, damaged=True)
    assert result.returncode != 0


def test_right_arrow_does_not_wrap_at_full_or_short_row_end(tmp_path):
    body = extract_function(UNIT.read_text(), "int ProdPicker::on_key_click(int repeat, int key)")
    result = compile_and_run(tmp_path, body)
    assert result.returncode == 0, result.stderr


def test_right_arrow_wrap_damage_control_fails_at_row_edges(tmp_path):
    body = extract_function(UNIT.read_text(), "int ProdPicker::on_key_click(int repeat, int key)")
    result = compile_and_run(tmp_path, body, damaged_right_edge=True)
    assert result.returncode != 0


def test_unsigned_division_damage_control_detects_negative_selection(tmp_path):
    body = extract_function(UNIT.read_text(), "int ProdPicker::on_key_click(int repeat, int key)")
    result = compile_and_run(tmp_path, body, damaged_signed_division=True)
    assert result.returncode != 0


RIGHT_CLICK_FIXTURE = r'''
#include <cassert>
#include <cstdint>
#include <vector>
std::vector<int> events;
struct Base { int faction_id_current = 2; } baseGlobal;
Base *BaseCurrent = &baseGlobal;
struct Prototype { int weapon_id = 0; int flags = 0; uint8_t obsolete_factions = 0; };
Prototype VehPrototypes[4];
struct WeaponRecord { int offense_rating = 2; };
WeaponRecord Weapon[4];
const int MaxVehProtoNum = 4;
const int PROTO_ACTIVE = 1;
int IsMultiplayerNet = 0;
int selectedMenuRow = 2;
char *label_get(int value) { static char label[2]; label[0] = char(value); label[1] = 0; return label; }
void synch_proto(int id) { events.push_back(100 + id); }
struct Entry { uint32_t field_8_ = 0x20; } entry;
using StringStructPayload = Entry;
struct StringList {
    int entry_count_ = 1;
    int current = 0;
    void seek_pos(int value) { current = value; }
    Entry *current_entry() { return &entry; }
    int current_id() { return 0; }
};
struct PullDown {
    std::vector<int> ids;
    void init() { }
    void add_item(int id, const char *) { ids.push_back(id); }
    void show(int, int, int) { }
    int get_selected() { return selectedMenuRow; }
    int item_id(int index) { return ids[index]; }
    uint32_t field_A0C_value() const { return 0x005D69F0U; }
};
struct ProdPicker {
    int field_A774_ = 0, field_B68_ = 0;
    StringList prod_a_;
    void on_left_click(int, int) { }
    void client_to_screen(int *, int *) { }
    void on_button_clicked(int) { events.push_back(4); }
    void calculate() { events.push_back(5); }
    void redraw() { events.push_back(6); }
    void on_right_click(int x, int y);
};
BODY
int main() {
    ProdPicker picker;
    picker.on_right_click(7, 9);
    assert(VehPrototypes[0].obsolete_factions == (1U << baseGlobal.faction_id_current));
    assert((events == std::vector<int>{100, 5, 6}));

    events.clear();
    selectedMenuRow = 3; // clear-obsolete action, menu id 3
    VehPrototypes[0].obsolete_factions = 0xFF;
    VehPrototypes[1].flags = PROTO_ACTIVE;
    VehPrototypes[1].obsolete_factions = 0xFF;
    picker.on_right_click(7, 9);
    assert(VehPrototypes[0].obsolete_factions == 0xFF); // inactive designs are untouched
    assert(VehPrototypes[1].obsolete_factions == 0xFB);
    assert((events == std::vector<int>{5, 6}));
}
'''


def test_right_click_context_actions_preserve_prototype_filters(tmp_path):
    body = extract_function(UNIT.read_text(), "void ProdPicker::on_right_click(int x, int y)")
    fixture = RIGHT_CLICK_FIXTURE.replace("BODY", body)
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    source = tmp_path / "prodpicker-right-click.cpp"
    executable = tmp_path / "prodpicker-right-click"
    source.write_text(fixture)
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr


CALCULATE_FIXTURE = r'''
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
struct RuleTable { int retool_exemption = 0, retool_pct_pen_prod_chg = 0; } Rules;
struct Base {
    int queue_production_id[2]{7, 0};
    int production_id_last = -3;
    int faction_id_current = 2;
    int mineral_surplus = 5;
    int minerals_accumulated = 0;
    int nutrients_accumulated = 0;
} base;
Base *BaseCurrent = &base;
int BaseIDCurrentSelected = 2;
struct GameStateFields { int game_state_ = 0x80; };
struct GameDataStorage { GameStateFields fields_; } GameDataStateStorage;
const int MaxWeaponNum = 0, MaxArmorNum = 0, MaxChassisNum = 0;
const int MaxVehProtoNum = 128;
const int PROTO_ACTIVE = 1;
struct VehPrototype { const char *veh_name = ""; int cost = 0, plan = 0, flags = 0, preq_tech = 0, weapon_id = 0; };
VehPrototype VehPrototypes[128];
struct VehicleStats { int offense_rating = 0, defense_rating = 0, speed = 0, preq_tech = 0; };
VehicleStats Weapon[1], Armor[1], Chassis[1];
struct PlayerData {
    int diff_level = 2;
    int proto_id_active[512]{};
    int proto_id_queue[512]{};
} PlayersData[8];
struct FacilityEntry { const char *name = ""; const char *effect = ""; int cost = 0; };
FacilityEntry Facility[0x86];
struct LabelsTable { const char **strings_ptr; };
const char *LabelStrings[256]{};
LabelsTable Labels{LabelStrings};
const char *label_get(int index) { return LabelStrings[index]; }
struct StringTableMock {
    bool saw_entry_address = false;
    const char *get(intptr_t address) {
        if (address == reinterpret_cast<intptr_t>(Labels.strings_ptr + 0xC4)) {
            saw_entry_address = true;
            return "BAD-ENTRY-ADDRESS";
        }
        return reinterpret_cast<const char *>(address);
    }
} StringTable;
int ProtoSortIds[128];
void proto_sort(int) { }
int has_tech(int, int) { return 0; }
int veh_avail(int id, int, int) { return id == 7; }
int facility_available = 0;
int facility_avail(int id, int, int, int) { return facility_available && id == 3; }
int cost_factor(int, int, int) { return 1; }
int veh_cost(int, int, int *flags) { *flags = 0; return 5; }
int base_making(int id, int) { return id == 7 ? 2 : 1; }
char *_itoa(int value, char *buffer, int radix) {
    assert(radix == 10);
    std::snprintf(buffer, 80, "%d", value);
    return buffer;
}
void say_stats_3(char *buffer, int) { buffer[0] = '\0'; }
struct StringList {
    char *pending_text_ = 0;
    uint32_t field_20_ = 0, field_24_ = 0;
    int current_position_ = 0, allocator_ = 0;
    std::vector<std::string> text;
    std::vector<int> ids;
    std::vector<uint32_t> flags;
    void remove_all() { text.clear(); ids.clear(); flags.clear(); }
    void add(int id) { text.push_back(pending_text_); ids.push_back(id); flags.push_back(field_20_); }
};
struct ProdPicker {
    uint32_t field_B68_ = 0;
    int field_A76C_ = 2, field_A77C_ = 0;
    int field_A818_ = 0, field_A774_ = -1;
    StringList prod_a_, prod_b_, prod_c_;
    int edbc_fac(int facility_id);
    int edbc_unit(int prototype_id);
    void calculate();
};
UNIT_CATEGORY_BODIES
CALCULATE_BODY
int main() {
    LabelStrings[0xC4 / 4] = "VEHICLE-TURNS";
    LabelStrings[0x10C / 4] = "NO-MINERALS";
    LabelStrings[0x90 / 4] = "ENERGY";
    LabelStrings[0x264 / 4] = "PER-TURN";
    LabelStrings[0x230 / 4] = "OBSOLETE";
    for (int i = 0; i < 128; ++i) ProtoSortIds[i] = -1;
    ProtoSortIds[0] = 7;
    VehPrototypes[7].veh_name = "UNIT";
    VehPrototypes[7].cost = 11;
    VehPrototypes[7].plan = 7;
    PlayersData[2].proto_id_active[7] = 4;
    PlayersData[2].proto_id_queue[7] = 2;
    Facility[3].name = "FACILITY";
    Facility[3].effect = "EFFECT";
    Facility[3].cost = 8;

    ProdPicker picker;
    picker.calculate();
    assert(picker.field_A818_ == 1 && picker.field_A774_ == 0);
    assert(picker.prod_a_.ids[0] == 7);
    assert(picker.prod_a_.text[0] == "UNIT(4 2)");
    assert(picker.prod_b_.text[0] == "1(3) VEHICLE-TURNS");
    assert(picker.prod_c_.text[0].empty());
    assert(picker.prod_a_.flags[0] == 0x20);
    assert(!StringTable.saw_entry_address);

    facility_available = 1;
    picker.calculate();
    assert(picker.field_A818_ == 2 && picker.field_A774_ == 1);
    assert((picker.prod_a_.ids == std::vector<int>{7, 3}));
    assert(picker.prod_a_.text[1] == "FACILITY");
    assert(picker.prod_b_.text[1] == "2 VEHICLE-TURNS");
    assert(picker.prod_c_.text[1] == "EFFECT");
    assert(picker.prod_a_.flags[1] == 0x10);
    assert(!StringTable.saw_entry_address);
}
'''


def calculate_source(*, broken_label=False):
    source = UNIT.read_text()
    body = extract_function(source, "void ProdPicker::calculate()")
    # Keep the complete production list-building prefix but stop before its
    # unrelated window-geometry/font setup; this control targets its real
    # string, category, cost and StringList behavior.
    render_setup = "\n    if (field_A818_ != 0) {"
    assert render_setup in body
    body = body[:body.index(render_setup)] + "\n}"
    body = body.replace("reinterpret_cast<int>(", "reinterpret_cast<intptr_t>(")
    if broken_label:
        correct = "label_get(0xC4 / 4)"
        assert correct in body
        body = body.replace(correct,
                            "StringTable.get(reinterpret_cast<intptr_t>(Labels.strings_ptr + 0xC4))",
                            1)
    categories = "\n".join(
        extract_function(source, signature).replace("reinterpret_cast<int>(", "reinterpret_cast<intptr_t>(")
        for signature in (
            "int ProdPicker::edbc_fac(int facility_id)",
            "int ProdPicker::edbc_unit(int prototype_id)",
        )
    )
    return (CALCULATE_FIXTURE.replace("UNIT_CATEGORY_BODIES", categories)
            .replace("CALCULATE_BODY", body))


def test_calculate_builds_vehicle_and_facility_lists_with_original_cost_labels(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    source = tmp_path / "prodpicker-calculate.cpp"
    executable = tmp_path / "prodpicker-calculate"
    source.write_text(calculate_source())
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr


def test_calculate_damage_control_catches_label_table_pointer_as_entry_id(tmp_path):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    source = tmp_path / "prodpicker-calculate-damaged.cpp"
    executable = tmp_path / "prodpicker-calculate-damaged"
    source.write_text(calculate_source(broken_label=True))
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode != 0


def test_exec_camera_angle_matches_pinned_float_word(tmp_path):
    execute = extract_function(UNIT.read_text(), "int ProdPicker::exec(")
    match = re.search(r"field_A744_\s*=\s*([0-9.]+f)", execute)
    assert match, "production exec must initialize its camera angle"
    source = tmp_path / "prodpicker-angle.cpp"
    executable = tmp_path / "prodpicker-angle"
    source.write_text(
        "#include <cassert>\n#include <cstdint>\n#include <cstring>\n"
        "int main() { float angle = " + match.group(1) + "; uint32_t bits = 0; "
        "std::memcpy(&bits, &angle, sizeof(bits)); assert(bits == 0x3F48F5C3U); }\n"
    )
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr


def test_exec_camera_angle_damage_control_rejects_pi_over_four(tmp_path):
    execute = extract_function(UNIT.read_text(), "int ProdPicker::exec(")
    match = re.search(r"field_A744_\s*=\s*([0-9.]+f)", execute)
    assert match, "production exec must initialize its camera angle"
    wrong = match.group(1).replace("0.785f", "0.78539818525314331055f")
    source = tmp_path / "prodpicker-angle-damaged.cpp"
    executable = tmp_path / "prodpicker-angle-damaged"
    source.write_text(
        "#include <cassert>\n#include <cstdint>\n#include <cstring>\n"
        "int main() { float angle = " + wrong + "; uint32_t bits = 0; "
        "std::memcpy(&bits, &angle, sizeof(bits)); assert(bits == 0x3F48F5C3U); }\n"
    )
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode != 0


def test_unk2_is_a_cdecl_callback_with_null_guard(tmp_path):
    header = (ROOT / "src/prodpicker.h").read_text()
    assert "static void __cdecl UNK2(int unused, ProdPicker *picker);" in header
    body = extract_function(UNIT.read_text(), "void __cdecl ProdPicker::UNK2(")
    source = tmp_path / "prodpicker-unk2.cpp"
    executable = tmp_path / "prodpicker-unk2"
    source.write_text(r'''
#include <cassert>
#define __cdecl
struct ProdPicker {
    int field_B64_ = 0;
    int field_A774_ = 9;
    int item = -1, update = -1;
    void draw_item(int value, int repaint) { item = value; update = repaint; }
    static void __cdecl UNK2(int, ProdPicker *picker);
};
BODY
int main() {
    void (__cdecl *callback)(int, ProdPicker *) = &ProdPicker::UNK2;
    callback(12, nullptr);
    ProdPicker picker;
    callback(12, &picker);
    assert(picker.field_B64_ == 1 && picker.item == 9 && picker.update == 1);
}
'''.replace("BODY", body))
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    result = subprocess.run([str(executable)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr


def modal_tail_fixture(*, damaged_parent=False):
    body = extract_function(UNIT.read_text(), "int ProdPicker::exec(")
    start = body.index("    if (field_B68_ == 0) {\n        set_modal(0, pop_wait")
    end = body.index("\n\n    if (modal == 0) hide();", start)
    tail = body[start:end]
    if damaged_parent:
        assert "set_modal(0, pop_wait, 0);" in tail
        assert "set_modal(0, pop_wait, &MainInterfaceGlobal);" in tail
        tail = tail.replace("set_modal(0, pop_wait, 0);",
                            "set_modal(0, pop_wait, &MainInterfaceGlobal);", 1)
        # Swap only the nonzero call after the first replacement.
        marker = "for (int buttonToSuppress = 0; buttonToSuppress < 22; ++buttonToSuppress)"
        position = tail.index(marker)
        prefix, suffix = tail[:position], tail[position:]
        suffix = suffix.replace("set_modal(0, pop_wait, &MainInterfaceGlobal);",
                                "set_modal(0, pop_wait, 0);", 1)
        tail = prefix + suffix
    return r'''
#include <cassert>
#include <cstdint>
#include <vector>
std::vector<int> events;
void *expected_parent = 0;
struct MainInterface { } MainInterfaceGlobal;
struct Button {
    int id = 0;
    uint32_t flags = 0;
    void set_input_suppressed(int value) {
        flags = value ? flags | 8U : flags & ~8U;
        events.push_back((value ? 100 : 200) + id);
    }
};
struct TutWindow { Button flatButtons_[22]; } TutWinBaseWindow;
int pop_wait() { events.push_back(999); return 7; }
struct ProdPicker {
    int field_B68_ = 0;
    void set_modal(int, int (*callback)(), void *parent) {
        assert(parent == expected_parent);
        if (field_B68_ != 0)
            for (int i = 0; i < 22; ++i)
                assert((TutWinBaseWindow.flatButtons_[i].flags & 8U) != 0);
        else
            for (int i = 0; i < 22; ++i)
                assert((TutWinBaseWindow.flatButtons_[i].flags & 8U) == 0);
        events.push_back(500);
        assert(callback() == 7);
        events.push_back(600);
    }
    void run_tail() {
TAIL
    }
};
int main() {
    std::vector<uint32_t> initial(22);
    for (int i = 0; i < 22; ++i) {
        TutWinBaseWindow.flatButtons_[i].id = i;
        TutWinBaseWindow.flatButtons_[i].flags = initial[i] = 0xA5A50002U | (i & 3);
    }
    ProdPicker picker;
    expected_parent = 0;
    picker.field_B68_ = 0;
    picker.run_tail();
    assert((events == std::vector<int>{500, 999, 600}));
    for (int i = 0; i < 22; ++i) assert(TutWinBaseWindow.flatButtons_[i].flags == initial[i]);

    events.clear();
    picker.field_B68_ = 1;
    expected_parent = &MainInterfaceGlobal;
    picker.run_tail();
    std::vector<int> expected;
    for (int i = 0; i < 22; ++i) expected.push_back(100 + i);
    expected.push_back(500); expected.push_back(999); expected.push_back(600);
    for (int i = 0; i < 22; ++i) expected.push_back(200 + i);
    assert(events == expected);
    for (int i = 0; i < 22; ++i) assert(TutWinBaseWindow.flatButtons_[i].flags == initial[i]);
}
'''.replace("TAIL", tail)


def run_modal_tail(tmp_path, *, damaged_parent=False):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ compiler unavailable")
    source = tmp_path / ("prodpicker-modal-damaged.cpp" if damaged_parent
                         else "prodpicker-modal.cpp")
    executable = source.with_suffix("")
    source.write_text(modal_tail_fixture(damaged_parent=damaged_parent))
    subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(executable)], check=True,
                   capture_output=True, text=True)
    return subprocess.run([str(executable)], capture_output=True, text=True)


def test_exec_modal_tail_uses_correct_parent_and_suppression_order(tmp_path):
    result = run_modal_tail(tmp_path)
    assert result.returncode == 0, result.stderr


def test_exec_modal_parent_damage_control_rejects_reversed_parent(tmp_path):
    result = run_modal_tail(tmp_path, damaged_parent=True)
    assert result.returncode != 0
