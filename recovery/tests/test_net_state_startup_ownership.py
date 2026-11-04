"""Host control for the actual global storage constructors and wire PODs."""

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
CONTROL = ROOT / "recovery/tests/net_state_startup_placement_control.cpp"


def extract_struct(source: str, name: str) -> str:
    signature = f"struct {name} {{"
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
            if char in ('"', "'"):
                state = "string" if char == '"' else "character"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1] + ";"
        elif state == "line" and char == "\n":
            state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 1
        elif state in ("string", "character"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (
                state == "character" and char == "'"
            ):
                state = "code"
        index += 1
    raise AssertionError(f"unterminated actual declaration: {name}")


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_global_startup_constructors_preserve_unwritten_and_packet_bytes(
    compiler_name: str,
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")

    with tempfile.TemporaryDirectory(prefix="net-state-startup-owner-") as directory:
        executable = Path(directory) / "net_state_startup_placement_control"
        header = (ROOT / "src/netwin.h").read_text(encoding="utf-8")
        declarations = "\n".join(
            extract_struct(header, name)
            for name in (
                "NetWinLobbyRow",
                "NetWinLobbyRowsStorage",
                "NetWinLobbyPacketHeader",
                "NetWinLobbySinglePacket",
                "NetWinLobbyFullPacket",
                "NetWinSessionBlock",
                "NetWinSessionDataStorage",
                "NetWinSessionOptionsState",
                "NetWinSessionOptionsStorage",
                "NetWinSessionOptionsPacket",
            )
        )
        control = CONTROL.read_text(encoding="utf-8").replace(
            "/* ACTUAL_NETWIN_DECLARATIONS */", declarations, 1
        )
        control_path = Path(directory) / "net_state_startup_placement_control.cpp"
        control_path.write_text(control, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
             str(control_path), "-o", str(executable)],
            cwd=ROOT,
            capture_output=True,
            text=True,
            timeout=45,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run(
            [str(executable)], capture_output=True, text=True, timeout=15
        )
        assert result.returncode == 0, result.stdout + result.stderr
