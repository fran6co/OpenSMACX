"""Installable host controls for the actual Texture::draw_mask_dest source.

The permanent copy belongs beside this fixture in recovery/tests. It extracts
only the mapped current definition, never substitutes the ignored candidate.
The host setup-edge and pointer globals are controlled doubles, so this does
not establish target ABI, x87/native equivalence, or game behavior.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import Mode, read_text


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "CMakeLists.txt").is_file()
            and (parent / "src/texture.cpp").is_file())
TEXTURE = ROOT / "src/texture.cpp"
BASELINE = "22570989525da4ba3b2f96bcd06189f285517d51"
FIXTURE = Path(__file__).with_name("draw_mask_host_control.cpp")
SIGNATURE = "void Texture::draw_mask_dest("
INSERTION = "// ACTUAL_METHOD_INSERTION"


def _masked(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )


def _definition(source, signature):
    masked = _masked(source)
    start = masked.find(signature)
    if start < 0 or masked.find(signature, start + 1) >= 0:
        raise AssertionError(f"expected exactly one code definition: {signature}")
    brace = masked.find("{", start)
    depth = 0
    for end in range(brace, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated Texture::draw_mask_dest")


def _record(source):
    records = read_text(source, TEXTURE)
    matches = [record for record in records if record.address == 0x0061E540]
    assert len(matches) == 1
    return matches[0]


def _continuity(record):
    lessons = () if record.origin is None else tuple(
        (lesson.kind, lesson.key, lesson.prose,
         tuple(line for _, line in lesson.lines)) for lesson in record.origin
    )
    return (record.address, record.mode, record.name, record.image_spans,
            record.byte_exact, record.semantic, record.kind, record.exclusion,
            record.levers, record.ruled_out, record.unrecoverable,
            record.deferred, record.source_role, record.source_role_reason,
            lessons)


def _actual_tu(source):
    definition = _definition(source, SIGNATURE)
    helpers = "\n\n".join(
        _definition(source, signature)
        for signature in (
            "static uintptr_t texture_pointer_address(",
            "static uintptr_t texture_pointer_offset(",
            "static void *texture_pointer_from_address(",
        )
    )
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(INSERTION) == 1
    return fixture.replace(INSERTION, helpers + "\n\n" + definition, 1), definition


def _actual_method_fixture(source, definition):
    helpers = "\n\n".join(
        _definition(source, signature)
        for signature in (
            "static uintptr_t texture_pointer_address(",
            "static uintptr_t texture_pointer_offset(",
            "static void *texture_pointer_from_address(",
        )
    )
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(INSERTION) == 1
    return fixture.replace(INSERTION, helpers + "\n\n" + definition, 1)


def test_identity_spans_claims_and_legacy_history_are_preserved():
    baseline_source = subprocess.run(
        ["git", "show", f"{BASELINE}:src/texture.cpp"], cwd=ROOT,
        check=True, text=True, capture_output=True, timeout=30,
    ).stdout
    assert _continuity(_record(TEXTURE.read_text(encoding="utf-8"))) == \
        _continuity(_record(baseline_source))
    record = _record(TEXTURE.read_text(encoding="utf-8"))
    assert record.mode is Mode.FILE
    assert record.image_spans == ((0x0061E540, 0x0061EACE),)
    assert _definition(TEXTURE.read_text(encoding="utf-8"), SIGNATURE) in record.region


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_installed_draw_mask_host_controls(compiler, tmp_path):
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    source = TEXTURE.read_text(encoding="utf-8")
    translation_unit, definition = _actual_tu(source)
    assert "setup_edge_gouraud(&left_edge, min_y_index)" in definition
    assert "setup_edge_gouraud(&right_edge, min_y_index)" in definition
    assert "*blit_max_y_index = i;" in definition
    assert "const unsigned char mapped = remap[texel];" in definition
    assert "pixels_ != 0" in definition
    assert "buffer->rect1_" in definition
    assert "buffer->stride_" in definition
    assert "const int initial_span = texture_difference(" in definition
    assert "if (prior_span_fixed > 0) {" in definition
    assert "if (!left_refresh_failed) {" in definition
    assert "texture_span_pixels(" in definition
    cpp = tmp_path / "texture_draw_mask_actual.cpp"
    binary = tmp_path / "texture_draw_mask_actual"
    cpp.write_text(translation_unit, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-no-pie", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_method_remap_zero_mutant_fails_pixel_control(compiler, tmp_path):
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    source = TEXTURE.read_text(encoding="utf-8")
    definition = _definition(source, SIGNATURE)
    mutant = definition.replace("if (mapped != 0)", "if (true)", 1)
    assert mutant != definition
    translation_unit = _actual_method_fixture(source, mutant)
    cpp = tmp_path / "texture_draw_mask_mutant.cpp"
    binary = tmp_path / "texture_draw_mask_mutant"
    cpp.write_text(translation_unit, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-no-pie", str(cpp), "-o", str(binary)],
        check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=30)
    assert ran.returncode != 0


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("mutation",
                         ("gradient_phase", "left_refresh", "packed_count"))
def test_actual_method_phase_mutants_fail_source_controls(compiler, mutation,
                                                          tmp_path):
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    source = TEXTURE.read_text(encoding="utf-8")
    definition = _definition(source, SIGNATURE)
    if mutation == "gradient_phase":
        wrong = "*blit_slot_4e4 = active_du;"
        replacement = (
            "*blit_slot_4e4 = texture_span_step("
            "texture_difference(right_edge.accumA, left_edge.accumA), "
            "texture_difference(right_edge.accumC, left_edge.accumC));"
        )
    elif mutation == "left_refresh":
        wrong = "if (!left_refresh_failed) {"
        replacement = "if (true) {"
    else:
        wrong = ("texture_span_pixels(\n"
                 "                                        texture_difference(right_x,\n"
                 "                                                           *blit_span_right))")
        replacement = "static_cast<unsigned int>(right_x - *blit_span_right)"
    mutant = definition.replace(wrong, replacement, 1)
    assert mutant != definition
    translation_unit = _actual_method_fixture(source, mutant)
    cpp = tmp_path / f"texture_draw_mask_{mutation}_mutant.cpp"
    binary = tmp_path / f"texture_draw_mask_{mutation}_mutant"
    cpp.write_text(translation_unit, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-no-pie", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=30)
    assert ran.returncode != 0
