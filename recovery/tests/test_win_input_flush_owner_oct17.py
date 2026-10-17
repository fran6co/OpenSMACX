"""Host controls for the source-owned Win input flush pump."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

from match_recovery.import_source import parse_imports
from match_recovery.object_source import parse_objects


ROOT = Path(__file__).resolve().parents[2]


def test_input_pump_dependency_identities_are_source_owned() -> None:
    source = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    # The parser needs a valid pin; this test verifies syntax and source facts,
    # not the binary identity, original layout, or runtime behavior.
    bindings = parse_objects(source, "src/win.cpp", "0" * 64, address_space="pe:va")
    for symbol, address in (
        ("?WinPointerOwner1@@3PAVWin@@A", 0x009B7ACC),
        ("?WinPointerOwner2@@3PAVWin@@A", 0x009B7AD0),
    ):
        binding, = [item for item in bindings if item.symbol == symbol]
        assert binding.origin.address == address
        assert binding.span is None
        assert binding.parent_symbol is None and binding.target_symbol is None
    binding, = [item for item in parse_imports(source, "src/win.cpp")
                if item.symbol == "__imp__PeekMessageA@20"]
    assert (binding.module, binding.name, binding.ordinal) == ("USER32.dll", "PeekMessageA", None)


def flush_body() -> str:
    source = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    signature = "void __cdecl flush_input()"
    assert source.count(signature) == 1
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        depth += (source[end] == "{") - (source[end] == "}")
        if depth == 0:
            return source[opening + 1 : end]
    raise AssertionError("unterminated flush_input")


def harness(body: str) -> str:
    return r"""
#include <cassert>
#include <cstddef>

#define __cdecl
#define WM_KEYFIRST 0x100
// Pinned pushes at5FD131/5FD147 use0x108, not a modern SDK's0x109.
#define WM_KEYLAST 0x108
#define WM_MOUSEFIRST 0x200
#define WM_MOUSELAST 0x209
#define PM_REMOVE 1

struct MSG { unsigned int message; };
enum { PEEK = 1, CLEAR_OWNER1, CLEAR_OWNER2, NETWORK_POLL };
struct Event {
    int kind;
    unsigned int first, last, remove;
    int result, null1, null2, null_handle;
};
static Event events[32];
static int event_count;
static void record(Event event) { events[event_count++] = event; }
static Event event(int kind, unsigned int first = 0, unsigned int last = 0,
                   unsigned int remove = 0, int result = 0,
                   int null1 = 0, int null2 = 0, int null_handle = 0) {
    Event value = {kind, first, last, remove, result, null1, null2, null_handle};
    return value;
}

struct PointerSlot {
    bool is_null;
    int clear_kind;
    PointerSlot &operator=(std::nullptr_t);
};
static PointerSlot WinPointerOwner1;
static PointerSlot WinPointerOwner2;
PointerSlot &PointerSlot::operator=(std::nullptr_t) {
    is_null = true;
    record(event(clear_kind, 0, 0, 0, 0,
                 WinPointerOwner1.is_null, WinPointerOwner2.is_null));
    return *this;
}

static int queue_messages[16];
static int queue_count;
static int network_poll_count;
static void add_message(int message) { queue_messages[queue_count++] = message; }

int PeekMessage(MSG *message, void *window, unsigned int first,
                unsigned int last, unsigned int remove) {
    int found = -1;
    for (int index = 0; index < queue_count; ++index) {
        if (queue_messages[index] >= static_cast<int>(first) &&
            queue_messages[index] <= static_cast<int>(last)) {
            found = index;
            break;
        }
    }
    int result = 0;
    if (found >= 0) {
        message->message = queue_messages[found];
        for (int index = found + 1; index < queue_count; ++index)
            queue_messages[index - 1] = queue_messages[index];
        --queue_count;
        result = 1;
    }
    record(event(PEEK, first, last, remove, result,
                 WinPointerOwner1.is_null, WinPointerOwner2.is_null,
                 window == NULL));
    return result;
}

void check_net() {
    record(event(NETWORK_POLL, 0, 0, 0, network_poll_count + 1,
                 WinPointerOwner1.is_null, WinPointerOwner2.is_null));
    ++network_poll_count;
    if (network_poll_count < 3) {
        WinPointerOwner1.is_null = false;
        WinPointerOwner2.is_null = false;
    }
}

void __cdecl flush_input() {
""" + body + r"""
}

static void reset() {
    event_count = 0;
    queue_count = 0;
    network_poll_count = 0;
    WinPointerOwner1.is_null = false;
    WinPointerOwner1.clear_kind = CLEAR_OWNER1;
    WinPointerOwner2.is_null = false;
    WinPointerOwner2.clear_kind = CLEAR_OWNER2;
}
static bool same(const Event &actual, const Event &expected) {
    return actual.kind == expected.kind && actual.first == expected.first &&
           actual.last == expected.last && actual.remove == expected.remove &&
           actual.result == expected.result && actual.null1 == expected.null1 &&
           actual.null2 == expected.null2 && actual.null_handle == expected.null_handle;
}
static Event expected_peek(unsigned int first, unsigned int last, int result) {
    return event(PEEK, first, last, PM_REMOVE, result, 0, 0, 1);
}
static Event expected_mark(int kind, int first_null, int second_null, int result = 0) {
    return event(kind, 0, 0, 0, result, first_null, second_null);
}
static bool expect_events(const Event *expected, int count) {
    if (event_count != count) return false;
    for (int index = 0; index < count; ++index)
        if (!same(events[index], expected[index])) return false;
    return true;
}

static int populated_queues() {
    reset();
    add_message(0x201); add_message(0x100); add_message(0x202);
    add_message(0x108); add_message(0x109); add_message(0x20A); add_message(0x777);
    flush_input();
    const Event expected[] = {
        expected_peek(WM_KEYFIRST, WM_KEYLAST, 1),
        expected_peek(WM_KEYFIRST, WM_KEYLAST, 1),
        expected_peek(WM_KEYFIRST, WM_KEYLAST, 0),
        expected_mark(CLEAR_OWNER1, 1, 0),
        expected_mark(CLEAR_OWNER2, 1, 1),
        expected_mark(NETWORK_POLL, 1, 1, 1),
        expected_peek(WM_MOUSEFIRST, WM_MOUSELAST, 1),
        expected_peek(WM_MOUSEFIRST, WM_MOUSELAST, 1),
        expected_peek(WM_MOUSEFIRST, WM_MOUSELAST, 0),
        expected_mark(CLEAR_OWNER1, 1, 0),
        expected_mark(CLEAR_OWNER2, 1, 1),
        expected_mark(NETWORK_POLL, 1, 1, 2),
        expected_mark(NETWORK_POLL, 0, 0, 3),
    };
    if (!expect_events(expected, sizeof(expected) / sizeof(expected[0]))) return 1;
    if (queue_count != 3 || queue_messages[0] != 0x109 ||
        queue_messages[1] != 0x20A || queue_messages[2] != 0x777) return 2;
    if (network_poll_count != 3 || WinPointerOwner1.is_null || WinPointerOwner2.is_null) return 3;
    return 0;
}

static int empty_queues() {
    reset();
    flush_input();
    const Event expected[] = {
        expected_peek(WM_KEYFIRST, WM_KEYLAST, 0),
        expected_mark(CLEAR_OWNER1, 1, 0),
        expected_mark(CLEAR_OWNER2, 1, 1),
        expected_mark(NETWORK_POLL, 1, 1, 1),
        expected_peek(WM_MOUSEFIRST, WM_MOUSELAST, 0),
        expected_mark(CLEAR_OWNER1, 1, 0),
        expected_mark(CLEAR_OWNER2, 1, 1),
        expected_mark(NETWORK_POLL, 1, 1, 2),
        expected_mark(NETWORK_POLL, 0, 0, 3),
    };
    return expect_events(expected, sizeof(expected) / sizeof(expected[0])) ? 0 : 1;
}

int main() {
    int result = populated_queues(); if (result) return 10 + result;
    result = empty_queues(); if (result) return 20 + result;
    return 0;
}
"""


def compile_and_run(compiler: str, source: str) -> int:
    with tempfile.TemporaryDirectory(prefix="osmx-win-input-flush-") as tmp:
        binary = Path(tmp) / "flush-controls"
        built = subprocess.run(
            [compiler, "-std=c++11", "-O0", "-x", "c++", "-", "-o", str(binary)],
            input=source, text=True, capture_output=True, timeout=45,
        )
        assert built.returncode == 0, built.stderr
        return subprocess.run([str(binary)], check=False, timeout=10).returncode


def compilers() -> list[str]:
    found = [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]
    assert found, "a host C++ compiler is required for the behavioral controls"
    return found


def test_actual_input_flush_drains_and_polls_in_order() -> None:
    body = flush_body()
    source = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    win_header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    console_header = (ROOT / "src/console.h").read_text(encoding="utf-8")
    assert source.count("// ORIGINAL: 0x005FD120 ?flush_input@@YAXXZ") == 1
    assert "void __cdecl flush_input();" in win_header
    assert "flush_input();" not in console_header
    assert "func_console_flush_input" not in console_header
    assert body.count("check_net();") == 3
    assert "PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE)" in body
    assert "PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE)" in body
    for compiler in compilers():
        assert compile_and_run(compiler, harness(body)) == 0


@pytest.mark.parametrize("mutation", ["poll_before_clear", "drop_final_poll", "swap_drains"])
def test_input_flush_order_and_poll_count_mutants_fail(mutation: str) -> None:
    body = flush_body()
    if mutation == "poll_before_clear":
        before = (
            "    WinPointerOwner1 = nullptr;\n"
            "    WinPointerOwner2 = nullptr;\n"
            "    check_net();"
        )
        after = (
            "    check_net();\n"
            "    WinPointerOwner1 = nullptr;\n"
            "    WinPointerOwner2 = nullptr;"
        )
        assert body.count(before) == 2
        body = body.replace(before, after, 1)
    elif mutation == "drop_final_poll":
        assert body.count("check_net();") == 3
        start, end = body.rsplit("    check_net();", 1)
        body = start + end
    else:
        assert body.count("WM_KEYFIRST, WM_KEYLAST") == 2
        assert body.count("WM_MOUSEFIRST, WM_MOUSELAST") == 2
        body = body.replace("WM_KEYFIRST, WM_KEYLAST", "TEMP_KEY_RANGE").replace(
            "WM_MOUSEFIRST, WM_MOUSELAST", "WM_KEYFIRST, WM_KEYLAST"
        ).replace("TEMP_KEY_RANGE", "WM_MOUSEFIRST, WM_MOUSELAST")
    for compiler in compilers():
        assert compile_and_run(compiler, harness(body)) != 0
