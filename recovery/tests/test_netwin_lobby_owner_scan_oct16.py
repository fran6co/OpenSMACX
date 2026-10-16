"""Host controls for the actual NetWin lobby-owner scan source block."""
from pathlib import Path
import importlib.util
import os
import shutil
import subprocess

import pytest

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/netwin.cpp").is_file())
EXTRACTOR = ROOT / "recovery/tests/test_netwin_owner_behavior.py"
spec = importlib.util.spec_from_file_location("netwin_owner_extract_oct16", EXTRACTOR)
extractor = importlib.util.module_from_spec(spec)
spec.loader.exec_module(extractor)


def declaration(header: str, name: str) -> str:
    start = header.index(f"struct {name} {{")
    opening = header.index("{", start)
    depth = 0
    for index in range(opening, len(header)):
        if header[index] == "{":
            depth += 1
        elif header[index] == "}":
            depth -= 1
            if depth == 0:
                end = header.index(";", index) + 1
                return header[start:end]
    raise AssertionError(f"unclosed declaration: {name}")


def actual_scan_block() -> str:
    source = (ROOT / "src/netwin.cpp").read_text()
    body = extractor.extract_function(source, "void NetWin::draw_session_info()")
    start = body.index("int connected = 0;")
    loop = body.index("for (int slot", start)
    opening = body.index("{", loop)
    depth = 0
    for index in range(opening, len(body)):
        if body[index] == "{":
            depth += 1
        elif body[index] == "}":
            depth -= 1
            if depth == 0:
                return body[start:index + 1]
    raise AssertionError("unclosed connected-player scan")


def fixture(block: str) -> str:
    header = (ROOT / "src/netwin.h").read_text()
    row = declaration(header, "NetWinLobbyRow")
    faction = declaration(header, "NetWinFactionRecord")
    return r'''#include <cstddef>
#include <cstdint>
#include <cstring>

''' + row + "\n" + faction + r'''
static_assert(sizeof(NetWinLobbyRow) == 0x17C, "actual lobby row stride");
static_assert(offsetof(NetWinLobbyRow, owner_) == 0x178, "actual owner offset");
static_assert(sizeof(NetWinFactionRecord) == 0x190, "actual faction record stride");
struct NetDaemonHostView { int player_count_; };
static NetDaemonHostView NetDaemonNet;
static NetWinLobbyRow NetWinLobbyRows[8];

struct ScanHarness {
    NetWinFactionRecord faction_records_[8]{};
    int tally() {
''' + block + r'''
        return connected;
    }
};

static bool scenario(int count, uint32_t claimed,
                     const uint32_t row_owners[8], int expected) {
    std::memset(NetWinLobbyRows, 0, sizeof(NetWinLobbyRows));
    for (int index = 0; index != 8; ++index)
        NetWinLobbyRows[index].owner_ = row_owners[index];
    NetDaemonNet.player_count_ = count;
    ScanHarness scan{};
    scan.faction_records_[0].owner_ = claimed;
    return scan.tally() == expected;
}

int main() {
    const uint32_t row_zero_only[8] = {42, 0, 0, 0, 0, 0, 0, 0};
    const uint32_t first_player[8] = {42, 42, 0, 0, 0, 0, 0, 0};
    const uint32_t middle_player[8] = {0, 0, 42, 0, 0, 0, 0, 0};
    const uint32_t last_player[8] = {0, 0, 0, 0, 0, 0, 0, 42};
    const uint32_t no_player[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    const uint32_t beyond_live_count[8] = {0, 0, 42, 0, 0, 0, 0, 0};

    if (!scenario(7, 42, row_zero_only, 1)) return 1; // starts at row one
    if (!scenario(7, 42, first_player, 0)) return 2;
    if (!scenario(7, 42, middle_player, 0)) return 3; // detects skipped stride
    if (!scenario(7, 42, last_player, 0)) return 4;
    if (!scenario(7, 42, no_player, 1)) return 5;
    if (!scenario(1, 42, first_player, 0)) return 6;
    if (!scenario(1, 42, no_player, 1)) return 7;
    if (!scenario(0, 42, no_player, 1)) return 8; // zero live players
    if (!scenario(-1, 42, no_player, 0)) return 9; // signed negative count
    if (!scenario(7, 0, first_player, 0)) return 10; // zero claim is excluded
    if (!scenario(0, 0, no_player, 0)) return 11;
    if (!scenario(1, 42, beyond_live_count, 1)) return 12; // don't scan row two
    return 0;
}
'''


def compile_and_run(compiler: str, block: str, tmp_path: Path, name: str) -> int:
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(fixture(block))
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    run = subprocess.run(
        [str(exe)], capture_output=True, text=True, timeout=10,
        env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"),
    )
    return run.returncode


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_source_scan_and_damage_controls(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    block = actual_scan_block()
    assert compile_and_run(compiler, block, tmp_path, "actual-scan") == 0

    starts_at_row_zero = block.replace(
        "&NetWinLobbyRows[1]", "&NetWinLobbyRows[0]", 1
    )
    assert starts_at_row_zero != block
    assert compile_and_run(compiler, starts_at_row_zero, tmp_path,
                           "row-zero-damage") != 0

    skips_rows = block.replace("++scan;", "scan += 2;", 1)
    assert skips_rows != block
    assert compile_and_run(compiler, skips_rows, tmp_path,
                           "stride-damage") != 0

    counts_zero_owner = block.replace("if (owned != 0)", "if (true)", 1)
    assert counts_zero_owner != block
    assert compile_and_run(compiler, counts_zero_owner, tmp_path,
                           "zero-owner-damage") != 0
