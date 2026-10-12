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
    ).replace("Facility[-production_id].name)", "Facility[-production_id].name_id)") \
     .replace("GameDataStateStorage.fields_.game_rules_", "GameRules") \
     .replace("GameDataStateStorage.fields_.game_more_preferences_", "GameMorePreferences")


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
static bool change_queue;
static char production[] = "SECRET UNIT";
static char changed_production[] = "CHANGED UNIT";
static char objective_text[] = "objective";
static char descriptor[] = "GOVERNOR";
struct Base {{ int faction_id_current; int queue_production_id[1]; uint32_t governor; }} base;
// Extra host rows observe unchecked selection without executing out-of-bounds
// C++. They do not assert original adjacent allocation contents.
struct FacilityRecord {{ int name_id; }} Facility[20];
struct VehiclePrototype {{ LPSTR veh_name; }} VehPrototypes[20];
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
static int is_objective(int) {{
    // Synthetic phase-order hook, not a claim about the current pure owner.
    if (change_queue) base.queue_production_id[0] = 2;
    return objective_result;
}}
static int spying(int) {{ return spy_result; }}
static LPSTR label_get(int id) {{ ++label_calls; return id == 898 ? objective_text : descriptor; }}
static void CharUpperA(char *s) {{ for (; *s; ++s) *s = (char)toupper((unsigned char)*s); }}
static void run(int pref, int window_pref, int owner, int spy, int objective, uint32_t rules) {{
    GameMorePreferences = pref ? MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES : 0;
    field_1DD70_ = window_pref ? 0x08000000U : 0;
    faction = 1; base_id = 3; pixel_x = 20; pixel_y = 30;
    base.faction_id_current = owner; base.queue_production_id[0] = 1; base.governor = 0;
    VehPrototypes[1].veh_name = production;
    VehPrototypes[2].veh_name = changed_production;
    change_queue = false;
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
    run(1, 0, 1, 0, 1, 0); actual_branch();
    assert(canvas_storage.count == 1 && strcmp(canvas_storage.written[0], "(OBJECTIVE: SECRET UNIT)") == 0);
    run(1, 0, 1, 0, 0, 0); base.governor = 0x80000001U; actual_branch();
    assert(strcmp(canvas_storage.written[0], "(GOVERNOR: SECRET UNIT)") == 0);
    run(1, 0, 1, 0, 1, 0); change_queue = true; actual_branch();
    assert(strcmp(canvas_storage.written[0], "(OBJECTIVE: CHANGED UNIT)") == 0);
    run(1, 0, 1, 0, 0, 0); base.queue_production_id[0] = 12;
    VehPrototypes[12].veh_name = production; actual_branch();
    assert(canvas_storage.count == 1 && strcmp(canvas_storage.written[0], "(SECRET UNIT)") == 0);
    run(1, 0, 1, 0, 0, 0); base.queue_production_id[0] = -12;
    Facility[12].name_id = 12; actual_branch();
    assert(canvas_storage.count == 1 && strcmp(canvas_storage.written[0], "(SECRET UNIT)") == 0);
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


def actual_display_gate() -> str:
    source = SOURCE.read_text()
    function = source.index(SIGNATURE)
    top_start = source.index("    if (MapBaseDisplayOverride == 0", function)
    top_end = source.index(") {", top_start)
    top_condition = source[top_start + len("    if (") : top_end].replace(
        "GameDataStateStorage.fields_.game_rules_", "GameRules"
    )
    start = source.index("    const bool show_base_names =", function)
    stop = source.index("    if (show_base_names) {", start)
    inner_gate = source[start:stop].replace(
        "GameDataStateStorage.fields_.game_more_preferences_",
        "GameMorePreferences",
    ).replace("GameDataStateStorage.fields_.game_rules_", "GameRules")
    return f"    if ({top_condition}) return;\n" + inner_gate


def gate_fixture(gate: str) -> str:
    return f"""
#include <stdint.h>
#include <assert.h>
typedef uint32_t DWORD;
const DWORD MPREF_MAP_SHOW_BASE_NAMES = 0x2000U;
static DWORD GameMorePreferences, GameRules, field_1DD70_;
static int MapBaseDisplayOverride, ControlTurnPhase;
static int field_1DD98_;
struct RECT {{ int left, top, right, bottom; }} saved_clip;
struct Canvas {{ int clip_restores; void set_clip(const RECT *) {{ ++clip_restores; }} }} canvas_storage;
static Canvas *canvas = &canvas_storage;
static int landmark_draws;
static void actual_gate_and_continue() {{
{gate}
    // Reaching this continuation models the later visible-landmark path. It
    // does not claim any separate native landmark allocation or draw API.
    ++landmark_draws;
}}
static void reset(unsigned map_gate, unsigned preference_gate, int zoom) {{
    // Keep the original top-level override path open while independently
    // checking each inner display gate and the later visible-landmark path.
    MapBaseDisplayOverride = 1;
    ControlTurnPhase = 0;
    GameRules = 0;
    field_1DD70_ = map_gate ? 0x800U : 0;
    GameMorePreferences = preference_gate ? MPREF_MAP_SHOW_BASE_NAMES : 0;
    field_1DD98_ = zoom;
    canvas_storage.clip_restores = 0;
    landmark_draws = 0;
}}
int main() {{
    reset(0, 1, 0); actual_gate_and_continue();
    assert(canvas_storage.clip_restores == 1 && landmark_draws == 0);
    reset(1, 0, 0); actual_gate_and_continue();
    assert(canvas_storage.clip_restores == 1 && landmark_draws == 0);
    reset(1, 1, -11); actual_gate_and_continue();
    assert(canvas_storage.clip_restores == 1 && landmark_draws == 0);
    reset(1, 1, -10); actual_gate_and_continue();
    assert(canvas_storage.clip_restores == 0 && landmark_draws == 1);
}}
"""


def test_actual_display_gates_return_before_landmark_continuation(tmp_path):
    gate = actual_display_gate()
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "at least GCC or Clang is required for host controls"

    def run(candidate, compiler, label):
        cpp = tmp_path / f"{label}.cpp"
        exe = tmp_path / label
        cpp.write_text(gate_fixture(candidate))
        subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            check=True, timeout=60,
        )
        return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)

    for index, compiler in enumerate(compilers):
        result = run(gate, compiler, f"display_gate_{index}")
        assert result.returncode == 0, result.stdout + result.stderr

        # Removing the new early gate recreates the original source defect:
        # either disabled preference continues into the landmark pass.
        missing_gate = gate.replace(
            "    if (!show_base_names) {\n"
            "        canvas->set_clip(&saved_clip);\n"
            "        return;\n"
            "    }\n", "", 1,
        )
        assert missing_gate != gate
        result = run(missing_gate, compiler, f"display_gate_missing_{index}")
        assert result.returncode != 0

        # Each half of the conjunction is independently load-bearing.
        missing_window_flag = gate.replace(
            "(field_1DD70_ & 0x800U) != 0", "true", 1,
        )
        assert missing_window_flag != gate
        result = run(missing_window_flag, compiler, f"display_gate_window_{index}")
        assert result.returncode != 0

        missing_preference_flag = gate.replace(
            "(GameMorePreferences & MPREF_MAP_SHOW_BASE_NAMES) != 0", "true", 1,
        )
        assert missing_preference_flag != gate
        result = run(missing_preference_flag, compiler, f"display_gate_preference_{index}")
        assert result.returncode != 0


def test_pinned_display_gates_share_landmark_cleanup_exit():
    from hashlib import sha256
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    path = Path(os.environ.get(
        "OPENSMACX_IMAGE", PROJECT / ".opensmacx/game/terranx_original.exe"
    ))
    assert path.is_file(), f"pinned original image missing: {path}"
    raw = path.read_bytes()
    pin = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    assert sha256(raw).hexdigest() == pin
    image = PeImage(raw, pin)
    instructions = {
        item.address: item
        for item in Cs(CS_ARCH_X86, CS_MODE_32).disasm(
            image.read(Span(0x467DE0, 0x4688E0)), 0x467DE0
        )
    }
    expected = {
        0x467E4F: ("mov", "eax, dword ptr [esi + 0x1dd70]"),
        0x467E55: ("test", "ah, 8"),
        0x467E58: ("je", "0x4688c6"),
        0x467E5E: ("mov", "eax, dword ptr [0x9a6494]"),
        0x467E63: ("test", "ah, 0x20"),
        0x467E66: ("je", "0x4688c6"),
        0x4688C6: ("mov", "edx, dword ptr [esi]"),
        0x4688D6: ("call", "0x5d8000"),
        0x4688DF: ("ret", ""),
        0x4686D7: ("call", "0x5dacb0"),
    }
    for address, (mnemonic, operands) in expected.items():
        assert (instructions[address].mnemonic, instructions[address].op_str.lower()) == (
            mnemonic, operands
        ), hex(address)
