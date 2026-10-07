"""Host controls over the installed source bodies of three address-only leaves.

The small offset views expose only the fields each body touches. They do not
assert a shared or original C++ class, complete object extent, or domain role.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS

ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/IMAGE_PIN").is_file())
SOURCE = ROOT / "src/leaf_recoveries.cpp"


def _mask(source):
    return COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " "
                              for char in match[0]), source)


def extract(source, signature):
    masked = _mask(source)
    masked_signature = _mask(signature)
    assert masked.count(masked_signature) == 1, "ambiguous definition: " + signature
    start = masked.find(masked_signature)
    assert start >= 0, "missing actual source definition: " + signature
    opening = masked.find("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{": depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unbalanced actual source definition: " + signature)


def extract_struct(source, name):
    signature = "struct " + name + " {"
    masked = _mask(source)
    start = masked.find(signature)
    assert start >= 0, "missing actual source view: " + name
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
    raise AssertionError("unbalanced actual source view: " + name)


PREAMBLE = r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <cstring>
#define __fastcall
#define __cdecl
'''

MAIN = r'''int main() {
  // These sizeof/offsetof checks describe only the two test views, not the
  // complete game object layout or an original C++ class.
  assert(sizeof(Leaf4C8070OffsetView) == 12);
  assert(offsetof(Leaf4C8070OffsetView, dword_4_) == 4);
  assert(offsetof(Leaf4C8070OffsetView, dword_8_) == 8);
  assert(sizeof(Leaf408470OffsetView) == 24);
  assert(offsetof(Leaf408470OffsetView, dword_10_) == 0x10);
  assert(offsetof(Leaf408470OffsetView, dword_14_) == 0x14);

  struct Guarded4C { uint32_t before; Leaf4C8070OffsetView view; uint32_t after; };
  Guarded4C receiver = {0x11223344u, {0, 0x12345678u, 0xA5A50000u}, 0x55667788u};
  void *returned = leaf_004c8070_redirect(&receiver.view, 0);
  assert(returned == &receiver.view);
  assert(receiver.view.dword_0_ == 0x24u);
  assert(receiver.view.dword_4_ == 0u);
  assert(receiver.view.dword_8_ == 0xA5A50001u);
  assert(receiver.before == 0x11223344u && receiver.after == 0x55667788u);
  receiver.view.dword_8_ = 0xFFFFFFFFu;
  leaf_004c8070_redirect(&receiver.view, 0);
  assert(receiver.view.dword_8_ == 0xFFFFFFFFu);
  assert(receiver.before == 0x11223344u && receiver.after == 0x55667788u);

  assert(range(80u, 20u, 10) == 35);
  assert(range(0u, 5u, 0) == -2);
  assert(range(0u, 6u, 0) == -3);
  assert(range(0u, 0u, -1) == 0);
  assert(range(1u, 0u, -2) == 0);
  assert(range(0x80000000u, 0u, 0) == -1073741824);
  assert(range(0x7FFFFFFFu, 0u, 0) == 1073741823);
  assert(range(0xFFFFFFFFu, 0x7FFFFFFFu, 0) == -1073741824);
  assert(range(0xFFFFFFFFu, 0u, 1) == 0);
  return 0;
}
'''


def _host_source(source, helper_4c8070=None, helper_408470=None):
    type_4c = extract_struct(source, "Leaf4C8070OffsetView")
    type_408 = extract_struct(source, "Leaf408470OffsetView")
    body_4c = helper_4c8070 or extract(source, "void *__fastcall leaf_004c8070_redirect(")
    body_408 = helper_408470 or extract(source, "int __cdecl leaf_00408470_redirect(")
    input_helper = r'''static Leaf408470OffsetView input_view(uint32_t high, uint32_t low) {
  Leaf408470OffsetView value = {{0,0,0,0}, low, high};
  return value;
}
static int range(uint32_t high, uint32_t low, int addend) {
  struct Guarded408 { uint32_t before; Leaf408470OffsetView view; uint32_t after; };
  Guarded408 value = {0xA1B2C3D4u, input_view(high, low), 0xD4C3B2A1u};
  const int result = leaf_00408470_redirect(&value.view, addend);
  assert(value.before == 0xA1B2C3D4u && value.after == 0xD4C3B2A1u);
  return result;
}
'''
    return PREAMBLE + type_4c + "\n" + type_408 + "\n" + body_4c + \
        "\n" + body_408 + "\n" + input_helper + MAIN


def compile_run(compiler, source, expected=True):
    with tempfile.TemporaryDirectory(prefix="leaf-neutral-view-") as temp:
        cpp = Path(temp) / "controls.cpp"
        exe = Path(temp) / "controls"
        cpp.write_text(source)
        built = subprocess.run(
            [compiler, "-std=c++98", "-O0", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
            capture_output=True, text=True)
        assert built.returncode == 0, built.stdout + built.stderr
        ran = subprocess.run([str(exe)], capture_output=True, text=True)
        assert (ran.returncode == 0) == expected, ran.stdout + ran.stderr


def assert_pinned_4c_order(helper):
    load = helper.index("uint32_t old_dword_8 = receiver->dword_8_;")
    clear = helper.index("receiver->dword_4_ = 0;")
    set_bit = helper.index("old_dword_8 |= 1U;")
    set_prefix = helper.index("receiver->dword_0_ = 0x24;")
    publish = helper.index("receiver->dword_8_ = old_dword_8;")
    assert load < clear < set_bit < set_prefix < publish


def test_actual_source_bodies_preserve_pinned_order_and_arithmetic():
    source = SOURCE.read_text()
    assert "// ORIGINAL: 0x004C8070 sub_4c8070 0x004C8070-0x004C8089 BYTE_EXACT" in source
    assert "// ORIGINAL: 0x00408470 sub_408470 0x00408470-0x0040848A BYTE_EXACT" in source
    helper_4c = extract(source, "void *__fastcall leaf_004c8070_redirect(")
    assert_pinned_4c_order(helper_4c)

    helper_408 = extract(source, "int __cdecl leaf_00408470_redirect(")
    assert helper_408.index("view->dword_14_") < helper_408.index("view->dword_10_")
    assert "std::memcpy(&signed_total, &total_bits, sizeof(signed_total));" in helper_408
    assert "return signed_total / 2;" in helper_408
    assert "high_bits - low_bits + static_cast<uint32_t>(addend)" in helper_408

    compilers = (shutil.which("g++"), shutil.which("clang++"))
    assert all(compilers), "GCC and Clang are required"
    translation = _host_source(source)
    for compiler in compilers:
        compile_run(compiler, translation)


def test_order_mutant_is_rejected_by_the_source_order_control():
    source = SOURCE.read_text()
    helper = extract(source, "void *__fastcall leaf_004c8070_redirect(")
    mutant = helper.replace(
        "uint32_t old_dword_8 = receiver->dword_8_;\n    receiver->dword_4_ = 0;",
        "receiver->dword_4_ = 0;\n    uint32_t old_dword_8 = receiver->dword_8_;", 1)
    assert mutant != helper
    with pytest.raises(AssertionError):
        assert_pinned_4c_order(mutant)


def test_arithmetic_mutants_fail_behavioral_controls_with_gcc_and_clang():
    source = SOURCE.read_text()
    helper = extract(source, "int __cdecl leaf_00408470_redirect(")
    mutations = (
        ("return signed_total / 2;", "return signed_total / 4;"),
        ("return signed_total / 2;", "return signed_total >> 1;"),
        ("high_bits - low_bits + static_cast<uint32_t>(addend)",
         "high_bits + low_bits + static_cast<uint32_t>(addend)"),
    )
    compilers = (shutil.which("g++"), shutil.which("clang++"))
    assert all(compilers), "GCC and Clang are required"
    for before, after in mutations:
        assert before in helper
        mutant = helper.replace(before, after, 1)
        translation = _host_source(source, helper_408470=mutant)
        for compiler in compilers:
            compile_run(compiler, translation, expected=False)

# Host pointer width may differ; this tests behavior, not original x86 layout.
TABLE_MAIN = r'''static void *events[8];
static int event_count;
static ThreePointerTableReceiverView *mutated_receiver;
static ThreePointerRowView *replacement_table;
static void *mutate_on;

static void observed_free(void *p) {
    events[event_count++] = p;
    if (p == mutate_on) {
        mutated_receiver->entries_ = replacement_table;
        mutated_receiver->count_ = 1;
    }
}

static void reset() { event_count = 0; mutate_on = 0; }

int main() {
    int values[10];
    ThreePointerRowView initial[2] = {{&values[0], &values[1], &values[2]}, {&values[6], &values[7], &values[8]}};
    ThreePointerRowView replacement[2] = {{&values[3], &values[4], &values[5]}, {&values[6], &values[7], &values[8]}};
    struct Guarded { ThreePointerTableReceiverView receiver; uint32_t canary; };
    Guarded g;
    g.receiver.entries_ = initial; g.receiver.count_ = 2; g.receiver.field_08_ = 77;
    g.canary = 0xA55A1234u;
    mutated_receiver = &g.receiver; replacement_table = replacement;
    mutate_on = &values[0];
    sub_616b40(&g.receiver, 0);
    assert(event_count == 4);
    assert(events[0] == &values[0] && events[1] == &values[4]);
    assert(events[2] == &values[5] && events[3] == replacement);
    assert(g.receiver.entries_ == 0 && g.receiver.count_ == 0 && g.receiver.field_08_ == 0);
    assert(g.canary == 0xA55A1234u);
    assert(initial[0].field_00_ == &values[0] && replacement[0].field_04_ == &values[4]);

    reset();
    g.receiver.entries_ = 0; g.receiver.count_ = 4; g.receiver.field_08_ = 9;
    sub_616b40(&g.receiver, 0);
    assert(event_count == 0 && g.receiver.entries_ == 0 && g.receiver.count_ == 0 && g.receiver.field_08_ == 0);

    reset();
    g.receiver.entries_ = replacement; g.receiver.count_ = 0; g.receiver.field_08_ = 9;
    sub_616b40(&g.receiver, 0);
    assert(event_count == 1 && events[0] == replacement);
    assert(g.receiver.entries_ == 0 && g.receiver.count_ == 0 && g.receiver.field_08_ == 0);

    reset();
    g.receiver.entries_ = replacement; g.receiver.count_ = -1; g.receiver.field_08_ = 9;
    sub_616b40(&g.receiver, 0);
    assert(event_count == 1 && events[0] == replacement);
    assert(g.receiver.entries_ == 0 && g.receiver.count_ == 0 && g.receiver.field_08_ == 0);
    reset();
    initial[0].field_00_ = 0; initial[0].field_08_ = 0;
    g.receiver.entries_ = initial; g.receiver.count_ = 1;
    sub_616b40(&g.receiver, 0);
    assert(event_count == 2 && events[0] == &values[1] && events[1] == initial);
    assert(g.canary == 0xA55A1234u);
    return 0;
}
'''


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
@pytest.mark.parametrize("damage", (None, "cached_rows", "cached_count"))
def test_actual_three_pointer_table_teardown(compiler_name, damage):
    source = SOURCE.read_text()
    declarations = extract_struct(source, "ThreePointerRowView") + "\n"
    declarations += extract_struct(source, "ThreePointerTableReceiverView")
    body = extract(source, 'extern "C" void __fastcall sub_616b40(')
    if damage == "cached_rows":
        anchor = "                row = receiver->entries_ + index;"
        assert body.count(anchor) == 2
        body = body.replace(anchor, "")
    elif damage == "cached_count":
        anchor = "        if (0 < receiver->count_) {"
        assert body.count(anchor) == 1
        body = body.replace(anchor,
            "        const int32_t cached_count = receiver->count_;\n"
            "        if (0 < cached_count) {", 1)
        assert body.count("index < receiver->count_") == 1
        body = body.replace("index < receiver->count_", "index < cached_count", 1)
    assert "ThreePointerRowView *const table = receiver->entries_;" in body
    assert "free(table);" in body
    translation = (PREAMBLE + declarations +
        "\nstatic void observed_free(void *);\n#define free observed_free\n" +
        body + "\n#undef free\n" + TABLE_MAIN)
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " is unavailable")
    compile_run(compiler, translation, expected=damage is None)
