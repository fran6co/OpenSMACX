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
    valid_begin = click.index("                if (valid) {", triad_begin)
    invalid_else = click.index("                } else {\n                    parse_say(", valid_begin)
    # The valid arm ends immediately before the original failure-message arm.
    triad_valid = click[triad_begin:invalid_else] + "                }\n            }"
    return base_lookup, triad_valid


BASE_HARNESS = r'''#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

static void require(bool ok, int line) {
    if (!ok) { std::fprintf(stderr, "assertion at %d\n", line); std::exit(92); }
}
#define REQUIRE(x) require((x), __LINE__)
struct Base { const char *name_string; int x, y; int16_t auto_forward_land_base_id;
    int16_t auto_forward_sea_base_id; int16_t auto_forward_air_base_id; };
static Base base_storage[2] = {{"negative-slot", 0, 0, 0, 0, 0},
                               {"source", 0, 0, 0, 0, 0}};
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
static Base source = {"source", 0, 0, 0, 0, 0};
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
struct Base { short x, y; unsigned char prefix[0x10c];
    short auto_forward_land_base_id; short auto_forward_sea_base_id;
    short auto_forward_air_base_id; short padding_0; };
static_assert(offsetof(Base, auto_forward_land_base_id) == 0x110,
              "test route member offset");
static_assert(offsetof(Base, padding_0) == 0x116, "test adjacent word offset");
static Base Bases[4] = {};
static int16_t Triad[4] = {};
static int selected_triad, target_base, editor_tool;
static Base source = {};
static int port_to_port(int, int) { return 1; }
static void test_selected_triad() {
    (void)Triad;
    TRIAD_VALID_ARM
}
int main() {
    selected_triad = 3; target_base = editor_tool = 2;
    Bases[editor_tool].auto_forward_air_base_id = 7;
    Bases[editor_tool].padding_0 = 9;
    test_selected_triad();
    REQUIRE(Bases[editor_tool].auto_forward_air_base_id == 7);
    REQUIRE(Bases[editor_tool].padding_0 == -1);
    selected_triad = -1;
    Bases[editor_tool].auto_forward_air_base_id = 7;
    Bases[editor_tool].padding_0 = 9;
    test_selected_triad();
    REQUIRE(Bases[editor_tool].auto_forward_air_base_id == 7);
    REQUIRE(Bases[editor_tool].padding_0 == 9);
}
'''


def compile_and_run(compiler, tmp_path, source_path):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is not installed")
    base_lookup, triad_valid = extract_route_blocks(source_path)
    for name, harness, replacement in (
        ("negative_base", BASE_HARNESS, "BASE_LOOKUP"),
        ("nonnegative_triad", TRIAD_HARNESS, "TRIAD_VALID_ARM"),
    ):
        source = harness.replace(replacement,
                                 base_lookup if name == "negative_base" else triad_valid)
        cpp = tmp_path / f"{name}.cpp"
        binary = tmp_path / name
        cpp.write_text(source)
        subprocess.run([compiler_path, "-std=c++11", "-Wall", "-Wextra",
                        "-Werror", str(cpp), "-o", str(binary)],
                       check=True, timeout=30)
        subprocess.run([str(binary)], check=True, timeout=30)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_click_preserves_unchecked_original_routes(compiler, tmp_path):
    # Permanent control is intentionally bound to the real checkout source;
    # it has no candidate/overlay fallback.
    compile_and_run(compiler, tmp_path, ROOT / "src/mapwin.cpp")
