"""Exercise live retool order and pin the original's two field reads.

The bounded host harness checks body behavior, not native VC6 ABI/layout or
complete game behavior. The pinned instruction check is a separate witness.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source.asm import Listing
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
PINNED_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
DEFAULT_IMAGE = ROOT / ".opensmacx/game/terranx_original.exe"
START = 0x004E4810
END = 0x004E48B0
SIGNATURE = "int __cdecl base_lose_minerals("


def extract_definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


CPP_PREFIX = r'''#include <stdint.h>
#include <string.h>
#define __cdecl
#define UNUSED(name)
struct RulesState { uint32_t retool_pct_pen_prod_chg; uint32_t retool_exemption; };
struct BaseState {
  uint8_t faction_id_current;
  int minerals_accumulated_2;
  int production_id_last;
  int queue_production_id[10];
};
static RulesState Rules;
static BaseState Bases[2];
static int human_calls;
static int making_calls;
static int making_args[4];
static int mutate_queue_after_first_call;
static int is_human(uint8_t faction) { ++human_calls; return faction == 1; }
static int base_making(int production_id, int base_id) {
  making_args[making_calls] = production_id;
  ++making_calls;
  if (mutate_queue_after_first_call && making_calls == 1)
    Bases[base_id].queue_production_id[0] = 52;
  return production_id < 40 ? 1 : 2;
}
'''

CPP_MAIN = r'''
static void reset_case() {
  memset(&Rules, 0, sizeof(Rules));
  memset(Bases, 0, sizeof(Bases));
  human_calls = making_calls = 0;
  memset(making_args, 0, sizeof(making_args));
  mutate_queue_after_first_call = 0;
  Rules.retool_pct_pen_prod_chg = 50;
  Rules.retool_exemption = 20;
  Bases[0].faction_id_current = 1;
  Bases[0].minerals_accumulated_2 = 100;
  Bases[0].production_id_last = 31;
  Bases[0].queue_production_id[0] = 41;
}

int main() {
  // This injected oracle stub changes the queue on its first call; that tests
  // live read sequencing only and does not claim real base_making mutates it.
  // The second read must occur after the previous-item classification.
  reset_case(); mutate_queue_after_first_call = 1;
  if (base_lose_minerals(0, 999) != 40 || making_calls != 2
      || making_args[0] != 31 || making_args[1] != 52) return 1;

  // Equal production classifications do not lose minerals.
  reset_case(); Bases[0].queue_production_id[0] = 32;
  if (base_lose_minerals(0, 0) != 0 || making_calls != 2
      || making_args[0] != 31 || making_args[1] != 32) return 2;

  // Exemption is strict: equality and values below it are both free.
  reset_case(); Bases[0].minerals_accumulated_2 = 20;
  if (base_lose_minerals(0, 0) != 0 || making_calls != 2) return 3;
  reset_case(); Bases[0].minerals_accumulated_2 = 19;
  if (base_lose_minerals(0, 0) != 0 || making_calls != 2) return 4;

  // A non-human faction short-circuits both classifiers.
  reset_case(); Bases[0].faction_id_current = 2;
  if (base_lose_minerals(0, 0) != 0 || human_calls != 1 || making_calls != 0) return 5;

  // Zero percent short-circuits before touching an invalid base ID.
  reset_case(); Rules.retool_pct_pen_prod_chg = 0;
  if (base_lose_minerals(999, 0) != 0 || human_calls != 0 || making_calls != 0) return 6;
  // With a valid base it also must not evaluate is_human or either classifier.
  reset_case(); Rules.retool_pct_pen_prod_chg = 0;
  if (base_lose_minerals(0, 0) != 0 || human_calls != 0 || making_calls != 0) return 7;

  // Check the signed percentage arithmetic with bounded values above 100%.
  reset_case(); Rules.retool_pct_pen_prod_chg = 175;
  if (base_lose_minerals(0, 0) != 140) return 8;
  return 0;
}
'''


def actual_body() -> str:
    source = (ROOT / "src/base.cpp").read_text(encoding="utf-8")
    body = extract_definition(source, SIGNATURE)
    assert "BODY GOES HERE." not in body
    assert re.search(
        r"const\s+int\s+previous\s*=\s*base_making\(Bases\[base_id\]\.production_id_last,\s*base_id\);",
        body,
    ), "the previous production classification must be an explicit first call"
    assert re.search(
        r"const\s+int\s+queued\s*=\s*base_making\(Bases\[base_id\]\.queue_production_id\[0\],\s*base_id\);",
        body,
    ), "the queued production must be read live by an explicit second call"
    return body


def compile_and_run(tmp_path: Path, compiler: str, body: str, name: str):
    source_path = tmp_path / f"{name}.cpp"
    executable = tmp_path / name
    source_path.write_text(CPP_PREFIX + "\n" + body + "\n" + CPP_MAIN, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source_path), "-o", str(executable)],
        cwd=tmp_path,
        capture_output=True,
        text=True,
        check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, check=False)


def _replace_once(pattern: str, replacement: str, body: str) -> str:
    changed, count = re.subn(pattern, replacement, body, count=1)
    assert count == 1, f"mutation anchor missing: {pattern}"
    return changed


def mutations(body: str) -> dict[str, str]:
    previous_pattern = (
        r"const\s+int\s+previous\s*=\s*base_making\(Bases\[base_id\]\.production_id_last,\s*base_id\);"
    )
    queued_pattern = (
        r"const\s+int\s+queued\s*=\s*base_making\(Bases\[base_id\]\.queue_production_id\[0\],\s*base_id\);"
    )
    previous = re.search(previous_pattern, body)
    queued = re.search(queued_pattern, body)
    assert previous and queued and previous.start() < queued.start()

    reversed_calls = body[:previous.start()] + queued.group(0) + body[previous.end():queued.start()] + previous.group(0) + body[queued.end():]
    missing_previous = _replace_once(previous_pattern, "const int previous = 0;", body)
    missing_queued = _replace_once(queued_pattern, "const int queued = 0;", body)
    normalized_return = _replace_once(
        r"return\s+min_accum\s*-\s*\(100\s*-\s*\(int\)Rules\.retool_pct_pen_prod_chg\)\s*"
        r"\*\s*\(min_accum\s*-\s*\(int\)Rules\.retool_exemption\)\s*/\s*100\s*"
        r"-\s*\(int\)Rules\.retool_exemption\s*;",
        "return min_accum != 0;",
        body,
    )
    wrong_arithmetic = _replace_once(
        r"\*\s*\(min_accum\s*-\s*\(int\)Rules\.retool_exemption\)\s*/\s*100",
        "* (min_accum - (int)Rules.retool_exemption) / 10",
        body,
    )
    wrong_guard = _replace_once(r"&&\s*is_human\(", "|| is_human(", body)
    return {
        "reversed_call_order": reversed_calls,
        "missing_previous_call": missing_previous,
        "missing_queued_call": missing_queued,
        "normalized_result": normalized_return,
        "wrong_percentage_scale": wrong_arithmetic,
        "guard_or": wrong_guard,
    }


def test_actual_base_retool_body_and_compiling_mutants(tmp_path: Path) -> None:
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "one host C++ compiler is required for the focused behavior control"
    body = actual_body()
    for compiler in compilers:
        result = compile_and_run(tmp_path, compiler, body, f"base-retool-{Path(compiler).name}")
        assert result.returncode == 0, result.stdout + result.stderr
        for name, mutant in mutations(body).items():
            damaged = compile_and_run(
                tmp_path, compiler, mutant, f"{Path(compiler).name}-{name}"
            )
            assert damaged.returncode != 0, f"{compiler} let {name} escape"


def test_pinned_original_retool_field_order_and_extent() -> None:
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", DEFAULT_IMAGE))
    assert image_path.is_file(), f"pinned original image is required: {image_path}"
    raw = image_path.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PINNED_SHA256
    pin = image_pin(ROOT)
    assert pin == PINNED_SHA256
    listing = Listing(PeImage(raw, pin).read(Span(START, END)), START)
    assert sum(len(instruction.bytes) for instruction in listing.instructions) == END - START
    assert listing.instructions[-1].address + len(listing.instructions[-1].bytes) == END
    native = {instruction.address: instruction for instruction in listing.instructions}

    before_call = native[0x004E4847]
    first_call = native[0x004E4850]
    after_call = native[0x004E4855]
    second_call = native[0x004E485F]
    # Bases is at 0x97D040: the linked displacements include its address.
    assert before_call.mnemonic == "mov" and "0x97d084" in before_call.op_str.lower()
    assert first_call.mnemonic == "call" and "0x4e4700" in first_call.op_str.lower()
    assert after_call.mnemonic == "mov" and "0x97d090" in after_call.op_str.lower()
    assert second_call.mnemonic == "call" and "0x4e4700" in second_call.op_str.lower()
    assert before_call.address < first_call.address < after_call.address < second_call.address
