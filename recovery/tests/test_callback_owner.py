from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]

OWNERS = (
    "alphamovie", "basewin", "credits", "newtechwin", "prodpicker",
    "replaywin", "gamma", "basebutton",
)
PATTERN = re.compile(
    r"(?m)^(?:int|void)\s+(?:AlphaMovie::on_key_click|BaseWin::vslot_49|"
    r"Credits::on_key_click|Credits::on_left_click|"
    r"NewTechWin::on_button_clicked|ProdPicker::vslot_49|"
    r"ReplayWin::on_left_click|ReplayWin::on_right_click|"
    r"ReplayWin::on_button_clicked|ReplayWin::on_key_click|"
    r"Gamma::vslot_49|BaseButton::on_dialog_focus)"
    r"\s*\([^;{}]*\)\s*\{"
)

def extracted_sources():
    chunks = []
    for owner in OWNERS:
        text = (ROOT / "src" / (owner + ".cpp")).read_text()
        for match in PATTERN.finditer(text):
            opening = text.find("{", match.start(), match.end())
            depth = 0
            for index in range(opening, len(text)):
                if text[index] == "{": depth += 1
                elif text[index] == "}":
                    depth -= 1
                    if depth == 0:
                        chunks.append(text[match.start():index + 1])
                        break
            else:
                raise AssertionError(f"unterminated body in {owner}.cpp")
    assert len(chunks) == 12, f"expected 12 callback bodies, got {len(chunks)}"
    return "\n\n".join(chunks)

@pytest.fixture(params=("g++", "clang++"))
def compiler(request):
    path = shutil.which(request.param)
    if path is None:
        pytest.skip(request.param + " unavailable")
    return path


def run(tmp_path, source, compiler):
    cpp, exe = tmp_path / "callbacks.cpp", tmp_path / "callbacks"
    cpp.write_text(source)
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            "-iquote", str(Path(__file__).parent), str(cpp), "-o", str(exe)],
                           text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], text=True, capture_output=True, timeout=10)

def test_actual_owner_callback_bodies_dispatch_through_typed_slots(tmp_path, compiler):
    source = extracted_sources()
    controls = (Path(__file__).parent / "callback_owner_control.cpp").read_text()
    result = run(tmp_path, '#include "callback_owner_fixture.h"\n' + source + "\n" + controls, compiler)
    assert result.returncode == 0, result.stdout + result.stderr

def test_actual_alpha_movie_mutant_rejects_missing_modal_dispatch(tmp_path, compiler):
    source = extracted_sources()
    needle = "int AlphaMovie::on_key_click(int, int) {\n  release_modal();\n  return 1;\n}"
    mutant = source.replace(needle, "int AlphaMovie::on_key_click(int, int) { return 1; }", 1)
    assert mutant != source
    controls = (Path(__file__).parent / "callback_owner_control.cpp").read_text()
    result = run(tmp_path, '#include "callback_owner_fixture.h"\n' + mutant + "\n" + controls, compiler)
    assert result.returncode == 1

def test_actual_prod_picker_mutant_rejects_dropped_scroll_forwarding(tmp_path, compiler):
    source = extracted_sources()
    needle = "void ProdPicker::vslot_49(int code, int position) {\n  vslot_48(code, position);\n}"
    mutant = source.replace(needle, "void ProdPicker::vslot_49(int, int) {}", 1)
    assert mutant != source
    controls = (Path(__file__).parent / "callback_owner_control.cpp").read_text()
    result = run(tmp_path, '#include "callback_owner_fixture.h"\n' + mutant + "\n" + controls, compiler)
    assert result.returncode == 6
