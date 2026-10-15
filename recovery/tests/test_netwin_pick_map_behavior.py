"""Compile and run controls against the actual current NetWin::pick_map body.

The fixture supplies host doubles for the method's dependencies; it does not
model the target ABI or prove native/gameplay behavior. The method itself is
extracted from src/netwin.cpp on each run, so this test does not carry a second
copy of recovered production logic.
"""
import os
from pathlib import Path
import shutil
import subprocess

import pytest


PROJECT_ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = Path(os.environ.get("NETWIN_SOURCE_ROOT", PROJECT_ROOT))
SOURCE_FILE = SOURCE_ROOT / "src" / "netwin.cpp"
HEADER_FILE = SOURCE_ROOT / "src" / "netwin.h"
FIXTURE = Path(__file__).with_name("netwin_pick_map_host_control.cpp")
LOCK_MASK_MARKER = "/* NETWIN_GAME_DATA_LOCK_MASK_INSERTION */"
MARKER = "/* NETWIN_PICK_MAP_BODY_INSERTION */"
RECORD_MARKER = "/* NETWIN_FACTION_RECORD_INSERTION */"
SESSION_BLOCK_MARKER = "/* NETWIN_SESSION_BLOCK_INSERTION */"
SESSION_PACKET_MARKER = "/* NETWIN_SESSION_PACKET_INSERTION */"


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
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
                    return source[start:index + 1]
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
    raise AssertionError(f"unclosed function body for {signature}")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_netwin_pick_map_actual_body(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    assert SOURCE_FILE.is_file(), f"NetWin source not found: {SOURCE_FILE}"
    assert HEADER_FILE.is_file(), f"NetWin header not found: {HEADER_FILE}"
    assert FIXTURE.is_file(), f"host fixture not found: {FIXTURE}"

    body = extract_function(SOURCE_FILE.read_text(), "void NetWin::pick_map()")
    fixture = FIXTURE.read_text()
    assert fixture.count(LOCK_MASK_MARKER) == 1
    assert fixture.count(MARKER) == 1
    assert fixture.count(RECORD_MARKER) == 1
    record = extract_function(
        HEADER_FILE.read_text(), "struct NetWinFactionRecord"
    ) + ";"
    session_block = extract_function(
        HEADER_FILE.read_text(), "struct NetWinSessionBlock"
    ) + ";"
    session_packet = extract_function(
        HEADER_FILE.read_text(), "struct NetWinSessionLobbyPacket"
    ) + ";"
    lock_header = (SOURCE_ROOT / "src" / "game_data_state_window.h").read_text()
    lock_bytes = extract_function(
        lock_header, "struct GameDataLockMaskBytes"
    ) + ";"
    lock_view = extract_function(
        lock_header, "union GameDataLockMaskView"
    ) + ";"

    def generate(candidate_body: str) -> str:
        return (fixture.replace(LOCK_MASK_MARKER, lock_bytes + "\n" + lock_view)
                .replace(RECORD_MARKER, record)
                 .replace(SESSION_BLOCK_MARKER, session_block)
                 .replace(SESSION_PACKET_MARKER, session_packet)
                 .replace(MARKER, candidate_body))

    generated = generate(body)
    generated_path = tmp_path / "netwin_pick_map_host_control.cpp"
    generated_path.write_text(generated)
    executable = tmp_path / f"netwin-pick-map-{compiler_name}"
    command = [
        compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
        str(generated_path), "-o", str(executable),
    ]
    built = subprocess.run(
        command, capture_output=True, text=True, timeout=60, check=False
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run(
        [str(executable)], capture_output=True, text=True, timeout=30, check=False
    )
    assert ran.returncode == 0, ran.stdout + ran.stderr

    damaged_body = body.replace(
        "record.adjective_faction_", "record.leader_title_", 1
    )
    assert damaged_body != body, "server roster mutation anchor must be present"
    damaged_source = generate(damaged_body)
    damaged_path = tmp_path / "netwin_pick_map_damaged.cpp"
    damaged_path.write_text(damaged_source)
    damaged_executable = tmp_path / f"netwin-pick-map-damaged-{compiler_name}"
    damaged_build = subprocess.run(
        [
            compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
            str(damaged_path), "-o", str(damaged_executable),
        ],
        capture_output=True, text=True, timeout=60, check=False,
    )
    assert damaged_build.returncode == 0, damaged_build.stdout + damaged_build.stderr
    damaged_run = subprocess.run(
        [str(damaged_executable)], capture_output=True, text=True,
        timeout=30, check=False,
    )
    assert damaged_run.returncode == 98, damaged_run.stdout + damaged_run.stderr
    assert "CONTROL FAILURE: server roster stores adjective in named +0x7A slot" in damaged_run.stderr

    alive_as_human = body.replace(
        "bytes_.alive_mask_", "bytes_.human_mask_"
    )
    assert alive_as_human != body, "the alive-byte consumer must remain in the actual body"
    alive_damaged_path = tmp_path / "netwin_pick_map_wrong_mask.cpp"
    alive_damaged_path.write_text(generate(alive_as_human))
    alive_damaged_executable = tmp_path / f"netwin-pick-map-wrong-mask-{compiler_name}"
    alive_damaged_build = subprocess.run(
        [
            compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
            str(alive_damaged_path), "-o", str(alive_damaged_executable),
        ],
        capture_output=True, text=True, timeout=60, check=False,
    )
    assert alive_damaged_build.returncode == 0, alive_damaged_build.stdout + alive_damaged_build.stderr
    alive_damaged_run = subprocess.run(
        [str(alive_damaged_executable)], capture_output=True, text=True,
        timeout=30, check=False,
    )
    assert alive_damaged_run.returncode != 0, "wrong named-byte member escaped behavioral controls"


def test_pick_map_uses_alive_byte_at_each_pinned_consumer_and_keeps_count_masks():
    body = extract_function(SOURCE_FILE.read_text(), "void NetWin::pick_map()")
    assert body.count("lock_masks_.bytes_.alive_mask_") == 3
    assert "lock_masks_.enable_mask_ >> 8" not in body
    assert body.count("bytes_.alive_mask_ & 0xFEu") == 2
    status_byte = body.index(
        "NetWinSessionOptions.status_ =\n                GameDataStateStorage.fields_.lock_masks_.bytes_.alive_mask_;"
    )
    assert body.index("bytes_.alive_mask_ & 0xFEu") < status_byte
    assert status_byte < body.rindex("bytes_.alive_mask_ & 0xFEu")
