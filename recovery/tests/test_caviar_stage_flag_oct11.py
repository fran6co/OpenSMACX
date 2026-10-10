"""Pinned record-flag recovery controls; not complete renderer runtime proof."""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.source import read_text
from test_caviar_shape_chain_oct11 import assert_caviar_owner_continuity
from test_caviar_scene_product_source import FUNCTIONS, FIXTURE, extract_definition

ROOT = Path(__file__).resolve().parents[2]
BASE = "3e9c57dbfe23c1163ef50a4f6c610609b52603bb"


def test_original_identity_claims_symbols_and_history_preserved():
    path = ROOT / "src/caviar.cpp"
    old = subprocess.check_output(["git", "show", BASE + ":src/caviar.cpp"],
                                  cwd=ROOT, text=True)
    assert_caviar_owner_continuity(read_text(old, path))


@pytest.mark.parametrize("damage", ("original_pointer_test", "polarity", "unmasked_flag"))
def test_flag_provenance_and_polarity_damage_is_rejected(damage):
    source = (ROOT / "src/caviar.cpp").read_text()
    actual = extract_definition(source, "vox_stage_scene")
    if damage == "original_pointer_test":
        old = subprocess.check_output(["git", "show", BASE + ":src/caviar.cpp"],
                                      cwd=ROOT, text=True)
        damaged = extract_definition(old, "vox_stage_scene")
    else:
        old = "(record->setup_id_ & 1) == 0"
        assert actual.count(old) == 2
        new = "(record->setup_id_ & 1) != 0" if damage == "polarity" else "record->setup_id_ == 0"
        damaged = actual.replace(old, new)
    source = source.replace(actual, damaged, 1)
    extracted = "\n\n".join(extract_definition(source, name) for name in FUNCTIONS)
    program = FIXTURE.read_text().replace("/* PRODUCT_EXTRACTS */", extracted)
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    assert any(compilers), "an actual-source host compiler is required"
    with tempfile.TemporaryDirectory(prefix="caviar-stage-flag-") as directory:
        cpp = Path(directory) / "control.cpp"
        cpp.write_text(program)
        for compiler in filter(None, compilers):
            exe = Path(directory) / Path(compiler).name.replace("+", "p")
            subprocess.run([compiler, "-std=c++98", "-O2", str(cpp), "-o", str(exe)],
                           check=True, capture_output=True, timeout=60)
            result = subprocess.run([str(exe)], capture_output=True, timeout=30)
            assert result.returncode != 0, "damaged flag provenance/polarity passed"
