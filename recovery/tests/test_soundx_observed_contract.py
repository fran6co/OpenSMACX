"""Bounded semantic witnesses for the two Wave-In provider operations.

This deliberately stores neither executable-code fixtures nor a copied
provider vtable. Only pinned source addresses and operation semantics remain.
"""
from hashlib import sha256
from pathlib import Path
import struct

import pytest

from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.pe_exports import exports

ROOT = Path(__file__).resolve().parents[2]
GAME = ROOT / ".opensmacx/game/terranx_original.exe"
DLL = ROOT / ".opensmacx/game/soundx.dll"
GAME_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
DLL_SHA256 = "ac272e244b3f1339d9ce63bc2e7acf76605c6ac2695bba3230e72ea59039a3f5"


def _pinned(path, digest):
    if not path.is_file():
        pytest.skip(f"pinned original dependency unavailable: {path.name}")
    data = path.read_bytes()
    assert sha256(data).hexdigest() == digest, f"present original has wrong digest: {path}"
    return PeImage(data, digest)


def _disassemble(image, address, size):
    try:
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    except ImportError:
        pytest.skip("Capstone unavailable for bounded i386 instruction decoding")
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    raw = image.read(Span(address, address + size))
    instructions = tuple(decoder.disasm(raw, address))
    assert instructions and instructions[0].address == address
    assert instructions[-1].address + instructions[-1].size == address + size
    return instructions


def _by(instructions, mnemonic):
    return tuple(item for item in instructions if item.mnemonic == mnemonic)


def _assert_game_wrapper(instructions, operation_slot):
    rendered = tuple((item.mnemonic, item.op_str.lower()) for item in instructions)
    assert any(mnemonic == "mov" and operand == "ecx, dword ptr [ecx + 0x14]"
               for mnemonic, operand in rendered)
    assert any(mnemonic == "test" and operand == "ecx, ecx" for mnemonic, operand in rendered)
    assert any(mnemonic == "mov" and operand == "edx, dword ptr [ebp + 8]"
               for mnemonic, operand in rendered)
    calls = [operand for mnemonic, operand in rendered if mnemonic == "call"]
    assert f"dword ptr [eax + {operation_slot:#x}]" in calls
    null_result = [i for i in instructions if i.mnemonic == "mov" and
                   i.op_str.lower() == "eax, 0x13"]
    assert len(null_result) == 1
    assert any(i.mnemonic == "ret" and i.op_str == "4" for i in instructions)
    branches = [i for i in instructions if i.mnemonic in ("je", "jz")]
    assert any(int(i.op_str, 16) == null_result[0].address for i in branches)


def _assert_provider_codec(instructions):
    rendered = tuple((item.mnemonic, item.op_str.lower()) for item in instructions)
    assert any(mnemonic == "mov" and operand == "ecx, dword ptr [ecx + 0x14]"
               for mnemonic, operand in rendered)
    assert any(mnemonic == "mov" and operand == "edx, dword ptr [ebp + 8]"
               for mnemonic, operand in rendered)
    assert any(mnemonic == "call" and operand == "dword ptr [eax + 0x60]"
               for mnemonic, operand in rendered)
    assert any(mnemonic == "ret" and operand == "4" for mnemonic, operand in rendered)
    assert any(mnemonic == "mov" and operand == "eax, 0x13"
               for mnemonic, operand in rendered)


def _assert_provider_key(instructions):
    rendered = tuple((item.mnemonic, item.op_str.lower()) for item in instructions)
    assert any(mnemonic == "xor" and operand == "eax, eax" for mnemonic, operand in rendered)
    assert any(mnemonic == "ret" and operand == "4" for mnemonic, operand in rendered)


def _factory_vtable_address(instructions):
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM

    for item in instructions:
        if item.mnemonic != "mov" or len(item.operands) != 2:
            continue
        destination, source = item.operands
        if (destination.type == X86_OP_MEM and destination.mem.disp == 0 and
                source.type == X86_OP_IMM):
            return source.imm & 0xFFFFFFFF
    raise AssertionError("factory has no direct zero-offset object vptr store")


def _direct_jump_target(instruction):
    assert instruction.mnemonic == "jmp"
    target = int(instruction.op_str, 16)
    return target & 0xFFFFFFFF


def test_pinned_game_wrappers_forward_observed_slots_and_stack_arguments():
    image = _pinned(GAME, GAME_SHA256)
    codec = _disassemble(image, 0x004C5A80, 32)
    key = _disassemble(image, 0x004C5AA0, 32)
    _assert_game_wrapper(codec, 0x60)
    _assert_game_wrapper(key, 0x64)

    # Corrupt actual instruction operands, decode the modified bounded span,
    # and require the same semantic validator to reject each mutation.
    codec_raw = bytearray(image.read(Span(0x004C5A80, 0x004C5AA0)))
    codec_call = next(i for i in codec if i.mnemonic == "call")
    codec_raw[codec_call.address - 0x004C5A80 + codec_call.size - 1] ^= 4
    try:
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    except ImportError:
        pytest.skip("Capstone unavailable for mutation decode")
    mutated_codec = tuple(Cs(CS_ARCH_X86, CS_MODE_32).disasm(codec_raw, 0x004C5A80))
    with pytest.raises(AssertionError):
        _assert_game_wrapper(mutated_codec, 0x60)

    key_raw = bytearray(image.read(Span(0x004C5AA0, 0x004C5AC0)))
    arg_load = next(i for i in key if i.mnemonic == "mov" and "[ebp + 8]" in i.op_str.lower())
    key_raw[arg_load.address - 0x004C5AA0 + arg_load.size - 1] ^= 4
    mutated_key = tuple(Cs(CS_ARCH_X86, CS_MODE_32).disasm(key_raw, 0x004C5AA0))
    with pytest.raises(AssertionError):
        _assert_game_wrapper(mutated_key, 0x64)


def test_pinned_exports_factory_installed_entries_and_concrete_slot_contracts():
    image = _pinned(DLL, DLL_SHA256)
    entries = exports(image)
    by_ordinal = {entry.ordinal: entry for entry in entries}
    assert any("create_device@Dll_Wave_In_Device" in name for name in by_ordinal[9].names)
    assert any("delete_device@Dll_Wave_In_Device" in name for name in by_ordinal[10].names)

    # Decode the bounded factory body and derive the table from its actual vptr
    # immediate rather than embedding a table address/value array.
    factory = _disassemble(image, 0x1000CD30, 0x7E)
    vtable = _factory_vtable_address(factory)
    slot_targets = struct.unpack("<II", image.read(Span(vtable + 24 * 4, vtable + 26 * 4)))
    slot24_thunk, slot25_thunk = slot_targets
    slot24_jump = _disassemble(image, slot24_thunk, 5)
    slot25_jump = _disassemble(image, slot25_thunk, 5)
    slot24_body = _direct_jump_target(slot24_jump[0])
    slot25_body = _direct_jump_target(slot25_jump[0])

    codec = _disassemble(image, slot24_body, 32)
    key = _disassemble(image, slot25_body, 5)
    _assert_provider_codec(codec)
    _assert_provider_key(key)

    # A table-slot swap is rejected by the operation-specific decoders.
    with pytest.raises(AssertionError):
        _assert_provider_codec(_disassemble(image, slot25_body, 5))
    with pytest.raises(AssertionError):
        _assert_provider_key(_disassemble(image, slot24_body, 32))

    try:
        from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    except ImportError:
        pytest.skip("Capstone unavailable for mutation decode")
    codec_raw = bytearray(image.read(Span(slot24_body, slot24_body + 32)))
    call = next(i for i in codec if i.mnemonic == "call")
    codec_raw[call.address - slot24_body + call.size - 1] ^= 4
    mutated = tuple(Cs(CS_ARCH_X86, CS_MODE_32).disasm(codec_raw, slot24_body))
    with pytest.raises(AssertionError):
        _assert_provider_codec(mutated)

    key_raw = bytearray(image.read(Span(slot25_body, slot25_body + 5)))
    key_raw[0] = 0x90  # replace zeroing opcode with NOP
    mutated_key = tuple(Cs(CS_ARCH_X86, CS_MODE_32).disasm(key_raw, slot25_body))
    with pytest.raises(AssertionError):
        _assert_provider_key(mutated_key)
