"""Permanent owner-transfer checks for Win::set_field_128.

This test reads only the checkout's product source and the named historical
Git source. When copied into recovery/tests, it resolves the project root from
its own location; it does not consume a proposal manifest or overlay.
"""
from __future__ import annotations

import json
import os
import re
import shlex
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from match_recovery.source import function_line as source_function_line, read, read_text


def function_line(source: str, name: str) -> int:
    # Give the source locator an explicit file, not a large string that its
    # legacy path-or-text dispatch may attempt to stat as a filename.
    with tempfile.TemporaryDirectory() as directory:
        path = Path(directory) / "source.cpp"
        path.write_text(source)
        return source_function_line(path, name)

ADDRESS = 0x005F05C0
HISTORY_REVISION = "2c29aa969"
OLD_REDIRECT = "?field_accessor_005f05c0_redirect@@YIXPAX0H@Z"


def _root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "CMakeLists.txt").is_file() and (parent / "src/win.cpp").is_file():
            return parent
    raise AssertionError("could not locate OpenSMACX source root")


def _envelope(text: str, address: int, function: str) -> str:
    marker = f"// ORIGINAL: 0x{address:08X}"
    assert text.count(marker) == 1
    marker_at = text.index(marker)
    start = text.rfind("/*", 0, marker_at)
    assert start >= 0
    line = function_line(text, function)
    lines = text.splitlines(keepends=True)
    open_at = sum(map(len, lines[:line - 1]))
    brace = text.find("{", open_at)
    assert brace >= 0
    depth = 0
    for pos in range(brace, len(text)):
        if text[pos] == "{":
            depth += 1
        elif text[pos] == "}":
            depth -= 1
            if depth == 0:
                return text[start:pos + 1]
    raise AssertionError(f"unterminated {function}")


def _comment_lines(envelope: str) -> list[str]:
    comment = envelope[:envelope.index("*/") + 2]
    lines = []
    for line in comment.splitlines():
        if ("ORIGINAL: 0x005F05C0" in line
                or "RECOVERY_ROLE:" in line
                or line.lstrip().startswith("// symbol")
                or "RECOVERY_BINDING:" in line):
            continue
        lines.append(line.rstrip())
    return lines


def _method(source: str) -> str:
    line = function_line(source, "Win::set_field_128")
    lines = source.splitlines(keepends=True)
    start = sum(map(len, lines[:line - 1]))
    brace = source.find("{", start)
    assert brace >= 0
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[start:pos + 1]
    raise AssertionError("unterminated Win::set_field_128")


def _historical_source(root: Path) -> str:
    result = subprocess.run(
        ["git", "show", f"{HISTORY_REVISION}:src/field_accessors.cpp"],
        cwd=root, capture_output=True, text=True, check=False)
    assert result.returncode == 0, result.stderr
    return result.stdout


def test_migration_retains_history_and_records_observed_owner_symbol():
    root = _root()
    historical_text = _historical_source(root)
    historical = next(r for r in read_text(
        historical_text, root / "src/field_accessors.cpp")
                      if r.address == ADDRESS)
    current_text = (root / "src/win.cpp").read_text()
    current = next(r for r in read(root / "src/win.cpp")
                   if r.address == ADDRESS)

    assert historical.name == current.name == "sub_5f05c0"
    assert historical.image_spans == current.image_spans == ((ADDRESS, ADDRESS + 13),)
    assert historical.kind == current.kind == "game"
    assert historical.byte_exact is current.byte_exact is True
    assert historical.semantic is current.semantic is False
    assert historical.symbol == OLD_REDIRECT
    assert current.symbol == "?set_field_128@Win@@QAEXH@Z"
    assert current.source_role.value == "unclassified"
    for field in ("mode", "state", "kind", "exclusion", "byte_exact",
                  "semantic", "recipe", "levers", "ruled_out",
                  "unrecoverable", "deferred"):
        assert getattr(current, field) == getattr(historical, field), field
    assert not any(r.address == ADDRESS
                   for r in read(root / "src/field_accessors.cpp"))

    old_envelope = _envelope(historical_text, ADDRESS,
                             "field_accessor_005f05c0_redirect")
    new_envelope = _envelope(current_text, ADDRESS, "Win::set_field_128")
    assert _comment_lines(new_envelope) == _comment_lines(old_envelope)
    assert "// kind      game" in new_envelope

    binding = next(line for line in new_envelope.splitlines()
                   if "RECOVERY_BINDING: v1 " in line)
    detail = json.loads(binding.split("RECOVERY_BINDING: v1 ", 1)[1])
    joined = " ".join(detail["evidence"]) + " " + detail["reason"]
    assert OLD_REDIRECT in joined
    assert "caller" in joined and "owner" in joined
    assert "unqualified" in joined


def test_method_definition_is_real_code_not_a_comment_or_neighbor_method():
    root = _root()
    source = (root / "src/win.cpp").read_text()
    method = _method(source)
    assert re.search(r"\bfield_128_\s*=\s*static_cast<uint32_t>\(value\)", method)
    assert "reinterpret_cast" not in method
    assert "field_accessor_005f05c0_redirect" not in method

    fake = "// void Win::set_field_128(int value) { field_128_ = value; }\n"
    with pytest.raises(ValueError):
        function_line(fake, "Win::set_field_128")


def _build_and_run(root: Path, body: str, expected_success: bool) -> None:
    compiler = shlex.split(os.environ.get("CXX", "g++"))
    if not compiler or shutil.which(compiler[0]) is None:
        pytest.skip("host C++ compiler unavailable")
    source = r'''#include <stdint.h>
#include <assert.h>
struct Win {
  uint32_t before_;
  uint32_t field_124_;
  uint32_t field_128_;
  uint32_t after_;
  void set_field_128(int value);
};
''' + body + r'''
int main() {
  Win window = {0x11111111u, 0x22222222u, 0x33333333u, 0x44444444u};
  window.set_field_128(-1);
  assert(window.before_ == 0x11111111u);
  assert(window.field_124_ == 0x22222222u);
  assert(window.field_128_ == 0xFFFFFFFFu);
  assert(window.after_ == 0x44444444u);
  window.set_field_128(0x12345678);
  assert(window.field_128_ == 0x12345678u);
  return 0;
}
'''
    with tempfile.TemporaryDirectory(prefix="win-field-owner-") as temp:
        cpp = Path(temp) / "owner.cpp"
        exe = Path(temp) / "owner"
        cpp.write_text(source)
        built = subprocess.run(
            compiler + ["-std=c++98", "-Wall", "-Wextra", "-Werror",
                        str(cpp), "-o", str(exe)],
            capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True)
        assert (ran.returncode == 0) is expected_success, ran.stdout + ran.stderr


def test_actual_product_method_compiles_runs_and_rejects_comment_fake_mutants():
    root = _root()
    method = _method((root / "src/win.cpp").read_text())
    _build_and_run(root, method, True)

    comment_only = "void Win::set_field_128(int value) { (void)value; /* field_128_ = value; */ }"
    _build_and_run(root, comment_only, False)

    wrong_member = method.replace("field_128_", "field_124_")
    assert wrong_member != method
    _build_and_run(root, wrong_member, False)
