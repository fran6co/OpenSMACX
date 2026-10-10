"""Host controls for the shared NetDaemon turn word and popup checkbox dword."""
from __future__ import annotations

import re
import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/IMAGE_PIN").is_file())


def extract(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    matches = list(re.finditer(signature, masked))
    assert len(matches) == 1, f"expected one active function matching {signature}"
    start = matches[0].start()
    depth = 0
    for pos in range(masked.find("{", start), len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated function")


def extract_case(source: str, label: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source,
    )
    start = source.index(label)
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for pos in range(opening, len(masked)):
        if masked[pos] == "{":
            depth += 1
        elif masked[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated case")


SUPPORT = r'''#include <cassert>
#include <cstdint>
#include <cstring>
#define __cdecl
#define NullCallback void*
struct PrimaryState { uint32_t leading_words_[3]{}, flags_{}, trailing_words_[10]{}; };
struct Net { PrimaryState primary_state_{}; };
Net NetDaemonNet;
struct Fields { int net_turn_flags_=0, net_turn_faction_=0, game_more_preferences_=0; struct { struct { uint32_t alive_mask_=0; } bytes_; uint32_t enable_mask_=0; } lock_masks_; };
struct GameState { Fields fields_; };
GameState GameDataStateStorage;
struct Console { int faction_id_=0; void set_ai(int); void on_key_ai_shortcut(); };
Console ConsoleGlobal;
int ConsoleControlTurnActive=0, NextTurnReady=0;
int PopupWriteCheckState=0, ConsoleStrTechRandom=0;
struct PlayerData { int ai_growth=0, ai_tech=0, ai_wealth=0, ai_power=0; int unk_38[8]{}; } PlayersData[8];
int message_calls=0, sync_calls=0, popup_result=0, callback_word=0;
int message_data(int, int, int, int, int, int) { ++message_calls; return 0; }
int x_pop_short(int, int, NullCallback) {
    callback_word=PopupWriteCheckState;
    if (popup_result == 7) PopupWriteCheckState=0x12345608;
    if (popup_result == 8) PopupWriteCheckState=0x123456F2;
    return popup_result;
}
void synch_ai(int) { ++sync_calls; }
void popup_set_write_check_bit(int bit, int value) {
    if (value != 0) PopupWriteCheckState |= 1 << bit;
    else PopupWriteCheckState &= ~(1 << bit);
}
int social_select_calls=0;
void social_select(int) { ++social_select_calls; }
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_shared_turn_storage_and_popup_dword_actual_sources(tmp_path: Path,
                                                             compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    game = (ROOT / "src/game.cpp").read_text(encoding="utf-8")
    console = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    time_controls = extract(console, r"void\s+Console::time_controls\s*\(\)\s*\{")
    bodies = [
        extract(game, r"static\s+int\s+turns_assigned\s*\(\)\s*\{"),
        extract(console, r"inline\s+unsigned\s+int\s*&\s*console_turns_assigned\s*\(\)\s*\{"),
        extract(game, r"int\s+__cdecl\s+next_player_turn\s*\(\)\s*\{"),
        extract(game, r"int\s+__cdecl\s+current_server\s*\(\)\s*\{"),
        extract(console, r"void\s+Console::set_ai\s*\(\s*int\s+faction_id\s*\)\s*\{"),
    ]
    key_route = extract_case(console, "case 'R':")
    assert "0x0093E8C0" not in game[game.find("static int turns_assigned()"):game.find("int __cdecl next_player_turn()")]
    assert "NetDaemonNet.primary_state_.leading_words_[0]" in game
    assert "NetDaemonNet.primary_state_.leading_words_[0]" in console
    assert "console_turns_assigned()" in time_controls
    assert "reinterpret_cast<int *>(0x0093E8C0)" not in console
    assert "reinterpret_cast<unsigned char *>(0x009BC06C)" not in console
    assert "PopupWriteCheckState = static_cast<int>(flags);" in bodies[4]
    assert "const int answer = PopupWriteCheckState;" in bodies[4]
    assert "unk_38" not in bodies[4]
    assert "PlayersData[faction].ai_growth" in key_route
    assert "PlayersData[faction].ai_tech" in key_route
    assert "PlayersData[faction].ai_wealth" in key_route
    assert "PlayersData[faction].ai_power" in key_route
    assert "unk_38" not in key_route
    route_method = "void Console::on_key_ai_shortcut() { switch ('R') {\n" + key_route + "\n default: break; } native_shared_after_key:; }"
    source = SUPPORT + "\n".join(bodies) + "\n" + route_method + r'''
int main() {
  // The game and console accessors are the identical existing owner cell.
  NetDaemonNet.primary_state_.leading_words_[0]=0xFFFFFFFFu;
  assert(console_turns_assigned() == 0xFFFFFFFFu);
  assert(turns_assigned() == -1);
  assert(&console_turns_assigned() == &NetDaemonNet.primary_state_.leading_words_[0]);

  GameDataStateStorage.fields_.net_turn_flags_=0x10;
  GameDataStateStorage.fields_.net_turn_faction_=0;
  ConsoleControlTurnActive=0; ConsoleGlobal.faction_id_=0;
  assert(current_server()==1);
  GameDataStateStorage.fields_.net_turn_flags_=0;
  ConsoleGlobal.faction_id_=1;
  assert(current_server()==-1); // preserve the signed interpretation of 0xFFFFFFFF
  NetDaemonNet.primary_state_.leading_words_[0]=0;
  GameDataStateStorage.fields_.net_turn_flags_=0x10;
  NextTurnReady=0;
  assert(next_player_turn()==0 && NextTurnReady==0);
  NetDaemonNet.primary_state_.leading_words_[0]=1;
  GameDataStateStorage.fields_.net_turn_faction_=7;
  assert(next_player_turn()==1 && NextTurnReady==1);

  // set_ai zero-extends all four checkbox flags into the original dword
  // before the popup callback. The callback can replace the full dword;
  // only its low byte is read back, and cancellation leaves player data alone.
  PlayersData[2].ai_growth=0; PlayersData[2].ai_tech=1;
  PlayersData[2].ai_wealth=0; PlayersData[2].ai_power=1;
  PlayersData[2].unk_38[0]=0x55; PlayersData[2].unk_38[1]=0x66; PlayersData[2].unk_38[2]=0x99;
  PlayersData[2].unk_38[6]=0x77; PlayersData[2].unk_38[7]=0x88;
  PlayersData[1].unk_38[2]=0x99;
  popup_result=-1; PopupWriteCheckState=0x7FABCDEF;
  ConsoleGlobal.set_ai(2);
  assert(callback_word==0xA && PopupWriteCheckState==0xA);
  assert(PlayersData[2].ai_growth==0 && PlayersData[2].ai_tech==1);
  assert(PlayersData[2].ai_wealth==0 && PlayersData[2].ai_power==1 && sync_calls==0);
  assert(PlayersData[2].unk_38[0]==0x55 && PlayersData[2].unk_38[1]==0x66);
  assert(PlayersData[2].unk_38[2]==0x99);
  assert(PlayersData[2].unk_38[6]==0x77 && PlayersData[2].unk_38[7]==0x88);
  assert(PlayersData[1].unk_38[2]==0x99);
  popup_result=7;
  ConsoleGlobal.set_ai(2);
  assert(callback_word==0xA && PopupWriteCheckState==0x12345608);
  assert(PlayersData[2].ai_growth==0 && PlayersData[2].ai_tech==0);
  assert(PlayersData[2].ai_wealth==0 && PlayersData[2].ai_power==1 && sync_calls==1);
  popup_result=8;
  ConsoleGlobal.set_ai(2);
  assert(callback_word==0x8 && PopupWriteCheckState==0x123456F2);
  assert(PlayersData[2].ai_growth==0 && PlayersData[2].ai_tech==1);
  assert(PlayersData[2].ai_wealth==0 && PlayersData[2].ai_power==0 && sync_calls==2);

  // The R key's real extracted case follows the same named fields and uses
  // callback-updated low bits only; upper callback bytes remain irrelevant.
  ConsoleGlobal.faction_id_=2;
  GameDataStateStorage.fields_.game_more_preferences_=0x200;
  PlayersData[2].ai_growth=1; PlayersData[2].ai_tech=0;
  PlayersData[2].ai_wealth=1; PlayersData[2].ai_power=0;
  PlayersData[2].unk_38[2]=0x111; PlayersData[2].unk_38[3]=0x222;
  PlayersData[2].unk_38[4]=0x333; PlayersData[2].unk_38[5]=0x444;
  popup_result=8; ConsoleGlobal.on_key_ai_shortcut();
  assert(callback_word==5 && PopupWriteCheckState==0x123456F2);
  assert(PlayersData[2].ai_growth==0 && PlayersData[2].ai_tech==1);
  assert(PlayersData[2].ai_wealth==0 && PlayersData[2].ai_power==0 && sync_calls==3);
  assert(PlayersData[2].unk_38[2]==0x111 && PlayersData[2].unk_38[3]==0x222);
  assert(PlayersData[2].unk_38[4]==0x333 && PlayersData[2].unk_38[5]==0x444);
  popup_result=-1; PopupWriteCheckState=0x76543210;
  ConsoleGlobal.on_key_ai_shortcut();
  assert(callback_word==2 && PopupWriteCheckState==2 && sync_calls==3);
  assert(PlayersData[2].ai_growth==0 && PlayersData[2].ai_tech==1);
  GameDataStateStorage.fields_.game_more_preferences_=0;
  ConsoleGlobal.on_key_ai_shortcut();
  assert(social_select_calls==1);
  return 0;
}
'''
    path = tmp_path / "shared_turn_popup.cpp"
    executable = tmp_path / "shared_turn_popup"
    path.write_text(source, encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-O0", str(path), "-o", str(executable)], check=True)
    subprocess.run([str(executable)], check=True)

    wrong_owner = source.replace(
        "unsigned char flags = (PlayersData[faction_id].ai_growth != 0)",
        "unsigned char flags = (PlayersData[faction_id].unk_38[2] != 0)", 1)
    assert wrong_owner != source
    mutant_path = tmp_path / "wrong_ai_owner.cpp"
    mutant_executable = tmp_path / "wrong_ai_owner"
    mutant_path.write_text(wrong_owner, encoding="utf-8")
    subprocess.run([compiler, "-std=c++17", "-O0", str(mutant_path), "-o",
                    str(mutant_executable)], check=True)
    mutant = subprocess.run([str(mutant_executable)], capture_output=True, text=True)
    assert mutant.returncode != 0, "old unk_38 AI-priority receiver mutant survived"
