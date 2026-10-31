"""Pinned scalar handle evidence; this does not establish a larger global class."""
from dataclasses import replace
from pathlib import Path
import os

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86_const import X86_OP_IMM, X86_OP_MEM, X86_REG_EAX, X86_REG_EBX

from match_recovery.model import Origin, Span
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin

ROOT = Path(__file__).resolve().parents[2]
SYMBOL = "?PaletteInitialized@@3PAUHPALETTE__@@A"


@pytest.fixture(scope="module")
def evidence():
    path = Path(os.environ.get(
        "OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image unavailable")
    pin = image_pin(ROOT)
    image = PeImage(path.read_bytes(), pin)
    binding, = [row for row in read_objects(ROOT, pin) if row.symbol == SYMBOL]
    return image, binding


def _check_handle(image, binding):
    rows = {row.address: row for row in read_file(ROOT / "src/jackal/palette.cpp")}
    assert binding.evidence.address == 0x005FEBB0
    assert binding.evidence.entity_id == Origin(
        binding.origin.image_sha256, binding.origin.address_space,
        rows[0x005FEBB0].address).entity_id
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    instructions = {}
    for address in (0x005FEBB0, 0x005FECF0):
        for low, high in rows[address].image_spans:
            instructions.update((item.address, item) for item in decoder.disasm(
                image.read(Span(low, high)), low))
    imports = {item.address: (item.module.lower(), item.name)
               for item in image.imports()}
    create = instructions[0x005FECD6]
    assert create.mnemonic == "call" and create.operands[0].type == X86_OP_MEM
    assert imports[create.operands[0].mem.disp] == ("gdi32.dll", "CreatePalette")
    store = instructions[0x005FECDD]
    assert store.mnemonic == "mov" and store.operands[1].reg == X86_REG_EAX
    assert store.operands[0].type == X86_OP_MEM
    assert store.operands[0].mem.disp == binding.origin.address
    # The defining return-value store supplies scalar width, independently of
    # the current declaration, sizeof, neighboring symbols or zero-fill extent.
    assert binding.span.start == binding.origin.address
    assert binding.span.end - binding.span.start == store.operands[0].size
    load = instructions[0x005FECF0]
    assert load.mnemonic == "mov" and load.operands[0].reg == X86_REG_EAX
    assert load.operands[1].type == X86_OP_MEM
    assert load.operands[1].mem.disp == binding.origin.address
    assert load.operands[1].size == store.operands[0].size
    assert instructions[0x005FECF9].mnemonic == "push"
    assert instructions[0x005FECF9].operands[0].reg == X86_REG_EAX
    delete = instructions[0x005FECFA]
    assert delete.mnemonic == "call" and delete.operands[0].type == X86_OP_MEM
    assert imports[delete.operands[0].mem.disp] == ("gdi32.dll", "DeleteObject")
    for address in (0x005FEBC4, 0x005FED00):
        reset = instructions[address]
        assert reset.mnemonic == "mov" and reset.operands[0].type == X86_OP_MEM
        assert reset.operands[0].mem.disp == binding.origin.address
        assert reset.operands[0].size == store.operands[0].size
    zero = instructions[0x005FEBB6]
    assert zero.mnemonic == "xor"
    assert zero.operands[0].reg == zero.operands[1].reg == X86_REG_EBX
    assert instructions[0x005FEBC4].operands[1].reg == X86_REG_EBX
    assert instructions[0x005FED00].operands[1].type == X86_OP_IMM
    assert instructions[0x005FED00].operands[1].imm == 0
    assert image.read_initial(binding.span) == bytes(store.operands[0].size)


def test_original_handle_definition_and_consumer_bound(evidence):
    _check_handle(*evidence)


@pytest.mark.parametrize("damage", ["width", "target"])
def test_reject_wrong_scalar_width_or_object_target(evidence, damage):
    image, binding = evidence
    if damage == "width":
        damaged = replace(binding, span=Span(binding.span.start, binding.span.end + 4))
    else:
        shifted = binding.origin.address + 8
        damaged = replace(binding, origin=Origin(
            binding.origin.image_sha256, binding.origin.address_space, shifted),
            span=Span(shifted, shifted + 4))
    with pytest.raises(AssertionError):
        _check_handle(image, damaged)
