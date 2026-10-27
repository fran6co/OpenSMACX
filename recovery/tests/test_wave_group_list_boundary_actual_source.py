from __future__ import annotations

import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest


def repository_root() -> Path:
    for candidate in Path(__file__).resolve().parents:
        if (candidate / "src").is_dir() and (candidate / "recovery").is_dir():
            return candidate
    raise AssertionError("could not find repository root")


def definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                return source[start : offset + 1]
    raise AssertionError(f"unterminated definition: {signature}")


def declaration(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for offset in range(opening, len(source)):
        if source[offset] == "{":
            depth += 1
        elif source[offset] == "}":
            depth -= 1
            if depth == 0:
                end = source.index(";", offset) + 1
                return source[start:end]
    raise AssertionError(f"unterminated declaration: {signature}")


def class_member_definition(full_definition: str, qualified_name: str, member_name: str) -> str:
    prefix = f"{qualified_name}::{member_name}"
    assert full_definition.count(prefix) == 1
    return full_definition.replace(prefix, member_name, 1)


def source_parts(root: Path) -> dict[str, str]:
    header = (root / "src/wave_device.h").read_text()
    device = (root / "src/sounddevice.cpp").read_text()
    helper = (root / "src/wave_group_list.cpp").read_text()
    parts = {
        "node_type": declaration(header, "struct WaveGroupNode {"),
        "list_type": declaration(header, "struct WaveGroupList {"),
        "insert": definition(helper, "void WaveGroupList::insert(Wave *a1)"),
        "add_to_group": definition(device, "int Wave_Device::add_to_group(unsigned int group, Wave *wave)"),
    }
    assert "WaveGroupNode *prev;" in parts["node_type"]
    assert "WaveGroupNode *next;" in parts["node_type"]
    assert "Wave *wave;" in parts["node_type"]
    for member in ("WaveGroupNode *head;", "WaveGroupNode *tail;",
                   "WaveGroupNode *cursor;", "uint32_t count;"):
        assert member in parts["list_type"]
    assert "operator new(0xC)" in parts["insert"]
    assert "if (group > 0xF || !wave)" in parts["add_to_group"]
    assert "groups_[group].list_.insert(wave);" in parts["add_to_group"]
    assert parts["add_to_group"].index("groups_[group].list_.insert(wave);") < parts["add_to_group"].index("wave->group_slot_ = group;")
    assert "wave_group_insert_redirect" not in helper
    return parts


def harness(parts: dict[str, str]) -> str:
    return f"""
#include <cstddef>
#include <cstdint>
#include <new>

struct Wave {{ uint32_t group_slot_; }};
{parts['node_type']}
{parts['list_type']}

struct WaveControlGroupForTest {{ WaveGroupList list_; }};
class Wave_Device {{
public:
    WaveControlGroupForTest groups_[16];
    {class_member_definition(parts['add_to_group'], 'Wave_Device', 'add_to_group')}
}};

static WaveGroupNode allocation_pool[8];
static int allocation_count;
static std::size_t last_allocation_size;
static int fail_allocation;
static uint32_t *watched_group_slot;
static uint32_t allocation_saw_group_slot;

void *operator new(std::size_t size) {{
    last_allocation_size = size;
    if (watched_group_slot != 0) allocation_saw_group_slot = *watched_group_slot;
    if (fail_allocation) return 0;
    if (allocation_count >= 8) return 0;
    return &allocation_pool[allocation_count++];
}}
void operator delete(void *) noexcept {{}}

{parts['insert']}

static void reset_allocator() {{
    allocation_count = 0;
    last_allocation_size = 0;
    fail_allocation = 0;
    watched_group_slot = 0;
    allocation_saw_group_slot = 0;
    for (int i = 0; i < 8; ++i) {{
        allocation_pool[i].prev = 0;
        allocation_pool[i].next = 0;
        allocation_pool[i].wave = 0;
    }}
}}

static int test_empty_list_append_and_post_call_slot_store() {{
    reset_allocator();
    Wave_Device device = {{}};
    Wave wave = {{0xDEADBEEFU}};
    WaveGroupList &list = device.groups_[3].list_;
    list.head = 0; list.tail = 0; list.cursor = 0; list.count = 0;
    watched_group_slot = &wave.group_slot_;
    allocation_saw_group_slot = wave.group_slot_;
    if (device.add_to_group(3, &wave) != 0) return 1;
    if (allocation_count != 1 || last_allocation_size != 0xC) return 2;
    if (allocation_saw_group_slot != 0xDEADBEEFU) return 3;
    if (list.head != &allocation_pool[0] || list.tail != &allocation_pool[0]) return 4;
    if (list.cursor != 0 || list.count != 1) return 5;
    if (allocation_pool[0].prev != 0 || allocation_pool[0].next != 0 ||
        allocation_pool[0].wave != &wave) return 6;
    if (wave.group_slot_ != 3) return 7;
    return 0;
}}

static int test_nonempty_tail_append_preserves_cursor() {{
    reset_allocator();
    Wave_Device device = {{}};
    Wave wave = {{0xABCDEF01U}};
    Wave old_wave = {{5}};
    WaveGroupNode old_tail = {{0, 0, &old_wave}};
    WaveGroupNode cursor = {{0, 0, 0}};
    WaveGroupList &list = device.groups_[5].list_;
    list.head = &old_tail; list.tail = &old_tail; list.cursor = &cursor; list.count = 1;
    watched_group_slot = &wave.group_slot_;
    allocation_saw_group_slot = wave.group_slot_;
    if (device.add_to_group(5, &wave) != 0) return 11;
    if (allocation_count != 1 || last_allocation_size != 0xC) return 12;
    if (allocation_saw_group_slot != 0xABCDEF01U) return 13;
    if (list.head != &old_tail || old_tail.next != &allocation_pool[0]) return 14;
    if (list.tail != &allocation_pool[0] || allocation_pool[0].prev != &old_tail ||
        allocation_pool[0].next != 0 || allocation_pool[0].wave != &wave) return 15;
    if (list.cursor != &cursor || list.count != 2 || wave.group_slot_ != 5) return 16;
    return 0;
}}

static int test_empty_list_allocation_failure_keeps_original_effects() {{
    reset_allocator();
    Wave_Device device = {{}};
    Wave wave = {{0x12345678U}};
    WaveGroupList &list = device.groups_[0].list_;
    list.head = 0; list.tail = 0; list.cursor = 0; list.count = 0;
    fail_allocation = 1;
    watched_group_slot = &wave.group_slot_;
    allocation_saw_group_slot = wave.group_slot_;
    if (device.add_to_group(0, &wave) != 0) return 21;
    if (allocation_count != 0 || last_allocation_size != 0xC) return 22;
    if (allocation_saw_group_slot != 0x12345678U) return 23;
    if (list.head != 0 || list.tail != 0 || list.cursor != 0 || list.count != 1) return 24;
    if (wave.group_slot_ != 0) return 25;
    return 0;
}}

static int test_invalid_inputs_do_not_allocate_or_mutate() {{
    reset_allocator();
    Wave_Device device = {{}};
    Wave wave = {{0x76543210U}};
    if (device.add_to_group(16U, &wave) != 0xA) return 31;
    if (device.add_to_group(0xFFFFFFFFU, &wave) != 0xA) return 32;
    if (device.add_to_group(2U, 0) != 0xA) return 33;
    if (allocation_count != 0 || last_allocation_size != 0) return 34;
    if (wave.group_slot_ != 0x76543210U) return 35;
    for (int i = 0; i < 16; ++i) {{
        if (device.groups_[i].list_.head != 0 || device.groups_[i].list_.tail != 0 ||
            device.groups_[i].list_.count != 0) return 36;
    }}
    return 0;
}}

int main() {{
    int result = test_empty_list_append_and_post_call_slot_store(); if (result) return result;
    result = test_nonempty_tail_append_preserves_cursor(); if (result) return result;
    result = test_empty_list_allocation_failure_keeps_original_effects(); if (result) return result;
    result = test_invalid_inputs_do_not_allocate_or_mutate(); if (result) return result;
    return 0;
}}
"""


def compile_and_run(compiler: str, source: str) -> int:
    with tempfile.TemporaryDirectory(prefix="osmx-wave-list-boundary-") as temporary:
        executable = Path(temporary) / "actual-source-controls"
        compiled = subprocess.run(
            [compiler, "-std=c++11", "-O0", "-fcheck-new", "-x", "c++", "-", "-o", str(executable)],
            input=source,
            text=True,
            capture_output=True,
            timeout=45,
        )
        assert compiled.returncode == 0, compiled.stderr
        return subprocess.run([str(executable)], check=False, timeout=10).returncode


def host_compilers() -> list[str]:
    return [compiler for compiler in (shutil.which("g++"), shutil.which("clang++")) if compiler]


def test_actual_insert_and_add_to_group_effects() -> None:
    root = repository_root()
    parts = source_parts(root)
    insertion = parts["insert"]
    allocation = insertion.index("operator new(0xC)")
    explicit_null_arm = insertion.index("node = 0;")
    tail_snapshot = insertion.index("WaveGroupNode *const tail = this->tail;")
    tail_test = insertion.index("if (tail)")
    previous_next_store = insertion.index("(&tail->next) = node;")
    tail_reread = insertion.index("(&this->tail);", previous_next_store)
    tail_commit = insertion.index("this->tail = node;", tail_reread)
    assert allocation < explicit_null_arm < tail_snapshot < tail_test
    assert previous_next_store < tail_reread < tail_commit
    # The original live-tail allocation-failure case dereferences null at
    # node->prev. We preserve and source-check that quirk; we do not execute
    # the undefined fault path in a host test.
    tail_body = insertion[tail_test:]
    assert "node->prev =" in tail_body
    assert "if (node" not in tail_body

    compilers = host_compilers()
    assert compilers, "g++ or clang++ host compiler required"
    source = harness(parts)
    for compiler in compilers:
        assert compile_and_run(compiler, source) == 0


@pytest.mark.parametrize(
    ("function", "old", "new"),
    [
        ("add_to_group", "return 0xA;", "return 0;"),
        ("add_to_group", "wave->group_slot_ = group;", "wave->group_slot_ = 0;"),
        ("insert", "tail->next) = node;", "tail->next) = 0;"),
    ],
)
def test_source_mutants_fail_actual_effect_controls(function: str, old: str, new: str) -> None:
    parts = source_parts(repository_root())
    assert parts[function].count(old) == 1
    parts[function] = parts[function].replace(old, new, 1)
    source = harness(parts)
    compilers = host_compilers()
    assert compilers, "g++ or clang++ host compiler required"
    for compiler in compilers:
        assert compile_and_run(compiler, source) != 0
