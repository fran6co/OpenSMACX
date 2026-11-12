"""Synthetic controls for the per-callee register-preservation proof."""

import hashlib
import struct

import pytest

from match_recovery.associated_binary import _ENGINE, _path_bounded_dispatch
from match_recovery.callee_preservation import _STACK_PROBE, callee_preserves
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage


BASE = 0x401000
SECTION = 0x800


def _image(placed: dict[int, bytes], relocations=()) -> PeImage:
    """One executable .text section at BASE holding code placed at offsets."""
    text = bytearray(b"\xcc" * SECTION)
    for offset, code in placed.items():
        text[offset:offset + len(code)] = code
    raw = bytearray(0x400)
    raw[:2] = b"MZ"
    struct.pack_into("<I", raw, 0x3C, 0x80)
    raw[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", raw, 0x84, 0x14C, 1, 0, 0, 0, 224, 0)
    struct.pack_into("<H", raw, 0x98, 0x10B)
    struct.pack_into("<I", raw, 0x98 + 28, 0x400000)
    struct.pack_into("<I", raw, 0x98 + 92, 16)
    raw[0x178:0x180] = b".text\0\0\0"
    struct.pack_into("<IIII", raw, 0x180, SECTION, 0x1000, SECTION, 0x400)
    struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)
    raw += text
    if relocations:
        entries = [0x3000 + (site - BASE) for site in relocations]
        if len(entries) % 2:
            entries.append(0)
        block = struct.pack("<II", 0x1000, 8 + 2 * len(entries))
        block += b"".join(struct.pack("<H", entry) for entry in entries)
        struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1000 + SECTION, len(block))
        struct.pack_into("<I", raw, 0x180, SECTION + len(block))
        raw += block
        struct.pack_into("<I", raw, 0x180 + 8, SECTION + len(block))
    data = bytes(raw)
    return PeImage(data, hashlib.sha256(data).hexdigest())


def _call(source: int, target: int) -> bytes:
    return b"\xe8" + struct.pack("<i", target - (source + 5))


def _proves(placed, family="ebx", entry=0x100, relocations=()) -> None:
    callee_preserves(_image(placed, relocations), BASE + entry, family)


# mov eax,[esp+4]; add eax,1; ret
NEVER_WRITES = bytes.fromhex("8b 44 24 04 83 c0 01 c3")
# push ebx; mov ebx,5; pop ebx; ret
PUSH_POP = bytes.fromhex("53 bb 05 00 00 00 5b c3")
# push ebp; mov ebp,esp; sub esp,8; push ebx; xor ebx,ebx; mov [ebp-4],ebx;
# pop ebx; mov esp,ebp; pop ebp; ret 4
FRAMED_PUSH_POP = bytes.fromhex("55 8b ec 83 ec 08 53 31 db 89 5d fc 5b 8b e5 5d c2 04 00")
CLOBBERS = bytes.fromhex("bb 01 00 00 00 c3")  # mov ebx,1; ret


@pytest.mark.parametrize("code", [NEVER_WRITES, PUSH_POP, FRAMED_PUSH_POP])
def test_callee_that_never_writes_or_saves_and_restores_preserves(code):
    _proves({0x100: code})


def test_never_written_register_survives_a_clobber_of_another_register():
    _proves({0x100: CLOBBERS}, family="esi")


@pytest.mark.parametrize("code,reason", [
    (CLOBBERS, "overwrites ebx"),
    # Restored from the wrong slot: push ebx; xor ebx,ebx; push eax; pop ebx; ret.
    (bytes.fromhex("53 31 db 50 5b c3"), "without restoring"),
    # The save slot is released before the restore: push ebx; xor ebx,ebx; add esp,4; ret.
    (bytes.fromhex("53 31 db 83 c4 04 c3"), "releases its saved ebx slot"),
    # A fixed frame store overwrites the save slot: push ebx; xor ebx,ebx; mov [esp],eax; pop ebx; ret.
    (bytes.fromhex("53 31 db 89 04 24 5b c3"), "overwrites the save slot"),
    # Indirect transfers while ebx still holds the caller's value.
    (bytes.fromhex("ff d0 c3"), "unprovable call"),
    (bytes.fromhex("ff e0"), "unbounded indirect jump"),
    # An indirect call under a live save leaves the restoring POP's depth
    # unknown: push ebx; xor ebx,ebx; push 1; call [eax]; pop ebx; ret.
    (bytes.fromhex("53 31 db 6a 01 ff 10 5b c3"), "unknown stack depth"),
    # No reachable return at all: jmp $.
    (bytes.fromhex("eb fe"), "no reachable return"),
    (bytes.fromhex("cd 03 c3"), "unsupported transfer"),
])
def test_clobbering_unbalanced_or_indirect_callees_refuse(code, reason):
    with pytest.raises(ValueError, match=reason):
        _proves({0x100: code})


def test_nested_direct_calls_must_themselves_preserve_the_unsaved_register():
    # 0x100: call 0x180; ret
    caller = _call(BASE + 0x100, BASE + 0x180) + b"\xc3"
    _proves({0x100: caller, 0x180: NEVER_WRITES})
    with pytest.raises(ValueError, match="0x401180 overwrites ebx"):
        _proves({0x100: caller, 0x180: CLOBBERS})
    # Under a live save a clobbering nested callee is restored by the POP:
    # push ebx; call 0x180; pop ebx; ret.
    saved = b"\x53" + _call(BASE + 0x101, BASE + 0x180) + b"\x5b\xc3"
    _proves({0x100: saved, 0x180: CLOBBERS})


def test_recursive_callee_refuses():
    recursive = bytes.fromhex("85 c0 74 05") + _call(BASE + 0x104, BASE + 0x100) + b"\xc3"
    with pytest.raises(ValueError, match="recursive"):
        _proves({0x100: recursive})


def test_nested_stdcall_effect_is_tracked_to_the_restore():
    # push ebx; xor ebx,ebx; push 1; call 0x180 (ret 4); pop ebx; ret
    caller = bytes.fromhex("53 31 db 6a 01") + _call(BASE + 0x105, BASE + 0x180) + b"\x5b\xc3"
    _proves({0x100: caller, 0x180: bytes.fromhex("8b 44 24 04 c2 04 00")})
    # The same nested callee declared as `ret 8` misplaces the restore.
    with pytest.raises(ValueError, match="releases|without restoring"):
        _proves({0x100: caller, 0x180: bytes.fromhex("8b 44 24 04 c2 08 00")})


def test_bounded_jump_table_is_followed_and_unbounded_refuses():
    # cmp ecx,1; ja L; jmp [ecx*4+T]; L: ret; case0: ret; case1: mov ebx,1; ret
    table = BASE + 0x140
    code = bytes.fromhex("83 f9 01 77 07 ff 24 8d") + struct.pack("<I", table) + b"\xc3"
    case0, case1 = BASE + 0x100 + len(code), BASE + 0x100 + len(code) + 1
    placed = {0x100: code + b"\xc3" + CLOBBERS, 0x140: struct.pack("<II", case0, case0)}
    _proves(placed, relocations=(table, table + 4))
    clobbering = {0x100: placed[0x100], 0x140: struct.pack("<II", case0, case1)}
    with pytest.raises(ValueError, match="overwrites ebx"):
        _proves(clobbering, relocations=(table, table + 4))
    with pytest.raises(ValueError, match="HIGHLOW"):
        _proves(placed, relocations=(table,))
    unbounded = {0x100: bytes.fromhex("90 90 90 90 90") + code[5:] + b"\xc3", 0x140: placed[0x140]}
    with pytest.raises(ValueError, match="unbounded indirect jump"):
        _proves(unbounded, relocations=(table, table + 4))


def test_stack_probe_lemma_requires_exact_bytes_and_a_constant_size():
    # push ebp; mov ebp,esp; mov eax,2000h; call probe; push ebx; xor ebx,ebx;
    # mov [ebp-8],eax; pop ebx; mov esp,ebp; pop ebp; ret
    probe = BASE + 0x300
    head = bytes.fromhex("55 8b ec b8 00 20 00 00")
    code = (head + _call(BASE + 0x100 + len(head), probe)
            + bytes.fromhex("53 31 db 89 45 f8 5b 8b e5 5d c3"))
    _proves({0x100: code, 0x300: _STACK_PROBE})
    # The probed frame places the save slot below the 2000h-byte area, so a
    # store at the slot's real EBP offset is seen as an overwrite.
    overwrite = code[:-11] + bytes.fromhex("53 31 db 89 85 fc df ff ff 5b 8b e5 5d c3")
    with pytest.raises(ValueError, match="overwrites the save slot"):
        _proves({0x100: overwrite, 0x300: _STACK_PROBE})
    # A probe-like routine with different bytes has no admitted effect.
    altered = bytearray(_STACK_PROBE)
    altered[1:6] = bytes.fromhex("3d 00 20 00 00")
    with pytest.raises(ValueError, match="overwrites ebx|unknown stack depth"):
        _proves({0x100: code, 0x300: bytes(altered)})
    # Without a tracked EAX constant the probe's effect is unknown.
    no_size = code.replace(bytes.fromhex("b8 00 20 00 00"), bytes.fromhex("8b 45 08 90 90"))
    with pytest.raises(ValueError, match="overwrites ebx|unknown stack depth"):
        _proves({0x100: no_size, 0x300: _STACK_PROBE})


def test_unmodelled_registers_and_non_code_refuse():
    image = _image({0x100: NEVER_WRITES})
    with pytest.raises(ValueError, match="does not model"):
        callee_preserves(image, BASE + 0x100, "esp")
    with pytest.raises(ValueError, match="not file-backed"):
        callee_preserves(image, 0x10000, "ebx")


# map_menu shape: the dispatch is re-entered with an index held in EBX
# across a direct call.
#   mov ebx,1; L: cmp esi,4; mov eax,esi; ja DEF; D: jmp [eax*4+T]
#   case: push 1; call CALLEE; add esp,4; mov eax,ebx; jmp D; DEF: ret
def _dispatch_parent(prefix=b""):
    head = prefix + bytes.fromhex("bb 01 00 00 00 83 fe 04 8b c6 77 15 ff 24 85") + struct.pack("<I", BASE + 0x1F0)
    case = len(head)
    call_at = BASE + case + 2
    body = (head + bytes.fromhex("6a 01") + _call(call_at, BASE + 0x100)
            + bytes.fromhex("83 c4 04 8b c3 eb eb c3"))
    instructions = list(_ENGINE.disasm(body, BASE))
    assert sum(item.size for item in instructions) == len(body)
    dispatch = next(item for item in instructions if item.mnemonic == "jmp" and item.operands[0].type == 3)
    assert instructions[-2].mnemonic == "jmp" and instructions[-2].operands[0].imm == dispatch.address
    return body, instructions, dispatch, frozenset({BASE + case})


def test_map_menu_shaped_dispatch_is_bounded_through_a_preserving_callee():
    body, instructions, dispatch, targets = _dispatch_parent()
    span = Span(BASE, BASE + len(body))
    for callee in (NEVER_WRITES, PUSH_POP, FRAMED_PUSH_POP):
        image = _image({0: body, 0x100: callee})
        witness = _path_bounded_dispatch(instructions, dispatch, "eax", span, targets, image=image)
        assert witness["maximum_index"] == 4
        assert {(row["edge"], row["maximum"]) for row in witness["bounds"]} == {
            ("constant", 1), ("fact", 4)}
        assert witness["preserving_callees"] == [{"callee": BASE + 0x100, "register": "ebx"}]


@pytest.mark.parametrize("callee,reason", [
    (CLOBBERS, "overwrites ebx"),
    (bytes.fromhex("53 31 db 6a 01 ff 10 5b c3"), "unknown stack depth"),
    (bytes.fromhex("ff d0 c3"), "unprovable call"),
])
def test_map_menu_shaped_dispatch_refuses_unproved_callees(callee, reason):
    body, instructions, dispatch, targets = _dispatch_parent()
    span = Span(BASE, BASE + len(body))
    with pytest.raises(ValueError, match=reason):
        _path_bounded_dispatch(instructions, dispatch, "eax", span, targets,
                               image=_image({0: body, 0x100: callee}))
    # Without the pinned image no callee is trusted at all.
    with pytest.raises(ValueError, match="not bounded"):
        _path_bounded_dispatch(instructions, dispatch, "eax", span, targets)


def test_eh_registration_moves_are_admitted_only_for_a_proved_cleanup_frame():
    # mov eax,fs:[0]; push eax; mov fs:[0],esp; <map_menu shape>
    registration = bytes.fromhex("64 a1 00 00 00 00 50 64 89 25 00 00 00 00")
    body, instructions, dispatch, targets = _dispatch_parent(prefix=registration)
    span = Span(BASE, BASE + len(body))
    image = _image({0: body, 0x100: PUSH_POP})
    with pytest.raises(ValueError, match="segment-relative"):
        _path_bounded_dispatch(instructions, dispatch, "eax", span, targets, image=image)
    witness = _path_bounded_dispatch(instructions, dispatch, "eax", span, targets,
                                     image=image, eh_frame=True)
    assert witness["maximum_index"] == 4
    # Any other segment-relative access still refuses: mov eax,fs:[4].
    other = bytes.fromhex("64 a1 04 00 00 00 50 64 89 25 00 00 00 00")
    body, instructions, dispatch, targets = _dispatch_parent(prefix=other)
    with pytest.raises(ValueError, match="segment-relative"):
        _path_bounded_dispatch(instructions, dispatch, "eax", Span(BASE, BASE + len(body)),
                               targets, image=_image({0: body, 0x100: PUSH_POP}), eh_frame=True)
