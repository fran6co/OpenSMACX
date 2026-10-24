"""Actual-source controls for Console::use_time_bonus's gates and messages."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess


ROOT = Path(__file__).resolve().parents[2]


def _body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start : end + 1]
    raise AssertionError("unterminated Console::use_time_bonus body")


HARNESS = r'''
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct GameFields {
    uint32_t time_controls_selection_;
    uint32_t net_turn_flags_;
    int net_turn_faction_;
    uint32_t game_state_;
};
struct GameStorage { GameFields fields_; } GameDataStateStorage{};
int ConsoleControlTurnActive = 0;
int IsMultiplayerNet = 0;
struct ConsoleState { int faction_id_; } ConsoleGlobal{};
struct PlayerData { int time_bonus_count; } PlayersData[8]{};
// Match the native 0x20-byte row and +0x18 refresh member using a 32-bit
// name placeholder; host pointer width must not move the tested field.
struct TimeControlEntry {
    uint32_t name;
    int turn, base, unit, event, extra, refresh, accumulated;
};
static_assert(sizeof(TimeControlEntry) == 0x20, "native time-control stride");
static_assert(offsetof(TimeControlEntry, refresh) == 0x18,
              "native time-control refresh offset");
TimeControlEntry TimeControl[6]{};
struct NetDaemonPrimary { uint32_t leading_words_[3]; uint32_t flags_; int session_phase_; uint32_t trailing_words_[9]; };
struct NetDaemon { NetDaemonPrimary primary_state_; } NetDaemonNet{};

std::vector<std::string> events;
int selection_after_parse = -1;
int last_parse_arg0 = -1;
int last_parse_arg1 = -1;
int last_message_id = -1;
const char *last_popup = nullptr;

struct NetMsg {
    void pop(const char *key, int duration, int a, void *callback) {
        assert(duration == 5000 && a == 0 && callback == nullptr);
        last_popup = key;
        events.push_back(std::string("pop:") + key);
    }
} NetMsgPrimary;

void parse_num(int a, int value) {
    last_parse_arg0 = a;
    last_parse_arg1 = value;
    events.push_back("parse_num");
    if (selection_after_parse >= 0) {
        GameDataStateStorage.fields_.time_controls_selection_ =
            static_cast<uint32_t>(selection_after_parse);
    }
}
void message_data(int id, int, int, int, int, int) {
    last_message_id = id;
    events.push_back("message_data");
}

struct Console {
    int faction_id_;
    void use_time_bonus();
};

void Console::use_time_bonus() BODY

static void reset() {
    GameDataStateStorage.fields_ = {2, 0, 0, 0};
    ConsoleControlTurnActive = 0;
    IsMultiplayerNet = 1;
    ConsoleGlobal.faction_id_ = 3;
    for (auto &player : PlayersData) player.time_bonus_count = 1;
    for (int i = 0; i != 6; ++i) {
        TimeControl[i].extra = 100 + i;
        TimeControl[i].refresh = 700 + i;
    }
    NetDaemonNet.primary_state_.session_phase_ = 0;
    for (auto &word : NetDaemonNet.primary_state_.trailing_words_) word = 0;
    events.clear();
    selection_after_parse = -1;
    last_parse_arg0 = last_parse_arg1 = last_message_id = -1;
    last_popup = nullptr;
}

static void test_three_early_gates() {
    Console self{2};
    reset(); ConsoleControlTurnActive = 1; self.use_time_bonus();
    assert(events.empty());
    reset(); GameDataStateStorage.fields_.time_controls_selection_ = 0;
    self.use_time_bonus(); assert(events.empty());
    reset(); IsMultiplayerNet = 0; self.use_time_bonus();
    assert(events.empty());
}

static void test_network_turn_mismatch() {
    Console self{2}; reset();
    GameDataStateStorage.fields_.net_turn_flags_ = 0x10;
    GameDataStateStorage.fields_.net_turn_faction_ = 4;
    self.use_time_bonus();
    assert((events == std::vector<std::string>{"pop:SIMULNO"}));
    assert(last_popup && std::string(last_popup) == "SIMULNO");
}

static void test_no_bonus_keeps_selected_refresh_across_callback() {
    Console self{2}; reset();
    GameDataStateStorage.fields_.time_controls_selection_ = 3;
    PlayersData[3].time_bonus_count = 0;
    selection_after_parse = 4;
    self.use_time_bonus();
    assert((events == std::vector<std::string>{"parse_num", "pop:NOTIMEBONUSES"}));
    assert(last_parse_arg0 == 0 && last_parse_arg1 == 703);
    assert(GameDataStateStorage.fields_.time_controls_selection_ == 4);

    reset();
    GameDataStateStorage.fields_.time_controls_selection_ = 2;
    PlayersData[3].time_bonus_count = -1;
    self.use_time_bonus();
    assert((events == std::vector<std::string>{"parse_num", "pop:NOTIMEBONUSES"}));
    assert(last_parse_arg0 == 0 && last_parse_arg1 == 702);

    reset();
    GameDataStateStorage.fields_.time_controls_selection_ = 1;
    PlayersData[3].time_bonus_count = 0;
    TimeControl[1].extra = 111;
    TimeControl[1].refresh = 0;
    self.use_time_bonus();
    assert((events == std::vector<std::string>{"parse_num", "pop:NOTIMEBONUSES"}));
    assert(last_parse_arg0 == 0 && last_parse_arg1 == 0);
}

static void test_end_of_game_and_already_used_gates() {
    Console self{2}; reset();
    GameDataStateStorage.fields_.game_state_ = 0x800;
    self.use_time_bonus();
    assert((events == std::vector<std::string>{"pop:NOBONUSATEND"}));

    reset();
    GameDataStateStorage.fields_.game_state_ = 2;
    NetDaemonNet.primary_state_.trailing_words_[6] = 1u << 2;
    self.use_time_bonus();
    assert((events == std::vector<std::string>{"pop:NOBONUSDONE"}));
}

static void test_success_message() {
    Console self{2}; reset();
    GameDataStateStorage.fields_.game_state_ = 0;
    self.use_time_bonus();
    assert((events == std::vector<std::string>{"message_data"}));
    assert(last_message_id == 0x2306);
}

int main() {
    test_three_early_gates();
    test_network_turn_mismatch();
    test_no_bonus_keeps_selected_refresh_across_callback();
    test_end_of_game_and_already_used_gates();
    test_success_message();
}
'''


def test_actual_use_time_bonus_body_with_host_controls(tmp_path: Path) -> None:
    source = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    body = _body(source, "void Console::use_time_bonus()")
    assert "void Console::use_time_bonus()" in body
    program = HARNESS.replace("BODY", body[len("void Console::use_time_bonus() ") :])
    cpp = tmp_path / "use_time_bonus.cpp"
    cpp.write_text(program, encoding="utf-8")
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    binary = tmp_path / "use_time_bonus_controls"
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)


def test_use_time_bonus_gate_damage_control(tmp_path: Path) -> None:
    source = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    body = _body(source, "void Console::use_time_bonus()")
    anchor = "if (ConsoleControlTurnActive) return;"
    assert anchor in body
    mutated = body.replace(anchor, "if (false) return;", 1)
    program = HARNESS.replace("BODY", mutated[len("void Console::use_time_bonus() ") :])
    cpp = tmp_path / "use_time_bonus_mutant.cpp"
    cpp.write_text(program, encoding="utf-8")
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    binary = tmp_path / "use_time_bonus_mutant"
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        check=True,
    )
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    assert result.returncode != 0, "removing the turn-active gate escaped the host controls"


def test_use_time_bonus_wrong_time_control_field_damage_control(tmp_path: Path) -> None:
    source = (ROOT / "src/console.cpp").read_text(encoding="utf-8")
    body = _body(source, "void Console::use_time_bonus()")
    anchor = "TimeControl[selection].refresh"
    assert anchor in body
    mutated = body.replace(anchor, "TimeControl[selection].extra", 1)
    program = HARNESS.replace("BODY", mutated[len("void Console::use_time_bonus() ") :])
    cpp = tmp_path / "use_time_bonus_wrong_field_mutant.cpp"
    cpp.write_text(program, encoding="utf-8")
    compiler = shutil.which("clang++") or shutil.which("g++")
    assert compiler, "host C++ compiler required"
    binary = tmp_path / "use_time_bonus_wrong_field_mutant"
    subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        check=True,
    )
    result = subprocess.run([str(binary)], capture_output=True, text=True)
    assert result.returncode != 0, "substituting TimeControl.extra escaped the host controls"
