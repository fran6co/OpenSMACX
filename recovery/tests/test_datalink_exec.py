"""Actual-source host controls for Datalink::exec; not native/game proof."""
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


SOURCE = ROOT / "src" / "datalink.cpp"
FIXTURE = Path(__file__).with_name("datalink_exec_control.cpp").read_text()
SIGNATURE = r"void\s+Datalink::exec\s*\(\s*unsigned\s+int\s+topic\s*,\s*int\s+index\s*\)"
TOKEN = "// @@DATALINK_EXEC_DEFINITION@@"


def actual_definition():
    text = SOURCE.read_text(encoding="utf-8")
    body = definition(text, SIGNATURE)
    assert "PENDING_BODY" not in body
    assert "BODY GOES HERE" not in body
    return body


def compile_control(directory, body, compiler, name):
    assert FIXTURE.count(TOKEN) == 1
    source = FIXTURE.replace(TOKEN, body, 1)
    assert "@@DATALINK_EXEC_DEFINITION@@" not in source
    cpp = directory / (name + ".cpp")
    exe = directory / name
    cpp.write_text(source, encoding="utf-8")
    result = subprocess.run(
        [compiler, "-std=c++98", "-pedantic", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=45)
    assert result.returncode == 0, result.stdout + result.stderr
    return exe


def run_control(exe, should_pass):
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=15)
    if should_pass:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "Datalink::exec damage escaped behavior checks"


def replace_once(source, old, new):
    assert source.count(old) == 1, "expected exactly one mutation anchor: " + old
    return source.replace(old, new, 1)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_datalink_exec_callbacks_and_damage_controls(compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " host compiler unavailable")

    body = actual_definition()
    with tempfile.TemporaryDirectory(prefix="actual-datalink-exec-control-") as temp:
        directory = Path(temp)
        run_control(compile_control(directory, body, compiler, "actual"), True)

        mutants = {
            "missing_restore": replace_once(
                body,
                "ReportWinGlobal.shared_research_focus() = static_cast<int32_t>(field_A1C_);",
                "/* ReportWin focus restoration omitted */"),
            "wrong_focus_owner": body.replace(
                "ReportWinGlobal.shared_research_focus()", "DatalinkWrongFocus"),
            "wrong_selection": replace_once(
                body, "field_29DC_ * 10000 + index",
                "field_29DC_ * 1000 + index"),
            "missing_redraw": replace_once(body, "repaint_slot();",
                                            "/* redraw omitted */"),
            "missing_release": replace_once(body,
                                             "SubInterface::release_iface_mode();",
                                             "/* release omitted */"),
            "wrong_box_owner": replace_once(
                body, "static_cast<Win *>(this),\n                        0, -1",
                "static_cast<Win *>(&MainInterfaceGlobal),\n                        0, -1"),
        }
        assert mutants["wrong_focus_owner"] != body
        for name, mutant in mutants.items():
            run_control(compile_control(directory, mutant, compiler, name), False)
