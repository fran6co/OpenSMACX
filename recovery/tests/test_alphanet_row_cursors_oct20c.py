"""Actual-source host check for the typed AlphaNet lobby-row cursor."""

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import definition


ROOT = Path(__file__).resolve().parents[2]


HARNESS = r'''typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned int uint32_t;

struct NetWinLobbyRow {
    uint8_t prefix_;
    uint8_t ready_or_occupied_;
    uint8_t field_2_;
    int8_t faction_or_color_;
    uint8_t field_4_;
    char leader_name_[0x18];
    char formal_faction_name_[0x28];
    char noun_faction_[0x18];
    char adjective_faction_[0x80];
    char leader_title_[0x18];
    char adjective_leader_[0x80];
    uint8_t tail_[3];
    uint32_t owner_;
};
typedef char row_stride_check[sizeof(NetWinLobbyRow) == 0x17C ? 1 : -1];
NetWinLobbyRow *NetWinLobbyRows;

FUNCTION

struct GuardedRows {
    uint32_t before;
    NetWinLobbyRow rows[8];
    uint32_t after;
};

int main() {
    GuardedRows guarded;
    if (sizeof(guarded.rows) != 8 * 0x17C) return 1;
    guarded.before = 0x13579BDF;
    guarded.after = 0x2468ACE0;
    NetWinLobbyRows = guarded.rows;
    unsigned char *const first = (unsigned char *)&guarded.rows[0];
    for (unsigned int i = 0; i != sizeof(guarded.rows); ++i)
        first[i] = (unsigned char)(0x31 + i * 13);
    unsigned char before[sizeof(guarded.rows)];
    for (unsigned int i = 0; i != sizeof(guarded.rows); ++i)
        before[i] = first[i];

    reset_net_lobby_rows();
    for (int row = 0; row != 8; ++row) {
        if (NetWinLobbyRows[row].owner_ != 0 ||
            NetWinLobbyRows[row].prefix_ != 0xFF ||
            NetWinLobbyRows[row].ready_or_occupied_ != 0 ||
            NetWinLobbyRows[row].field_2_ != 0 ||
            NetWinLobbyRows[row].faction_or_color_ != -1 ||
            NetWinLobbyRows[row].field_4_ != 2)
            return 2;
    }
    for (unsigned int i = 0; i != sizeof(guarded.rows); ++i) {
        const unsigned int in_row = i % 0x17C;
        const bool selected = in_row < 5 || in_row >= 0x178;
        if (!selected && first[i] != before[i]) return 3;
    }
    if (guarded.before != 0x13579BDF || guarded.after != 0x2468ACE0)
        return 4;
    return 0;
}
'''


def _run(tmp_path, compiler, function, *, succeeds):
    source = HARNESS.replace("FUNCTION", function, 1)
    cpp = tmp_path / "alphanet_row_cursor.cpp"
    executable = tmp_path / "alphanet_row_cursor"
    cpp.write_text(source)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(executable)],
        capture_output=True, text=True, timeout=45)
    assert built.returncode == 0, built.stdout + built.stderr
    result = subprocess.run([str(executable)], capture_output=True, text=True,
                            timeout=15)
    if succeeds:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "damaged row traversal escaped its control"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_lobby_row_cursor_and_cross_row_damage_control(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " is unavailable")
    source = (ROOT / "src/alphanet.cpp").read_text(encoding="utf-8")
    actual = definition(source, r"void reset_net_lobby_rows\(\)\s*")

    with tempfile.TemporaryDirectory(prefix="alphanet-row-cursor-") as directory:
        tmp_path = Path(directory)
        _run(tmp_path, compiler, actual, succeeds=True)

        damaged = actual.replace("row < 8", "row < 7", 1)
        assert damaged != actual
        _run(tmp_path, compiler, damaged, succeeds=False)
