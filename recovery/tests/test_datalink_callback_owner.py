"""Actual-source callback controls; not native/game acceptance."""
from pathlib import Path
import shutil
import subprocess

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


@pytest.mark.parametrize("mutant", [
    None,
    ("help(value);", "help(value + 1);"),
    ("append_text(output, value, extra);", "append_text(output, extra, value);"),
    ("help(value);", "(void)value;"),
    ("help = help_callback;", "help = 0;"),
    ("append_text = text_callback;", "append_text = 0;"),
    ("help = help_callback;", "if (help_callback != 0) help = help_callback;"),
    ("append_text = text_callback;", "if (text_callback != 0) append_text = text_callback;"),
])
def test_actual_callback_row_methods(tmp_path, mutant):
    source_root = ROOT
    header = (source_root / "src/datalink.h").read_text()
    start = header.index("typedef void (__cdecl *DatalinkHelpCallback)")
    end = header.index("// Candidate lower-bound model", start)
    declarations = header[start:end]

    source = (source_root / "src/datalink.cpp").read_text()
    bodies = definition(
        source, r"void DatalinkEntryCallbacks::set_callbacks\([^)]*\)"
    ) + "\n" + definition(
        source, r"void DatalinkEntryCallbacks::invoke_help\([^)]*\) const"
    )
    bodies += "\n" + definition(
        source, r"void DatalinkEntryCallbacks::invoke_text\([^)]*\) const"
    )
    if mutant is not None:
        old, new = mutant
        assert bodies.count(old) == 1
        bodies = bodies.replace(old, new, 1)
    fixture = Path(__file__).with_name("datalink_callback_owner_control.cpp").read_text()
    actual = fixture.replace("// @@DECLARATIONS@@", declarations).replace(
        "// @@BODIES@@", bodies
    )

    available = [name for name in ("g++", "clang++") if shutil.which(name)]
    if not available:
        pytest.skip("no supported host C++ compiler")
    for compiler in available:
        cpp, binary = tmp_path / (compiler + ".cpp"), tmp_path / compiler
        cpp.write_text(actual)
        build = subprocess.run(
            [compiler, "-std=c++98", "-fsanitize=undefined", "-fno-sanitize-recover=all",
             str(cpp), "-o", str(binary)],
            capture_output=True, text=True, timeout=45,
        )
        assert build.returncode == 0, build.stdout + build.stderr
        run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
        if mutant is None:
            assert run.returncode == 0, run.stdout + run.stderr
        else:
            assert run.returncode != 0, "control did not reject altered callback dispatch"
