"""Execute actual ListBox/Dialogs bodies with host doubles, not target ABI proof."""
from __future__ import annotations

import shutil
import subprocess
import re
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

    bodies = actual_bodies()
    fixture = Path(__file__).with_name("listbox_rendering_host_control.cpp.in")
    fixture_text = fixture.read_text(encoding="utf-8")

    def compile_control(directory: Path, body: str) -> Path:
        directory.mkdir()
        (directory / "actual_listbox_rendering.inc").write_text(body)
        source = directory / "listbox_rendering_actual_host_control.cpp"
        source.write_text(fixture_text)
        binary = directory / "listbox_rendering_host_control"
        subprocess.run([compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror",
                        "-D__cdecl=", "-I", str(directory), str(source), "-o", str(binary)],
                       check=True, timeout=30)
        return binary

    good = compile_control(tmp_path / "good", bodies)
    subprocess.run([str(good)], check=True, timeout=30)

    signature = "void ListBox::draw_item(int index)"
    draw_body = extract_function(bodies, signature)

    def replace_draw(mutated: str) -> str:
        assert bodies.count(draw_body) == 1
        return bodies.replace(draw_body, mutated, 1)

    # Each negative control changes one original-supported rendering rule;
    # every mutant must compile before its behavioral assertion can fail.
    transpose = re.sub(
        r"(row\.left\s*=\s*)([^;]+)(;\s*row\.top\s*=\s*)([^;]+)(;)",
        lambda match: match.group(1) + match.group(4) + match.group(3)
        + match.group(2) + match.group(5), draw_body, count=1)
    assert transpose != draw_body
    assert "field_38_" in re.search(r"row\.left\s*=\s*([^;]+);", transpose).group(1)

    wrong_grid = draw_body.replace("row.left += along;", "row.top += along;", 1)
    wrong_grid = wrong_grid.replace("row.top += across;", "row.left += across;", 1)
    assert wrong_grid != draw_body

    wrong_span, span_count = re.subn(
        r"(row\.bottom\s*=\s*[^;]*\bspan\b[^;]*\+\s*)row\.top(\s*;)",
        r"\1row.left\2", draw_body, count=1, flags=re.DOTALL)
    assert span_count == 1 and wrong_span != draw_body

    for name, mutant in (("initial_transpose", transpose),
                         ("grid_wrong_edges", wrong_grid),
                         ("span_wrong_origin", wrong_span)):
        binary = compile_control(tmp_path / name, replace_draw(mutant))
        result = subprocess.run([str(binary)], check=False, timeout=30,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        assert result.returncode != 0, f"{name} mutant survived the asymmetric source control"

    wrong_focus = draw_body.replace(
        "window->is_dialog_focus() != 0",
        "window->is_dialog_focus() == 0", 1)
    assert wrong_focus != draw_body
    binary = compile_control(tmp_path / "wrong_focus", replace_draw(wrong_focus))
    result = subprocess.run([str(binary)], check=False, timeout=30,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert result.returncode != 0, "wrong selected-row focus mutant survived"

    wrong_fill_owner = draw_body.replace(
        "window->fill(&row, static_cast<int>(field_14_));",
        "canvas->fill(&row, static_cast<int>(field_14_));", 1)
    assert wrong_fill_owner != draw_body
    binary = compile_control(tmp_path / "wrong_fill_owner",
                             replace_draw(wrong_fill_owner))
    result = subprocess.run([str(binary)], check=False, timeout=30,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert result.returncode != 0, "Buffer fill receiver mutant survived"

    missing_live_guard = draw_body.replace(
        "index <= items.entry_count_ - 1",
        "index <= items.entry_count_ - 1 || items.entry_count_ <= index", 1)
    assert missing_live_guard != draw_body
    binary = compile_control(tmp_path / "missing_live_guard",
                             replace_draw(missing_live_guard))
    result = subprocess.run([str(binary)], check=False, timeout=30,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    assert result.returncode != 0, "unconditional post-callback reseek mutant survived"
