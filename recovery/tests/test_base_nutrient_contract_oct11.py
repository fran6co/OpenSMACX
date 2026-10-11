"""Execute the live Base nutrient body against its pinned x86 dword contract."""
from __future__ import annotations

import hashlib
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SOURCE = ROOT / "src/base.cpp"
SPAN = Span(0x004E9B70, 0x004E9CAD)

CPP_PREFIX = r'''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <type_traits>
#define __cdecl
struct SocialEffect {
  enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                     GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
  int values_[11];
};
struct PlayerData { SocialEffect soc_effect_pending; uint32_t nutrient_surplus_total; };
static_assert(std::is_unsigned<decltype(PlayerData::nutrient_surplus_total)>::value,
              "native faction surplus total is an unsigned dword");
struct Base {
  uint8_t faction_id_current; int8_t population_size; uint32_t state;
  int nutrients_accumulated, nutrient_intake_2, nutrient_consumption, nutrient_surplus;
  uint8_t facilities_built[12];
};
struct RuleData { uint32_t nutrient_req_citizen; };
static const int FAC_CHILDREN_CRECHE=2, RSC_NUTRIENTS=0;
static const uint32_t BSTATE_GOLDEN_AGE_ACTIVE=4;
Base bases[8], *BaseCurrent=&bases[0];
PlayerData PlayersData[8];
uint32_t BaseCurrentConvoyTo[4], BaseCurrentConvoyFrom[4];
RuleData Rules;
int BaseCurrentGrowthRate, BaseIDCurrentSelected, BaseUpkeepStage;
int has_fac_built_call(int facility, int base_id) {
  if (facility != FAC_CHILDREN_CRECHE || base_id != BaseIDCurrentSelected) return 0;
  return (bases[base_id].facilities_built[facility >> 3]
      & (1u << (facility & 7))) != 0;
}
int range(int value, int low, int high) {
  return value < low ? low : (value > high ? high : value);
}
int saturating_sub(int left, int right) {
  const int64_t difference=static_cast<int64_t>(left)-right;
  if (difference<INT_MIN) return INT_MIN;
  if (difference>INT_MAX) return INT_MAX;
  return static_cast<int>(difference);
}
'''

CPP_MAIN = r'''
void reset() {
  memset(bases,0,sizeof(bases)); memset(PlayersData,0,sizeof(PlayersData));
  memset(BaseCurrentConvoyTo,0,sizeof(BaseCurrentConvoyTo));
  memset(BaseCurrentConvoyFrom,0,sizeof(BaseCurrentConvoyFrom));
  memset(&Rules,0,sizeof(Rules));
  BaseCurrent=&bases[0]; BaseCurrent->faction_id_current=1;
  BaseCurrentGrowthRate=0; BaseIDCurrentSelected=0; BaseUpkeepStage=0;
}
int main() {
  // Bounded normal values exercise the ordinary growth, intake, consumption,
  // signed-surplus, accumulated-nutrient, and upkeep-total paths.
  reset(); BaseCurrent=&bases[1]; BaseCurrent->faction_id_current=1;
  BaseIDCurrentSelected=2;
  PlayersData[1].soc_effect_pending.values_[SocialEffect::GROWTH]=3;
  bases[2].facilities_built[FAC_CHILDREN_CRECHE >> 3]
      = 1u << (FAC_CHILDREN_CRECHE & 7);
  BaseCurrent->state=BSTATE_GOLDEN_AGE_ACTIVE;
  BaseCurrent->nutrient_intake_2=30; BaseCurrentConvoyTo[0]=5;
  BaseCurrentConvoyFrom[0]=2; BaseCurrent->population_size=3;
  Rules.nutrient_req_citizen=2; BaseCurrent->nutrients_accumulated=-3;
  BaseUpkeepStage=1; base_nutrient();
  if (BaseCurrentGrowthRate!=7 || BaseCurrent->nutrient_intake_2!=35
      || BaseCurrent->nutrient_consumption!=8 || BaseCurrent->nutrient_surplus!=27
      || BaseCurrent->nutrients_accumulated!=0
      || PlayersData[1].nutrient_surplus_total!=27) return 1;

  // Negative surplus only changes an exactly-zero accumulation to -1.
  reset(); BaseCurrentConvoyFrom[0]=1; BaseCurrent->population_size=1;
  Rules.nutrient_req_citizen=1; BaseCurrent->nutrients_accumulated=0;
  base_nutrient();
  if (BaseCurrent->nutrient_consumption!=2 || BaseCurrent->nutrient_surplus!=-2
      || BaseCurrent->nutrients_accumulated!=-1) return 2;

  // Native ADD to growth rate wraps twice rather than invoking signed UB.
  reset(); PlayersData[1].soc_effect_pending.values_[SocialEffect::GROWTH]=INT_MAX;
  BaseCurrent->facilities_built[FAC_CHILDREN_CRECHE >> 3]
      = 1u << (FAC_CHILDREN_CRECHE & 7);
  BaseCurrent->state=BSTATE_GOLDEN_AGE_ACTIVE;
  base_nutrient();
  if (BaseCurrentGrowthRate!=INT_MIN+3) return 3;

  // Intake ADD wraps, and that signed wrapped value selects the negative path.
  reset(); BaseCurrent->nutrient_intake_2=INT_MAX;
  BaseCurrentConvoyTo[0]=1; BaseCurrent->nutrients_accumulated=0;
  base_nutrient();
  if (BaseCurrent->nutrient_intake_2!=INT_MIN
      || BaseCurrent->nutrient_surplus!=INT_MIN
      || BaseCurrent->nutrients_accumulated!=-1) return 4;

  // Signed byte population is multiplied by the uint32 rule word modulo 2^32;
  // adding the uint32 convoy-from word likewise retains the low dword.
  reset(); BaseCurrent->population_size=-128;
  Rules.nutrient_req_citizen=UINT32_MAX; BaseCurrentConvoyFrom[0]=INT_MAX;
  BaseCurrent->nutrients_accumulated=-1; base_nutrient();
  if (BaseCurrent->nutrient_consumption!=INT_MIN+127
      || BaseCurrent->nutrient_surplus!=INT_MAX-126
      || BaseCurrent->nutrients_accumulated!=0) return 5;

  // SUB wraps INT_MIN - 1 to INT_MAX. Native signed JGE takes the nonnegative
  // branch and clears a negative accumulated value; no source safety guard.
  reset(); BaseCurrent->nutrient_intake_2=INT_MIN;
  BaseCurrentConvoyFrom[0]=1; BaseCurrent->nutrients_accumulated=-1;
  BaseUpkeepStage=1; base_nutrient();
  if (BaseCurrent->nutrient_consumption!=1
      || BaseCurrent->nutrient_surplus!=INT_MAX
      || BaseCurrent->nutrients_accumulated!=0
      || PlayersData[1].nutrient_surplus_total!=99) return 6;

  // Native ADD to the uint32 faction total wraps on overflow as well.
  reset(); BaseCurrent->nutrient_intake_2=1;
  PlayersData[1].nutrient_surplus_total=UINT32_MAX;
  BaseUpkeepStage=1; base_nutrient();
  if (BaseCurrent->nutrient_surplus!=1
      || PlayersData[1].nutrient_surplus_total!=0) return 7;
  return 0;
}
'''


def _helpers(source: str) -> str:
    start = source.index("int base_yield_signed_bits(")
    end = source.index("int base_yield_shift(", start)
    return source[start:end]


def _body(source: str) -> str:
    start = source.index("void __cdecl base_nutrient() {")
    end = source.index("\n/*\nPurpose: Calculate minerals", start)
    return source[start:end]


def _compile_run(body: str, compiler: str, optimization: str, expected: int) -> None:
    source = SOURCE.read_text()
    with tempfile.TemporaryDirectory(prefix="base-nutrient-contract-") as td:
        cpp = Path(td) / "base_nutrient.cpp"
        exe = Path(td) / "base_nutrient"
        cpp.write_text(CPP_PREFIX + "\n" + _helpers(source) + "\n" + body + "\n" + CPP_MAIN)
        subprocess.run([
            compiler, "-std=c++11", optimization, "-Wall", "-Wextra", "-Werror",
            str(cpp), "-o", str(exe)
        ], check=True)
        result = subprocess.run([str(exe)], check=False)
        assert result.returncode == expected, (
            f"{compiler} {optimization}: expected {expected}, got {result.returncode}"
        )


def test_base_nutrient_executes_complete_source_with_native_wrap_controls() -> None:
    source = SOURCE.read_text()
    body = _body(source)
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    assert compilers, "neither g++ nor clang++ is available"
    for compiler in compilers:
        for optimization in ("-O0", "-O2"):
            _compile_run(body, compiler, optimization, expected=0)


def test_base_nutrient_fixture_uses_source_facility_and_effect_indices() -> None:
    base_header = (ROOT / "src/base.h").read_text()
    faction_header = (ROOT / "src/faction.h").read_text()
    assert "FAC_CHILDREN_CRECHE = 2," in base_header
    assert "ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,\n        GROWTH," in faction_header
    assert "uint32_t nutrient_surplus_total;" in faction_header


def test_base_nutrient_wrap_control_rejects_saturating_sub_mutant() -> None:
    source = SOURCE.read_text()
    body = _body(source)
    correct = "base_yield_sub(\n        BaseCurrent->nutrient_intake_2, BaseCurrent->nutrient_consumption)"
    assert correct in body
    mutant = body.replace(correct,
        "saturating_sub(\n        BaseCurrent->nutrient_intake_2, BaseCurrent->nutrient_consumption)", 1)
    for compiler in ("g++", "clang++"):
        if shutil.which(compiler):
            for optimization in ("-O0", "-O2"):
                _compile_run(mutant, compiler, optimization, expected=6)


def test_base_nutrient_pinned_native_span_and_branch_anchors() -> None:
    image_value = os.environ.get("OPENSMACX_IMAGE")
    assert image_value, "OPENSMACX_IMAGE must explicitly identify the pinned original"
    image_path = Path(image_value)
    assert image_path.is_absolute() and image_path.is_file()
    image_data = image_path.read_bytes()
    assert hashlib.sha256(image_data).hexdigest() == PIN
    image = PeImage(image_data, PIN)
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(SPAN), SPAN.start))
    assert instructions and instructions[0].address == SPAN.start
    assert sum(instruction.size for instruction in instructions) == SPAN.end - SPAN.start
    native = {instruction.address: (instruction.mnemonic, instruction.op_str)
              for instruction in instructions}
    assert native[0x004E9BDB] == ("add", "dword ptr [0x90e918], 2")
    assert native[0x004E9BED] == ("add", "dword ptr [0x90e918], 2")
    assert native[0x004E9BF4] == ("mov", "ecx, dword ptr [0x90e93c]")
    assert native[0x004E9C00] == ("add", "edx, ecx")
    assert native[0x004E9C13] == ("movsx", "edx, byte ptr [eax + 6]")
    assert native[0x004E9C17] == ("imul", "edx, dword ptr [0x94973c]")
    assert native[0x004E9C1E] == ("add", "edx, esi")
    assert native[0x004E9C37] == ("sub", "ecx, edx")
    assert native[0x004E9C4A] == ("test", "ecx, ecx")
    assert native[0x004E9C4F] == ("jl", "0x4e9c5e")
    assert native[0x004E9C53] == ("jge", "0x4e9c6e")
    assert native[0x004E9C55] == ("mov", "dword ptr [eax + 0x3c], 0")
    assert native[0x004E9C62] == ("mov", "dword ptr [eax + 0x3c], 0xffffffff")
    assert native[0x004E9C77] == ("mov", "eax, dword ptr [eax + 0xd0]")
    assert native[0x004E9C81] == ("cmp", "eax, 0x63")
    assert native[0x004E9C84] == ("jle", "0x4e9ca1")
    assert native[0x004E9C91] == ("add", "ecx, eax")
    assert native[0x004E9C93] == ("mov", "dword ptr [edi + 0x96d1e0], ecx")
    assert native[0x004E9CA1] == ("add", "dword ptr [edi + 0x96d1e0], eax")

    source = SOURCE.read_text()
    assert "// ORIGINAL: 0x004E9B70 ?base_nutrient@@YAXXZ 0x004E9B70-0x004E9CAD" in source
