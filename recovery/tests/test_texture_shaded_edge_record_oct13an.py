"""Literal-source host controls for Texture::sub_61c630's EdgeScan locals.

The deterministic setup callback supplies arbitrary values for the word that
the original setup routine leaves untouched.  These checks prove that the
actual C++ method publishes that supplied word; they do not infer the original
stack value, native runtime behavior, or pixel impact.
"""
from dataclasses import fields
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import read_file, read_text
from match_recovery.tasks import TaskStore


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/texture.cpp"
HEADER = ROOT / "src/texture.h"
FIXTURE = ROOT / "recovery/tests/fixtures/texture_sub61f0d0_actual_host.cpp"
BASE = "b1684c290c5f4145438f98d2b5189e31f2f496c4"
TASK = "texture-shaded-edge-record-owner-oct13an"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SIGNATURE = "void Texture::sub_61c630(Buffer * bufIn, Vert * verts, Texture * texB, unsigned char * shadeTable, int vertCount, Vert *edge_vertices)"
METHOD_MARKER = "// ACTUAL_DRAW_TRANS2_INSERTION"
NOTE = """// STRUCTURAL EVIDENCE: setup_edge 0x624D80 writes through EdgeScan+0x30;
// sub_61c630 reads untouched firstEdge+0x34 into scratch 0x9BB498 at 0x61C875/87B.
// Preserve that indeterminate stack effect; its original value/pixel impact is unproved.
"""


def _extract(source: str, signature: str = SIGNATURE) -> str:
    masked = COMMENTS_OR_STRINGS.sub(lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert masked.count(signature) == 1, (signature, masked.count(signature))
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated source body: {signature}")


def _extract_record(source: str, name: str) -> str:
    signature = f"struct {name}"
    masked = COMMENTS_OR_STRINGS.sub(lambda match: re.sub(r"[^\n]", " ", match[0]), source)
    assert masked.count(signature) == 1
    start = masked.index(signature)
    opening = masked.index("{", start + len(signature))
    closing = masked.index("};", opening) + 1
    return source[start:closing + 1]


def _tokens(source: str) -> list[str]:
    token = re.compile(
        r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|'
        r'//[^\n]*|/\*.*?\*/|[A-Za-z_]\w*|0[xX][0-9A-Fa-f]+|\d+|'
        r'::|->|\+\+|--|&&|\|\||==|!=|<=|>=|<<|>>|'
        r'\+=|-=|\*=|/=|%=|&=|\|=|\^=|##|.', re.S)
    result = []
    for match in token.finditer(source):
        value = match.group(0)
        if value.startswith("//") or value.startswith("/*") or value.isspace():
            continue
        result.append(value)
    return result


def _expected_current_body(old_body: str) -> str:
    old = old_body
    declarations = (
        ("int e1[15]; for (int z1 = 0; z1 < 15; z1++) e1[z1] = 0;", "EdgeScan e1;"),
        ("int e2[15]; for (int z2 = 0; z2 < 15; z2++) e2[z2] = 0;", "EdgeScan e2;"),
    )
    for before, after in declarations:
        assert old.count(before) == 1, before
        old = old.replace(before, after, 1)

    # This is the complete source-visible field projection of the 15-word
    # original records.  deltaD remains unused, exactly as in the old body.
    names = {
        0: "f0", 1: "counter", 2: "id", 3: "accumA", 4: "accumB",
        5: "deltaA", 6: "deltaB", 7: "accumC", 8: "deltaC",
        9: "extraDeltaC", 10: "fracAccum", 11: "fracDelta", 12: "fracSub",
        13: "accumD", 14: "deltaD",
    }
    for edge in ("e1", "e2"):
        old = re.sub(rf"reinterpret_cast<EdgeScan \*>\({edge}\)", f"&{edge}", old)
        old = re.sub(rf"\b{edge}\[(\d+)\]", lambda m: f"{edge}.{names[int(m.group(1))]}", old)
    return old


def _strip_authorized_edge_delta(current: str, baseline: str) -> str:
    """Check the exact sub_61c630 delta, then return source with it reversed."""
    baseline_body = _extract(baseline)
    current_body = _extract(current)
    assert _tokens(current_body) == _tokens(_expected_current_body(baseline_body)), (
        "sub_61c630 may only replace the two zeroed int arrays/casts/indexes "
        "with the established EdgeScan fields")
    current = current.replace(current_body, baseline_body, 1)
    assert current.count(NOTE) == 1
    assert NOTE not in baseline
    return current.replace(NOTE, "", 1)


def _assert_actual_record_shape() -> str:
    source = HEADER.read_text(encoding="utf-8")
    record = _extract_record(source, "EdgeScan")
    members = re.findall(r"\bint\s+(\w+)\s*;", record)
    assert members == [
        "f0", "counter", "id", "accumA", "accumB", "deltaA", "deltaB",
        "accumC", "deltaC", "extraDeltaC", "fracAccum", "fracDelta",
        "fracSub", "accumD", "deltaD",
    ]
    return record


def _translation_unit(body: str) -> str:
    fixture = FIXTURE.read_text(encoding="utf-8")
    fixture = fixture.replace("#include <cassert>", "#include <cassert>\n#include <cstdio>\n#include <cstdlib>", 1)
    scale_anchor = "static int *blit_slot_498 = &cell_slot_498;"
    assert fixture.count(scale_anchor) == 1
    fixture = fixture.replace(
        scale_anchor,
        scale_anchor + "\nstatic float cell_scale_65536f = 1.0f;\nstatic float *blit_scale_65536f = &cell_scale_65536f;",
        1)
    assert fixture.count(METHOD_MARKER) == 1
    fixture = fixture.replace(METHOD_MARKER, body, 1)

    old_record = _extract_record(fixture, "EdgeScan")
    fixture = fixture.replace(old_record, _assert_actual_record_shape(), 1)
    declaration = "    int setup_edge(EdgeScan *, int);"
    assert fixture.count(declaration) == 1
    fixture = fixture.replace(
        declaration,
        declaration + "\n    void sub_61c630(Buffer *, Vert *, Texture *, unsigned char *, int, Vert *);",
        1)

    old_setup = """int Texture::setup_edge(EdgeScan *, int) {
    ++setup_edge_calls;
    active_buffer->events_.push_back(2);
    return 0;
}"""
    assert fixture.count(old_setup) == 1
    fixture = fixture.replace(old_setup, _scripted_setup(), 1)

    runner = _runner()
    main = _extract(fixture, "int main()")
    fixture = fixture.replace(main, runner, 1)
    return fixture


def _scripted_setup() -> str:
    return r"""
static int expected_poison;
static int setup_failure;
static int scratch_at_refresh;
static int setup_calls_for_case;
static void require(bool condition, const char *message) {
    if (!condition) {
        std::fprintf(stderr, "ASSERT98: %s\n", message);
        std::exit(98);
    }
}
int Texture::setup_edge(EdgeScan *edge, int index) {
    ++setup_edge_calls;
    ++setup_calls_for_case;
    active_buffer->events_.push_back(2);
    if (setup_calls_for_case == 1) {
        require(edge->f0 == -1 && index == 0, "first setup direction/index");
        edge->counter = 1; edge->id = 101;
        edge->accumA = 10; edge->accumB = 20; edge->accumC = 30;
        edge->deltaA = 1; edge->deltaB = 2; edge->deltaC = 3;
        edge->extraDeltaC = 4; edge->fracAccum = -2;
        edge->fracDelta = 1; edge->fracSub = 5;
        edge->accumD = expected_poison;
        edge->deltaD = expected_poison ^ 0x13579BDF;
        return 1;
    }
    if (setup_calls_for_case == 2) {
        require(edge->f0 == 1 && index == 0, "second setup direction/index");
        edge->counter = 1; edge->id = 202;
        edge->accumA = 100; edge->accumB = 200; edge->accumC = 300;
        edge->deltaA = 5; edge->deltaB = 6; edge->deltaC = 7;
        edge->extraDeltaC = 8; edge->fracAccum = -2;
        edge->fracDelta = 1; edge->fracSub = 9;
        edge->accumD = ~expected_poison;
        edge->deltaD = 0x2468ACE;
        return 1;
    }
    require(setup_calls_for_case == 3, "only original left edge refresh expected");
    require(index == 101, "refresh uses live edge id");
    require(edge->accumD == expected_poison, "setup_edge leaves accumD untouched");
    scratch_at_refresh = cell_slot_498;
    require(scratch_at_refresh == expected_poison,
            "scratch 0x9BB498 receives the supplied untouched accumD word");
    return setup_failure;
}
"""


def _runner() -> str:
    return r"""
static void run_publication_case(int poison) {
    setup_edge_calls = setup_calls_for_case = 0;
    setup_failure = 0;
    expected_poison = poison;
    scratch_at_refresh = cell_slot_498 = 0x5A5A5A5A;
    buffer.events_.clear();
    active_buffer = &buffer;
    buffer.stride_ = 1;
    buffer.rect1_.left = -100;
    buffer.rect1_.top = 1;
    buffer.rect1_.right = 100;
    buffer.rect1_.bottom = 1;
    buffer.data_ = pixels;
    texture.pixels_ = texels;
    texture.iWidth_ = 16;
    texture.iHeight_ = 16;
    texture.borrowed_ = 1;
    static Texture shade_texture;
    shade_texture.pixels_ = texels;
    shade_texture.borrowed_ = 1;
    static unsigned char remap[16] = {0};
    input_xy[0].x = 0; input_xy[0].y = 0;
    input_xy[1].x = 1; input_xy[1].y = 2;
    input_xy[2].x = 2; input_xy[2].y = 2;
    input_xy[3].x = 3; input_xy[3].y = 0;
    texture.sub_61c630(&buffer, input_xy, &shade_texture, remap, 4, input_xy);
    require(setup_calls_for_case == 3, "both setup calls and left refresh executed");
    require(scratch_at_refresh == poison && cell_slot_498 == poison,
            "poison reaches scratch before refresh");
}
int main() {
    run_publication_case(0x13579BDF);
    run_publication_case(-0x2468ACE);
    run_publication_case(0x10203040);
    return 0;
}
"""


def _build_run(source: str, compiler_name: str, tmp_path: Path, name: str):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    cpp = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp.write_text(source, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Wno-unused-parameter",
         "-Wno-unused-function", "-ffunction-sections", "-fdata-sections",
         "-Wl,--gc-sections", "-no-pie", str(cpp), "-o", str(binary)],
        cwd=ROOT, capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], cwd=ROOT, capture_output=True,
                          text=True, timeout=30)


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_literal_sub_61c630_publishes_test_supplied_accum_d(compiler_name, tmp_path):
    body = _extract(SOURCE.read_text(encoding="utf-8"))
    assert "EdgeScan e1;" in body and "EdgeScan e2;" in body
    assert "e1.f0 = -1;" in body and "e1.accumD" in body
    result = _build_run(_translation_unit(body), compiler_name, tmp_path,
                        "texture-shaded-edge-record-good")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(("old", "new", "name", "diagnostic"), (
    ("*blit_slot_498 = e1.accumD;", "*blit_slot_498 = 0;",
     "scratch-zero", "scratch 0x9BB498"),
    ("*blit_slot_498 = e1.accumD;", "*blit_slot_498 = e1.deltaD;",
     "wrong-record-field", "scratch 0x9BB498"),
    ("setup_edge(&e1, e1.id)", "setup_edge(&e1, e1.counter)",
     "refresh-counter-not-id", "refresh uses live edge id"),
))
def test_compile_success_publication_mutants_fail_with_named_diagnostic(
    compiler_name, tmp_path, old, new, name, diagnostic
):
    body = _extract(SOURCE.read_text(encoding="utf-8"))
    assert body.count(old) == 1
    mutant = body.replace(old, new, 1)
    result = _build_run(_translation_unit(mutant), compiler_name, tmp_path, name)
    assert result.returncode == 98, (result.returncode, result.stdout, result.stderr)
    assert diagnostic in result.stderr, result.stderr


def test_task_base_source_delta_and_full_annotation_history():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    assert {(item.address, item.address_space, item.image_sha256) for item in task.subjects} == {
        (0x61C630, "pe:va", PIN), (0x624D80, "pe:va", PIN), (0x623F50, "pe:va", PIN),
    }
    baseline = subprocess.check_output(
        ["git", "show", f"{BASE}:src/texture.cpp"], cwd=ROOT, timeout=30
    ).decode("utf-8")
    current = SOURCE.read_text(encoding="utf-8")
    restored = _strip_authorized_edge_delta(current, baseline)

    # The sibling draw_trans2 task's pre-existing, separately checked delta is
    # retained; this test protects only this task's three identities and their
    # complete metadata/history, while the sibling test checks its full source.
    old_rows = {row.address: row for row in read_text(baseline, SOURCE)}
    current_rows = {row.address: row for row in read_file(SOURCE)}
    expected_addresses = {0x61C630, 0x624D80, 0x623F50}
    assert expected_addresses <= old_rows.keys()
    assert old_rows.keys() == current_rows.keys()
    for address, actual in current_rows.items():
        before = old_rows[address]
        for field in fields(before):
            if field.name in {"line", "region", "origin"}:
                continue
            assert getattr(actual, field.name) == getattr(before, field.name), (hex(address), field.name)
        assert tuple((item.kind, item.key, item.prose,
                      tuple(line for _, line in item.lines)) for item in actual.origin) == \
               tuple((item.kind, item.key, item.prose,
                      tuple(line for _, line in item.lines)) for item in before.origin)
    assert "EdgeScan e1;" in _extract(current)
    assert "EdgeScan e2;" in _extract(current)
    assert _tokens(_extract(restored)) == _tokens(_extract(baseline))
