"""Host controls compiling the product GlobalListNode member bodies.

The test reads only tracked src/node.cpp and src/node.h. It needs neither the
pinned executable nor an ignored recovery artifact. These controls establish
source behavior on the host, not the original x86 layout or caller closure.
"""

from __future__ import annotations

import re
import shutil
import subprocess
import tempfile
from pathlib import Path

import pytest

ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/node.cpp").is_file())
SOURCE = ROOT / "src/node.cpp"
HEADER = ROOT / "src/node.h"
COMPILERS = [compiler for compiler in (shutil.which("g++"),
                                       shutil.which("clang++"))
             if compiler]


def _definition(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    quote = None
    escaped = False
    line_comment = block_comment = False
    index = opening
    while index < len(source):
        char = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""
        if line_comment:
            if char == "\n":
                line_comment = False
            index += 1
            continue
        if block_comment:
            if char == "*" and following == "/":
                block_comment = False
                index += 2
                continue
            index += 1
            continue
        if quote:
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif char == quote:
                quote = None
            index += 1
            continue
        if char == "/" and following == "/":
            line_comment = True
            index += 2
            continue
        if char == "/" and following == "*":
            block_comment = True
            index += 2
            continue
        if char in ("'", '"'):
            quote = char
        elif char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
        index += 1
    raise AssertionError("unterminated source definition: " + signature)


def _product_bodies():
    source = SOURCE.read_text(encoding="utf-8")
    return (_definition(source, "int GlobalListNode::enqueue()"),
            _definition(source, "int GlobalListNode::unlink()"))


def _fixture(enqueue: str, unlink: str) -> str:
    return r"""#include "node.h"
#include <cstring>

GlobalListNode *GlobalListHead = 0;
GlobalListNode *GlobalListTail = 0;

/* ACTUAL PRODUCT METHODS */

static void reset(GlobalListNode *node, unsigned int flags) {
    std::memset(node, 0, sizeof(*node));
    node->flags_ = flags;
}

int main() {
    GlobalListNode first, second, stale;
    GlobalListHead = GlobalListTail = 0;

    std::memset(&first, 0xA5, sizeof(first));
    first.flags_ = 0x80000180u;
    first.previous_ = first.next_ = 0;
    if (first.enqueue() != 0 || GlobalListHead != &first ||
        GlobalListTail != &first || first.previous_ != 0 ||
        first.next_ != 0 || first.flags_ != 0x80000182u) return 11;
    for (unsigned int i = 0; i < sizeof(first.opaque_prefix_); ++i)
        if (first.opaque_prefix_[i] != 0xA5) return 12;
    for (unsigned int i = 0; i < sizeof(first.opaque_8_17_); ++i)
        if (first.opaque_8_17_[i] != 0xA5) return 13;

    // The original guard checks links, not the set flag. Re-enqueueing the
    // sole node therefore creates a self-previous link, then clears next.
    if (first.enqueue() != 0 || first.previous_ != &first ||
        first.next_ != 0 || GlobalListHead != &first ||
        GlobalListTail != &first) return 14;
    if (first.unlink() != 0 || first.flags_ != 0x80000180u ||
        first.previous_ != 0 || first.next_ != 0 ||
        GlobalListHead != &first || GlobalListTail != &first) return 15;

    // Establish a two-node chain and prove append/removal preserve upper flag
    // bits while updating the actual head/tail and both intrusive links.
    GlobalListHead = GlobalListTail = 0;
    reset(&first, 0x80000180u);
    reset(&second, 0x40000204u);
    first.enqueue();
    second.enqueue();
    if (GlobalListHead != &first || GlobalListTail != &second ||
        first.previous_ != 0 || first.next_ != &second ||
        second.previous_ != &first || second.next_ != 0 ||
        first.flags_ != 0x80000182u || second.flags_ != 0x40000206u)
        return 16;
    second.unlink();
    if (GlobalListHead != &first || GlobalListTail != &first ||
        first.next_ != 0 || second.previous_ != 0 || second.next_ != 0 ||
        second.flags_ != 0x40000204u) return 17;
    first.unlink();
    if (GlobalListHead != 0 || GlobalListTail != 0 ||
        first.flags_ != 0x80000180u) return 18;

    // A nonmember with stale links is an early no-op for enqueue. An
    // unflagged unlink is also an early no-op and must not repair stale data.
    reset(&stale, 0xA0000040u);
    stale.next_ = &first;
    GlobalListHead = &second;
    GlobalListTail = &first;
    stale.enqueue();
    if (stale.next_ != &first || stale.previous_ != 0 ||
        stale.flags_ != 0xA0000040u || GlobalListHead != &second ||
        GlobalListTail != &first) return 19;
    stale.flags_ = 0xA0000040u;
    stale.unlink();
    if (stale.next_ != &first || stale.previous_ != 0 ||
        stale.flags_ != 0xA0000040u || GlobalListHead != &second ||
        GlobalListTail != &first) return 20;
    return 0;
}
""".replace("/* ACTUAL PRODUCT METHODS */", enqueue + "\n\n" + unlink)


def _run(tmp_path: Path, compiler: str, enqueue: str, unlink: str,
         expected_success: bool) -> None:
    tmp_path.mkdir(parents=True, exist_ok=True)
    cpp = tmp_path / "node_control.cpp"
    exe = tmp_path / "node_control"
    cpp.write_text(_fixture(enqueue, unlink).replace(
        '#include "node.h"', '#include "' + HEADER.as_posix() + '"'), encoding="utf-8")
    build = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
         str(cpp), "-o", str(exe)],
        capture_output=True, text=True, timeout=30)
    assert build.returncode == 0, build.stdout + build.stderr
    run = subprocess.run([str(exe)], capture_output=True, text=True,
                         timeout=10)
    assert (run.returncode == 0) is expected_success, run.stdout + run.stderr


@pytest.fixture(params=COMPILERS or [None],
                ids=[Path(item).name for item in COMPILERS] or ["no-host-compiler"])
def compiler(request):
    if request.param is None:
        pytest.skip("no GCC or Clang host compiler available")
    return request.param


def test_identity_claims_and_source_shape_are_not_seed_class_guesses():
    from match_recovery.source import read_file

    rows = read_file(SOURCE)
    by_address = {row.address: row for row in rows}
    assert set(by_address) == {0x004C4CB0, 0x004C4D10}
    assert by_address[0x004C4CB0].name == "sub_4c4cb0"
    assert by_address[0x004C4CB0].image_spans == ((0x004C4CB0, 0x004C4D01),)
    assert by_address[0x004C4CB0].byte_exact
    assert by_address[0x004C4D10].name == "sub_4c4d10"
    assert by_address[0x004C4D10].image_spans == ((0x004C4D10, 0x004C4D6C),)
    assert by_address[0x004C4D10].byte_exact

    header = HEADER.read_text(encoding="utf-8")
    assert re.search(r"unsigned\s+char\s+opaque_prefix_\[4\]", header)
    assert re.search(r"unsigned\s+int\s+flags_\s*;", header)
    assert re.search(r"GlobalListNode\s*\*\s*previous_", header)
    assert re.search(r"GlobalListNode\s*\*\s*next_", header)
    assert "RECOVERY_MEMBER: v1 GlobalListNode flags_ 0x4 0x004C4CB0" in header
    assert "RECOVERY_MEMBER: v3 GlobalListNode previous_ GlobalListNode* 0x18" in header
    assert "RECOVERY_MEMBER: v3 GlobalListNode next_ GlobalListNode* 0x1C" in header
    assert "?enqueue@Node@@" not in SOURCE.read_text(encoding="utf-8")
    assert "?unlink@ListNode4C4D10@@" not in SOURCE.read_text(encoding="utf-8")


def test_product_enqueue_unlink_semantics_and_damage_controls(tmp_path: Path,
                                                              compiler: str):
    enqueue, unlink = _product_bodies()
    with tempfile.TemporaryDirectory(prefix="node-owner-control-") as temp:
        directory = Path(temp)
        _run(directory / "good", compiler, enqueue, unlink, True)

        repeat_guard = enqueue.replace(
            "if (next_ == 0 && previous_ == 0)",
            "if (next_ == 0 && previous_ == 0 && (flags_ & 2u) == 0)", 1)
        assert repeat_guard != enqueue
        _run(directory / "repeat-guard-mutant", compiler,
             repeat_guard, unlink, False)

        word_truncation = enqueue.replace(
            "flags_ |= 2u;", "flags_ = (flags_ & 0xFFu) | 2u;", 1)
        assert word_truncation != enqueue
        _run(directory / "dword-flags-mutant", compiler,
             word_truncation, unlink, False)

        remove_guard = unlink.replace(
            "if ((flags_ & 2u) == 0)", "if (false)", 1)
        assert remove_guard != unlink
        _run(directory / "unlink-guard-mutant", compiler,
             enqueue, remove_guard, False)
