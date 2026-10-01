"""Actual-source host controls for GraphicWin, SetupWin and SocialWin bodies.

The source annotations are read through match_recovery.source; each compiled
body is then extracted from the mapped product translation unit at test time.
This test is behavioral evidence only, not a layout or native acceptance gate.
"""
from pathlib import Path
import os
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
HARNESS_PATH = Path(__file__).with_name("ui_owner_wave_harness.cpp")


def actual_body(relative: str, address: int, signature: str) -> str:
    source_path = ROOT / relative
    records = [r for r in read_file(source_path) if r.address == address]
    assert len(records) == 1, (relative, hex(address), len(records))
    text = source_path.read_text()
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start : end + 1]
    raise AssertionError(f"unterminated actual body {signature}")


def actual_sources():
    return {
        "BODY_GRAPHIC": actual_body("src/graphicwin.cpp", 0x005D5500,
                                    "int GraphicWin::load_pcx("),
        "BODY_SETUP": actual_body("src/setupwin.cpp", 0x004AE340,
                                  "void SetupWin::on_left_click("),
        "BODY_EXIT": actual_body("src/socialwin.cpp", 0x004AEF10,
                                 "void SocialWin::exit("),
        "BODY_KEY": actual_body("src/socialwin.cpp", 0x004B1E80,
                                "int SocialWin::on_key_click("),
        "BODY_BUTTON": actual_body("src/socialwin.cpp", 0x004B1E00,
                                   "void SocialWin::on_button_clicked("),
    }


def run_host(tmp_path, parts):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    harness = HARNESS_PATH.read_text()
    for token, body in parts.items():
        assert token in harness
        harness = harness.replace(token, body)
    source, binary = tmp_path / "ui-owner-wave.cpp", tmp_path / "ui-owner-wave"
    source.write_text(harness)
    built = subprocess.run(
        [compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
         "-Wno-implicit-fallthrough", "-fsanitize=address,undefined",
         "-fno-sanitize-recover=all", str(source), "-o", str(binary)],
        capture_output=True, text=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True,
                          timeout=10,
                          env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"))


def test_actual_five_owner_bodies(tmp_path):
    result = run_host(tmp_path, actual_sources())
    assert result.returncode == 0, result.stderr


MUTANTS = [
    ("spot_mutated_y", "on_mouse_feedback(x, original_y)",
     "on_mouse_feedback(x, y)", "original feedback coordinate"),
    ("paid_cost_cached_receiver",
     "gross_upheaval - PlayersData[factionID_].soc_upheaval_cost_paid",
     "gross_upheaval - PlayersData[0].soc_upheaval_cost_paid",
     "paid-cost and prompt-energy loads"),
    ("energy_prompt_cached_receiver",
     "parse_num(1, PlayersData[factionID_].energy_reserves);",
     "parse_num(1, PlayersData[0].energy_reserves);",
     "paid-cost and prompt-energy loads"),
    ("commit_cached_receiver",
     "PlayersData[factionID_].soc_upheaval_cost_paid += upheaval;",
     "PlayersData[0].soc_upheaval_cost_paid += upheaval;",
     "popup faction reload selects commit receiver"),
    ("sync_receiver_cached",
     "synch_soc(factionID_);", "synch_soc(2);",
     "sync stages independently reload live faction"),
    ("pcx_guard_changed", "(iFlags_ & 0x800000) != 0",
     "(iFlags_ & 0x400000) != 0", "GraphicWin guard"),
]


@pytest.mark.parametrize("name,old,new,failure", MUTANTS,
                         ids=[m[0] for m in MUTANTS])
def test_actual_body_damage_controls(tmp_path, name, old, new, failure):
    parts = actual_sources()
    owner = {"spot_mutated_y": "BODY_SETUP",
             "paid_cost_cached_receiver": "BODY_EXIT",
             "energy_prompt_cached_receiver": "BODY_EXIT",
             "commit_cached_receiver": "BODY_EXIT",
             "sync_receiver_cached": "BODY_EXIT",
             "pcx_guard_changed": "BODY_GRAPHIC"}[name]
    assert old in parts[owner], name
    parts[owner] = parts[owner].replace(old, new, 1)
    if name == "spot_mutated_y":
        # Replace the saved incoming coordinate with the output cell itself;
        # remove its now-dead save so the mutant still compiles strictly.
        parts[owner] = parts[owner].replace("    const int original_y = y;\n", "", 1)
    result = run_host(tmp_path, parts)
    assert result.returncode != 0, name
    assert failure in result.stderr, (name, result.stderr)
