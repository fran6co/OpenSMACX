"""Executable checks for the bounded Scroll static-default record.

The harnesses compile the actual record declaration, initializer, and
contiguous popup producer statements. They do not claim original aggregate
identity or allocation extent beyond the eleven observed words.
"""

from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/scroll.h"
SCROLL = ROOT / "src/scroll.cpp"
POPUP = ROOT / "src/popup.cpp"
RECORD = "struct ScrollCloseStaticDefaultsData {"
GLOBAL = "ScrollCloseStaticDefaultsData ScrollCloseStaticDefaults = {"
POPUPS = "void __cdecl popups_normal()"


def _block(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated block: {signature}")


def _record(header: str) -> str:
    start = header.index(RECORD)
    end = header.index("\n};", start) + 3
    return header[start:end]


def _global_definition(scroll: str) -> str:
    start = scroll.index(GLOBAL)
    end = scroll.index(";", start) + 1
    return scroll[start:end]


def _producer_statements(popup: str) -> list[str]:
    body = _block(popup, POPUPS)
    return [
        " ".join(line.strip().split())
        for line in body.splitlines()
        if "ScrollCloseStaticDefaults." in line
    ]


def _program(record: str, definition: str, body: str) -> str:
    return f'''#include <cassert>
#include <cstddef>
#include <cstdint>
using uint32_t = std::uint32_t;
{record}
static_assert(sizeof(ScrollCloseStaticDefaultsData) == 44, "observed word window size");
static_assert(offsetof(ScrollCloseStaticDefaultsData, paging) == 0, "paging offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, field_A34) == 4, "A34 offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, field_A30) == 8, "A30 offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, border_color) == 12, "border offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, range_maximum) == 16, "range offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, field_A48) == 20, "A48 offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, color) == 24, "color offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, field_A64) == 28, "A64 offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, bevel_thickness) == 32, "thickness offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, bevel_upper) == 36, "upper offset");
static_assert(offsetof(ScrollCloseStaticDefaultsData, bevel_lower) == 40, "lower offset");
{definition}
int main() {{
{body}
}}
'''


def _compile_run(compiler: str, program: str, tmp_path: Path, name: str) -> subprocess.CompletedProcess[str]:
    cpp = tmp_path / f"{name}.cpp"
    exe = tmp_path / name
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        capture_output=True,
        text=True,
        timeout=30,
        check=False,
    )
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(exe)], capture_output=True, text=True, timeout=30, check=False)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_scroll_static_defaults_have_observed_offsets_and_initial_words(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    record = _record(HEADER.read_text(encoding="utf-8"))
    definition = _global_definition(SCROLL.read_text(encoding="utf-8"))
    body = """
  assert(ScrollCloseStaticDefaults.paging == 2);
  assert(ScrollCloseStaticDefaults.field_A34 == 0xffffffffu);
  assert(ScrollCloseStaticDefaults.field_A30 == 250);
  assert(ScrollCloseStaticDefaults.border_color == -1);
  assert(ScrollCloseStaticDefaults.range_maximum == 100);
  assert(ScrollCloseStaticDefaults.field_A48 == 247);
  assert(ScrollCloseStaticDefaults.color == 247);
  assert(ScrollCloseStaticDefaults.field_A64 == 7);
  assert(ScrollCloseStaticDefaults.bevel_thickness == 2);
  assert(ScrollCloseStaticDefaults.bevel_upper == 255);
  assert(ScrollCloseStaticDefaults.bevel_lower == 248);
"""
    ran = _compile_run(compiler, _program(record, definition, body), tmp_path, "scroll_static_defaults")
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_actual_popup_producer_targets_and_order_are_executable(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    record = _record(HEADER.read_text(encoding="utf-8"))
    definition = _global_definition(SCROLL.read_text(encoding="utf-8"))
    producer = _producer_statements(POPUP.read_text(encoding="utf-8"))
    assert producer == [
        "ScrollCloseStaticDefaults.bevel_thickness = 0;",
        "ScrollCloseStaticDefaults.border_color = -1;",
        "ScrollCloseStaticDefaults.field_A64 = 0x41;",
        "ScrollCloseStaticDefaults.field_A48 = 0x41;",
        "ScrollCloseStaticDefaults.color = 0x41;",
    ]
    body = """
  ScrollCloseStaticDefaults = ScrollCloseStaticDefaultsData{
      301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311};
""" + "\n".join("  " + statement for statement in producer) + """
  assert(ScrollCloseStaticDefaults.paging == 301);
  assert(ScrollCloseStaticDefaults.field_A34 == 302);
  assert(ScrollCloseStaticDefaults.field_A30 == 303);
  assert(ScrollCloseStaticDefaults.border_color == -1);
  assert(ScrollCloseStaticDefaults.range_maximum == 305);
  assert(ScrollCloseStaticDefaults.field_A48 == 0x41);
  assert(ScrollCloseStaticDefaults.color == 0x41);
  assert(ScrollCloseStaticDefaults.field_A64 == 0x41);
  assert(ScrollCloseStaticDefaults.bevel_thickness == 0);
  assert(ScrollCloseStaticDefaults.bevel_upper == 310);
  assert(ScrollCloseStaticDefaults.bevel_lower == 311);
"""
    ran = _compile_run(compiler, _program(record, definition, body), tmp_path, "scroll_popup_defaults")
    assert ran.returncode == 0, ran.stdout + ran.stderr


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_offset_initializer_and_producer_mutations_are_detected(
    compiler_name: str, tmp_path: Path
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    record = _record(HEADER.read_text(encoding="utf-8"))
    definition = _global_definition(SCROLL.read_text(encoding="utf-8"))
    producer = _producer_statements(POPUP.read_text(encoding="utf-8"))

    moved = record.replace("  int border_color;\n  int range_maximum;",
                           "  int range_maximum;\n  int border_color;")
    offset_program = _program(moved, definition, "")
    offset_cpp = tmp_path / "scroll_wrong_offset.cpp"
    offset_cpp.write_text(offset_program, encoding="utf-8")
    offset_build = subprocess.run(
        [compiler, "-std=c++11", "-Wall", "-Wextra", "-Werror", str(offset_cpp), "-o", str(tmp_path / "wrong_offset")],
        capture_output=True, text=True, timeout=30, check=False,
    )
    assert offset_build.returncode != 0

    changed_definition = definition.replace("2, 0xFFFFFFFFU", "3, 0xFFFFFFFFU", 1)
    wrong_initial = _compile_run(
        compiler,
        _program(record, changed_definition, "assert(ScrollCloseStaticDefaults.paging == 2);"),
        tmp_path,
        "scroll_wrong_initial",
    )
    assert wrong_initial.returncode != 0

    wrong_producer = [line.replace("field_A64", "field_A48") if "field_A64" in line else line for line in producer]
    assert wrong_producer != producer
    producer_body = "\n".join("  " + line for line in wrong_producer)
    producer_body += "\n  assert(ScrollCloseStaticDefaults.field_A64 == 0x41);"
    wrong_target = _compile_run(
        compiler, _program(record, definition, producer_body), tmp_path, "scroll_wrong_producer_target"
    )
    assert wrong_target.returncode != 0
