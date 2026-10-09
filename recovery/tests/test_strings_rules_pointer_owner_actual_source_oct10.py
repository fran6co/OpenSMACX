"""Bounded host controls for the recovered LPSTR rule-string flows.

These compile source-extracted bodies/blocks; they do not claim Windows ABI,
runtime or whole-project coverage.
"""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = Path(__file__).resolve().parents[2]


def code_only(text: str) -> str:
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if c == "\n" else " " for c in match.group()),
        text,
    )


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
    raise AssertionError(f"unterminated source body: {signature}")


def compiler() -> str:
    result = shutil.which("g++") or shutil.which("clang++")
    if result is None:
        pytest.skip("host C++ compiler unavailable")
    return result


def build_and_run(tmp_path: Path, source: str, *, expected_success: bool) -> None:
    cc = compiler()
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / "source.cpp"
    exe = tmp_path / "source-test"
    cpp.write_text(source)
    built = subprocess.run(
        [cc, "-std=c++11", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
        capture_output=True, text=True,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True)
    assert (ran.returncode == 0) is expected_success, ran.stdout + ran.stderr


def test_rules_callers_keep_pointer_owners_and_original_indices() -> None:
    datalink = (ROOT / "src/datalink.cpp").read_text()
    header = (ROOT / "src/datalink.h").read_text()
    reportwin = (ROOT / "src/reportwin.cpp").read_text()
    technology = (ROOT / "src/technology.cpp").read_text()
    veh = (ROOT / "src/veh.cpp").read_text()

    assert "datalink_string_key" not in code_only(datalink)
    assert "extern LPSTR TechnologyHelpEntryName;" in header
    assert "LPSTR TechnologyHelpEntryName;" in code_only(datalink)
    assert "TerraformNameBase" not in code_only(datalink)
    category_macro = next(line for line in datalink.splitlines()
                          if line.startswith("#define CATEGORY_LABEL("))
    assert category_macro == "#define CATEGORY_LABEL(slot) StringTable.get(Labels.strings_ptr[(slot)])"
    terraform = code_only(extract_function(
        datalink, "void Datalink::set_cat_terraform()"))
    assert "const RulesOrder &order = Order[4 + terraform_index];" in terraform
    assert "StringTable.get(order.order)" in terraform
    assert "StringTable.get(order.order_sea)" in terraform

    weapon = extract_function(
        datalink,
        "void __cdecl help_tech_info(void (__cdecl *a1)(char *), int a2, int a3,")
    weapon_section = weapon[
        weapon.index("// ---- section 3: weapon records"):
        weapon.index("// ---- section 4: armor records")]
    weapon_code = code_only(weapon_section)
    assert "TechnologyHelpEntryName = record.name;" in weapon_code
    assert "record.name) == a2" not in weapon_code
    assert weapon_section.index("TechnologyHelpEntryKind = 0x63;") < weapon_section.index(
        "TechnologyHelpEntryName = record.name;") < weapon_section.index(
        'strcat(buf, " ");') < weapon_section.index('strcat(buf, "(");')

    unlock = code_only(extract_function(
        reportwin,
        "static void reportwin_draw_lab_unlock_sections(Buffer &canvas, int technology,"))
    for table in ("Chassis[i_chassis].offsv_1_name", "Weapon[i_weapon].name",
                  "Armor[i_armor].name"):
        assert f"TechnologyHelpEntryName = {table};" in unlock
    assert "TechnologyHelpEntryName = reinterpret_cast<LPSTR>(static_cast<intptr_t>(-1));" in code_only(technology)

    goody_call = "parse_say(0, Labels.strings_ptr[0x5B + existing_bonus],"
    assert goody_call in code_only(veh)
    assert "parse_say(0, existing_bonus, -1, -1);" not in code_only(veh)


WEAPON_HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstring>
typedef char *LPSTR;
struct RulesWeapon { int preq_tech; LPSTR name; signed char offense_rating; unsigned char mode; };
const int MaxWeaponNum = 26;
RulesWeapon Weapon[MaxWeaponNum] = {};
LPSTR TechnologyHelpEntryName = 0;
int TechnologyHelpEntryKind = -1;
struct StringTableSpy {
  LPSTR last; int calls;
  LPSTR get(LPSTR value) { last=value; ++calls; return value; }
} StringTable = {0, 0};
struct DatalinkSpy {
  void construct_id(char *out, int category, int index) {
    std::sprintf(out, "id%d:%d", category, index);
  }
} DatalinkMain;
static char label_strength[] = "Strength";
static char label_mode[] = "Mode";
LPSTR label_get(int slot) { return slot == 0x310/4 ? label_strength : label_mode; }
static char emitted[2048]; static int emits;
void emit(char *text) { std::strcpy(emitted, text); ++emits; }
static char * _itoa(int value, char *out, int radix) {
  assert(radix == 10); std::sprintf(out, "%d", value); return out;
}
void run(int a2, int a4, char *buf, char *numbuf, char *idbuf,
         void (*a1)(char *)) {
  int idx = 0; bool flushed = false;
  __WEAPON_SECTION__
}
int main() {
  char name[] = "Laser"; char buf[2048] = {}; char numbuf[80], idbuf[64];
  Weapon[0].preq_tech=7; Weapon[0].name=name;
  Weapon[0].offense_rating=3; Weapon[0].mode=1;
  run(7, 0, buf, numbuf, idbuf, emit);
  assert(emits==1 && std::strcmp(emitted, "^$LINK<Laser=id6:0> Strength(3)")==0);
  assert(TechnologyHelpEntryKind==0x63 && TechnologyHelpEntryName==name);
  assert(StringTable.calls==1 && StringTable.last==name);

  emits=0; StringTable.calls=0; TechnologyHelpEntryName=0;
  Weapon[0].name=name; Weapon[0].offense_rating=1; Weapon[0].mode=3;
  run(7, 0, buf, numbuf, idbuf, emit);
  assert(emits==1 && std::strcmp(emitted, "^$LINK<Laser=id6:0> Strength(Mode)")==0);
  assert(TechnologyHelpEntryName==name && StringTable.last==name);

  emits=0; StringTable.calls=0;
  Weapon[0].offense_rating=-1; Weapon[0].mode=1;
  run(7, 0, buf, numbuf, idbuf, emit);
  assert(emits==1 && std::strcmp(emitted, "^$LINK<Laser=id6:0> Strength(Strength)")==0);

  emits=0; StringTable.calls=0;
  Weapon[0].offense_rating=3; Weapon[0].mode=1;
  run(7, -1, buf, numbuf, idbuf, emit);
  assert(emits==1 && std::strcmp(emitted,
      "^Mode: $LINK<Laser=id6:0> Strength(3)")==0);

  emits=0; StringTable.calls=0; TechnologyHelpEntryName=name;
  run(8, 0, buf, numbuf, idbuf, emit);
  assert(emits==0 && StringTable.calls==0 && TechnologyHelpEntryName==name);
  return 0;
}
'''


def test_actual_weapon_format_body_and_pointer_identity(tmp_path: Path) -> None:
    datalink = (ROOT / "src/datalink.cpp").read_text()
    body = extract_function(
        datalink,
        "void __cdecl help_tech_info(void (__cdecl *a1)(char *), int a2, int a3,")
    section = body[
        body.index("// ---- section 3: weapon records"):
        body.index("// ---- section 4: armor records")]
    expected = WEAPON_HARNESS.replace("__WEAPON_SECTION__", section)
    build_and_run(tmp_path / "real", expected, expected_success=True)

    old_wrap = (
        '            TechnologyHelpEntryName = record.name;\n'
        '            strcat(buf, " ");\n'
        '            char *s2 = label_get(0x310 / 4);\n'
        '            strcat(buf, s2);\n'
        '            strcat(buf, "(");')
    mutant_wrap = (
        '            TechnologyHelpEntryName = record.name;\n'
        '            if (static_cast<int>(reinterpret_cast<intptr_t>(record.name)) == a2) {\n'
        '              strcat(buf, " ");\n'
        '              char *s2 = label_get(0x310 / 4);\n'
        '              strcat(buf, s2);\n'
        '              strcat(buf, "(");\n'
        '            }')
    assert old_wrap in section
    mutant = expected.replace(section, section.replace(old_wrap, mutant_wrap))
    # The legacy pointer-vs-tech-id gate compiles, but it drops original text.
    build_and_run(tmp_path / "mutant", mutant, expected_success=False)


def test_actual_tech_reset_and_goody_label_pointer_boundaries(tmp_path: Path) -> None:
    technology = (ROOT / "src/technology.cpp").read_text()
    achieved = code_only(extract_function(
        technology,
        "void __cdecl tech_achieved(int faction_id, int tech_id, int faction_id_with,"))
    reset_start = achieved.index("TechnologyHelpEntryName = reinterpret_cast<LPSTR>(static_cast<intptr_t>(-1));")
    reset_end = achieved.index("TechnologyHelpEntryKind = -1;", reset_start) + len(
        "TechnologyHelpEntryKind = -1;")
    reset_block = achieved[reset_start:reset_end]
    reset_harness = r'''#include <cassert>
#include <stdint.h>
typedef char *LPSTR; LPSTR TechnologyHelpEntryName=0; int TechnologyHelpEntryKind=0;
void reset() { __RESET_BLOCK__ }
int main() { reset(); assert((uintptr_t)TechnologyHelpEntryName == ~(uintptr_t)0);
  assert(TechnologyHelpEntryKind == -1); return 0; }
'''.replace("__RESET_BLOCK__", reset_block)
    build_and_run(tmp_path / "reset", reset_harness, expected_success=True)

    veh = code_only((ROOT / "src/veh.cpp").read_text())
    statement = "parse_say(0, Labels.strings_ptr[0x5B + existing_bonus],\n                              -1, -1);"
    assert statement in veh
    goody_harness = r'''#include <cassert>
#include <stdint.h>
typedef char *LPSTR;
LPSTR labels[0x70]; LPSTR received=0; int label_get_calls=0;
struct { LPSTR *strings_ptr; } Labels={labels};
LPSTR label_get(int i) { ++label_get_calls; return labels[i]; }
void parse_say(int, LPSTR text, int, int) { received=text; }
int main() { static char label_text[0x70][16];
  for (int i=0;i<0x70;++i) labels[i]=label_text[i];
  int existing_bonus=3; __GOODY_STATEMENT__
  assert(received==labels[0x5E] && label_get_calls==0); return 0; }
'''.replace("__GOODY_STATEMENT__", statement)
    build_and_run(tmp_path / "goody", goody_harness, expected_success=True)

    # A casted reconstruction of the prior wrong numeric call compiles but
    # supplies neither the original label index nor its owned pointer.
    mutant = goody_harness.replace(
        statement,
        "parse_say(0, label_get(0x5B + existing_bonus), -1, -1);")
    build_and_run(tmp_path / "goody-mutant", mutant, expected_success=False)


def test_actual_category_macro_uses_label_indices_not_byte_offsets(tmp_path: Path) -> None:
    datalink = (ROOT / "src/datalink.cpp").read_text()
    macro = next(line for line in datalink.splitlines()
                 if line.startswith("#define CATEGORY_LABEL("))
    harness = r'''#include <cassert>
typedef char *LPSTR;
LPSTR label_values[800];
struct { LPSTR *strings_ptr; } Labels = {label_values};
struct Spy { LPSTR last; int calls; LPSTR get(LPSTR s) { last=s; ++calls; return s; } } StringTable={0,0};
__CATEGORY_MACRO__
int main() { static char text[800][16];
  for (int i=0;i<800;++i) label_values[i]=text[i];
  LPSTR first=CATEGORY_LABEL(714); assert(first==label_values[714]);
  assert(StringTable.last==label_values[714] && StringTable.calls==1);
  LPSTR second=CATEGORY_LABEL(178); assert(second==label_values[178]);
  assert(second!=first && StringTable.last==label_values[178] && StringTable.calls==2);
  return 0; }
'''.replace("__CATEGORY_MACRO__", macro)
    build_and_run(tmp_path / "actual", harness, expected_success=True)
    mutant_macro = macro.replace("[(slot)]", "[(slot) / 4]")
    mutant = harness.replace(macro, mutant_macro)
    build_and_run(tmp_path / "wrong-stride", mutant, expected_success=False)


def test_actual_reactor_zero_keeps_borrowed_palette_address(tmp_path: Path) -> None:
    veh = (ROOT / "src/veh.cpp").read_text()
    rule_helper = extract_function(
        veh, "static const char *prototype_rule_name(LPSTR string_id)")
    reactor_helper = extract_function(
        veh, "static const char *prototype_reactor_name(int reactor_id)")
    harness = r'''#include <cassert>
typedef char *LPSTR;
struct RulesReactor { LPSTR name_short; };
RulesReactor Reactor[4] = {};
static unsigned char borrowed_palette[8][4];
struct PaletteSpy {
  const void *animation_colours(int slot) const { return borrowed_palette[slot]; }
} g_PALETTE2;
static char resolved[]="resolved";
struct StringSpy {
  LPSTR last; int calls;
  LPSTR get(LPSTR address) { last=address; ++calls; return resolved; }
} StringTable={0,0};
__SOURCE_HELPERS__
int main() {
  char reactor_name[]="reactor";
  Reactor[0].name_short=reactor_name;
  assert(prototype_reactor_name(0)==resolved);
  assert(StringTable.last==reinterpret_cast<char *>(borrowed_palette[4]));
  assert(StringTable.calls==1);
  assert(prototype_reactor_name(1)==resolved);
  assert(StringTable.last==reactor_name && StringTable.calls==2);
  return 0;
}
'''.replace("__SOURCE_HELPERS__", rule_helper + "\n" + reactor_helper)
    build_and_run(tmp_path / "actual", harness, expected_success=True)

    wrong_slot = reactor_helper.replace("animation_colours(4)",
                                        "animation_colours(3)")
    mutant = harness.replace(reactor_helper, wrong_slot)
    build_and_run(tmp_path / "wrong-slot", mutant, expected_success=False)
