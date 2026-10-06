"""Bounded host controls for the installed Texture raster source.

These tests compile only definitions lexically extracted from current
src/texture.cpp. A Git baseline is read through the framework source parser to
protect mapped identity, spans, claims, and lesson history. This is not an ABI,
VC6/native-code, raster-edge-equivalence, production-edge-helper, x87/carry-
equivalence, or game-acceptance test. `setup_edge`, `sub_624eb0`, and buffer
access are controlled host doubles; pixel checks are limited to the exercised
paths and mutation sensitivity.
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
BASELINE = "5fe79d193"
SOURCE_IDS = (
    (0x0061B780, "Texture::sub_61b780"),
    (0x00620A10, "Texture::sub_620a10"),
    (0x0061D090, "Texture::sub_61d090"),
)
HELPERS = (
    ("static unsigned int texture_raster_pixel_count(",),
    ("static int texture_raster_advance_light(",),
    ("static void texture_raster_span(",),
    ("static int texture_raster_step_gouraud(",),
    ("static int texture_raster_step_plain(",),
    ("static void texture_raster_gouraud_rows(",),
    ("static void texture_raster_plain_rows(",),
)
METHODS = tuple(f"void {name}(" for _, name in SOURCE_IDS)
INSERTION = "// ACTUAL_TEXTURE_DEFINITIONS_INSERTED_BY_TEST"
FIXTURE = Path(__file__).with_name("texture_raster_actual_host_control.cpp")


def _mask_comments_and_literals(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )


def _extract_definition(source, signature):
    """Extract one actual C++ definition; comment/string decoys are invisible."""
    masked = _mask_comments_and_literals(source)
    start = masked.find(signature)
    if start < 0 or masked.find(signature, start + 1) >= 0:
        raise AssertionError(f"expected one code definition for {signature!r}")
    brace = masked.find("{", start)
    if brace < 0:
        raise AssertionError(f"body not found for {signature!r}")
    depth = 0
    for end in range(brace, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError(f"unterminated C++ definition for {signature!r}")


def _record_map(text):
    rows = read_text(text, TEXTURE)
    result = {}
    for address, _ in SOURCE_IDS:
        matches = [row for row in rows if row.address == address]
        assert len(matches) == 1, f"framework parser must find 0x{address:08X} once"
        result[address] = matches[0]
    return result


def _lesson_history(row):
    if row.origin is None:
        return None
    return tuple((lesson.kind, lesson.key, lesson.prose,
                  tuple(line for _, line in lesson.lines))
                 for lesson in row.origin)


def _continuity_fields(row):
    # The current emitted symbol is intentionally excluded: source-level
    # receiver changes require a fresh candidate COFF observation. The catalog
    # identity, original extents, claims, and authored lessons are stable.
    return (
        row.address, row.mode, row.name, row.image_spans, row.byte_exact,
        row.semantic, row.kind, row.exclusion, row.levers, row.ruled_out,
        row.unrecoverable, row.deferred, row.source_role,
        row.source_role_reason, _lesson_history(row),
    )


def _baseline_source():
    result = subprocess.run(
        ["git", "show", f"{BASELINE}:src/texture.cpp"], cwd=ROOT,
        check=True, text=True, capture_output=True, timeout=30,
    )
    return result.stdout


def actual_installed_definitions(source=None):
    if source is None:
        source = TEXTURE.read_text(encoding="utf-8")
    records = _record_map(source)
    definitions = []
    for (address, owner), signature in zip(SOURCE_IDS, METHODS):
        row = records[address]
        assert row.mode is Mode.FILE and row.name
        assert owner in row.region, f"0x{address:08X} parser region lacks its C++ owner"
        definition = _extract_definition(source, signature)
        assert definition in row.region, f"0x{address:08X} body is outside mapped region"
        definitions.append(definition)
    helpers = [_extract_definition(source, signature[0]) for signature in HELPERS]
    return helpers + definitions


def _translation_unit(source):
    actual = actual_installed_definitions(source)
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(INSERTION) == 1
    return fixture.replace(INSERTION, "\n\n".join(actual), 1)


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_installed_texture_raster_source_host_controls(compiler, tmp_path):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is unavailable")
    source = TEXTURE.read_text(encoding="utf-8")
    cpp = tmp_path / "texture_actual.cpp"
    binary = tmp_path / "texture_actual"
    cpp.write_text(_translation_unit(source), encoding="utf-8")
    built = subprocess.run(
        [compiler_path, "-std=c++11", "-Wall", "-Wextra",
         "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=45)
    assert ran.returncode == 0, ran.stdout + ran.stderr


def test_framework_identity_spans_claims_and_history_match_git_baseline():
    baseline = _record_map(_baseline_source())
    current_source = TEXTURE.read_text(encoding="utf-8")
    current = _record_map(current_source)
    for address, _ in SOURCE_IDS:
        assert _continuity_fields(current[address]) == _continuity_fields(baseline[address])
        assert current[address].mode is Mode.FILE


def test_fake_comment_and_literal_definitions_do_not_confuse_extraction():
    source = TEXTURE.read_text(encoding="utf-8")
    expected = actual_installed_definitions(source)
    decoys = (
        "\n/* void Texture::sub_61b780(Buffer *b, int *v, int z, int m) { } */\n"
        "static const char *fake_texture_signature = \"void Texture::sub_620a10( { }\";\n"
    )
    assert actual_installed_definitions(source + decoys) == expected


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_behavioral_mutant_in_actual_helper_fails_pixel_assertions(compiler, tmp_path):
    compiler_path = shutil.which(compiler)
    if compiler_path is None:
        pytest.skip(f"{compiler} is unavailable")
    source = TEXTURE.read_text(encoding="utf-8")
    mutated = source.replace("keyed && borrowed", "keyed", 1)
    assert mutated != source, "mutant did not alter current installed source"
    cpp = tmp_path / "texture_mutant.cpp"
    binary = tmp_path / "texture_mutant"
    cpp.write_text(_translation_unit(mutated), encoding="utf-8")
    built = subprocess.run(
        [compiler_path, "-std=c++11", "-Wall", "-Wextra",
         "-Wno-unused-parameter", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], check=False, capture_output=True,
                         text=True, timeout=45)
    assert ran.returncode != 0, "pixel behavior mutant escaped fixture assertions"

