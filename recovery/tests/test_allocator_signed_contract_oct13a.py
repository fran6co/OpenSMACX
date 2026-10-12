"""Bounded controls for the signed allocator request and heap contracts.

The C++ projections use actual extracted method bodies and explicit allocator,
CRT and UI collaborators. They are not live-game or linked-preservation proof.
"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys
import tempfile

import pytest


ROOT = Path(__file__).resolve().parents[2]


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for end in range(opening, len(source)):
        if source[end] == "{":
            depth += 1
        elif source[end] == "}":
            depth -= 1
            if depth == 0:
                return source[start:end + 1]
    raise AssertionError("unterminated body: " + signature)


def source_parts():
    general_h = (ROOT / "src/general.h").read_text(encoding="utf-8")
    text_h = (ROOT / "src/text.h").read_text(encoding="utf-8")
    heap_h = (ROOT / "src/heap.h").read_text(encoding="utf-8")
    general_cpp = (ROOT / "src/general.cpp").read_text(encoding="utf-8")
    heap_cpp = (ROOT / "src/heap.cpp").read_text(encoding="utf-8")
    compat_h = (ROOT / "src/vc6_compat.h").read_text(encoding="utf-8")
    assert "#define nullptr 0" in compat_h
    assert re.search(r"LPVOID\s+__cdecl\s+mem_get\s*\(\s*int\s+size\s*\)\s*;", general_h)
    assert re.search(r"LPVOID\s+__cdecl\s+mem_get\s*\(\s*int\s+size\s*\)\s*;", text_h)
    assert re.search(r"\b(?:bool|BOOL)\s+init\s*\(\s*int\s+req_size\s*\)\s*;", heap_h)
    assert re.search(r"\bLPVOID\s+get\s*\(\s*int\s+req_size\s*\)\s*;", heap_h)
    init_match = re.search(r"(?m)^(?:bool|BOOL) Heap::init\(int req_size\)", heap_cpp)
    assert init_match, "signed Heap::init implementation is missing"
    for relative in ("general.h", "general.cpp", "text.h", "heap.h", "heap.cpp", "basebutton.cpp"):
        text = (ROOT / "src" / relative).read_text(encoding="utf-8")
        code = re.sub(r"(?s)/\*.*?\*/|//[^\n]*", "", text)
        assert "mem_get_old" not in code, relative
    return {
        "mem_get": body(general_cpp, "LPVOID __cdecl mem_get(int size)"),
        # The production VC6 compatibility header defines nullptr as literal 0;
        # use the equivalent C++98 NULL spelling in this isolated fixture.
        "init": body(heap_cpp, init_match.group(0)).replace("nullptr", "NULL"),
        "get": body(heap_cpp, "LPVOID Heap::get(int req_size)"),
    }


def test_signed_contract_declarations_and_canonical_basebutton_callers():
    source_parts()
    general_h = (ROOT / "src/general.h").read_text(encoding="utf-8")
    text_h = (ROOT / "src/text.h").read_text(encoding="utf-8")
    heap_h = (ROOT / "src/heap.h").read_text(encoding="utf-8")
    button = (ROOT / "src/basebutton.cpp").read_text(encoding="utf-8")
    assert "LPVOID __cdecl mem_get(int size);" in general_h
    assert "LPVOID __cdecl mem_get(int size);" in text_h
    assert re.search(r"\bBOOL\s+init\(int req_size\);", heap_h)
    assert "LPVOID get(int req_size);" in heap_h
    for signature in ("int BaseButton::set_bubble_text(", "int BaseButton::set_name(",
                      "int BaseButton::init("):
        method = body(button, signature)
        assert "mem_get(" in method
        assert "mem_get_old(" not in method
    assert "mem_get_old(" not in re.sub(r"(?s)/\*.*?\*/|//[^\n]*", "", button)


MEM_GET_PRELUDE = r'''#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <setjmp.h>
typedef void *LPVOID;
typedef int BOOL;
#define __cdecl
#define MB_ICONWARNING 0x30
static jmp_buf exit_target;
static int exit_code, allocation_fails, event_count;
static char events[16];
static size_t requested_bytes;
static char allocation[32];
static void record(char value) { assert(event_count < 16); events[event_count++] = value; }
static void *fixture_malloc(size_t size) {
  requested_bytes = size; record('M'); return allocation_fails ? 0 : allocation;
}
static int MessageBoxA(void *, const char *, const char *, unsigned int flags) {
  record('B'); assert(flags == MB_ICONWARNING); return 0;
}
static void fixture_exit(int code) {
  exit_code = code; record('E'); longjmp(exit_target, 1);
}
#define malloc fixture_malloc
#define exit fixture_exit
'''


MEM_GET_MAIN = r'''
int main() {
#ifdef _M_IX86
  assert(sizeof(size_t) == 4);
#endif
  allocation_fails = 0; event_count = 0;
  LPVOID value = mem_get(-1);
  assert(value == allocation);
  assert(requested_bytes == (size_t)-1);
  assert((unsigned int)requested_bytes == 0xffffffffu);
  assert(event_count == 1 && events[0] == 'M');
#ifdef _M_IX86
  // Text's size_t argument narrows to the pinned int parameter, then malloc's
  // size_t conversion preserves the complete 32-bit pattern on this ABI.
  size_t large_request = (size_t)0x80000001u;
  event_count = 0;
  assert(mem_get(large_request) == allocation);
  assert(requested_bytes == (size_t)0x80000001u);
  assert(event_count == 1 && events[0] == 'M');
#endif
  allocation_fails = 1; event_count = 0; exit_code = 0;
  if (setjmp(exit_target) == 0) {
    (void)mem_get(7);
    assert(0);
  }
  assert(exit_code == 4);
  assert(event_count == 3 && events[0] == 'M' && events[1] == 'B' && events[2] == 'E');
  return 0;
}
'''


HEAP_PRELUDE = r'''#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
typedef signed char int8_t;
typedef unsigned int uint32_t;
typedef void *LPVOID;
typedef int BOOL;
#define __cdecl
#define MB_OK 0
static char events[128]; static int event_count;
static char StringTemp[512];
static unsigned char arena[8192];
static int mem_get_null, realloc_fails, exit_code;
static size_t last_request, realloc_sizes[16]; static int realloc_count;
static void *last_free, *new_base;
static jmp_buf exit_target;
static void record(char c) { assert(event_count < 127); events[event_count++] = c; events[event_count] = 0; }
static void *mem_get(int size) {
  record('M'); last_request = (size_t)(unsigned int)size;
  return mem_get_null ? 0 : arena + 256;
}
static void fixture_free(void *p) { record('F'); last_free = p; }
static void *fixture_realloc(void *p, size_t size) {
  record('R'); assert(realloc_count < 16); realloc_sizes[realloc_count++] = size;
  assert(p != 0); return realloc_fails ? 0 : new_base;
}
static char *fixture_strcat(char *dst, const char *src) {
  record('C'); return strcat(dst, src);
}
static void fixture_add_lf(char *dst) { record('L'); strcat(dst, "\n"); }
static void fixture_say_num(int value) {
  char digits[32]; record('N'); sprintf(digits, "%d", value); strcat(StringTemp, digits);
}
static int MessageBoxA(void *, const char *message, const char *title, unsigned int flags) {
  record('B'); assert(strcmp(title, "Heap Notice!!") == 0); assert(flags == MB_OK);
  assert(strcmp(message, StringTemp) == 0); return 0;
}
static void fixture_exit(int code) { record('E'); exit_code = code; longjmp(exit_target, 1); }
struct Heap {
  int8_t err_flags_; LPVOID base_; LPVOID current_; size_t base_size_; size_t free_size_;
  BOOL init(int req_size); LPVOID get(int req_size);
};
#define free fixture_free
#define realloc fixture_realloc
#define strcat fixture_strcat
#define add_lf fixture_add_lf
#define say_num fixture_say_num
#define exit fixture_exit
'''


HEAP_MAIN = r'''
int main() {
  Heap h;
    // init's original BOOL result and reset/allocation order. A null result here
  // is a bounded collaborator contract control, not mem_get's fatal failure path.
  h.err_flags_ = 3; h.base_ = arena; h.current_ = arena + 10;
  h.base_size_ = 12; h.free_size_ = 5; mem_get_null = 0; event_count = 0;
  BOOL init_result = h.init(9);
  assert(init_result == 0 && last_free == arena && last_request == 9u);
  assert(h.base_ == arena + 256 && h.current_ == h.base_);
  assert(h.base_size_ == 9u && h.free_size_ == 9u && h.err_flags_ == 0);
  assert(event_count == 2 && events[0] == 'F' && events[1] == 'M');
  h.base_ = 0; h.current_ = 0; h.base_size_ = 0; h.free_size_ = 0;
  mem_get_null = 1; event_count = 0;
  init_result = h.init(13);
  assert(init_result == 1 && last_request == 13u);
  assert(event_count == 1 && events[0] == 'M');

  // A signed negative request does not grow; native int subtraction preserves
  // the source's historical pointer/free-space effects.
  h.err_flags_ = 0; h.base_ = arena; h.current_ = arena + 100;
  h.base_size_ = 1024; h.free_size_ = 5; new_base = arena + 2048;
  realloc_fails = 0; realloc_count = 0; event_count = 0;
  LPVOID old_current = h.current_;
  LPVOID returned = h.get(-1);
  assert(returned == old_current && realloc_count == 0);
  assert(h.current_ == (LPVOID)((size_t)old_current - 1u));
  assert(h.free_size_ == 6u && event_count == 0);

  // Borrowed storage has bit 0 set: insufficient signed space returns null
  // without attempting realloc. No allocation or guard normalization occurs.
  h.err_flags_ = 1; h.current_ = arena + 100; h.free_size_ = 4;
  event_count = 0; realloc_count = 0;
  assert(h.get(5) == 0);
  assert(realloc_count == 0 && event_count == 0 && h.current_ == arena + 100);

  // Growth retains the original current_ even when realloc supplies another
  // base address; then the request advances the old pointer by raw byte count.
  h.err_flags_ = 0; h.base_ = arena; h.current_ = arena + 100;
  h.base_size_ = 1024; h.free_size_ = 0; new_base = arena + 2048;
  realloc_fails = 0; realloc_count = 0; event_count = 0;
  old_current = h.current_; returned = h.get(2050);
  assert(returned == old_current && h.base_ == new_base);
  assert(realloc_count == 3 && realloc_sizes[0] == 2048 &&
         realloc_sizes[1] == 3072 && realloc_sizes[2] == 4096);
  assert(h.current_ == (LPVOID)((size_t)old_current + 2050u));
  assert(h.current_ != (LPVOID)((size_t)new_base + 2050u));
  assert(h.base_size_ == 4096 && h.free_size_ == 1022);

  // Realloc failure preserves formatting and API/exit ordering.
  h.err_flags_ = 0; h.base_ = arena; h.current_ = arena + 10;
  h.base_size_ = 10; h.free_size_ = 2; realloc_fails = 1;
  realloc_count = 0; event_count = 0; StringTemp[0] = 0; exit_code = 0;
  if (setjmp(exit_target) == 0) {
    (void)h.get(12);
    assert(0);
  }
  assert(exit_code == 3 && realloc_count == 1);
  assert(strcmp(events, "RCLCNLCNLCNBE") == 0);
  assert(strcmp(StringTemp,
      "Aborting due to a heap shortage!\nBase size: 10\nFree size: 2\nRequested size: 12") == 0);
  return 0;
}
'''


def compile_run(program: str, compiler: str):
    with tempfile.TemporaryDirectory(prefix="allocator-signed-contract-") as temp:
        source = Path(temp) / "control.cpp"
        exe = Path(temp) / "control"
        source.write_text(program, encoding="utf-8")
        built = subprocess.run(
            [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source), "-o", str(exe)],
            capture_output=True, text=True, timeout=45,
        )
        assert built.returncode == 0, built.stdout + built.stderr
        return subprocess.run([str(exe)], capture_output=True, text=True, timeout=15)


def heap_program(parts, init=None, get=None):
    return HEAP_PRELUDE + (init or parts["init"]) + "\n" + (get or parts["get"]) + HEAP_MAIN


def test_actual_allocator_bodies_and_negative_controls():
    parts = source_parts()
    compilers = [name for name in ("g++", "clang++") if shutil.which(name)]
    assert compilers
    mem_program = MEM_GET_PRELUDE + parts["mem_get"] + MEM_GET_MAIN
    for compiler in compilers:
        result = compile_run(mem_program, compiler)
        assert result.returncode == 0, compiler + ": " + result.stdout + result.stderr

        good = compile_run(heap_program(parts), compiler)
        assert good.returncode == 0, compiler + ": " + good.stdout + good.stderr

        signed_mutant = re.sub(
            r"static_cast<int>\(free_size_\)\s*<\s*req_size",
            "static_cast<unsigned int>(free_size_) < static_cast<unsigned int>(req_size)",
            parts["get"], count=1)
        assert signed_mutant != parts["get"]
        bad = compile_run(heap_program(parts, get=signed_mutant), compiler)
        assert bad.returncode != 0, "unsigned request comparison mutant survived"

        refreshed_current = parts["get"].replace(
            "LPVOID(size_t(current_) + req_size)", "LPVOID(size_t(base_) + req_size)", 1)
        assert refreshed_current != parts["get"]
        bad = compile_run(heap_program(parts, get=refreshed_current), compiler)
        assert bad.returncode != 0, "rebase-after-realloc mutant survived"

        wrong_result = parts["init"].replace("return false;", "return true;", 1)
        assert wrong_result != parts["init"]
        bad = compile_run(heap_program(parts, init=wrong_result), compiler)
        assert bad.returncode != 0, "inverted Heap::init success result survived"


def test_native_allocator_controls_under_existing_lease(tmp_path):
    if not os.environ.get("MATCH_RECOVERY_RESOURCE_LEASE"):
        pytest.skip("native controls require a coordinator compiler lease")
    from match_recovery.measurement import Measurement
    from match_recovery.processes import run_bounded
    from recovery.environment.compile import compile_object
    from recovery.environment.wine import VC6_ROOT, environment

    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required under the coordinator lease")
    parts = source_parts()
    settings = Measurement.read(ROOT)
    wrapper = ROOT / "recovery/environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")

    def build(name, flags, program):
        source = tmp_path / (name + ".cpp")
        obj = tmp_path / (name + ".obj")
        exe = tmp_path / (name + ".exe")
        source.write_text(program, encoding="utf-8")
        obj.write_bytes(compile_object(source, command, tuple(flags) + settings.layout_flags))
        linked = run_bounded(
            [sys.executable, str(wrapper), "--tool=link", "/nologo", "/subsystem:console",
             "/incremental:no", "/out:" + str(exe), str(obj), "kernel32.lib"],
            cwd=tmp_path, timeout=60,
        )
        assert linked.returncode == 0, (linked.stdout + linked.stderr).decode(errors="replace")
        return exe

    def run(exe):
        with tempfile.TemporaryFile() as output:
            result = subprocess.run(["wine32", str(exe)], env=environment(), stdout=output,
                                    stderr=output, timeout=30)
            output.seek(0)
            return result.returncode, output.read().decode(errors="replace")

    programs = [
        ("mem_get", MEM_GET_PRELUDE + parts["mem_get"] + MEM_GET_MAIN),
        ("heap", heap_program(parts)),
        ("heap-unsigned", heap_program(parts, get=re.sub(
            r"static_cast<int>\(free_size_\)\s*<\s*req_size",
            "static_cast<unsigned int>(free_size_) < static_cast<unsigned int>(req_size)",
            parts["get"], count=1))),
        ("heap-rebase", heap_program(parts, get=parts["get"].replace(
            "LPVOID(size_t(current_) + req_size)", "LPVOID(size_t(base_) + req_size)", 1))),
        ("heap-result", heap_program(parts, init=parts["init"].replace("return false;", "return true;", 1))),
    ]
    for profile, flags in enumerate(settings.flag_sets):
        for name, program in programs:
            code, output = run(build("allocator-%s-%d" % (name, profile), flags, program))
            if name == "mem_get" or name == "heap":
                assert code == 0, "%s profile %d: %s" % (name, profile, output)
            else:
                assert code != 0, "%s mutant survived profile %d: %s" % (name, profile, output)
