"""Behavioral and pinned-image witnesses for Win::set_base_window.

The host tests compile the actual C++ definition with bounded service doubles.
They test source-level sequencing; they do not establish the original game ABI,
generated lifetime, linked retention, or runtime behavior.
"""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest


ROOT = next(parent for parent in Path(__file__).resolve().parents
            if (parent / "src/win.cpp").is_file())
SOURCE = ROOT / "src/win.cpp"
BASE = "195e7c7babae8b2f1f28d33dcc21fbc436c7b623"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
START, END = 0x005F4D90, 0x005F4EB1
SIGNATURE = "int Win::set_base_window(GraphicWin *window)"
TOKEN = re.compile(
    r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.S
)


def definition(text: str) -> str:
    hits = list(re.finditer(re.escape(SIGNATURE), text))
    assert len(hits) == 1, (SIGNATURE, len(hits))
    start = hits[0].start()
    opened = False
    depth = 0
    for token in TOKEN.finditer(text, hits[0].end()):
        if token.group() == "{":
            opened = True
            depth += 1
        elif token.group() == "}":
            depth -= 1
            if opened and depth == 0:
                return text[start:token.end()]
    raise AssertionError("unterminated set_base_window definition")


def actual_body(mutant: str | None = None) -> str:
    body = definition(SOURCE.read_text(encoding="utf-8"))
    if mutant == "publish_before_resize":
        assert body.count("    poWinBase_ = window;\n") == 1
        body = body.replace("    poWinBase_ = window;\n", "", 1)
        needle = "    if (poWinBase_ != 0) {"
        assert body.count(needle) == 1
        body = body.replace(needle, "    poWinBase_ = window;\n" + needle, 1)
    elif mutant == "snapshot_root_count":
        needle = "    int i = 0;\n"
        assert body.count(needle) == 1
        body = body.replace(needle, needle + "    const int saved_count = WinRootCount;\n", 1)
        assert body.count("i < WinRootCount") == 1
        body = body.replace("i < WinRootCount", "i < saved_count", 1)
    elif mutant == "drop_flagged_recursion":
        needle = "            if (WinRootWindows[i]->iSomeFlag_ & 1) {"
        assert body.count(needle) == 1
        body = body.replace(needle, "            if (false) {", 1)
    elif mutant == "omit_update":
        assert body.count("    on_update();\n") == 1
        body = body.replace("    on_update();\n", "", 1)
    elif mutant == "wrong_return":
        assert body.count("    return 0;\n") == 2
        body = body.replace("    return 0;\n", "    return 1;\n", 1)
    elif mutant is not None:
        raise AssertionError("unknown mutation: " + mutant)
    return body


HOST = r"""
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

struct RECT { int left, top, right, bottom; };
static bool EqualRect(const RECT *a, const RECT *b) {
    return a->left == b->left && a->top == b->top &&
           a->right == b->right && a->bottom == b->bottom;
}
struct GraphicWin;
struct Win {
    RECT outer_rect_{};
    Win *poWinBase_ = nullptr;
    uint32_t iSomeFlag_ = 0;
    std::string tag;
    virtual ~Win() = default;
    virtual void on_update();
    int set_base_window(GraphicWin *window);
};
static std::vector<std::string> events;
struct GraphicWin : Win {
    RECT rect2_{};
    int resize_count = 0;
    int resize_width = 0, resize_height = 0, resize_mode = 0;
    Win *owner = nullptr;
    int resize(int width, int height, int mode) {
        ++resize_count;
        resize_width = width; resize_height = height; resize_mode = mode;
        if (owner) {
            assert(owner->poWinBase_ == resize_expected_base);
            events.push_back("resize");
        }
        return 0;
    }
    static Win *resize_expected_base;
};
Win *GraphicWin::resize_expected_base = nullptr;

static Win *WinRootWindows[8]{};
static int WinRootCount = 0;
static int WinZOrderCount = 0;
static int WinZOrderFlag = 0;
static Win *WinZOrderWindow = nullptr;
static void (*recurse_hook)(Win *) = nullptr;
static void recurse_zorder(Win *window) {
    events.push_back(window->tag);
    if (recurse_hook) recurse_hook(window);
}
struct TaggedWin : Win {};
static Win *update_expected_base = nullptr;
void Win::on_update() {
    assert(poWinBase_ == update_expected_base);
    events.push_back("update");
}

@@BODY@@

static void reset() {
    events.clear(); recurse_hook = nullptr; update_expected_base = nullptr;
    for (auto &root : WinRootWindows) root = nullptr;
    WinRootCount = WinZOrderCount = WinZOrderFlag = 0;
    WinZOrderWindow = nullptr;
}
static void append_root(Win *root) { WinRootWindows[WinRootCount++] = root; }

static void test_early_exits() {
    reset();
    Win owner; GraphicWin candidate;
    owner.outer_rect_ = {0, 0, 20, 30};
    owner.poWinBase_ = &candidate;
    candidate.owner = &owner;
    WinZOrderCount = 7; WinZOrderFlag = 9;
    assert(owner.set_base_window(nullptr) == 0x10);
    assert(owner.set_base_window(&candidate) == 0);
    assert(owner.poWinBase_ == &candidate);
    assert(candidate.resize_count == 0 && events.empty());
    assert(WinZOrderCount == 7 && WinZOrderFlag == 9);
}
static void test_outgoing_rectangle_and_publication_order() {
    reset();
    Win owner, outgoing; GraphicWin candidate;
    owner.poWinBase_ = &outgoing;
    outgoing.outer_rect_ = {4, 6, 29, 46};
    candidate.rect2_ = {0, 0, 1, 1};
    candidate.owner = &owner;
    GraphicWin::resize_expected_base = &outgoing;
    update_expected_base = &candidate;
    WinZOrderCount = 12;
    assert(owner.set_base_window(&candidate) == 0);
    assert(candidate.resize_count == 1 && candidate.resize_width == 25 &&
           candidate.resize_height == 40 && candidate.resize_mode == 1);
    assert(owner.poWinBase_ == &candidate);
    assert(WinZOrderCount == 0);
    assert((events == std::vector<std::string>{"resize", "update"}));
}
static void test_no_base_rectangle_and_equal_rectangle() {
    reset();
    Win owner; GraphicWin candidate;
    owner.outer_rect_ = {2, 3, 17, 28};
    candidate.rect2_ = {0, 0, 1, 1};
    candidate.owner = &owner;
    GraphicWin::resize_expected_base = nullptr;
    update_expected_base = &candidate;
    assert(owner.set_base_window(&candidate) == 0);
    assert(candidate.resize_count == 1 && candidate.resize_width == 15 &&
           candidate.resize_height == 25 && candidate.resize_mode == 1);
    assert((events == std::vector<std::string>{"resize", "update"}));

    reset();
    Win second_owner; GraphicWin same;
    second_owner.outer_rect_ = {2, 3, 17, 28};
    same.rect2_ = second_owner.outer_rect_;
    same.owner = &second_owner;
    update_expected_base = &same;
    assert(second_owner.set_base_window(&same) == 0);
    assert(same.resize_count == 0 && second_owner.poWinBase_ == &same);
    assert((events == std::vector<std::string>{"update"}));
}
static void mutate_roots_after_callback(Win *window) {
    if (window->tag == "A") {
        WinRootCount = 2;
        WinRootWindows[1] = WinRootWindows[2];
        WinZOrderWindow = WinRootWindows[1];
        WinZOrderCount = 5; WinZOrderFlag = 6;
    }
}
static void test_live_root_reloads_and_highlight_reset() {
    reset();
    Win owner; GraphicWin candidate;
    TaggedWin a, b, c;
    a.tag = "A"; b.tag = "B"; c.tag = "C";
    a.iSomeFlag_ = c.iSomeFlag_ = 1;
    append_root(&a); append_root(&b); append_root(&c);
    WinZOrderWindow = &b; WinZOrderCount = 11; WinZOrderFlag = 4;
    recurse_hook = mutate_roots_after_callback;
    update_expected_base = &candidate;
    assert(owner.set_base_window(&candidate) == 0);
    // The callback changes the active count and slot one. The loop must read
    // both globals again, visit C, and clear the now-selected C state.
    assert(events.size() == 3);
    assert(events[0] == "A"); assert(events[1] == "C"); assert(events[2] == "update");
    assert(WinZOrderCount == 0 && WinZOrderFlag == 0);
}
static void test_unflagged_root_is_not_recursed() {
    reset();
    Win owner; GraphicWin candidate; TaggedWin a;
    a.tag = "A"; append_root(&a);
    update_expected_base = &candidate;
    assert(owner.set_base_window(&candidate) == 0);
    assert((events == std::vector<std::string>{"update"}));
}
int main() {
    static_assert(sizeof(int) == 4, "the original target uses 32-bit int");
    test_early_exits();
    test_outgoing_rectangle_and_publication_order();
    test_no_base_rectangle_and_equal_rectangle();
    test_live_root_reloads_and_highlight_reset();
    test_unflagged_root_is_not_recursed();
}
"""


def compile_and_run(body: str, compiler: str, optimization: str) -> None:
    source = HOST.replace("@@BODY@@", body)
    with tempfile.TemporaryDirectory(prefix="win-set-base-") as tmp:
        cpp = Path(tmp) / "case.cpp"
        exe = Path(tmp) / "case"
        cpp.write_text(source, encoding="utf-8")
        compiled = subprocess.run(
            [compiler, "-std=c++17", optimization, "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True, text=True,
        )
        assert compiled.returncode == 0, compiled.stdout + compiled.stderr
        result = subprocess.run([str(exe)], capture_output=True, text=True)
        assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("optimization", ["-O0", "-O2"])
def test_actual_source_host_behavior_with_available_compilers(optimization: str) -> None:
    compilers = [compiler for compiler in ("g++", "clang++") if shutil.which(compiler)]
    if not compilers:
        pytest.skip("no supported host C++ compiler")
    for compiler in compilers:
        compile_and_run(actual_body(), compiler, optimization)


@pytest.mark.parametrize("mutant", [
    "publish_before_resize", "snapshot_root_count", "drop_flagged_recursion",
    "omit_update", "wrong_return",
])
def test_named_compiling_damage_controls_fail(mutant: str) -> None:
    compiler = shutil.which("g++") or shutil.which("clang++")
    if not compiler:
        pytest.skip("no supported host C++ compiler")
    body = actual_body(mutant)
    source = HOST.replace("@@BODY@@", body)
    with tempfile.TemporaryDirectory(prefix="win-set-base-mutant-") as tmp:
        cpp = Path(tmp) / "mutant.cpp"
        exe = Path(tmp) / "mutant"
        cpp.write_text(source, encoding="utf-8")
        compiled = subprocess.run(
            [compiler, "-std=c++17", "-O0", "-Wall", "-Wextra", "-Werror", str(cpp), "-o", str(exe)],
            capture_output=True, text=True,
        )
        assert compiled.returncode == 0, compiled.stdout + compiled.stderr
        result = subprocess.run([str(exe)], capture_output=True, text=True)
        assert result.returncode != 0, f"damage control survived: {mutant}"


def test_source_transport_inverse_is_exact_except_recorded_claim() -> None:
    current = SOURCE.read_text(encoding="utf-8")
    body = definition(current)
    assert "    poWinBase_ = window;\n\n    WinZOrderCount = 0;\n    int i = 0;" in body
    restored_body = body.replace(
        "    poWinBase_ = window;\n\n    WinZOrderCount = 0;\n    int i = 0;",
        "    poWinBase_ = window;\n\n    int i = 0;\n    WinZOrderCount = 0;", 1,
    )
    assert restored_body != body
    current = current.replace(body, restored_body, 1)
    baseline = subprocess.run(
        ["git", "show", f"{BASE}:src/win.cpp"], cwd=ROOT,
        check=True, capture_output=True, text=True,
    ).stdout
    original_claim = "// ORIGINAL: 0x005F4D90 sub_5f4d90 0x005F4D90-0x005F4EB1"
    recorded_claim = original_claim + " BYTE_EXACT"
    assert current.count(recorded_claim) == 1
    current = current.replace(recorded_claim, original_claim, 1)
    assert current == baseline


def test_pinned_original_full_span_and_order_anchors() -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    configured = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(configured) if configured else ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original unavailable; host tests do not replace original anchors")
    data = image_path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    image = PeImage(data, PIN)
    span = Span(START, END)
    raw = image.read(span)
    assert len(raw) == END - START == 289
    decoded = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(raw, START))
    assert len(decoded) == 96
    assert decoded[0].address == START
    assert decoded[-1].address + decoded[-1].size == END
    assert all(left.address + left.size == right.address
               for left, right in zip(decoded, decoded[1:]))
    by_address = {instruction.address: instruction for instruction in decoded}

    def instruction(address: int, mnemonic: str, operands: str) -> None:
        row = by_address[address]
        assert (row.mnemonic, row.op_str) == (mnemonic, operands)

    # Early null/same-base exits and the two distinct rectangle/resize paths.
    instruction(0x5F4D9A, "jne", "0x5f4da6")
    instruction(0x5F4D9D, "mov", "eax, 0x10")
    instruction(0x5F4DAE, "jne", "0x5f4db7")
    instruction(0x5F4DC9, "call", "dword ptr [0x6692bc]")
    instruction(0x5F4DFD, "call", "dword ptr [edx + 0xc]")
    instruction(0x5F4E10, "call", "dword ptr [0x6692bc]")
    instruction(0x5F4E3B, "call", "dword ptr [eax + 0xc]")
    # Base publication precedes zero publication; EBp is the zero source, then
    # the loop index is initialized independently before the root traversal.
    instruction(0x5F4E3E, "mov", "dword ptr [edi + 0xa8], esi")
    instruction(0x5F4E49, "xor", "edx, edx")
    instruction(0x5F4E4B, "xor", "ebp, ebp")
    instruction(0x5F4E4F, "mov", "dword ptr [0x9b7b30], ebp")
    instruction(0x5F4E55, "jle", "0x5f4e9f")
    instruction(0x5F4E57, "mov", "ecx, dword ptr [0x9b7a6c]")
    instruction(0x5F4E62, "cmp", "ecx, edx")
    instruction(0x5F4E70, "mov", "dword ptr [0x9b7a78], edx")
    instruction(0x5F4E76, "mov", "eax, dword ptr [esi]")
    instruction(0x5F4E82, "call", "0x5f4ec0")
    instruction(0x5F4E87, "mov", "ecx, dword ptr [0x9b7a6c]")
    instruction(0x5F4E92, "mov", "eax, dword ptr [0x9b7b34]")
    instruction(0x5F4E9F, "mov", "eax, dword ptr [edi]")
    instruction(0x5F4EA3, "call", "dword ptr [eax + 0xfc]")
    instruction(0x5F4EAB, "xor", "eax, eax")
    instruction(0x5F4EAE, "ret", "4")
