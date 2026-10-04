"""C++98 host controls that compile the integrated Popup drawing body."""

from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name("popup_background_fixture.cpp")
SIGNATURE = "int Popup::on_draw_background() {"


def source_body(source: str) -> str:
    from test_game_logic_seed_batch_behavior import _definition
    return _definition(source, SIGNATURE[:-2])


def compile_and_run(body: str) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("a host C++ compiler is required")
    template = FIXTURE.read_text()
    assert template.count("// ACTUAL_SOURCE_BODY") == 1
    with tempfile.TemporaryDirectory() as temporary:
        cpp = Path(temporary) / "popup_background_fixture.cpp"
        executable = Path(temporary) / "popup_background_fixture"
        cpp.write_text(template.replace("// ACTUAL_SOURCE_BODY", body))
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(executable)],
            capture_output=True,
            text=True,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(executable)], capture_output=True, text=True)


def actual_body() -> str:
    return source_body((ROOT / "src" / "popup.cpp").read_text())


def test_actual_body_preserves_local_no_dialog_anchor_drawing() -> None:
    result = compile_and_run(actual_body())
    assert result.returncode == 0, result.stdout + result.stderr


def test_actual_body_keeps_diplomacy_buffer_for_final_border() -> None:
    result = compile_and_run(actual_body())
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize(
    "old,new,case",
    [
        (
            "if (type == 6 || type == 2) {\n      drawing_buffer->box_sprite(&title, &InterfaceStandardPopupBoxStyle);",
            "if (type == 6 && type == 2) {\n      drawing_buffer->box_sprite(&title, &InterfaceStandardPopupBoxStyle);",
            "wrong type branch must fail the local style assertion",
        ),
        (
            "TextColourTable[68]",
            "TextColourTable[180]",
            "wrong caption palette pair must fail",
        ),
        (
            "drawing_buffer->box_sprite(&outer, &InterfaceBoxStyles[3]);\n  return external_window_ == 0;",
            "static_cast<Buffer *>(this)->box_sprite(&outer, &InterfaceBoxStyles[3]);\n  return external_window_ == 0;",
            "wrong final receiver must fail the diplomacy control",
        ),
        (
            "outer.right = final_right + 3;",
            "outer.right = final_right + 2;",
            "wrong final border expansion must fail geometry assertion",
        ),
    ],
)
def test_host_control_rejects_body_mutation(old: str, new: str, case: str) -> None:
    body = actual_body()
    assert body.count(old) == 1, case
    result = compile_and_run(body.replace(old, new, 1))
    assert result.returncode != 0, case
