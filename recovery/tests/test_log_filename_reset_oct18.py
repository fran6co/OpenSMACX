"""Bounded host controls and pinned instruction witnesses for Log::init.

The harness extracts the real Log class and initializer. Dependency spies keep
freed input storage quarantined, so tests observe call order without claiming
the native result of a dangling-pointer read. They do not establish game ABI,
runtime, generated lifetime, or linked preservation.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest
from test_log_native_formatting_oct19 import (
    extract_log_owner_definition,
    extract_log_storage_declarations,
)


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/log.h").is_file())
HEADER = ROOT / "src/log.h"
SOURCE = ROOT / "src/log.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
TOKEN = re.compile(
    r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.S
)


def braced_definition(text: str, start: int) -> tuple[int, int]:
    opened = False
    depth = 0
    for token in TOKEN.finditer(text, start):
        if token.group() == "{":
            if not opened:
                opened = True
                body_start = token.start()
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if opened and depth == 0:
                return body_start, token.end()
    raise AssertionError("unterminated extracted C++ body")


def class_text() -> str:
    text = HEADER.read_text(encoding="utf-8")
    hits = list(re.finditer(r"\bclass\s+Log\s*\{", text))
    assert len(hits) == 1, len(hits)
    _, end = braced_definition(text, hits[0].start())
    close = text.find(";", end)
    assert close >= end
    return text[hits[0].start():close + 1]


def method_text(text: str, signature: str) -> tuple[str, int, int]:
    hits = list(re.finditer(re.escape(signature), text))
    assert len(hits) == 1, (signature, len(hits))
    brace, end = braced_definition(text, hits[0].end())
    return text[hits[0].start():end], brace, end


def instrumented_class(mutant: str | None = None) -> str:
    source = class_text().replace("MEASURED ", "")
    assert source.count(" private:") == 1
    # Test-only controlled visibility lets dependency spies inspect the live
    # receiver without changing data layout or the production declaration.
    source = source.replace(" private:", " public:", 1)
    destructor, brace, _ = method_text(source, "~Log()")
    injected = "{ ++destructor_entries; if (init_active) std::exit(98);"
    if mutant == "destructor_probe_disabled":
        injected = "{ ++destructor_entries;"
    # Splice at the actual destructor's opening brace, retaining every original
    # statement after the instrumented entry probe.
    absolute = source.index(destructor)
    relative_brace = brace - absolute
    source = source[:absolute + relative_brace] + injected + source[absolute + relative_brace + 1:]

    return dependency_rewritten(source)


def init_text(mutant: str | None = None) -> str:
    text = SOURCE.read_text(encoding="utf-8")
    init, brace, _ = method_text(text, "int Log::init(LPCSTR input)")
    absolute = text.index(init)
    relative_brace = brace - absolute
    # This guard is test-only instrumentation: destructor entry during init
    # exits before the old-ended-receiver function can continue.
    probe = "\n    InitScope init_scope;"
    init = init[:relative_brace + 1] + probe + init[relative_brace + 1:]
    if mutant == "self_destruct":
        insert_at = relative_brace + 1 + len(probe)
        init = init[:insert_at] + "\n    this->~Log();" + init[insert_at:]
    elif mutant == "wrong_alloc_failure":
        assert init.count("return 4;") == 1
        init = init.replace("return 4;", "return 0;", 1)
    elif mutant == "free_on_open_failure":
        needle = "    if (!file) {"
        assert init.count(needle) == 1
        init = init.replace(needle, needle + "\n        spy_free(log_file_);\n        log_file_ = nullptr;", 1)
    elif mutant == "wrong_open_pointer":
        needle = "env_open(input, LogInitializeMode)"
        assert init.count(needle) == 1
        init = init.replace(needle, "env_open(log_file_, LogInitializeMode)", 1)
    elif mutant == "omit_null_store":
        needle = "        log_file_ = nullptr;"
        assert init.count(needle) == 1
        init = init.replace(needle, "        /* damage: omitted post-free null publication */", 1)
    elif mutant not in (None, "early_strlen", "change_flag"):
        raise AssertionError("unknown mutation: " + mutant)
    if mutant in ("early_strlen", "change_flag"):
        guard = "if (!input) {\n        return 16;\n    }"
        assert init.count(guard) == 1
        inserted = "\n    (void)spy_strlen(input);" if mutant == "early_strlen" else "\n    set_state(1);"
        init = init.replace(guard, guard + inserted, 1)
    return dependency_rewritten(init)


PRELUDE = r"""
#include <cassert>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using BOOL = int;
using LPSTR = char *;
using LPCSTR = const char *;
static bool init_active = false;
static int destructor_entries = 0;
static int frees = 0, lengths = 0, allocations = 0, opens = 0, closes = 0;
static bool allocation_fails = false, open_fails = false;
static bool require_free_before_length = false, free_seen = false;
static bool expect_null_at_length = false, expect_null_at_alloc = false;
static size_t expected_alloc_size = 0;
static std::string expected_copy_text, expected_open_path, expected_open_mode;
static const char *expected_input_pointer = nullptr;
static BOOL IsLoggingDisabled = 0;
static std::vector<std::string> events;
static std::vector<char *> allocated;
static char allocation_storage[32][256];
static FILE fake_file{};
class Log;
static Log *observed_log = nullptr;
static char *observed_filename();
static void spy_free(void *pointer);
static size_t spy_strlen(const char *text);
static void *spy_mem_get(size_t size);
static char *spy_strcat(char *destination, const char *source);
static FILE *spy_env_open(const char *path, const char *mode);
static int spy_fclose(FILE *file);
static int spy_fprintf(FILE *, const char *, ...);

struct InitScope {
    InitScope() { assert(!init_active); init_active = true; }
    ~InitScope() { init_active = false; }
};
static void clear_observations() {
    events.clear(); frees = lengths = allocations = opens = closes = 0;
    free_seen = false; require_free_before_length = false;
    expect_null_at_length = expect_null_at_alloc = false;
    expected_alloc_size = 0;
    expected_copy_text.clear(); expected_open_path.clear(); expected_open_mode.clear();
    expected_input_pointer = nullptr;
    allocation_fails = open_fails = false;
}
"""


def dependency_rewritten(source: str) -> str:
    for old, new in (
        (r"(?<![A-Za-z0-9_])free\(", "spy_free("),
        (r"(?<![A-Za-z0-9_])strlen\(", "spy_strlen("),
        (r"(?<![A-Za-z0-9_])mem_get\(", "spy_mem_get("),
        (r"(?<![A-Za-z0-9_])strcat\(", "spy_strcat("),
        (r"(?<![A-Za-z0-9_])env_open\(", "spy_env_open("),
        (r"(?<![A-Za-z0-9_])fclose\(", "spy_fclose("),
        (r"(?<![A-Za-z0-9_])fprintf\(", "spy_fprintf("),
    ):
        source = re.sub(old, new, source)
    return source


def harness(mutant: str | None = None) -> str:
    klass = instrumented_class()
    init = init_text(mutant)
    storage = extract_log_storage_declarations(("LogInitializeMode", "LogAppendMode"))
    say = dependency_rewritten(extract_log_owner_definition(
        "MEASURED void Log::say(LPCSTR str1, LPCSTR str2, int num1, int num2, int num3)"))
    source = PRELUDE + "\n" + storage + "\n" + klass + "\n" + say + r"""

static char *observed_filename() {
    return observed_log ? observed_log->log_file_ : nullptr;
}
static void spy_free(void *pointer) {
    ++frees; free_seen = true; events.push_back("free");
    if (pointer) {
        bool found = false;
        for (char *known : allocated) if (known == pointer) found = true;
        assert(found); // storage is deliberately quarantined, not released
        if (observed_log) assert(observed_filename() == pointer);
    }
}
static size_t spy_strlen(const char *text) {
    ++lengths; events.push_back("strlen");
    if (expected_input_pointer) assert(text == expected_input_pointer);
    if (require_free_before_length) assert(free_seen);
    if (expect_null_at_length && observed_log) assert(observed_filename() == nullptr);
    if (!expected_copy_text.empty()) assert(std::strlen(text) == expected_copy_text.size());
    return std::strlen(text);
}
static void *spy_mem_get(size_t size) {
    ++allocations; events.push_back("alloc");
    if (expect_null_at_alloc && observed_log) assert(observed_filename() == nullptr);
    if (expected_alloc_size) assert(size == expected_alloc_size);
    if (allocation_fails) return nullptr;
    assert(size <= sizeof(allocation_storage[0]));
    char *slot = allocation_storage[(allocated.size()) % 32];
    allocated.push_back(slot);
    std::memset(slot, 0xCC, sizeof(allocation_storage[0]));
    return slot;
}
static char *spy_strcat(char *destination, const char *source) {
    events.push_back("strcat");
    if (expected_input_pointer) assert(source == expected_input_pointer);
    if (!expected_copy_text.empty()) assert(expected_copy_text == source);
    bool found = false;
    for (char *known : allocated) if (known == destination) found = true;
    assert(found);
    char *result = std::strcat(destination, source);
    if (!expected_copy_text.empty()) assert(expected_copy_text == destination);
    return result;
}
static FILE *spy_env_open(const char *path, const char *mode) {
    ++opens; events.push_back(std::string("open:") + mode);
    assert(path != nullptr && mode != nullptr);
    if (expected_input_pointer) assert(path == expected_input_pointer);
    if (!expected_open_path.empty()) assert(expected_open_path == path);
    if (!expected_open_mode.empty()) assert(expected_open_mode == mode);
    if (open_fails) return nullptr;
    return &fake_file;
}
static int spy_fclose(FILE *file) {
    assert(file == &fake_file); ++closes; events.push_back("close"); return 0;
}
static int spy_fprintf(FILE *, const char *, ...) { events.push_back("fprintf"); return 0; }

@@INIT@@

static void expect_init(Log &log, const char *text, const char *input) {
    observed_log = &log;
    expected_input_pointer = input;
    expected_copy_text = text;
    expected_open_path = text;
    expected_open_mode = "wt";
    expected_alloc_size = std::strlen(text) + 1;
    expect_null_at_length = expect_null_at_alloc = true;
}
static void expect_log_write(Log &log, const char *text) {
    observed_log = &log;
    expected_input_pointer = observed_filename();
    expected_copy_text.clear(); expected_alloc_size = 0;
    expected_open_path = text; expected_open_mode = "at";
    expect_null_at_length = expect_null_at_alloc = false;
}

static void expect_events(std::initializer_list<const char *> expected) {
    assert(events.size() == expected.size());
    size_t i = 0;
    for (const char *entry : expected) assert(events[i++] == entry);
}
static void test_null_and_free_order_alias() {
    clear_observations();
    {
        Log log;
        const char *initial_input = "first.log";
        expect_init(log, initial_input, initial_input);
        assert(log.init(initial_input) == 0);
        assert(log.init(nullptr) == 16);
        // Null input returns before any free, length, allocation or open.
        expect_events({"strlen", "alloc", "strcat", "open:wt", "close"});
        char *previous = allocated.back();
        clear_observations();
        require_free_before_length = true;
        expect_init(log, "first.log", previous);
        // The input aliases the old resource. spy_free quarantines its storage
        // so we can observe order without asserting the native dangling-read result.
        assert(log.init(previous) == 0);
        expect_events({"free", "strlen", "alloc", "strcat", "open:wt", "close"});
        assert(frees == 1 && lengths == 1 && allocations == 1 && opens == 1 && closes == 1);
    }
    assert(destructor_entries == 1);
    assert(frees == 2); // init releases old storage; final scope releases new storage
}
static void test_alloc_failure_clears_old_resource_and_preserves_flag() {
    clear_observations(); destructor_entries = 0;
    {
        Log log;
        const char *old_input = "old.log";
        expect_init(log, old_input, old_input);
        assert(log.init(old_input) == 0);
        log.set_state(0); // disabled
        clear_observations(); allocation_fails = true;
        expected_copy_text = "new.log"; expected_alloc_size = std::strlen("new.log") + 1;
        expect_null_at_length = expect_null_at_alloc = true;
        assert(log.init("new.log") == 4);
        expect_events({"free", "strlen", "alloc"});
        assert(frees == 1 && lengths == 1 && allocations == 1 && opens == 0);
        allocation_fails = false;
        const char *after_failure_input = "after-failure.log";
        expect_init(log, after_failure_input, after_failure_input);
        assert(log.init(after_failure_input) == 0);
        clear_observations();
        expect_log_write(log, "after-failure.log");
        log.say("test", nullptr, 1, 2, 3);
        assert(opens == 0); // init did not change the disabled flag
    }
    assert(destructor_entries == 1 && frees == 1);
}
static void test_open_failure_retains_owned_filename_and_enabled_state() {
    clear_observations(); destructor_entries = 0;
    {
        Log log;
        log.set_state(1); // enabled
        open_fails = true;
        const char *retained_input = "retained.log";
        expect_init(log, retained_input, retained_input);
        assert(log.init(retained_input) == 6);
        expect_events({"strlen", "alloc", "strcat", "open:wt"});
        char *owned = allocated.back();
        (void)owned;
        clear_observations();
        expect_log_write(log, "retained.log");
        log.say("test", nullptr, 1, 2, 3);
        assert(opens == 1 && events[0] == "open:at");
        clear_observations();
    }
    assert(destructor_entries == 1 && frees == 1);
    assert(events.size() == 1 && events[0] == "free");
}
static void test_repeated_init_and_single_final_destruction() {
    clear_observations(); destructor_entries = 0;
    {
        Log log;
        const char *one_input = "one.log";
        expect_init(log, one_input, one_input);
        assert(log.init(one_input) == 0);
        clear_observations();
        const char *two_input = "two.log";
        expect_init(log, two_input, two_input);
        assert(log.init(two_input) == 0);
        expect_events({"free", "strlen", "alloc", "strcat", "open:wt", "close"});
        assert(destructor_entries == 0); // no destructor during either init
        clear_observations();
    }
    assert(destructor_entries == 1 && frees == 1);
}
int main() {
    static_assert(sizeof(int) == 4, "the original target uses 32-bit int");
    test_null_and_free_order_alias();
    test_alloc_failure_clears_old_resource_and_preserves_flag();
    test_open_failure_retains_owned_filename_and_enabled_state();
    test_repeated_init_and_single_final_destruction();
}
"""
    assert source.count("@@INIT@@") == 1
    return source.replace("@@INIT@@", init, 1)


def compile_and_run(source: str, compiler: str, optimization: str,
                    expected_exit: int | None = 0) -> subprocess.CompletedProcess[str]:
    with tempfile.TemporaryDirectory(prefix="log-reset-") as directory:
        cpp = Path(directory) / "case.cpp"
        exe = Path(directory) / "case"
        cpp.write_text(source, encoding="utf-8")
        compiled = subprocess.run(
            [compiler, "-std=c++17", optimization, "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True, text=True,
        )
        assert compiled.returncode == 0, compiled.stdout + compiled.stderr
        result = subprocess.run([str(exe)], capture_output=True, text=True)
        if expected_exit is None:
            assert result.returncode != 0, "damage control survived"
        else:
            assert result.returncode == expected_exit, result.stdout + result.stderr
        return result


@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_actual_class_and_init_lifetime_behavior(optimization: str) -> None:
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    if not compilers:
        pytest.skip("no host C++ compiler available")
    for compiler in compilers:
        compile_and_run(harness(), compiler, optimization)


def test_unexpected_destructor_during_init_exits_before_post_lifetime_code() -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler available")
    source = harness("self_destruct")
    # The instrumented destructor exits at entry (98) while init's lifetime
    # guard is active; it never executes the old destructor or continues init.
    compile_and_run(source, compiler, "-O0", expected_exit=98)


@pytest.mark.parametrize("mutant", [
    "early_strlen", "change_flag", "wrong_alloc_failure", "free_on_open_failure",
    "omit_null_store", "wrong_open_pointer",
])
def test_named_compiling_damage_controls_fail(mutant: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no host C++ compiler available")
    compile_and_run(harness(mutant), compiler, "-O0", expected_exit=None)


def test_filename_constructor_still_does_not_initialize_disabled_flag() -> None:
    klass = class_text()
    constructor, _, _ = method_text(klass, "Log(LPCSTR input)")
    assert "is_disabled_" not in constructor


def test_pinned_destructor_and_init_complete_spans_and_critical_order() -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    configured = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(configured) if configured else ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; host tests do not replace image evidence")
    data = image_path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    image = PeImage(data, PIN)
    cs = Cs(CS_ARCH_X86, CS_MODE_32)

    def decode(start: int, end: int, count: int):
        raw = image.read(Span(start, end))
        assert len(raw) == end - start
        instructions = list(cs.disasm(raw, start))
        assert len(instructions) == count
        assert instructions[0].address == start
        assert instructions[-1].address + instructions[-1].size == end
        assert all(a.address + a.size == b.address for a, b in zip(instructions, instructions[1:]))
        return {instruction.address: instruction for instruction in instructions}

    destructor = decode(0x00626020, 0x0062603A, 11)
    init = decode(0x00626040, 0x006260C3, 52)

    def expect(rows, address: int, mnemonic: str, operands: str) -> None:
        instruction = rows[address]
        assert (instruction.mnemonic, instruction.op_str) == (mnemonic, operands)

    expect(destructor, 0x626023, "mov", "eax, dword ptr [esi]")
    expect(destructor, 0x62602A, "call", "0x644ef2")
    expect(destructor, 0x626032, "mov", "dword ptr [esi], 0")
    expect(init, 0x62604A, "jne", "0x626056")
    expect(init, 0x62604D, "mov", "eax, 0x10")
    expect(init, 0x62605D, "call", "0x644ef2")
    expect(init, 0x626065, "mov", "dword ptr [esi], 0")
    expect(init, 0x62606C, "call", "0x6453e0")
    expect(init, 0x626073, "call", "0x5d4510")
    expect(init, 0x62607B, "mov", "dword ptr [esi], eax")
    expect(init, 0x626081, "pop", "edi")
    expect(init, 0x626082, "mov", "eax, 4")
    expect(init, 0x626088, "ret", "4")
    expect(init, 0x626091, "push", "eax")
    expect(init, 0x626090, "push", "edi")
    expect(init, 0x626092, "call", "0x645470")
    expect(init, 0x62609C, "push", "edi")
    expect(init, 0x62609D, "call", "0x634bb0")
    expect(init, 0x6260A7, "jne", "0x6260b3")
    expect(init, 0x6260AA, "mov", "eax, 6")
    expect(init, 0x6260B4, "call", "0x645598")
    expect(init, 0x6260BC, "xor", "eax, eax")
    expect(init, 0x6260C0, "ret", "4")
