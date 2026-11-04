"""Behavioral controls for the typed Win flag read in win_reparent_window.

The fixture executes the installed owner body with narrow Win32 stubs. It is
not a Win32 ABI or whole-game test; the pinned PE check separately anchors the
original byte-load, branch order, imports, and stdcall cleanup.
"""

from pathlib import Path
import hashlib
import os
import re
import shutil
import subprocess
import textwrap

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_text
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]
ADDRESS = 0x005F4D50
END = 0x005F4D90
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BODY_SHA256 = "6815431bae8a99fb62bd85cb3e8acb9777b7e7abe4a4ebf7dd93c4fab6f2d8dd"


def _definition(text, signature):
    start = text.index(signature)
    opening = text.index("{", start + len(signature))
    depth = 0
    state = "code"
    escaped = False
    index = opening
    while index < len(text):
        char = text[index]
        nxt = text[index + 1] if index + 1 < len(text) else ""
        if state == "line":
            if char == "\n":
                state = "code"
        elif state == "block":
            if char == "*" and nxt == "/":
                state = "code"
                index += 1
        elif state in ("string", "char"):
            if escaped:
                escaped = False
            elif char == "\\":
                escaped = True
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif char == "/" and nxt == "/":
            state = "line"
            index += 1
        elif char == "/" and nxt == "*":
            state = "block"
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
                return text[start:index + 1]
        index += 1
    raise AssertionError(f"unterminated definition: {signature}")


def _current():
    cpp = (ROOT / "src/win.cpp").read_text(encoding="utf-8")
    signature = "int __stdcall win_reparent_window(HWND hwnd, int unused)"
    return cpp, _definition(cpp, signature)


def _run_host(tmp_path, compiler, body, name):
    executable = shutil.which(compiler)
    if executable is None:
        pytest.skip(f"{compiler} unavailable")
    program = textwrap.dedent(r"""\
        #include <cassert>
        #include <cstdint>
        #include <vector>
        #define __stdcall
        typedef int HWND;
        const int GWL_USERDATA = -21;
        const uint32_t WinFlagClipToParent = 0x20;
        struct Win { uint32_t iFlags_; };
        static Win *owner_slot = 0;
        static HWND parent_slot = 0;
        static HWND matched_parent = 7;
        static HWND target_parent = 9;
        static std::vector<int> events;
        static HWND last_set_hwnd = 0;
        static HWND last_set_parent = 0;
        static int last_userdata_index = 0;
        static HWND last_userdata_hwnd = 0;
        static HWND last_parent_hwnd = 0;
        typedef intptr_t LONG_PTR;
        LONG_PTR GetWindowLongA(HWND hwnd, int index) {
            events.push_back(1);
            last_userdata_hwnd = hwnd;
            last_userdata_index = index;
            return reinterpret_cast<LONG_PTR>(owner_slot);
        }
        HWND GetParent(HWND hwnd) {
            events.push_back(2);
            last_parent_hwnd = hwnd;
            return parent_slot;
        }
        HWND SetParent(HWND hwnd, HWND parent) {
            events.push_back(3);
            last_set_hwnd = hwnd;
            last_set_parent = parent;
            return 0;
        }
        HWND WinReparentMatchParent = 0;
        HWND WinReparentTargetParent = 0;
        BODY
        static void reset_observations() {
            events.clear();
            last_set_hwnd = last_set_parent = 0;
            last_userdata_hwnd = last_parent_hwnd = 0;
            last_userdata_index = 0;
        }
        static void check_call(int unused, const std::vector<int> &expected) {
            const uint32_t old_flags = owner_slot ? owner_slot->iFlags_ : 0;
            assert(win_reparent_window(13, unused) == 1);
            assert(events == expected);
            assert(last_userdata_hwnd == 13 && last_userdata_index == GWL_USERDATA);
            if (expected.size() >= 2) assert(last_parent_hwnd == 13);
            if (expected.size() == 3) {
                assert(last_set_hwnd == 13 && last_set_parent == target_parent);
            } else {
                assert(last_set_hwnd == 0 && last_set_parent == 0);
            }
            if (owner_slot) assert(owner_slot->iFlags_ == old_flags);
        }
        int main() {
            Win owner;
            assert(WinFlagClipToParent == 0x20);
            WinReparentMatchParent = 7;
            WinReparentTargetParent = 9;

            // Null userdata returns after the lookup; the ignored second
            // argument still has stdcall-shaped source behavior and result 1.
            owner_slot = 0;
            parent_slot = 7;
            reset_observations();
            check_call(-1, std::vector<int>{1});

            // Zero flags reach the parent check. Individual unrelated low
            // and high bits also do not stand in for the 0x20 flag.
            owner_slot = &owner;
            owner.iFlags_ = 0;
            parent_slot = matched_parent;
            reset_observations();
            check_call(0x1234, std::vector<int>{1, 2, 3});
            owner.iFlags_ = 1;
            parent_slot = 8;
            reset_observations();
            check_call(99, std::vector<int>{1, 2});
            owner.iFlags_ = 0x80000000u;
            parent_slot = matched_parent;
            reset_observations();
            check_call(17, std::vector<int>{1, 2, 3});

            // The mask blocks regardless of unrelated high bits.
            owner.iFlags_ = 0x80000020u;
            parent_slot = 8;
            reset_observations();
            check_call(0, std::vector<int>{1});
            owner.iFlags_ = 0xffffffffu;
            reset_observations();
            check_call(-1, std::vector<int>{1});

            // Other high bits do not alias the low-byte flag; a wrong parent
            // stops before SetParent, while the matching parent reaches it.
            owner.iFlags_ = 0xffffffdfu;
            parent_slot = 8;
            reset_observations();
            check_call(17, std::vector<int>{1, 2});
            parent_slot = matched_parent;
            reset_observations();
            check_call(0, std::vector<int>{1, 2, 3});
            return 0;
        }
    """).replace("BODY", body)
    cpp_path = tmp_path / f"{name}.cpp"
    binary = tmp_path / name
    cpp_path.write_text(program, encoding="utf-8")
    built = subprocess.run([executable, "-std=c++11", "-Wall", "-Wextra",
                            "-Werror", str(cpp_path), "-o", str(binary)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stdout + built.stderr
    return subprocess.run([str(binary)], capture_output=True, text=True)


def test_mapped_owner_field_mask_and_claim_are_continuous():
    cpp, body = _current()
    baseline = subprocess.check_output(
        ["git", "show", "7ae732328ee74d7dbbb3ee069950146fa550140e:src/win_reparent.cpp"],
        cwd=ROOT, text=True, timeout=30)
    old_mask = ("(*reinterpret_cast<const uint8_t *>(&owner->iFlags_) &\n"
                "         WinFlagClipToParent) == 0")
    new_mask = "(owner->iFlags_ & WinFlagClipToParent) == 0"
    assert baseline.count(old_mask) == 1
    signature = "int __stdcall win_reparent_window(HWND hwnd, int unused)"
    assert body == _definition(baseline.replace(old_mask, new_mask, 1), signature)

    header = (ROOT / "src/win.h").read_text(encoding="utf-8")
    assert re.search(r"\buint32_t\s+iFlags_\s*;\s*//\s*0x98\b", header)
    assert re.search(r"WinFlagClipToParent\s*=\s*0x20\s*;\s*//\s*iFlags_", header)
    assert "reinterpret_cast<Win *>(" in body  # keep the Win32 userdata boundary
    assert "reinterpret_cast<const uint8_t *>(&owner->iFlags_)" not in body
    assert re.search(r"owner->iFlags_\s*&\s*WinFlagClipToParent", body)
    assert "(void)unused;" in body

    records = read_text(cpp, ROOT / "src/win.cpp")
    row, = [record for record in records if record.address == ADDRESS]
    assert row.image_spans == ((ADDRESS, END),)
    assert row.byte_exact is True and row.semantic is False
    assert row.symbol == "?win_reparent_window@@YGHPAUHWND__@@H@Z"


def test_pinned_full_span_proves_mask_branch_import_order_and_stdcall_cleanup():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    explicit = os.environ.get("OPENSMACX_IMAGE")
    image_path = Path(explicit) if explicit else ROOT / ".opensmacx/game/terranx_original.exe"
    if not image_path.is_file():
        pytest.skip("pinned original required; set OPENSMACX_IMAGE")
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    assert image_pin(ROOT) == PIN
    raw = image.read(Span(ADDRESS, END))
    assert hashlib.sha256(raw).hexdigest() == BODY_SHA256

    imports = {(entry.name, entry.address) for entry in image.imports()
               if entry.module.lower() == "user32.dll"}
    assert ("GetWindowLongA", 0x0066934C) in imports
    assert ("GetParent", 0x006692C4) in imports
    assert ("SetParent", 0x006692C0) in imports

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = list(decoder.disasm(raw, ADDRESS))
    cursor = ADDRESS
    by_address = {}
    for instruction in instructions:
        assert instruction.address == cursor, f"undecoded original byte at {cursor:#x}"
        by_address[instruction.address] = instruction
        cursor += instruction.size
    assert cursor == END and len(instructions) == 20
    assert by_address[0x005F4D50].mnemonic == "push"
    assert by_address[0x005F4D55].op_str == "-0x15"
    assert by_address[0x005F4D58].mnemonic == "call"
    assert by_address[0x005F4D58].operands[0].mem.disp == 0x0066934C
    assert by_address[0x005F4D60].operands[0].imm == 0x005F4D87
    mask = by_address[0x005F4D62]
    assert mask.mnemonic == "test" and mask.operands[0].size == 1
    assert mask.operands[0].mem.disp == 0x98 and mask.operands[1].imm == 0x20
    assert by_address[0x005F4D69].operands[0].imm == 0x005F4D87
    assert by_address[0x005F4D6C].operands[0].mem.disp == 0x006692C4
    assert by_address[0x005F4D72].operands[1].mem.disp == 0x009B7B40
    assert by_address[0x005F4D78].operands[0].imm == 0x005F4D87
    assert by_address[0x005F4D7A].operands[1].mem.disp == 0x009B7B44
    assert by_address[0x005F4D81].operands[0].mem.disp == 0x006692C0
    assert by_address[0x005F4D87].mnemonic == "mov"
    assert by_address[0x005F4D87].operands[1].imm == 1
    assert by_address[0x005F4D8D].mnemonic == "ret"
    assert by_address[0x005F4D8D].operands[0].imm == 8


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
def test_actual_source_semantics_run_on_host_compilers(tmp_path, compiler):
    _, body = _current()
    result = _run_host(tmp_path, compiler, body, "win_reparent_actual")
    assert result.returncode == 0, result.stdout + result.stderr


@pytest.mark.parametrize("compiler", ("g++", "clang++"))
@pytest.mark.parametrize("old,new", (
    ("WinFlagClipToParent) == 0", "WinFlagClipToParent) != 0"),
    ("WinFlagClipToParent", "0x40"),
    ("parent == WinReparentMatchParent", "parent != WinReparentMatchParent"),
    ("owner != 0 &&", "true &&"),
    ("WinReparentTargetParent", "WinReparentMatchParent"),
))
def test_semantic_mutations_fail_at_runtime(tmp_path, compiler, old, new):
    _, body = _current()
    assert old in body
    damaged = body.replace(old, new, 1)
    assert damaged != body
    result = _run_host(tmp_path, compiler, damaged, "win_reparent_mutant")
    assert result.returncode != 0, "mutant unexpectedly passed actual-source fixture"
