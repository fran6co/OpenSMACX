"""Exercise the actual workshop source, not a second recovered implementation."""
from pathlib import Path
import os
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_workshop_flows(compiler, tmp_path):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"host compiler unavailable: {compiler}")
    source = (ROOT / "src/designwin.cpp").read_text()
    helper_start = source.index("static int workshop_signed_bits(")
    helper_end = source.index("// ORIGINAL: 0x00583CD0", helper_start)
    body_start = source.index("int __cdecl design_workshop(int faction_id, int mode)")
    # The recovered workshop is the final definition in this owning TU.
    actual = source[helper_start:helper_end] + source[body_start:]
    harness = (Path(__file__).parent / "designwin_workshop_harness.cpp").read_text()
    anchor = "// ACTUAL_BODY_INSERTION_POINT"
    assert harness.count(anchor) == 1
    cpp = tmp_path / "workshop.cpp"
    cpp.write_text(harness.replace(anchor, actual))
    binary = tmp_path / "workshop"
    built = subprocess.run([executable, "-std=c++03", "-Wall", "-Wextra",
                            "-Werror", "-pedantic", "-fsanitize=address,undefined",
                            "-fno-pie", "-no-pie", str(cpp), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    environment = dict(os.environ, ASAN_OPTIONS="detect_leaks=0")
    # LSan cannot run under sandbox ptrace; ASan/UBSan remain enabled.
    ran = subprocess.run([str(binary)], env=environment,
                         capture_output=True, text=True)
    assert ran.returncode == 0, ran.stdout + ran.stderr
