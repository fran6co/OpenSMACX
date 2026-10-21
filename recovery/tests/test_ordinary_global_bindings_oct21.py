"""Pinned source/global-identity and operand evidence for 20 data objects.

These checks establish start identities only. They do not assert full object
extents, initializers, lifetime, physical linker selection, or strict matches.
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

# address: (source unit, mangled original symbol, exact existing declaration)
EXPECTED = {
    0x009B8EC0: ("src/dialog.cpp", "?DialogDefaultFonts@@3PAPAVFont@@A", r"Font\s*\*\s*DialogDefaultFonts\[3\]\s*;"),
    0x0090EA44: ("src/base.cpp", "?BaseSiteClaimX@@3PAHA", r"int\s+BaseSiteClaimX\[4\]\s*;"),
    0x0090EA54: ("src/base.cpp", "?BaseSiteClaimY@@3PAHA", r"int\s+BaseSiteClaimY\[4\]\s*;"),
    0x009392B4: ("src/time.cpp", "?Blink2Counter@@3HA", r"int\s+Blink2Counter\s*;"),
    0x00939FD4: ("src/time.cpp", "?PlanLineCounter@@3HA", r"int\s+PlanLineCounter\s*;"),
    0x00703DE0: ("src/time.cpp", "?TurnRedrawPending@@3HA", r"int\s+TurnRedrawPending\s*;"),
    0x009BE6D4: ("src/cursor.cpp", "?CursorWidth@@3HA", r"int\s+CursorWidth\s*;"),
    0x009BE6D8: ("src/cursor.cpp", "?CursorHeight@@3HA", r"int\s+CursorHeight\s*;"),
    0x0093F670: ("src/netdaemon.cpp", "?SpockNotifyPending@@3HA", r"int\s+SpockNotifyPending\s*;"),
    0x0093F674: ("src/netdaemon.cpp", "?SpockHelpActive@@3HA", r"int\s+SpockHelpActive\s*;"),
    0x00939FD0: ("src/console.cpp", "?AutoPlayCounter@@3HA", r"int\s+AutoPlayCounter\s*=\s*0\s*;"),
    0x0068F220: ("src/interface_fonts.cpp", "?FontSizeTable@@3QBHB", r"const\s+int\s+FontSizeTable\[FontSizeTableCount\]\s*=\s*\{"),
    0x009B8D00: ("src/basepop.cpp", "?BasePopFalloutGate@@3HA", r"int\s+BasePopFalloutGate\s*;"),
    0x009B8FCC: ("src/editgroup.cpp", "?EditGroupDefault@@3IA", r"uint32_t\s+EditGroupDefault\s*;"),
    0x009B7B68: ("src/pulldown.cpp", "?PullDownDefaultFont@@3PAVFont@@A", r"Font\s*\*\s*PullDownDefaultFont\s*;"),
    0x009BC2D0: ("src/win.cpp", "?DirectDrawClipRect@@3UtagRECT@@A", r"RECT\s+DirectDrawClipRect\s*;"),
    0x009B7AA8: ("src/win.cpp", "?WinTrackingMode@@3HA", r"int\s+WinTrackingMode\s*;"),
    0x009B7AB0: ("src/win.cpp", "?WinTrackingX@@3HA", r"int\s+WinTrackingX\s*;"),
    0x009B7AB4: ("src/win.cpp", "?WinTrackingY@@3HA", r"int\s+WinTrackingY\s*;"),
    0x0067022C: ("src/jackal/palette.cpp", "?SystemColours@@3QBEB", r"const\s+uint8_t\s+SystemColours\[80\]\s*=\s*\{"),
}

# Kinds describe operand direction and width at exact original instructions.
# Address immediates are kept distinct from data reads. FontSizeTable has an
# explicit subsequent indexed-read witness; SystemColours is evidenced here by
# its original base-address load, without inferring extent or later-use bounds.
WITNESSES = {
    0x009B8EC0: (("store_abs_reg_addend", 0x00609D35, "eax", 4, 0),
                 ("store_abs_reg_addend", 0x00609D42, "eax", 4, 4),
                 ("store_abs_reg_addend", 0x00609D47, "ecx", 4, 8)),
    0x0090EA44: (("cmp_abs_read", 0x004F8120, "eax", 4),),
    0x0090EA54: (("cmp_abs_read", 0x004F8128, "ecx", 4),),
    0x009392B4: (("load_abs", 0x0050EE67, "eax", 4),
                 ("store_abs_reg", 0x0050EE72, "eax", 4)),
    0x00939FD4: (("load_abs", 0x0050EEAE, "eax", 4),
                 ("store_abs_reg", 0x0050EEBA, "eax", 4)),
    0x00703DE0: (("load_abs", 0x0050EF22, "eax", 4),),
    0x009BE6D4: (("store_abs_reg", 0x0063B91D, "eax", 4),),
    0x009BE6D8: (("store_abs_reg", 0x0063B924, "eax", 4),),
    0x0093F670: (("load_abs", 0x0051F1B9, "eax", 4),),
    0x0093F674: (("store_abs_imm", 0x0051F1D9, 1, 4),),
    0x00939FD0: (("load_abs", 0x0050E890, "eax", 4),
                 ("store_abs_reg", 0x0050E899, "eax", 4),
                 ("store_abs_imm", 0x0050E8BC, 0, 4)),
    0x0068F220: (("address_imm", 0x005882FF, "edx"),
                 ("indexed_read", 0x00588304, "ecx", "edx", 4)),
    0x009B8D00: (("load_abs", 0x00604A90, "eax", 4),),
    0x009B8FCC: (("load_abs", 0x00611AD3, "ecx", 4),),
    0x009B7B68: (("load_abs", 0x005F8920, "eax", 4),
                 ("store_abs_reg", 0x005F8931, "eax", 4)),
    0x009BC2D0: (("push_imm", 0x0062D201, 4),
                 ("push_imm", 0x0062D21A, 4)),
    0x009B7AA8: (("store_abs_reg", 0x005F2509, "ebx", 4),),
    0x009B7AB0: (("store_abs_reg", 0x005F2510, "eax", 4),),
    0x009B7AB4: (("store_abs_reg", 0x005F2515, "ecx", 4),),
    0x0067022C: (("address_imm", 0x005FE36A, "ecx"),),
}


@pytest.fixture(scope="module")
def source_objects():
    # Use the native parser both narrowly (to assert each unit's annotation
    # beside its declaration) and project-wide (to run global conflict checks).
    parsed_by_unit = {}
    source_by_unit = {}
    for address, (unit, _, _) in EXPECTED.items():
        if unit not in parsed_by_unit:
            source = (ROOT / unit).read_text(encoding="utf-8")
            source_by_unit[unit] = source
            parsed_by_unit[unit] = parse_objects(source, unit, PIN, address_space="pe:va")
    all_bindings = read_objects(ROOT, PIN)
    return parsed_by_unit, source_by_unit, all_bindings


@pytest.mark.parametrize("address", EXPECTED, ids=lambda address: EXPECTED[address][1])
def test_each_proposed_binding_is_native_v1_unbounded_and_next_to_its_definition(source_objects, address):
    parsed_by_unit, source_by_unit, all_bindings = source_objects
    unit, symbol, declaration = EXPECTED[address]
    # The package returns an ordered tuple; keep the query explicit to avoid
    # depending on line order across a source file's other bindings.
    matches = [row for row in parsed_by_unit[unit] if row.origin.address == address]
    assert len(matches) == 1
    row = matches[0]
    assert row.symbol == symbol
    assert row.unit == unit
    assert row.origin.address_space == "pe:va"
    assert row.origin.image_sha256 == PIN
    assert row.span is None and row.evidence is None
    assert row.parent_symbol is None and row.target_symbol is None

    source_lines = source_by_unit[unit].splitlines()
    annotation = source_lines[row.line - 1]
    assert annotation == f"// RECOVERY_OBJECT: v1 0x{address:08X} {symbol}"
    assert re.fullmatch(r"// RECOVERY_OBJECT: v1 0x[0-9A-Fa-f]+ \S+", annotation)
    assert re.fullmatch(declaration + r"(?:\s*//.*)?", source_lines[row.line].strip())

    # read_objects has already rejected project-wide duplicate starts/symbols
    # and overlap, but make this candidate-set statement direct and readable.
    assert sum(item.symbol == symbol for item in all_bindings) == 1
    assert sum(item.origin.address == address for item in all_bindings) == 1


def _pinned_image():
    configured = os.environ.get("OPENSMACX_IMAGE")
    path = Path(configured) if configured else ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("optional pinned original image is absent; native source tests still run")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN, "wrong image supplied for pinned evidence test"
    return PeImage(raw, PIN)


def _instruction(image, address):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = tuple(decoder.disasm(image.read(Span(address, address + 12)), address))
    assert instructions and instructions[0].address == address
    return instructions[0], instructions[1:]


def _reg(ins, operand):
    return ins.reg_name(operand.reg)


def _assert_abs_mem_operand(ins, index, address, width):
    from capstone.x86 import X86_OP_MEM, X86_REG_INVALID

    op = ins.operands[index]
    assert op.type == X86_OP_MEM and op.size == width
    assert op.mem.base == X86_REG_INVALID
    assert op.mem.index == X86_REG_INVALID
    assert op.mem.disp == address


def _check_witness(image, target, witness):
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

    kind, address, *args = witness
    ins, following = _instruction(image, address)
    if kind == "load_abs":
        register, width = args
        assert ins.mnemonic == "mov" and len(ins.operands) == 2
        assert ins.operands[0].type == X86_OP_REG and _reg(ins, ins.operands[0]) == register
        _assert_abs_mem_operand(ins, 1, target, width)
    elif kind == "cmp_abs_read":
        register, width = args
        assert ins.mnemonic == "cmp" and len(ins.operands) == 2
        assert ins.operands[0].type == X86_OP_REG and _reg(ins, ins.operands[0]) == register
        _assert_abs_mem_operand(ins, 1, target, width)
    elif kind == "store_abs_reg":
        register, width = args
        assert ins.mnemonic == "mov" and len(ins.operands) == 2
        _assert_abs_mem_operand(ins, 0, target, width)
        assert ins.operands[1].type == X86_OP_REG and _reg(ins, ins.operands[1]) == register
    elif kind == "store_abs_reg_addend":
        register, width, addend = args
        assert ins.mnemonic == "mov" and len(ins.operands) == 2
        _assert_abs_mem_operand(ins, 0, target + addend, width)
        assert ins.operands[1].type == X86_OP_REG and _reg(ins, ins.operands[1]) == register
    elif kind == "store_abs_imm":
        value, width = args
        assert ins.mnemonic == "mov" and len(ins.operands) == 2
        _assert_abs_mem_operand(ins, 0, target, width)
        assert ins.operands[1].type == X86_OP_IMM and ins.operands[1].imm == value
    elif kind == "push_imm":
        width, = args
        assert ins.mnemonic == "push" and len(ins.operands) == 1
        assert ins.operands[0].type == X86_OP_IMM and ins.operands[0].size == width
        assert ins.operands[0].imm == target
    elif kind == "address_imm":
        register, = args
        assert ins.mnemonic == "mov" and len(ins.operands) == 2
        assert ins.operands[0].type == X86_OP_REG and _reg(ins, ins.operands[0]) == register
        assert ins.operands[1].type == X86_OP_IMM and ins.operands[1].imm == target
    elif kind == "indexed_read":
        destination, base_register, width = args
        assert ins.mnemonic == "mov" and len(ins.operands) == 2
        assert ins.operands[0].type == X86_OP_REG and _reg(ins, ins.operands[0]) == destination
        source = ins.operands[1]
        assert source.type == X86_OP_MEM and source.size == width
        assert ins.reg_name(source.mem.base) == base_register
    else:
        raise AssertionError("unknown witness kind: " + kind)


@pytest.mark.parametrize("address", EXPECTED, ids=lambda address: EXPECTED[address][1])
def test_each_binding_has_pinned_original_operand_direction_and_width(address):
    image = _pinned_image()
    witnesses = WITNESSES[address]
    assert witnesses, "every proposed source identity needs at least one independent original witness"
    for witness in witnesses:
        _check_witness(image, address, witness)

