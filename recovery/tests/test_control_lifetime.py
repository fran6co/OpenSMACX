"""C++98 host controls over actual product destructor definitions.

This is a behavioral projection only. It does not model or prove VC6 ABI,
target layout, table writes, or original binary equivalence.
"""
import importlib.util
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

HERE = Path(__file__).resolve().parent
ROOT = next(parent for parent in HERE.parents
            if (parent / "src/game.cpp").is_file())
FIXTURE = (HERE / "control_lifetime_control.cpp").read_text()
for owner, header in (("ListBox", "listbox"), ("CheckBox", "jackal/checkbox"),
                      ("EditGroup", "editgroup")):
    declaration = re.search(r"class\s+" + owner + r"\s*:[^{]+\{",
                            (ROOT / ("src/" + header + ".h")).read_text())
    assert declaration, "actual product virtual-base declaration missing"
    FIXTURE, count = re.subn(r"class\s+" + owner + r"\s*:[^{]+\{",
                            lambda _: declaration.group(), FIXTURE, count=1)
    assert count == 1
LEXER_PATH = ROOT / "recovery/tests/test_pact_ascent.py"
SPEC = importlib.util.spec_from_file_location("actual_source_lexer", LEXER_PATH)
LEXER = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(LEXER)


def actual_bodies():
    return {
        "listbox": LEXER.definition(
            (ROOT / "src/listbox.cpp").read_text(),
            r"ListBox\s*::~\s*ListBox\s*\(\s*\)"),
        "checkbox": LEXER.definition(
            (ROOT / "src/jackal/checkbox.cpp").read_text(),
            r"CheckBox\s*::~\s*CheckBox\s*\(\s*\)"),
        "editgroup": LEXER.definition(
            (ROOT / "src/editgroup.cpp").read_text(),
            r"EditGroup\s*::~\s*EditGroup\s*\(\s*\)"),
    }


def compile_control(directory, bodies, compiler):
    source = FIXTURE
    for key in ("listbox", "checkbox", "editgroup"):
        token = "// @@" + key.upper() + "_DESTRUCTOR@@"
        assert token in source
        source = source.replace(token, bodies[key], 1)
    assert "@@" not in source
    cpp = directory / "control_lifetime_control.cpp"
    exe = directory / "destructor_lifetime_control"
    cpp.write_text(source)
    result = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=45)
    assert result.returncode == 0, result.stdout + result.stderr
    return exe


def run_control(exe, expect_success):
    result = subprocess.run([str(exe)], capture_output=True, text=True, timeout=15)
    if expect_success:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "destructor lifecycle mutant escaped controls"


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_product_destructors_and_damage_controls(compiler_name):
    source_bodies = actual_bodies()
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " host compiler unavailable")
    compilers = [compiler]

    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="generated-dtor-control-") as temp:
            directory = Path(temp)
            run_control(compile_control(directory, source_bodies, compiler), True)

            mutants = []
            removed_edit_close = dict(source_bodies)
            removed_edit_close["editgroup"] = re.sub(
                r"EditGroup::~EditGroup\(\)\s*\{\s*close\(\);\s*\}",
                "EditGroup::~EditGroup() {}", removed_edit_close["editgroup"], count=1)
            assert removed_edit_close["editgroup"] != source_bodies["editgroup"]
            mutants.append(removed_edit_close)

            removed_checkbox_reset = dict(source_bodies)
            removed_checkbox_reset["checkbox"] = re.sub(
                r"field_4_\s*=\s*0;\s*key_notification_callback_\s*=\s*0;\s*double_click_callback_\s*=\s*0;",
                "/* reset omitted */", removed_checkbox_reset["checkbox"], count=1)
            assert removed_checkbox_reset["checkbox"] != source_bodies["checkbox"]
            mutants.append(removed_checkbox_reset)

            swapped_checkbox_close = dict(source_bodies)
            swapped_checkbox_close["checkbox"] = re.sub(
                r"Dialog::close\(\);\s*GraphicWin::close\(\);",
                "GraphicWin::close();\n    Dialog::close();",
                swapped_checkbox_close["checkbox"], count=1)
            assert swapped_checkbox_close["checkbox"] != source_bodies["checkbox"]
            mutants.append(swapped_checkbox_close)

            swapped_virtual_bases = dict(source_bodies)
            fixture = FIXTURE.replace(
                "public virtual GraphicWin, public virtual Dialog",
                "public virtual Dialog, public virtual GraphicWin")
            source = fixture
            for key in ("listbox", "checkbox", "editgroup"):
                source = source.replace("// @@" + key.upper() + "_DESTRUCTOR@@",
                                        source_bodies[key], 1)
            cpp = directory / "swapped_virtual_bases.cpp"
            exe = directory / "swapped_virtual_bases"
            cpp.write_text(source)
            result = subprocess.run(
                [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
                 str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=45)
            assert result.returncode == 0, result.stdout + result.stderr
            run_control(exe, False)

            for mutant in mutants:
                run_control(compile_control(directory, mutant, compiler), False)
