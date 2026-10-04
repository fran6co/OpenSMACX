"""Actual-source live Net ID / callback / return snapshot control."""
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


SOURCE = ROOT / "src/net_class.cpp"
FIXTURE = Path(__file__).with_name("net_join_live_id_control.cpp").read_text()
SIGNATURE = r"int\s+Net::join_session\s*\(NetAllocationNode\s*\*session,\s*char\s*\*player_name,"
MARKER = "// @@JOIN_SESSION_DEFINITION@@"


def build_and_run(compiler, body, expected_exit, directory, name):
    source = FIXTURE.replace(MARKER, body, 1)
    assert source.count(MARKER) == 0
    source_path = directory / (name + ".cpp")
    binary_path = directory / name
    source_path.write_text(source)
    build = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(source_path), "-o", str(binary_path)],
        capture_output=True, text=True, timeout=45,
    )
    assert build.returncode == 0, build.stdout + build.stderr
    result = subprocess.run([str(binary_path)], capture_output=True, timeout=15)
    assert result.returncode == expected_exit, (
        name + ": exit " + str(result.returncode) + ", expected " + str(expected_exit)
    )


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_net_join_uses_live_member_for_create_host_add_and_return_snapshot(compiler_name):
    body = definition(SOURCE.read_text(), SIGNATURE)
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " host compiler unavailable")

    # A minimal old-local mutant must be caught specifically by the CreatePlayer
    # output-address identity assertion (exit 31); do not trust result-only tests.
    before = "    if (NetDirectPlay->CreatePlayer(\n            &field_760_,"
    after = "    DPID player_id = field_760_;\n    if (NetDirectPlay->CreatePlayer(\n            &player_id,"
    assert body.count(before) == 1
    old_local_mutant = body.replace(before, after, 1)

    with tempfile.TemporaryDirectory(prefix="net-join-live-id-") as temporary:
        directory = Path(temporary)
        build_and_run(compiler, body, 0, directory, compiler_name + "-current")
        build_and_run(compiler, old_local_mutant, 31, directory,
                      compiler_name + "-old-local-mutant")
