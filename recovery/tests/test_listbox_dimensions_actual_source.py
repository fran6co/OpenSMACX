"""Host controls for product ListBox definitions, including the pinned fault.

The complete definitions are extracted from src/listbox.cpp on every run. A
malformed null-payload state is executed in a subprocess so its preserved
unchecked fault can be distinguished from a normalized safe path.
"""
from pathlib import Path
import re
import os
import signal
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
             item_body: str | None = None,
             args: tuple[str, ...] = ()) -> subprocess.CompletedProcess:
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
    return subprocess.run([str(exe), *args], capture_output=True, text=True,
                          timeout=30)


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


def test_product_uses_typed_payload_read_for_malformed_null_path():
    body = extract_definition((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))
    assert "reinterpret_cast<char **>" not in body
    assert "string_payload()" in body and "payload->text" in body
    assert "head == 0" in body or "head != 0" in body
    # The malformed branch is exercised as a subprocess fault control below.


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_dimensions_preserves_malformed_null_payload_fault(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = build(tmp_path)
    binary = tmp_path / f"dimensions-malformed-{compiler_name}"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    str(source), "-o", str(binary)], check=True,
                   capture_output=True, text=True, timeout=30)
    result = subprocess.run([str(binary), "malformed-null-payload"],
                            capture_output=True, text=True, timeout=30)
    if os.name == "posix":
        assert result.returncode == -signal.SIGSEGV, (
            "malformed head-null/current-nonnull payload did not retain the "
            f"pinned null-dereference fault: return={result.returncode}, "
            f"stderr={result.stderr}"
        )
    else:
        assert result.returncode != 0, "malformed null payload unexpectedly returned"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_safety_normalized_payload_mutant_returns_normally(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_definition((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))
    safe, count = re.subn(
        r"name\s*=\s*payload->text\s*;",
        "name = payload == 0 ? 0 : payload->text;", body, count=1)
    assert count == 1, "typed payload read anchor disappeared"
    result = run_body(tmp_path, safe, compiler, f"dimensions-safe-{compiler_name}",
                      args=("malformed-null-payload",))
    assert result.returncode == 0, (
        "safe null-payload counterfactual should return with initialized layout; "
        f"return={result.returncode}, stderr={result.stderr}"
    )


def test_damage_controls_reject_wrong_layout_slots():
    body = extract_definition((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))
    # The pinned font loads are Dialog +0x70/+0x74/+0x78, not +0xAC/+0xB0/+0xB4.
    assert all(field in body for field in ("font1_", "font2_", "font3_"))
    assert "field_2C_" in body and "current_position_" in body
    assert "WinFlag" not in body  # retain the original raw 0x40 condition
    # Correct circular behavior is source-owned by the list fields and next
    # link; replacing it with a safe helper can silently suppress the malformed
    # pinned path and cursor side effects.
    assert "current_position_" in body
    assert "->next" in body
    assert "string_payload()" in body and "payload->text" in body
    assert "reinterpret_cast<char **>" not in body


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_behavioral_damage_control_detects_wrong_color_binding(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_definition((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))
    good = run_body(tmp_path, body, compiler, "dimensions-good")
    assert good.returncode == 0, good.stderr
    pattern = r"(?<![A-Za-z0-9_])(?:dialog\s*\.\s*|Dialog\s*::\s*)?text_color_b_(?![A-Za-z0-9_])"
    damaged, count = re.subn(pattern, lambda match: match[0].replace(
        "text_color_b_", "text_color_a_"), body, count=1)
    assert count == 1, "damage anchor disappeared: color argument mapping"
    bad = run_body(tmp_path, damaged, compiler, "dimensions-damaged")
    assert bad.returncode != 0, "behavioral control failed to detect changed color slot"


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("mutation", ("font", "signed_division"))
def test_behavioral_damage_controls_detect_font_and_signed_division(
        compiler_name, tmp_path, mutation):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    body = extract_definition((ROOT / "src/listbox.cpp").read_text(encoding="utf-8"))
    if mutation == "font":
        pattern = r"(?<![A-Za-z0-9_])(?:dialog\s*\.\s*|Dialog\s*::\s*)?font2_(?![A-Za-z0-9_])"
        damaged, count = re.subn(pattern, lambda match: match[0].replace(
            "font2_", "font1_"), body, count=1)
    else:
        pattern = (r"ScrollDefaultThickness\s*/\s*"
                   r"((?:dialog\s*\.\s*|Dialog\s*::\s*)?column_count_)")
        damaged, count = re.subn(
            pattern,
            lambda match: "ScrollDefaultThickness / (" + match[1] + " + 1)",
            body, count=1)
    assert count == 1, f"{mutation} damage anchor disappeared"
    good = run_body(tmp_path, body, compiler, "good")
    assert good.returncode == 0, good.stderr
    bad = run_body(tmp_path, damaged, compiler, "damaged")
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


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_item_rejects_old_negative_width_height(compiler_name, tmp_path):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(f"{compiler_name} unavailable")
    source = (ROOT / "src/listbox.cpp").read_text(encoding="utf-8")
    dimensions = extract_definition(source)
    item = extract_definition(source, ITEM_SIGNATURE)
    assert item.count("if (count != 0)") == 1
    old_item = item.replace("if (count != 0)", "if (count > 0)")
    result = run_body(tmp_path, dimensions, compiler, "old-negative-height", old_item)
    assert result.returncode != 0, "negative-width control admitted the old height selection"
