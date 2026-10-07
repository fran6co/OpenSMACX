"""Actual-source controls for bounded arithmetic and pointer-table owners.

The fixtures describe only accessed fields. They do not assert original class
identity, complete allocation extent, or caller domain.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SPAN_SOURCE = ROOT / "src/span_math.cpp"
SPAN_HEADER = ROOT / "src/span_math.h"
TABLE_SOURCE = ROOT / "src/table_storage.cpp"
TABLE_HEADER = ROOT / "src/table_storage.h"


def _mask(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                              for char in match[0]), source)


def extract(source, signature):
    masked = _mask(source)
    needle = _mask(signature)
    assert masked.count(needle) == 1, "ambiguous actual definition: " + signature
    start = masked.find(needle)
    assert start >= 0, "missing actual definition: " + signature
    opening = masked.find("{", start)
    assert opening >= 0
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{": depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unbalanced actual definition: " + signature)


def extract_struct(source, name):
    signature = "struct " + name + " {"
    masked = _mask(source)
    start = masked.find(signature)
    assert start >= 0, "missing actual source type: " + name
    opening = masked.find("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{": depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                end = masked.find(";", index)
                assert end >= 0
                return source[start:end + 1]
    raise AssertionError("unbalanced actual source type: " + name)


def compile_run(compiler, translation, expected=True):
    with tempfile.TemporaryDirectory(prefix="leaf-owner-host-") as temp:
        cpp = Path(temp) / "controls.cpp"
        exe = Path(temp) / "controls"
        cpp.write_text(translation, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-O0", "-Wall", "-Wextra",
             str(cpp), "-o", str(exe)], capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True)
        assert (ran.returncode == 0) == expected, ran.stdout + ran.stderr


SPAN_PREAMBLE = r'''#include <assert.h>
#include <stdint.h>
#include <string.h>
'''
SPAN_MAIN = r'''int main() {
  using span_math::EndpointPairView;
  EndpointPairView view = {{0,0,0,0}, 20u, 80u};
  assert(sizeof(EndpointPairView) >= 0x18);
  assert(span_math::half_span_plus_addend(&view, 10) == 35);
  view.high_at_14_ = 5u; view.low_at_10_ = 0u;
  assert(span_math::half_span_plus_addend(&view, 0) == 2);
  view.high_at_14_ = 6u;
  assert(span_math::half_span_plus_addend(&view, 0) == 3);
  view.high_at_14_ = 0u;
  assert(span_math::half_span_plus_addend(&view, -1) == 0);
  view.high_at_14_ = 1u;
  assert(span_math::half_span_plus_addend(&view, -2) == 0);
  view.high_at_14_ = 0x80000000u;
  assert(span_math::half_span_plus_addend(&view, 0) == -1073741824);
  view.high_at_14_ = 0x7FFFFFFFu;
  assert(span_math::half_span_plus_addend(&view, 0) == 1073741823);
  view.high_at_14_ = 0xFFFFFFFFu; view.low_at_10_ = 0x7FFFFFFFu;
  assert(span_math::half_span_plus_addend(&view, 0) == -1073741824);
  view.high_at_14_ = 0xFFFFFFFFu; view.low_at_10_ = 0u;
  assert(span_math::half_span_plus_addend(&view, 1) == 0);
  return 0;
}
'''


def span_translation(source, header, body=None):
    actual_header = extract_struct(header, "EndpointPairView")
    actual_body = body or extract(source, "int span_math::half_span_plus_addend(")
    actual_body = actual_body.replace("int span_math::half_span_plus_addend(",
                                      "int half_span_plus_addend(", 1)
    return (SPAN_PREAMBLE + "namespace span_math {\n" + actual_header + "\n" +
            "int half_span_plus_addend(const EndpointPairView *, int);\n" +
            actual_body + "\n}\n" + SPAN_MAIN)


def test_actual_span_owner_preserves_load_order_and_wrap_arithmetic():
    source = SPAN_SOURCE.read_text(encoding="utf-8")
    header = SPAN_HEADER.read_text(encoding="utf-8")
    body = extract(source, "int span_math::half_span_plus_addend(")
    assert body.index("view->high_at_14_") < body.index("view->low_at_10_")
    assert "high_bits - low_bits + static_cast<uint32_t>(addend)" in body
    assert "memcpy(&signed_total, &total_bits, sizeof(signed_total));" in body
    assert "return signed_total / 2;" in body
    compilers = (shutil.which("g++"), shutil.which("clang++"))
    assert all(compilers), "GCC and Clang are required"
    for compiler in compilers:
        compile_run(compiler, span_translation(source, header))


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", ("divide", "shift", "add", "load_order"))
def test_span_mutants_fail_behavioral_or_source_order_controls(compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    source = SPAN_SOURCE.read_text(encoding="utf-8")
    header = SPAN_HEADER.read_text(encoding="utf-8")
    body = extract(source, "int span_math::half_span_plus_addend(")
    if damage == "divide":
        body = body.replace("return signed_total / 2;", "return signed_total / 4;", 1)
    elif damage == "shift":
        body = body.replace("return signed_total / 2;", "return signed_total >> 1;", 1)
    elif damage == "add":
        body = body.replace("high_bits - low_bits + static_cast<uint32_t>(addend)",
                            "high_bits + low_bits + static_cast<uint32_t>(addend)", 1)
    else:
        body = body.replace("const uint32_t high_bits = view->high_at_14_;\n"
                            "    const uint32_t low_bits = view->low_at_10_;",
                            "const uint32_t low_bits = view->low_at_10_;\n"
                            "    const uint32_t high_bits = view->high_at_14_;", 1)
        assert body.index("view->low_at_10_") < body.index("view->high_at_14_")
        return
    compile_run(compiler, span_translation(source, header, body), expected=False)


TABLE_PREAMBLE = r'''#include <assert.h>
#include <stdint.h>
'''
TABLE_FIXTURES = r'''static void *events[16];
static int event_count;
static PointerTripletTable *mutated_receiver;
static PointerTriplet *replacement_table;
static void *mutate_on;
static void observed_free(void *p) {
  events[event_count++] = p;
  if (p == mutate_on) {
    mutated_receiver->table_ = replacement_table;
    mutated_receiver->count_ = 1;
  }
}
static void reset_events() { event_count = 0; mutate_on = 0; }
int main() {
  int values[10];
  PointerTriplet initial[2] = {{&values[0], &values[1], &values[2]},
                               {&values[6], &values[7], &values[8]}};
  PointerTriplet replacement[2] = {{&values[3], &values[4], &values[5]},
                                  {&values[6], &values[7], &values[8]}};
  struct Guarded { PointerTripletTable receiver; uint32_t canary; };
  Guarded g;
  g.receiver.table_ = initial; g.receiver.count_ = 2; g.receiver.field_08_ = 77;
  g.canary = 0xA55A1234u;
  mutated_receiver = &g.receiver; replacement_table = replacement;
  mutate_on = &values[0];
  g.receiver.release_contents();
  assert(event_count == 4);
  assert(events[0] == &values[0] && events[1] == &values[4]);
  assert(events[2] == &values[5] && events[3] == replacement);
  assert(g.receiver.table_ == 0 && g.receiver.count_ == 0 && g.receiver.field_08_ == 0);
  assert(g.canary == 0xA55A1234u);
  assert(initial[0].first_ == &values[0] && replacement[0].second_ == &values[4]);

  reset_events();
  g.receiver.table_ = 0; g.receiver.count_ = 4; g.receiver.field_08_ = 9;
  g.receiver.release_contents();
  assert(event_count == 0 && g.receiver.table_ == 0 &&
         g.receiver.count_ == 0 && g.receiver.field_08_ == 0);

  reset_events();
  g.receiver.table_ = replacement; g.receiver.count_ = 0; g.receiver.field_08_ = 9;
  g.receiver.release_contents();
  assert(event_count == 1 && events[0] == replacement);
  assert(g.receiver.table_ == 0 && g.receiver.count_ == 0 && g.receiver.field_08_ == 0);

  reset_events();
  g.receiver.table_ = replacement; g.receiver.count_ = -1; g.receiver.field_08_ = 9;
  g.receiver.release_contents();
  assert(event_count == 1 && events[0] == replacement);
  assert(g.receiver.table_ == 0 && g.receiver.count_ == 0 && g.receiver.field_08_ == 0);

  reset_events();
  initial[0].first_ = 0; initial[0].third_ = 0;
  g.receiver.table_ = initial; g.receiver.count_ = 1;
  g.receiver.release_contents();
  assert(event_count == 2 && events[0] == &values[1] && events[1] == initial);
  assert(g.canary == 0xA55A1234u);
  return 0;
}
'''


def table_translation(source, header, body=None, damage=None):
    pointer = extract_struct(header, "PointerTriplet")
    table = extract_struct(header, "PointerTripletTable")
    actual = body or extract(source, "void PointerTripletTable::release_contents()")
    if damage == "cached_rows":
        anchor = "                row = receiver->table_ + index;"
        assert actual.count(anchor) == 2
        actual = actual.replace(anchor, "", 2)
    elif damage == "cached_count":
        anchor = "        if (0 < receiver->count_) {"
        assert actual.count(anchor) == 1
        actual = actual.replace(anchor,
            "        const int32_t cached_count = receiver->count_;\n"
            "        if (0 < cached_count) {", 1)
        assert "index < receiver->count_" in actual
        actual = actual.replace("index < receiver->count_", "index < cached_count", 1)
    declarations = pointer + "\n" + table
    harness = TABLE_PREAMBLE + declarations + "\n"
    harness += "static void observed_free(void *);\n#define free observed_free\n"
    harness += actual + "\n#undef free\n" + TABLE_FIXTURES
    return harness, actual


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "cached_rows", "cached_count"))
def test_actual_pointer_triplet_teardown_preserves_live_reload_behavior(compiler_name, damage):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    source = TABLE_SOURCE.read_text(encoding="utf-8")
    header = TABLE_HEADER.read_text(encoding="utf-8")
    body = extract(source, "void PointerTripletTable::release_contents()")
    assert "if (receiver->table_ != 0)" in body
    assert body.index("row = receiver->table_ + index;") < body.index("row->first_")
    assert "while (index < receiver->count_)" in body
    translation, _ = table_translation(source, header, body, damage)
    compile_run(compiler, translation, expected=damage is None)
