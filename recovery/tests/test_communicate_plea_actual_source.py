"""Exercise the actual communicate plea fragment and preserve its call count.

This focused host test complements, rather than upgrades, the full owner
fixture. It extracts the active source expression; the helper compiles and
executes those exact statements against pinned arithmetic cases and mutants.
"""
from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/faction.cpp").exists())
SOURCE = ROOT / "src/faction.cpp"


def extract():
    text = SOURCE.read_text()
    row, = [r for r in read_file(SOURCE) if r.address == 0x54FFD0]
    assert row.name == "?communicate@@YAXHHH@Z"
    assert row.image_spans == ((0x54FFD0, 0x558980), (0x65FF4F, 0x65FF64))
    body = row.region
    gate_begin = body.index("if ((dialogue_flags & 0x40) == 0)")
    amount_at = body.index("communicate_plea_amount = ", gate_begin)
    amount_end = body.index("\nparse_num(0, communicate_plea_amount);", amount_at)
    gate = body[gate_begin:amount_at]
    amount = body[amount_at:amount_end]
    assert gate.count("wants_to_speak(faction_id, peer_id)") == 1
    assert "communicate_plea_amount" in amount
    return amount


def run_snippet(compiler, amount, tmp_path):
    source = r'''#include <climits>
#include <cstdio>
int amount_for(int communicate_peer_reserves) {
  int communicate_plea_amount;
  AMOUNT_BODY
  return communicate_plea_amount;
}
int main() {
  const int inputs[] = {10, 24, 25, 49, 50, 100, 400, INT_MAX};
  const int expected[] = {10, 24, 25, 25, 25, 50, 200, 1073741800};
  for (unsigned i = 0; i < sizeof(inputs) / sizeof(inputs[0]); ++i) {
    if (amount_for(inputs[i]) != expected[i]) return 2;
  }
  return 0;
}
'''.replace("AMOUNT_BODY", amount)
    cpp = tmp_path / "plea.cpp"
    exe = tmp_path / "plea"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++98", "-O1", "-fsanitize=undefined",
                            "-fno-sanitize-recover=all", str(cpp), "-o", str(exe)],
                           capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_plea_amount_and_single_speaker_call(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    amount = extract()
    run = run_snippet(compiler, amount, tmp_path)
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_plea_amount_mutation_is_detected(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip("host compiler unavailable")
    amount = extract()
    mutant = amount.replace(
        "communicate_peer_reserves / 50 * 25",
        "communicate_peer_reserves / 50")
    assert mutant != amount
    run = run_snippet(compiler, mutant, tmp_path)
    assert run.returncode != 0
