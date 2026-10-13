from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())


def extract_route_blocks(source_path):
    source = source_path.read_text()
    begin = source.index("void MapWin::click(")
    end = source.index("\n}", begin) + 2
    click = source[begin:end]

    base_begin = click.index("            const int target_base = base_find(")
    base_end_marker = "            parse_says(1, Bases[target_base].name_string, -1, -1);"
    base_end = click.index(base_end_marker, base_begin) + len(base_end_marker)
    base_lookup = click[base_begin:base_end]

    triad_begin = click.index("            if (selected_triad >= 0")
    opening = click.index("{", triad_begin)
    depth = 0
    for end in range(opening, len(click)):
        depth += (click[end] == "{") - (click[end] == "}")
        if not depth:
            triad_block = click[triad_begin:end + 1]
            break
    else:
        raise AssertionError("unterminated selected-triad block")
    return base_lookup, triad_block


BASE_HARNESS = r'''#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

static void require(bool ok, int line) {
    if (!ok) { std::fprintf(stderr, "assertion at %d\n", line); std::exit(92); }
}
#define REQUIRE(x) require((x), __LINE__)
struct Base {
    enum AutoForwardSlot { AutoForwardLand, AutoForwardSea,
                           AutoForwardAir, AutoForwardReserved,
                           AutoForwardSlotCount };
    const char *name_string; int x, y; int16_t auto_forward_slots[4];
};
static Base base_storage[2] = {{"negative-slot", 0, 0, {0, 0, 0, 0}},
                               {"source", 0, 0, {0, 0, 0, 0}}};
static Base *Bases = &base_storage[1];
static int base_result;
static std::vector<int> events;
static int BaseIDCurrentSelected = 8;
struct NetMock {
    void unlock_base(int id) { REQUIRE(id == 8); events.push_back(1); }
} NetDaemonNet;
static int base_find(int, int, int, int, int, int) {
    events.push_back(2); return base_result;
}
static void parse_says(int owner, const char *text, int, int) {
    REQUIRE(owner == 0 || owner == 1);
    REQUIRE(text != nullptr);
    events.push_back(10 + owner);
}
static int tile_x, tile_y, local_faction;
static Base source = {"source", 0, 0, {0, 0, 0, 0}};
static void test_base_lookup() {
    (void)BaseIDCurrentSelected; (void)NetDaemonNet;
    BASE_LOOKUP
}
int main() {
    // A negative native result indexes the addressable sentinel immediately;
    // the fixture preserves that original unchecked access without host UB.
    base_result = -1; events.clear(); test_base_lookup();
    REQUIRE((events == std::vector<int>{2, 10, 11}));
}
'''


TRIAD_HARNESS = r'''#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <vector>

static void require(bool ok, int line) {
    if (!ok) { std::fprintf(stderr, "assertion at %d\n", line); std::exit(93); }
}
#define REQUIRE(x) require((x), __LINE__)
static const int MapLongitude = 4;
static unsigned char tile_data[256];
static unsigned char *map_tile_bytes() { return tile_data; }
struct Base {
    enum AutoForwardSlot { AutoForwardLand, AutoForwardSea,
                           AutoForwardAir, AutoForwardReserved,
                           AutoForwardSlotCount };
    short x, y; unsigned char prefix[0x10c];
    short auto_forward_slots[AutoForwardSlotCount]; short talent_total;
};
static_assert(offsetof(Base, auto_forward_slots) == 0x110,
              "test route member offset");
static_assert(offsetof(Base, talent_total) == 0x118, "test next member offset");
static Base Bases[4] = {};
static int16_t Triad[4] = {};
static int selection, target_base, editor_tool, port_result = 1;
static Base source = {};
static int port_to_port(int, int) { return port_result; }
static int parse_calls, pop_calls, last_parse_owner, last_parse_triad;
static void parse_say(int owner, int triad, int, int) {
    ++parse_calls; last_parse_owner = owner; last_parse_triad = triad;
}
struct NetMsgMock {
    void pop(const char *key, int code, int, void *) {
        REQUIRE(key != nullptr && code == 0x1388); ++pop_calls;
    }
} NetMsgPrimary;
static void test_selected_triad() {
    const int selected_triad = selection;
    TRIAD_VALID_ARM
}
int main() {
    target_base = 1; editor_tool = 2;
    source.x = 2; source.y = 1; Bases[target_base].x = 4; Bases[target_base].y = 1;
    tile_data[((source.x >> 1) + source.y * MapLongitude) * 4 + 3] = 5;
    tile_data[((Bases[target_base].x >> 1) + Bases[target_base].y * MapLongitude) * 4 + 3] = 5;

    // Each ordinary route index writes only its corresponding destination.
    selection = 0; test_selected_triad();
    REQUIRE(Bases[editor_tool].auto_forward_slots[0] == target_base);
    REQUIRE(Bases[editor_tool].auto_forward_slots[1] == 0);
    REQUIRE(Bases[editor_tool].auto_forward_slots[2] == 0);
    selection = 1; test_selected_triad();
    REQUIRE(Bases[editor_tool].auto_forward_slots[1] == target_base);
    REQUIRE(Bases[editor_tool].auto_forward_slots[2] == 0);
    selection = 2; test_selected_triad();
    REQUIRE(Bases[editor_tool].auto_forward_slots[2] == target_base);

    // The original land/sea eligibility failures keep every route untouched
    // and preserve both the parse_say and network-pop failure side effects.
    selection = 0; Bases[editor_tool].auto_forward_slots[0] = 11;
    tile_data[((Bases[target_base].x >> 1) + Bases[target_base].y * MapLongitude) * 4 + 3] = 6;
    test_selected_triad();
    REQUIRE(Bases[editor_tool].auto_forward_slots[0] == 11);
    REQUIRE(parse_calls == 1 && last_parse_owner == 2 && last_parse_triad == Triad[0]);
    REQUIRE(pop_calls == 1);
    selection = 1; port_result = 0; Bases[editor_tool].auto_forward_slots[1] = 12;
    test_selected_triad();
    REQUIRE(Bases[editor_tool].auto_forward_slots[1] == 12);
    REQUIRE(parse_calls == 2 && last_parse_triad == Triad[1]);
    REQUIRE(pop_calls == 2);

    // A nonnegative out-of-domain result still writes the physically adjacent
    // reserved word; a negative result does not enter the route block.
    selection = 3; target_base = editor_tool = 2;
    Bases[editor_tool].auto_forward_slots[2] = 7;
    Bases[editor_tool].auto_forward_slots[3] = 9;
    test_selected_triad();
    REQUIRE(Bases[editor_tool].auto_forward_slots[2] == 7);
    REQUIRE(Bases[editor_tool].auto_forward_slots[3] == -1);
    selection = -1;
    Bases[editor_tool].auto_forward_slots[2] = 7;
    Bases[editor_tool].auto_forward_slots[3] = 9;
    test_selected_triad();
    REQUIRE(Bases[editor_tool].auto_forward_slots[2] == 7);
    REQUIRE(Bases[editor_tool].auto_forward_slots[3] == 9);
}
'''


def compile_triad(compiler, tmp_path, source_path, block, name):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is not installed")
    source = TRIAD_HARNESS.replace("TRIAD_VALID_ARM", block)
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(source)
    subprocess.run([compiler_path, "-std=c++11", "-Wall", "-Wextra",
                    "-Werror", str(cpp), "-o", str(binary)],
                   check=True, timeout=30)
    return subprocess.run([str(binary)], capture_output=True, timeout=30)


def compile_and_run(compiler, tmp_path, source_path):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is not installed")
    base_lookup, triad_block = extract_route_blocks(source_path)
    for name, harness, replacement in (
        ("negative_base", BASE_HARNESS, "BASE_LOOKUP"),
    ):
        source = harness.replace(replacement, base_lookup)
        cpp = tmp_path / f"{name}.cpp"
        binary = tmp_path / name
        cpp.write_text(source)
        subprocess.run([compiler_path, "-std=c++11", "-Wall", "-Wextra",
                        "-Werror", str(cpp), "-o", str(binary)],
                       check=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)
    result = compile_triad(compiler, tmp_path, source_path, triad_block,
                           "nonnegative_triad")
    assert result.returncode == 0, result.stderr.decode(errors="replace")


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_click_preserves_unchecked_original_routes(compiler, tmp_path):
    # Permanent control is intentionally bound to the real checkout source;
    # it has no candidate/overlay fallback.
    compile_and_run(compiler, tmp_path, ROOT / "src/mapwin.cpp")


@pytest.mark.parametrize("name,mutate", [
    ("shifted_route_slot", lambda block: block.replace(
        "auto_forward_slots[selected_triad]",
        "auto_forward_slots[selected_triad + 1]")),
    ("positive_only_guard", lambda block: block.replace(
        "if (selected_triad >= 0)", "if (selected_triad > 0)")),
])
def test_actual_route_slot_and_guard_damage_is_detected(tmp_path, name, mutate):
    _, block = extract_route_blocks(ROOT / "src/mapwin.cpp")
    changed = mutate(block)
    assert changed != block, name
    result = compile_triad("g++", tmp_path, ROOT / "src/mapwin.cpp", changed, name)
    assert result.returncode != 0, (name, "mutant unexpectedly passed")
