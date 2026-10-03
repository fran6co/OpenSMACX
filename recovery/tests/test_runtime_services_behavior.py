"""Actual source controls; host doubles are not native ABI or game proof."""
import os
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


from match_recovery.source import read_file
from match_recovery.source.reader import _extract_forward_text

ROOT = Path(__file__).resolve().parents[2]
FIXTURE_FILE = Path(__file__).with_name("runtime_services") / "control.cpp"
SUBJECTS = [('src/strings.cpp', 5920944), ('src/netdaemon.cpp', 5370560), ('src/faction.cpp', 5370368), ('src/game.cpp', 5365904), ('src/sound_loader.cpp', 5004688), ('src/game.cpp', 5829200)]

def actual_bodies():
    bodies = []
    for source, address in SUBJECTS:
        path = ROOT / source
        record = next(r for r in read_file(path) if r.address == address)
        body = _extract_forward_text(path.read_text().splitlines(), record.line, record.name)
        assert "BODY GOES HERE." not in body and "PENDING_BODY(" not in body
        bodies.append(body)
    return "\n\n".join(bodies)


def materialize(bodies: str, fixture: str, directory: Path) -> Path:
    assert fixture.count("/* CANDIDATE_BODIES */") == 1
    source = fixture.replace("/* CANDIDATE_BODIES */", bodies)
    path = directory / "runtime_service_control.cpp"
    path.write_text(source)
    return path


@pytest.fixture(params=["g++", "clang++"])
def compiler(request):
    path = shutil.which(request.param)
    if path is None:
        pytest.skip(f"{request.param} is unavailable")
    return path


def run_case(binary: Path, case: str, expected_success=True):
    result = subprocess.run(
        [str(binary), case], capture_output=True, text=True, timeout=20,
        env={**os.environ, "UBSAN_OPTIONS": "halt_on_error=1"},
    )
    if expected_success:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, f"mutated candidate unexpectedly passed {case}"


def build(compiler: str, source: Path, out: Path):
    result = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-fsanitize=undefined",
         "-fno-sanitize-recover=all", str(source), "-o", str(out)],
        capture_output=True,
        text=True,
        timeout=60,
    )
    assert result.returncode == 0, result.stdout + result.stderr


def test_six_extracted_bodies_and_mutation_controls(compiler, tmp_path):
    fixture = FIXTURE_FILE.read_text()
    bodies = actual_bodies()
    source = materialize(bodies, fixture, tmp_path)
    binary = tmp_path / "control"
    build(compiler, source, binary)
    for case in [
        "space", "wait2", "waitloop", "tamper-ok", "tamper-fail",
        "release-guard", "release-full", "release-callback-clears-guard",
        "system-close", "waitloop-callback-reloads",
    ]:
        run_case(binary, case)

    mutant = bodies.replace(
        "NetDaemonNet.net_tasks();",
        'event("mutated-net-pump");',
        1,
    )
    assert mutant != bodies
    mutant_source = materialize(mutant, fixture, tmp_path)
    mutant_binary = tmp_path / "mutant"
    build(compiler, mutant_source, mutant_binary)
    run_case(mutant_binary, "wait2", expected_success=False)

    bad_tamper = bodies.replace(
        "if (status == ERROR_SUCCESS) {",
        "if (status != ERROR_SUCCESS) {",
        1,
    )
    assert bad_tamper != bodies
    bad_tamper_binary = tmp_path / "bad_tamper"
    build(compiler, materialize(bad_tamper, fixture, tmp_path), bad_tamper_binary)
    run_case(bad_tamper_binary, "tamper-ok", expected_success=False)

    bad_shutdown = bodies.replace("opening->show(1);", "opening->show(0);", 1)
    assert bad_shutdown != bodies
    bad_shutdown_binary = tmp_path / "bad_shutdown"
    build(compiler, materialize(bad_shutdown, fixture, tmp_path), bad_shutdown_binary)
    run_case(bad_shutdown_binary, "system-close", expected_success=False)

    stale_wait_loop = bodies.replace(
        "const int open_faction = ConsoleGlobal.faction_id_;",
        "const int open_faction = faction;",
        1,
    )
    assert stale_wait_loop != bodies
    stale_wait_loop_binary = tmp_path / "stale_wait_loop"
    build(compiler, materialize(stale_wait_loop, fixture, tmp_path),
          stale_wait_loop_binary)
    run_case(stale_wait_loop_binary, "waitloop-callback-reloads",
             expected_success=False)
