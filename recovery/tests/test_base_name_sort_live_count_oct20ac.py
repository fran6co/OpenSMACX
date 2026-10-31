"""Actual-source controls for the native live-bound base-name sort.

The pinned body loads the count for its entry guard at 0x4D5FF4, then
reloads it in the inner loop at 0x4D6050 before the 0x4D6059..0x4D605E
continuation test. The controlled comparator mutation below witnesses that
source read timing; it does not claim the production CRT comparator changes
BaseCurrentCount or establish asynchronous runtime behavior.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile
import os
from test_mapwin_cache import braced

from match_recovery.source import Mode, read_file


ROOT = Path(__file__).resolve().parents[2]
ADDRESS = 0x004D5FF0
SIGNATURE = "void __cdecl sub_4d5ff0(int *id_list)"


def _masked_code(source):
    """Mask comments and literals while preserving offsets and braces."""
    out = list(source)
    index = 0
    state = "code"
    quote = ""
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 2
                state = "line"
                continue
            if char == "/" and nxt == "*":
                out[index] = out[index + 1] = " "
                index += 2
                state = "block"
                continue
            if char in ('"', "'"):
                quote = char
                out[index] = " "
                state = "quoted"
        elif state == "line":
            if char == "\n":
                state = "code"
            else:
                out[index] = " "
        elif state == "block":
            if char == "*" and nxt == "/":
                out[index] = out[index + 1] = " "
                index += 2
                state = "code"
                continue
            if char != "\n":
                out[index] = " "
        else:
            if char == "\\":
                out[index] = " "
                if index + 1 < len(source) and source[index + 1] != "\n":
                    out[index + 1] = " "
                    index += 2
                    continue
            elif char == quote:
                out[index] = " "
                state = "code"
            elif char != "\n":
                out[index] = " "
        index += 1
    return "".join(out)


def _actual_owner():
    records = [row for row in read_file(ROOT / "src" / "base.cpp")
               if row.address == ADDRESS]
    assert len(records) == 1, "missing or duplicate original owner mapping"
    row = records[0]
    assert row.mode == Mode.FILE
    assert row.name == "sub_4d5ff0"
    assert row.image_spans == ((0x4D5FF0, 0x4D606E),)
    assert row.region and SIGNATURE in row.region
    masked = _masked_code(row.region)
    start = masked.index(SIGNATURE)
    opening = masked.index("{", start)
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                body = row.region[start:index + 1]
                break
    else:
        raise AssertionError("source-reader owner has no balanced function body")
    assert "_stricmp" in body and "swap(cursor, cursor + 1)" in body
    source = (ROOT / "src/base.cpp").read_text()
    helpers = "\n".join(braced(source, head) for head in (
        "int base_yield_signed_bits(uint32_t value)", "int base_yield_sub(int left, int right)"))
    return helpers + "\n" + body


_FIXTURE = r'''#include <cassert>
#include <stdint.h>
#include <limits.h>
#include <cctype>
#include <cstring>
#define __cdecl

struct Base { const char *name_string; };
static Base Bases[8];
struct Fields { int base_current_count_; };
struct State { Fields fields_; };
static State GameDataStateStorage;

static int comparator_calls;
static int mutate_on_call;
static int count_after_mutation;

static int _stricmp(const char *left, const char *right) {
    ++comparator_calls;
    if (mutate_on_call != 0 && comparator_calls == mutate_on_call) {
        GameDataStateStorage.fields_.base_current_count_ = count_after_mutation;
    }
    for (;;) {
        const unsigned char a = static_cast<unsigned char>(*left++);
        const unsigned char b = static_cast<unsigned char>(*right++);
        const int lower_a = std::tolower(a);
        const int lower_b = std::tolower(b);
        if (lower_a != lower_b) return lower_a - lower_b;
        if (a == 0) return 0;
    }
}

static void swap(int *left, int *right) {
    const int temporary = *left;
    *left = *right;
    *right = temporary;
}

// ACTUAL_FUNCTION_INSERTION_POINT

static void reset_controls(int count) {
    GameDataStateStorage.fields_.base_current_count_ = count;
    comparator_calls = 0;
    mutate_on_call = 0;
    count_after_mutation = count;
}

static void test_stable_caseblind_order_and_multiple_passes() {
    Bases[0].name_string = "beta";
    Bases[1].name_string = "ALPHA";
    Bases[2].name_string = "alpha";
    Bases[3].name_string = "delta";
    Bases[4].name_string = "charlie";
    reset_controls(5);
    int ids[] = {0, 1, 2, 3, 4};
    sub_4d5ff0(ids);
    const int expected[] = {1, 2, 0, 4, 3};
    for (int i = 0; i != 5; ++i) assert(ids[i] == expected[i]);
    assert(comparator_calls > 4); // more than one complete pass
}

static void test_nonpositive_and_singleton_counts_do_not_read_nodes() {
    const int counts[] = {0, 1, -1, -7};
    for (unsigned int i = 0; i != sizeof(counts) / sizeof(counts[0]); ++i) {
        reset_controls(counts[i]);
        int ids[] = {7, 6, 5};
        sub_4d5ff0(ids);
        assert(ids[0] == 7 && ids[1] == 6 && ids[2] == 5);
        assert(comparator_calls == 0);
    }
}

static void test_count_reload_after_comparison() {
    Bases[0].name_string = "a";
    Bases[1].name_string = "b";
    Bases[2].name_string = "c";
    Bases[3].name_string = "d";
    reset_controls(4);
    mutate_on_call = 1;
    count_after_mutation = 2;
    int ids[] = {0, 1, 2, 3};
    sub_4d5ff0(ids);
    // The live inner bound is rechecked after the first comparison. This is
    // only a controlled source-read witness, not a claim about CRT behavior.
    assert(comparator_calls == 1);
    assert(ids[0] == 0 && ids[1] == 1 && ids[2] == 2 && ids[3] == 3);
}

void test_wrapped_decrement_keeps_native_unsafe_entry() {
    Bases[0].name_string = "a"; Bases[1].name_string = "b";
    reset_controls(INT_MIN);
    mutate_on_call = 1; count_after_mutation = 0;
    int ids[] = {0, 1};
    sub_4d5ff0(ids);
    // Native DEC wraps INT_MIN to INT_MAX before its signed JLE. The
    // controlled comparator shrinks the latch count before an unsafe walk.
    assert(comparator_calls == 1);
}

int main() {
    test_stable_caseblind_order_and_multiple_passes();
    test_nonpositive_and_singleton_counts_do_not_read_nodes();
    test_count_reload_after_comparison();
    test_wrapped_decrement_keeps_native_unsafe_entry();
    return 0;
}
'''


def _compile_run(directory, body, compiler):
    marker = "// ACTUAL_FUNCTION_INSERTION_POINT"
    assert _FIXTURE.count(marker) == 1
    source = _FIXTURE.replace(marker, body)
    cpp = directory / "base_name_sort.cpp"
    exe = directory / "base_name_sort"
    cpp.write_text(source)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        check=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
    )
    return subprocess.run([str(exe)], check=False, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT, text=True)


def test_base_name_sort_actual_source_controls():
    compilers = [compiler for compiler in ("g++", "clang++")
                 if shutil.which(compiler)]
    assert compilers, "neither GCC nor Clang is available"
    body = _actual_owner()
    with tempfile.TemporaryDirectory(prefix="osmx-base-sort-host-") as temp:
        for compiler in compilers:
            result = _compile_run(Path(temp), body, compiler)
            assert result.returncode == 0, f"{compiler}: {result.stdout}"


def test_base_name_sort_stale_bound_mutant_is_detected():
    compilers = [compiler for compiler in ("g++", "clang++")
                 if shutil.which(compiler)]
    assert compilers, "neither GCC nor Clang is available"
    body = _actual_owner()
    # Freeze only the final live-count read, independent of loop spelling.
    field = "GameDataStateStorage.fields_.base_current_count_"
    latch = body.rfind(field)
    assert latch > body.index(SIGNATURE)
    stale_body = body[:latch] + "cached_count" + body[latch + len(field):]
    opening = stale_body.index("{", stale_body.index(SIGNATURE)) + 1
    stale_body = stale_body[:opening] + "\n    const int cached_count = " + field + ";\n" + stale_body[opening:]
    with tempfile.TemporaryDirectory(prefix="osmx-base-sort-mutants-") as temp:
        for compiler in compilers:
            result = _compile_run(Path(temp), stale_body, compiler)
            assert result.returncode != 0, (
                f"{compiler} did not detect the stale-bound mutant"
            )


def test_pinned_wrapping_entry_and_live_sort_latch():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    image=PeImage(Path(os.environ['OPENSMACX_IMAGE']).read_bytes(),'01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    rows={i.address:(i.mnemonic,i.op_str)for i in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(Span(0x4D5FF0,0x4D606E)),0x4D5FF0)}
    assert rows[0x4D5FFE]==('dec','eax')
    assert rows[0x4D6004]==('jle','0x4d6067')
    assert rows[0x4D6050]==('mov','eax, dword ptr [0x9a64cc]')
    assert rows[0x4D6059]==('lea','ecx, [eax - 1]')
    assert rows[0x4D605E]==('jl','0x4d6009')
