"""Host-level behavior controls for genuine NetWin method bodies.

The test extracts methods from the current project source (or from an optional
NETWIN_SOURCE_ROOT overlay), injects them into the host-double fixture, then
compiles/runs the extracted bodies. These controls exercise call order and state
transitions only; the fixture hierarchy is not the target ABI and passing does
not establish native or runtime behavior.
"""
import os
from pathlib import Path
import shutil
import subprocess

import pytest
from test_net_hotseat_owner_actual_source_oct6 import installed_packet_declarations


PROJECT_ROOT = Path(__file__).resolve().parents[2]
SOURCE_ROOT = Path(os.environ.get("NETWIN_SOURCE_ROOT", PROJECT_ROOT))
SOURCE_FILE = SOURCE_ROOT / "src" / "netwin.cpp"
FIXTURE = Path(__file__).with_name("netwin_owner_host_control.cpp")

CASES = (
    (1, "NetWin::~NetWin()", ["-std=c++17", "-Wall", "-Wextra", "-Werror"]),
    (2, "void NetWin::prepare_game()", [
        "-std=c++17", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined"
    ]),
    (3, "int NetWin::pick_pass(int", ["-std=c++98", "-Dnullptr=0"]),
)


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
@pytest.mark.parametrize("kind,signature,flags", CASES)
def test_netwin_method_body(compiler_name, kind, signature, flags, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    assert SOURCE_FILE.is_file(), f"NetWin source not found: {SOURCE_FILE}"
    assert FIXTURE.is_file(), f"host fixture not found: {FIXTURE}"

    source = SOURCE_FILE.read_text()
    body = extract_function(source, signature)
    if kind == 3:
        # Reuse the actual shared packet declarations, not a fixture-only layout.
        body = "\n".join(
            (installed_packet_declarations(("NetWinLobbyPacketHeader",
                                            "NetWinLobbySinglePacket")),
             extract_function(source, "static void *memcpy_call("), body)
        )
    fixture = FIXTURE.read_text()
    marker = "/* NETWIN_BODY_INSERTION */"
    assert fixture.count(marker) == 3
    # Inject into the matching compile-time branch. The unused markers are
    # comments and remain inert in their preprocessor branches.
    branches = fixture.split(marker)
    assert len(branches) == 4
    generated = ""
    for index, branch in enumerate(branches[:-1]):
        generated += branch
        if index == kind - 1:
            generated += body
        generated += marker
    generated += branches[-1]
    generated_path = tmp_path / "netwin_owner_host_control.cpp"
    generated_path.write_text(generated)
    executable = tmp_path / f"netwin-control-{compiler_name}-{kind}"
    command = [
        compiler, *flags, f"-DNETWIN_OWNER_CONTROL_KIND={kind}",
        str(generated_path), "-o", str(executable)
    ]
    built = subprocess.run(
        command, capture_output=True, text=True, timeout=60, check=False
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run(
        [str(executable)], capture_output=True, text=True, timeout=30, check=False
    )
    assert ran.returncode == 0, ran.stdout + ran.stderr
