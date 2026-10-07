"""Permanent proposed regression controls: read only the real checkout source."""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest
from match_recovery import source

ROOT = None
for parent in Path(__file__).resolve().parents:
    if (parent / "src/general.cpp").is_file():
        ROOT = parent
        break
if ROOT is None:
    raise RuntimeError("cannot locate actual OpenSMACX source checkout")
GENERAL = ROOT / "src/general.cpp"
LEAF = ROOT / "src/leaf_recoveries.cpp"
BASELINE = "d1bbb0e1b"
SIGNATURES = {
    0x00532A50: "int __stdcall divide_and_increment_quotient_if_remainder",
    0x00592DB0: "int __cdecl point_in_inclusive_rect",
}
OLD_SIGNATURES = {
    0x00532A50: "int __stdcall leaf_00532a50_redirect",
    0x00592DB0: "int __cdecl leaf_00592db0_redirect",
}
HARNESS = r"""
int main() {
    int q = 0, r = 0;
    if (divide_and_increment_quotient_if_remainder(-7, &q, &r, 2) != -2 || q != -2 || r != -1) return 1;
    if (divide_and_increment_quotient_if_remainder(7, &q, &r, 2) != 4 || q != 4 || r != 1) return 2;
    if (divide_and_increment_quotient_if_remainder(-8, &q, &r, 2) != -4 || q != -4 || r != 0) return 3;
    if (divide_and_increment_quotient_if_remainder(-7, &q, &r, -2) != 4 || q != 4 || r != -1) return 4;
    int alias = 0;
    if (divide_and_increment_quotient_if_remainder(7, &alias, &alias, 2) != 4 || alias != 4) return 5;
    if (!point_in_inclusive_rect(0, 0, 0, 0, 3, 3)) return 6;
    if (!point_in_inclusive_rect(3, 3, 0, 0, 3, 3)) return 7;
    if (point_in_inclusive_rect(-1, 0, 0, 0, 3, 3)) return 8;
    if (point_in_inclusive_rect(0, 4, 0, 0, 3, 3)) return 9;
    if (point_in_inclusive_rect(0, 0, 4, 0, 3, 3)) return 10;
    if (point_in_inclusive_rect(0, 0, 0, 4, 3, 3)) return 11;
    if (!point_in_inclusive_rect(INT_MIN, INT_MIN, INT_MIN, INT_MIN, INT_MAX, INT_MAX)) return 12;
    if (!point_in_inclusive_rect(INT_MAX, INT_MAX, INT_MIN, INT_MIN, INT_MAX, INT_MAX)) return 13;
    if (point_in_inclusive_rect(INT_MIN, 0, 0, INT_MIN, INT_MAX, INT_MAX)) return 14;
    if (point_in_inclusive_rect(0, INT_MIN, INT_MIN, 0, INT_MAX, INT_MAX)) return 15;
    if (point_in_inclusive_rect(0, 0, 4, 0, 3, 3)) return 16;
    return 0;
}
"""


def function_bounds(text: str, signature: str) -> tuple[int, int]:
    if text.count(signature) != 1:
        raise AssertionError(f"function signature not unique: {signature}")
    start = text.index(signature)
    brace = text.index("{", start + len(signature))
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return start, pos + 1
    raise AssertionError("unterminated function")


def definition(text: str, signature: str) -> str:
    start, end = function_bounds(text, signature)
    return text[start:end]


def baseline_source() -> str:
    result = subprocess.run(["git", "show", f"{BASELINE}:src/leaf_recoveries.cpp"],
                            cwd=ROOT, capture_output=True, check=True, text=True)
    return result.stdout


def source_envelope(text: str, address: int) -> str:
    marker = f"// ORIGINAL: 0x{address:08X}"
    at = text.index(marker)
    start = text.rfind("/*", 0, at)
    end = text.find("*/", at)
    if start < 0 or end < 0:
        raise AssertionError(f"missing source envelope for 0x{address:08X}")
    return text[start:end + 2]


def expected_archived_envelope(text: str, address: int, old_symbol: str) -> str:
    lines = []
    for line in source_envelope(text, address).splitlines():
        if line.lstrip().startswith("// RECOVERY_ROLE:"):
            line = "// HISTORICAL scaffold role: " + line.split(":", 1)[1].strip()
        elif line.startswith(f"// ORIGINAL: 0x{address:08X}"):
            line = line.replace("// ORIGINAL:", "// historical original marker:", 1)
        elif line.lstrip().startswith("// symbol"):
            line = "// historical scaffold emitted-symbol hint: " + old_symbol
        lines.append(line)
    return "\n".join(lines)


def actual_archived_envelope(text: str, address: int) -> str:
    marker = f"// historical original marker: 0x{address:08X}"
    at = text.index(marker)
    start = text.rfind("/*", 0, at)
    end = text.find("*/", at)
    lines = text[start:end + 2].splitlines()
    # The native writer places the live annotation before the archived marker;
    # the source binding lives there too. Remove only those active additions.
    lines = [line for line in lines
             if not line.startswith(f"// ORIGINAL: 0x{address:08X}")
             and not line.startswith("// RECOVERY_BINDING:")
             and not line.startswith("// symbol    ")]
    return "\n".join(lines)


def compile_and_run(tmp_path: Path, definitions: str) -> subprocess.CompletedProcess:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host GCC/Clang unavailable; native compiler is not run here")
    program = "#include <climits>\n#define __cdecl\n#define __stdcall\n" + definitions + "\n" + HARNESS
    executable = tmp_path / "primitive-utilities-control"
    built = subprocess.run([compiler, "-std=c++11", "-O0", "-x", "c++", "-o", str(executable), "-"],
                           input=program, capture_output=True, check=False, text=True)
    assert built.returncode == 0, built.stderr
    return subprocess.run([str(executable)], capture_output=True, check=False)


def test_migrated_source_has_complete_historical_identity_and_comment_continuity() -> None:
    assert GENERAL.is_file() and LEAF.is_file()
    current_text = GENERAL.read_text(encoding="utf-8")
    old_text = baseline_source()
    old_rows = {row.address: row for row in source.read_text(old_text, LEAF)
                if row.address in SIGNATURES}
    current_rows = {row.address: row for row in source.read(GENERAL)
                    if row.address in SIGNATURES}
    assert set(old_rows) == set(SIGNATURES) == set(current_rows)
    continuity_fields = ("name", "state", "mode", "image_spans", "kind",
                         "byte_exact", "semantic", "exclusion", "levers",
                         "ruled_out", "unrecoverable", "deferred")
    for address, old in old_rows.items():
        current = current_rows[address]
        for field in continuity_fields:
            assert getattr(current, field) == getattr(old, field), (hex(address), field)
        assert current.source_role is not source.SourceRole.SCAFFOLD
        assert current.path == GENERAL
        old_symbol = old.symbol
        assert f"// historical scaffold emitted-symbol hint: {old_symbol}" in current_text
        assert actual_archived_envelope(current_text, address) == expected_archived_envelope(
            old_text, address, old_symbol)
        old_body = definition(old_text, OLD_SIGNATURES[address])
        new_body = definition(current_text, SIGNATURES[address])
        assert old_body.replace(OLD_SIGNATURES[address], SIGNATURES[address], 1) == new_body
    leaf_rows = source.read(LEAF)
    assert not [row for row in leaf_rows if row.address in SIGNATURES]
    for address in SIGNATURES:
        assert "build/recovery/" not in current_rows[address].region


def test_actual_root_source_behavior(tmp_path: Path) -> None:
    actual = GENERAL.read_text(encoding="utf-8")
    bodies = "\n\n".join(definition(actual, signature)
                          for signature in SIGNATURES.values())
    result = compile_and_run(tmp_path, bodies)
    assert result.returncode == 0, result.stderr.decode(errors="replace")


def test_mutant_quotient_increment_condition_is_rejected(tmp_path: Path) -> None:
    actual = GENERAL.read_text(encoding="utf-8")
    bodies = "\n\n".join(definition(actual, signature)
                          for signature in SIGNATURES.values())
    mutant, count = bodies.replace("if (remainder != 0)", "if (remainder > 0)", 1), bodies.count("if (remainder != 0)")
    assert count == 1
    assert compile_and_run(tmp_path, mutant).returncode != 0


def test_mutant_remainder_quotient_write_order_is_rejected(tmp_path: Path) -> None:
    actual = GENERAL.read_text(encoding="utf-8")
    bodies = "\n\n".join(definition(actual, signature)
                          for signature in SIGNATURES.values())
    before = ("    *remainder_out = remainder;\n"
              "    if (remainder != 0) {\n"
              "        ++quotient;\n"
              "    }\n"
              "    *quotient_out = quotient;")
    after = ("    if (remainder != 0) {\n"
             "        ++quotient;\n"
             "    }\n"
             "    *quotient_out = quotient;\n"
             "    *remainder_out = remainder;")
    mutant, count = bodies.replace(before, after, 1), bodies.count(before)
    assert count == 1
    assert compile_and_run(tmp_path, mutant).returncode != 0


def test_mutant_exclusive_left_edge_is_rejected(tmp_path: Path) -> None:
    actual = GENERAL.read_text(encoding="utf-8")
    bodies = "\n\n".join(definition(actual, signature)
                          for signature in SIGNATURES.values())
    mutant, count = bodies.replace("if (x < left", "if (x <= left", 1), bodies.count("if (x < left")
    assert count == 1
    assert compile_and_run(tmp_path, mutant).returncode != 0
