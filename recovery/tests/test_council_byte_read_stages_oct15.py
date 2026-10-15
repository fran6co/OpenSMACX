"""Pinned access-width and actual-source ownership checks for council masks."""

from pathlib import Path
import hashlib
import os
import shutil
import subprocess
import tempfile
import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
PIN_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
CAN_CALL_SOURCE = (ROOT / "src/council.cpp").read_text()


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for pos in range(opening, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated function: {signature}")


def test_pinned_council_mask_instruction_widths_and_offsets():
    if not PIN.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    image_bytes = PIN.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == PIN_SHA256
    image = PeImage(image_bytes, PIN_SHA256)
    byte_sites = (
        0x52C9D9, 0x52CE49, 0x52CE7F, 0x52CF26, 0x52CF32,
        0x52CFF2, 0x52D07E, 0x52D0D7, 0x52D1C5, 0x52D249,
        0x52D368, 0x52D406, 0x52D444, 0x52D57D, 0x52D5C9,
        0x52D6D6, 0x52D6EA, 0x52D7DE,
        0x52C6D5, 0x52C72C, 0x52C7A0, 0x52C7DE, 0x52C850,
        0x52AEDD, 0x52AF25, 0x52B3DF, 0x52B4B8, 0x52B4FD,
    )
    for address in byte_sites:
        raw = image.read(Span(address, address + 6))
        if raw[0] == 0xA0:
            assert raw[1] in (0xE8, 0xE9), hex(address)
            assert raw[2:5] == bytes.fromhex("64 9A 00"), hex(address)
        else:
            assert raw[0] in (0x84, 0x8A), hex(address)
            assert raw[2] in (0xE8, 0xE9), hex(address)
            assert raw[3:6] == bytes.fromhex("64 9A 00"), hex(address)

    dword_sites = (0x52CB19, 0x52C7FA, 0x52B250, 0x52B378, 0x52B437)
    for address in dword_sites:
        raw = image.read(Span(address, address + 6))
        if raw[0] == 0xA1:
            assert raw[:5] == bytes.fromhex("A1 E8 64 9A 00"), hex(address)
        else:
            assert raw == bytes.fromhex("8B 15 E8 64 9A 00"), hex(address)


def test_council_source_uses_named_byte_views_and_keeps_dword_stages():
    source = (ROOT / "src/council.cpp").read_text()
    call = body(source, "void __cdecl call_council(int faction_id)")
    admission = body(source, "int __cdecl can_call_council(int faction_id, int announce)")
    score = body(source, "int __cdecl wants_prop(int faction, int proposal, int option)")

    human = "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_"
    alive = "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_"
    assert call.count(human) == 14
    assert call.count(alive) == 5
    assert "human_mask = GameDataStateStorage.fields_.lock_masks_.enable_mask_" not in call
    assert "human_mask = GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_;" in call
    assert "GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1 << faction_id)" in call
    assert admission.count(human) == 4
    assert admission.count(alive) == 1
    assert "GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFF" in admission
    assert score.count(human) == 1
    assert score.count(alive) == 4
    assert score.count("GameDataStateStorage.fields_.lock_masks_.enable_mask_ & 0xFFu") == 3
    assert "enable_mask_ >> 8" not in score


CAN_CALL_HARNESS = r'''#include <cassert>
#include <stdint.h>
#include <cstring>
#define __cdecl
struct BaseSecretProject { unsigned char bytes_[0x100]; int empath_guild() const { return -1; } };
#include "@@GAME_DATA_HEADER@@"
GameDataStateWindow GameDataStateStorage;
static const int MaxPlayerNum = 8;
struct Player { uint32_t rule_flags; unsigned char rest_[0x59C - 4]; };
struct PlayerData { uint32_t diplo_treaties[8]; };
struct Base { int faction_id_current; };
struct ConsoleType { int faction_id_; };
struct NetMsg { int pops; NetMsg() : pops(0) {}
  void pop(const char *, int, int, const char *) { ++pops; }
};
static Player Players[MaxPlayerNum + 1] = {};
static PlayerData PlayersData[MaxPlayerNum + 1] = {};
static Base Bases[512] = {};
static ConsoleType ConsoleGlobal = {};
static NetMsg NetMsgPrimary;
static int IsMultiplayerPBEM = 0;
static int IsMultiplayerNet = 0;
static int CouncilPbemGate = 0;
static const char CouncilSunspotLabel[] = "SUNSPOTSNO2";
static const char CouncilSunspotPcx[] = "solar_sm.pcx";
static const char CouncilPbemLabel[] = "COUNCILHOT";
static const char CouncilBlockedLabel[] = "COUNCILNEEDALL";
static const char CouncilBlockedPcx[] = "council_sm.pcx";
'''


def can_call_body(source: str) -> str:
    return body(source, "int __cdecl can_call_council(int faction_id, int announce)")


def compile_can_call(body_source: str) -> subprocess.CompletedProcess:
    harness = CAN_CALL_HARNESS.replace(
        "@@GAME_DATA_HEADER@@", str(ROOT / "src/game_data_state_window.h"))
    program = harness + "\n" + body_source + r'''
int main() {
    GameDataStateStorage.fields_.sunspot_duration_ = 1;
    ConsoleGlobal.faction_id_ = 8;
    GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x00000100u;
    assert(can_call_council(8, 1) == 0);
    assert(NetMsgPrimary.pops == 0);

    std::memset(&GameDataStateStorage, 0, sizeof GameDataStateStorage);
    std::memset(Players, 0, sizeof Players);
    std::memset(PlayersData, 0, sizeof PlayersData);
    NetMsgPrimary.pops = 0;
    ConsoleGlobal.faction_id_ = 0;
    GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ = 1u << 2;
    PlayersData[1].diplo_treaties[2] = 0;
    assert(can_call_council(1, 0) == 0);
    assert(NetMsgPrimary.pops == 0);
}
'''
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    assert compilers, "GCC or Clang is required for host source controls"
    result = None
    with tempfile.TemporaryDirectory(prefix="council-byte-mask-") as temp:
        cpp = Path(temp) / "control.cpp"
        cpp.write_text(program)
        for compiler in compilers:
            exe = Path(temp) / (Path(compiler).name.replace("+", "p"))
            build = subprocess.run(
                [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                 str(cpp), "-o", str(exe)],
                capture_output=True, text=True)
            assert build.returncode == 0, build.stdout + build.stderr
            result = subprocess.run([str(exe)], capture_output=True, text=True)
            assert result.returncode == 0, result.stdout + result.stderr
    assert result is not None
    return result


def test_can_call_uses_actual_human_and_alive_byte_fields():
    source = CAN_CALL_SOURCE
    native_body = can_call_body(source)
    assert "bytes_.human_mask_" in native_body
    assert "bytes_.alive_mask_" in native_body
    assert "enable_mask_ & 0xFF" in native_body
    compile_can_call(native_body)

    wrong_human = native_body.replace(
        "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_",
        "GameDataStateStorage.fields_.lock_masks_.enable_mask_", 1)
    with tempfile.TemporaryDirectory(prefix="council-human-mutant-") as temp:
        cpp = Path(temp) / "control.cpp"
        harness = CAN_CALL_HARNESS.replace(
            "@@GAME_DATA_HEADER@@", str(ROOT / "src/game_data_state_window.h"))
        cpp.write_text(harness + "\n" + wrong_human + r'''
int main() {
  GameDataStateStorage.fields_.sunspot_duration_ = 1;
  ConsoleGlobal.faction_id_ = 8;
  GameDataStateStorage.fields_.lock_masks_.enable_mask_ = 0x100u;
  can_call_council(8, 1);
  assert(NetMsgPrimary.pops == 0);
}
''')
        compiler = shutil.which("g++")
        assert compiler
        exe = Path(temp) / "mutant"
        subprocess.run([compiler, "-std=c++11", str(cpp), "-o", str(exe)], check=True)
        failed = subprocess.run([str(exe)], capture_output=True)
        assert failed.returncode != 0

    wrong_alive = native_body.replace(
        "GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_",
        "GameDataStateStorage.fields_.lock_masks_.bytes_.human_mask_", 1)
    with tempfile.TemporaryDirectory(prefix="council-alive-mutant-") as temp:
        cpp = Path(temp) / "control.cpp"
        harness = CAN_CALL_HARNESS.replace(
            "@@GAME_DATA_HEADER@@", str(ROOT / "src/game_data_state_window.h"))
        cpp.write_text(harness + "\n" + wrong_alive + r'''
int main() {
  GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_ = 1u << 2;
  PlayersData[1].diplo_treaties[2] = 0;
  assert(can_call_council(1, 0) == 0);
}
''')
        compiler = shutil.which("g++")
        assert compiler
        exe = Path(temp) / "mutant"
        subprocess.run([compiler, "-std=c++11", str(cpp), "-o", str(exe)], check=True)
        failed = subprocess.run([str(exe)], capture_output=True)
        assert failed.returncode != 0

