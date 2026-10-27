"""Host controls compile the actual WaveControlGroup lifecycle bodies.

They exercise the owning type's initialize/drain behavior only; they do not
prove target ABI layout, compiler array-EH emission, runtime-provider identity,
or byte equivalence.
"""
from __future__ import annotations

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def extract_function(source: str, signature: str) -> str:
    assert source.count(signature) == 1, signature
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for index in range(opening, len(source)):
        depth += (source[index] == "{") - (source[index] == "}")
        if depth == 0:
            return source[start:index + 1]
    raise AssertionError("unterminated " + signature)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_wave_control_group_actual_member_lifetime(compiler_name: str, tmp_path: Path):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} is unavailable")
    header = (ROOT / "src/wave_device.h").read_text(encoding="utf-8")
    declarations = header[header.index("struct WaveGroupNode {"):
                         header.index(" /*\n  * Wave_Device class")]
    owner = (ROOT / "src/sounddevice.cpp").read_text(encoding="utf-8")
    ctor = extract_function(owner, "WaveControlGroup::WaveControlGroup()")
    dtor = extract_function(owner, "WaveControlGroup::~WaveControlGroup()")
    harness = tmp_path / "wave_group.cpp"
    binary = tmp_path / "wave_group"
    harness.write_text(r"""
#include <cassert>
#include <cstdlib>
#include <stdint.h>
#define __fastcall
#define nullptr 0
static void *freed[8];
static unsigned int freed_count;
void operator delete(void *p) throw() {
    if (p) freed[freed_count++] = p;
    std::free(p);
}
struct Wave {};
""" + declarations + "\n" + ctor + "\n" + dtor + r"""
static WaveGroupNode *node(Wave *wave, WaveGroupNode *prev, WaveGroupNode *next) {
    WaveGroupNode *value = static_cast<WaveGroupNode *>(std::malloc(sizeof(WaveGroupNode)));
    assert(value != 0);
    value->prev = prev; value->next = next; value->wave = wave;
    return value;
}
    int main() {
        Wave a, b;
        WaveGroupNode cursor;
        WaveGroupNode *n1, *n2, *n3;
        {
            WaveControlGroup group;
            assert(group.list_.head == 0 && group.list_.tail == 0 && group.list_.cursor == 0);
            assert(group.list_.count == 0 && group.volume == 0x7f && group.enabled == 1);
            n3 = node(&b, 0, 0);
            n2 = node(&b, 0, n3);
            n1 = node(&a, 0, n2);
        n2->prev = n1; n3->prev = n2;
        group.list_.head = n1; group.list_.tail = n3; group.list_.cursor = &cursor; group.list_.count = 3;
        }
        assert(freed_count == 3);
        assert(freed[0] == n1 && freed[1] == n2 && freed[2] == n3);

    freed_count = 0;
    WaveGroupList list = {0, 0, &cursor, 0};
    assert(list.remove_head() == 0);
    assert(!list.head && !list.tail && list.cursor == &cursor && list.count == 0);
    WaveGroupNode *pop_tail = node(&b, 0, 0);
    WaveGroupNode *pop_empty = node(0, 0, pop_tail);
    WaveGroupNode *pop_head = node(&a, 0, pop_empty);
    pop_empty->prev = pop_head; pop_tail->prev = pop_empty;
    list.head = pop_head; list.tail = pop_tail; list.count = 3;
    assert(list.remove_head() == &a);
    assert(list.head == pop_empty && list.tail == pop_tail && pop_empty->prev == 0);
    assert(list.cursor == &cursor && list.count == 2);
    assert(list.remove_head() == 0);
    assert(list.head == pop_tail && list.tail == pop_tail && pop_tail->prev == 0);
    assert(list.cursor == &cursor && list.count == 1);
    assert(list.remove_head() == &b);
    assert(!list.head && !list.tail && list.cursor == &cursor && list.count == 0);
    assert(list.remove_head() == 0 && freed_count == 3);
    assert(freed[0] == pop_head && freed[1] == pop_empty && freed[2] == pop_tail);

    freed_count = 0;
    WaveGroupNode *tail = node(&b, 0, 0);
    WaveGroupNode *empty = node(0, 0, tail);
    WaveGroupNode *head = node(&a, 0, empty);
    empty->prev = head; tail->prev = empty;
    {
        WaveControlGroup group;
        group.list_.head = head; group.list_.tail = tail; group.list_.count = 3;
        group.list_.cursor = &cursor;
    }
    // The historical null-wave break leaves the rest of the intrusive list
    // allocated; the host case releases that intentionally untouched suffix.
    assert(freed_count == 2);
    std::free(tail);
}
""", encoding="utf-8")
    built = subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                            str(harness), "-o", str(binary)],
                           capture_output=True, text=True, timeout=60)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(binary)], capture_output=True, text=True, timeout=30)
    assert ran.returncode == 0, ran.stdout + ran.stderr
