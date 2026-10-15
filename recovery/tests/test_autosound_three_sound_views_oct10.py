"""Use-derived pointer views; no ownership or original type-spelling proof."""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_text
from match_recovery.tasks import TaskStore
from test_scroll_default_and_icon_owner_closure_oct9 import _definition
from test_texture_raster_actual_source import _continuity_fields

ROOT = Path(__file__).resolve().parents[2]
BASE = "38f3563ec3e23a6d1e0b692161999bfd036388a7"


def test_original_identity_claim_and_history_continuity():
    task = TaskStore(ROOT).read("listbox-autosound-parent-closure-oct13x")
    for filename in ("autosound.cpp", "win.cpp"):
        path = ROOT / "src" / filename
        prior = subprocess.check_output(["git", "show", BASE + ":src/" + filename],
                                        cwd=ROOT, text=True)
        historical = {r.address: r for r in read_text(prior, path)}
        task_prior = subprocess.check_output(
            ["git", "show", f"{task.base_revision}:src/{filename}"],
            cwd=ROOT, text=True)
        baseline = {r.address: r for r in read_text(task_prior, path)}
        after = {r.address: r for r in read_text(path.read_text(), path)}
        # Keep the older continuity baseline meaningful while allowing rows
        # already present in this task's immutable base. The task-base check
        # below is exact, so this is not an open-ended addition allowlist.
        assert set(historical) <= set(after)
        for address, row in historical.items():
            assert _continuity_fields(row) == _continuity_fields(after[address])
            assert row.symbol == after[address].symbol
        assert set(baseline) == set(after)
        for address, row in baseline.items():
            assert _continuity_fields(row) == _continuity_fields(after[address])
            assert row.symbol == after[address].symbol


@pytest.mark.parametrize("wrong_default", [False, True])
def test_actual_lifecycle_copies_independent_defaults_and_resets(tmp_path, wrong_default):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("host compiler unavailable")
    header = (ROOT / "src/autosound.h").read_text()
    defaults_header = (ROOT / "src/autosound_defaults.h").read_text()
    # The host exercises methods, not 32-bit layout; native controls own ABI.
    header = re.sub(r"^#pragma once.*$", "", header, flags=re.M)
    header = re.sub(r'static_assert\(sizeof\(AutoSound\) == 0x98,\s*"[^"]*"\);', "", header)
    text = (ROOT / "src/autosound.cpp").read_text()
    bodies = "\n".join(_definition(text, signature) for signature in (
        "AutoSound::AutoSound()", "void AutoSound::close()",
        "void AutoSound::close2()", "void AutoSound::init()"))
    if wrong_default:
        bodies = bodies.replace("AutoSoundDefaults.val_11_", "AutoSoundDefaults.val_10_")
    source = """
#include <stdint.h>
#include <cassert>
#define MEASURED
struct Sound {};
""" + header + "\n" + defaults_header + "\nAutoSoundDefaultsBlock AutoSoundDefaults = {};\n" + bodies + """
static Sound *expected_10, *expected_11, *expected_12;
struct Observed : AutoSound {
  void check_defaults() {
    assert(val_10_ == expected_10);
    assert(val_11_ == expected_11);
    assert(val_12_ == expected_12);
  }
  void check_zero() { assert(!val_10_ && !val_11_ && !val_12_); }
};
int main() {
  Sound sound_10, sound_11, sound_12;
  expected_10 = &sound_10; expected_11 = &sound_11; expected_12 = &sound_12;
  AutoSoundDefaults.val_10_=&sound_10;
  AutoSoundDefaults.val_11_=&sound_11;
  AutoSoundDefaults.val_12_=&sound_12;
  Observed record; record.check_defaults();
  record.init(); record.check_zero();
  record.close(); record.check_defaults();
  record.init(); record.close2(); record.check_defaults();
}
"""
    path, binary = tmp_path / "lifecycle.cpp", tmp_path / "lifecycle"
    path.write_text(source)
    result = subprocess.run([compiler, "-D__cdecl=", str(path), "-o", str(binary)],
                            capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    result = subprocess.run([str(binary)], capture_output=True, timeout=30)
    assert (result.returncode == 0) == (not wrong_default)


def test_only_supported_fields_change_view():
    header = (ROOT / "src/autosound.h").read_text()
    for field, offset in ((10, "28"), (11, "2C"), (12, "30")):
        assert f"Sound *val_{field}_;" in header
        assert f"RECOVERY_MEMBER: v3 AutoSound val_{field}_ Sound* 0x{offset}" in header
    assert "Sound *val_20_;" in header
    assert "RECOVERY_MEMBER: v3 AutoSound val_20_ Sound* 0x50 0x005F6550" in header
    assert "Sound *val_31_;" in header
    assert "RECOVERY_MEMBER: v3 AutoSound val_31_ Sound* 0x7C 0x00607A20" in header
    assert "Sound *legacy_sound_31() const { return val_31_; }" in header
    assert "legacy_value_31()" not in header
    defaults = (ROOT / "src/autosound.cpp").read_text()
    assert "AutoSoundDefaultsBlock AutoSoundDefaults;" in defaults
