"""Actual-source controls for integrated PullDown/FlatButton bodies.

The fixture supplies dependency doubles; definitions come from the mapped
product bodies. These checks cover bounded drawing behavior, not native
layout, byte matching, linked preservation or full-game execution.
"""
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

import pytest
from match_recovery.source import Mode, read_file


HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[1]
FIXTURE = HERE / "renderer_owners_harness.cpp"
sys.path.insert(0, str(ROOT / "recovery/tests"))
from test_game_logic_seed_batch_behavior import _definition

RENDERERS = {
    "pulldown": ("src/pulldown.cpp", 0x005F98B0,
                 "void PullDown::on_redraw()"),
    "flatbutton": ("src/flatbutton.cpp", 0x00607E50,
                   "void FlatButton::on_redraw()"),
}


def actual_bodies():
    result = {}
    for key, (relative, address, signature) in RENDERERS.items():
        records = [record for record in read_file(ROOT / relative)
                   if record.address == address]
        assert len(records) == 1, "missing/duplicate mapped renderer " + key
        record, = records
        assert record.mode == Mode.BODY and record.region
        assert signature in record.region
        result[key] = _definition(record.region, signature)
    return result["pulldown"], result["flatbutton"]


def host_run(pull, flat):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("GCC or Clang host compiler is required")
    harness = FIXTURE.read_text()
    assert harness.count("// ACTUAL_PULLDOWN_BODY") == 1
    assert harness.count("// ACTUAL_FLATBUTTON_BODY") == 1
    # Product ABI is x86 and carries these pointers as 32-bit ints. The
    # host-only callback double ignores their value, so explicitly widen the
    # conversions for 64-bit hosts without changing product source.
    pull = pull.replace("reinterpret_cast<int>(this)",
                        "static_cast<int>(reinterpret_cast<uintptr_t>(this))")
    pull = pull.replace("reinterpret_cast<int>(name_)",
                        "static_cast<int>(reinterpret_cast<uintptr_t>(name_))")
    flat = flat.replace("reinterpret_cast<int>(this)",
                        "static_cast<int>(reinterpret_cast<uintptr_t>(this))")
    flat = flat.replace("reinterpret_cast<int>(name_)",
                        "static_cast<int>(reinterpret_cast<uintptr_t>(name_))")
    harness = harness.replace("// ACTUAL_PULLDOWN_BODY", pull)
    harness = harness.replace("// ACTUAL_FLATBUTTON_BODY", flat)
    with tempfile.TemporaryDirectory(prefix="late-ui-renderer-pytest-") as tmp:
        cpp = Path(tmp) / "renderer.cpp"
        exe = Path(tmp) / "renderer"
        cpp.write_text(harness)
        built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra",
                                str(cpp), "-o", str(exe)],
                               capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True)


def test_integrated_redraw_bodies_cover_valid_render_branches():
    pull, flat = actual_bodies()
    run = host_run(pull, flat)
    assert run.returncode == 0, run.stdout + run.stderr


@pytest.mark.parametrize(
    "which,old,new,reason",
    [
        ("pull", "if (index == selected_)", "if (index != selected_)",
         "selected-row mutation should fail"),
        ("flat", "if (center != 0)", "if (center == 0)",
         "center-sprite mutation should fail"),
        ("flat", "rect_right - 3 - ellipsis_width", "rect_right - ellipsis_width",
         "native right inset must not be lost"),
        ("flat", "clipped_text.right + clipped_width", "rect_right - 3",
         "second write translates the clipped rectangle, not a dots-only box"),
        ("flat", "{rect_left + 3, rect_top,", "{rect_left + 3, y,",
         "fallback keeps the native rectangle top, not the wrap baseline"),
    ],
)
def test_host_control_rejects_renderer_body_mutation(which, old, new, reason):
    pull, flat = actual_bodies()
    target = pull if which == "pull" else flat
    assert target.count(old) == 1, "integrated body anchor changed: " + reason
    target = target.replace(old, new, 1)
    run = host_run(target if which == "pull" else pull,
                   target if which == "flat" else flat)
    assert run.returncode != 0, reason
