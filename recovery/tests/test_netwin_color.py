"""Execute the product faction-colour body against typed lobby/clone records."""

import subprocess
from pathlib import Path


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


def test_netwin_color_uses_lobby_and_clone_members(tmp_path: Path) -> None:
    header = (ROOT / "src/netwin.h").read_text()
    source = (ROOT / "src/netwin.cpp").read_text()
    structs = "\n".join(
        _braced(header, name) + ";"
        for name in ("struct NetWinLobbyRow", "struct NetWinFactionRecord")
    )
    body = _braced(source, "int NetWin::get_player_color(int faction)")
    harness = r"""
#include <cassert>
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
    NetWinFactionRecord records[4] = {};
    win.faction_records_ = records;
    NetWinLobbyRows[1].faction_or_color_ = -1;
    assert(win.get_player_color(1) == 0xFF);
    NetWinLobbyRows[1].faction_or_color_ = 4;
    records[0].color_ = 7;
    assert(win.get_player_color(1) == 7);

    records[0].record_count_ = 4;
    NetWinLobbyRows[1].faction_or_color_ = 0;
    NetWinLobbyRows[2].faction_or_color_ = 1;
    NetWinLobbyRows[3].faction_or_color_ = 0;
    records[2].color_ = 9;
    assert(win.get_player_color(3) == 9);
}
"""
    cpp = tmp_path / "color.cpp"
    binary = tmp_path / "color"
    cpp.write_text(harness)
    subprocess.run(
        ["g++", "-std=c++17", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(binary)],
        check=True,
    )
    subprocess.run([str(binary)], check=True)
