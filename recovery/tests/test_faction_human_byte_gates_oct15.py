"""Actual-source controls for the byte-wide faction human-mask gates."""
from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/faction.cpp"


def body(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError(f"unterminated body: {signature}")


STEAL_SIGNATURE = "int __cdecl steal_tech(int faction_id, int faction_id_with,"
DEMAND_SIGNATURE = "void __cdecl demands_withdrawal(int faction_id, int faction_id_with)"
COMPILERS = [p for p in (shutil.which("g++"), shutil.which("clang++")) if p]


HARNESS = r'''#include <cassert>
#include <cstdint>
#include <cstddef>
#define __cdecl

struct LockBytes { uint8_t human_mask_, alive_mask_, unassigned_[2]; };
union LockMask { uint32_t enable_mask_; LockBytes bytes_; };
struct Fields { LockMask lock_masks_; int sunspot_duration_; };
struct GameDataState { Fields fields_; };
GameDataState GameDataStateStorage={};
struct ConsoleState { int faction_id_; };
ConsoleState ConsoleGlobal={};
int IsMultiplayerNet=0;
int PopupDialogFactionID=0, GenderDefault=0, PluralityDefault=0;
char StringTemp[256]={};
struct Player { int noun_gender, is_noun_plural; char *noun_faction; };
Player Players[32]={};

struct MessageQueue {
  int calls=0;
  void pop(const char *, int, int, const char *) { ++calls; }
} NetMsgPrimary;

int topic_pick_result=0, topic_pick_calls=0;
int achieved_calls=0, achieved_mask=0;
bool mutate_after_achieved=false;
int reset_calls=0, reset_faction=-1;
int commlink_result=0, commlink_calls=0;
bool mutate_during_commlink=false;
int commlink_mask=0;
int demand_result=0, demand_calls=0;

int tech_pick(int, int, int, char *) {
  ++topic_pick_calls;
  return topic_pick_result;
}
void message_data(int, int, int, int, int, int) {}
void parse_says(int, char *, int, int) {}
void say_tech(char *, int, int) {}
void tech_achieved(int, int, int, int) {
  ++achieved_calls;
  if (mutate_after_achieved)
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ =
        static_cast<uint32_t>(achieved_mask);
}
void bases_reset(int, int faction, int) {
  ++reset_calls;
  reset_faction=faction;
}
int commlink_attempter(int, int) {
  ++commlink_calls;
  if (mutate_during_commlink)
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ =
        static_cast<uint32_t>(commlink_mask);
  return commlink_result;
}
int demand_withdrawal(int, int) { ++demand_calls; return demand_result; }

STEAL_BODY
DEMAND_BODY

int main() {
  // Valid human-player bits still take the original early-return paths.
  IsMultiplayerNet=1;
  ConsoleGlobal.faction_id_=0;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=(1u << 2) | (1u << 26);
  topic_pick_calls=0;
  assert(steal_tech(2,1,0)==0 && topic_pick_calls==0);

  GameDataStateStorage.fields_.lock_masks_.enable_mask_=(1u << 2) | (1u << 26);
  commlink_result=0; commlink_calls=0; demand_calls=0;
  mutate_during_commlink=false;
  GameDataStateStorage.fields_.sunspot_duration_=0;
  demands_withdrawal(0,2);
  assert(commlink_calls==1 && demand_calls==0);

  // The mask must be reread after tech_achieved: its callback turns faction
  // 2 human, so the post-callback base reset is suppressed.
  IsMultiplayerNet=0;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
  topic_pick_result=7; topic_pick_calls=0;
  achieved_calls=0; reset_calls=0;
  mutate_after_achieved=true; achieved_mask=(1 << 2) | (1u << 26);
  assert(steal_tech(2,1,0)==0);
  assert(topic_pick_calls==1 && achieved_calls==1 && reset_calls==0);

  // Independently exercise the post-callback gate: faction 24 is never used
  // as an array index on this path. The callback sets only the corresponding
  // high dword bit, which is outside the native human-mask byte.
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
  topic_pick_result=7; topic_pick_calls=0;
  achieved_calls=0; reset_calls=0;
  mutate_after_achieved=true; achieved_mask=(1u << 24);
  assert(steal_tech(24,1,0)==0);
  assert(topic_pick_calls==1 && achieved_calls==1 && reset_calls==1 &&
         reset_faction==24);

  // The gate follows the live byte after the commlink callback. Since it
  // publishes faction 2 as human, no withdrawal demand may be issued.
  IsMultiplayerNet=1;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
  mutate_during_commlink=true; commlink_mask=(1 << 2) | (1u << 26);
  commlink_calls=0; demand_calls=0;
  demands_withdrawal(0,2);
  assert(commlink_calls==1 && demand_calls==0);

  // Gate-only high-byte controls intentionally avoid all faction-array
  // indexing. The native byte test ignores bit 24 even when it is set in the
  // adjacent dword; the prior dword operand would incorrectly block/skip.
  IsMultiplayerNet=1; ConsoleGlobal.faction_id_=0;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=1u << 24;
  topic_pick_result=-1; topic_pick_calls=0;
  mutate_after_achieved=false; achieved_calls=0; reset_calls=0;
  assert(steal_tech(24,1,0)==1 && topic_pick_calls==1);

  IsMultiplayerNet=1;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_=0;
  mutate_during_commlink=true; commlink_mask=(1u << 24);
  commlink_calls=0; demand_calls=0;
  demands_withdrawal(0,24);
  assert(commlink_calls==1 && demand_calls==1);
  return 0;
}
'''


def source_bodies(mutant=None):
    text = SOURCE.read_text()
    steal = body(text, STEAL_SIGNATURE)
    demand = body(text, DEMAND_SIGNATURE)
    assert steal.count("bytes_.human_mask_") == 2
    assert demand.count("bytes_.human_mask_") == 1
    if mutant == "steal_early_dword":
        steal = steal.replace("bytes_.human_mask_", "enable_mask_", 1)
    elif mutant == "steal_post_dword":
        first = steal.index("bytes_.human_mask_")
        second = steal.index("bytes_.human_mask_", first + 1)
        steal = steal[:second] + steal[second:].replace(
            "bytes_.human_mask_", "enable_mask_", 1)
    elif mutant == "demand_dword":
        demand = demand.replace("bytes_.human_mask_", "enable_mask_")
    return steal, demand


def program(mutant=None):
    steal, demand = source_bodies(mutant)
    return HARNESS.replace("STEAL_BODY", steal).replace("DEMAND_BODY", demand)


def compile_run(tmp_path, compiler, text):
    cpp = tmp_path / "faction-human-byte-gates.cpp"
    exe = tmp_path / "faction-human-byte-gates"
    cpp.write_text(text)
    build = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-unused-parameter", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    if build.returncode:
        return build, None
    return build, subprocess.run([str(exe)], capture_output=True,
                                 text=True, timeout=10)


def test_only_the_three_native_human_byte_gates_use_the_byte_view():
    steal, demand = source_bodies()
    assert steal.count("bytes_.human_mask_") == 2
    assert demand.count("bytes_.human_mask_") == 1
    assert "enable_mask_" not in steal
    assert "enable_mask_" not in demand
    # Callback order is load-bearing: one gate follows tech_achieved, the
    # other follows commlink_attempter. Keep these as direct live reads.
    assert steal.index("tech_achieved(") < steal.index("bytes_.human_mask_", steal.index("tech_achieved("))
    assert demand.index("commlink_attempter(") < demand.index("bytes_.human_mask_")


@pytest.mark.parametrize("compiler", COMPILERS)
def test_actual_source_byte_gates_and_callback_rereads(compiler, tmp_path):
    build, run = compile_run(tmp_path, compiler, program())
    assert build.returncode == 0, build.stderr
    assert run.returncode == 0, run.stderr


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("mutant", [
    "steal_early_dword", "steal_post_dword", "demand_dword",
])
def test_old_dword_operand_mutants_compile_then_fail(compiler, mutant, tmp_path):
    build, run = compile_run(tmp_path, compiler, program(mutant))
    assert build.returncode == 0, build.stderr
    assert run.returncode != 0, "high-byte mask mutant escaped the actual-body controls"
