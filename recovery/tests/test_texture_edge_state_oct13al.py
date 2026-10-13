"""Actual-source recurrence controls for Texture's transparent polygon edges.

The host fixture extracts the complete current draw_trans2 body and drives its
real EdgeScan locals with a narrow deterministic setup_edge callback. It checks
row-state behavior, not native ABI, x87 equivalence, or full pixel equivalence.
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
from recovery.tests.test_texture_shaded_edge_record_oct13an import _strip_authorized_edge_delta


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/texture.cpp"
FIXTURE = ROOT / "recovery/tests/fixtures/texture_sub61f0d0_actual_host.cpp"
BASE = "202000126fbfdb8e8dcdd0b9409d18fcf82cfa1d"
TASK = "texture-edge-state-owner-oct13al"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
SIGNATURE = "int Texture::draw_trans2(Buffer *a1, Vert *a2, int a3)"
INSERTION = "// ACTUAL_DRAW_TRANS2_INSERTION"


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
    raise AssertionError(f"unterminated function: {signature}")


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


def _history(row):
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin)


def _replace_once(source: str, old: str, new: str, label: str) -> str:
    assert source.count(old) == 1, (label, source.count(old), old)
    return source.replace(old, new, 1)


def _expected_current_body(old: str) -> str:
    replacements = (
        ("""                EdgeLocal leftEdge;
                leftEdge.setupSentinel = -1;
                int setupResult = this->setup_edge(reinterpret_cast<EdgeScan *>(&leftEdge), minYIdx);""",
         """                EdgeScan leftEdge;
                leftEdge.f0 = -1;
                int setupResult = this->setup_edge(&leftEdge, minYIdx);""", "left EdgeScan"),
        ("""                    EdgeLocal rightEdge;
                    rightEdge.setupSentinel = 1;
                    setupResult = this->setup_edge(reinterpret_cast<EdgeScan *>(&rightEdge), minYIdx);""",
         """                    EdgeScan rightEdge;
                    rightEdge.f0 = 1;
                    setupResult = this->setup_edge(&rightEdge, minYIdx);""", "right EdgeScan"),
        ("this->setup_edge(reinterpret_cast<EdgeScan *>(&leftEdge), leftEdge.refreshIdx)",
         "this->setup_edge(&leftEdge, leftEdge.id)", "left refresh"),
        ("this->setup_edge(reinterpret_cast<EdgeScan *>(&rightEdge), rightEdge.refreshIdx)",
         "this->setup_edge(&rightEdge, rightEdge.id)", "right refresh"),
        ("""                                    leftEdge.accumA += leftEdge.stepA;
                                    leftEdge.accumB += leftEdge.stepB;
                                    leftEdge.accumC += leftEdge.stepC;
                                    leftEdge.threshold += leftEdge.stepD;
                                    if (leftEdge.threshold > 0) {
                                        leftEdge.accumC += leftEdge.stepCExtra;
                                        leftEdge.threshold -= leftEdge.accumD;
                                    }""",
         """                                    leftEdge.accumA += leftEdge.deltaA;
                                    leftEdge.accumB += leftEdge.deltaB;
                                    leftEdge.accumC += leftEdge.deltaC;
                                    leftEdge.fracAccum += leftEdge.fracDelta;
                                    if (leftEdge.fracAccum > 0) {
                                        leftEdge.accumC += leftEdge.extraDeltaC;
                                        leftEdge.fracAccum -= leftEdge.fracSub;
                                    }""", "left recurrence"),
        ("""                                        rightEdge.accumB += rightEdge.stepA;
                                        rightEdge.accumA += rightEdge.stepB;
                                        rightEdge.accumC += rightEdge.stepC;
                                        rightEdge.threshold += rightEdge.stepD;
                                        if (rightEdge.threshold > 0) {
                                            rightEdge.accumC += rightEdge.stepCExtra;
                                            rightEdge.threshold -= rightEdge.accumD;
                                        }""",
         """                                        rightEdge.accumA += rightEdge.deltaA;
                                        rightEdge.accumB += rightEdge.deltaB;
                                        rightEdge.accumC += rightEdge.deltaC;
                                        rightEdge.fracAccum += rightEdge.fracDelta;
                                        if (rightEdge.fracAccum > 0) {
                                            rightEdge.accumC += rightEdge.extraDeltaC;
                                            rightEdge.fracAccum -= rightEdge.fracSub;
                                        }""", "right recurrence"),
    )
    for old_part, new_part, label in replacements:
        old = _replace_once(old, old_part, new_part, label)
    return old


def _assert_source_delta(current: str, baseline: str) -> None:
    current = _strip_authorized_edge_delta(current, baseline)
    old_body = _extract(baseline)
    current_body = _extract(current)
    assert _tokens(current_body) == _tokens(_expected_current_body(old_body)), (
        "draw_trans2 must contain only the evidence-backed EdgeScan conversion")
    current = current.replace(current_body, old_body, 1)

    edge_local = """struct EdgeLocal {
    int setupSentinel;
    int counter;
    int refreshIdx;
    int accumA;
    int accumB;
    int stepA;
    int stepB;
    int accumC;
    int stepC;
    int stepCExtra;
    int accumD;
    int stepD;
    int threshold;
};

"""
    marker = "// ORIGINAL: 0x00619D80 Texture::draw_trans2"
    assert baseline.count(edge_local + marker) == 1
    assert edge_local not in current
    current = current.replace(marker, edge_local + marker, 1)
    assert _tokens(current) == _tokens(baseline), "only EdgeLocal removal and draw_trans2 delta"

    structural = """// STRUCTURAL EVIDENCE: Pinned setup_edge 0x624D80 writes EdgeScan offsets
// +0x28/+0x2c/+0x30 as fracAccum/fracDelta/fracSub; draw_trans2 row blocks
// 0x619FE2-0x61A000 and 0x61A039-0x61A074 confirm the matching DDA updates.
// This supersedes the former local-overlay interpretation above.
"""
    assert current.count(structural) == 1
    current_without_note = current.replace(structural, "", 1)
    comments = lambda text: re.findall(r"//[^\n]*|/\*.*?\*/", text, re.S)
    assert comments(current_without_note) == comments(baseline), (
        "historical source comments/TRIED notes must remain unchanged")


SETUP_STUB = """int Texture::setup_edge(EdgeScan *, int) {
    ++setup_edge_calls;
    active_buffer->events_.push_back(2);
    return 0;
}"""


SCRIPTED_SETUP = r"""
static int edge_wrap_case;
struct EdgeSnapshot {
    int id, counter, accumA, accumB, deltaA, deltaB, accumC;
    int fracAccum, fracDelta, fracSub, extraDeltaC;
};
static EdgeScan *script_left;
static EdgeScan *script_right;
static EdgeSnapshot seen_left, seen_right;
static EdgeSnapshot snapshot_edge(EdgeScan *e) {
    EdgeSnapshot s = {e->id, e->counter, e->accumA, e->accumB,
                      e->deltaA, e->deltaB, e->accumC, e->fracAccum,
                      e->fracDelta, e->fracSub, e->extraDeltaC};
    return s;
}
int Texture::setup_edge(EdgeScan *edge, int index) {
    ++setup_edge_calls;
    active_buffer->events_.push_back(2);
    if (setup_edge_calls == 1) {
        script_left = edge;
        assert(edge->f0 == -1 && index == 0);
        edge->counter = 2; edge->id = 3;
        edge->accumA = 100; edge->accumB = 200; edge->accumC = 300;
        edge->deltaA = 5; edge->deltaB = 7; edge->deltaC = 11;
        edge->extraDeltaC = 13; edge->fracSub = 4;
        edge->fracAccum = edge_wrap_case == 1 || edge_wrap_case == 2 ? -1 : -2;
        edge->fracDelta = edge_wrap_case == 1 ? 2 : 1;
        return 1;
    }
    if (setup_edge_calls == 2) {
        script_right = edge;
        assert(edge->f0 == 1 && index == 0);
        edge->counter = 2; edge->id = 1;
        edge->accumA = 1000; edge->accumB = 2000; edge->accumC = 3000;
        edge->deltaA = 17; edge->deltaB = 19; edge->deltaC = 23;
        edge->extraDeltaC = 29; edge->fracSub = 9;
        edge->fracAccum = edge_wrap_case == 1 ? 4 : (edge_wrap_case == 2 ? -1 : -2);
        edge->fracDelta = 1;
        return 1;
    }
    if (edge == script_left) {
        assert(setup_edge_calls == 3 && index == 3);
        seen_left = snapshot_edge(edge);
        return 1; // left refresh success continues at the original rightEdgeStep label
    }
    assert(edge == script_right && setup_edge_calls == 4 && index == 1);
    seen_right = snapshot_edge(edge);
    return 0; // right refresh termination retains the original branch polarity
}
"""


EDGE_TEST = r"""
static void test_edge_row_recurrences(int case_id) {
    reset_case();
    edge_wrap_case = case_id;
    texture.borrowed_ = 1;
    buffer.stride_ = 1;
    buffer.rect1_.left = -100;
    buffer.rect1_.right = 10000;
    buffer.rect1_.top = 100;
    buffer.rect1_.bottom = 2;
    input_xy[0].x = 0; input_xy[0].y = 0;
    input_xy[1].x = 1; input_xy[1].y = 2;
    input_xy[2].x = 2; input_xy[2].y = 2;
    input_xy[3].x = 3; input_xy[3].y = 0;
    std::memset(pixels, 0xA5, sizeof(pixels));
    const int result = texture.draw_trans2(&buffer, input_xy, 0);
    assert(result == 0 && setup_edge_calls == 4);
    assert(script_left != script_right && seen_left.id == 3 && seen_right.id == 1);
    assert(seen_left.counter == 0 && seen_right.counter == 0);

    const int left_c = case_id == 1 ? 324 : 311;
    const int right_c = case_id == 1 ? 3052 : 3023;
    const int left_fraction = case_id == 1 ? -3 : (case_id == 2 ? 0 : -1);
    const int right_fraction = case_id == 1 ? -4 : (case_id == 2 ? 0 : -1);
    assert(seen_left.accumA == 105 && seen_left.accumB == 207 &&
           seen_left.accumC == left_c && seen_left.fracAccum == left_fraction);
    assert(seen_right.accumA == 1017 && seen_right.accumB == 2019 &&
           seen_right.accumC == right_c && seen_right.fracAccum == right_fraction);
    // Second row published the post-first-step state for each independent edge.
    assert(cell_slot_4f8 == 105 && cell_slot_53c == 207 && cell_span_right == left_c);
    assert(cell_slot_4c4 == 1017 && cell_slot_530 == 2019 && cell_span_left == right_c);
    assert(buffer.events_.size() == 6 && buffer.events_[0] == 1 &&
           buffer.events_[5] == 3);
    for (unsigned int i = 0; i < sizeof(pixels); ++i) assert(pixels[i] == 0xA5);
}
static void test_edge_recurrences() {
    test_edge_row_recurrences(0); // both fractions remain nonpositive
    test_edge_row_recurrences(1); // both fractions cross positive and subtract fracSub
    test_edge_row_recurrences(2); // exact zero is nonpositive and must not wrap
}
"""


def _translation_unit(body: str) -> str:
    fixture = FIXTURE.read_text(encoding="utf-8")
    assert fixture.count(INSERTION) == 1
    assert fixture.count(SETUP_STUB) == 1
    fixture = fixture.replace(SETUP_STUB, SCRIPTED_SETUP, 1)
    fixture = fixture.replace(INSERTION, body, 1)
    function_anchor = "int main() {"
    assert fixture.count(function_anchor) == 1
    fixture = fixture.replace(function_anchor, EDGE_TEST + "\n" + function_anchor, 1)
    old_main = _extract(fixture, "int main()")
    fixture = fixture.replace(
        old_main, "int main() { test_edge_recurrences(); return 0; }", 1)
    return fixture


def _build_run(source: str, compiler_name: str, tmp_path: Path, name: str) -> subprocess.CompletedProcess:
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
def test_literal_draw_trans2_edge_recurrences_and_refreshes(compiler_name, tmp_path):
    source = SOURCE.read_text(encoding="utf-8")
    body = _extract(source)
    assert "EdgeScan leftEdge;" in body and "EdgeLocal" not in body
    assert "leftEdge.fracAccum += leftEdge.fracDelta;" in body
    assert "leftEdge.fracAccum -= leftEdge.fracSub;" in body
    assert "rightEdge.accumA += rightEdge.deltaA;" in body
    result = _build_run(_translation_unit(body), compiler_name, tmp_path, "texture-edge-good")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(("old", "new", "name"), (
    ("rightEdge.accumA += rightEdge.deltaA;\n                                        rightEdge.accumB += rightEdge.deltaB;",
     "rightEdge.accumA += rightEdge.deltaB;\n                                        rightEdge.accumB += rightEdge.deltaA;",
     "right-ab-delta-swap"),
    ("leftEdge.fracAccum -= leftEdge.fracSub;",
     "leftEdge.fracAccum -= leftEdge.fracDelta;", "left-wrong-fraction-subtract"),
    ("rightEdge.fracAccum -= rightEdge.fracSub;",
     "rightEdge.fracAccum -= rightEdge.fracDelta;", "right-wrong-fraction-subtract"),
    ("if (leftEdge.fracAccum > 0) {", "if (leftEdge.fracAccum >= 0) {",
     "left-zero-boundary-wrap"),
    ("if (rightEdge.fracAccum > 0) {", "if (rightEdge.fracAccum >= 0) {",
     "right-zero-boundary-wrap"),
))
def test_compile_success_edge_recurrence_mutants_fail_runtime(
    compiler_name, tmp_path, old, new, name
):
    body = _extract(SOURCE.read_text(encoding="utf-8"))
    assert body.count(old) == 1
    mutant = body.replace(old, new, 1)
    result = _build_run(_translation_unit(mutant), compiler_name, tmp_path, name)
    assert result.returncode != 0, f"runtime control missed {name}"


def test_task_base_source_delta_preserves_texture_history():
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    assert len(task.subjects) == 2
    assert all(subject.image_sha256 == PIN and subject.address_space == "pe:va"
               for subject in task.subjects)
    baseline_bytes = subprocess.check_output(
        ["git", "show", f"{BASE}:src/texture.cpp"], cwd=ROOT, timeout=30)
    baseline = baseline_bytes.decode("utf-8")
    current = SOURCE.read_text(encoding="utf-8")
    _assert_source_delta(current, baseline)

    old_rows = {row.address: row for row in read_text(baseline, SOURCE)}
    current_rows = {row.address: row for row in read_file(SOURCE)}
    assert old_rows.keys() == current_rows.keys()
    for address, previous in old_rows.items():
        actual = current_rows[address]
        for field in fields(previous):
            if field.name in {"line", "region", "origin"}:
                continue
            assert getattr(actual, field.name) == getattr(previous, field.name), (
                hex(address), field.name)
        assert _history(actual) == _history(previous), (hex(address), "historical annotation")
