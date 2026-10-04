"""Actual DDInit source lifecycle under controlled host API substitutes, not COM ABI proof."""
from pathlib import Path
import shutil
import subprocess
import re
import pytest
from recovery.tests.test_pact_ascent import ROOT, definition


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_ddinit_lifetime(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    header = (ROOT / "src/win.h").read_text()
    start = header.index("class DDInit { public:")
    declaration = header[start:header.index("};", start) + 2]
    source = (ROOT / "src/win.cpp").read_text()
    global_definition = re.search(r"^DDInit WinDisplayInit;[^\n]*", source, re.MULTILINE)
    assert global_definition is not None
    bodies = [definition(source, p) for p in (
        r"DDInit::DDInit\(\)", r"DDInit::~DDInit\(\)",
        r"int DDInit::init_default_mode\([^)]*\)", r"void DDInit::teardown\(\)")]
    mutations = {
        **{"missing_" + field: (0, field + " = 0;", "(void)" + field + ";")
           for field in ("hwnd_", "surf_", "field_8_", "locked_surface_",
                         "locked_bits_", "unknown_18_", "unknown_1C_")},
        "clears_pitch": (0, "hwnd_ = 0;", "hwnd_ = 0; pitch_ = 0;"),
        "no_teardown": (1, "teardown();", "(void)0;"),
        "wrong_mode": (2, "init(0, 0, depth, 1)", "init(1, 0, depth, 1)"),
        "wrong_unlock": (3, "Unlock(locked_bits_)", "Unlock(0)"),
        "keeps_bits": (3, "locked_bits_ = 0;", "(void)locked_bits_;"),
        "no_restore": (3, "surf_->RestoreDisplayMode();", "(void)surf_;"),
        "no_release": (3, "surf_->Release();", "(void)surf_;"),
        "no_unregister": (3, "UnregisterClassA(DDInitClassName, WinInstance);", "(void)0;"),
        "keeps_window": (3, "hwnd_ = 0;", "(void)hwnd_;"),
        "releases_secondary": (3, "locked_surface_ = 0;", "if (locked_surface_) locked_surface_->Release(); locked_surface_ = 0;"),
        "no_metrics": (3, "DDInitRefreshScreenMetrics();", "(void)0;"),
    }
    fixture = Path(__file__).with_name("ddinit_lifetime_control.cpp").read_text()
    for name in ("actual", *mutations):
        selected = list(bodies)
        if name != "actual":
            index, before, after = mutations[name]
            assert selected[index].count(before) == 1, name
            selected[index] = selected[index].replace(before, after, 1)
        cpp, binary = tmp_path / (name + ".cpp"), tmp_path / name
        cpp.write_text(fixture.replace("// @@DECLARATION@@", declaration)
                       .replace("// @@BODIES@@", "\n".join(selected))
                       .replace("// @@GLOBAL@@", global_definition.group()))
        build = subprocess.run([compiler, "-std=c++98", "-fsanitize=undefined",
                                "-fno-sanitize-recover=all", str(cpp), "-o", str(binary)],
                               capture_output=True, text=True, timeout=45)
        assert build.returncode == 0, name + build.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
        assert (run.returncode == 0) == (name == "actual"), name + run.stderr
