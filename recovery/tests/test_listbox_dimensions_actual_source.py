"""Host controls for the product ListBox::calculate_dimensions definition.

The complete definition is extracted from src/listbox.cpp on every run. These
controls exercise ordinary well-formed list/layout branches only; the pinned
malformed head-null/current-nonnull absolute [4] read is asserted present but
is intentionally never executed by a host process.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

ROOT = Path(__file__).resolve().parents[2]
FIXTURE = Path(__file__).with_name("listbox_dimensions_host_control.cpp")
SIGNATURE = "int ListBox::calculate_dimensions()"
ITEM_SIGNATURE = "int ListBox::item(char *text, int id)"


def extract_definition(source: str, signature: str = SIGNATURE) -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    code = COMMENTS_OR_STRINGS.sub(lambda m: re.sub(r"[^\n]", " ", m[0]), source)
    if code.count(signature) != 1:
        raise AssertionError(f"expected one product definition: {signature}")
    start = code.index(signature)
    opening = code.index("{", start)
    depth = 0
    for end in range(opening, len(code)):
        if code[end] == "{":
            depth += 1
        elif code[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated product dimensions definition")


def build(tmp_path: Path) -> Path:
    source = (ROOT / "src/listbox.cpp").read_text(encoding="utf-8")
    body = extract_definition(source)
    item = extract_definition(source, ITEM_SIGNATURE)
    harness = FIXTURE.read_text(encoding="utf-8")
    marker = "/* PRODUCT_DEFINITION */"
    assert harness.count(marker) == 1
    cpp = tmp_path / "dimensions-product.cpp"
    assert harness.count("/* PRODUCT_ITEM_DEFINITION */") == 1
    harness = harness.replace(marker, body)
    cpp.write_text(harness.replace("/* PRODUCT_ITEM_DEFINITION */", item), encoding="utf-8")
    return cpp


def run_body(tmp_path: Path, body: str, compiler: str, stem: str,
             item_body: str | None = None) -> subprocess.CompletedProcess:
    harness = FIXTURE.read_text(encoding="utf-8")
    marker = "/* PRODUCT_DEFINITION */"
    assert harness.count(marker) == 1
    cpp = tmp_path / f"{stem}.cpp"
    exe = tmp_path / stem
    source_text = (ROOT / "src/listbox.cpp").read_text(encoding="utf-8")
    item = extract_definition(source_text, ITEM_SIGNATURE) if item_body is None else item_body
    harness = harness.replace(marker, body)
    cpp.write_text(harness.replace("/* PRODUCT_ITEM_DEFINITION */", item), encoding="utf-8")
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(cpp), "-o", str(exe)], check=True,
                   capture_output=True, text=True, timeout=30)
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_dimensions_source(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = build(tmp_path)
    binary = tmp_path / f"dimensions-{compiler_name}"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    subprocess.run([str(binary)], check=True, capture_output=True,
                   text=True, timeout=30)


def test_product_keeps_malformed_path_but_host_control_does_not_execute_it():
    body = extract_definition((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))
    assert "reinterpret_cast<char **>(4)" in body or "reinterpret_cast<char **>(0x4)" in body
    assert "entries.head_ == 0" in body or "head == 0" in body
    # Host fixture's well-formed lists always supply a nonnull head.


def test_damage_controls_reject_wrong_layout_slots():
    body = extract_definition((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))
    # The pinned font loads are Dialog +0x70/+0x74/+0x78, not +0xAC/+0xB0/+0xB4.
    assert "dialog.font1_" in body and "dialog.font2_" in body and "dialog.font3_" in body
    assert "field_2C_" in body and "entries.current_position_" in body
    assert "WinFlag" not in body  # retain the original raw 0x40 condition
    # Correct circular behavior is source-owned by the list fields and next
    # link; replacing it with a safe helper can silently suppress the malformed
    # pinned path and cursor side effects.
    assert "entries.current_position_" in body
    assert "entries.current_->next" in body or "current + 0xc" in body


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_behavioral_damage_control_detects_wrong_color_binding(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_definition((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))
    good = run_body(tmp_path, body, compiler, "dimensions-good")
    assert good.returncode == 0, good.stderr
    damaged = body.replace("dialog.text_color_b_", "dialog.text_color_a_")
    assert damaged != body, "damage anchor disappeared: color argument mapping"
    bad = run_body(tmp_path, damaged, compiler, "dimensions-damaged")
    assert bad.returncode != 0, "behavioral control failed to detect changed color slot"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("old,new", (
    ("dialog.font2_", "dialog.font1_"),
    ("ScrollDefaultThickness / static_cast<int32_t>(dialog.field_54_)",
     "ScrollDefaultThickness / dialog.field_54_"),
))
def test_behavioral_damage_controls_detect_font_and_signed_division(
        compiler_name, tmp_path, old, new):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_definition((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))
    assert old in body
    good = run_body(tmp_path, body, compiler, "good")
    assert good.returncode == 0, good.stderr
    bad = run_body(tmp_path, body.replace(old, new), compiler, "damaged")
    assert bad.returncode != 0, "damaged font or signed division escaped the control"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_item_damage_control_rejects_wrong_receiver_field(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/listbox.cpp").read_text(encoding="utf-8")
    dimensions = extract_definition(source)
    item = extract_definition(source, ITEM_SIGNATURE)
    assert "Buffer::field_18_" in item
    bad = run_body(tmp_path, dimensions, compiler, "wrong-height-owner",
                   item.replace("Buffer::field_18_", "field_18_"))
    assert bad.returncode != 0, "ListBox scroll state substituted for Buffer height"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_item_source_controls_measurement_and_insertion(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = build(tmp_path)
    binary = tmp_path / f"dimensions-item-{compiler_name}"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    subprocess.run([str(binary)], check=True, capture_output=True,
                   text=True, timeout=30)
