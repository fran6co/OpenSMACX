"""Actual-source controls for generators' initial and live BaseCount reads."""
from __future__ import annotations

import hashlib
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest
from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
REPO_ROOT = next(parent for parent in Path(__file__).resolve().parents
                 if (parent / "src/IMAGE_PIN").is_file())
SOURCE = ROOT / "src/game.cpp"
PINNED_IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE", REPO_ROOT / ".opensmacx/game/terranx_original.exe"))
PINNED_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SPAN = Span(0x005247B0, 0x00524862)


CPP_PREFIX = r'''#include <stdint.h>
#include <string.h>
#define __cdecl
static const int RFLAG_ALIEN = 0x80;
struct Player { int rule_flags; };
// Only fields used by this function are modeled. The fixture does not claim
// to reproduce the game's physical Base layout.
struct Base { uint8_t faction_id_current; int8_t population_size; uint8_t facilities_built[12]; };
struct Fields { int base_current_count_; };
struct GameData { Fields fields_; };
struct RuleData { int size_base_subspace_gen; };
Player Players[8];
Base Bases[256];
GameData GameDataStateStorage;
RuleData Rules;
int facility_calls, mutate_mode, facility_enabled[256];
int has_fac_built(int facility, int index) {
  if (facility != 0x28) return 0;
  ++facility_calls;
  if (facility_calls == 1 && mutate_mode == 1) GameDataStateStorage.fields_.base_current_count_ = 3;
  if (facility_calls == 1 && mutate_mode == 2) GameDataStateStorage.fields_.base_current_count_ = 1;
  return facility_enabled[index]
      && (Bases[index].facilities_built[0x28 >> 3] & (1u << (0x28 & 7))) != 0;
}
'''


CPP_MAIN = r'''
static void reset(int total) {
  memset(Players, 0, sizeof(Players));
  memset(Bases, 0, sizeof(Bases));
  memset(facility_enabled, 1, sizeof(facility_enabled));
  GameDataStateStorage.fields_.base_current_count_ = total;
  Rules.size_base_subspace_gen = 2;
  facility_calls = 0;
  mutate_mode = 0;
}
static void make_base(int index, int faction, int population, int facility = 1) {
  Bases[index].faction_id_current = static_cast<uint8_t>(faction);
  Bases[index].population_size = static_cast<int8_t>(population);
  if (facility) Bases[index].facilities_built[0x28 >> 3] = 1u << (0x28 & 7);
}
int main() {
  int small = -1;

  // The initial positive guard enters once; the original loop-tail reload
  // observes growth made by a body call and visits all three bases.
  reset(1); Players[2].rule_flags = RFLAG_ALIEN;
  make_base(0, 2, 3); make_base(1, 2, 1); make_base(2, 2, -1);
  mutate_mode = 1;
  int large = generators(2, &small);
  if (large != 1 || small != 2 || facility_calls != 3) return 1;

  // Shrinking the live count during the first body stops before the next
  // base, even though the entry snapshot was three.
  reset(3); Players[2].rule_flags = RFLAG_ALIEN;
  make_base(0, 2, 3); make_base(1, 2, 1); make_base(2, 2, 2);
  mutate_mode = 2;
  large = generators(2, &small);
  if (large != 1 || small != 0 || facility_calls != 1) return 2;

  // A present owner with a missing facility follows the false-bit path and
  // does not contribute to either count.
  reset(2); Players[2].rule_flags = RFLAG_ALIEN;
  make_base(0, 2, 3, 0); make_base(1, 2, 1, 0);
  large = generators(2, &small);
  if (large != 0 || small != 0 || facility_calls != 2) return 3;

  // Initial nonpositive signed bounds do not enter, but initialize count_out.
  reset(0); Players[2].rule_flags = RFLAG_ALIEN;
  small = 9; large = generators(2, &small);
  if (large != 0 || small != 0 || facility_calls != 0) return 4;
  reset(-1); Players[2].rule_flags = RFLAG_ALIEN;
  small = 9; large = generators(2, &small);
  if (large != 0 || small != 0 || facility_calls != 0) return 5;

  // Mixed ownership and signed int8 population are retained. Equal-to-limit
  // counts as large; lower and negative population count only through out.
  reset(4); Players[2].rule_flags = RFLAG_ALIEN;
  make_base(0, 2, -128); make_base(1, 3, 4);
  make_base(2, 2, 1); make_base(3, 2, 2);
  large = generators(2, 0);
  if (large != 1 || facility_calls != 3) return 6;

  // Null out pointer remains optional; non-alien return still zeroes a
  // supplied output without inspecting bases.
  reset(2); Players[2].rule_flags = 0;
  make_base(0, 2, 3); small = 9;
  large = generators(2, &small);
  if (large != 0 || small != 0 || facility_calls != 0) return 7;
  return 0;
}
'''


def _generators_body() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    signature = "int __cdecl generators(int faction_id, int *count_out) {"
    start = masked.index(signature)
    opening = masked.index("{", start)
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated generators definition")


def _compile_run(body: str, compiler: str, expected: int) -> None:
    with tempfile.TemporaryDirectory(prefix="game-generators-live-count-") as directory:
        cpp = Path(directory) / "generators.cpp"
        exe = Path(directory) / "generators"
        cpp.write_text(CPP_PREFIX + "\n" + body + "\n" + CPP_MAIN, encoding="utf-8")
        subprocess.run([compiler, "-std=c++11", "-O0", "-Wall", "-Wextra", "-Werror",
                        str(cpp), "-o", str(exe)], check=True, timeout=60)
        result = subprocess.run([str(exe)], check=False, timeout=30)
        assert result.returncode == expected, (
            f"{compiler}: expected exit {expected}, got {result.returncode}"
        )


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_generators_actual_body_uses_live_bound_and_rejects_snapshot_mutant(
        compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")

    row, = [row for row in read_file(SOURCE) if row.address == SPAN.start]
    assert row.image_spans == ((SPAN.start, SPAN.end),)
    assert row.name == "?generators@@YAHHPAH@Z"

    actual = _generators_body()
    live_test = "} while (index < GameDataStateStorage.fields_.base_current_count_);"
    snapshot_test = "} while (index < total);"
    assert actual.count(live_test) == 1
    assert "if (total > 0)" in actual
    _compile_run(actual, compiler, expected=0)

    snapshot_mutant = actual.replace(live_test, snapshot_test, 1)
    assert snapshot_mutant != actual
    _compile_run(snapshot_mutant, compiler, expected=1)


def test_generators_pinned_span_proves_initial_guard_and_live_signed_reload() -> None:
    data = PINNED_IMAGE.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PINNED_SHA256
    image = PeImage(data, PINNED_SHA256)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(SPAN), SPAN.start))
    assert instructions and instructions[0].address == SPAN.start
    assert sum(item.size for item in instructions) == SPAN.end - SPAN.start
    native = {item.address: (item.mnemonic, item.op_str) for item in instructions}
    assert native[0x005247E3] == ("mov", "eax, dword ptr [0x9a64cc]")
    assert native[0x005247F0] == ("jle", "0x524858")
    assert native[0x0052482E] == ("movsx", "eax, byte ptr [esi + 0x97d046]")
    assert native[0x00524835] == ("cmp", "eax, dword ptr [0x949864]")
    assert native[0x00524848] == ("mov", "eax, dword ptr [0x9a64cc]")
    assert native[0x0052484D] == ("inc", "edi")
    assert native[0x0052484E] == ("add", "esi, 0x134")
    assert native[0x00524854] == ("cmp", "edi, eax")
    assert native[0x00524856] == ("jl", "0x5247f4")
