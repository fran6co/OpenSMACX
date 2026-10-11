"""Pinned control-flow anchors for shared movie-reader publication order."""

from __future__ import annotations

import os
from pathlib import Path

import pytest


ROOT = Path(__file__).resolve().parents[2]
SRC = ROOT / "src"
PINNED_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def test_pinned_frame_and_header_reader_commit_boundaries() -> None:
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from capstone.x86 import (
        X86_OP_IMM, X86_OP_MEM, X86_OP_REG,
        X86_REG_EAX, X86_REG_ESI,
    )
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    default_images = (
        ROOT / ".opensmacx/game/terranx_original.exe",
        ROOT.parents[1] / "game/terranx_original.exe",
    )
    image_path = Path(os.environ["OPENSMACX_IMAGE"]) if "OPENSMACX_IMAGE" in os.environ else next(
        (candidate for candidate in default_images if candidate.is_file()),
        default_images[0],
    )
    if not image_path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(image_path.read_bytes(), PINNED_SHA256)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True

    def decoded(start: int, end: int):
        instructions = list(decoder.disasm(image.read(Span(start, end)), start))
        assert sum(item.size for item in instructions) == end - start
        return {item.address: item for item in instructions}

    def mem(insn, operand: int, address: int) -> bool:
        item = insn.operands[operand]
        return item.type == X86_OP_MEM and item.mem.base == 0 and item.mem.index == 0 and item.mem.disp == address

    def immediate(insn, operand: int, value: int) -> bool:
        item = insn.operands[operand]
        return item.type == X86_OP_IMM and item.imm == value

    def reg(insn, operand: int, value: int) -> bool:
        item = insn.operands[operand]
        return item.type == X86_OP_REG and item.reg == value

    header = decoded(0x5D2E70, 0x5D35A4)
    # The five header/prelude short-count decisions are unsigned JAE branches;
    # keep this distinct from the signed thresholds inside coefficient decode.
    for address in (0x5D2EDC, 0x5D2F7F, 0x5D2FA5, 0x5D2FEF, 0x5D3042):
        assert header[address].mnemonic == "jae"
    # The packet reader also uses JAE before its flagged-prefix refill.
    assert header[0x5D30C0].mnemonic == "cmp"
    assert reg(header[0x5D30C0], 0, 19)  # eax
    assert immediate(header[0x5D30C0], 1, 0x10)
    assert mem(header[0x5D30C3], 0, 0x9C4654)
    assert mem(header[0x5D30C8], 0, 0x9C4650)
    assert header[0x5D30CE].mnemonic == "jae"
    assert mem(header[0x5D30E7], 0, 0x9C4654)
    assert mem(header[0x5D30F5], 0, 0x9C4650)
    assert mem(header[0x5D30FB], 0, 0x9C464C)
    # The no-flags path commits accumulator/count before refill; after refill,
    # count, accumulator, then cursor. Flagged refill stores cursor alone.
    assert mem(header[0x5D2ED1], 0, 0x9C4650)
    assert mem(header[0x5D2ED7], 0, 0x9C4654)
    assert mem(header[0x5D2EF5], 0, 0x9C4654)
    assert mem(header[0x5D2EFF], 0, 0x9C4650)
    assert mem(header[0x5D2F05], 0, 0x9C464C)
    assert mem(header[0x5D2FC3], 0, 0x9C464C)
    assert mem(header[0x5D3011], 0, 0x9C464C)

    frame = decoded(0x5D35B0, 0x5D3980)
    assert frame[0x5D3627].mnemonic == "mov"
    assert reg(frame[0x5D3627], 0, X86_REG_EAX)
    assert mem(frame[0x5D3627], 1, 0x9B336C)
    assert frame[0x5D362C].mnemonic == "movsx"
    assert frame[0x5D362C].operands[1].type == X86_OP_MEM
    assert frame[0x5D362C].operands[1].mem.base == X86_REG_ESI
    assert frame[0x5D362C].operands[1].mem.disp == 0x0d
    assert frame[0x5D3640].mnemonic == "jne"
    assert 0x5D3627 < 0x5D362C < 0x5D3640
    assert frame[0x5D380A].mnemonic == "mov"
    assert mem(frame[0x5D380A], 0, 0x9B336C)
    assert frame[0x5D3814].mnemonic == "mov"
    assert frame[0x5D381A].mnemonic == "mov"
    assert frame[0x5D381A].operands[1].type == X86_OP_MEM
    assert frame[0x5D381A].operands[1].mem.base == X86_REG_ESI
    assert frame[0x5D381A].operands[1].mem.disp == 0x10
    assert frame[0x5D381E].operands[1].mem.disp == 0x12
    assert mem(frame[0x5D382D], 0, 0x9C4654)
    assert immediate(frame[0x5D382D], 1, 0x20)
    assert mem(frame[0x5D3837], 0, 0x9C4650)
    assert mem(frame[0x5D383C], 1, 0x696938)
    assert mem(frame[0x5D3844], 0, 0x9C464C)
    assert mem(frame[0x5D384A], 0, 0x9B3364)
    assert (0x5D380A < 0x5D3814 < 0x5D381A < 0x5D381E < 0x5D382D
            < 0x5D3837 < 0x5D383C < 0x5D3844 < 0x5D384A)
