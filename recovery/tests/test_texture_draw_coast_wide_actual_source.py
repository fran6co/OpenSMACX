"""Actual-source controls for Texture::draw_coast at 0x623F80.

These controls validate the mapped owner body and its original identity; they
do not claim x87 equivalence or application acceptance.
"""
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import Mode, read_file
from texture_direct_storage_host import direct_storage_prelude

ROOT = next(p for p in Path(__file__).resolve().parents if (p / "src/texture.cpp").is_file())
SOURCE = ROOT / "src/texture.cpp"
ADDRESS = 0x00623F80
SPAN = ((0x00623F80, 0x00624626),)
FIXTURE = ROOT / "recovery/tests/fixtures/texture_draw_coast_wide_host.cpp"
SIGNATURE = (
    "void Texture::draw_coast(Buffer * a1, Vert * a2, int * a3, Texture * a4, "
    "unsigned char * a5, int a6, Vert * a7, int a8) {"
)


def _body(source: str) -> str:
    start = source.index(SIGNATURE)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                line_start = source.rfind("\n", 0, start) + 1
                return source[line_start:end + 1]
    raise AssertionError("unterminated 0x623F80 owner body")


def _contract(body: str) -> None:
    required = (
        "TextureRasterPixelBase = a1->get_data();",
        "EdgeScan edge1;",
        "EdgeScan edge2;",
        "edge1.f0 = -1;",
        "edge2.f0 = 1;",
        "setup_edge_gouraud(&edge1, minYIdx)",
        "setup_edge_gouraud(&edge2, minYIdx)",
        "int minYIdx = a8;",
        "const unsigned int base_source_offset = texture_draw_source_index(",
        "--edge1.counter;",
        "--edge2.counter;",
        "edge1.accumA += edge1.deltaA;",
        "edge1.accumB += edge1.deltaB;",
        "edge1.accumC += edge1.deltaC;",
        "edge1.accumD += edge1.deltaD;",
        "edge1.accumC += edge1.extraDeltaC;",
        "edge1.fracAccum -= edge1.fracSub;",
        "edge2.accumA += edge2.deltaA;",
        "edge2.accumB += edge2.deltaB;",
        "edge2.accumC += edge2.deltaC;",
        "edge2.accumD += edge2.deltaD;",
        "edgeSpan = (edge2.accumC - edge1.accumC) * 0x10000;",
        "dA = static_cast<long double>(edge2.accumA - edge1.accumA);",
        "dB = static_cast<long double>(edge2.accumB - edge1.accumB);",
        "dC = static_cast<long double>(edge2.accumD - edge1.accumD);",
        "TextureRasterScratch4E4.dword = static_cast<int>(",
        "TextureRasterScratch4C8.dword = static_cast<int>(",
        "TextureRasterLightPackA.low_word = static_cast<unsigned short>(",
        "texture_draw_keyed_coast_fill_span(",
        "texture_draw_coast_span(",
        "TextureDrawSpanResult span_result;",
        "span_result.packed_uv_sink = &(TextureRasterScratch4A4);",
        "span_result.preload_pixels =",
        "span_result.prefetched_source =\n                                            span_result.preload_pixels[base_source_offset];",
        "TextureRasterScratch504 = reinterpret_cast<unsigned char *>(src);",
        "TextureRasterShadeCursor = texture_pointer_address(src) + (texture_pointer_address(TextureRasterScratch554) - texture_pointer_address(TextureRasterScratch544));",
        "texture_span_pixels(static_cast<short>(rightX) -",
        "_control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);",
        "if ((TextureRasterDither & 1) == 0)",
        "const int row_light = TextureRasterScratch498;",
        "TextureRasterScratch498 = row_light >> 8;",
        "TextureRasterScratch504 = reinterpret_cast<unsigned char *>(src);",
        "a1->free_data(1)",
        "epilogue_only:\n    texture_reset_fpu();\n    return;",
    )
    missing = [part for part in required if part not in body]
    assert not missing, f"0x623F80 native owner evidence missing: {missing}"
    assert "int edge1[14]" not in body and "int edge2[14]" not in body
    assert "setup_edge_gouraud(&edge1, a8)" not in body
    assert "reinterpret_cast<char *>(&TextureRasterShadeCursor) + texOff" not in body
    assert "texOffHi" not in body and "texOffLo" not in body
    assert "a3 != 0 && a5 != 0 && a4 != 0" not in body
    assert "if (pixels_ != 0 && a3 != 0 && a5 != 0)" in body
    assert "reinterpret_cast<int *>(this)" not in body
    assert "TextureRasterScratch554 = a4->pixels_;" in body
    assert body.count("edge2.accumC - edge1.accumC") == 2
    assert body.count("edge2.accumA - edge1.accumA") == 2
    assert body.count("edge2.accumB - edge1.accumB") == 2
    assert body.count("edge2.accumD - edge1.accumD") == 2
    assert body.index("a1->get_data()") < body.index("texture_reset_fpu();")
    assert body.index("_control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);") < body.index("long double invDx")
    assert body.index("a1->free_data(1)") < body.index("epilogue_only:\n    texture_reset_fpu();")
    assert body.index("TextureRasterScratch504 = reinterpret_cast<unsigned char *>(src);") < body.index("texture_draw_keyed_coast_fill_span(")
    assert body.index("TextureRasterScratch504 = reinterpret_cast<unsigned char *>(src);") < body.index("TextureRasterShadeCursor = texture_pointer_address(src)")
    assert body.index("TextureRasterScratch498 = row_light >> 8;") < body.index("TextureRasterAlternateRowNext.value =")
    assert body.index("span_result.prefetched_source =") < body.index("if ((TextureRasterDither & 1) == 0)")


def test_actual_source_keeps_original_identity_and_full_owner_mapping() -> None:
    record, = [row for row in read_file(SOURCE) if row.address == ADDRESS]
    assert record.mode is Mode.FILE
    assert record.image_spans == SPAN
    assert record.name == "?draw_coast@Texture@@QAEXPAUBuffer@@PAUVert@@PAHPAUTexture@@PAEHPAUVert@@H@Z"
    assert record.symbol == "?draw_coast@Texture@@QAEXPAVBuffer@@PAUVert@@PAHPAV1@PAEH1H@Z"
    assert not record.byte_exact
    _contract(_body(SOURCE.read_text(encoding="utf-8")))


@pytest.mark.parametrize("helper_name", (
    "texture_draw_coast_span", "texture_draw_keyed_coast_fill_span",
))
def test_actual_shared_kernel_publishes_packed_scratch_before_pixel_access(helper_name) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    helper = _definition(source, helper_name)
    packed_store = helper.index("*result->packed_uv_sink =")
    pixel_read = helper.index("remap[coast_pixels[source_offset]]")
    assert packed_store < pixel_read


def test_actual_kernels_keep_unconditional_volatile_next_source_loads() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    assert "volatile const unsigned char *preload_pixels;" in _struct_definition(
        source, "TextureDrawSpanResult"
    )
    for helper_name in ("texture_draw_coast_span", "texture_draw_keyed_coast_fill_span"):
        helper = _definition(source, helper_name)
        load = "result->prefetched_source = result->preload_pixels[source_offset];"
        assert helper.count(load) == 1
        assert helper.index("++destination;") < helper.index(load)


@pytest.mark.parametrize(
    "old,new",
    (
        ("edge2.accumC - edge1.accumC", "edge2.fracAccum - edge1.fracAccum"),
        ("edge2.accumA - edge1.accumA", "edge2.accumB - edge1.accumB"),
        ("setup_edge_gouraud(&edge1, minYIdx)", "setup_edge_gouraud(&edge1, a8)"),
        ("int minYIdx = a8;", "int minYIdx = a6;"),
        ("base_source_offset = texture_draw_source_index(",
         "base_source_offset = texture_draw_source_index_signed("),
        ("TextureRasterScratch504 = reinterpret_cast<unsigned char *>(src);",
         "TextureRasterScratch504 = reinterpret_cast<unsigned char *>(src + 1);"),
        ("if (pixels_ != 0 && a3 != 0 && a5 != 0)",
         "if (pixels_ != 0 && a3 != 0 && a5 != 0 && a4 != 0)"),
        ("_control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);",
         "_control87(_PC_64 | _RC_NEAR, _MCW_PC | _MCW_RC);"),
    ),
)
def test_actual_source_controls_reject_edge_channel_or_shade_slot_mutants(old: str, new: str) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = _body(source)
    mutant = body.replace(old, new, 1)
    assert mutant != body
    with pytest.raises(AssertionError):
        _contract(mutant)


def _definition(source: str, marker: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: re.sub(r"[^\n]", " ", match[0]), source
    )
    matches = []
    for match in re.finditer(r"\b" + re.escape(marker) + r"\s*\(", masked):
        depth = 1
        end = match.end()
        while end < len(masked) and depth:
            if masked[end] == "(":
                depth += 1
            elif masked[end] == ")":
                depth -= 1
            end += 1
        while end < len(masked) and masked[end].isspace():
            end += 1
        if depth == 0 and end < len(masked) and masked[end] == "{":
            matches.append((match.start(), end))
    assert len(matches) == 1, (marker, len(matches))
    start, opening = matches[0]
    depth = 0
    for end in range(opening, len(masked)):
        if masked[end] == "{":
            depth += 1
        elif masked[end] == "}":
            depth -= 1
            if depth == 0:
                line_start = source.rfind("\n", 0, start) + 1
                return source[line_start:end + 1]
    raise AssertionError(f"unterminated {marker}")


def _struct_definition(source: str, name: str) -> str:
    marker = f"struct {name} {{"
    start = source.index(marker)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1] + ";"
    raise AssertionError(f"unterminated struct {name}")


HELPER_NAMES = (
    "texture_span_pixels",
    "texture_draw_source_step",
    "texture_draw_source_index",
    "texture_draw_next_source_index",
    "texture_draw_coast_span",
    "texture_draw_keyed_coast_fill_span",
)


def _translation_unit(
    source: str, body: str | None = None, coast_helper: str | None = None,
) -> str:
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count("static int TextureRowPitchScratch;") <= 1
    fixture = fixture.replace("static int TextureRowPitchScratch;", "", 1)
    helpers_by_name = {name: _definition(source, name) for name in HELPER_NAMES}
    if coast_helper is not None:
        helpers_by_name["texture_draw_coast_span"] = coast_helper
    helpers = "\n\n".join(helpers_by_name[name] for name in HELPER_NAMES)
    result_struct = _struct_definition(source, "TextureDrawSpanResult")
    owner = _body(source) if body is None else body
    # Actual pointer-valued cells keep host pointers intact. Existing integer
    # scratch slots retain their explicit 32-bit bridge in this fixture.
    owner = re.sub(r"reinterpret_cast<int>\((a[2357]|src)\)",
                   r"ptr32(\1)", owner)
    return (fixture.replace("// TEXTURE_DIRECT_STORAGE_PRELUDE",
                            direct_storage_prelude(source), 1)
            .replace("// ACTUAL_RESULT_STRUCT", result_struct, 1)
            .replace("// ACTUAL_HELPERS", helpers, 1)
            .replace("// ACTUAL_METHOD", owner, 1))


def _compile_and_run(code: str, compiler: str, tmp_path: Path, name: str) -> int:
    tool = shutil.which(compiler)
    if tool is None:
        pytest.skip(f"{compiler} unavailable")
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(code, encoding="utf-8")
    built = subprocess.run(
        [tool, "-std=c++11", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-no-pie", str(cpp), "-o", str(binary)],
        cwd=ROOT, check=False, capture_output=True, text=True, timeout=45,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], check=False, capture_output=True,
                          text=True, timeout=30).returncode


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_installed_owner_executes_clipped_two_row_keyed_coast(
    compiler: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    assert _compile_and_run(_translation_unit(source), compiler, tmp_path,
                            "texture_draw_coast_actual") == 0


@pytest.mark.parametrize(
    "old,new",
    (
        ("static_cast<unsigned char>(TextureRasterScratch494), pixel_count,\n"
         "                                                &span_result)",
         "static_cast<unsigned char>(TextureRasterScratch494 + 1), pixel_count,\n"
         "                                                &span_result)"),
        ("TextureRasterScratch4E4.dword, TextureRasterScratch4C8.dword, row_light,",
         "TextureRasterScratch4E4.dword, TextureRasterScratch4C8.dword, row_light + 0x10000,"),
        ("if ((TextureRasterDither & 1) == 0)", "if ((TextureRasterDither & 1) != 0)"),
        ("if (TextureRasterSpanRight < TextureRasterClipLeft)", "if (false)"),
        ("span_result.packed_uv_sink = &(TextureRasterScratch4A4);",
         "span_result.packed_uv_sink = 0;"),
    ),
)
def test_actual_owner_runtime_controls_reject_pixel_mutants(
    old: str, new: str, tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    body = _body(source)
    mutant = body.replace(old, new, 1)
    assert mutant != body
    assert _compile_and_run(_translation_unit(source, mutant), "g++", tmp_path,
                            "texture_draw_coast_mutant") != 0


def test_actual_next_source_preload_mutant_changes_mapped_one_pixel(
    tmp_path: Path,
) -> None:
    source = SOURCE.read_text(encoding="utf-8")
    helper = _definition(source, "texture_draw_coast_span")
    load = "result->prefetched_source = result->preload_pixels[source_offset];"
    mutant = helper.replace(load, "result->prefetched_source = result->prefetched_source;", 1)
    assert mutant != helper
    assert _compile_and_run(_translation_unit(source, coast_helper=mutant),
                            "g++", tmp_path, "texture_draw_coast_preload_mutant") != 0
