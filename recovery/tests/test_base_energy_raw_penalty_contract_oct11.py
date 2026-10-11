"""Pin the raw 32-bit penalty contract to the original Base energy span."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path

from capstone import CS_ARCH_X86, CS_MODE_32, Cs
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage

ROOT = Path(__file__).resolve().parents[2]
PINNED_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
DEFAULT_IMAGE = ROOT / ".opensmacx/game/terranx_original.exe"


def test_base_energy_raw_penalty_uses_pinned_native_contract() -> None:
    image = Path(os.environ.get("OPENSMACX_IMAGE", DEFAULT_IMAGE))
    assert image.is_file(), f"pinned original image is required, not skipped: {image}"
    image_bytes = image.read_bytes()
    assert hashlib.sha256(image_bytes).hexdigest() == PINNED_SHA256

    # Decode the mapped complete native span, and pin the control-flow split,
    # raw IMULs, copy clamp, and raw-word LEAs rather than trusting comments.
    start_va, end_va = 0x004EB560, 0x004EC3A6
    pe = PeImage(image_bytes, PINNED_SHA256)
    span = Span(start_va, end_va)
    decoded = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(pe.read(span), start_va))
    native = {instruction.address: (instruction.mnemonic, instruction.op_str)
              for instruction in decoded}
    assert decoded and decoded[0].address == start_va
    assert sum(len(instruction.bytes) for instruction in decoded) == end_va - start_va
    assert decoded[-1].address + len(decoded[-1].bytes) == end_va
    assert native[0x004EC212] == ("cmp", "edi, ecx")
    assert native[0x004EC214][0] == "jle" and native[0x004EC214][1] == "0x4ec2a9"
    assert native[0x004EC21A] == ("imul", "esi, eax")
    assert native[0x004EC21D] == ("test", "esi, esi")
    assert native[0x004EC22D] == ("mov", "eax, esi")
    assert native[0x004EC25B] == ("lea", "eax, [esi + esi]")
    assert native[0x004EC2A9] == ("imul", "esi, eax")
    assert native[0x004EC2AC] == ("lea", "eax, [esi + esi]")
    assert native[0x004EC2E9] == ("test", "esi, esi")

    source = (ROOT / "src/base.cpp").read_text()
    start = source.index("void __cdecl base_energy() {")
    end = source.index("\n// ORIGINAL:", start)
    body = source[start:end]
    assert "// ORIGINAL: 0x004EB560 ?base_energy@@YAXXZ 0x004EB560-0x004EC3A6" in source
    assert "const int raw_penalty = base_yield_mul(efficiency_factor, commerce_delta);" in body
    assert "penalty = base_yield_add(raw_penalty, raw_penalty);" in body
    assert "penalty *= 2;" not in body
