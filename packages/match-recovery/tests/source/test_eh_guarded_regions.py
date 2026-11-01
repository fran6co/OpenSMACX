"""Independent PE/COFF controls for a bit-guarded frame cleanup.

All code/data below belongs to the synthetic owner fixture. No game bytes,
addresses or owner names enter these controls.
"""

import hashlib
import struct

import pytest

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Result, Span
from match_recovery.pe_coff import Coff
from match_recovery.source.eh_actions import EHFrameGuardFact
from match_recovery.source.eh_regions import (_cleanup_action, _decoded, bind_eh_regions,
                                              eh_funcinfo_reference_target)
import test_eh_vector_regions as fixture


def _guarded_fixture(monkeypatch, *, width=14, short_branch=False, mask=1,
                     extra_relocation=False):
    tail = fixture._tail_action

    def action(value):
        if not isinstance(value, tuple):
            return tail(value)
        code, operand = tail(value[1])
        prefix = b"\x8b\x45\xe0\x83\xe0" + bytes([mask]) + b"\x85\xc0"
        branch = ((b"\x74" + bytes([len(code)])) if short_branch else
                  b"\x0f\x84" + struct.pack("<i", len(code)))
        return prefix + branch + code + b"\xc3", len(prefix + branch) + operand

    monkeypatch.setattr(fixture, "_tail_action", action)
    return fixture._build([("tail", ("guarded", width)), ("tail", 8), ("array", {})],
                          object_reloc_damage="extra_relocation" if extra_relocation else None)


def _object_damage(built, offset, value):
    raw, image, spans, record, original, compiled = built
    obj = Coff(raw)
    child = obj.code_sections(obj.function(fixture.ROOT_NAME))[1]
    pointer, = struct.unpack_from("<I", raw, 20 + (child - 1) * 40 + 20)
    damaged = bytearray(raw)
    damaged[pointer + offset] = value
    return bytes(damaged), image, spans, record, original, compiled


@pytest.mark.parametrize("mask", [1, 2])
@pytest.mark.parametrize("width", [8, 11, 14])
@pytest.mark.parametrize("short_branch", [False, True])
def test_guarded_actions_bind_complete_map_and_compare(monkeypatch, tmp_path,
                                                      width, short_branch, mask):
    built = _guarded_fixture(monkeypatch, width=width, short_branch=short_branch,
                             mask=mask)
    _obj, regions, result = fixture._measure(built, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    ownership = regions[0].ownership
    assert len(ownership.original_actions) == len(ownership.compiled_actions) == 3
    original, compiled = ownership.original_actions[0], ownership.compiled_actions[0]
    assert original.guard == compiled.guard == EHFrameGuardFact(-32, mask, "eax")
    assert original.frame_displacement == compiled.frame_displacement == -16
    assert original.transfers[0].operand_offset == compiled.transfers[0].operand_offset
    assert all(action.guard is None for action in ownership.original_actions[1:])
    assert all(action.guard is None for action in ownership.compiled_actions[1:])


@pytest.mark.parametrize("mask", [1, 2])
@pytest.mark.parametrize(("offset", "value"), [
    (1, 0x4D),       # Flag loaded into ECX instead of EAX.
    (2, 0),          # Return-address/frame word is not a supported flag slot.
    (2, 0xE1),       # Unaligned flag word.
    (4, 0xE8),       # SUB instead of the bounded AND.
    (5, 0),          # A zero mask cannot guard a construction cleanup.
    (5, 3),          # Composite masks are outside the observed grammar.
    (5, 4),          # Other single-bit masks still lack evidence.
    (5, 0xFF),       # Sign-extended masks are not one of the observed tests.
    (7, 0xC9),       # TEST ECX, ECX does not test the masked EAX.
    (9, 0x85),       # JNE must not invert the return condition.
    (10, 13),        # Branch into the tail jump rather than the final RET.
    (10, 15),        # Branch outside the complete action.
    (23, 0xE8),      # CALL + RET is not an unconditional destructor tail transfer.
    (28, 0x90),      # No plain final return.
])
def test_damaged_guarded_object_refuses(monkeypatch, offset, value, mask):
    built = _object_damage(_guarded_fixture(monkeypatch, mask=mask), offset, value)
    raw, image, spans, *_ = built
    with pytest.raises(ValueError):
        bind_eh_regions(raw, fixture.ROOT_NAME, image, spans)


@pytest.mark.parametrize("mask", [1, 2])
def test_valid_changed_guard_slot_is_mismatch_not_a_relocation_waiver(monkeypatch,
                                                                    tmp_path, mask):
    built = _object_damage(_guarded_fixture(monkeypatch, mask=mask), 2, 0xE4)
    _obj, regions, result = fixture._measure(built, tmp_path)
    assert regions[0].ownership.original_actions[0].guard.frame_displacement == -32
    assert regions[0].ownership.compiled_actions[0].guard.frame_displacement == -28
    assert result.verdict.value == "MISMATCH", result
    assert "frame guard" in result.diagnostic


@pytest.mark.parametrize("mask", [1, 2])
def test_unaccounted_relocation_in_guard_prefix_refuses(monkeypatch, mask):
    raw, image, spans, *_ = _guarded_fixture(monkeypatch, mask=mask, extra_relocation=True)
    with pytest.raises(ValueError, match="missing or unaccounted relocations"):
        bind_eh_regions(raw, fixture.ROOT_NAME, image, spans)


@pytest.mark.parametrize("mask", [1, 2])
def test_action_boundaries_and_nonzero_base_remain_exact(mask):
    # A fixture-owned conditional cleanup, with no receiver adjustment.
    code = bytes.fromhex("8b45e083e0") + bytes([mask]) + bytes.fromhex("85c074088b4df0e900000000c3")
    action = _cleanup_action(_decoded(code, 0x120), code_base=0x100)
    assert (action.start, action.end) == (0x20, 0x20 + len(code))
    assert action.transfers[0].operand_offset == 0x20 + 14
    for altered in (code[:-1], code + b"\x90", code[:10] + b"\x90" + code[10:]):
        with pytest.raises(ValueError):
            _cleanup_action(_decoded(altered, 0x120), code_base=0x100)


def _indexed_targets(obj):
    targets = {
        "??1TailElement@@QAE@XZ": fixture.TAIL_VA,
        "??1Element@@QAE@XZ": fixture.CALLBACK_VA,
        "??_M@YGXPAXIHP6EX0@Z@Z": fixture.HELPER_VA,
        "___CxxFrameHandler": fixture.RUNTIME_VA,
    }
    return {str(index): targets[symbol.name] for index, symbol in obj.symbols.items()
            if symbol.name in targets}


@pytest.mark.parametrize(("original_mask", "compiled_mask"), [(1, 2), (2, 1)])
@pytest.mark.parametrize("short_branch", [False, True])
def test_changed_valid_guard_mask_is_semantic_mismatch_and_cannot_prove_funcinfo(
        monkeypatch, tmp_path, original_mask, compiled_mask, short_branch):
    built = _object_damage(_guarded_fixture(monkeypatch, mask=original_mask,
                                           short_branch=short_branch), 5, compiled_mask)
    obj, regions, result = fixture._measure(built, tmp_path)
    original = regions[0].ownership.original_actions[0]
    compiled = regions[0].ownership.compiled_actions[0]
    assert original.guard == EHFrameGuardFact(-32, original_mask, "eax")
    assert compiled.guard == EHFrameGuardFact(-32, compiled_mask, "eax")
    assert result.verdict.value == "MISMATCH", result
    assert "frame guard" in result.diagnostic
    raw, image, spans, *_ = built
    with pytest.raises(ValueError, match="frame guard"):
        eh_funcinfo_reference_target(raw, fixture.ROOT_NAME, image, spans,
                                    _indexed_targets(obj))


def test_second_bit_guard_proves_only_owned_funcinfo_with_resolved_transfers(monkeypatch):
    raw, image, spans, *_ = _guarded_fixture(monkeypatch, mask=2)
    obj = Coff(raw)
    indexed_targets = _indexed_targets(obj)
    index, address = eh_funcinfo_reference_target(
        raw, fixture.ROOT_NAME, image, spans, indexed_targets)
    assert obj.symbols[index].name == "$Tinfo"
    assert address == fixture.RDATA_VA

    binding = FunctionBinding(Origin(hashlib.sha256(image.data).hexdigest(), "pe:va", fixture.ROOT_SPAN[0]),
                              tuple(Span(start, end) for start, end in spans),
                              fixture.ROOT_NAME, "synthetic_guarded_eh.cpp")
    external_targets = {obj.symbols[int(index)].name: target
                        for index, target in indexed_targets.items()}
    row, = check_objects((binding,), {binding.unit: obj}, image,
                         external_targets=external_targets)
    assert row["result"] == Result.PASS
    info, = (item for item in row["diagnostic"]["emitted_references"]
             if item["symbol"] == "$Tinfo")
    assert info["binding_address"] == f"0x{fixture.RDATA_VA:08X}"


@pytest.mark.parametrize(("symbol", "reason"), [
    ("??1TailElement@@QAE@XZ", "cleanup transfer"),
    ("___CxxFrameHandler", "handler runtime"),
])
def test_second_bit_guard_does_not_waive_wrong_transfer_target(monkeypatch, symbol, reason):
    raw, image, spans, *_ = _guarded_fixture(monkeypatch, mask=2)
    obj = Coff(raw)
    targets = _indexed_targets(obj)
    index, = (str(index) for index, item in obj.symbols.items() if item.name == symbol)
    targets[index] += 4
    with pytest.raises(ValueError, match=reason):
        eh_funcinfo_reference_target(raw, fixture.ROOT_NAME, image, spans, targets)


@pytest.mark.parametrize("predecessor", [0, 2])
def test_second_bit_guard_does_not_waive_changed_or_invalid_predecessor(
        monkeypatch, predecessor):
    raw, image, spans, *_ = _guarded_fixture(monkeypatch, mask=2)
    obj = Coff(raw)
    info, = (symbol for symbol in obj.symbols.values() if symbol.name == "$Tinfo")
    pointer, = struct.unpack_from("<I", raw, 20 + (info.section - 1) * 40 + 20)
    damaged = bytearray(raw)
    # The third state's original predecessor is one. Zero remains a valid
    # backward edge, while two is an invalid self edge; neither proves equality.
    struct.pack_into("<I", damaged, pointer + info.value + 32 + 2 * 8, predecessor)
    reason = "changed predecessor topology" if predecessor == 0 else "predecessor"
    with pytest.raises(ValueError, match=reason):
        eh_funcinfo_reference_target(bytes(damaged), fixture.ROOT_NAME, image, spans,
                                    _indexed_targets(obj))
