"""Host controls that compile the currently installed timer callback body."""
import os
from pathlib import Path
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/maininterface.cpp"
FIXTURE = Path(__file__).with_name("timer_callback_host.cpp")
BUILD_TMP = Path(__file__).with_name("tmp")


def extract_callback(source):
    signature = "void __cdecl timer_callback_daemon(int a1, int a2)"
    start = source.find(signature)
    assert start >= 0, "current free callback definition missing"
    brace = source.find("{", start + len(signature))
    assert brace >= 0
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated callback definition")


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_current_timer_callback_actual_source(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    BUILD_TMP.mkdir(parents=True, exist_ok=True)
    fixture = FIXTURE.read_text(encoding="utf-8")
    callback = extract_callback(SOURCE.read_text(encoding="utf-8"))
    assert fixture.count("/* CALLBACK_SOURCE */") == 1
    unit = fixture.replace("/* CALLBACK_SOURCE */", callback)
    cpp = tmp_path / "timer_callback.cpp"
    exe = tmp_path / "timer_callback"
    cpp.write_text(unit, encoding="utf-8")
    env = os.environ.copy()
    env["TMPDIR"] = str(BUILD_TMP)
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-omit-frame-pointer",
         str(cpp), "-o", str(exe)],
        cwd=ROOT, env=env, capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    run_env = env.copy()
    # LeakSanitizer is incompatible with this sandbox's ptrace restrictions.
    run_env["ASAN_OPTIONS"] = "detect_leaks=0"
    ran = subprocess.run([str(exe)], cwd=ROOT, env=run_env,
                         capture_output=True, text=True, timeout=60)
    assert ran.returncode == 0, ran.stdout + ran.stderr
