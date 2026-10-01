"""Actual-product-source Pulldown regression coverage.

The C++ fixture is a host projection only: member bodies are extracted from
the mapped product TU at test time; fixture doubles expose effects and do not
copy product implementations.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/pulldown.cpp"
FIXTURE = Path(__file__).with_name("pulldown_owner_wave_harness.cpp")
METHODS = {
    "UNK1": (0x005F8970, "void PullDown::UNK1()"),
    "ON_MOUSE_MOVE": (0x005FA6B0, "void PullDown::on_mouse_move(int x, int y)"),
    "MOUSE_MOVE": (0x005F9590, "void PullDown::mouse_move(int, int y)"),
    "PULLDOWN_UPDATE": (0x005F9F60, "void PullDown::pulldown_update(int old_selected, int new_selected)"),
}


def braced(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[start:end + 1]
    raise AssertionError(f"unterminated actual product body: {signature}")


def actual_bodies() -> dict[str, str]:
    records = read_file(SOURCE)
    product = SOURCE.read_text()
    bodies = {}
    for key, (address, signature) in METHODS.items():
        matched, = [record for record in records if record.address == address]
        assert matched.state.value == "implemented", (key, matched.state)
        assert Path(matched.path) == SOURCE
        body = braced(product, signature)
        assert "BODY GOES HERE" not in body and len(body) > 100, key
        bodies[key] = body
    return bodies


def compile_and_run(tmp_path: Path, bodies: dict[str, str]):
    compiler = shutil.which("clang++") or shutil.which("g++")
    if not compiler:
        pytest.skip("host C++ compiler required")
    fixture = FIXTURE.read_text()
    marker = "// __ACTUAL_BODIES_SLOT__"
    assert fixture.count(marker) == 1
    injected = "\n\n".join(bodies[key] for key in METHODS)
    source = tmp_path / "pulldown-owner-wave.cpp"
    binary = tmp_path / "pulldown-owner-wave"
    source.write_text(fixture.replace(marker, injected))
    built = subprocess.run(
        [compiler, "-std=c++17", "-O1", "-Wall", "-Wextra", "-Werror",
         "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
         str(source), "-o", str(binary)],
        text=True, capture_output=True, timeout=30,
    )
    assert built.returncode == 0, built.stderr
    return subprocess.run(
        [str(binary)], text=True, capture_output=True, timeout=10,
        env=dict(os.environ, ASAN_OPTIONS="detect_leaks=0"),
    )


def test_actual_four_product_pulldown_bodies(tmp_path):
    result = compile_and_run(tmp_path, actual_bodies())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("case", [
    "old_visibility", "selected_callback_mode", "new_mnemonic_metric",
    "padding_reload", "old_expanded_fill", "initial_read_order",
    "rendered_text_reload", "mnemonic_metric_order",
])
def test_actual_pulldown_update_damage_controls(tmp_path, case):
    bodies = actual_bodies()
    body = bodies["PULLDOWN_UPDATE"]
    if case == "old_visibility":
        damaged, count = re.subn(r"if \(\(flags & 1u\) != 0\) \{",
                                 "if ((flags & 1u) == 0) {", body, count=1)
    elif case == "selected_callback_mode":
        damaged, count = body.replace("field_F38_(this, 1, &row);",
                                      "field_F38_(this, 0, &row);"), 1
    elif case == "new_mnemonic_metric":
        damaged, count = body.replace("const int center_height = buffer->text_height();",
                                      "const int center_height = buffer->text_line_height();"), 1
    elif case == "padding_reload":
        damaged = body.replace("row.left += PullDownTextPadding;", "row.left += padding;")
        damaged = damaged.replace("row.right -= PullDownTextPadding;", "row.right -= padding;")
        count = int(damaged != body)
    elif case == "old_expanded_fill":
        old = """row.left -= PullDownTextPadding;
            row.right += PullDownTextPadding;
            fill(&row, PullDownDefaultTextColor);"""
        damaged, count = body.replace(old, "fill(&row, PullDownDefaultTextColor);", 1), 1
    elif case == "initial_read_order":
        old = """const int pixel_width = buffer->pixel_width();
  const int row_height = buffer->text_line_height();
  const int border = PullDownBorderPadding;
  const int padding = PullDownTextPadding;"""
        new = """const int border = PullDownBorderPadding;
  const int padding = PullDownTextPadding;
  const int row_height = buffer->text_line_height();
  const int pixel_width = buffer->pixel_width();"""
        damaged, count = body.replace(old, new), 1
    elif case == "rendered_text_reload":
        damaged = body.replace("char *const rendered_text = item.text;",
                               "char *const rendered_text = text;")
        count = body.count("char *const rendered_text = item.text;")
    else:
        old = """const int letter_width = buffer->text_width(mnemonic, 1);
          const int center_height = buffer->text_height();
          const int underline_y =
              underline_bottom - ((underline_height - center_height) / 2)
              - leading;
          const int prefix_width = buffer->text_width(
              text, static_cast<int>(mnemonic - text));"""
        new = """const int prefix_width = buffer->text_width(
              text, static_cast<int>(mnemonic - text));
          const int letter_width = buffer->text_width(mnemonic, 1);
          const int center_height = buffer->text_height();
          const int underline_y =
              underline_bottom - ((underline_height - center_height) / 2)
              - leading;"""
        damaged, count = body.replace(old, new), body.count(old)
    expected_count = 2 if case == "rendered_text_reload" else 1
    assert count == expected_count and damaged != body, case
    bodies["PULLDOWN_UPDATE"] = damaged
    result = compile_and_run(tmp_path, bodies)
    assert result.returncode != 0, (case, result.stderr)


@pytest.mark.parametrize("member", ["UNK1", "ON_MOUSE_MOVE", "MOUSE_MOVE", "PULLDOWN_UPDATE"])
def test_actual_member_deletion_is_detected(tmp_path, member):
    bodies = actual_bodies()
    opening = bodies[member].index("{")
    no_op_body = {
        "UNK1": "{}",
        "ON_MOUSE_MOVE": "{ (void)x; (void)y; }",
        "MOUSE_MOVE": "{ (void)y; }",
        "PULLDOWN_UPDATE": "{ (void)old_selected; (void)new_selected; }",
    }[member]
    bodies[member] = bodies[member][:opening] + no_op_body
    result = compile_and_run(tmp_path, bodies)
    assert result.returncode != 0, (member, result.stderr)
