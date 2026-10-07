"""Host-only controls extracting the installed ListBox::attach definition."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS


ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/listbox.cpp").is_file())
SOURCE = ROOT / "src/listbox.cpp"
FIXTURE = Path(__file__).with_name("listbox_attach_host_control.cpp")
SIGNATURE = "int ListBox::attach(GraphicWin *a1, int a2, int a3, int a4)"
INSERTION = "// ACTUAL_ATTACH_INSERTION"


def extract_method(source: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    assert masked.count(SIGNATURE) == 1, "expected one installed attach method"
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start + len(SIGNATURE))
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated ListBox::attach body")


def actual_fixture() -> str:
    source = SOURCE.read_text(encoding="utf-8")
    method = extract_method(source)
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(INSERTION) == 1
    assert "GraphicWin &graphic = *this;" in method
    assert "Dialog &dialog = *this;" in method
    assert "Buffer &buffer = graphic;" in method
    assert "self - 0xA60" not in method
    assert "vbt[" not in method
    # The pinned update path calls ListBox's concrete override, not a
    # dynamically selected override belonging to a further-derived owner.
    assert "this->ListBox::attach(" in source
    return fixture.replace(INSERTION, method, 1)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_installed_attach_guards_fallback_scroll_and_receiver_dispatch(
        compiler_name: str) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = actual_fixture()
    with tempfile.TemporaryDirectory(prefix="listbox-attach-host-") as tmp_name:
        tmp = Path(tmp_name)
        cpp = tmp / "attach.cpp"
        exe = tmp / "attach"
        cpp.write_text(source, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Wno-unused-parameter",
             "-Wno-unused-function", str(cpp), "-o", str(exe)],
            cwd=tmp, capture_output=True, text=True, timeout=60,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], cwd=tmp, capture_output=True,
                             text=True, timeout=30)
        assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("old,new", (
    ("return 3;", "return 7;"),
    ("dialog.field_5C_ = dialog.field_2C_;",
     "dialog.field_5C_ = dialog.field_30_;"),
    ("static_cast<int32_t>(field_20_) > static_cast<int32_t>(field_1C_)",
     "field_20_ > field_1C_"),
))
def test_attach_controls_reject_behavior_damage(tmp_path, old, new):
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    source = actual_fixture()
    assert old in source
    damaged = source.replace(old, new)
    cpp = tmp_path / "damaged.cpp"
    exe = tmp_path / "damaged"
    cpp.write_text(damaged, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=60,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], cwd=tmp_path, capture_output=True,
                         text=True, timeout=30)
    assert ran.returncode != 0, "damaged behavior escaped the actual-body control"
