"""Serialized PE/COFF controls for the bounded MOV/SUB EH action form."""

import hashlib
import struct

import pytest

from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source.asm import compare_subject, subject_asm
from match_recovery.source.eh_actions import EHActionKind
from match_recovery.source.eh_regions import (
    _cleanup_action,
    _decoded,
    bind_eh_regions,
)
from match_recovery.source.reader import read_text
from test_eh_regions import (
    BASE,
    ROOT,
    SPANS,
    _EHFixtureSection,
    _coff,
    _nested_local_owner_fixture,
    _pe,
)

COLD_VA = BASE + 0x1100
PE_TEXT_RAW = 0x400
PE_COLD_RAW = PE_TEXT_RAW + 0x100


def _mov_sub_action(code, *, frame_disp=-16, operation="sub8", immediate=12):
    """Replace the fixture's 6-byte LEA plus 5-byte JMP with an 11-byte action."""
    original = bytes(code[PE_COLD_RAW:PE_COLD_RAW + 11])
    assert original[6] == 0xE9
    if operation == "sub8":
        arithmetic = bytes((0x83, 0xE9, immediate & 0xFF))
    elif operation == "add8":
        arithmetic = bytes((0x83, 0xC1, immediate & 0xFF))
    elif operation == "wrong_sub_register":
        arithmetic = bytes((0x83, 0xEB, immediate & 0xFF))
    elif operation == "wrong_opcode":
        arithmetic = bytes((0x33, 0xC9, 0x90))
    else:
        raise ValueError("unknown 11-byte arithmetic fixture")
    action = bytes((0x8B, 0x4D, frame_disp & 0xFF)) + arithmetic + b"\xE9" + original[7:11]
    assert len(action) == 11
    result = bytearray(code)
    result[PE_COLD_RAW:PE_COLD_RAW + 11] = action
    return bytes(result)


def _patch_object_action(code, *, frame_disp=-16, operation="sub8", immediate=12):
    result = bytearray(code)
    section_table = 20
    child_header = section_table + 40
    child_raw = struct.unpack_from("<I", result, child_header + 20)[0]
    assert result[child_raw + 6] == 0xE9
    if operation == "sub8":
        arithmetic = bytes((0x83, 0xE9, immediate & 0xFF))
    elif operation == "add8":
        arithmetic = bytes((0x83, 0xC1, immediate & 0xFF))
    elif operation == "wrong_sub_register":
        arithmetic = bytes((0x83, 0xEB, immediate & 0xFF))
    elif operation == "wrong_opcode":
        arithmetic = bytes((0x33, 0xC9, 0x90))
    else:
        raise ValueError("unknown 11-byte arithmetic fixture")
    action = bytes((0x8B, 0x4D, frame_disp & 0xFF)) + arithmetic + b"\xE9" + bytes(
        result[child_raw + 7:child_raw + 11])
    assert len(action) == 11
    result[child_raw:child_raw + 11] = action
    return bytes(result)


def _make_fixture(*, original_frame=-16, original_operation="sub8",
                  original_immediate=12, object_frame=-16,
                  object_operation="sub8", object_immediate=12,
                  image_damage=None, object_damage=None):
    image_bytes = _mov_sub_action(
        _pe(damage=image_damage), frame_disp=original_frame,
        operation=original_operation, immediate=original_immediate)
    object_bytes = _patch_object_action(
        _coff(damage=object_damage), frame_disp=object_frame,
        operation=object_operation, immediate=object_immediate)
    if object_damage == "nonzero_relative_addend":
        object_bytes = bytearray(object_bytes)
        child_header = 20 + 40
        child_raw = struct.unpack_from("<I", object_bytes, child_header + 20)[0]
        struct.pack_into("<I", object_bytes, child_raw + 7, 1)
        object_bytes = bytes(object_bytes)
    return image_bytes, object_bytes


def _record(spans=SPANS):
    root_start, root_end = spans[0]
    cold_start, cold_end = spans[1]
    text = (f"// ORIGINAL: 0x{root_start:08X} test-owned "
            f"0x{root_start:08X}-0x{root_end:08X};"
            f"0x{cold_start:08X}-0x{cold_end:08X} FILE\n"
            f"// symbol {ROOT}\nint recover() {{ return 0; }}\n")
    record, = read_text(text, "synthetic_eh_sub.cpp")
    return record


def _bind(image_bytes, object_bytes, spans=SPANS):
    image = PeImage(image_bytes, hashlib.sha256(image_bytes).hexdigest())
    return bind_eh_regions(object_bytes, ROOT, image, spans)


def _compare(image_bytes, object_bytes, tmp_path, spans=SPANS):
    image_path = tmp_path / "synthetic-sub-eh.exe"
    image_path.write_bytes(image_bytes)
    record = _record(spans)
    candidate = subject_asm(object_bytes, record, "serialized-sub-eh")
    return compare_subject(record, image_path, candidate)


def _wide_sub_fixture():
    """Use a two-action owner to serialize the supported imm32 SUB form."""
    spans = (SPANS[0], (SPANS[1][0], SPANS[1][0] + 32))
    image = bytearray(_pe(cold_size=32, mode="lea_pair"))
    image_stub = bytes(image[PE_COLD_RAW + 22:PE_COLD_RAW + 32])
    first = (b"\x8b\x4d\xf0\x81\xe9\x0c\0\0\0\xe9"
             + struct.pack("<i", (BASE + 0x11F0) - (COLD_VA + 14)))
    second = (b"\x8d\x4d\xf0\xe9"
              + struct.pack("<i", (BASE + 0x11E0) - (COLD_VA + 22)))
    image[PE_COLD_RAW:PE_COLD_RAW + 32] = first + second + image_stub
    struct.pack_into("<I", image, 0x600 + 44, COLD_VA + 14)

    object_bytes = bytearray(_coff(cold_size=32, mode="lea_pair"))
    child_header = 20 + 40
    child_raw = struct.unpack_from("<I", object_bytes, child_header + 20)[0]
    object_stub = bytes(object_bytes[child_raw + 22:child_raw + 32])
    object_first = b"\x8b\x4d\xf0\x81\xe9\x0c\0\0\0\xe9\0\0\0\0"
    object_second = b"\x8d\x4d\xf0\xe9\0\0\0\0"
    object_bytes[child_raw:child_raw + 32] = object_first + object_second + object_stub
    reloc_pointer = struct.unpack_from("<I", object_bytes, child_header + 24)[0]
    assert struct.unpack_from("<I", object_bytes, reloc_pointer)[0] == 7
    struct.pack_into("<I", object_bytes, reloc_pointer, 10)
    parsed = Coff(bytes(object_bytes))
    cleanup2 = next(symbol for symbol in parsed.symbols.values()
                    if symbol.name == "$Lcleanup2")
    symbol_table = struct.unpack_from("<I", object_bytes, 8)[0]
    struct.pack_into("<I", object_bytes, symbol_table + cleanup2.index * 18 + 8, 14)
    return bytes(image), bytes(object_bytes), spans


def _wrong_relocation_kind(object_bytes):
    result = bytearray(object_bytes)
    child_header = 20 + 40
    relocation_pointer = struct.unpack_from("<I", result, child_header + 24)[0]
    # The first child relocation is the cleanup branch operand at +7.
    assert struct.unpack_from("<I", result, relocation_pointer)[0] == 7
    struct.pack_into("<H", result, relocation_pointer + 8, 6)
    return bytes(result)


def _move_cleanup_relocation(object_bytes):
    result = bytearray(object_bytes)
    child_header = 20 + 40
    relocation_pointer = struct.unpack_from("<I", result, child_header + 24)[0]
    assert struct.unpack_from("<I", result, relocation_pointer)[0] == 7
    struct.pack_into("<I", result, relocation_pointer, 6)
    return bytes(result)


def _overlap_cleanup_relocations(object_bytes):
    result = bytearray(_coff(damage="extra_child_relocation"))
    result = bytearray(_patch_object_action(result))
    child_header = 20 + 40
    relocation_pointer = struct.unpack_from("<I", result, child_header + 24)[0]
    relocation_count = struct.unpack_from("<H", result, child_header + 32)[0]
    # Fixture's extra edge is at +1; point it into the cleanup operand at +7.
    extra = relocation_pointer + (relocation_count - 1) * 10
    struct.pack_into("<I", result, extra, 7)
    return bytes(result)


def _invalid_original_target(image_bytes):
    result = bytearray(image_bytes)
    # JMP starts at cold+6; its next instruction is cold+11. Point into .rdata.
    struct.pack_into("<i", result, PE_COLD_RAW + 7,
                     (BASE + 0x2000) - (COLD_VA + 11))
    return bytes(result)


def _gap_original_action(image_bytes):
    result = bytearray(image_bytes)
    # PE raw .rdata begins at 0x600; FuncInfo unwind entry starts at +36.
    struct.pack_into("<I", result, 0x600 + 36, COLD_VA + 1)
    return bytes(result)


def test_serialized_mov_sub_action_binds_original_and_coff_and_compares(tmp_path):
    image_bytes, object_bytes = _make_fixture()
    root, cold = _bind(image_bytes, object_bytes)
    assert root.ownership.cleanup_offsets == (0,)
    assert cold.ownership.original_cleanups == (SPANS[1][0],)
    assert cold.ownership.original_actions[0].kind is EHActionKind.FRAME_TRANSFER
    assert cold.ownership.compiled_actions[0].kind is EHActionKind.FRAME_TRANSFER
    assert (cold.ownership.original_actions[0].frame_displacement,
            cold.ownership.original_actions[0].base_adjustment,
            cold.ownership.original_actions[0].scratch_register) == (-16, -12, "ecx")
    assert (cold.ownership.compiled_actions[0].frame_displacement,
            cold.ownership.compiled_actions[0].base_adjustment,
            cold.ownership.compiled_actions[0].scratch_register) == (-16, -12, "ecx")
    assert (cold.ownership.original_actions[0].start,
            cold.ownership.original_actions[0].end) == (0, 11)
    assert (cold.ownership.compiled_actions[0].start,
            cold.ownership.compiled_actions[0].end) == (0, 11)
    assert len(Coff(object_bytes).code_sections(Coff(object_bytes).function(ROOT))) == 2
    result = _compare(image_bytes, object_bytes, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result


def test_serialized_wide_sub_and_later_action_keep_independent_map_boundaries(tmp_path):
    image_bytes, object_bytes, spans = _wide_sub_fixture()
    root, cold = _bind(image_bytes, object_bytes, spans)
    assert root.ownership.cleanup_offsets == (0, 14)
    assert cold.ownership.original_cleanups == (spans[1][0], spans[1][0] + 14)
    assert [action.end - action.start for action in cold.ownership.original_actions] == [14, 8]
    assert [action.end - action.start for action in cold.ownership.compiled_actions] == [14, 8]
    assert cold.ownership.original_actions[0].base_adjustment == -12
    assert cold.ownership.compiled_actions[0].base_adjustment == -12
    result = _compare(image_bytes, object_bytes, tmp_path, spans)
    assert result.verdict.value == "BYTE_EXACT", result


@pytest.mark.parametrize(("field", "value"), [
    ("frame", -20), ("subconstant", 13),
])
def test_valid_object_mov_sub_operand_changes_are_mismatches(field, value, tmp_path):
    options = {"object_frame": -16, "object_immediate": 12}
    options["object_frame" if field == "frame" else "object_immediate"] = value
    image_bytes, object_bytes = _make_fixture(**options)
    _bind(image_bytes, object_bytes)
    result = _compare(image_bytes, object_bytes, tmp_path)
    assert result.verdict.value == "MISMATCH", result


def test_valid_add_action_binds_but_does_not_match_original_sub(tmp_path):
    image_bytes, object_bytes = _make_fixture(object_operation="add8",
                                               object_immediate=(-12) & 0xFF)
    _bind(image_bytes, object_bytes)
    result = _compare(image_bytes, object_bytes, tmp_path)
    assert result.verdict.value == "MISMATCH", result


@pytest.mark.parametrize("side", ["original", "compiled"])
@pytest.mark.parametrize("operation", ["wrong_sub_register", "wrong_opcode"])
def test_wrong_mov_sub_operands_refuse(side, operation):
    image_bytes, object_bytes = _make_fixture(
        original_operation=operation if side == "original" else "sub8",
        object_operation=operation if side == "compiled" else "sub8")
    with pytest.raises(ValueError):
        _bind(image_bytes, object_bytes)


@pytest.mark.parametrize("action", [
    # Address-size/operand-size prefixes and MOV disp32 are outside the exact
    # bounded encoding family, even when a decoder can parse the instructions.
    b"\x66\x8b\x4d\xf0\x83\xe9\x0c\xe9\0\0\0\0",
    b"\x8b\x8d\xf0\xff\xff\xff\x83\xe9\x0c\xe9\0\0\0\0",
    b"\x67\x8b\x4d\xf0\x83\xe9\x0c\xe9\0\0\0\0",
    # A call or short branch is not the required complete rel32 JMP transfer.
    b"\x8b\x4d\xf0\x83\xe9\x0c\xe8\0\0\0\0",
    b"\x8b\x4d\xf0\x83\xe9\x0c\xeb\0",
    # The action extent must end exactly at the transfer, not absorb padding.
    b"\x8b\x4d\xf0\x83\xe9\x0c\xe9\0\0\0\0\x90",
])
def test_frame_action_rejects_prefix_width_branch_and_trailing_variants(action):
    with pytest.raises(ValueError):
        _cleanup_action(_decoded(action, 0), code_base=0)


@pytest.mark.parametrize("damage", [
    "missing_destructor_edge", "extra_child_relocation", "nonzero_relative_addend",
    "bad_destructor", "wrong_cleanup_edge",
])
def test_existing_coff_ownership_damage_controls_still_refuse(damage):
    image_bytes, object_bytes = _make_fixture(object_damage=damage)
    with pytest.raises(ValueError):
        _bind(image_bytes, object_bytes)


def test_moved_and_wrong_kind_cleanup_relocations_refuse():
    image_bytes, object_bytes = _make_fixture()
    for damaged in (_move_cleanup_relocation(object_bytes),
                    _wrong_relocation_kind(object_bytes)):
        with pytest.raises(ValueError):
            _bind(image_bytes, damaged)


def test_overlapping_cleanup_relocation_refuses():
    image_bytes, object_bytes = _make_fixture()
    damaged = _overlap_cleanup_relocations(object_bytes)
    with pytest.raises(ValueError):
        _bind(image_bytes, damaged)


def test_original_gap_and_nonexecutable_cleanup_target_refuse():
    _image_bytes, object_bytes = _make_fixture()
    for damaged in (_gap_original_action(_make_fixture()[0]),
                    _invalid_original_target(_make_fixture()[0])):
        with pytest.raises(ValueError):
            _bind(damaged, object_bytes)


def test_local_complete_destructor_closure_remains_supported():
    image_bytes, object_bytes = _make_fixture(object_damage="first_local_defined")
    root, cold = _bind(image_bytes, object_bytes)
    assert root.ownership.cleanup_offsets == (0,)
    assert cold.ownership.compiled_actions[0].transfers[0].target == "??1Time@@QAE@XZ"


def test_local_nested_destructor_closure_accepts_mov_sub_action():
    from match_recovery.source import eh_regions

    obj = _nested_local_owner_fixture()
    child = obj.sections[1]
    # Keep the child section, its rel32 relocation at +7, handler, FuncInfo,
    # and associations intact; only replace its existing 11-byte cleanup.
    action = b"\x8b\x4d\xf0\x83\xe9\x0c\xe9" + child.data[7:11]
    obj.sections = (obj.sections[0],
                    _EHFixtureSection(action + child.data[11:],
                                      child.relocations, child.flags, child.name),
                    *obj.sections[2:])
    owner = obj.function("??1NestedOwner@@QAE@XZ")
    walk = eh_regions._LocalEHWalk(active={10}, complete=set(), roots=1)
    eh_regions._validate_local_complete_destructor(obj, owner, 10, 11, walk, 1)
