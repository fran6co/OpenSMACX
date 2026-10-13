"""Actual-source controls for NetWin's two recovered record text members."""

from pathlib import Path
import shutil
import subprocess

import pytest
from test_net_hotseat_owner_actual_source_oct6 import installed_packet_declarations


ROOT = Path(__file__).resolve().parents[2]
HEADER = (ROOT / "src/netwin.h").read_text()
SOURCE = (ROOT / "src/netwin.cpp").read_text()


def _definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[start:index + 1]
        elif state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 2
                continue
        elif state in ("string", "character"):
            if char == "\\":
                index += 2
                continue
            if (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError(f"unterminated definition: {signature}")


def _struct(text: str, name: str) -> str:
    return _definition(text, f"struct {name}")


RECORD_LAYOUT = r"""
#include <cstddef>
#include <cstdint>
#include <type_traits>
RECORD_TYPE
static_assert(sizeof(NetWinFactionRecord) == 0x190, "record stride");
static_assert(offsetof(NetWinFactionRecord, adjective_faction_) == 0x7A,
              "adjective slot offset");
static_assert(sizeof(((NetWinFactionRecord *)nullptr)->adjective_faction_) == 0x80,
              "adjective slot extent");
static_assert(offsetof(NetWinFactionRecord, leader_title_) == 0xFA,
              "title slot offset");
static_assert(sizeof(((NetWinFactionRecord *)nullptr)->leader_title_) == 0x18,
              "title slot extent");
static_assert(0xFA + sizeof(((NetWinFactionRecord *)nullptr)->leader_title_) == 0x112,
              "unresolved tail starts at +0x112");
static_assert(std::is_same<decltype(NetWinFactionRecord::adjective_faction_),
                           char[0x80]>::value,
              "adjective slot remains a char array");
static_assert(std::is_same<decltype(NetWinFactionRecord::leader_title_),
                           char[0x18]>::value,
              "title slot remains a char array");
int main() { return 0; }
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_record_header_field_offsets_extents_and_types(
    tmp_path: Path, compiler_name: str
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    record = _struct(HEADER, "NetWinFactionRecord") + ";"
    cpp = tmp_path / "netwin-record-layout.cpp"
    exe = tmp_path / "netwin-record-layout"
    cpp.write_text(RECORD_LAYOUT.replace("RECORD_TYPE", record))
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True,
                         timeout=10, check=False)
    assert ran.returncode == 0, ran.stdout + ran.stderr


FILL_RUNTIME = r"""
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdint>
ACTUAL_TYPES
static_assert(sizeof(NetWinFactionRecord) == 0x190, "record stride");
static char *label_get(int) { return const_cast<char *>("fallback"); }
class NetWin {
public:
    NetWinFactionRecord *faction_records_;
    void fill_player(NetWinLobbyRowView, int);
};
FILL_BODY
static int check(bool condition, const char *message) {
    if (!condition) {
        std::fprintf(stderr, "CONTROL FAILURE: %s\n", message);
        return 98;
    }
    return 0;
}
int main() {
    NetWinFactionRecord records[3] = {};
    NetWin win;
    win.faction_records_ = records;
    std::strcpy(records[1].leader_name_, "ObservedLeader");
    std::strcpy(records[1].formal_faction_name_, "ObservedFormal");
    std::strcpy(records[1].noun_faction_, "ObservedNoun");
    std::strcpy(records[1].adjective_faction_, "ObservedAdjective");
    std::strcpy(records[1].leader_title_, "ObservedTitle");
    unsigned char *bytes = reinterpret_cast<unsigned char *>(records);
    char *adjective_leader = reinterpret_cast<char *>(bytes + 0x190 + 0x112);
    std::memset(adjective_leader, 'q', 0x7F);
    adjective_leader[0x7F] = 0;
    bytes[0x190 + 0x192] = 0xE7;
    NetWinLobbyRow player = {};
    win.fill_player(NetWinLobbyRowView(player), 1);
    if (check(std::strcmp(player.adjective_faction_, "ObservedAdjective") == 0,
              "copied adjective from record +0x7A")) return 98;
    if (check(std::strcmp(player.leader_title_, "ObservedTitle") == 0,
              "copied title from record +0xFA")) return 98;
    if (check(std::strcmp(player.leader_name_, "ObservedLeader") == 0,
              "leader name remains independently sourced")) return 98;
    if (check(std::strcmp(player.formal_faction_name_, "ObservedFormal") == 0,
              "formal faction name remains independently sourced")) return 98;
    if (check(std::strcmp(player.noun_faction_, "ObservedNoun") == 0,
              "noun remains independently sourced")) return 98;
    if (check(player.adjective_leader_[0] == 'q' &&
              player.adjective_leader_[0x7E] == 'q' &&
              player.adjective_leader_[0x7F] == 0,
              "leader adjective still reads the cross-row +0x112 spill")) return 98;
    if (check(player.tail_[0] == 0xE7,
              "gender byte still aliases the next row at +0x192")) return 98;
    return 0;
}
"""


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_fill_player_named_slots_and_compile_successful_damage_control(
    tmp_path: Path, compiler_name: str
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    types = installed_packet_declarations(("NetWinFactionRecord", "NetWinLobbyRow",
                                           "NetWinLobbyRowView"))
    body = _definition(SOURCE, "void NetWin::fill_player(")

    def compile_and_run(method: str, expected_status: int, label: str,
                        expected_message: str | None = None) -> None:
        harness = FILL_RUNTIME.replace("ACTUAL_TYPES", types).replace("FILL_BODY", method)
        cpp = tmp_path / f"netwin-fill-{label}.cpp"
        exe = tmp_path / f"netwin-fill-{label}"
        cpp.write_text(harness)
        built = subprocess.run(
            [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=30, check=False,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True,
                             timeout=10, check=False)
        assert ran.returncode == expected_status, ran.stdout + ran.stderr
        if expected_message is not None:
            assert expected_message in ran.stderr

    compile_and_run(body, 0, "current")
    damaged = body.replace(
        "record.adjective_faction_", "record.leader_title_", 1
    )
    assert damaged != body, "mutation anchor must identify the extracted source"
    compile_and_run(damaged, 98, "wrong-adjective-slot",
                    "CONTROL FAILURE: copied adjective from record +0x7A")
