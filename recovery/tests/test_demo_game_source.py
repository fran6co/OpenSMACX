"""Bounded actual-source controls preserve native Demo quirks and dispatch."""
from pathlib import Path
import os
import subprocess
import tempfile



HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
SOURCE = Path(os.environ.get("DEMO_OWNER_SOURCE", str(ROOT / "src/game.cpp")))
FIXTURE = HERE / "fixtures/demo_game_source.cpp"


def extract(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        if text[end] == "{": depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0: return text[start:end + 1]
    raise AssertionError("unterminated source body: " + signature)


def test_tracked_game_cpp_demo_owner_bodies_compile_and_run():
    tracked = SOURCE.read_text()
    signatures = {
        "EXTRACT_SET_DEMO_RECT": "static void set_demo_rect(",
        "EXTRACT_DEMO_REDRAW": "void Demo::redraw()",
        "EXTRACT_DEMO_SELECT": "void Demo::on_left_down(",
        "EXTRACT_DEMO_TIMER": "void __cdecl Demo::timer_callback(",
        "EXTRACT_DEMO_INIT": "int Demo::init()",
    }
    missing = [signature for signature in signatures.values() if signature not in tracked]
    assert not missing, "product Demo owner lacks bodies: " + repr(missing)
    source = FIXTURE.read_text()
    for marker, signature in signatures.items():
        source = source.replace("/* " + marker + " */", extract(tracked, signature))
    with tempfile.TemporaryDirectory(prefix="opensmacx-demo-game-source-") as temp:
        cpp, exe = Path(temp) / "candidate.cpp", Path(temp) / "candidate"
        cpp.write_text(source)
        compilers = [os.environ.get("CXX", "g++")]
        if compilers[0] == "g++": compilers.append("clang++")
        for compiler in compilers:
            built = subprocess.run([compiler, "-std=c++98", "-O0", str(cpp), "-o", str(exe)],
                                   capture_output=True, text=True, timeout=60)
            assert built.returncode == 0, built.stdout + built.stderr
            ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
            assert ran.returncode == 0, (compiler, ran.stdout, ran.stderr)
