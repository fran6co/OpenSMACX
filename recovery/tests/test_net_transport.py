"""Actual-source host sender controls; not a native ABI or game proof."""
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


FIXTURE = Path(__file__).with_name("net_transport_control.cpp").read_text()


def actual_bodies():
    source = (ROOT / "src/net_class.cpp").read_text()
    return {
        "// @@NET_PACKET_TYPE_BODY@@": definition(
            source, r"int\s+Net::send_packet_type\s*\([^)]*\)"),
        "// @@NET_SEND_BODY@@": definition(
            source, r"int\s+Net::send\s*\([^)]*\)"),
    }


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_sender_peer_state_wire_types_and_callback_order(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    bodies = actual_bodies()
    for body in bodies.values():
        assert "PENDING_BODY" not in body and "BODY GOES HERE" not in body
        assert body.count("? 0 : players_[name_index].name_and_tail_") == 2
        assert "name_index == 16) name_index = 0" not in body

    def run(directory, name, replacements, expected):
        text = FIXTURE
        for token, body in replacements.items():
            assert text.count(token) == 1
            text = text.replace(token, body)
        cpp, executable = directory / (name + ".cpp"), directory / name
        cpp.write_text(text)
        built = subprocess.run(
            [compiler, "-std=c++98", "-pedantic", "-Wall", "-Wextra", "-Werror",
             str(cpp), "-o", str(executable)], capture_output=True, text=True,
            timeout=45)
        assert built.returncode == 0, built.stdout + built.stderr
        result = subprocess.run([str(executable)], capture_output=True,
                                text=True, timeout=15)
        assert (result.returncode == 0) == expected, result.stdout + result.stderr

    with tempfile.TemporaryDirectory(prefix="net-transport-actual-") as temp:
        directory = Path(temp)
        run(directory, "actual", bodies, True)
        token = "// @@NET_SEND_BODY@@"
        body = bodies[token]
        mutants = {
            "wrong_direct_wire_word": body.replace(
                "const uint16_t packet_type = 1;", "const uint16_t packet_type = 4;"),
            "unsigned_size_comparison": body.replace(
                "data_size > static_cast<int>(field_6E4_)",
                "static_cast<unsigned int>(data_size) > field_6E4_"),
            "wrong_reset_event_order": body.replace(
                "ResetEvent(thread_info_.events_[1]);", "ResetEvent(thread_info_.events_[0]);"),
        }
        for name, mutant in mutants.items():
            assert mutant != body, name
            run(directory, name, dict(bodies, **{token: mutant}), False)
