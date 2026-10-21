"""Source-owned, start-only bindings for six recovered global objects.

These checks establish original starts and emitted source identities only.
They do not claim object extents, full parent layout, lifetime, initializer,
strict-reference closure, or linked retention.
"""

from hashlib import sha256
import os
from pathlib import Path
import re

import pytest

from match_recovery.model import Span
from match_recovery.object_source import parse_objects, read_objects
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"

# Each declaration already exists. The v1 annotation records its start only.
EXPECTED = {
    0x0094F1B8: ("src/alpha.cpp", "?TimeControl@@3PAURulesTimeControl@@A",
                 r"RulesTimeControl\s+TimeControl\[MaxTimeControlNum\]\s*;"),
    0x00946580: ("src/alpha.cpp", "?SocialEffects@@3PAURulesSocialEffect@@A",
                 r"RulesSocialEffect\s+SocialEffects\[MaxSocialEffectNum\]\s*;"),
    0x0090EA38: ("src/base.cpp", "?BaseAtErrorGuard@@3HA", r"int\s+BaseAtErrorGuard\s*;"),
    0x0093A94C: ("src/base.cpp", "?BaseAtDebugPopupFlag@@3HA", r"int\s+BaseAtDebugPopupFlag\s*;"),
    0x009A6490: ("src/game.cpp", "?GameDataStateStorage@@3UGameDataStateWindow@@A",
                 r"GameDataStateWindow\s+GameDataStateStorage\s*;"),
    0x00937118: ("src/game.cpp", "?CoverWindow@@3VGraphicWin@@A", r"GraphicWin\s+CoverWindow\s*;"),
}


@pytest.fixture(scope="module")
def source_snapshot():
    parsed = {}
    sources = {}
    for address, (unit, _, _) in EXPECTED.items():
        if unit not in parsed:
            sources[unit] = (ROOT / unit).read_text(encoding="utf-8")
            parsed[unit] = parse_objects(sources[unit], unit, PIN, address_space="pe:va")
    # The native reader also validates project-wide symbol/start/extent
    # ownership, including interactions with every existing object binding.
    return parsed, sources, read_objects(ROOT, PIN)


@pytest.mark.parametrize("address", EXPECTED, ids=lambda address: EXPECTED[address][1])
def test_global_has_native_unbounded_start_binding_next_to_definition(source_snapshot, address):
    parsed, sources, all_bindings = source_snapshot
    unit, symbol, declaration = EXPECTED[address]
    matches = [row for row in parsed[unit] if row.origin.address == address]
    assert len(matches) == 1
    row = matches[0]
    assert (row.symbol, row.unit, row.origin.image_sha256, row.origin.address_space) == (
        symbol, unit, PIN, "pe:va")
    assert row.span is None and row.evidence is None
    assert row.parent_symbol is None and row.target_symbol is None

    lines = sources[unit].splitlines()
    assert lines[row.line - 1] == f"// RECOVERY_OBJECT: v1 0x{address:08X} {symbol}"
    assert re.fullmatch(declaration + r"(?:\s*//.*)?", lines[row.line].strip())
    assert sum(item.symbol == symbol for item in all_bindings) == 1
    assert sum(item.origin.address == address for item in all_bindings) == 1


def _pinned_image():
    configured = os.environ.get("OPENSMACX_IMAGE")
    path = Path(configured) if configured else ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("optional pinned original image is absent; native source-binding checks still run")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN, "wrong image supplied for pinned global operand evidence"
    return PeImage(raw, PIN)


def _instruction(image, address):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = tuple(decoder.disasm(image.read(Span(address, address + 12)), address))
    assert instructions and instructions[0].address == address
    return instructions[0]


def _assert_register(ins, operand_index, register):
    from capstone.x86 import X86_OP_REG

    operand = ins.operands[operand_index]
    assert operand.type == X86_OP_REG and ins.reg_name(operand.reg) == register


def _assert_absolute_memory(ins, operand_index, address, width):
    from capstone.x86 import X86_OP_MEM, X86_REG_INVALID

    operand = ins.operands[operand_index]
    assert operand.type == X86_OP_MEM and operand.size == width
    assert operand.mem.base == X86_REG_INVALID
    assert operand.mem.index == X86_REG_INVALID
    assert operand.mem.disp == address


def _assert_absolute_load(image, address, target, register, width):
    ins = _instruction(image, address)
    assert ins.mnemonic == "mov" and len(ins.operands) == 2
    _assert_register(ins, 0, register)
    _assert_absolute_memory(ins, 1, target, width)


def _assert_absolute_store_register(image, address, target, register, width):
    ins = _instruction(image, address)
    assert ins.mnemonic == "mov" and len(ins.operands) == 2
    _assert_absolute_memory(ins, 0, target, width)
    _assert_register(ins, 1, register)


def _assert_absolute_address_load(image, address, target, register):
    from capstone.x86 import X86_OP_IMM

    ins = _instruction(image, address)
    assert ins.mnemonic == "mov" and len(ins.operands) == 2
    _assert_register(ins, 0, register)
    assert ins.operands[1].type == X86_OP_IMM and ins.operands[1].imm == target


@pytest.mark.parametrize("address", EXPECTED, ids=lambda address: EXPECTED[address][1])
def test_global_start_has_pinned_original_operand_witnesses(address):
    image = _pinned_image()

    if address == 0x0094F1B8:
        # read_rules begins the first row at its turn word (+4), stores the
        # name pointer at ESI-4, then walks 0x20-byte rows. use_time_bonus
        # indexes refresh at table start + 0x18; none of this claims capacity.
        _assert_absolute_address_load(image, 0x005875AC, address + 4, "esi")
        name_store = _instruction(image, 0x005875BB)
        from capstone.x86 import X86_OP_MEM
        assert name_store.mnemonic == "mov" and len(name_store.operands) == 2
        target = name_store.operands[0]
        assert target.type == X86_OP_MEM and target.size == 4
        assert name_store.reg_name(target.mem.base) == "esi"
        assert target.mem.index == 0 and target.mem.disp == -4
        _assert_register(name_store, 1, "eax")
        row_index = _instruction(image, 0x005170D4)
        from capstone.x86 import X86_OP_IMM
        assert row_index.mnemonic == "shl" and len(row_index.operands) == 2
        _assert_register(row_index, 0, "edx")
        assert row_index.operands[1].type == X86_OP_IMM and row_index.operands[1].imm == 5
        refresh = _instruction(image, 0x005170D7)
        assert refresh.mnemonic == "mov" and len(refresh.operands) == 2
        _assert_register(refresh, 0, "ecx")
        from capstone.x86 import X86_OP_MEM
        source = refresh.operands[1]
        assert source.type == X86_OP_MEM and source.size == 4
        assert refresh.reg_name(source.mem.base) == "edx"
        assert source.mem.disp == address + 0x18
        return

    if address == 0x00946580:
        _assert_absolute_address_load(image, 0x00586BA6, address, "edi")
        _assert_absolute_address_load(image, 0x00586DD3, address, "edi")
    elif address == 0x0090EA38:
        _assert_absolute_load(image, 0x004E3AD8, address, "eax", 4)
        _assert_absolute_load(image, 0x004E3B09, address, "eax", 4)
        for site, value in ((0x004E3B17, 1), (0x004E3B49, 0)):
            ins = _instruction(image, site)
            from capstone.x86 import X86_OP_IMM
            assert ins.mnemonic == "mov" and len(ins.operands) == 2
            _assert_absolute_memory(ins, 0, address, 4)
            assert ins.operands[1].type == X86_OP_IMM and ins.operands[1].imm == value
    elif address == 0x0093A94C:
        _assert_absolute_load(image, 0x004E3B12, address, "eax", 4)
    elif address == 0x009A6490:
        _assert_absolute_store_register(image, 0x0059E961, address, "eax", 4)
    elif address == 0x00937118:
        for site in (0x004B4C42, 0x004B4CB1, 0x004B4CDA):
            _assert_absolute_address_load(image, site, address, "ecx")
    else:
        raise AssertionError(f"no pinned witness configured for {address:#x}")

