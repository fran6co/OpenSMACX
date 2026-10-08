"""Actual-body controls for pinned say_stats separator and name branches.

The full owner is extracted from current src/veh.cpp. Host-only pointer-to-int
casts are explicitly narrowed through intptr_t; lookup compares the narrowed
addresses of fixture strings and never dereferences a key. These bounded
controls test sea, air, and transport formatting, not all owner branches or
native ABI. An optional pinned-image witness skips when the image is absent.
"""

from pathlib import Path
import shutil
import re
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src" / "veh.cpp"
SIGNATURE = "void __cdecl say_stats(LPSTR stat, int proto_id, LPSTR custom_spacer)"


def extract_function(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unterminated function: {signature}")


def actual_body() -> str:
    body = extract_function(SOURCE.read_text(encoding="utf-8"), SIGNATURE)
    # 32-bit Windows accepts these C-style pointer-to-int casts. Narrow through
    # intptr_t solely for the 64-bit host fixture; fake keys stay low and are
    # never dereferenced. Keep this compatibility rewrite explicit.
    replacements = {
        "(int)PlansFullName[3]": "static_cast<int>(reinterpret_cast<intptr_t>(PlansFullName[3]))",
        "(int)*((LPSTR *)Labels.strings_ptr + 312)": "static_cast<int>(reinterpret_cast<intptr_t>(*((LPSTR *)Labels.strings_ptr + 312)))",
        "(int)*((LPSTR *)Labels.strings_ptr + 196)": "static_cast<int>(reinterpret_cast<intptr_t>(*((LPSTR *)Labels.strings_ptr + 196)))",
        "(int)PlansShortName[plan]": "static_cast<int>(reinterpret_cast<intptr_t>(PlansShortName[plan]))",
        "int(PlansShortName[mode])": "static_cast<int>(reinterpret_cast<intptr_t>(PlansShortName[mode]))",
        "(int)*((LPSTR *)Labels.strings_ptr + 163)": "static_cast<int>(reinterpret_cast<intptr_t>(*((LPSTR *)Labels.strings_ptr + 163)))",
        "(int)*((LPSTR *)Labels.strings_ptr + 162)": "static_cast<int>(reinterpret_cast<intptr_t>(*((LPSTR *)Labels.strings_ptr + 162)))",
        "(int)PlansFullName[mode]": "static_cast<int>(reinterpret_cast<intptr_t>(PlansFullName[mode]))",
    }
    for old, new in replacements.items():
        body = body.replace(old, new)
    return body


PREFIX = r'''#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#define __cdecl
typedef char *LPSTR;
enum { PLAN_RECONNAISANCE=1, PLAN_DEFENSIVE=2, PLAN_NAVAL_TRANSPORT=3,
       TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2, WPN_MODE_TRANSPORT=4,
       CHSI_FOIL=1 };
struct Proto { uint8_t plan, chassis_id, weapon_id, ability_flags;
               uint8_t carry_capacity; uint32_t reactor_id; };
struct ChassisInfo { uint8_t triad, speed; };
struct WeaponInfo { uint8_t mode; };
struct RulesInfo { int move_rate_roads; };
struct LabelsInfo { LPSTR *strings_ptr; };
static Proto VehPrototypes[8] = {};
static ChassisInfo Chassis[8] = {};
static WeaponInfo Weapon[8] = {};
static RulesInfo Rules = { 3 };
static LabelsInfo Labels = {};
static LPSTR PlansFullName[16] = {};
static LPSTR PlansShortName[16] = {};
static LPSTR label_storage[313] = {};
static char sea_name[] = "Sea";
static char air_name[] = "Air";
static char plan_name[] = "Transport";
static char empty_name[] = "";
static int key_for(const char *pointer) {
    return static_cast<int>(reinterpret_cast<intptr_t>(pointer));
}
struct FakeStringTable {
    char *get(int key) {
        if (key == key_for(plan_name)) return plan_name;
        if (key == key_for(sea_name)) return sea_name;
        if (key == key_for(air_name)) return air_name;
        if (key == key_for(empty_name)) return empty_name;
        assert(!"unexpected label key");
        return empty_name;
    }
} StringTable;
static int get_proto_offense_rating(int) { return 1; }
static int get_proto_defense_rating(int) { return 1; }
static int speed_proto(int) { return 3; }
static void say_offense(LPSTR out, int) { strcat(out, "1"); }
static void say_defense(LPSTR out, int) { strcat(out, "1"); }
static char *_itoa(int value, char *out, int radix) {
    assert(radix == 10);
    sprintf(out, "%d", value);
    return out;
}
'''


MAIN = r'''
static void prepare_case(int triad, int plan, int capacity) {
    VehPrototypes[0].plan = 0;
    VehPrototypes[0].chassis_id = 0;
    VehPrototypes[0].weapon_id = 0;
    VehPrototypes[0].ability_flags = 0;
    VehPrototypes[0].plan = static_cast<uint8_t>(plan);
    VehPrototypes[0].carry_capacity = static_cast<uint8_t>(capacity);
    VehPrototypes[0].reactor_id = 1;
    Chassis[0].triad = static_cast<uint8_t>(triad);
    Chassis[0].speed = 1;
    Weapon[0].mode = 3;
    PlansFullName[3] = plan_name;
    label_storage[163] = sea_name;
    label_storage[162] = air_name;
    Labels.strings_ptr = label_storage;
}

static void check_output(int triad, int plan, int capacity, const char *expected) {
    prepare_case(triad, plan, capacity);
    char output[128] = {};
    say_stats(output, 0, 0);
    assert(strcmp(output, expected) == 0);
}

int main() {
    check_output(TRIAD_SEA, 0, 0, "Transport,  Sea");
    check_output(TRIAD_AIR, 0, 0, "Transport,  Air");
    check_output(TRIAD_SEA, PLAN_NAVAL_TRANSPORT, 5, "Transport(5),  Sea");
    check_output(TRIAD_LAND, PLAN_NAVAL_TRANSPORT, 5, "Transport(5)");
    return 0;
}
'''


def compile_fixture(body: str, compiler: str, tmp_path: Path, name: str):
    source = PREFIX + body + MAIN
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    build = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=60, check=False,
    )
    return build, binary


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_current_actual_say_stats_body_matches_pinned_sea_air_and_transport(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = actual_body()
    assert 'strcat(stat, ", "); // Original separator; the later triad space remains.' in body
    build, binary = compile_fixture(body, compiler, tmp_path, f"say-stats-{compiler_name}")
    assert build.returncode == 0, build.stdout + build.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30, check=False)
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_missing_separator_space_mutant_compiles_but_fails_output(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    body = actual_body()
    anchor = 'strcat(stat, ", "); // Original separator; the later triad space remains.'
    assert body.count(anchor) == 1
    mutant = body.replace(anchor, 'strcat(stat, ","); // Mutated separator', 1)
    build, binary = compile_fixture(mutant, compiler, tmp_path, f"say-stats-mutant-{compiler_name}")
    assert build.returncode == 0, build.stdout + build.stderr
    run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30, check=False)
    assert run.returncode != 0, "missing-space formatting mutant survived"


def test_optional_pinned_image_separator_literals():
    image_path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.exists():
        pytest.skip("pinned executable is absent; host source controls remain independent")
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    assert image.read(Span(0x00682E90, 0x00682E93)) == b", \0"
    assert image.read(Span(0x00682820, 0x00682822)) == b" \0"
