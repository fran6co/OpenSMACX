"""Typed player reads: pinned evidence, bounded source delta and actual bodies.

Host controls establish values/order in their declared domain, not native ABI,
threaded observations, original allocation completeness or whole-game acceptance.
"""
from pathlib import Path
import hashlib
import os
import re
import shutil
import subprocess

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
BASE = "11075a39a9833b37a559061163eaea0b0f83656c"
PIN_SHA = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EDITS = {
    "src/council.cpp": (
        ("        const unsigned char *rec = reinterpret_cast<const unsigned char *>(\n"
         "            &Players[1].rule_flags);\n"
         "        int i = 1;\n"
         "        for (;;) {\n"
         "            if ((*rec & 0x80) == 0) {",
         "        for (int i = 1; i < 8; ++i) {\n"
         "            if ((Players[i].rule_flags & 0x80) == 0) {"),
        ("            rec += 0x59C;\n"
         "            ++i;\n"
         "            if (rec >= reinterpret_cast<const unsigned char *>(\n"
         "                           &Players[8].rule_flags)) {\n"
         "                break;\n"
         "            }\n", ""),
    ),
    "src/netwin.cpp": (
        ("    unsigned char color = reinterpret_cast<const unsigned char *>(\n"
         "        &NetDaemonNet.allocation_node().max_players_)[0];",
         "    unsigned char color = static_cast<unsigned char>(\n"
         "        NetDaemonNet.allocation_node().max_players_);"),
        ("        row->tail_[0] = reinterpret_cast<const uint8_t *>(\n"
         "            &player->is_leader_female)[0];",
         "        row->tail_[0] = static_cast<uint8_t>(player->is_leader_female);"),
    ),
}


def baseline(unit):
    return subprocess.run(["git", "show", BASE + ":" + unit], cwd=ROOT,
                          check=True, capture_output=True).stdout.decode()


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start + len(signature))
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError("unterminated source function")


@pytest.mark.parametrize("unit", tuple(EDITS))
def test_entire_owner_delta_preserves_annotations_history_and_other_bodies(unit):
    expected = baseline(unit)
    for old, new in EDITS[unit]:
        assert expected.count(old) == 1
        expected = expected.replace(old, new, 1)
    # Complete source equality also protects every marker, binding, historical
    # claim, seed finding, declaration, non-target body and static initializer.
    current = (ROOT / unit).read_text()
    if unit == "src/council.cpp":
        from test_general_owner_split_oct17 import inverse_split, LIVE_PATHS
        current = inverse_split({key: path.read_bytes() for key, path in LIVE_PATHS.items()})["council"].decode()
    assert current == expected


def test_pinned_original_player_width_stride_and_range_evidence():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN_SHA
    image = PeImage(raw, PIN_SHA)
    for start, end, sha in (
        (0x52C670, 0x52C87D, "a85b752b683b205c2c130d77fd8699ab059800142a080fc68c32f89cd0239cf1"),
        (0x481E90, 0x481FC9, "fb8f9a93698eccda0331d0f855cdda0df14ba54837d04cd95aa4787d428c2775"),
    ):
        assert hashlib.sha256(image.read(Span(start, end))).hexdigest() == sha
    # Player table base946A50, native stride59C, rule_flags+508:
    # first player9474F4, non-loaded sentinel949C38. CMP begins at52C821,
    # after the INC ESI at52C820; do not mistake that INC for the CMP.
    for address, encoded in (
        (0x52C781, "BF F4 74 94 00"),
        (0x52C78E, "F6 07 80"),
        (0x52C81A, "81 C7 9C 05 00 00 46 81 FF 38 9C 94 00"),
        (0x481E90, "A0 B0 D4 93 00"),
        (0x481F9E, "8A 97 7C FC FF FF"),
    ):
        expected = bytes.fromhex(encoded)
        assert image.read(Span(address, address + len(expected))) == expected
    assert 0x946A50 + 0x59C + 0x508 == 0x9474F4
    assert 0x946A50 + 8 * 0x59C + 0x508 == 0x949C38


HARNESS = r'''
#include <cassert>
#include <cstring>
#include <stddef.h>
#include <stdint.h>
#define __cdecl
typedef int BOOL;
@@PLAYER@@
typedef char check_player_stride[(sizeof(Player)==0x59C)?1:-1];
typedef char check_rule_offset[(offsetof(Player,rule_flags)==0x508)?1:-1];
typedef char check_gender_offset[(offsetof(Player,is_leader_female)==0)?1:-1];
static int ProjectBase = -1;
struct BaseSecretProject { unsigned char bytes_[0x100]; int empath_guild() const { return ProjectBase; } };
#include "@@GAME_HEADER@@"
GameDataStateWindow GameDataStateStorage;
// Extra fixture slot permits comparison with the old out-of-range member
// sentinel. It is not a new production allocation or a safety claim.
static Player Players[9];
struct PlayerData { unsigned int diplo_treaties[8]; };
static PlayerData PlayersData[9];
struct Base { int faction_id_current; };
static Base Bases[1];
struct ConsoleType { int faction_id_; };
static ConsoleType ConsoleGlobal;
struct NetMsg {
  int pops; const char *label; const char *pcx;
  void pop(const char *text, int flags, int zero, const char *picture) {
    assert(flags == -5000 && zero == 0); ++pops; label=text; pcx=picture;
  }
};
static NetMsg NetMsgPrimary;
static int IsMultiplayerPBEM, IsMultiplayerNet, CouncilPbemGate;
static const char CouncilSunspotLabel[]="SUNSPOTSNO2";
static const char CouncilSunspotPcx[]="solar_sm.pcx";
static const char CouncilPbemLabel[]="COUNCILHOT";
static const char CouncilBlockedLabel[]="COUNCILNEEDALL";
static const char CouncilBlockedPcx[]="council_sm.pcx";
@@BASELINE@@
@@CANDIDATE@@
static void reset() {
  std::memset(Players,0,sizeof Players); std::memset(PlayersData,0,sizeof PlayersData);
  std::memset(&GameDataStateStorage,0,sizeof GameDataStateStorage);
  std::memset(&NetMsgPrimary,0,sizeof NetMsgPrimary);
  ConsoleGlobal.faction_id_=1; ProjectBase=-1;
  IsMultiplayerPBEM=IsMultiplayerNet=CouncilPbemGate=0;
}
static void compare(int caller, int announce, int expected, int pops=0,
                    const char *label=0, const char *pcx=0) {
  NetMsgPrimary.pops=0; NetMsgPrimary.label=0; NetMsgPrimary.pcx=0;
  assert(baseline_can_call(caller,announce)==expected);
  assert(NetMsgPrimary.pops==pops && NetMsgPrimary.label==label && NetMsgPrimary.pcx==pcx);
  NetMsgPrimary.pops=0; NetMsgPrimary.label=0; NetMsgPrimary.pcx=0;
  assert(can_call_council(caller,announce)==expected);
  assert(NetMsgPrimary.pops==pops && NetMsgPrimary.label==label && NetMsgPrimary.pcx==pcx);
}
int main() {
  reset();
  // Independent bit-set oracle, not a copy of the candidate loop. All 128
  // alive sets x128 alien sets x7 callers: 114,688 admission scenarios.
  for (unsigned alien=0;alien<128;++alien) {
    for (int i=1;i<8;++i) Players[i].rule_flags=0x40000000 | (((alien>>(i-1))&1)?0x80:0);
    for (unsigned alive=0;alive<128;++alive) {
      GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=static_cast<uint8_t>(alive<<1);
      for (int caller=1;caller<8;++caller) {
        unsigned bit=1u<<caller;
        bool caller_alien=((alien<<1)&bit)!=0;
        bool unmet=(((alive<<1)&~(alien<<1)&~bit)!=0);
        compare(caller,0,caller_alien||unmet?0:1);
      }
    }
  }
  reset(); GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=1;
  compare(1,0,1); // slot0 is never scanned
  reset(); GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=1u<<7;
  compare(1,0,0); // slot7 must not be omitted
  Players[7].rule_flags=0x80; compare(1,0,1);
  Players[7].rule_flags=0x100; compare(1,0,0); // unrelated rule bit
  Players[7].rule_flags=-128; compare(1,0,1); // negative int still has alien bit
  reset(); GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=1u<<2;
  PlayersData[1].diplo_treaties[2]=0x8000000; compare(1,0,1);
  PlayersData[1].diplo_treaties[2]=8; compare(1,0,1);
  PlayersData[1].diplo_treaties[2]=0;
  compare(1,1,0); // no human caller: no announcement
  IsMultiplayerNet=1;
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=6; compare(1,0,1);
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=2; compare(1,0,0);
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=4; compare(1,0,0);
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=2;
  compare(1,1,0,1,CouncilBlockedLabel,CouncilBlockedPcx);
  IsMultiplayerNet=0; GameDataStateStorage.fields_.game_state_=0x80;
  compare(1,1,1); // current human faction bypasses the network gate
  ConsoleGlobal.faction_id_=3; compare(1,1,0);
  reset(); GameDataStateStorage.fields_.sunspot_duration_=1;
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=2;
  compare(1,1,0,1,CouncilSunspotLabel,CouncilSunspotPcx);
  compare(1,0,0);
  reset(); IsMultiplayerPBEM=CouncilPbemGate=1;
  GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_=2;
  compare(1,1,0,1,CouncilPbemLabel,0);
  reset(); ProjectBase=0; Bases[0].faction_id_current=1;
  GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_=4; compare(1,0,1);
  reset(); GameDataStateStorage.fields_.net_turn_flags_=4;
  for (int state=0;state<4;++state) {
    GameDataStateStorage.fields_.game_state_=state*0x40;
    compare(1,0,state==3?1:0);
  }
  return 0;
}
'''


def program(candidate):
    faction = (ROOT / "src/faction.h").read_text()
    player = re.search(r"struct Player \{.*?\n\};", faction, re.S)
    assert player
    signature = "int __cdecl can_call_council(int faction_id, int announce)"
    before = body(baseline("src/council.cpp"), signature).replace("can_call_council(", "baseline_can_call(", 1)
    return (HARNESS.replace("@@PLAYER@@", player.group())
            .replace("@@GAME_HEADER@@", str(ROOT / "src/game_data_state_window.h"))
            .replace("@@BASELINE@@", before).replace("@@CANDIDATE@@", candidate))


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", (None, "omit_last", "omit_first", "wrong_flag", "include_self"))
def test_actual_council_scan_and_damaging_mutants(tmp_path, compiler_name, mutation):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip("host compiler unavailable")
    signature = "int __cdecl can_call_council(int faction_id, int announce)"
    candidate = body((ROOT / "src/council.cpp").read_text(), signature)
    changes = {
        "omit_last": ("i < 8", "i < 7"),
        "omit_first": ("int i = 1", "int i = 2"),
        "wrong_flag": ("Players[i].rule_flags & 0x80", "Players[i].rule_flags & 0x100"),
        "include_self": ("i != faction_id", "true"),
    }
    if mutation:
        old, new = changes[mutation]
        assert candidate.count(old) == 1
        candidate = candidate.replace(old, new, 1)
    cpp, exe = tmp_path / "council.cpp", tmp_path / "council"
    cpp.write_text(program(candidate))
    built = subprocess.run([compiler, "-std=c++98", "-O0", "-Wall", "-Wextra", "-Werror",
                            str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=15)
    assert (result.returncode == 0) is (mutation is None), result.stdout + result.stderr
