"""Apply-ready controls over installed source; does not import the emitter."""
from __future__ import annotations

import shutil
import subprocess
import os
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import Mode, SourceRole, read_file, read_text


def root_dir() -> Path:
    override = os.environ.get("OPENSMACX_SOURCE_ROOT")
    if override:
        return Path(override).resolve()
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "CMakeLists.txt").is_file() and (candidate / "src/IMAGE_PIN").is_file():
            return candidate
    raise RuntimeError("repository root not found")


def _blank(match):
    return "".join("\n" if char == "\n" else " " for char in match.group(0))


def code_only(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(_blank, text)


def extract_function(text: str, signature: str) -> str:
    masked = code_only(text)
    assert masked.count(signature) == 1, signature
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated function {signature}")


def extract_block_containing(text: str, marker: str) -> str:
    masked = code_only(text)
    assert masked.count(marker) == 1, marker
    position = masked.index(marker)
    opening = masked.rfind("{", 0, position)
    assert opening >= 0
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return text[opening:end + 1]
    raise AssertionError(f"unterminated block containing {marker}")


TECH_LOOP_MARKER = "for (int row = 0; row < MaxTerrainNum - 1; ++row)"
TECH_HECK_SIGNATURE = "void __cdecl tech_heck(void (__cdecl *a1)(char *), int a2)"
EMIT_HELP_SIGNATURE = "static void emit_help(void (__cdecl *cb)(char *))"


def actual_tech_loop(source_root: Path | None = None) -> tuple[str, str]:
    root = source_root or root_dir()
    datalink = (root / "src/datalink.cpp").read_text()
    tech_heck = extract_function(datalink, TECH_HECK_SIGNATURE)
    loop = extract_block_containing(tech_heck, TECH_LOOP_MARKER)
    helper = extract_function(datalink, EMIT_HELP_SIGNATURE)
    return loop, helper


def test_installed_records_keep_seed_identity_and_history():
    root = root_dir()
    rows = {row.address: row for row in read_file(root / "src/datalink.cpp")}
    baseline = subprocess.check_output(
        ["git", "show", "5e3a67cd6:src/datalink.cpp"], cwd=root, text=True)
    previous = {row.address: row for row in read_text(baseline, "src/datalink.cpp")}
    for address in (0x0044B380, 0x0044B9E0):
        row = rows[address]
        old = previous[address]
        for field in ("name", "image_spans", "mode", "byte_exact", "semantic",
                      "symbol", "kind", "exclusion", "recipe", "levers",
                      "ruled_out", "unrecoverable", "deferred"):
            assert getattr(row, field) == getattr(old, field), (hex(address), field)
        assert row.mode is Mode.FILE
        assert row.source_role is SourceRole.UNCLASSIFIED
        assert row.source_role_reason == ""
        assert not row.extract_error


def test_installed_help_reset_and_report_store_paths_are_real_code():
    root = root_dir()
    datalink = (root / "src/datalink.cpp").read_text()
    technology = (root / "src/technology.cpp").read_text()
    reportwin = (root / "src/reportif.cpp").read_text()
    tech_heck = code_only(extract_function(
        datalink, "void __cdecl tech_heck(void (__cdecl *a1)(char *), int a2)"))
    help_info = code_only(extract_function(
        datalink, "void __cdecl help_tech_info(void (__cdecl *a1)(char *), int a2, int a3,"))
    achieved = code_only(extract_function(
        technology, "void __cdecl tech_achieved(int faction_id, int tech_id, int faction_id_with,"))
    assert "reinterpret_cast<int *>(0x00" not in tech_heck
    assert "reinterpret_cast<char *>(0x00" not in tech_heck
    assert "Technology[a2].flags" in tech_heck
    assert "reinterpret_cast<int *>(0x00" not in help_info
    assert "reinterpret_cast<char *>(0x00" not in help_info
    for kind in ("0x62", "0x63", "0x64"):
        assert help_info.count(f"TechnologyHelpEntryKind = {kind};") == 1
    sentinel = "TechnologyHelpEntryName = reinterpret_cast<LPSTR>(static_cast<intptr_t>(-1));"
    assert achieved.count(sentinel) == 1
    assert achieved.count("TechnologyHelpEntryKind = -1;") == 1
    selection = achieved.index("FirstProposedPrototype = -1;")
    reset = achieved.index(sentinel)
    presentation = achieved.index("say_tech(StringTemp, tech_id, FALSE);", reset)
    assert selection < reset < presentation

    helper_start = reportwin.index("static void reportwin_append_number(")
    helper_end = reportwin.index("// The three-per-category records", helper_start)
    helper_region = reportwin[helper_start:helper_end]
    section = code_only(extract_function(
        helper_region,
        "static void reportwin_draw_lab_unlock_sections(Buffer &canvas, int technology,"))
    for kind, table, detail in (
        ("0x62", "Chassis[i_chassis].offsv_1_name", "reportwin_append_chassis_lab_detail"),
        ("0x63", "Weapon[i_weapon].name", "reportwin_append_weapon_lab_detail"),
        ("0x64", "Armor[i_armor].name", "reportwin_append_armor_lab_detail"),
    ):
        assert section.count(f"TechnologyHelpEntryKind = {kind};") == 1
        entry = section.rfind("reportwin_append_lab_entry", 0,
                              section.index(f"TechnologyHelpEntryKind = {kind};"))
        kind_at = section.index(f"TechnologyHelpEntryKind = {kind};")
        name_at = section.index("TechnologyHelpEntryName =", kind_at)
        detail_at = section.index(detail, name_at)
        gate = section.rfind("if (draw_pass) {", 0, kind_at)
        gate_end = section.find("}", name_at)
        assert 0 <= entry < gate < kind_at < name_at < gate_end < detail_at


REPORT_HARNESS = r'''#include <cassert>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define __cdecl
typedef char *LPSTR;
struct RulesReactor { char *name; int preq_tech; };
struct RulesChassis { char *offsv_1_name; int preq_tech; int speed, range; };
struct RulesWeapon { char *name; signed char offense_rating; unsigned char mode; int preq_tech; };
struct RulesArmor { char *name; char defense_rating; int preq_tech; };
struct RulesAbility { char *name; int preq_tech; };
struct VehPrototype { unsigned char flags; int preq_tech; char veh_name[32]; };
struct RulesFacility { char *name; int preq_tech, free_tech; };
struct RulesCitizen { char *singular_name; int preq_tech; };
struct Table { char *get(char *name) { return name; } } StringTable;
struct Datalink { void construct_id(char *, int, int) {} } DatalinkMain;
struct Buffer {
  int wrapped, measured;
  char last[2048];
  int wrap(char *s, int, int y, int) {
    ++wrapped; strcpy(last, s); return y + 1;
  }
  int wrap_height(char *s, int) {
    ++measured; strcpy(last, s); return 1;
  }
};
enum { MaxVehProtoFactionNum=64 };
RulesReactor Reactor[4]; RulesChassis Chassis[9]; RulesWeapon Weapon[26];
RulesArmor Armor[14]; RulesAbility Ability[29];
VehPrototype VehPrototypes[MaxVehProtoFactionNum];
RulesFacility Facility[134]; RulesCitizen Citizen[7];
char StringTemp[2048]; char *TechnologyHelpEntryName=(char*)-1; int TechnologyHelpEntryKind=-1;
char *label_get(int) { return (char*)"LABEL"; }
void help_create_link(char *, char *, char *) {}
int _itoa(int value, char *out, int radix) {
  if (radix == 10) sprintf(out, "%d", value); else sprintf(out, "%x", value);
  return 0;
}
__REPORT_HELPERS__
int main() {
  Buffer canvas = {}; uint32_t width = 100; int y = 0, measured = 0;
  char chassis_name[]="chassis", weapon_name[]="weapon", armor_name[]="armor";
  Chassis[0].preq_tech=7; Chassis[0].offsv_1_name=chassis_name; Chassis[0].speed=2;
  Weapon[0].preq_tech=8; Weapon[0].name=weapon_name; Weapon[0].offense_rating=1;
  Armor[0].preq_tech=9; Armor[0].name=armor_name; Armor[0].defense_rating=3;
  reportwin_draw_lab_unlock_sections(canvas,7,false,false,0,y,&width,measured);
  assert(TechnologyHelpEntryName==(char*)-1 && TechnologyHelpEntryKind==-1);
  assert(canvas.measured==1 && canvas.wrapped==0);
  reportwin_draw_lab_unlock_sections(canvas,7,false,true,0,y,&width,measured);
  assert(TechnologyHelpEntryName==chassis_name && TechnologyHelpEntryKind==0x62);
  assert(canvas.wrapped==1);
  reportwin_draw_lab_unlock_sections(canvas,8,false,false,0,y,&width,measured);
  assert(TechnologyHelpEntryName==chassis_name && TechnologyHelpEntryKind==0x62);
  assert(canvas.measured==2 && canvas.wrapped==1);
  reportwin_draw_lab_unlock_sections(canvas,8,false,true,0,y,&width,measured);
  assert(TechnologyHelpEntryName==weapon_name && TechnologyHelpEntryKind==0x63);
  assert(canvas.wrapped==2);
  reportwin_draw_lab_unlock_sections(canvas,9,false,false,0,y,&width,measured);
  assert(TechnologyHelpEntryName==weapon_name && TechnologyHelpEntryKind==0x63);
  assert(canvas.measured==3 && canvas.wrapped==2);
  reportwin_draw_lab_unlock_sections(canvas,9,false,true,0,y,&width,measured);
  assert(TechnologyHelpEntryName==armor_name && TechnologyHelpEntryKind==0x64);
  assert(canvas.wrapped==3);
  return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_installed_reportwin_draw_pass_host_control(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    root = root_dir()
    reportwin = (root / "src/reportif.cpp").read_text()
    number = extract_function(
        reportwin, "static void reportwin_append_number(char *text, int value)")
    start = reportwin.index("static void reportwin_append_lab_entry(")
    end = reportwin.index("// The three-per-category records", start)
    helpers = number + "\n\n" + reportwin[start:end]
    source = tmp_path / "installed-reportwin.cpp"
    binary = tmp_path / "installed-reportwin"
    source.write_text(REPORT_HARNESS.replace("__REPORT_HELPERS__", helpers))
    subprocess.run([executable, "-std=c++98", str(source), "-o", str(binary)],
                   check=True, capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)


TECH_LOOP_HARNESS = r'''#include <cassert>
#include <cstring>
#define __cdecl
typedef char *LPSTR;
struct RulesTerraforming { int prerequisite_tech[2]; };
struct OrderRow { char *order; char *order_sea; };
static const int MaxTerrainNum = 20;
static RulesTerraforming Terraforming[MaxTerrainNum];
static OrderRow Order[24];
static int TechnologyHelpPrefixEmitted = 1;
static int TechnologyHelpSectionSelector = 0;
static char TechnologyHelpPrefixText[] = "prefix";
static char StringTemp[64];
static char text_buffer[64];
static char *TextBufferGetPtr = text_buffer;
static int events[32], event_count;
static int parse_say_count, parse_string_count, emit_count, callback_mode;
static char order_seen[4][24];
static void parse_say(int, char *order, int, int) {
    events[event_count++] = 1;
    strcpy(order_seen[parse_say_count++], order);
    strcpy(TextBufferGetPtr, order);
}
static void parse_string(char *out, char *in) {
    events[event_count++] = 2;
    strcpy(out, in);
    ++parse_string_count;
}
static void tech_callback(char *) {
    events[event_count++] = 3;
    ++emit_count;
    if (emit_count == 1) {
        if (callback_mode == 1) {
            Terraforming[0].prerequisite_tech[0] = 9;
            Terraforming[0].prerequisite_tech[1] = 7;
        } else if (callback_mode == 2) {
            Terraforming[0].prerequisite_tech[0] = 7;
            Terraforming[0].prerequisite_tech[1] = 7;
        } else if (callback_mode == 3) {
            Terraforming[0].prerequisite_tech[0] = 9;
        }
    }
}
EMIT_HELP_BODY
static void run_loop(int a2, void (__cdecl *a1)(char *)) {
    LOOP_BODY
}
static void reset_case(int land, int sea, int mode) {
    for (int row = 0; row < MaxTerrainNum; ++row) {
        Terraforming[row].prerequisite_tech[0] = -1;
        Terraforming[row].prerequisite_tech[1] = -1;
    }
    Terraforming[0].prerequisite_tech[0] = land;
    Terraforming[0].prerequisite_tech[1] = sea;
    Order[4].order = (char *)"land-order";
    Order[4].order_sea = (char *)"sea-order";
    event_count = parse_say_count = parse_string_count = emit_count = 0;
    callback_mode = mode;
    TechnologyHelpPrefixEmitted = 1;
}
static void check_events(int expected_emit) {
    assert(emit_count == expected_emit);
    assert(parse_say_count == expected_emit);
    assert(parse_string_count == expected_emit);
    assert(event_count == expected_emit * 3);
    for (int i = 0; i < expected_emit; ++i) {
        assert(events[i * 3] == 1 && events[i * 3 + 1] == 2 && events[i * 3 + 2] == 3);
    }
}
int main() {
    reset_case(7, 8, 0);
    run_loop(7, tech_callback);
    check_events(1);
    assert(strcmp(order_seen[0], "land-order") == 0);

    // The land emitter changes both live row fields. The sea leg now matches
    // the requested technology and is no longer a duplicate of land.
    reset_case(7, 8, 1);
    run_loop(7, tech_callback);
    check_events(2);
    assert(strcmp(order_seen[0], "land-order") == 0);
    assert(strcmp(order_seen[1], "sea-order") == 0);

    // A callback can make the two current prerequisites equal; the sea leg
    // must be suppressed after its live read.
    reset_case(7, 8, 2);
    run_loop(7, tech_callback);
    check_events(1);

    // The live land value can also make a formerly duplicate pair distinct.
    reset_case(7, 7, 3);
    run_loop(7, tech_callback);
    check_events(2);
    assert(strcmp(order_seen[1], "sea-order") == 0);
    return 0;
}
'''


def _replace_once(text: str, old: str, new: str) -> str:
    assert text.count(old) == 1, (old, text.count(old))
    return text.replace(old, new, 1)


def _cached_row_mutant(loop: str) -> str:
    live = """for (int i = 0; i < 2; ++i) {
                const RulesTerraforming &terraform = Terraforming[row];
                const int prerequisite = i == 0 ? terraform.prerequisite_tech[0]
                                                : terraform.prerequisite_tech[1];"""
    snapshot = """const RulesTerraforming &terraform = Terraforming[row];
            const int prereq[2] = {terraform.prerequisite_tech[0], terraform.prerequisite_tech[1]};
            for (int i = 0; i < 2; ++i) {
                const int prerequisite = prereq[i];"""
    mutant = _replace_once(loop, live, snapshot)
    return _replace_once(
        mutant,
        "(i != 1 || terraform.prerequisite_tech[0] != terraform.prerequisite_tech[1])",
        "(i != 1 || prereq[0] != prereq[1])",
    )


def _compile_and_run_loop(loop: str, helper: str, compiler: str,
                          tmp_path: Path, expect_success: bool) -> None:
    tmp_path.mkdir(parents=True, exist_ok=True)
    source = tmp_path / "tech-loop.cpp"
    binary = tmp_path / "tech-loop"
    program = TECH_LOOP_HARNESS.replace("EMIT_HELP_BODY", helper).replace("LOOP_BODY", loop)
    source.write_text(program)
    built = subprocess.run([compiler, "-std=c++98", str(source), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    if expect_success:
        assert result.returncode == 0, result.stderr
    else:
        assert result.returncode != 0, "causal stale-read mutant unexpectedly passed"


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_tech_heck_loop_reloads_live_land_and_sea_prerequisites(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    loop, helper = actual_tech_loop()
    _compile_and_run_loop(loop, helper, executable, tmp_path, True)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_tech_heck_loop_rejects_cached_row_and_missing_duplicate_suppression(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    loop, helper = actual_tech_loop()
    cached = _cached_row_mutant(loop)
    no_duplicate_suppression = _replace_once(
        loop,
        "(i != 1 || terraform.prerequisite_tech[0] != terraform.prerequisite_tech[1])",
        "(i != 1 || true)",
    )
    _compile_and_run_loop(cached, helper, executable, tmp_path / "cached", False)
    _compile_and_run_loop(no_duplicate_suppression, helper, executable,
                          tmp_path / "duplicate", False)
