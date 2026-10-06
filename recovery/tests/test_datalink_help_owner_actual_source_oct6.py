"""Apply-ready controls over installed source; does not import the emitter."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import Mode, SourceRole, read_file, read_text


def root_dir() -> Path:
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
    reportwin = (root / "src/reportwin.cpp").read_text()
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
    assert achieved.count("TechnologyHelpEntryName = -1;") == 1
    assert achieved.count("TechnologyHelpEntryKind = -1;") == 1
    selection = achieved.index("FirstProposedPrototype = -1;")
    reset = achieved.index("TechnologyHelpEntryName = -1;")
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
struct Table { char *get(int) { return (char*)"TABLE_NAME"; } } StringTable;
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
char StringTemp[2048]; int TechnologyHelpEntryName=-1, TechnologyHelpEntryKind=-1;
char *label_get(int) { return (char*)"LABEL"; }
void help_create_link(char *, char *, char *) {}
int _itoa(int value, char *out, int radix) {
  if (radix == 10) sprintf(out, "%d", value); else sprintf(out, "%x", value);
  return 0;
}
__REPORT_HELPERS__
int main() {
  Buffer canvas = {}; uint32_t width = 100; int y = 0, measured = 0;
  Chassis[0].preq_tech=7; Chassis[0].offsv_1_name=(char*)11; Chassis[0].speed=2;
  Weapon[0].preq_tech=8; Weapon[0].name=(char*)12; Weapon[0].offense_rating=1;
  Armor[0].preq_tech=9; Armor[0].name=(char*)13; Armor[0].defense_rating=3;
  reportwin_draw_lab_unlock_sections(canvas,7,false,false,0,y,&width,measured);
  assert(TechnologyHelpEntryName==-1 && TechnologyHelpEntryKind==-1);
  assert(canvas.measured==1 && canvas.wrapped==0);
  reportwin_draw_lab_unlock_sections(canvas,7,false,true,0,y,&width,measured);
  assert(TechnologyHelpEntryName==11 && TechnologyHelpEntryKind==0x62);
  assert(canvas.wrapped==1);
  reportwin_draw_lab_unlock_sections(canvas,8,false,false,0,y,&width,measured);
  assert(TechnologyHelpEntryName==11 && TechnologyHelpEntryKind==0x62);
  assert(canvas.measured==2 && canvas.wrapped==1);
  reportwin_draw_lab_unlock_sections(canvas,8,false,true,0,y,&width,measured);
  assert(TechnologyHelpEntryName==12 && TechnologyHelpEntryKind==0x63);
  assert(canvas.wrapped==2);
  reportwin_draw_lab_unlock_sections(canvas,9,false,false,0,y,&width,measured);
  assert(TechnologyHelpEntryName==12 && TechnologyHelpEntryKind==0x63);
  assert(canvas.measured==3 && canvas.wrapped==2);
  reportwin_draw_lab_unlock_sections(canvas,9,false,true,0,y,&width,measured);
  assert(TechnologyHelpEntryName==13 && TechnologyHelpEntryKind==0x64);
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
    reportwin = (root / "src/reportwin.cpp").read_text()
    number = extract_function(
        reportwin, "static void reportwin_append_number(char *text, int value)")
    start = reportwin.index("static void reportwin_append_lab_entry(")
    end = reportwin.index("// The three-per-category records", start)
    helpers = number + "\n\n" + reportwin[start:end]
    helpers = helpers.replace(
        "reinterpret_cast<int>(name)",
        "static_cast<int>(reinterpret_cast<intptr_t>(name))")
    source = tmp_path / "installed-reportwin.cpp"
    binary = tmp_path / "installed-reportwin"
    source.write_text(REPORT_HARNESS.replace("__REPORT_HELPERS__", helpers))
    subprocess.run([executable, "-std=c++98", str(source), "-o", str(binary)],
                   check=True, capture_output=True, text=True)
    subprocess.run([str(binary)], check=True, capture_output=True, text=True)
