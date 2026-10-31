"""Actual two-argument tech_name behavior and its pinned out-of-line callers."""
from pathlib import Path
from hashlib import sha256
import os
import re
import shutil
import subprocess
import pytest
from match_recovery.source import read_file
ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def actual_body():
    text = (ROOT / "src/technology.cpp").read_text()
    start = text.index("LPSTR __cdecl tech_name(int tech_id, BOOL category_lvl) {")
    end = text.index("\n}", start) + 2
    return text[start:end]


def test_ordinary_body_owner_and_distinct_alpha_overload():
    header = (ROOT / "src/technology.h").read_text()
    assert "LPSTR __cdecl tech_name(int tech_id, BOOL category_lvl);" in header
    assert not re.search(r"tech_name\([^;]*\)\s*\{", header)
    record, = [r for r in read_file(ROOT / "src/technology.cpp") if r.address == 0x5B9EF0]
    assert record.byte_exact and not record.body
    alpha = (ROOT / "src/alpha.cpp").read_text()
    assert re.search(r"int\s+__cdecl\s+tech_name\(LPSTR\s+", alpha)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "missing-reset", "wrong-pointer"))
def test_actual_wrapper_resets_shared_storage_and_forwards_arguments(compiler, damage, tmp_path):
    if not shutil.which(compiler):
        pytest.skip(compiler + " unavailable")
    body = actual_body()
    if damage == "missing-reset":
        assert body.count("TechName[0] = 0;") == 1
        body = body.replace("TechName[0] = 0;", "")
    elif damage == "wrong-pointer":
        assert body.count("return TechName;") == 1
        body = body.replace("return TechName;", "return TechName + 1;")
    harness = r'''
#include <cassert>
#include <climits>
#include <cstring>
#define __cdecl
using LPSTR = char*;
using BOOL = int;
static char TechName[80];
static int expected_id, expected_category, calls;
static void say_tech(char *output, int id, int category) {
    assert(output == TechName && output[0] == 0);
    assert(id == expected_id && category == expected_category);
    ++calls; std::strcat(output, "observed");
}
''' + body + r'''
int main() {
    int ids[] = {INT_MIN, -1, 0, 88, INT_MAX};
    int categories[] = {INT_MIN, -1, 0, 1, 2, INT_MAX};
    for(int id : ids) for(int category : categories) {
        expected_id = id; expected_category = category;
        std::strcpy(TechName, "stale suffix");
        char *result = tech_name(id, category);
        assert(result == TechName && !std::strcmp(result, "observed"));
    }
    assert(calls == 30);
}
'''
    cpp = tmp_path / "wrapper.cpp"; exe = tmp_path / "wrapper"
    cpp.write_text(harness)
    result = subprocess.run([compiler, "-std=c++11", "-O2", "-Wall", "-Wextra", "-Werror",
                             str(cpp), "-o", str(exe)], text=True, capture_output=True)
    assert result.returncode == 0, result.stdout + result.stderr
    result = subprocess.run([str(exe)], text=True, capture_output=True)
    assert (result.returncode == 0) == (damage is None), result.stdout + result.stderr


def test_all_nine_pinned_two_argument_calls_remain_separate_from_alpha_parser():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = Path(os.environ.get("OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not path.is_file():
        pytest.skip("pinned image unavailable")
    raw = path.read_bytes(); assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN); decoder = Cs(CS_ARCH_X86, CS_MODE_32); decoder.detail = True
    for site in (0x42E0BF, 0x42E58A, 0x42EB78, 0x42F1BF, 0x431767,
                 0x42F638, 0x42FC71, 0x4301E5, 0x473ACF):
        call = next(decoder.disasm(image.read(Span(site, site + 5)), site))
        assert call.mnemonic == "call" and call.operands[0].imm == 0x5B9EF0
    alpha = [r for r in read_file(ROOT / "src/alpha.cpp")
             if "tech_name" in (r.symbol or r.name)]
    assert len(alpha) == 1 and alpha[0].address != 0x5B9EF0
