"""Host regression controls for the real NetFifo owner; not target ABI proof."""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/net_fifo.h"
SOURCE = ROOT / "src/net_fifo.cpp"


def _class_declaration(text: str) -> str:
    start = text.find("class NetFifo {")
    assert start >= 0, "actual NetFifo class declaration missing"
    brace = text.find("{", start)
    depth = 0
    for end in range(brace, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                semi = text.find(";", end)
                assert semi >= 0, "unterminated actual NetFifo class declaration"
                return text[start:semi + 1]
    raise AssertionError("unterminated actual NetFifo class declaration")


def _code_without_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = "\n".join(line for line in text.splitlines()
                       if not line.lstrip().startswith("//"))
    return "\n".join(line for line in text.splitlines()
                       if not line.lstrip().startswith("#include"))


FIXTURE_PREFIX = r'''#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct CRITICAL_SECTION { int depth; };
static int enter_calls, leave_calls, delete_calls, alloc_calls, fail_alloc_at;
static int frees;
void InitializeCriticalSection(CRITICAL_SECTION *lock) { lock->depth = 0; }
void DeleteCriticalSection(CRITICAL_SECTION *lock) { assert(lock && lock->depth == 0); ++delete_calls; }
void EnterCriticalSection(CRITICAL_SECTION *lock) { assert(lock); ++lock->depth; ++enter_calls; }
void LeaveCriticalSection(CRITICAL_SECTION *lock) { assert(lock && lock->depth == 1); --lock->depth; ++leave_calls; }
void *mem_get(int size) {
  ++alloc_calls;
  if (alloc_calls == fail_alloc_at) return 0;
  void *p = malloc(static_cast<size_t>(size));
  if (p) memset(p, 0xA5, static_cast<size_t>(size));
  return p;
}
void free_counted(void *p) { if (p) { ++frees; free(p); } }
#define free free_counted

'''


FIXTURE_MAIN = r'''
int main() {
  const char first[] = {'a','b'};
  const char second[] = {'c','d','e'};
  char out[8] = {};
  unsigned int size = 0;
  int caller = -1, signed_size = -1;
  unsigned int key = 0;

  {
    NetFifo q;
    assert(q.add_head(first, 2, 10, 7, 111) == 0);
    NetFifo::NetFifoNode *old_tail = q.tail_;
    assert(q.add_head(second, 3, 20, 8, 222) == 0);
    assert(q.head_->key_ == 20 && q.head_->next_ == old_tail);
    assert(q.tail_ == old_tail && q.count_ == 2);

    assert(q.peek_key_caller(out, 10, 7, &size) == 111);
    assert(size == 2 && memcmp(out, first, 2) == 0 && q.count_ == 2);
    assert(q.peek_caller(out, &key, 8, &size) == 222);
    assert(key == 20 && size == 3 && memcmp(out, second, 3) == 0);
    assert(q.peek_key(out, 20, &caller, &signed_size) == 222);
    assert(caller == 8 && signed_size == 3 && memcmp(out, second, 3) == 0);
    caller = -1; signed_size = -1; key = 0;
    assert(q.peek_head(out, &key, &caller, &signed_size) == 222);
    assert(key == 20 && caller == 8 && signed_size == 3);
    assert(q.peek_key(out, 99, &caller, &signed_size) == 0);

    assert(q.remove_key_caller(out, 10, 7, &size) == 111);
    assert(size == 2 && q.head_->key_ == 20 && q.head_->next_ == 0 && q.count_ == 1);
    // Removing the old tail intentionally leaves tail_ dangling.
    assert(q.tail_ == old_tail);
    key = 0; size = 0;
    assert(q.remove_caller(out, &key, 8, &size) == 222);
    assert(key == 20 && size == 3 && q.head_ == 0 && q.count_ == 0);
  }

  {
    NetFifo q;
    assert(q.add_head(first, 2, 31, 2, 301) == 0);
    assert(q.add_head(second, 3, 32, 3, 302) == 0);
    key = 31; size = 0; caller = -1;
    assert(q.get(out, &key, &caller, reinterpret_cast<int *>(&size)) == 301);
    assert(key == 31 && size == 2 && caller == 2 && memcmp(out, first, 2) == 0);
    key = 0; caller = -1; size = 0;
    assert(q.get(out, &key, &caller, &size) == 302);
    assert(key == 32 && size == 3 && caller == 3 && memcmp(out, second, 3) == 0);
    assert(q.head_ == 0 && q.count_ == 0);
  }

  {
    NetFifo broken;
    alloc_calls = 0; fail_alloc_at = 2;
    assert(broken.add_head(first, 2, 30, 9, 333) == 4);
    // Preserve the original partial linked state on payload allocation fail.
    assert(broken.head_ != 0 && broken.tail_ == broken.head_);
    assert(broken.head_->data_ == 0 && broken.count_ == 0);
  }

  {
    NetFifo zero_size;
    assert(zero_size.add_head(0, 0, 41, 4, 401) == 0);
    // The native body leaves data_ and data_size_ untouched for size zero.
    // Check deterministic node bytes, then clear only the indeterminate
    // pointer for safe host fixture cleanup; never assert a normalized value.
    assert(zero_size.head_->data_size_ == 0xA5A5A5A5u);
    zero_size.head_->data_ = 0;
  }

  {
    NetFifo queue;
    assert(queue.add_head(first, 2, 51, 5, 501) == 0);
    void *const payload = queue.head_->data_;
    const int before = frees;
    assert(queue.remove_caller(0, 0, 5, 0) == 501);
    // Original remove_caller leaves payload allocated without output storage.
    assert(frees == before + 1);
    free_counted(payload);
  }
  assert(enter_calls == leave_calls);
  assert(delete_calls == 5);
  assert(frees >= 9);
  return 0;
}
'''


def _compiled_source(owner_source: str) -> str:
    header = HEADER.read_text()
    declaration = _class_declaration(header)
    definitions = _code_without_comments(owner_source)
    required = (
        "NetFifo::NetFifo()", "NetFifo::~NetFifo()", "NetFifo::close()",
        "NetFifo::add_head(", "NetFifo::remove_key_caller(",
        "NetFifo::remove_caller(", "NetFifo::peek_key_caller(",
        "NetFifo::peek_caller(", "NetFifo::peek_key(",
        "NetFifo::peek_head(", "NetFifo::get(",
    )
    for identity in required:
        assert identity in definitions, f"actual owner definition missing: {identity}"
    return FIXTURE_PREFIX + declaration + "\n" + definitions + FIXTURE_MAIN


def _compile_and_run(compiler: str, source: str, directory: Path, label: str) -> subprocess.CompletedProcess[str]:
    cpp = directory / f"netfifo-{label}.cpp"
    exe = directory / f"netfifo-{label}-{compiler.replace('+', 'p')}"
    cpp.write_text(source)
    subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
        check=True,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )
    return subprocess.run([str(exe)], check=False, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.STDOUT)


def test_actual_net_fifo_source_and_mutant() -> None:
    owner_source = SOURCE.read_text()
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers, "neither GCC nor Clang is available for host controls"
    source = _compiled_source(owner_source)

    needle = "this->count_ += 1;"
    assert owner_source.count(needle) == 1, "mutant anchor must identify add_head only"
    mutant_owner = owner_source.replace(needle, "this->count_ += 2;", 1)
    mutant = _compiled_source(mutant_owner)

    with tempfile.TemporaryDirectory(prefix="osmx-netfifo-actual-source-") as temp:
        directory = Path(temp)
        for compiler in compilers:
            actual = _compile_and_run(compiler, source, directory, "actual")
            assert actual.returncode == 0, f"{compiler} actual source failed:\n{actual.stdout}"
            damaged = _compile_and_run(compiler, mutant, directory, "mutant")
            assert damaged.returncode != 0, f"{compiler} failed to detect the count mutant"
