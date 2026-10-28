"""Independent PE/COFF controls for a bit-guarded frame cleanup.

All code/data below belongs to the synthetic owner fixture. No game bytes,
addresses or owner names enter these controls.
"""

import struct

import pytest

from match_recovery.pe_coff import Coff
from match_recovery.source.eh_actions import EHFrameGuardFact
from match_recovery.source.eh_regions import _cleanup_action, _decoded, bind_eh_regions
import test_eh_vector_regions as fixture


def _guarded_fixture(monkeypatch, *, width=14, short_branch=False, extra_relocation=False):
    tail = fixture._tail_action

    def action(value):
        if not isinstance(value, tuple):
            return tail(value)
        code, operand = tail(value[1])
        prefix = b"\x8b\x45\xe0\x83\xe0\x01\x85\xc0"
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


@pytest.mark.parametrize("width", [8, 11, 14])
@pytest.mark.parametrize("short_branch", [False, True])
def test_guarded_actions_bind_complete_map_and_compare(monkeypatch, tmp_path,
                                                      width, short_branch):
    built = _guarded_fixture(monkeypatch, width=width, short_branch=short_branch)
    _obj, regions, result = fixture._measure(built, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    ownership = regions[0].ownership
    assert len(ownership.original_actions) == len(ownership.compiled_actions) == 3
    original, compiled = ownership.original_actions[0], ownership.compiled_actions[0]
    assert original.guard == compiled.guard == EHFrameGuardFact(-32, 1, "eax")
    assert original.frame_displacement == compiled.frame_displacement == -16
    assert original.transfers[0].operand_offset == compiled.transfers[0].operand_offset
    assert all(action.guard is None for action in ownership.original_actions[1:])
    assert all(action.guard is None for action in ownership.compiled_actions[1:])


@pytest.mark.parametrize(("offset", "value"), [
    (1, 0x4D),       # Flag loaded into ECX instead of EAX.
    (2, 0),          # Return-address/frame word is not a supported flag slot.
    (2, 0xE1),       # Unaligned flag word.
    (4, 0xE8),       # SUB instead of the bounded AND.
    (5, 2),          # A different flag bit is outside this grammar.
    (7, 0xC9),       # TEST ECX, ECX does not test the masked EAX.
    (9, 0x85),       # JNE must not invert the return condition.
    (10, 13),        # Branch into the tail jump rather than the final RET.
    (10, 15),        # Branch outside the complete action.
    (23, 0xE8),      # CALL + RET is not an unconditional destructor tail transfer.
    (28, 0x90),      # No plain final return.
])
def test_damaged_guarded_object_refuses(monkeypatch, offset, value):
    built = _object_damage(_guarded_fixture(monkeypatch), offset, value)
    raw, image, spans, *_ = built
    with pytest.raises(ValueError):
        bind_eh_regions(raw, fixture.ROOT_NAME, image, spans)


def test_valid_changed_guard_slot_is_mismatch_not_a_relocation_waiver(monkeypatch,
                                                                    tmp_path):
    built = _object_damage(_guarded_fixture(monkeypatch), 2, 0xE4)
    _obj, regions, result = fixture._measure(built, tmp_path)
    assert regions[0].ownership.original_actions[0].guard.frame_displacement == -32
    assert regions[0].ownership.compiled_actions[0].guard.frame_displacement == -28
    assert result.verdict.value == "MISMATCH", result
    assert "frame guard" in result.diagnostic


def test_unaccounted_relocation_in_guard_prefix_refuses(monkeypatch):
    raw, image, spans, *_ = _guarded_fixture(monkeypatch, extra_relocation=True)
    with pytest.raises(ValueError, match="missing or unaccounted relocations"):
        bind_eh_regions(raw, fixture.ROOT_NAME, image, spans)


def test_action_boundaries_and_nonzero_base_remain_exact():
    # A fixture-owned conditional cleanup, with no receiver adjustment.
    code = bytes.fromhex("8b45e083e00185c074088b4df0e900000000c3")
    action = _cleanup_action(_decoded(code, 0x120), code_base=0x100)
    assert (action.start, action.end) == (0x20, 0x20 + len(code))
    assert action.transfers[0].operand_offset == 0x20 + 14
    for altered in (code[:-1], code + b"\x90", code[:10] + b"\x90" + code[10:]):
        with pytest.raises(ValueError):
            _cleanup_action(_decoded(altered, 0x120), code_base=0x100)
