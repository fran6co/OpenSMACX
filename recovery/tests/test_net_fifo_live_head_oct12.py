"""Full actual FIFO-owner controls; host semantics, not native ABI/game proof.

The payload-to-head witnesses use allocated nodes and non-overlapping memcpy
buffers. A native DWORD pointer is a host-sized real pointer here; no truncated
pointer, raw object offset, fabricated callback inside memcpy, or native fault
is asserted. Native instructions independently establish each later reload,
including those after scalar writes that cannot portably replace a host pointer.
The historical overlapping get span remains an obligation, not a corrected span.
"""
from __future__ import annotations

import os
import json
from pathlib import Path
import resource
import shutil
import subprocess

import pytest

from test_net_fifo_actual_source import _class_declaration, _code_without_comments
from test_netwin_owner_behavior import extract_function

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/net_fifo.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
GET = "int NetFifo::get(void *data, unsigned int *key, int *type,"
PEEK = "int NetFifo::peek_head("
HEAD_METHODS = (GET, PEEK)

PREFIX = r'''
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <new>
#include <string>
#include <vector>
struct CRITICAL_SECTION { int depth; };
static std::string events;
static std::vector<void *> freed;
void InitializeCriticalSection(CRITICAL_SECTION *p) { p->depth = 0; }
void DeleteCriticalSection(CRITICAL_SECTION *p) { assert(p->depth == 0); }
void EnterCriticalSection(CRITICAL_SECTION *p) {
    assert(p->depth == 0); ++p->depth; events += 'E';
}
void LeaveCriticalSection(CRITICAL_SECTION *p) {
    assert(p->depth == 1); --p->depth; events += 'L';
}
void *mem_get(size_t size);
static void tracked_free(void *p) {
    if (p) { freed.push_back(p); events += 'F'; std::free(p); }
}
static void *tracked_copy(void *out, const void *in, size_t size) {
    events += 'M'; return std::memcpy(out, in, size);
}
#define free tracked_free
#define memcpy tracked_copy
'''

SUPPORT = r'''
void *mem_get(size_t size) {
    // The complete TU includes untouched native-0x18 add(), although these
    // head-reader controls never call it. Back a node-sized request with a
    // complete wider-pointer host node, as the transport owner fixture does.
    // This is not native allocation/layout proof or a product size change.
    if (size == 0x18 && size < sizeof(NetFifo::NetFifoNode))
        size = sizeof(NetFifo::NetFifoNode);
    return std::malloc(size);
}
'''

MAIN = r'''
#undef free
#undef memcpy
typedef NetFifo::NetFifoNode Node;
static Node *node(int key, int caller, int result) {
    Node *p = new Node();
    p->key_ = static_cast<unsigned int>(key);
    p->caller_value_ = caller; p->result_ = result;
    return p;
}
// Nodes passed to the product's free are genuinely malloc-owned, not new-owned.
static Node *malloc_node(int key, int caller, int result) {
    Node *p = static_cast<Node *>(std::malloc(sizeof(Node)));
    assert(p); new (p) Node();
    p->key_ = static_cast<unsigned int>(key);
    p->caller_value_ = caller; p->result_ = result;
    return p;
}
static bool was_freed(void *p) {
    for (size_t i = 0; i < freed.size(); ++i) if (freed[i] == p) return true;
    return false;
}
static void reset() { events.clear(); freed.clear(); }
static void detach(NetFifo &q) { q.head_ = q.tail_ = 0; q.count_ = 0; }
int main() {
    {
        NetFifo q;
        Node *a = malloc_node(11, 12, 13), *b = malloc_node(21, 22, 23);
        Node *c = malloc_node(31, 32, 33), *d = malloc_node(41, 42, 43);
        a->next_ = c; b->next_ = d;
        void *payload = std::malloc(sizeof(b)); assert(payload);
        std::memcpy(payload, &b, sizeof(b));
        a->data_ = payload; a->data_size_ = sizeof(b);
        q.head_ = a; q.tail_ = c; q.count_ = 2;
        unsigned int key = 0, size = 0; int caller = 0;
        reset();
        int result = q.get(&q.head_, &key, &caller, &size);
        assert(q.head_ == d && q.tail_ == c && q.count_ == 1);
        assert(key == 11 && caller == 12 && result == 13 && size == sizeof(b));
        assert(events == "EMFFL" && was_freed(payload) && was_freed(a));
        assert(!was_freed(b) && !was_freed(c) && !was_freed(d));
        detach(q); std::free(b); std::free(c); std::free(d);
    }
    {
        NetFifo q;
        Node *a = node(11, 12, 13), *b = node(21, 22, 23);
        void *payload = std::malloc(sizeof(b)); assert(payload);
        std::memcpy(payload, &b, sizeof(b));
        a->data_ = payload; a->data_size_ = sizeof(b);
        q.head_ = q.tail_ = a; q.count_ = 1;
        unsigned int key = 0; int caller = 0, size = 0;
        reset();
        int result = q.peek_head(&q.head_, &key, &caller, &size);
        assert(q.head_ == b && q.tail_ == a && q.count_ == 1);
        assert(key == 21 && caller == 22 && result == 23 && size == sizeof(b));
        assert(events == "EML" && freed.empty());
        detach(q); std::free(payload); delete a; delete b;
    }
    {
        NetFifo q; Node *a = node(31, 32, 33);
        a->data_size_ = 17; q.head_ = q.tail_ = a; q.count_ = 1;
        // Corresponding signed/unsigned int aliasing is permitted C++ aliasing.
        // Size then key then caller stores must precede the result read.
        reset();
        int result = q.peek_head(0, reinterpret_cast<unsigned int *>(&a->caller_value_),
                                 &a->result_, &a->result_);
        assert(result == 31 && a->caller_value_ == 31 && a->result_ == 31);
        assert(q.head_ == a && q.tail_ == a && q.count_ == 1);
        assert(events == "EL" && freed.empty());
        detach(q); delete a;
    }
    {
        NetFifo q; Node *a = malloc_node(51, 52, 53);
        a->data_size_ = 17; q.head_ = q.tail_ = a; q.count_ = 1;
        reset();
        int result = q.get(0, reinterpret_cast<unsigned int *>(&a->caller_value_),
                           &a->result_, reinterpret_cast<unsigned int *>(&a->result_));
        assert(result == 51 && q.head_ == 0 && q.tail_ == a && q.count_ == 0);
        assert(events == "EFL" && freed.size() == 1 && was_freed(a));
        detach(q);
    }
    {
        NetFifo q; Node *a = malloc_node(61, 62, 63);
        void *replacement = std::malloc(8); assert(replacement);
        void *original = std::malloc(sizeof(replacement)); assert(original);
        std::memcpy(original, &replacement, sizeof(replacement));
        a->data_ = original; a->data_size_ = sizeof(replacement);
        q.head_ = q.tail_ = a; q.count_ = 1;
        reset();
        assert(q.get(&a->data_, static_cast<unsigned int *>(0), 0, 0) == 63);
        assert(events == "EMFFL" && was_freed(replacement) && was_freed(a));
        assert(!was_freed(original) && q.head_ == 0 && q.tail_ == a && q.count_ == 0);
        detach(q); std::free(original);
    }
    {
        NetFifo q; Node *a = malloc_node(64, 65, 66);
        void *replacement = 0;
        void *original = std::malloc(sizeof(replacement)); assert(original);
        std::memcpy(original, &replacement, sizeof(replacement));
        a->data_ = original; a->data_size_ = sizeof(replacement);
        q.head_ = q.tail_ = a; q.count_ = 1;
        reset();
        // A valid copy may clear the selected payload pointer, so the late
        // conditional free skips it and leaks the original allocation.
        assert(q.get(&a->data_, static_cast<unsigned int *>(0), 0, 0) == 66);
        assert(events == "EMFL" && was_freed(a) && !was_freed(original));
        assert(q.head_ == 0 && q.count_ == 0);
        detach(q); std::free(original);
    }
    {
        NetFifo q; Node *a = malloc_node(71, 72, 0);
        void *payload = std::malloc(4); assert(payload);
        a->data_ = payload; a->data_size_ = 4;
        q.head_ = q.tail_ = a; q.count_ = 0;
        reset();
        // Native null-output leak, zero result despite removal, stale tail,
        // and unsigned decrement of partial/inconsistent state all survive.
        assert(q.get(0, static_cast<unsigned int *>(0), 0, 0) == 0);
        assert(events == "EFL" && was_freed(a) && !was_freed(payload));
        assert(q.head_ == 0 && q.tail_ == a && q.count_ == UINT32_MAX);
        detach(q); std::free(payload);
    }
    {
        NetFifo q; Node *a = malloc_node(81, 82, -1);
        a->data_size_ = 19; q.head_ = q.tail_ = a; q.count_ = 1;
        unsigned int key = 0, size = 0; int caller = 0; char out[32] = {};
        reset();
        assert(q.get(out, &key, &caller, &size) == -1);
        assert(key == 81 && caller == 82 && size == 19 && out[0] == 0);
        assert(events == "EFL" && q.head_ == 0 && q.tail_ == a && q.count_ == 0);
        detach(q);
    }
    {
        NetFifo q; Node *a = node(91, 92, 0);
        char payload[3] = {'a', 'b', 'c'};
        a->data_ = payload; a->data_size_ = 3;
        q.head_ = q.tail_ = a; q.count_ = 1;
        unsigned int key = 0; int caller = 0, size = 0;
        reset();
        assert(q.peek_head(0, &key, &caller, &size) == 0);
        assert(key == 91 && caller == 92 && size == 3);
        assert(events == "EL" && freed.empty() && q.head_ == a && q.count_ == 1);
        detach(q); delete a;
    }
    {
        NetFifo q; Node *a = malloc_node(101, 102, 103);
        void *payload = std::malloc(1); assert(payload);
        a->data_ = payload; a->data_size_ = 0;
        q.head_ = q.tail_ = a; q.count_ = 1;
        unsigned int key = 0, length = 99; int caller = 0, size = 99;
        char out = 'x';
        reset();
        assert(q.peek_head(&out, &key, &caller, &size) == 103);
        assert(events == "EML" && freed.empty() && size == 0 && out == 'x');
        reset();
        assert(q.get(&out, &key, &caller, &length) == 103);
        assert(events == "EMFFL" && was_freed(payload) && was_freed(a));
        assert(length == 0 && key == 101 && caller == 102 && out == 'x');
        assert(q.head_ == 0 && q.tail_ == a && q.count_ == 0);
        detach(q);
    }
    {
        NetFifo q; Node *stale = node(1, 2, 3);
        q.tail_ = stale; q.count_ = 5;
        unsigned int key = 99, length = 99; int caller = 99, size = 99;
        char out[8] = {};
        reset();
        assert(q.get(out, &key, &caller, &length) == 0);
        assert(q.peek_head(out, &key, &caller, &size) == 0);
        assert(key == 99 && caller == 99 && length == 99 && size == 99 && out[0] == 0);
        assert(events == "ELEL" && freed.empty());
        assert(q.head_ == 0 && q.tail_ == stale && q.count_ == 5);
        detach(q); delete stale;
    }
}
'''


def owner_probe(source: str) -> str:
    declaration = _class_declaration((ROOT / "src/net_fifo.h").read_text())
    definitions = _code_without_comments(source)
    assert extract_function(source, GET) in source
    assert extract_function(source, PEEK) in source
    return PREFIX + declaration + "\n" + SUPPORT + definitions + MAIN


def restore_live_head(source: str) -> str:
    """Reverse only this two-body closure, retaining its original baseline.

    Older exact scope guards can use this bridge before their existing checks;
    unrelated source and any mapping/claim/lesson changes still refuse.
    """
    from match_recovery.source import read_text
    task = json.loads((ROOT / "recovery/tasks/net-fifo-live-head-oct12.json").read_text())
    assert task["base_revision"] == "908fd8cdcd7db78475f500bb03c4f143560bdb98"
    assert {subject["address"] for subject in task["subjects"]} == {0x633F70, 0x634200}
    baseline = subprocess.check_output(
        ["git", "show", task["base_revision"] + ":src/net_fifo.cpp"], cwd=ROOT, text=True)
    def identity(row):
        return (row.address, row.name, row.image_spans, row.symbol, row.byte_exact,
                row.semantic, row.source_role, row.source_role_reason, row.function_fragment,
                row.mode, row.state, row.body, row.kind, row.exclusion,
                row.levers, row.ruled_out, row.unrecoverable, row.deferred,
                tuple((lesson.kind, lesson.key, lesson.prose,
                       tuple(text for _, text in lesson.lines)) for lesson in row.origin))
    assert [identity(row) for row in read_text(source, SOURCE)] == [
        identity(row) for row in read_text(baseline, SOURCE)]
    restored = source
    for signature in HEAD_METHODS:
        assert signature in restored, "authorized head body cannot be restored"
        current = extract_function(restored, signature)
        assert restored.count(current) == 1
        restored = restored.replace(current, extract_function(baseline, signature), 1)
    assert restored == baseline, "outside the two authorized head-reader bodies changed"
    return restored


@pytest.mark.parametrize("mutant", [None, "outside-body", "original-identity", "failed-restoration"])
def test_two_body_scope_and_historical_identity_damage(mutant):
    source = SOURCE.read_text()
    if mutant == "outside-body":
        needle = "this->count_ += 1;"
        assert source.count(needle) == 1
        source = source.replace(needle, "this->count_ += 2;", 1)
    elif mutant == "original-identity":
        from match_recovery.source import read_text
        row = next(row for row in read_text(source, SOURCE) if row.address == 0x633F70)
        source = source.replace(row.name, row.name + "_damaged", 1)
    elif mutant == "failed-restoration":
        assert source.count(GET) == 1
        source = source.replace(GET, GET.replace("unsigned int *key", "unsigned int key"), 1)
    if mutant:
        with pytest.raises(AssertionError):
            restore_live_head(source)
    else:
        restore_live_head(source)


def damage(source: str, name: str) -> str:
    get = extract_function(source, GET)
    peek = extract_function(source, PEEK)
    if name == "selected_unlink":
        changed = get.replace("head_ = head_->next_;", "head_ = node->next_;")
        before = get
    elif name == "selected_peek":
        changed = peek.replace("head_->", "node->")
        before = peek
    elif name == "free_live_head":
        changed = get.replace("free(node);", "free(head_);")
        before = get
    elif name == "live_get_metadata":
        changed = get.replace("*key = node->key_;", "*key = head_->key_;")
        before = get
    elif name == "eager_get_result":
        changed = get.replace("int result = node->result_;", "")
        changed = changed.replace("if (key != 0) {", "int result = node->result_;\n  if (key != 0) {")
        before = get
    elif name == "eager_peek_result":
        changed = peek.replace("const int result = head_->result_;", "")
        changed = changed.replace("if (size_out != 0) {", "const int result = head_->result_;\n    if (size_out != 0) {")
        before = peek
    elif name == "cached_payload_free":
        changed = get.replace("memcpy(data, node->data_, node->data_size_);",
                              "void *saved_data = node->data_;\n    memcpy(data, node->data_, node->data_size_);")
        changed = changed.replace("free(node->data_);", "free(saved_data);")
        before = get
    elif name == "repair_null_output_leak":
        changed = get.replace("if (node->data_ != 0 && data != 0) {", "if (node->data_ != 0) {")
        changed = changed.replace("memcpy(data, node->data_, node->data_size_);",
                              "if (data != 0) memcpy(data, node->data_, node->data_size_);")
        before = get
    elif name == "peek_removes_count":
        changed = peek.replace("LeaveCriticalSection(&lock_);\n    return result;",
                               "--count_;\n    LeaveCriticalSection(&lock_);\n    return result;")
        before = peek
    else:
        raise AssertionError(name)
    assert changed != before, f"missing damage anchor: {name}"
    assert source.count(before) == 1
    return source.replace(before, changed, 1)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("mutant", [None, "selected_unlink", "selected_peek", "free_live_head",
                                   "live_get_metadata", "eager_get_result", "eager_peek_result",
                                   "cached_payload_free", "repair_null_output_leak", "peek_removes_count"])
def test_actual_complete_owner_and_executed_damage(tmp_path, compiler, mutant):
    if not shutil.which(compiler):
        pytest.skip(f"{compiler} unavailable; host control not executed")
    source = SOURCE.read_text()
    if mutant:
        source = damage(source, mutant)
    binary = tmp_path / "fifo-head-probe"
    build = subprocess.run([compiler, "-std=c++17", "-Wall", "-Wextra", "-Werror",
                            "-x", "c++", "-", "-o", str(binary)],
                           input=owner_probe(source), text=True, capture_output=True, timeout=60)
    assert build.returncode == 0, build.stdout + build.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10,
                            preexec_fn=lambda: resource.setrlimit(resource.RLIMIT_CORE, (0, 0)))
    if mutant:
        assert result.returncode != 0 and "assert" in result.stderr.lower(), result.stderr
        witness = {
            "selected_unlink": "q.head_ == d",
            "selected_peek": "key == 21",
            "free_live_head": "was_freed(a)",
            "live_get_metadata": "key == 11",
            "eager_get_result": "result == 51",
            "eager_peek_result": "key == 21",
            "cached_payload_free": "was_freed(replacement)",
            "repair_null_output_leak": '!was_freed(payload)',
            "peek_removes_count": "q.head_ == b",
        }
        assert witness[mutant] in result.stderr, result.stderr
    else:
        assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_owner_optimized_undefined_behavior_control(tmp_path, compiler):
    if not shutil.which(compiler):
        pytest.skip(f"{compiler} unavailable; host control not executed")
    binary = tmp_path / "fifo-head-optimized"
    build = subprocess.run([compiler, "-std=c++17", "-O2", "-Wall", "-Wextra", "-Werror",
                            "-fsanitize=undefined", "-fno-sanitize-recover=all",
                            "-x", "c++", "-", "-o", str(binary)],
                           input=owner_probe(SOURCE.read_text()), text=True, capture_output=True, timeout=60)
    assert build.returncode == 0, build.stdout + build.stderr
    result = subprocess.run([str(binary)], capture_output=True, text=True, timeout=10)
    assert result.returncode == 0, result.stdout + result.stderr


def test_pinned_full_head_reader_control_and_live_reads():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    assert path.is_absolute(), "native pin requires absolute image path"
    if not path.is_file():
        pytest.skip("pinned executable unavailable; native evidence not checked")
    image = PeImage(path.read_bytes(), PIN)
    disassembler = Cs(CS_ARCH_X86, CS_MODE_32)
    instructions = {}
    for start, end in ((0x633F70, 0x63401D), (0x634200, 0x63427E)):
        rows = list(disassembler.disasm(image.read(Span(start, end)), start))
        assert rows[0].address == start and rows[-1].address + rows[-1].size == end
        assert sum(row.size for row in rows) == end - start
        assert (rows[-1].mnemonic, rows[-1].op_str) == ("ret", "0x10")
        instructions.update({row.address: (row.mnemonic, row.op_str) for row in rows})
    expected = {
        0x633F79: ("call", "dword ptr [0x66917c]"),
        0x633F7F: ("mov", "esi, dword ptr [edi]"),
        0x633F83: ("jne", "0x633f94"),
        0x633F86: ("call", "dword ptr [0x669174]"),
        0x633F9A: ("je", "0x633fa1"),
        0x633F9C: ("mov", "ecx, dword ptr [esi + 0x10]"),
        0x633F9F: ("mov", "dword ptr [eax], ecx"),
        0x633FA1: ("mov", "eax, dword ptr [esi + 0xc]"),
        0x633FA6: ("je", "0x633fd5"),
        0x633FAE: ("je", "0x633fd5"),
        0x633FB0: ("mov", "edx, dword ptr [esi + 0x10]"),
        0x633FB6: ("call", "0x645930"),
        0x633FBB: ("mov", "eax, dword ptr [esi + 0xc]"),
        0x633FC3: ("je", "0x633fce"),
        0x633FC6: ("call", "0x644ef2"),
        0x633FCE: ("mov", "dword ptr [esi + 0xc], 0"),
        0x633FD5: ("mov", "eax, dword ptr [edi]"),
        0x633FD7: ("mov", "ecx, dword ptr [eax + 0x14]"),
        0x633FE0: ("mov", "dword ptr [edi], ecx"),
        0x633FE2: ("je", "0x633fe9"),
        0x633FE4: ("mov", "edx, dword ptr [esi + 4]"),
        0x633FE7: ("mov", "dword ptr [eax], edx"),
        0x633FEF: ("je", "0x633ff5"),
        0x633FF1: ("mov", "ecx, dword ptr [esi]"),
        0x633FF3: ("mov", "dword ptr [eax], ecx"),
        0x633FF6: ("mov", "ebx, dword ptr [esi + 8]"),
        0x633FFB: ("je", "0x634006"),
        0x633FFE: ("call", "0x644ef2"),
        0x634006: ("mov", "ecx, dword ptr [edi + 8]"),
        0x63400A: ("dec", "ecx"),
        0x63400B: ("mov", "dword ptr [edi + 8], ecx"),
        0x63400E: ("call", "dword ptr [0x669174]"),
        0x634014: ("mov", "eax, ebx"),
        0x634208: ("call", "dword ptr [0x66917c]"),
        0x63420E: ("mov", "eax, dword ptr [esi]"),
        0x634212: ("jne", "0x634222"),
        0x634215: ("call", "dword ptr [0x669174]"),
        0x634228: ("je", "0x63422f"),
        0x63422A: ("mov", "eax, dword ptr [eax + 0x10]"),
        0x63422D: ("mov", "dword ptr [ecx], eax"),
        0x63422F: ("mov", "eax, dword ptr [esi]"),
        0x634231: ("mov", "ecx, dword ptr [eax + 0xc]"),
        0x634236: ("je", "0x63424e"),
        0x63423E: ("je", "0x63424e"),
        0x634240: ("mov", "eax, dword ptr [eax + 0x10]"),
        0x634246: ("call", "0x645930"),
        0x634254: ("je", "0x63425d"),
        0x634256: ("mov", "ecx, dword ptr [esi]"),
        0x634258: ("mov", "edx, dword ptr [ecx + 4]"),
        0x63425B: ("mov", "dword ptr [eax], edx"),
        0x634263: ("je", "0x63426b"),
        0x634265: ("mov", "ecx, dword ptr [esi]"),
        0x634267: ("mov", "edx, dword ptr [ecx]"),
        0x634269: ("mov", "dword ptr [eax], edx"),
        0x63426B: ("mov", "eax, dword ptr [esi]"),
        0x63426E: ("mov", "esi, dword ptr [eax + 8]"),
        0x634271: ("call", "dword ptr [0x669174]"),
        0x634277: ("mov", "eax, esi"),
    }
    for address, instruction in expected.items():
        assert instructions[address] == instruction, hex(address)
    source = SOURCE.read_text()
    assert "0x00633F70-0x00634067" in source
    assert "0x00634200-0x0063427E" in source
