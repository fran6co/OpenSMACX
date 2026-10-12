"""Focused value-result contract and identity-continuity controls for Matrix3."""

from pathlib import Path
import os
import re
import shutil
import subprocess
import sys

import pytest
from match_recovery import binding_source
from match_recovery.source import SourceRole, read_text


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/matrix3.h"
SOURCE = ROOT / "src/matrix3.cpp"
BASE = "ba13b343766c3d50a7a2ccd77d1f8624b6dde677"
SCAFFOLD_REASON = (
    "Bounded numerical receiver view only; shared original object ownership, "
    "containing class, full extent, callers and lifetime remain unproved. "
    "Source consolidation and complete arithmetic bodies do not resolve this "
    "structural obligation."
)

# Source-reader facts captured from the committed ba13b343 baseline. Only the
# three symbols listed separately below are expected to change in this task.
BASELINE_IDENTITIES = (
    (0x006346C0, "sub_6346c0", ((0x006346C0, 0x006346E4),), "game", True, False,
     "?identity@Matrix3@@QAEPAU1@XZ", "body"),
    (0x006346F0, "sub_6346f0", ((0x006346F0, 0x00634712),), "game", True, False,
     "?reset_identity@Matrix3@@QAEXXZ", "body"),
    (0x00634720, "sub_634720", ((0x00634720, 0x006347BE),), "game", False, False,
     "?add_into@Matrix3@@QAEPAU1@PAU1@PBU1@@Z", "file"),
    (0x006347C0, "sub_6347c0", ((0x006347C0, 0x006347F0),), "game", False, False,
     "?add_assign@Matrix3@@QAEXPBU1@@Z", "body"),
    (0x006347F0, "sub_6347f0", ((0x006347F0, 0x006348BC),), "game", False, False,
     "?multiply_assign@Matrix3@@QAEXPBU1@@Z", "file"),
    (0x006348C0, "sub_6348c0", ((0x006348C0, 0x006348E7),), "game", False, False,
     "?scale_assign@Matrix3@@QAEXM@Z", "body"),
    (0x006348F0, "sub_6348f0", ((0x006348F0, 0x00634920),), "game", False, False,
     "?subtract_assign@Matrix3@@QAEXPBU1@@Z", "body"),
    (0x00634B70, "sub_634b70", ((0x00634B70, 0x00634B94),), "game", True, False,
     "?write_identity@Matrix3@@QAEPAU1@XZ", "body"),
    (0x00634980, "sub_634980", ((0x00634980, 0x00634A50),), "game", False, False,
     "?multiply_into@Matrix3@@QAEPAU1@PAU1@PBU1@@Z", "file"),
    (0x00634920, "sub_634920", ((0x00634920, 0x00634975),), "", True, False,
     "?equals@Matrix3@@QAE_NPBM@Z", "body"),
    (0x00634A50, "sub_634a50", ((0x00634A50, 0x00634AC9),), "game", False, False,
     "?transform@Matrix3@@QAEXPAMPBM@Z", "file"),
    (0x00634AD0, "sub_634ad0", ((0x00634AD0, 0x00634B67),), "game", False, False,
     "?scale_into@Matrix3@@QAEPAU1@PAU1@M@Z", "file"),
)
CURRENT_SYMBOLS = {
    0x00634720: "?add@Matrix3@@QAE?AU1@PBU1@@Z",
    0x00634980: "?multiply@Matrix3@@QAE?AU1@PBU1@@Z",
    0x00634AD0: "?scaled@Matrix3@@QAE?AU1@M@Z",
}
EXPECTED_LEVERS = {
    0x00634920: (("lever", "two", "indexed reads, not a hoisted difference. The image keeps `this`"),),
}

HARNESS = r"""
typedef Matrix3 (Matrix3::*Matrix3AddMember)(const Matrix3 *);
typedef Matrix3 (Matrix3::*Matrix3MultiplyMember)(const Matrix3 *);
typedef Matrix3 (Matrix3::*Matrix3ScaledMember)(float);
static Matrix3AddMember add_value_member = &Matrix3::add;
static Matrix3MultiplyMember multiply_value_member = &Matrix3::multiply;
static Matrix3ScaledMember scaled_value_member = &Matrix3::scaled;

static bool same(const Matrix3 &matrix, const float *expected) {
    for (int i = 0; i < 9; ++i)
        if (matrix.cells[i] != expected[i]) return false;
    return true;
}
int main() {
    int i;
    Matrix3 left;
    Matrix3 right;
    const float a[9] = {1,2,3,4,5,6,7,8,9};
    const float descending[9] = {9,8,7,6,5,4,3,2,1};
    const float sum[9] = {10,10,10,10,10,10,10,10,10};
    const float product[9] = {30,24,18,84,69,54,138,114,90};
    const float scaled[9] = {2,4,6,8,10,12,14,16,18};
    for (i = 0; i < 9; ++i) {
        left.cells[i] = a[i];
        right.cells[i] = descending[i];
    }
    Matrix3 separate = (left.*add_value_member)(&right);
    if (!same(separate, sum)) return 1;
    left = left.add(&right); // assignment target aliases the receiver
    if (!same(left, sum)) return 2;

    for (i = 0; i < 9; ++i) left.cells[i] = a[i];
    separate = (left.*multiply_value_member)(&right);
    if (!same(separate, product)) return 3;
    left = left.multiply(&right); // assignment target aliases the receiver
    if (!same(left, product)) return 4;

    for (i = 0; i < 9; ++i) left.cells[i] = a[i];
    separate = (left.*scaled_value_member)(2.0f);
    if (!same(separate, scaled)) return 5;
    left = left.scaled(2.0f); // assignment target aliases the receiver
    if (!same(left, scaled)) return 6;

    for (i = 0; i < 9; ++i) {
        left.cells[i] = a[i];
        right.cells[i] = a[i];
    }
    const float doubled[9] = {2,4,6,8,10,12,14,16,18};
    right = left.add(&right); // assignment target aliases the operand
    if (!same(right, doubled)) return 7;
    return 0;
}
"""


def _body_span(text: str, method: str) -> tuple[int, int]:
    masked = list(text)
    i = 0
    state = "code"
    quote = ""
    while i < len(text):
        if state == "code":
            if text.startswith("//", i):
                masked[i:i + 2] = [" ", " "]
                i += 2
                state = "line"
                continue
            if text.startswith("/*", i):
                masked[i:i + 2] = [" ", " "]
                i += 2
                state = "block"
                continue
            if text[i] in "\"'":
                quote = text[i]
                masked[i] = " "
                i += 1
                state = "literal"
                continue
            i += 1
            continue
        if state == "line":
            if text[i] == "\n":
                state = "code"
            else:
                masked[i] = " "
            i += 1
            continue
        if state == "block":
            if text.startswith("*/", i):
                masked[i:i + 2] = [" ", " "]
                i += 2
                state = "code"
                continue
            if text[i] != "\n":
                masked[i] = " "
            i += 1
            continue
        if state == "literal":
            if text[i] == "\\":
                masked[i] = " "
                if i + 1 < len(text) and text[i + 1] != "\n":
                    masked[i + 1] = " "
                i += 2
                continue
            if text[i] == quote:
                masked[i] = " "
                i += 1
                state = "code"
                continue
            if text[i] != "\n":
                masked[i] = " "
            i += 1
    code = "".join(masked)
    hits = list(re.finditer(r"Matrix3\s*::\s*" + re.escape(method) + r"\s*\(", code))
    assert len(hits) == 1, f"expected exactly one definition of {method}"
    opening = code.find("{", hits[0].end())
    assert opening >= 0
    depth = 0
    for index in range(opening, len(code)):
        if code[index] == "{":
            depth += 1
        elif code[index] == "}":
            depth -= 1
            if depth == 0:
                return opening, index
    raise AssertionError(f"unclosed definition of {method}")


def _replace_body(text: str, method: str, old: str, new: str) -> str:
    opening, closing = _body_span(text, method)
    body = text[opening + 1:closing]
    assert body.count(old) == 1
    return text[:opening + 1] + body.replace(old, new, 1) + text[closing:]


def _compile_and_run(text: str, compiler: str, tmp_path: Path, label: str):
    unit = tmp_path / f"matrix3-{label}.cpp"
    binary = tmp_path / f"matrix3-{label}"
    unit.write_text(text + "\n" + HARNESS)
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         "-iquote", str(ROOT / "src"), str(unit), "-o", str(binary)],
        text=True, capture_output=True, timeout=30)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(binary)], text=True, capture_output=True, timeout=10)


def test_value_signatures_and_non_target_declarations_match_baseline():
    current_header = HEADER.read_text()
    expected = (
        "Matrix3 *identity();",
        "void reset_identity();",
        "Matrix3 add(const Matrix3 *other);",
        "void add_assign(const Matrix3 *other);",
        "void multiply_assign(const Matrix3 *other);",
        "void scale_assign(float scale);",
        "void subtract_assign(const Matrix3 *other);",
        "bool equals(const float *other);",
        "Matrix3 multiply(const Matrix3 *other);",
        "void transform(float *out, const float *in);",
        "Matrix3 scaled(float scale);",
        "Matrix3 *write_identity();",
    )
    declarations = tuple(
        line.strip() for line in current_header.splitlines()
        if "(" in line and ";" in line
    )
    assert declarations == expected
    for signature in (
        "Matrix3 add(const Matrix3 *other);",
        "Matrix3 multiply(const Matrix3 *other);",
        "Matrix3 scaled(float scale);",
    ):
        assert current_header.count(signature) == 1
    assert "add_into(" not in current_header
    assert "multiply_into(" not in current_header
    assert "scale_into(" not in current_header
    assert "typedef Matrix3 (Matrix3::*Matrix3AddMember)(const Matrix3 *);" in HARNESS
    assert "typedef Matrix3 (Matrix3::*Matrix3MultiplyMember)(const Matrix3 *);" in HARNESS
    assert "typedef Matrix3 (Matrix3::*Matrix3ScaledMember)(float);" in HARNESS


def _identity_snapshot(text: str):
    rows = read_text(text, SOURCE.resolve())
    by_address = {row.address: row for row in rows}
    assert len(rows) == 12
    assert len(by_address) == len(rows), "original addresses must be unique"
    expected_by_address = {entry[0]: entry for entry in BASELINE_IDENTITIES}
    assert set(by_address) == set(expected_by_address)

    snapshot = {}
    for address, row in by_address.items():
        baseline = expected_by_address[address]
        assert row.name == baseline[1]
        assert row.image_spans == baseline[2]
        assert row.kind == baseline[3]
        assert row.byte_exact is baseline[4]
        assert row.semantic is baseline[5]
        assert row.source_role is SourceRole.SCAFFOLD
        assert row.source_role_reason == SCAFFOLD_REASON
        assert row.state.value == "implemented"
        assert row.mode.value == baseline[7]
        expected_symbol = CURRENT_SYMBOLS.get(address, baseline[6])
        assert row.symbol == expected_symbol
        lessons = tuple((lesson.kind, lesson.key, lesson.prose) for lesson in row.origin)
        if address in EXPECTED_LEVERS:
            assert lessons == EXPECTED_LEVERS[address]
        else:
            assert all(lesson[0] != "lever" for lesson in lessons)
        snapshot[address] = row
    return snapshot


def test_original_claims_symbols_roles_and_levers_match_ba13_baseline():
    current = SOURCE.read_text()
    rows = _identity_snapshot(current)
    assert set(CURRENT_SYMBOLS) == {0x00634720, 0x00634980, 0x00634AD0}
    for address in CURRENT_SYMBOLS:
        row = rows[address]
        assert row.byte_exact is False
        assert row.source_role is SourceRole.SCAFFOLD
    for historical in ("Historical explicit-pointer binding: v1", "// TRIED:", "// LEVER:"):
        assert historical in current


def test_removed_original_marker_and_wrong_symbol_are_rejected():
    current = SOURCE.read_text()
    marker = "// ORIGINAL: 0x006346F0 sub_6346f0 0x006346F0-0x00634712 BYTE_EXACT"
    role = "// RECOVERY_ROLE: SCAFFOLD " + SCAFFOLD_REASON
    with pytest.raises(AssertionError):
        _identity_snapshot(current.replace(role + "\n" + marker + "\n", "", 1))

    wrong_symbol = current.replace(
        "// symbol    ?identity@Matrix3@@QAEPAU1@XZ",
        "// symbol    ?identity@Matrix3@@QAEPAU2@XZ",
        1,
    )
    with pytest.raises(AssertionError):
        _identity_snapshot(wrong_symbol)


def test_binding_rationale_is_adjacent_and_keeps_structural_uncertainty():
    text = SOURCE.read_text()
    rows = read_text(text, SOURCE.resolve())
    by_address = {row.address: row for row in rows}
    parsed = binding_source.parse(text, rows)
    for address in CURRENT_SYMBOLS:
        row = by_address[address]
        rationale = parsed[row.line]
        assert "first stack slot" in rationale["reason"]
        assert "explicit output-pointer API" in rationale["reason"]
        assert "original class identity" in rationale["reason"]
        assert "complete extent" in rationale["reason"]
        assert "lifetime" in rationale["reason"]

    marker = "// ORIGINAL: 0x00634720 sub_634720 0x00634720-0x006347BE FILE"
    separated = text.replace(
        marker + "\n// RECOVERY_BINDING:",
        marker + "\n// intervening comment\n// RECOVERY_BINDING:",
        1,
    )
    with pytest.raises(ValueError, match="immediately after its ORIGINAL marker"):
        binding_source.parse(separated, read_text(separated, SOURCE.resolve()))


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_source_value_results_and_alias_assignments(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} not installed")
    result = _compile_and_run(SOURCE.read_text(), compiler, tmp_path, "positive")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize(("method", "before", "after"), (
    ("add", "other->cells[row * 3 + column] +", "other->cells[row * 3 + column] -"),
    ("multiply", "other->cells[depth * 3 + column]", "other->cells[column * 3 + depth]"),
    ("multiply", "result.cells[row * 3 + column] = sum;",
     "result.cells[row * 3 + column] = 0.0f;"),
    ("scaled", "scale * cells[i]", "scale + cells[i]"),
))
def test_value_result_source_mutants_fail_host_controls(
    tmp_path, compiler_name, method, before, after
):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} not installed")
    mutant = _replace_body(SOURCE.read_text(), method, before, after)
    result = _compile_and_run(mutant, compiler, tmp_path, f"mutant-{method}")
    assert result.returncode != 0, f"{method} mutation escaped value-result host control"


@pytest.mark.parametrize("damage", [None, "zero_product_store"])
def test_actual_value_results_and_aliases_on_vc6_both_configured_profiles(
    tmp_path: Path, damage: str | None,
) -> None:
    """Bounded value-result/alias control, not x87 or whole-game proof."""
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from match_recovery.source_inventory import image_pin
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native controls require a coordinator-owned compiler lease")
    assert image_pin(ROOT) == "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")

    source_text = SOURCE.read_text()
    definitions = []
    for method in ("add", "multiply", "scaled"):
        start = re.search(r"(?m)^Matrix3 Matrix3::" + method + r"\s*\(", source_text)
        assert start is not None
        _, closing = _body_span(source_text, method)
        definition = source_text[start.start():closing + 1]
        definitions.append(definition)
    methods = HEADER.read_text() + "\n" + "\n".join(definitions)
    if damage == "zero_product_store":
        old = "result.cells[row * 3 + column] = sum;"
        assert methods.count(old) == 1
        methods = methods.replace(old,
            "result.cells[row * 3 + column] = 0.0f;", 1)
    harness = methods + "\n" + HARNESS

    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    for index, flags in enumerate(settings.flag_sets):
        source = tmp_path / ("matrix3_values_%d.cpp" % index)
        source.write_text(harness)
        object_bytes = compile_object(
            source, command, (*flags, *settings.layout_flags)
        )
        obj = tmp_path / ("matrix3_values_%d.obj" % index)
        obj.write_bytes(object_bytes)
        executable = tmp_path / ("matrix3_values_%d.exe" % index)
        linked = run_bounded(
            [sys.executable, str(wrapper), "--tool=link", "/nologo",
             "/subsystem:console", "/incremental:no", "/out:" + str(executable),
             str(obj), "kernel32.lib"], cwd=tmp_path, timeout=60,
        )
        assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
        result = subprocess.run(
            ["wine32", str(executable)], env=environment(),
            capture_output=True, text=True, timeout=30,
        )
        expected = {None: 0, "zero_product_store": 3}[damage]
        assert result.returncode == expected, result.stdout + result.stderr
