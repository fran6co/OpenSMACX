"""Actual-source controls for WinNodeList's parent allocator member."""
from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path
import re

from match_recovery.includes import COMMENTS_OR_STRINGS


def root() -> Path:
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/winnodelist.h").is_file():
            return parent
    raise RuntimeError("cannot locate OpenSMACX source tree")


def body(source: str, signature: str) -> str:
    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if c == "\n" else " " for c in match[0]), source)
    start = masked.find(signature)
    assert start >= 0 and masked.find(signature, start + len(signature)) < 0
    opening = masked.find("{", start + len(signature))
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def test_all_parent_allocator_reads_use_the_typed_list_member():
    repo = root()
    source = (repo / "src/win.cpp").read_text(encoding="utf-8")
    init = body(source, "int Win::init(int x, int y, int width, int height,")
    set_parent = body(source, "void Win::set_parent_dialog(Win *dialog)")
    remove_parent = body(source, "void Win::remove_parent_dialog()")
    close = body(source, "void Win::close()")

    assert "parent_window->list_.allocator_" in init
    assert "parent + 0xdc" not in init.lower()
    assert set_parent.count("dialog->list_.allocator_") == 8
    assert "b + 0xdc" not in set_parent.lower()
    assert "parent->list_.allocator_ == 0" in remove_parent
    assert "parent->list_.allocator_ == 0" in close
    assert "reinterpret_cast<Heap *>(*reinterpret_cast<int *>(" not in set_parent

    header = (repo / "src/winnodelist.h").read_text(encoding="utf-8")
    assert "class Heap;" in header
    assert "Heap *allocator_;" in header
    assert "external_" not in header
    assert "allocator_ == 0 && count_ > 0" in header
    assert "005EC493/0x005EC4A7" in header
    assert "0x005EC9A0" in header and "0x005EBCD7" in header
    assert "No writer" in header and "nonzero allocator is identified" in header


def test_source_allocation_expressions_select_heap_and_malloc_paths():
    repo = root()
    source = (repo / "src/win.cpp").read_text(encoding="utf-8")
    init = body(source, "int Win::init(int x, int y, int width, int height,")
    set_parent = body(source, "void Win::set_parent_dialog(Win *dialog)")
    init_node = re.search(r"WinDialogNode \*newNode = (.*?);", init, re.S)
    assert init_node is not None
    set_nodes = re.findall(r"WinDialogNode \*(?:iv2|newnode) = (.*?);", set_parent, re.S)
    assert len(set_nodes) == 2
    # Run the actual source ternaries in small host functions. Only pointer
    # selection and requested sizes are under test; this is not a whole-body
    # execution or a claim about the original runtime's nonzero writer.
    node_exprs = [set_nodes[0], init_node.group(1)]
    assert all("get(0x1c)" in expr.lower() and "mem_get(0x1c)" in expr.lower()
               for expr in node_exprs)
    heap_payloads = re.findall(r"dialog->list_\.allocator_->get\(4\)", set_parent)
    fallback_payloads = re.findall(r"= mem_get\(4\);", set_parent)
    assert len(heap_payloads) == len(fallback_payloads) == 2

    compilers = [path for name in ("clang++", "g++") if (path := shutil.which(name))]
    assert compilers, "a host C++ compiler is required for allocation controls"
    # Keep the source expressions verbatim inside the harness.
    program = f'''#include <cassert>
#include <cstddef>
#include <vector>
#include "winnodelist.h"
static std::vector<std::size_t> heap_requests, malloc_requests;
struct Heap {{ void *get(std::size_t n) {{ heap_requests.push_back(n); return reinterpret_cast<void *>(n + 16); }} }};
static void *mem_get(std::size_t n) {{ malloc_requests.push_back(n); return reinterpret_cast<void *>(n + 32); }}
struct Dialog {{ WinNodeList list_; }};
static void *set_node_a(Dialog *dialog) {{ return {node_exprs[0]}; }}
static void *set_node_b(Dialog *dialog) {{ return {node_exprs[0]}; }}
static WinDialogNode *init_node(Dialog *parent_window) {{ return {node_exprs[1]}; }}
static void *set_payload_heap(Dialog *dialog) {{ return {heap_payloads[0]}; }}
static void *set_payload_malloc() {{ return mem_get(4); }}
int main() {{
  Dialog dialog;
  Heap heap;
  dialog.list_.allocator_ = &heap;
  assert(set_node_a(&dialog) == reinterpret_cast<void *>(0x1c + 16));
  assert(set_node_b(&dialog) == reinterpret_cast<void *>(0x1c + 16));
  assert(set_payload_heap(&dialog) == reinterpret_cast<void *>(4 + 16));
  assert((heap_requests == std::vector<std::size_t>{{0x1c, 0x1c, 4}}));
  dialog.list_.allocator_ = 0;
  assert(set_node_a(&dialog) == reinterpret_cast<void *>(0x1c + 32));
  assert(init_node(&dialog) == reinterpret_cast<WinDialogNode *>(0x1c + 32));
  assert(set_payload_malloc() == reinterpret_cast<void *>(4 + 32));
  assert((malloc_requests == std::vector<std::size_t>{{0x1c, 0x1c, 4}}));
}}
'''
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="win-list-allocation-") as temp:
            source_path = Path(temp) / "allocation.cpp"
            executable = Path(temp) / "allocation"
            source_path.write_text(program, encoding="utf-8")
            subprocess.run([compiler, "-std=c++11", "-iquote", "src", str(source_path), "-o",
                            str(executable)], cwd=repo, check=True, timeout=60)
            subprocess.run([str(executable)], check=True, timeout=10)


def test_real_header_preserves_nonzero_allocator_teardown_guard():
    compilers = [path for name in ("clang++", "g++") if (path := shutil.which(name))]
    assert compilers, "a host C++ compiler is required for the allocator control"
    repo = root()
    program = r'''#include <cassert>
#include <cstddef>
#include <cstring>
#include <new>
#include "winnodelist.h"
int main() {
  alignas(WinNodeList) unsigned char owned_storage[sizeof(WinNodeList)];
  WinNodeList *owned = new (owned_storage) WinNodeList;
  assert(owned->allocator_ == 0);
  owned->head_ = reinterpret_cast<WinDialogNode *>(1);
  owned->current_ = reinterpret_cast<WinDialogNode *>(2);
  owned->count_ = 3;
  owned->tail_ = 2;
  owned->allocator_ = reinterpret_cast<Heap *>(3);
  const std::size_t head_offset = reinterpret_cast<unsigned char *>(&owned->head_) - owned_storage;
  const std::size_t current_offset = reinterpret_cast<unsigned char *>(&owned->current_) - owned_storage;
  const std::size_t count_offset = reinterpret_cast<unsigned char *>(&owned->count_) - owned_storage;
  const std::size_t tail_offset = reinterpret_cast<unsigned char *>(&owned->tail_) - owned_storage;
  owned->~WinNodeList();
  void *head; void *current; int count; int tail;
  std::memcpy(&head, owned_storage + head_offset, sizeof(head));
  std::memcpy(&current, owned_storage + current_offset, sizeof(current));
  std::memcpy(&count, owned_storage + count_offset, sizeof(count));
  std::memcpy(&tail, owned_storage + tail_offset, sizeof(tail));
  assert(head == 0 && tail == 0 && count == 0);
  assert(current == reinterpret_cast<void *>(2));

  alignas(WinNodeList) unsigned char empty_storage[sizeof(WinNodeList)];
  WinNodeList *empty = new (empty_storage) WinNodeList;
  empty->count_ = 9;
  empty->tail_ = 4;
  const std::size_t empty_count_offset = reinterpret_cast<unsigned char *>(&empty->count_) - empty_storage;
  const std::size_t empty_tail_offset = reinterpret_cast<unsigned char *>(&empty->tail_) - empty_storage;
  empty->~WinNodeList();
  std::memcpy(&count, empty_storage + empty_count_offset, sizeof(count));
  std::memcpy(&tail, empty_storage + empty_tail_offset, sizeof(tail));
  assert(count == 9 && tail == 0);
}
'''
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix="winnodelist-allocator-") as temp:
            source = Path(temp) / "control.cpp"
            executable = Path(temp) / "control"
            source.write_text(program, encoding="utf-8")
            subprocess.run([compiler, "-std=c++11", "-iquote", "src", str(source), "-o",
                            str(executable)], cwd=repo, check=True, timeout=60)
            subprocess.run([str(executable)], check=True, timeout=10)
