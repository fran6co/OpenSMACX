"""Actual-source host controls for the original VideoFrameQueue effects.

The fixture extracts the seven mapped definitions and their real node/queue
declarations.  It supplies only allocator/free stand-ins so error branches can
be observed without changing product code or relying on the game's allocator.
"""
from __future__ import annotations

import pathlib
import re
import shutil
import subprocess

import pytest


ROOT = pathlib.Path(__file__).resolve().parents[2]
QUEUE_CPP = ROOT / "src/video_queue.cpp"
VIDEO_HEADER = ROOT / "src/video.h"
PINNED_SPANS = (
    ("0x0063E520", "0x0063E520-0x0063E534"),
    ("0x0063E540", "0x0063E540-0x0063E58F"),
    ("0x0063E590", "0x0063E590-0x0063E5DF"),
    ("0x0063E5E0", "0x0063E5E0-0x0063E6DD"),
    ("0x0063E6E0", "0x0063E6E0-0x0063E7EE"),
    ("0x0063E7F0", "0x0063E7F0-0x0063E81C"),
    ("0x0063E820", "0x0063E820-0x0063E85B"),
)
DEFINITIONS = (
    ("VideoFrameQueue::VideoFrameQueue()", "VideoFrameQueue::VideoFrameQueue()"),
    ("void VideoFrameQueue::set_limit(int limit)", "void VideoFrameQueue::set_limit(int limit)"),
    ("int VideoFrameQueue::allocate(", "int VideoFrameQueue::allocate("),
    ("int VideoFrameQueue::append(", "int VideoFrameQueue::append("),
    ("unsigned char *VideoFrameQueue::peek(", "unsigned char *VideoFrameQueue::peek("),
    ("void VideoFrameQueue::discard_head()", "void VideoFrameQueue::discard_head()"),
    ("void VideoFrameQueue::clear()", "void VideoFrameQueue::clear()"),
)


def _block_end(text: str, opening: int) -> int:
    """Find a C++ brace block end while ignoring comments and quoted braces."""
    depth = 0
    state = "code"
    index = opening
    while index < len(text):
        char = text[index]
        following = text[index + 1] if index + 1 < len(text) else ""
        if state == "line_comment":
            if char == "\n":
                state = "code"
        elif state == "block_comment":
            if char == "*" and following == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and following == "/":
            state = "line_comment"
            index += 1
        elif char == "/" and following == "*":
            state = "block_comment"
            index += 1
        elif char == '"':
            state = "string"
        elif char == "'":
            state = "char"
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return index + 1
        index += 1
    raise AssertionError("unterminated C++ brace block")


def _extract_definition(source: str, signature: str) -> str:
    starts = [match.start() for match in re.finditer(re.escape(signature), source)]
    assert len(starts) == 1, f"expected one definition for {signature!r}, got {len(starts)}"
    start = starts[0]
    opening = source.find("{", start + len(signature))
    assert opening >= 0, f"missing body for {signature!r}"
    end = _block_end(source, opening)
    return source[start:end]


def _extract_declaration(source: str, signature: str) -> str:
    starts = [match.start() for match in re.finditer(re.escape(signature), source)]
    assert len(starts) == 1, f"expected one declaration for {signature!r}, got {len(starts)}"
    start = starts[0]
    opening = source.find("{", start + len(signature))
    assert opening >= 0, f"missing declaration body for {signature!r}"
    end = _block_end(source, opening)
    semicolon = source.find(";", end)
    assert semicolon >= 0
    return source[start:semicolon + 1]


def _source_fixture(mutation: str | None = None) -> str:
    cpp = QUEUE_CPP.read_text(encoding="utf-8")
    header = VIDEO_HEADER.read_text(encoding="utf-8")
    methods = {key: _extract_definition(cpp, signature) for key, signature in DEFINITIONS}
    node_decl = _extract_declaration(header, "struct VideoFrameNode")
    queue_decl = _extract_declaration(header, "class VideoFrameQueue")
    # This access-only fixture change permits assertions on the queue's exact
    # state after its native error paths; it does not alter field order/layout.
    queue_decl = queue_decl.replace("private:", "public:")

    if mutation == "skip_failure_cleanup":
        for key in ("int VideoFrameQueue::allocate(", "int VideoFrameQueue::append("):
            old = "discard_head();"
            assert old in methods[key]
            methods[key] = methods[key].replace(old, "/* damaged: no head discard */", 1)
    elif mutation == "initialize_zero_byte_fields":
        key = "int VideoFrameQueue::allocate("
        needle = "    if (byte_count != 0) {"
        assert needle in methods[key]
        methods[key] = methods[key].replace(
            needle,
            "    tail_->bytes = 0;\n    tail_->byte_count = 0;\n" + needle,
            1,
        )
    elif mutation == "peek_ignores_optional_output":
        key = "unsigned char *VideoFrameQueue::peek("
        old = "if (next_frame_id != 0) *next_frame_id = head_->frame_id;"
        assert old in methods[key]
        methods[key] = methods[key].replace(old, "*next_frame_id = head_->frame_id;", 1)
    elif mutation == "set_limit_does_not_clear":
        key = "void VideoFrameQueue::set_limit(int limit)"
        # Remove the cleanup effects, independent of helper versus local-loop spelling.
        assert "head_" in methods[key] and "limit_ = limit;" in methods[key]
        methods[key] = "void VideoFrameQueue::set_limit(int limit) { limit_ = limit; }"
    elif mutation == "non_equality_limit_gate":
        for key in ("int VideoFrameQueue::allocate(", "int VideoFrameQueue::append("):
            old = "if (count_ == limit_) return 0x14;"
            assert old in methods[key]
            methods[key] = methods[key].replace(old, "if (count_ > limit_) return 0x14;", 1)
    elif mutation is not None:
        raise AssertionError(f"unknown mutation {mutation!r}")

    definitions = "\n".join(methods[key] for key, _signature in DEFINITIONS)
    return r'''#include <cassert>
#include <cstddef>
#include <cstring>

/*ACTUAL_NODE_DECLARATION*/
/*ACTUAL_QUEUE_DECLARATION*/

static unsigned char zero_byte_residue;
static VideoFrameNode node_pool[32];
static unsigned char payload_pool[32][64];
static int node_used, payload_used, fail_node, fail_payload;
static int node_frees, payload_frees, residue_frees;
static VideoFrameNode *last_node;

static void reset_allocator() {
  node_used = payload_used = fail_node = fail_payload = 0;
  node_frees = payload_frees = residue_frees = 0;
  last_node = 0;
}

static void *mem_get(int size) {
  if (size == static_cast<int>(sizeof(VideoFrameNode))) {
    if (fail_node) { fail_node = 0; return 0; }
    if (node_used == 32) return 0;
    VideoFrameNode *node = &node_pool[node_used++];
    node->frame_id = -1;
    node->frame_flags = 0xA5A5A5A5U;
    // A valid, non-null allocator residue lets the zero-byte path be tested
    // without evaluating an indeterminate pointer representation.
    node->bytes = &zero_byte_residue;
    node->byte_count = 0xA5A5A5A5U;
    node->next = 0;
    last_node = node;
    return node;
  }
  if (fail_payload) { fail_payload = 0; return 0; }
  if (size <= 0 || size > 64 || payload_used == 32) return 0;
  unsigned char *payload = payload_pool[payload_used++];
  memset(payload, 0xCC, sizeof payload_pool[0]);
  return payload;
}

static void queue_test_free(void *pointer) {
  if (!pointer) return;
  if (pointer == &zero_byte_residue) { ++residue_frees; return; }
  for (int index = 0; index < node_used; ++index) {
    if (pointer == &node_pool[index]) { ++node_frees; return; }
  }
  for (int index = 0; index < payload_used; ++index) {
    if (pointer == payload_pool[index]) { ++payload_frees; return; }
  }
  assert(!"queue attempted to free a pointer outside the fixture allocator");
}

#define free queue_test_free
/*ACTUAL_QUEUE_METHODS*/
#undef free

static void fifo_peek_discard_and_clear() {
  reset_allocator();
  VideoFrameQueue queue;
  int id = 0x12345678;
  unsigned int flags = 0x87654321U;
  assert(queue.peek(&id, &flags) == 0);
  assert(id == 0x12345678 && flags == 0x87654321U);
  assert(queue.peek(0, &flags) == 0 && flags == 0x87654321U);
  assert(queue.peek(&id, 0) == 0 && id == 0x12345678);

  const unsigned char first[] = {1, 2, 3};
  const unsigned char second[] = {7, 8};
  assert(queue.append(first, 3, 41, 0x11U) == 0);
  assert(queue.append(second, 2, 42, 0x22U) == 0);
  assert(queue.count_ == 2 && node_used == 2 && payload_used == 2);
  unsigned char *front = queue.peek(&id, &flags);
  assert(front == payload_pool[0] && id == 41 && flags == 0x11U);
  assert(front[0] == 1 && front[1] == 2 && front[2] == 3);
  assert(queue.peek(0, &flags) == front && flags == 0x11U);
  assert(queue.peek(&id, 0) == front && id == 41);

  queue.discard_head();
  assert(queue.count_ == 1 && queue.head_ == &node_pool[1]);
  unsigned char *next = queue.peek(&id, &flags);
  assert(next == payload_pool[1] && id == 42 && flags == 0x22U);
  assert(next[0] == 7 && next[1] == 8);
  queue.set_limit(0);
  assert(queue.count_ == 0 && queue.head_ == 0 && queue.tail_ == 0 && queue.limit_ == 0);
  assert(node_frees == 2 && payload_frees == 2);
  const int nodes_before_limit = node_used;
  unsigned char *out = &zero_byte_residue;
  assert(queue.append(second, 2, 43, 0) == 0x14);
  assert(queue.allocate(&out, 2, 43, 0) == 0x14 && out == 0);
  assert(node_used == nodes_before_limit);
  queue.discard_head();
  assert(queue.count_ == 0 && queue.head_ == 0);
}

static void allocate_and_uninitialized_zero_byte_fields() {
  reset_allocator();
  VideoFrameQueue queue;
  unsigned char *out = &zero_byte_residue;
  assert(queue.allocate(0, 2, 7, 9) == 3);
  assert(out == &zero_byte_residue && queue.count_ == 0);
  assert(queue.allocate(&out, 3, 51, 0x55U) == 0);
  assert(out == payload_pool[0] && queue.count_ == 1);
  int id = 0;
  unsigned int flags = 0;
  assert(queue.peek(&id, &flags) == payload_pool[0]);
  assert(id == 51 && flags == 0x55U);
  queue.clear();

  reset_allocator();
  VideoFrameQueue zero_append;
  const unsigned char ignored[] = {0xEE};
  assert(zero_append.append(ignored, 0, 61, 0x66U) == 0);
  assert(zero_append.head_->bytes == &zero_byte_residue);
  assert(zero_append.head_->byte_count == 0xA5A5A5A5U);
  zero_append.clear();
  assert(residue_frees == 1);

  reset_allocator();
  VideoFrameQueue zero_allocate;
  out = 0;
  assert(zero_allocate.allocate(&out, 0, 62, 0x67U) == 0);
  assert(out == &zero_byte_residue);
  assert(zero_allocate.head_->byte_count == 0xA5A5A5A5U);
  zero_allocate.clear();
  assert(residue_frees == 1);

  reset_allocator();
  VideoFrameQueue null_source;
  assert(null_source.append(0, 3, 63, 0x68U) == 0);
  assert(payload_pool[0][0] == 0xCC && payload_pool[0][1] == 0xCC &&
         payload_pool[0][2] == 0xCC);
  null_source.clear();
}

static void preserve_original_allocation_failure_state() {
  reset_allocator();
  VideoFrameQueue node_failure;
  fail_node = 1;
  assert(node_failure.append(0, 0, 1, 0) == 4);
  assert(node_failure.head_ == 0 && node_failure.tail_ == 0 && node_failure.count_ == 0);
  unsigned char *out = &zero_byte_residue;
  fail_node = 1;
  assert(node_failure.allocate(&out, 0, 1, 0) == 4);
  assert(out == 0 && node_failure.head_ == 0 && node_failure.count_ == 0);

  reset_allocator();
  VideoFrameQueue queued_node_failure;
  assert(queued_node_failure.append(0, 0, 2, 0) == 0);
  VideoFrameNode *old_tail = queued_node_failure.tail_;
  fail_node = 1;
  assert(queued_node_failure.append(0, 0, 3, 0) == 4);
  assert(queued_node_failure.head_ == old_tail && queued_node_failure.tail_ == old_tail);
  assert(old_tail->next == 0 && queued_node_failure.count_ == 1);
  unsigned char *unchanged_out = &zero_byte_residue;
  fail_node = 1;
  assert(queued_node_failure.allocate(&unchanged_out, 0, 4, 0) == 4);
  assert(unchanged_out == 0 && queued_node_failure.head_ == old_tail &&
         queued_node_failure.tail_ == old_tail && old_tail->next == 0 &&
         queued_node_failure.count_ == 1);
  queued_node_failure.clear();

  reset_allocator();
  VideoFrameQueue empty_payload_failure;
  fail_payload = 1;
  assert(empty_payload_failure.append(0, 5, 2, 0) == 4);
  assert(empty_payload_failure.head_ == 0);
  assert(empty_payload_failure.tail_ == last_node); // original leaves stale tail
  assert(empty_payload_failure.count_ == -1);       // decremented before increment
  empty_payload_failure.clear();
  assert(empty_payload_failure.tail_ == 0 && empty_payload_failure.count_ == 0);

  reset_allocator();
  VideoFrameQueue queued_payload_failure;
  assert(queued_payload_failure.append(0, 0, 3, 0) == 0);
  VideoFrameNode *new_tail = 0;
  fail_payload = 1;
  assert(queued_payload_failure.append(0, 5, 4, 0) == 4);
  new_tail = last_node;
  // Failure discards the old head, not the newly linked node, leaving one
  // linked node with the pre-increment count decremented to zero.
  assert(queued_payload_failure.head_ == new_tail);
  assert(queued_payload_failure.tail_ == new_tail);
  assert(queued_payload_failure.count_ == 0 && new_tail->frame_id == 4);
  assert(new_tail->bytes == 0 && new_tail->next == 0);
  queued_payload_failure.clear();

  reset_allocator();
  VideoFrameQueue allocate_payload_failure;
  out = &zero_byte_residue;
  fail_payload = 1;
  assert(allocate_payload_failure.allocate(&out, 5, 5, 0) == 4);
  assert(out == 0 && allocate_payload_failure.head_ == 0);
  assert(allocate_payload_failure.count_ == -1);
  allocate_payload_failure.clear();
}

int main() {
  fifo_peek_discard_and_clear();
  allocate_and_uninitialized_zero_byte_fields();
  preserve_original_allocation_failure_state();
  return 0;
}
'''.replace("/*ACTUAL_NODE_DECLARATION*/", node_decl).replace(
        "/*ACTUAL_QUEUE_DECLARATION*/", queue_decl
    ).replace("/*ACTUAL_QUEUE_METHODS*/", definitions)


def _compile_and_run(tmp_path: pathlib.Path, compiler: str, source: str, name: str):
    source_path = tmp_path / f"{name}.cpp"
    binary_path = tmp_path / name
    source_path.write_text(source, encoding="utf-8")
    compiled = subprocess.run(
        [compiler, "-std=c++11", "-O0", str(source_path), "-o", str(binary_path)],
        capture_output=True,
        text=True,
    )
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(binary_path)], capture_output=True, text=True)


def _compiler() -> str:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    return compiler


def test_all_seven_original_queue_mappings_are_extracted():
    cpp = QUEUE_CPP.read_text(encoding="utf-8")
    for address, span in PINNED_SPANS:
        assert f"// ORIGINAL: {address} sub_" in cpp
        assert span in cpp
    for key, signature in DEFINITIONS:
        assert key in _source_fixture()
        assert len(re.findall(re.escape(signature), cpp)) == 1


def test_actual_video_queue_methods_preserve_original_effects(tmp_path):
    completed = _compile_and_run(tmp_path, _compiler(), _source_fixture(), "video_queue_faithful")
    assert completed.returncode == 0, completed.stderr


@pytest.mark.parametrize(
    "mutation",
    (
        "skip_failure_cleanup",
        "initialize_zero_byte_fields",
        "peek_ignores_optional_output",
        "set_limit_does_not_clear",
        "non_equality_limit_gate",
    ),
)
def test_actual_video_queue_controls_reject_behavior_damage(tmp_path, mutation):
    completed = _compile_and_run(
        tmp_path, _compiler(), _source_fixture(mutation), f"video_queue_{mutation}"
    )
    assert completed.returncode != 0, mutation
