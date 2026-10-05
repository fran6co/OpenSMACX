"""Compile branch controls from the balanced real product definition."""
import pathlib
import shutil
import subprocess

import pytest


ROOT = pathlib.Path(__file__).resolve().parents[2]
PRODUCT = ROOT / "src" / "stringstruct.cpp"
FIXTURE = pathlib.Path(__file__).with_name("tech_wizard_unk1_host_control.cpp")
SIGNATURE = 'extern "C" void __cdecl tech_wizard_unk1('
INSERTION = "// PRODUCT_BODY_INSERTED_BY_TEST"


def extract_body(text):
    start = text.find(SIGNATURE)
    assert start >= 0, "tech_wizard_unk1 product definition is missing"
    opening = text.find("{", start)
    assert opening >= 0
    depth = 0
    for position in range(opening, len(text)):
        if text[position] == "{":
            depth += 1
        elif text[position] == "}":
            depth -= 1
            if depth == 0:
                return text[start:position + 1]
    raise AssertionError("unbalanced actual source body")


def run(compiler, body, mode, tmp_path, expected):
    source_text = FIXTURE.read_text()
    assert source_text.count(INSERTION) == 1
    source = tmp_path / ("tech-wizard-" + mode + ".cpp")
    source.write_text(source_text.replace(INSERTION, body))
    binary = tmp_path / ("tech-wizard-" + mode)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(source), "-o", str(binary)], check=True, timeout=30)
    result = subprocess.run([str(binary), mode], cwd=tmp_path, timeout=30)
    assert (result.returncode == 0) is expected


def test_actual_product_body_and_mutants(tmp_path):
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    compilers = [compiler for compiler in compilers if compiler]
    if not compilers:
        pytest.skip("neither g++ nor clang++ is installed")
    body = extract_body(PRODUCT.read_text())
    modes = (
        "negative", "second-negative", "rank1", "rank0", "growth-ahead-tech",
        "equal-growth-wealth", "forced", "sum-zero", "same-tech",
        "upper-bound", "last-tech", "first-high", "occupied-sprite-list",
        "exec-results",
    )
    for compiler in compilers:
        for mode in modes:
            run(compiler, body, mode, tmp_path, True)
        mutants = (
            ("second-negative", body.replace(
                "first_tech < 0 || second_tech < 0", "first_tech < 0", 1)),
            ("rank1", body.replace("lookup_b < lookup_mid && lookup_a < lookup_mid",
                                    "lookup_b > lookup_mid && lookup_a < lookup_mid", 1)),
            ("occupied-sprite-list", body.replace("entry_count_ == 0",
                                                   "entry_count_ != 0", 1)),
            ("forced", body.replace("force_roll != 0 ? 3 :",
                                     "force_roll != 0 ? 2 :", 1)),
            ("last-tech", body.replace("second_tech < MaxTechnologyNum",
                                        "second_tech < MaxTechnologyNum - 1", 1)),
            ("growth-ahead-tech", body.replace(
                "lookup_mid < lookup_b && lookup_a < lookup_b",
                "lookup_b >= lookup_mid && lookup_mid > lookup_a", 1)),
        )
        for mode, mutant in mutants:
            assert mutant != body
            run(compiler, mutant, mode, tmp_path, False)
