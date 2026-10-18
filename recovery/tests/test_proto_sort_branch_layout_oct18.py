"""Behavior controls for the actual proto_sort definition.

The host harness executes the extracted production body with bounded table and
call doubles. Its oracle independently models the 128 prototype slots and
checks the IDs/keys passed to a deterministic sort double; it does not test the
recovered sort implementation. It checks source semantics, not the candidate's
branch spelling, and is not a native ABI or game-runtime proof.
"""

from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/map.cpp"
PIN_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
PROTO_SORT_SPAN = (0x005A6270, 0x005A63C9)


def _definition(text: str, signature: str) -> str:
    match = re.search(signature, text)
    assert match, f"definition not found: {signature}"
    opening = text.index("{", match.start())
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                return text[match.start():end + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def _actual_struct(name: str) -> str:
    source = (ROOT / "src/veh.h").read_text(encoding="utf-8")
    return _definition(source, rf"struct\s+{re.escape(name)}\s*\{{")


def _actual_body() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    return _definition(source, r"void\s+__cdecl\s+proto_sort\s*\(\s*int\s+faction_id\s*\)")


SUPPORT = r'''
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>
#define __cdecl
using LPSTR = char *;
using BOOL = int;
// ACTUAL_TYPES
enum { PROTO_ACTIVE = 1 };
constexpr int MaxVehProtoFactionNum = 64;
constexpr int MaxVehProtoNum = 512;
VehPrototype VehPrototypes[MaxVehProtoNum];
RulesChassis Chassis[19];
int ProtoSortCount = -1;
int ProtoSortIds[128];
struct Call { char kind; int first; int second; };
std::vector<Call> calls;
std::vector<int> input_ids, input_keys, output_ids, output_keys;
int sort_calls = 0;
int tech_allowed[32][8];
int weapon_values[512][8];
int armor_values[512][8];
void check(bool ok, const char *what) {
    if (!ok) { std::fprintf(stderr, "proto_sort assertion: %s\n", what); std::exit(91); }
}
int has_tech(int tech, int faction) {
    calls.push_back({'T', tech, faction});
    return tech_allowed[tech][faction];
}
int weap_val(int prototype_id, int faction) {
    calls.push_back({'W', prototype_id, faction});
    return weapon_values[prototype_id][faction];
}
int arm_val(int armor_id, int faction) {
    calls.push_back({'A', armor_id, faction});
    return armor_values[armor_id][faction];
}
void sort(int count, int *ids, int *keys) {
    ++sort_calls;
    check(count == 128 && ids == ProtoSortIds, "sort arguments");
    input_ids.assign(ids, ids + count);
    input_keys.assign(keys, keys + count);
    std::vector<int> order(count);
    for (int i = 0; i < count; ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [keys](int left, int right) {
        return keys[left] < keys[right];
    });
    for (int i = 0; i < count; ++i) {
        output_ids.push_back(ids[order[i]]);
        output_keys.push_back(keys[order[i]]);
    }
    for (int i = 0; i < count; ++i) {
        ids[i] = output_ids[i];
        keys[i] = output_keys[i];
    }
}
unsigned state = 0xC001D00Du;
unsigned next_random() { state = state * 1664525u + 1013904223u; return state; }
'''


HARNESS = r'''
struct ExpectedCall { char kind; int first; int second; };
void same_calls(const std::vector<Call> &actual, const std::vector<ExpectedCall> &expected) {
    check(actual.size() == expected.size(), "call count/order length");
    for (std::size_t i = 0; i < actual.size(); ++i) {
        check(actual[i].kind == expected[i].kind
              && actual[i].first == expected[i].first
              && actual[i].second == expected[i].second, "ordered helper call");
    }
}
void run_case(int faction, unsigned seed) {
    state = seed;
    calls.clear(); input_ids.clear(); input_keys.clear();
    output_ids.clear(); output_keys.clear(); sort_calls = 0;
    for (int i = 0; i < MaxVehProtoNum; ++i) {
        VehPrototypes[i] = VehPrototype{};
        VehPrototypes[i].flags = (next_random() % 5u) ? PROTO_ACTIVE : 0;
        VehPrototypes[i].preq_tech = static_cast<int16_t>(next_random() % 32u);
        VehPrototypes[i].chassis_id = static_cast<uint8_t>(next_random() % 19u);
        VehPrototypes[i].plan = static_cast<uint8_t>(next_random() % 15u);
        VehPrototypes[i].armor_id = static_cast<uint8_t>(next_random() % 14u);
        VehPrototypes[i].weapon_id = static_cast<uint8_t>(next_random() % 26u);
        weapon_values[i][faction] = static_cast<int>(next_random() % 33u);
        armor_values[i][faction] = static_cast<int>(next_random() % 10u);
    }
    for (int i = 0; i < 19; ++i) {
        Chassis[i].triad = static_cast<int>(next_random() % 3u);
        Chassis[i].speed = static_cast<int>(next_random() % 9u);
    }
    for (int tech = 0; tech < 32; ++tech) {
        for (int f = 0; f < 8; ++f) tech_allowed[tech][f] = (next_random() >> 29) & 1u;
    }
    // Force all decision categories, including the exact 63/64 boundary.
    VehPrototypes[0].flags = PROTO_ACTIVE;
    VehPrototypes[0].preq_tech = 1;
    tech_allowed[1][faction] = 0;                 // denied predefined
    VehPrototypes[1].flags = 0;                   // inactive predefined
    VehPrototypes[2].flags = PROTO_ACTIVE;
    VehPrototypes[2].preq_tech = 1;               // still denied when faction is zero
    VehPrototypes[3].flags = 0;
    VehPrototypes[63].flags = PROTO_ACTIVE;
    VehPrototypes[63].preq_tech = 2;
    tech_allowed[2][faction] = 1;                 // available last predefined
    const int first_faction = faction * 64;
    VehPrototypes[first_faction].flags = 0;       // inactive first faction slot
    VehPrototypes[first_faction + 1].flags = PROTO_ACTIVE;

    std::vector<int> expected_ids, expected_keys;
    std::vector<ExpectedCall> expected_calls;
    int expected_count = 0;
    for (int ordinal = 0; ordinal < 128; ++ordinal) {
        const int id = (ordinal & 63) + (ordinal < 64 ? 0 : faction * 64);
        const VehPrototype &p = VehPrototypes[id];
        expected_ids.push_back(id);
        const bool active = (p.flags & PROTO_ACTIVE) != 0;
        const bool tech_ok = id >= 64 || (tech_allowed[p.preq_tech][faction] != 0);
        if (active && id < 64) expected_calls.push_back({'T', p.preq_tech, faction});
        const bool available = active && tech_ok;
        int key;
        if (!available) {
            if (id >= 64) { key = 0x34FB5E38; ++expected_count; }
            else key = 999999999;
        } else {
            expected_calls.push_back({'W', id, faction});
            expected_calls.push_back({'A', p.armor_id, faction});
            key = (Chassis[p.chassis_id].triad + p.plan * 16) << 18;
            key += (32 - weapon_values[id][faction]) << 9;
            key += (8 - Chassis[p.chassis_id].speed) << 6;
            key += 8 - armor_values[p.armor_id][faction];
            ++expected_count;
        }
        expected_keys.push_back(key);
    }
    std::vector<int> order(128);
    for (int i = 0; i < 128; ++i) order[i] = i;
    std::stable_sort(order.begin(), order.end(), [&expected_keys](int left, int right) {
        return expected_keys[left] < expected_keys[right];
    });
    std::vector<int> expected_sorted_ids, expected_sorted_keys;
    for (int index : order) {
        expected_sorted_ids.push_back(expected_ids[index]);
        expected_sorted_keys.push_back(expected_keys[index]);
    }

    const int calls_before = sort_calls;
    proto_sort(faction);
    check(sort_calls == calls_before + 1, "one final sort");
    check(ProtoSortCount == expected_count, "available count reset and accumulated");
    check(input_ids == expected_ids && input_keys == expected_keys, "all 128 source ids/keys");
    check(output_ids == expected_sorted_ids && output_keys == expected_sorted_keys,
          "stable sorted output");
    check(std::equal(ProtoSortIds, ProtoSortIds + 128, expected_sorted_ids.begin()),
          "published sorted ids");
    same_calls(calls, expected_calls);
}
int main() {
    for (int i = 0; i < 128; ++i) ProtoSortIds[i] = -777;
    ProtoSortCount = 12345;
    run_case(1, 0x12345678u);
    run_case(3, 0xA5A5F00Du); // overwrites prior arrays and resets count
    run_case(7, 0x0BADCAFEu);
    run_case(0, 0x76543210u); // overlapping predefined/faction IDs remain observable
    return 0;
}
'''


def _program(body: str, threshold: int = 64) -> str:
    return (SUPPORT.replace("// ACTUAL_TYPES", _actual_struct("VehPrototype") + ";\n"
                            + _actual_struct("RulesChassis") + ";")
            .replace("constexpr int MaxVehProtoFactionNum = 64;",
                     f"constexpr int MaxVehProtoFactionNum = {threshold};")
            + body + HARNESS)


def _compile_run(source: str, compiler: str, optimization: str,
                 tmp_path: Path) -> subprocess.CompletedProcess[str]:
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} is unavailable")
    cpp = tmp_path / f"proto_sort_{compiler}_{optimization[1:]}.cpp"
    binary = cpp.with_suffix("")
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [executable, "-std=c++17", "-Wall", "-Wextra", "-Werror", optimization,
         str(cpp), "-o", str(binary)], capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_actual_proto_sort_semantics_across_all_slots_and_repeated_calls(
    compiler: str, optimization: str, tmp_path: Path,
) -> None:
    ran = _compile_run(_program(_actual_body()), compiler, optimization, tmp_path)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def _assert_mutant_rejected(body: str, compiler: str, tmp_path: Path) -> None:
    ran = _compile_run(_program(body), compiler, "-O2", tmp_path)
    assert ran.returncode != 0, "behavior harness accepted a deliberate proto_sort mutant"


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_semantic_damage_controls_reject_key_count_and_boundary_mutants(
    compiler: str, tmp_path: Path,
) -> None:
    body = _actual_body()

    wrong_key, replacements = re.subn(
        r"key\s*=\s*0x34FB5E38\s*;", "key = 0x34FB5E39;", body, count=1,
    )
    assert replacements == 1
    _assert_mutant_rejected(wrong_key, compiler, tmp_path / "wrong-key")

    wrong_count, replacements = re.subn(
        r"ProtoSortCount\s*\+\+\s*;", "ProtoSortCount += 2;", body, count=1,
    )
    assert replacements == 1
    _assert_mutant_rejected(wrong_count, compiler, tmp_path / "wrong-count")

    threshold_test = re.compile(
        r"if\s*\(\s*proto_id\s*(>=|<)\s*MaxVehProtoFactionNum\s*\)"
    )
    match = threshold_test.search(body)
    assert match, "could not identify the inner faction/predefined split for a mutation control"
    comparator = match.group(1)
    replacement = ("if (proto_id >= MaxVehProtoFactionNum + 1)" if comparator == ">="
                   else "if (proto_id < MaxVehProtoFactionNum - 1)")
    wrong_threshold = body[:match.start()] + replacement + body[match.end():]
    _assert_mutant_rejected(wrong_threshold, compiler, tmp_path / "wrong-threshold")


def _pinned_image() -> Path:
    configured = os.environ.get("OPENSMACX_IMAGE")
    if configured:
        return Path(configured)
    local = ROOT / ".opensmacx/game/terranx_original.exe"
    if local.is_file():
        return local
    if len(ROOT.parents) > 2:
        coordinator = ROOT.parents[2] / ".opensmacx/game/terranx_original.exe"
        if coordinator.is_file():
            return coordinator
    return local


def test_pinned_original_branch_uses_signed_jl_and_high_arm_key() -> None:
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    from match_recovery.source.asm import Listing

    image_path = _pinned_image()
    if not image_path.is_file():
        pytest.skip("pinned original image unavailable; host behavior is not an original-code proof")
    data = image_path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN_SHA256
    span = Span(*PROTO_SORT_SPAN)
    code = PeImage(data, PIN_SHA256).read(span)
    assert len(code) == span.end - span.start
    listing = Listing(code, span.start).lines
    by_address = {int(line.strip().split()[0].rstrip(":"), 16): line for line in listing}
    assert "cmp ebx, 0x40" in by_address[0x005A6376].lower()
    assert "jl 0x5a638d" in by_address[0x005A6379].lower()
    assert "mov esi, 0x34fb5e38" in by_address[0x005A6380].lower()
    assert "inc eax" in by_address[0x005A6385].lower()
    assert "mov esi, 0x3b9ac9ff" in by_address[0x005A638D].lower()

    source = SOURCE.read_text(encoding="utf-8")
    mapped = source[source.rfind("// ORIGINAL:", 0, source.index("void __cdecl proto_sort(int faction_id)")):
                    source.index("void __cdecl proto_sort(int faction_id)")]
    assert "0x005A6270" in mapped and "0x005A63C9" in mapped
