import os
from pathlib import Path
import shutil
import subprocess
import tempfile


SOURCE_ROOT = Path(os.environ.get(
    "CHECK_TAMPER_SOURCE_ROOT",
    Path(__file__).resolve().parents[2] / "src",
))
FIXTURE = Path(__file__).with_name("check_tamper_host_control.cpp")


def extract_function(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start : pos + 1]
    raise AssertionError(f"unterminated function: {signature}")


def test_check_tamper_actual_body_with_host_registry_doubles():
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        import pytest
        pytest.skip("host C++ compiler unavailable")

    source = (SOURCE_ROOT / "game.cpp").read_text()
    body = extract_function(source, "void __cdecl check_tamper()")
    fixture = FIXTURE.read_text()
    assert fixture.count("/*CHECK_TAMPER_BODY*/") == 1
    header = (SOURCE_ROOT / "game.h").read_text()
    start = header.index("struct TamperRecord {")
    end = header.index("};", start) + 2
    record = header[start:end]
    assert fixture.count("/* ACTUAL_TAMPER_RECORD */") == 1
    translation_unit = fixture.replace("/*CHECK_TAMPER_BODY*/", body).replace("/* ACTUAL_TAMPER_RECORD */", record)

    with tempfile.TemporaryDirectory(prefix="check-tamper-host-") as temp:
        cpp = Path(temp) / "check_tamper.cpp"
        exe = Path(temp) / "check_tamper"
        cpp.write_text(translation_unit)
        compile_result = subprocess.run(
            [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True, text=True, timeout=60,
        )
        assert compile_result.returncode == 0, compile_result.stderr
        run_result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
        assert run_result.returncode == 0, run_result.stdout + run_result.stderr
        assert "actual-body host controls: PASS" in run_result.stdout
