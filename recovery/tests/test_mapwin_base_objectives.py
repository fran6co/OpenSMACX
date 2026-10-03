from pathlib import Path
import os
import shutil
import subprocess
import tempfile


PROJECT = Path(
    os.environ.get("OPENSMACX_SOURCE_ROOT", Path(__file__).resolve().parents[2])
).resolve()
SOURCE = PROJECT / "src/mapwin.cpp"
SIGNATURE = "void MapWin::draw_base_names()"


def matching_brace(text: str, opening: int) -> int:
    depth = 0
    for pos in range(opening, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return pos
    raise AssertionError("unbalanced C++ block")


def actual_detail_branch() -> str:
    source = SOURCE.read_text()
    function = source.index(SIGNATURE)
    start = source.index("if (!production_detail)", function)
    first_open = source.index("{", start)
    first_close = matching_brace(source, first_open)
    owner = source.index("else if (base.faction_id_current == faction ||", first_close)
    owner_open = source.index("{", owner)
    owner_close = matching_brace(source, owner_open)
    fallback = source.index("else {", owner_close)
    fallback_open = source.index("{", fallback)
    fallback_close = matching_brace(source, fallback_open)
    block = source[start : fallback_close + 1]
    # The project uses 32-bit pointer-as-integer string IDs. This host fixture
    # replaces only that representation with its integer ID field.
    return block.replace(
        "reinterpret_cast<int>(Facility[-production_id].name)",
        "Facility[-production_id].name_id",
    )


def test_actual_mapwin_detail_branch():
    block = actual_detail_branch()
    fixture = f"""
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
typedef char *LPSTR;
const int MaxFacilityNum = 10;
const int MaxVehProtoNum = 10;
const uint32_t MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES = 1;
static uint32_t GameMorePreferences, GameRules, field_1DD70_;
static int faction, base_id, pixel_x, pixel_y, label_calls, objective_result, spy_result;
static char production[] = "SECRET UNIT";
static char objective_text[] = "objective";
static char descriptor[] = "GOVERNOR";
struct Base {{ int faction_id_current; int queue_production_id[1]; uint32_t governor; }} base;
struct FacilityRecord {{ int name_id; }} Facility[MaxFacilityNum];
struct VehiclePrototype {{ LPSTR veh_name; }} VehPrototypes[MaxVehProtoNum];
struct StringTableMock {{ LPSTR get(int) {{ return production; }} }} StringTable;
struct FontMock {{ int line_advance() {{ return 12; }} }} font_a_;
struct CanvasMock {{
    char written[4][256]; int count;
    void write_cent_l(const char *s, int, int, int) {{
        if (count < 4) {{ strcpy(written[count], s); ++count; }}
    }}
}} canvas_storage;
static CanvasMock *canvas = &canvas_storage;
static int field_1DDBC_, field_1DDC8_;
static int is_objective(int) {{ return objective_result; }}
static int spying(int) {{ return spy_result; }}
static LPSTR label_get(int id) {{ ++label_calls; return id == 898 ? objective_text : descriptor; }}
static void CharUpperA(char *s) {{ for (; *s; ++s) *s = (char)toupper((unsigned char)*s); }}
static void run(int pref, int window_pref, int owner, int spy, int objective, uint32_t rules) {{
    GameMorePreferences = pref ? MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES : 0;
    field_1DD70_ = window_pref ? 0x08000000U : 0;
    faction = 1; base_id = 3; pixel_x = 20; pixel_y = 30;
    base.faction_id_current = owner; base.queue_production_id[0] = 1; base.governor = 0;
    VehPrototypes[1].veh_name = production;
    GameRules = rules; spy_result = spy; objective_result = objective;
    label_calls = 0; canvas_storage.count = 0;
}}
static void actual_branch() {{
    const bool production_detail =
        (GameMorePreferences & MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES) != 0 ||
        (field_1DD70_ & 0x08000000U) != 0;
    {block}
}}
int main() {{
    run(1, 0, 2, 0, 1, 0); actual_branch();
    assert(canvas_storage.count == 1 && strcmp(canvas_storage.written[0], "OBJECTIVE") == 0);
    assert(strstr(canvas_storage.written[0], "SECRET UNIT") == 0);
    run(0, 1, 2, 0, 1, 0); actual_branch();
    assert(canvas_storage.count == 1 && strcmp(canvas_storage.written[0], "OBJECTIVE") == 0);
    run(1, 0, 2, 0, 0, 0); actual_branch();
    assert(canvas_storage.count == 0 && label_calls == 0);
    run(1, 0, 2, 0, 1, 0x20000000U); actual_branch();
    assert(canvas_storage.count == 0 && label_calls == 0);
    run(1, 0, 1, 0, 0, 0); actual_branch();
    assert(canvas_storage.count == 1 && strcmp(canvas_storage.written[0], "(SECRET UNIT)") == 0);
    run(1, 0, 2, 1, 0, 0); actual_branch();
    assert(canvas_storage.count == 1 && strcmp(canvas_storage.written[0], "(SECRET UNIT)") == 0);
    run(0, 0, 2, 0, 1, 0); actual_branch();
    assert(canvas_storage.count == 1 && strcmp(canvas_storage.written[0], "OBJECTIVE") == 0);
}}
"""
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "no host C++ compiler available"
    with tempfile.TemporaryDirectory(prefix="mapwin-base-detail-v2-") as tmp:
        cpp = Path(tmp) / "control.cpp"
        cpp.write_text(fixture)
        for compiler in compilers:
            exe = Path(tmp) / compiler.replace("+", "p")
            subprocess.run(
                [compiler, "-std=c++98", str(cpp), "-o", str(exe)],
                check=True,
                timeout=60,
            )
            subprocess.run([str(exe)], check=True, timeout=30)
