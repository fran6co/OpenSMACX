"""Host behavior controls extracted from the admitted NetFifo owner.

These checks are deliberately not target-ABI, native-match, or game proofs.
"""
from __future__ import annotations

from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/net_fifo.h"
SOURCE = ROOT / "src/net_fifo.cpp"


def _class_declaration(text: str) -> str:
    start = text.find("class NetFifo {")
    assert start >= 0, "worker NetFifo class declaration missing"
    brace = text.find("{", start)
    depth = 0
    for end in range(brace, len(text)):
        if text[end] == "{":
            depth += 1
        elif text[end] == "}":
            depth -= 1
            if depth == 0:
                semi = text.find(";", end)
                assert semi >= 0
                return text[start:semi + 1]
    raise AssertionError("unterminated NetFifo declaration")


def _extract_function(source: str, identity: str) -> str:
    match = re.search(r"(?m)^[^\n]*\b" + re.escape(identity) + r"\s*\([^;]*?\)\s*\{", source)
    assert match, f"worker definition missing: {identity}"
    brace = source.find("{", match.start())
    depth = 0
    for end in range(brace, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[match.start():end + 1]
    raise AssertionError(f"unterminated worker definition: {identity}")


BASE_REVISION = "7a20029edfe4fb109e048bdcdce23f0e44cc0117"


PREFIX = r'''#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <vector>

struct CRITICAL_SECTION { int depth; };
class NetFifo;
static NetFifo *active_queue;
static std::vector<CRITICAL_SECTION *> live_locks;
static std::vector<void *> nodes;
static std::vector<void *> payloads;
static std::vector<void *> freed_nodes;
static std::vector<void *> freed_payloads;
static std::vector<char> events;

void tracked_free(void *p);
void InitializeCriticalSection(CRITICAL_SECTION *lock) {
  assert(lock != 0);
  for (std::vector<CRITICAL_SECTION *>::iterator i = live_locks.begin(); i != live_locks.end(); ++i)
    assert(*i != lock);
  lock->depth = 0;
  live_locks.push_back(lock);
  events.push_back('I');
}
void EnterCriticalSection(CRITICAL_SECTION *lock) {
  bool found = false;
  for (std::vector<CRITICAL_SECTION *>::iterator i = live_locks.begin(); i != live_locks.end(); ++i)
    if (*i == lock) found = true;
  assert(found && lock->depth == 0);
  lock->depth = 1;
  events.push_back('E');
}
void LeaveCriticalSection(CRITICAL_SECTION *lock);
void DeleteCriticalSection(CRITICAL_SECTION *lock) {
  assert(lock != 0 && lock->depth == 0);
  for (std::vector<CRITICAL_SECTION *>::iterator i = live_locks.begin(); i != live_locks.end(); ++i) {
    if (*i == lock) {
      live_locks.erase(i);
      events.push_back('D');
      return;
    }
  }
  assert(false);
}
#define free tracked_free
'''


LOCK_OBSERVER = r'''
void LeaveCriticalSection(CRITICAL_SECTION *lock) {
  assert(lock != 0 && lock->depth == 1 && active_queue != 0);
  assert(active_queue->head_ == 0 && active_queue->tail_ == 0 && active_queue->count_ == 0);
  lock->depth = 0;
  events.push_back('L');
}
'''


MAIN = r'''
void *allocate_record(std::vector<void *> &registry, unsigned int bytes) {
  void *p = malloc(bytes);
  assert(p != 0);
  registry.push_back(p);
  return p;
}
NetFifo::NetFifoNode *make_node(NetFifo::NetFifoNode *next, void *data) {
  NetFifo::NetFifoNode *node = static_cast<NetFifo::NetFifoNode *>(allocate_record(nodes, sizeof(NetFifo::NetFifoNode)));
  node->caller_value_ = 17;
  node->key_ = 23;
  node->result_ = 31;
  node->data_ = data;
  node->data_size_ = data == 0 ? 0 : 4;
  node->next_ = next;
  return node;
}
void tracked_free(void *p) {
  assert(p != 0 && active_queue != 0);
  for (std::vector<void *>::iterator i = payloads.begin(); i != payloads.end(); ++i) {
    if (*i == p) {
      for (std::vector<void *>::iterator j = freed_payloads.begin(); j != freed_payloads.end(); ++j) assert(*j != p);
      assert(active_queue->head_ != 0 && active_queue->head_->data_ == p);
      freed_payloads.push_back(p);
      events.push_back('P');
      ::free(p);
      return;
    }
  }
  for (std::vector<void *>::iterator i = nodes.begin(); i != nodes.end(); ++i) {
    if (*i == p) {
      for (std::vector<void *>::iterator j = freed_nodes.begin(); j != freed_nodes.end(); ++j) assert(*j != p);
      assert(active_queue->head_ == p);
      assert(active_queue->head_->data_ == 0);
      freed_nodes.push_back(p);
      events.push_back('N');
      ::free(p);
      return;
    }
  }
  assert(false);
}
void clear_observations() {
  nodes.clear(); payloads.clear(); freed_nodes.clear(); freed_payloads.clear(); events.clear();
  live_locks.clear();
}
void assert_events(const char *wanted) {
  assert(events.size() == __builtin_strlen(wanted));
  for (unsigned int i = 0; i < events.size(); ++i) assert(events[i] == wanted[i]);
}

int main() {
  clear_observations();
  {
    NetFifo q;
    active_queue = &q;
    q.close();
    assert(q.head_ == 0 && q.tail_ == 0 && q.count_ == 0);
    assert(live_locks.size() == 1 && q.lock_.depth == 0);
    q.close();
    assert(q.head_ == 0 && q.tail_ == 0 && q.count_ == 0);
  }
  active_queue = 0;
  assert_events("IELELELD");
  assert(freed_nodes.empty() && freed_payloads.empty() && live_locks.empty());

  clear_observations();
  {
    NetFifo q;
    active_queue = &q;
    void *pa = allocate_record(payloads, 4);
    void *pc = allocate_record(payloads, 4);
    NetFifo::NetFifoNode *c = make_node(0, pc);
    NetFifo::NetFifoNode *b = make_node(c, 0);
    NetFifo::NetFifoNode *a = make_node(b, pa);
    q.head_ = a; q.tail_ = c; q.count_ = 3;
  }
  active_queue = 0;
  assert_events("IEPNNPNLD");
  assert(freed_nodes.size() == 3 && freed_payloads.size() == 2 && live_locks.empty());

  clear_observations();
  {
    NetFifo q;
    active_queue = &q;
    void *pa = allocate_record(payloads, 4);
    NetFifo::NetFifoNode *b = make_node(0, 0);
    NetFifo::NetFifoNode *a = make_node(b, pa);
    q.head_ = a; q.tail_ = b; q.count_ = 2;
    q.close();
    assert(q.head_ == 0 && q.tail_ == 0 && q.count_ == 0);
    assert(live_locks.size() == 1 && q.lock_.depth == 0);
  }
  active_queue = 0;
  assert_events("IEPNNLELD");
  assert(freed_nodes.size() == 2 && freed_payloads.size() == 1 && live_locks.empty());
  return 0;
}
'''


def _source_forms() -> dict[str, tuple[str, str]]:
    header = HEADER.read_text()
    source = SOURCE.read_text()
    declaration = _class_declaration(header)
    ctor = _extract_function(source, "NetFifo::NetFifo")
    dtor = _extract_function(source, "NetFifo::~NetFifo")
    close = _extract_function(source, "NetFifo::close")
    assert "drain_queue" not in declaration and "drain_queue" not in source
    assert "close();" not in dtor and "DeleteCriticalSection(&lock_);" in dtor
    assert "EnterCriticalSection(&lock_);" in dtor and "while (head_ != 0)" in dtor
    assert "tail_ = 0;" in dtor and "count_ = 0;" in dtor
    assert dtor.find("LeaveCriticalSection(&lock_);") < dtor.find("DeleteCriticalSection(&lock_);")
    assert "EnterCriticalSection(&lock_);" in close and "while (head_ != 0)" in close
    assert "tail_ = 0;" in close and "count_ = 0;" in close
    assert "LeaveCriticalSection(&lock_);" in close

    current = (declaration, "\n".join((dtor, ctor, close)))

    def baseline(path: str) -> str:
        result = subprocess.run(["git", "show", BASE_REVISION + ":" + path], cwd=ROOT,
                                check=True, text=True, stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT)
        return result.stdout

    old_header = baseline("src/net_fifo.h")
    old_source = baseline("src/net_fifo.cpp")
    old_decl = _class_declaration(old_header)
    old_ctor = _extract_function(old_source, "NetFifo::NetFifo")
    old_dtor = _extract_function(old_source, "NetFifo::~NetFifo")
    old_close = _extract_function(old_source, "NetFifo::close")
    assert "close();" in old_dtor and "DeleteCriticalSection(&lock_);" in old_dtor
    assert "while (head_ != 0)" in old_close and "tail_ = 0;" in old_close
    former = (old_decl, "\n".join((old_ctor, old_dtor, old_close)))
    return {"current-direct-dtor": current, "former-close": former}


def _program(declaration: str, methods: str) -> str:
    return PREFIX + declaration + "\n" + LOCK_OBSERVER + methods + "\n#undef free\n" + MAIN


def _compile_run(compiler: str, source: str, directory: Path, label: str) -> subprocess.CompletedProcess[str]:
    cpp = directory / (label + ".cpp")
    exe = directory / label
    cpp.write_text(source)
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", str(cpp), "-o", str(exe)],
                   check=True, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return subprocess.run([str(exe)], check=False, text=True, stdout=subprocess.PIPE,
                          stderr=subprocess.STDOUT)


def test_actual_worker_destructor_and_former_close_behavior_and_mutants() -> None:
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    if not compilers:
        pytest.skip("neither GCC nor Clang is available for host controls")
    forms = _source_forms()
    current_decl, current_methods = forms["current-direct-dtor"]
    mutants: dict[str, tuple[str, str]] = {}

    dtor = _extract_function(current_methods, "NetFifo::~NetFifo")
    no_tail_dtor = dtor.replace("  tail_ = 0;\n", "", 1)
    assert no_tail_dtor != dtor
    mutants["missing-dtor-tail-reset"] = (current_decl, current_methods.replace(dtor, no_tail_dtor, 1))

    close = _extract_function(current_methods, "NetFifo::close")
    no_tail_close = close.replace("  tail_ = 0;\n", "", 1)
    assert no_tail_close != close
    mutants["missing-close-tail-reset"] = (current_decl, current_methods.replace(close, no_tail_close, 1))

    reordered = current_methods.replace(
        "  LeaveCriticalSection(&lock_);\n    DeleteCriticalSection(&lock_);",
        "    DeleteCriticalSection(&lock_);\n  LeaveCriticalSection(&lock_);", 1)
    assert reordered != current_methods
    mutants["delete-before-unlock"] = (current_decl, reordered)

    reinitialized = current_methods.replace("DeleteCriticalSection(&lock_);",
                                             "InitializeCriticalSection(&lock_);", 1)
    assert reinitialized != current_methods
    mutants["delete-to-initialize"] = (current_decl, reinitialized)

    with tempfile.TemporaryDirectory(prefix="osmx-netfifo-destructor-source-") as temp:
        directory = Path(temp)
        for compiler in compilers:
            for label, (declaration, methods) in forms.items():
                result = _compile_run(compiler, _program(declaration, methods), directory,
                                      compiler + "-" + label)
                assert result.returncode == 0, f"{compiler} {label} failed:\n{result.stdout}"
            for label, (declaration, methods) in mutants.items():
                result = _compile_run(compiler, _program(declaration, methods), directory,
                                      compiler + "-" + label)
                assert result.returncode != 0, f"{compiler} missed {label}"

