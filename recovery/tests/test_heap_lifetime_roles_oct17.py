"""Actual-source host controls for live heap reset versus C++ teardown."""
import os
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest
from capstone import CS_ARCH_X86, CS_MODE_32, CS_OP_IMM, CS_OP_REG, Cs
from capstone.x86_const import X86_REG_ECX

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin
from recovery.tests.test_pact_ascent import _closing_brace, definition


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/heap.cpp").is_file())

FIXTURE = r'''#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef void *LPVOID;
typedef char *LPSTR;
typedef const char *LPCSTR;
typedef int BOOL;
#define __cdecl
#define UNUSED(x)
#define MEASURED
#include "@@HEAP_HEADER@@"

struct HeapLifetimeFixtureAccess;
// @@STRINGS_CLASS@@
// @@TEXTINDEX_CLASS@@
static const int MaxTextIndexNum = 4;

enum { EVENT_ALLOC = 1, EVENT_FREE = 2, EVENT_DOUBLE_FREE = 3,
       EVENT_NULL_FREE = 4, EVENT_SENTINEL = 5 };
struct Event { int kind; void *pointer; int label; int count; int name_byte; };
static Event events[128];
static int event_count;
static void record_event(int kind, void *pointer, int label, int count, int name_byte) {
    assert(event_count < 128);
    events[event_count].kind = kind;
    events[event_count].pointer = pointer;
    events[event_count].label = label;
    events[event_count].count = count;
    events[event_count].name_byte = name_byte;
    ++event_count;
}
static void reset_events() { event_count = 0; }

struct Allocation { void *pointer; int active; uint32_t *count; char *name; };
static Allocation allocations[64];
static int allocation_count;

static void *mem_get(int size) {
    void *pointer = malloc((size_t)size);
    if (pointer) {
        assert(allocation_count < 64);
        allocations[allocation_count].pointer = pointer;
        allocations[allocation_count].active = 1;
        allocations[allocation_count].count = 0;
        allocations[allocation_count].name = 0;
        ++allocation_count;
        record_event(EVENT_ALLOC, pointer, -1, -1, -1);
    }
    return pointer;
}

static void host_heap_free(void *pointer) {
    if (!pointer) {
        record_event(EVENT_NULL_FREE, 0, -1, -1, -1);
        return;
    }
    for (int i = 0; i < allocation_count; ++i) {
        if (allocations[i].pointer == pointer && allocations[i].active) {
            allocations[i].active = 0;
            int count = allocations[i].count ? (int)*allocations[i].count : -1;
            int name_byte = allocations[i].name ? (unsigned char)allocations[i].name[0] : -1;
            record_event(EVENT_FREE, pointer, -1, count, name_byte);
            free(pointer);
            return;
        }
    }
    record_event(EVENT_DOUBLE_FREE, pointer, -1, -1, -1);
}

// @@HEAP_CONSTRUCTOR@@
// @@HEAP_RESET@@
// @@HEAP_DESTRUCTOR@@
// @@HEAP_INIT@@
// @@STRINGS_SHUTDOWN@@
// @@TEXTINDEX_CONSTRUCTOR@@
// @@TEXTINDEX_DESTRUCTOR@@
// @@CLEAR_INDEX_HELPER@@

struct HeapLifetimeFixtureAccess {
    static void set_populated(Strings &value, bool populated) {
        value.is_populated_ = populated;
    }
    static bool is_populated(const Strings &value) { return value.is_populated_; }
    static Heap &heap(TextIndex &value) { return value.heap_; }
    static uint32_t count(TextIndex &value) { return value.section_count_; }
    static char name_first(TextIndex &value) { return value.file_name_[0]; }
    static void prepare_text_index(TextIndex &value, uint32_t count_value,
                                   char name_first_value) {
        value.section_count_ = count_value;
        value.file_name_[0] = name_first_value;
        void *base = value.heap_.get_base();
        assert(base);
        for (int i = 0; i < allocation_count; ++i) {
            if (allocations[i].pointer == base && allocations[i].active) {
                allocations[i].count = &value.section_count_;
                allocations[i].name = value.file_name_;
                return;
            }
        }
        assert(!"TextIndex heap allocation was not tracked");
    }
};

struct Probe {
    bool alive;
    int label;
    Probe(int value) : alive(false), label(value) {}
};
struct Sentinel {
    Probe *probe;
    explicit Sentinel(Probe *value) : probe(value) { probe->alive = true; }
    ~Sentinel() {
        probe->alive = false;
        record_event(EVENT_SENTINEL, 0, probe->label, -1, -1);
    }
};
struct ObservedHeap : Heap {
    Sentinel sentinel;
    explicit ObservedHeap(Probe *probe) : sentinel(probe) {}
};
struct ObservedStrings : Strings {
    Sentinel sentinel;
    explicit ObservedStrings(Probe *probe) : sentinel(probe) {}
};
struct ObservedTextIndex : TextIndex {
    Sentinel sentinel;
    explicit ObservedTextIndex(Probe *probe) : sentinel(probe) {}
};

static void expect_event(int index, int kind, void *pointer = 0,
                         int label = -1, int count = -1, int name_byte = -1) {
    assert(index < event_count);
    assert(events[index].kind == kind);
    if (pointer || events[index].pointer) assert(events[index].pointer == pointer);
    assert(events[index].label == label);
    assert(events[index].count == count);
    assert(events[index].name_byte == name_byte);
}

static void check_heap_reset_and_reuse() {
    Probe probe(10);
    void *last = 0;
    reset_events();
    {
        ObservedHeap heap(&probe);
        heap.reset();
        assert(event_count == 0);  // null guard: no free call for empty storage
        assert(heap.get_base() == 0 && heap.current_ == 0 && heap.free_size_ == 0);

        assert(heap.init(32) == 0);
        void *first = heap.get_base();
        reset_events();
        heap.reset();
        assert(event_count == 1);
        expect_event(0, EVENT_FREE, first);
        assert(probe.alive && heap.get_base() == 0 && heap.current_ == 0);
        heap.reset();
        assert(event_count == 1); // repeat reset is harmless

        assert(heap.init(16) == 0); // same live object can be reused
        void *second = heap.get_base();
        assert(second);
        reset_events();
        heap.reset();
        assert(event_count == 1);
        expect_event(0, EVENT_FREE, second);
        assert(probe.alive);

        assert(heap.init(24) == 0);
        last = heap.get_base();
        reset_events();
    }
    assert(!probe.alive);
    assert(event_count == 2);
    expect_event(0, EVENT_SENTINEL, 0, 10);
    expect_event(1, EVENT_FREE, last); // implicit Heap base teardown after member
}

static void check_strings_shutdown_is_live_reset() {
    Probe probe(20);
    void *last = 0;
    reset_events();
    {
        ObservedStrings strings(&probe);
        Heap &heap = static_cast<Heap &>(strings);
        assert(heap.init(40) == 0);
        HeapLifetimeFixtureAccess::set_populated(strings, true);
        void *first = heap.get_base();
        reset_events();
        assert(strings.shutdown() == 0);
        assert(!HeapLifetimeFixtureAccess::is_populated(strings));
        assert(probe.alive && heap.get_base() == 0);
        assert(event_count == 1);
        expect_event(0, EVENT_FREE, first);
        assert(strings.shutdown() == 0);
        assert(event_count == 1);
        assert(heap.init(12) == 0); // shutdown did not end the base lifetime
        last = heap.get_base();
        reset_events();
    }
    assert(!probe.alive);
    assert(event_count == 2);
    expect_event(0, EVENT_SENTINEL, 0, 20);
    assert(events[1].kind == EVENT_FREE);
    expect_event(1, EVENT_FREE, last);
}

static void check_text_clear_and_count_gates() {
    void *bases[5];
    reset_events();
    {
        TextIndex indexes[5];
        const uint32_t counts[5] = {1, 0, 2, 1, 7};
        for (int i = 0; i != 5; ++i) {
            Heap &heap = HeapLifetimeFixtureAccess::heap(indexes[i]);
            assert(heap.init(16 + i) == 0);
            bases[i] = heap.get_base();
            HeapLifetimeFixtureAccess::prepare_text_index(indexes[i], counts[i], 'X');
        }
        reset_events();
        text_clear_index_source(indexes);
        assert(event_count == 3);
        expect_event(0, EVENT_FREE, bases[0], -1, 1, 'X');
        expect_event(1, EVENT_FREE, bases[2], -1, 2, 'X');
        expect_event(2, EVENT_FREE, bases[3], -1, 1, 'X');
        for (int i = 0; i != 4; ++i) {
            assert(HeapLifetimeFixtureAccess::count(indexes[i]) == 0);
        }
        assert(HeapLifetimeFixtureAccess::count(indexes[4]) == 7);
        assert(HeapLifetimeFixtureAccess::heap(indexes[1]).get_base() == bases[1]);
        assert(HeapLifetimeFixtureAccess::heap(indexes[4]).get_base() == bases[4]);

        text_clear_index_source(indexes);
        assert(event_count == 3); // cleared counts suppress repeat frees
    }
    assert(event_count == 5); // only the count-zero and out-of-range heaps remain
    expect_event(3, EVENT_FREE, bases[4], -1, 0, 0); // fifth element is outside clear loop
    expect_event(4, EVENT_FREE, bases[1], -1, 0, 0); // reverse array destruction
}

static Probe observed_index_probes[4] = {Probe(0), Probe(1), Probe(2), Probe(3)};
static int observed_index_next;
struct ObservedTextIndexArrayElement : TextIndex {
    Sentinel sentinel;
    ObservedTextIndexArrayElement() : sentinel(&observed_index_probes[observed_index_next++]) {}
};

static void check_ordinary_array_destruction_order() {
    void *bases[4];
    observed_index_next = 0;
    reset_events();
    {
        ObservedTextIndexArrayElement indexes[4];
        for (int i = 0; i != 4; ++i) {
            Heap &heap = HeapLifetimeFixtureAccess::heap(indexes[i]);
            assert(heap.init(24 + i) == 0);
            bases[i] = heap.get_base();
            HeapLifetimeFixtureAccess::prepare_text_index(indexes[i], 1, 'Y');
        }
        reset_events();
    }
    assert(event_count == 8);
    for (int i = 0; i != 4; ++i) {
        int index = 3 - i;
        expect_event(i * 2, EVENT_SENTINEL, 0, index);
        expect_event(i * 2 + 1, EVENT_FREE, bases[index], -1, 0, 0);
    }
}

int main() {
    check_heap_reset_and_reuse();
    check_strings_shutdown_is_live_reset();
    check_text_clear_and_count_gates();
    check_ordinary_array_destruction_order();
    return 0;
}
'''


def _class_definition(source: str, prefix: str) -> str:
    match = re.search(r"(?m)^" + re.escape(prefix), source)
    assert match, "actual class declaration not found: " + prefix
    start = match.start()
    opening = source.index("{", start + len(prefix))
    end = _closing_brace(source, opening)
    if end + 1 < len(source) and source[end + 1] == ";":
        end += 1
    return source[start:end + 1]


def _actual_sources():
    heap_cpp = (ROOT / "src/heap.cpp").read_text()
    strings_h = (ROOT / "src/strings.h").read_text()
    strings_cpp = (ROOT / "src/strings.cpp").read_text()
    textindex_h = (ROOT / "src/textindex.h").read_text()
    textindex_cpp = (ROOT / "src/textindex.cpp").read_text()

    strings_class = _class_definition(strings_h, "class Strings : public Heap")
    assert strings_class.count("class Strings : public Heap") == 1
    strings_class = strings_class.replace(
        "class Strings : public Heap {",
        "class Strings : public Heap {\n  friend struct HeapLifetimeFixtureAccess;", 1)
    textindex_class = _class_definition(textindex_h, "class TextIndex")
    assert textindex_class.count("friend struct HeapLifetimeFixtureAccess;") == 0
    textindex_class = textindex_class.replace(
        "class TextIndex {",
        "class TextIndex {\n  friend struct HeapLifetimeFixtureAccess;", 1)

    helper_start = textindex_h.index("inline void text_clear_index_source(")
    helper_open = textindex_h.index("{", helper_start)
    helper_end = _closing_brace(textindex_h, helper_open)
    clear_helper = textindex_h[helper_start:helper_end + 1]

    heap_constructor = definition(heap_cpp, r"(?m)^Heap::Heap\s*\(\)")
    heap_reset = definition(heap_cpp, r"(?m)^void Heap::reset\s*\(\)")
    heap_destructor = definition(heap_cpp, r"(?m)^Heap::~Heap\s*\(\)")
    heap_init = definition(heap_cpp, r"(?m)^BOOL Heap::init\s*\(int\s+req_size\)")
    strings_shutdown = definition(strings_cpp, r"(?m)^int Strings::shutdown\s*\(\)")
    textindex_constructor = definition(textindex_cpp, r"(?m)^TextIndex::TextIndex\s*\(\)")
    textindex_destructor = definition(textindex_cpp, r"(?m)^TextIndex::~TextIndex\s*\(\)")

    def bridge(body: str) -> str:
        count = body.count("std::free(base_);") + body.count("\n        free(base_);")
        body = body.replace("std::free(base_);", "host_heap_free(base_);")
        body = body.replace("\n        free(base_);", "\n        host_heap_free(base_);")
        assert count and "std::free(base_);" not in body
        assert "\n        free(base_);" not in body
        return body.replace("nullptr", "0")

    heap_constructor = heap_constructor.replace("nullptr", "0")
    heap_reset = bridge(heap_reset)
    heap_destructor = bridge(heap_destructor)
    heap_init = bridge(heap_init).replace("nullptr", "0")

    return {
        "strings_class": strings_class,
        "textindex_class": textindex_class,
        "heap_constructor": heap_constructor,
        "heap_reset": heap_reset,
        "heap_destructor": heap_destructor,
        "heap_init": heap_init,
        "strings_shutdown": strings_shutdown,
        "textindex_constructor": textindex_constructor,
        "textindex_destructor": textindex_destructor,
        "clear_helper": clear_helper,
    }


def _program(sources):
    result = FIXTURE.replace("@@HEAP_HEADER@@", str(ROOT / "src/heap.h"))
    for key, token in (
        ("strings_class", "// @@STRINGS_CLASS@@"),
        ("textindex_class", "// @@TEXTINDEX_CLASS@@"),
        ("heap_constructor", "// @@HEAP_CONSTRUCTOR@@"),
        ("heap_reset", "// @@HEAP_RESET@@"),
        ("heap_destructor", "// @@HEAP_DESTRUCTOR@@"),
        ("heap_init", "// @@HEAP_INIT@@"),
        ("strings_shutdown", "// @@STRINGS_SHUTDOWN@@"),
        ("textindex_constructor", "// @@TEXTINDEX_CONSTRUCTOR@@"),
        ("textindex_destructor", "// @@TEXTINDEX_DESTRUCTOR@@"),
        ("clear_helper", "// @@CLEAR_INDEX_HELPER@@"),
    ):
        assert result.count(token) == 1
        result = result.replace(token, sources[key], 1)
    for token in (
        "@@HEAP_HEADER@@", "@@STRINGS_CLASS@@", "@@TEXTINDEX_CLASS@@",
        "@@HEAP_CONSTRUCTOR@@", "@@HEAP_RESET@@", "@@HEAP_DESTRUCTOR@@",
        "@@HEAP_INIT@@", "@@STRINGS_SHUTDOWN@@", "@@TEXTINDEX_CONSTRUCTOR@@",
        "@@TEXTINDEX_DESTRUCTOR@@", "@@CLEAR_INDEX_HELPER@@",
    ):
        assert token not in result
    return result


def _compile_run(compiler, directory, label, sources, should_pass):
    cpp = directory / (label + ".cpp")
    binary = directory / label
    cpp.write_text(_program(sources))
    compiled = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(binary)],
        capture_output=True, text=True, timeout=45,
    )
    assert compiled.returncode == 0, compiled.stdout + compiled.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
    if should_pass:
        assert result.returncode == 0, result.stdout + result.stderr
    else:
        assert result.returncode != 0, "actual-source lifetime mutation escaped: " + label


@pytest.mark.parametrize("compiler_name", ("g++", "clang++"))
def test_actual_heap_strings_textindex_lifetimes_and_damage_controls(compiler_name):
    compiler = shutil.which(compiler_name)
    if not compiler:
        pytest.skip(compiler_name + " unavailable")
    sources = _actual_sources()
    with tempfile.TemporaryDirectory(prefix="heap-lifetime-role-oct17-") as temp:
        directory = Path(temp)
        _compile_run(compiler, directory, "actual", sources, True)

        reversed_clear = dict(sources)
        before = "indexes[i].heap_.reset();\n            indexes[i].section_count_ = 0;"
        after = "indexes[i].section_count_ = 0;\n            indexes[i].heap_.reset();"
        assert sources["clear_helper"].count(before) == 1
        reversed_clear["clear_helper"] = sources["clear_helper"].replace(before, after, 1)
        _compile_run(compiler, directory, "damage_reversed_clear", reversed_clear, False)

        no_null_guard = dict(sources)
        guard = "if (base_) {\n        host_heap_free(base_);"
        assert sources["heap_reset"].count(guard) == 1
        no_null_guard["heap_reset"] = sources["heap_reset"].replace(
            guard, "{\n        host_heap_free(base_);", 1)
        _compile_run(compiler, directory, "damage_null_guard", no_null_guard, False)


def test_source_selects_live_reset_for_strings_and_index_clear():
    sources = _actual_sources()
    clear_helper = sources["clear_helper"]
    call = "indexes[i].heap_.reset();"
    assert clear_helper.count(call) == 1
    assert "indexes[i].heap_.shutdown();" not in clear_helper
    strings_shutdown = sources["strings_shutdown"]
    assert "Heap::reset();" in strings_shutdown
    assert "Heap::~Heap" not in strings_shutdown
    # The reset and shutdown bodies intentionally share cleanup effects. The
    # coordinator's native role checker owns wrong-entry damage controls;
    # these host checks only keep source call sites explicit.


def _pinned_image():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image unavailable")
    return PeImage(path.read_bytes(), image_pin(ROOT))


def test_pinned_cleanup_bodies_and_constructor_destructor_call_roles():
    image = _pinned_image()
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    def decode(start, end):
        rows = list(decoder.disasm(image.read(Span(start, end)), start))
        assert sum(row.size for row in rows) == end - start
        return rows

    def has_transfer(rows, mnemonic, target):
        return any(row.mnemonic == mnemonic and row.operands
                   and row.operands[0].type == CS_OP_IMM
                   and row.operands[0].imm == target for row in rows)

    reset_rows = decode(0x5D4580, 0x5D45AA)
    destructor_rows = decode(0x5D45B0, 0x5D45DA)
    def normalize(rows, start, end):
        normalized = []
        for row in rows:
            if (row.mnemonic.startswith("j") and row.operands
                    and row.operands[0].type == CS_OP_IMM
                    and start <= row.operands[0].imm < end):
                normalized.append((row.mnemonic, row.operands[0].imm - start))
            else:
                normalized.append((row.mnemonic, row.op_str))
        return normalized

    assert normalize(reset_rows, 0x5D4580, 0x5D45AA) == normalize(
        destructor_rows, 0x5D45B0, 0x5D45DA)
    assert has_transfer(reset_rows, "call", 0x644EF2)
    assert has_transfer(destructor_rows, "call", 0x644EF2)
    assert reset_rows[0].address == 0x5D4580
    assert destructor_rows[0].address == 0x5D45B0

    strings_ctor = decode(0x6168D0, 0x6168E3)
    strings_dtor = decode(0x6169C0, 0x6169C5)
    text_ctor = decode(0x5FDF40, 0x5FDF5F)
    text_dtor = decode(0x5FDF60, 0x5FDF78)
    assert has_transfer(strings_ctor, "call", 0x5D4560)
    assert has_transfer(strings_dtor, "jmp", 0x5D45B0)
    assert has_transfer(text_ctor, "call", 0x5D4560)
    assert has_transfer(text_dtor, "jmp", 0x5D45B0)
    assert any(row.mnemonic == "lea" and len(row.operands) == 2
               and row.operands[0].type == CS_OP_REG
               and row.operands[0].reg == X86_REG_ECX
               and row.operands[1].mem.disp == 0x104 for row in text_ctor)
