"""Bounded original-frame evidence for FameWin::demote's numeric buffer.

The pinned frame has an 0x588-byte Buffer followed by an 0x50-byte local
region whose first byte is passed to each _itoa call. This supports an
80-byte source extent; it does not recover an original debug name or prove the
literal original declaration.
"""
from pathlib import Path
import hashlib
import os
import re

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE = Path(os.environ.get(
    "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
DECODER = Cs(CS_ARCH_X86, CS_MODE_32)


def definition(text: str, signature: str) -> str:
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    token = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]', re.S)
    for match in token.finditer(text, opening):
        if match.group() == "{":
            depth += 1
        elif match.group() == "}":
            depth -= 1
            if depth == 0:
                return text[start:match.end()]
    raise AssertionError(f"unterminated definition: {signature}")


def instructions(image: PeImage, start: int, end: int):
    raw = image.read(Span(start, end))
    decoded = tuple(DECODER.disasm(raw, start))
    assert sum(item.size for item in decoded) == len(raw)
    return decoded


def test_demote_source_extent_matches_pinned_frame_local_region():
    raw_image = IMAGE.read_bytes()
    assert hashlib.sha256(raw_image).hexdigest() == PIN
    image = PeImage(raw_image, PIN)
    source = (ROOT / "src/famewin.cpp").read_text()
    body = definition(source, "void FameWin::demote(int rank) {")
    buffer_header = (ROOT / "src/buffer.h").read_text()
    assert re.search(r"static_assert\s*\(\s*sizeof\(Buffer\)\s*==\s*0x588\b",
                     buffer_header)

    # EH registration occupies EBP-0x0c through EBP-1; the local allocation
    # below it is exactly 0x5d8 bytes.
    prologue = instructions(image, 0x44AC10, 0x44AC2E)
    assert [(i.mnemonic, i.op_str) for i in prologue] == [
        ("push", "ebp"),
        ("mov", "ebp, esp"),
        ("push", "-1"),
        ("push", "0x654ec5"),
        ("mov", "eax, dword ptr fs:[0]"),
        ("push", "eax"),
        ("mov", "dword ptr fs:[0], esp"),
        ("sub", "esp, 0x5d8"),
    ]

    # The object starts at EBP-0x5e4 and the original constructor/destructor
    # calls receive that exact address. The Buffer size puts its upper edge at
    # EBP-0x5c, exactly where _itoa's output pointer begins.
    ctor = instructions(image, 0x44AC32, 0x44AC3D)
    assert [(i.mnemonic, i.op_str) for i in ctor] == [
        ("lea", "ecx, [ebp - 0x5e4]"), ("call", "0x5d7210")]
    dtor = instructions(image, 0x44AE1E, 0x44AE30)
    assert [(i.mnemonic, i.op_str) for i in dtor] == [
        ("lea", "ecx, [ebp - 0x5e4]"),
        ("mov", "dword ptr [ebp - 4], 0xffffffff"),
        ("call", "0x5d7410"),
    ]

    # Observe only the four _itoa argument windows, not a whole-function
    # byte freeze. Each passes EBP-0x5c as argument two (the output destination).
    itoa_windows = (
        (0x44AC6C, 0x44AC78, "ecx", None),
        (0x44AC95, 0x44ACA4, "eax", "[edi + 1]"),
        (0x44ACF9, 0x44AD05, "ecx", None),
        (0x44AD22, 0x44AD31, "eax", "[edi + 2]"),
    )
    for start, end, output_register, value_address in itoa_windows:
        expected = [("lea", output_register + ", [ebp - 0x5c]"), ("push", "0xa")]
        if value_address is None:
            expected += [("push", "ecx"), ("push", "eax")]
        else:
            expected += [("lea", "ecx, " + value_address), ("push", "eax"), ("push", "ecx")]
        expected.append(("call", "0x64fc88"))
        assert [(i.mnemonic, i.op_str) for i in instructions(image, start, end)] == expected

    buffer_start = -0x5E4
    buffer_size = 0x588
    numeric_start = -0x5C
    eh_record = -0x0C
    frame_allocation = 0x5D8
    assert buffer_start + buffer_size == numeric_start
    inferred_extent = eh_record - numeric_start
    assert inferred_extent == 0x50 == 80
    assert buffer_size + inferred_extent == frame_allocation

    assert re.search(r"\bchar\s+number\s*\[\s*80\s*\]\s*;", body)
    calls = re.findall(r"_itoa\s*\(([^;]*?)\);", body)
    assert len(calls) == 4
    for call in calls:
        # The middle argument is the actual output array at every call site.
        args = [arg.strip() for arg in call.split(",")]
        assert len(args) == 3 and args[1:] == ["number", "10"]
