"""Original battle storage starts; not allocation, lifetime or link-selection proof."""
from hashlib import sha256
import os
from pathlib import Path

import pytest

from match_recovery.model import Span
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
STARTS = (
    (0x90F550, "?VehBattleDisplayTerrain@@3PADA"),
    (0x912420, "?VehBasicBattleMorale@@3PAIA"),
    (0x9155F0, "?VehBattleModifier@@3PAHA"),
    (0x90F554, "?VehBattleDisplay@@3PAY0FA@DA"),
)


@pytest.mark.parametrize("address,symbol", STARTS)
def test_battle_storage_has_one_start_only_source_identity(address, symbol):
    bindings = read_objects(ROOT, PIN)
    matches = [row for row in bindings if row.symbol == symbol]
    assert len(matches) == 1
    binding = matches[0]
    assert binding.unit == "src/veh.cpp"
    assert binding.origin.address == address
    assert binding.span is None and binding.evidence is None
    assert binding.parent_symbol is None and binding.target_symbol is None


@pytest.fixture(scope="module")
def original():
    configured = os.environ.get("OPENSMACX_IMAGE")
    candidates = ([Path(configured)] if configured else
                  [parent / ".opensmacx/game/terranx_original.exe"
                   for parent in (ROOT, *ROOT.parents)])
    path = next((candidate for candidate in candidates if candidate.is_file()), None)
    if path is None:
        if configured:
            pytest.fail("configured original executable is unavailable")
        pytest.skip("pinned original is required for operand witnesses")
    data = path.read_bytes()
    assert sha256(data).hexdigest() == PIN
    return PeImage(data, PIN)


# Memory operands prove direction/width and indexed dword selection. ADD
# immediates prove an address calculation, not a read or allocation extent.
@pytest.mark.parametrize("site,mnemonic,index,address,width,register,scale", (
    (0x5011AE, "mov", 0, 0x90F550, 4, None, 1),
    (0x50320F, "mov", 1, 0x90F550, 4, None, 1),
    (0x501931, "mov", 0, 0x912420, 4, "edx", 4),
    (0x501C1C, "mov", 0, 0x912424, 4, None, 1),
    (0x4227DC, "mov", 1, 0x912420, 4, "edi", 4),
    (0x501D7A, "mov", 0, 0x9155F0, 4, "eax", 4),
    (0x4228A3, "add", 1, 0x9155F0, None, None, None),
    (0x501D87, "add", 1, 0x90F554, None, None, None),
    (0x4228A9, "add", 1, 0x90F554, None, None, None),
))
def test_original_battle_storage_operands(original, site, mnemonic, index,
                                          address, width, register, scale):
    import capstone
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM

    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    decoder.detail = True
    instruction, = decoder.disasm(original.read(Span(site, site + 15)), site, count=1)
    assert instruction.address == site and instruction.mnemonic == mnemonic
    operand = instruction.operands[index]
    if width is None:
        assert operand.type == X86_OP_IMM and operand.imm == address
    else:
        assert operand.type == X86_OP_MEM and operand.size == width
        assert operand.mem.disp == address and operand.mem.base == 0
        assert (instruction.reg_name(operand.mem.index) if operand.mem.index else None) == register
        assert operand.mem.scale == scale
