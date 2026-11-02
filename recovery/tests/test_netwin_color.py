"""Execute the product faction-colour body against typed lobby/clone records."""

import subprocess
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]


def _braced(source: str, prefix: str) -> str:
    start = source.index(prefix)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start : index + 1]
    raise AssertionError(f"unclosed {prefix}")


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_netwin_color_uses_lobby_and_clone_members(tmp_path: Path, compiler: str) -> None:
    header = (ROOT / "src/netwin.h").read_text()
    source = (ROOT / "src/netwin.cpp").read_text()
    structs = "\n".join(
        _braced(header, name) + ";"
        for name in ("struct NetWinLobbyRow", "struct NetWinFactionRecord")
    )
    body = _braced(source, "int NetWin::get_player_color(int faction)")
    harness = r"""
#include <cassert>
#include <cstdio>
#include <cstddef>
#include <cstdint>
using std::uint8_t;
using std::uint32_t;
using std::int8_t;
// Host behavior control only; native callback calling convention is not tested.
#ifndef __fastcall
#define __fastcall
#endif
""" + structs + r"""
NetWinLobbyRow NetWinLobbyRows[8] = {};
struct NetDaemon { int player_count_ = 3; } NetDaemonNet;
struct NetWin {
    void *faction_records_ = nullptr;
    int get_player_color(int faction);
};
""" + body + r"""
int main() {
    NetDaemonNet.player_count_ = 3;
    NetWin win;
    assert(win.get_player_color(1) == 0);
    // Keep every signed-byte record index physically valid without adding a
    // product bound: the pin uses the signed colour unchanged when unique.
    NetWinFactionRecord physical_records[256] = {};
    NetWinFactionRecord *records = physical_records + 128;
    win.faction_records_ = records;
    NetWinLobbyRows[1].faction_or_color_ = -1;
    assert(win.get_player_color(1) == 0xFF);
    NetWinLobbyRows[1].faction_or_color_ = 4;
    records[0].color_ = 7;
    records[4].color_ = 29;
    if (win.get_player_color(1) != 29) {
        std::fputs("unique-colour-keeps-signed-record-index\n", stderr);
        return 41;
    }

    records[0].record_count_ = 4;
    NetWinLobbyRows[1].faction_or_color_ = 0;
    NetWinLobbyRows[2].faction_or_color_ = 1;
    NetWinLobbyRows[3].faction_or_color_ = 0;
    records[2].color_ = 9;
    assert(win.get_player_color(3) == 9);

    records[0].color_ = 7;
    records[1].color_ = 17;
    records[3].color_ = 37;
    NetDaemonNet.player_count_ = 3;
    NetWinLobbyRows[1].faction_or_color_ = 0;
    NetWinLobbyRows[2].faction_or_color_ = 1;
    NetWinLobbyRows[3].faction_or_color_ = 2;
    NetWinLobbyRows[4].faction_or_color_ = 0;
    records[0].record_count_ = 3;
    // Every candidate is used: native returns the index at logical count,
    // without adding a clamp. Physical backing remains large enough.
    assert(win.get_player_color(4) == 37);
    records[0].record_count_ = 0;
    assert(win.get_player_color(4) == 7);
    records[0].record_count_ = 3;
    NetDaemonNet.player_count_ = 0;
    assert(win.get_player_color(4) == 7);
    NetDaemonNet.player_count_ = -1;
    assert(win.get_player_color(4) == 37);

    NetWinLobbyRows[1].faction_or_color_ = 3;
    NetWinLobbyRows[2].faction_or_color_ = 4;
    assert(win.get_player_color(2) == 29);
    NetWinLobbyRows[2].faction_or_color_ = 3;
    NetDaemonNet.player_count_ = 2;
    assert(win.get_player_color(2) == 7);

    for (int color = -128; color <= 127; ++color) {
        records[color].color_ = static_cast<uint8_t>(color + 128);
    }
    for (int color = -128; color <= 127; ++color) {
        NetWinLobbyRows[1].faction_or_color_ = static_cast<int8_t>(color);
        const int expected = color == -1 ? 0xFF : color + 128;
        if (win.get_player_color(1) != expected) {
            std::fputs("signed-byte-unique-index-or-ff-sentinel\n", stderr);
            return 42;
        }
    }
    return 0;
}
"""
    cpp = tmp_path / "color.cpp"
    binary = tmp_path / "color"
    cpp.write_text(harness)
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)

    assert body.count("int index = color;") == 1
    mutant = harness.replace("int index = color;", "int index = 0;", 1)
    mutant_cpp = tmp_path / "old_zero_index.cpp"
    mutant_binary = tmp_path / "old_zero_index"
    mutant_cpp.write_text(mutant)
    subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         str(mutant_cpp), "-o", str(mutant_binary)], check=True,
    )
    rejected = subprocess.run([str(mutant_binary)], capture_output=True, text=True)
    assert rejected.returncode == 41
    assert "unique-colour-keeps-signed-record-index" in rejected.stderr
