"""Execute actual ListBox/Dialogs bodies with host doubles, not target ABI proof."""
from __future__ import annotations

import shutil
import subprocess
from pathlib import Path

import pytest
from match_recovery.source import Mode, read_text


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/listbox.cpp").is_file())
SOURCE_SIGNATURES = (
    ("src/listbox.cpp", "void ListBox::on_scrolling(int a, int b)", 0x60C5D0,
     (0x60C5D0, 0x60C691)),
    ("src/listbox.cpp", "void ListBox::draw_item(int index)", 0x60B530,
     (0x60B530, 0x60C1D8)),
    ("src/dialogs.cpp", "void Dialogs::on_scrolled(int code, int pos)", 0x612F80,
     (0x612F80, 0x612FA0)),
    ("src/dialogs.cpp", "void Dialogs::on_scrolling(int a, int b)", 0x612FA0,
     (0x612FA0, 0x612FC0)),
)


def extract_function(text: str, signature: str) -> str:
    if text.count(signature) != 1:
        raise AssertionError(f"expected one current definition: {signature}")
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and nxt == "/":
            state = "line"
            index += 1
        elif char == "/" and nxt == "*":
            state = "block"
            index += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return text[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated function: {signature}")


def actual_bodies() -> str:
    bodies = []
    by_path = {}
    for relative, signature, address, span in SOURCE_SIGNATURES:
        source_path = ROOT / relative
        text = by_path.setdefault(relative, source_path.read_text(encoding="utf-8"))
        rows = [row for row in read_text(text, source_path) if row.address == address]
        assert len(rows) == 1
        assert rows[0].mode is Mode.BODY
        assert signature in rows[0].region
        assert rows[0].image_spans == (span,)
        bodies.append(extract_function(text, signature))
    return "\n\n".join(bodies)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_extracted_listbox_scroll_render_and_dialog_bridges(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"host {compiler_name} unavailable")

    (tmp_path / "actual_listbox_rendering.inc").write_text(actual_bodies())
    fixture = Path(__file__).with_name("listbox_rendering_host_control.cpp.in")
    source = tmp_path / "listbox_rendering_actual_host_control.cpp"
    source.write_text(fixture.read_text(encoding="utf-8"))
    binary = tmp_path / "listbox_rendering_host_control"
    subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                    "-D__cdecl=", "-I", str(tmp_path), str(source), "-o", str(binary)],
                   check=True, timeout=30)
    subprocess.run([str(binary)], check=True, timeout=30)
