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


def _eh_framed_callee(arm: bytes) -> tuple[dict[int, bytes], tuple[int, ...]]:
    """custom_planet shape: VC6 EH frame, saved EBX, a bounded table, teardown.

    push ebp; mov ebp,esp; mov eax,fs:[0]; push -1; push HANDLER; push eax;
    mov fs:[0],esp; push ebx; xor ebx,ebx; cmp ecx,1; ja OUT;
    jmp [ecx*4+T]; A0: <arm>; jmp OUT; A1: inc ebx;
    OUT: pop ebx; mov ecx,[ebp-0Ch]; mov fs:[0],ecx; mov esp,ebp; pop ebp; ret
    The handler is only registered, never on the normal return path.
    """
    table = BASE + 0x1F0
    head = (bytes.fromhex("55 8b ec 64 a1 00 00 00 00 6a ff 68") + struct.pack("<I", BASE + 0x300)
            + bytes.fromhex("50 64 89 25 00 00 00 00 53 31 db 83 f9 01 77")
            + bytes([len(arm) + 2 + 7 + 1]) + bytes.fromhex("ff 24 8d") + struct.pack("<I", table))
    arm0 = BASE + 0x100 + len(head)
    body = head + arm + bytes([0xEB, 1]) + b"\x43"
    arm1 = BASE + 0x100 + len(body) - 1
    body += bytes.fromhex("5b 8b 4d f4 64 89 0d 00 00 00 00 8b e5 5d c3")
    instructions = list(_ENGINE.disasm(body, BASE + 0x100))
    assert sum(item.size for item in instructions) == len(body)
    return {0x100: body, 0x1F0: struct.pack("<II", arm0, arm1),
            0x300: b"\xcc", 0x380: bytes.fromhex("8b 44 24 04 c2 04 00")}, (table, table + 4)


def test_eh_framed_table_dispatching_callee_preserves_through_its_teardown():
    # Arm 0: push 1; call 0x380 (ret 4). Only the stdcall effect moves ESP.
    arm = b"\x6a\x01" + _call(BASE + 0x100 + 41, BASE + 0x380)
    placed, relocations = _eh_framed_callee(arm)
    assert placed[0x100][39:41] == b"\x6a\x01"
    _proves(placed, relocations=relocations)
    # The registration pushes are ordinary depth: the same frame with the
    # nested callee declared `ret 8` misplaces the restore.
    wrong = {**placed, 0x380: bytes.fromhex("8b 44 24 04 c2 08 00")}
    with pytest.raises(ValueError, match="releases|without restoring|unknown stack depth"):
        _proves(wrong, relocations=relocations)


def test_eh_framed_callee_with_a_virtual_call_under_the_save_refuses():
    # Arm 0: push 1; mov eax,[ecx]; call [eax]. The image installs no vtable,
    # so the image-wide slot proof has no candidate and the call stays unprovable.
    placed, relocations = _eh_framed_callee(bytes.fromhex("6a 01 8b 01 ff 10 90 90"))
    with pytest.raises(ValueError, match="restores ebx at an unknown stack depth"):
        _proves(placed, relocations=relocations)


# Virtual calls through a frame-constant vtable (user decision 2026-11-17).
RDATA = BASE + 0x1000
VTABLE, VBTABLE = RDATA + 0x10, RDATA + 0x40


OTHER8, OTHER0 = RDATA + 0x80, RDATA + 0xA0


def _disagreeing_vtables() -> tuple[dict[int, bytes], dict[int, bytes], set[int]]:
    """Two more installed vtables whose slots return with ret 8 and ret 0.

    With them in the image the image-wide slot proof cannot agree on one stack
    effect, so the frame-constant controls below see only their own rule.
    """
    installer = (bytes.fromhex("c7 01") + struct.pack("<I", OTHER8)  # mov dword [ecx],OTHER8
                 + bytes.fromhex("c7 01") + struct.pack("<I", OTHER0) + b"\xc3")
    placed = {0x700: installer, 0x740: RET8, 0x760: RET0}
    rdata = {OTHER8 - RDATA: struct.pack("<II", BASE + 0x740, BASE + 0x740),
             OTHER0 - RDATA: struct.pack("<II", BASE + 0x760, BASE + 0x760)}
    relocations = {BASE + 0x702, BASE + 0x708, OTHER8, OTHER8 + 4, OTHER0, OTHER0 + 4}
    return placed, rdata, relocations


def _image_with_rdata(placed: dict[int, bytes], rdata: dict[int, bytes], relocations=(),
                      *, others=True) -> PeImage:
    """.text at BASE, read-only .rdata at BASE+0x1000 and a .reloc section."""
    if others:
        extra_placed, extra_rdata, extra_relocations = _disagreeing_vtables()
        placed = {**extra_placed, **placed}
        rdata = {**extra_rdata, **rdata}
        relocations = set(relocations) | extra_relocations
    text = bytearray(b"\xcc" * SECTION)
    for offset, code in placed.items():
        text[offset:offset + len(code)] = code
    data = bytearray(0x200)
    for offset, words in rdata.items():
        data[offset:offset + len(words)] = words
    pages: dict[int, list[int]] = {}
    for site in sorted(relocations):
        pages.setdefault((site - 0x400000) & ~0xFFF, []).append(0x3000 | (site & 0xFFF))
    block = b""
    for page, entries in sorted(pages.items()):
        if len(entries) % 2:
            entries.append(0)
        block += struct.pack("<II", page, 8 + 2 * len(entries))
        block += b"".join(struct.pack("<H", entry) for entry in entries)
    reloc = block + bytes(-len(block) % 0x200 or (0x200 if not block else 0))
    raw = bytearray(0x400)
    raw[:2] = b"MZ"
    struct.pack_into("<I", raw, 0x3C, 0x80)
    raw[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", raw, 0x84, 0x14C, 3, 0, 0, 0, 224, 0)
    struct.pack_into("<H", raw, 0x98, 0x10B)
    struct.pack_into("<I", raw, 0x98 + 28, 0x400000)
    struct.pack_into("<I", raw, 0x98 + 92, 16)
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x3000, len(block))
    layout = ((b".text", 0x1000, text, 0x60000020), (b".rdata", 0x2000, data, 0x40000040),
              (b".reloc", 0x3000, reloc, 0x42000040))
    offset = 0x400
    for index, (name, rva, body, flags) in enumerate(layout):
        header = 0x178 + 40 * index
        raw[header:header + 8] = name.ljust(8, b"\0")
        struct.pack_into("<IIII", raw, header + 8, len(body), rva, len(body), offset)
        struct.pack_into("<I", raw, header + 36, flags)
        offset += len(body)
    for _, _, body, _ in layout:
        raw += body
    image = bytes(raw)
    return PeImage(image, hashlib.sha256(image).hexdigest())


RET4 = bytes.fromhex("8b 44 24 04 c2 04 00")  # mov eax,[esp+4]; ret 4
RET8 = bytes.fromhex("8b 44 24 04 c2 08 00")
RET0 = bytes.fromhex("8b c1 c3")  # mov eax,ecx; ret
STORE_VTABLE = bytes.fromhex("c7 45 f8") + struct.pack("<I", VTABLE)  # mov dword [ebp-8],VTABLE
SAVED_HEAD = bytes.fromhex("55 8b ec 83 ec 08 53 31 db")  # push ebp; mov ebp,esp; sub esp,8; push ebx; xor ebx,ebx
# push 1; lea ecx,[ebp-8]; mov eax,[ebp-8]; call [eax+4]; pop ebx; mov esp,ebp; pop ebp; ret
SAVED_CALL = bytes.fromhex("6a 01 8d 4d f8 8b 45 f8 ff 50 04 5b 8b e5 5d c3")


def _virtual(body: bytes, slot_targets=(0x380, 0x3A0), *, targets=None, relocate_store=True,
             relocate_slots=True, extra=()):
    """custom_planet-shaped callee at 0x100 whose vtable at VTABLE holds two slots."""
    placed = {0x100: body, 0x380: RET4, 0x3A0: RET4, 0x3C0: RET0, 0x3E0: CLOBBERS,
              **(targets or {})}
    rdata = {VTABLE - RDATA: b"".join(struct.pack("<I", BASE + target) for target in slot_targets)}
    relocations = set(extra)
    if relocate_slots:
        relocations |= {VTABLE, VTABLE + 4}
    store = body.find(STORE_VTABLE)
    if relocate_store and store >= 0:
        relocations.add(BASE + 0x100 + store + 3)
    return _image_with_rdata(placed, rdata, relocations)


def test_virtual_call_through_a_frame_constant_vtable_is_proved_with_evidence():
    body = SAVED_HEAD + STORE_VTABLE + SAVED_CALL
    image = _virtual(body)
    evidence = callee_preserves(image, BASE + 0x100, "ebx")
    call = BASE + 0x100 + len(SAVED_HEAD) + len(STORE_VTABLE) + 8
    assert evidence == [{
        "call": call, "vtable": VTABLE, "slot": VTABLE + 4, "target": BASE + 0x3A0,
        "stores": [{"address": BASE + 0x100 + len(SAVED_HEAD), "constant": VTABLE,
                    "frame_offset": -12}],
        "table_reads": []}]
    # The resolved target's own stack effect is what places the restore.
    with pytest.raises(ValueError, match="releases|without restoring"):
        callee_preserves(_virtual(body, targets={0x3A0: RET8}), BASE + 0x100, "ebx")


def test_virtual_target_must_itself_preserve_an_unsaved_register():
    # push ebp; mov ebp,esp; sub esp,8; mov [ebp-8],VTABLE; lea ecx,[ebp-8];
    # mov eax,[ebp-8]; call [eax]; mov esp,ebp; pop ebp; ret
    body = (bytes.fromhex("55 8b ec 83 ec 08") + STORE_VTABLE
            + bytes.fromhex("8d 4d f8 8b 45 f8 ff 10 8b e5 5d c3"))
    assert callee_preserves(_virtual(body, (0x3C0, 0x3C0)), BASE + 0x100, "ebx")[0]["target"] == BASE + 0x3C0
    with pytest.raises(ValueError, match="0x4013e0 overwrites ebx"):
        callee_preserves(_virtual(body, (0x3E0, 0x3C0)), BASE + 0x100, "ebx")


@pytest.mark.parametrize("between", [
    bytes.fromhex("89 75 f8"),  # mov [ebp-8],esi: the vptr slot is overwritten
    bytes.fromhex("88 44 0d fa"),  # mov [ebp+ecx-6],al: unknown frame index
    bytes.fromhex("f3 ab"),  # rep stosd: a string store
])
def test_vptr_slot_written_before_the_call_refuses(between):
    body = SAVED_HEAD + STORE_VTABLE + between + SAVED_CALL
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(_virtual(body), BASE + 0x100, "ebx")
    # A disjoint frame store keeps the fact: mov [ebp-4],esi.
    callee_preserves(_virtual(SAVED_HEAD + STORE_VTABLE + bytes.fromhex("89 75 fc") + SAVED_CALL),
                     BASE + 0x100, "ebx")


def test_vtable_from_a_non_constant_source_refuses():
    non_constant = SAVED_HEAD + bytes.fromhex("89 75 f8 90 90 90 90") + SAVED_CALL  # mov [ebp-8],esi
    code_vtable = SAVED_HEAD + bytes.fromhex("c7 45 f8") + struct.pack("<I", BASE + 0x380) + SAVED_CALL
    cases = [
        _virtual(non_constant),
        _virtual(SAVED_HEAD + STORE_VTABLE + SAVED_CALL, relocate_store=False),
        _virtual(SAVED_HEAD + STORE_VTABLE + SAVED_CALL, relocate_slots=False),
        _virtual(code_vtable, extra=(BASE + 0x100 + len(SAVED_HEAD) + 3,)),
    ]
    for image in cases:
        with pytest.raises(ValueError, match="unknown stack depth"):
            callee_preserves(image, BASE + 0x100, "ebx")
    # Only one path stores the constant: the store must dominate the call.
    # push ebp; mov ebp,esp; sub esp,8; push ebx; xor ebx,ebx; test ecx,ecx;
    # jz +7; mov [ebp-8],VTABLE; <call>
    branch = SAVED_HEAD + bytes.fromhex("85 c9 74 07") + STORE_VTABLE + SAVED_CALL
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(_virtual(branch), BASE + 0x100, "ebx")


def _vbase_body(vbptr_store: bytes) -> bytes:
    """Virtual-base deleting call: vbptr at [ebp-10h], vbase vptr at vbptr+[vbtable+4]."""
    return (bytes.fromhex("55 8b ec 83 ec 10 53 31 db") + vbptr_store
            + bytes.fromhex("8b 4d f0 8b 51 04")  # mov ecx,[ebp-10h]; mov edx,[ecx+4]
            + bytes.fromhex("c7 44 15 f0") + struct.pack("<I", VTABLE)  # mov [ebp+edx-10h],VTABLE
            + bytes.fromhex("6a 01 8b 45 f0 8b 50 04")  # push 1; mov eax,[ebp-10h]; mov edx,[eax+4]
            + bytes.fromhex("8d 4c 15 f0 8b 01 ff 10")  # lea ecx,[ebp+edx-10h]; mov eax,[ecx]; call [eax]
            + bytes.fromhex("5b 8b e5 5d c3"))


def test_virtual_base_adjusted_call_needs_a_constant_vbtable():
    store = bytes.fromhex("c7 45 f0") + struct.pack("<I", VBTABLE)
    body = _vbase_body(store)
    vbptr_store = BASE + 0x109
    vptr_store = vbptr_store + 7 + 6
    relocations = {vbptr_store + 3, vptr_store + 4}
    image = _image_with_rdata(
        {0x100: body, 0x380: RET4},
        {VTABLE - RDATA: struct.pack("<I", BASE + 0x380), VBTABLE - RDATA: struct.pack("<ii", 0, 8)},
        relocations | {VTABLE})
    evidence = callee_preserves(image, BASE + 0x100, "ebx")
    assert len(evidence) == 1
    assert evidence[0]["target"] == BASE + 0x380 and evidence[0]["vtable"] == VTABLE
    assert {"cell": VBTABLE + 4, "value": 8} in evidence[0]["table_reads"]
    assert {(row["address"], row["constant"]) for row in evidence[0]["stores"]} == {
        (vbptr_store, VBTABLE), (vptr_store, VTABLE)}
    # A vbptr from a non-constant source, or a relocated vbtable cell (an
    # address, not a virtual-base offset), leaves the frame-constant rule
    # without a vtable. The call still reaches its vptr through a
    # virtual-base offset, so ASSUMPTION (a) narrows the image-wide slot
    # proof to the vtables installed by indexed stores: VTABLE alone (the
    # disagreeing OTHER8/OTHER0 tables are installed by plain stores).
    unknown = _vbase_body(bytes.fromhex("89 75 f0 90 90 90 90"))
    images = [
        _image_with_rdata(
            {0x100: unknown, 0x380: RET4},
            {VTABLE - RDATA: struct.pack("<I", BASE + 0x380), VBTABLE - RDATA: struct.pack("<ii", 0, 8)},
            {vptr_store + 4, VTABLE}),
        _image_with_rdata(
            {0x100: body, 0x380: RET4},
            {VTABLE - RDATA: struct.pack("<I", BASE + 0x380), VBTABLE - RDATA: struct.pack("<ii", 0, 8)},
            relocations | {VTABLE, VBTABLE + 4}),
    ]
    for image in images:
        evidence = callee_preserves(image, BASE + 0x100, "ebx")
        assert [(row["kind"], row["targets"], row["vtables"]) for row in evidence] == [
            ("image_wide_slot", (BASE + 0x380,), 1)]
    # Without the indexed install the narrowed set is empty: unprovable.
    image = _image_with_rdata(
        {0x100: unknown, 0x380: RET4},
        {VTABLE - RDATA: struct.pack("<I", BASE + 0x380), VBTABLE - RDATA: struct.pack("<ii", 0, 8)},
        {VTABLE})
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(image, BASE + 0x100, "ebx")


# Image-wide slot proof (user decision 2026-11-17, option 1). Assumes objects
# carry only vtables the image installs with a relocated `mov dword [mem],imm32`.
VT_A, VT_B = RDATA + 0x10, RDATA + 0x20
# push ebx; xor ebx,ebx; push 1; mov eax,[ecx]; call [eax+4]; pop ebx; ret
HEAP_CALL = bytes.fromhex("53 31 db 6a 01 8b 01 ff 50 04 5b c3")
# ecx-based constructors: mov dword [ecx],VT_A; ret / mov dword [ecx],VT_B; ret
INSTALL_A = bytes.fromhex("c7 01") + struct.pack("<I", VT_A) + b"\xc3"
INSTALL_B = bytes.fromhex("c7 01") + struct.pack("<I", VT_B) + b"\xc3"


def _heap_call_image(slot_a: bytes, slot_b: bytes, *, body=HEAP_CALL, install_b=True,
                     relocate_installs=True) -> PeImage:
    placed = {0x100: body, 0x200: INSTALL_A, 0x400: RET4, 0x420: slot_a, 0x440: slot_b}
    if install_b:
        placed[0x210] = INSTALL_B
    rdata = {VT_A - RDATA: struct.pack("<II", BASE + 0x400, BASE + 0x420),
             VT_B - RDATA: struct.pack("<II", BASE + 0x400, BASE + 0x440)}
    relocations = {VT_A, VT_A + 4, VT_B, VT_B + 4}
    if relocate_installs:
        relocations |= {BASE + 0x202, BASE + 0x212}
    return _image_with_rdata(placed, rdata, relocations, others=False)


def test_slot_call_through_every_installed_vtable_is_proved_with_evidence():
    evidence = callee_preserves(_heap_call_image(RET4, RET4), BASE + 0x100, "ebx")
    assert evidence == [{
        "call": BASE + 0x107, "kind": "image_wide_slot", "slot_offset": 4,
        "targets": (BASE + 0x420, BASE + 0x440), "vtables": 2, "virtual_base": False,
        "stores": [], "table_reads": []}]


def test_slot_call_with_differing_effects_or_no_installed_vtable_refuses():
    # One installed vtable's slot pops 8 bytes: no shared effect.
    with pytest.raises(ValueError, match="restores ebx at an unknown stack depth"):
        callee_preserves(_heap_call_image(RET4, RET8), BASE + 0x100, "ebx")
    # A slot target whose effect cannot be established.
    unknown = bytes.fromhex("ff e0")  # jmp eax
    with pytest.raises(ValueError, match="restores ebx at an unknown stack depth"):
        callee_preserves(_heap_call_image(RET4, unknown), BASE + 0x100, "ebx")
    # No relocated installing store: nothing is a vtable, the call stays unprovable.
    with pytest.raises(ValueError, match="restores ebx at an unknown stack depth"):
        callee_preserves(_heap_call_image(RET4, RET4, relocate_installs=False),
                         BASE + 0x100, "ebx")
    # Only VT_A installed: its slot alone decides (one RET4 candidate).
    evidence = callee_preserves(_heap_call_image(RET4, RET8, install_b=False), BASE + 0x100, "ebx")
    assert evidence[0]["targets"] == (BASE + 0x420,) and evidence[0]["vtables"] == 1


def test_slot_targets_must_preserve_an_unsaved_register():
    # mov eax,[ecx]; push 1; call [eax+4]; ret 0 -- EBX never saved.
    body = bytes.fromhex("8b 01 6a 01 ff 50 04 c3")
    callee_preserves(_heap_call_image(RET4, RET4, body=body), BASE + 0x100, "ebx")
    clobber4 = bytes.fromhex("bb 01 00 00 00 c2 04 00")  # mov ebx,1; ret 4
    with pytest.raises(ValueError, match="overwrites ebx"):
        callee_preserves(_heap_call_image(RET4, clobber4, body=body), BASE + 0x100, "ebx")


# ASSUMPTION (a): a call that reaches its vptr through a virtual-base offset
# only sees vtables installed through indexed (virtual-base) stores.
# push ebx; xor ebx,ebx; push 1; mov eax,[esi]; mov ecx,[eax+4]; add ecx,esi;
# mov edx,[ecx]; call [edx]; pop ebx; ret
VBASE_HEAP_CALL = bytes.fromhex("53 31 db 6a 01 8b 06 8b 48 04 03 ce 8b 11 ff 12 5b c3")


def _vbase_heap_image(indexed_target: bytes, plain_target: bytes) -> PeImage:
    # mov ecx,[esi+4]; mov dword [esi+ecx],VT_A; ret  -- an indexed install
    # mov dword [esi],VT_B; ret                       -- a plain install
    placed = {0x100: VBASE_HEAP_CALL,
              0x200: bytes.fromhex("8b 4e 04 c7 04 0e") + struct.pack("<I", VT_A) + b"\xc3",
              0x220: bytes.fromhex("c7 06") + struct.pack("<I", VT_B) + b"\xc3",
              0x400: indexed_target, 0x420: plain_target}
    rdata = {VT_A - RDATA: struct.pack("<I", BASE + 0x400), VT_B - RDATA: struct.pack("<I", BASE + 0x420)}
    relocations = {VT_A, VT_B, BASE + 0x206, BASE + 0x222}
    return _image_with_rdata(placed, rdata, relocations, others=False)


def test_virtual_base_reached_call_sees_only_indexed_installs():
    evidence = callee_preserves(_vbase_heap_image(RET4, RET8), BASE + 0x100, "ebx")
    assert [(row["targets"], row["vtables"], row["virtual_base"]) for row in evidence] == [
        ((BASE + 0x400,), 1, True)]
    # The narrowed slot decides the effect: ret 8 there releases the saved slot.
    with pytest.raises(ValueError, match="releases its saved ebx slot"):
        callee_preserves(_vbase_heap_image(RET8, RET4), BASE + 0x100, "ebx")
    # The plain heap call (no virtual-base offset) still sees both installs.
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(_heap_call_image(RET4, RET8), BASE + 0x100, "ebx")


# ASSUMPTION (b): every installed vbtable's virtual-base offsets are proved
# non-negative, so an indexed virtual-base store into the frame writes only at
# or above its displacement and a frame-constant vptr below survives it.
VBT = RDATA + 0x60


def _floor_image(offset: int) -> PeImage:
    # push ebp; mov ebp,esp; sub esp,8; push ebx; xor ebx,ebx; mov [ebp-8],VTABLE;
    # mov ecx,[ebp-4]; mov edx,[ecx+4]; mov dword [ebp+edx-4],VTABLE; <SAVED_CALL>
    body = (SAVED_HEAD + STORE_VTABLE + bytes.fromhex("8b 4d fc 8b 51 04 c7 44 15 fc")
            + struct.pack("<I", VTABLE) + SAVED_CALL)
    store = BASE + 0x100 + len(SAVED_HEAD)
    indexed_store = store + len(STORE_VTABLE) + 6
    # mov dword [ecx],VBT; ret -- installs the vbtable candidate
    placed = {0x100: body, 0x200: bytes.fromhex("c7 01") + struct.pack("<I", VBT) + b"\xc3",
              0x380: RET4, 0x3A0: RET4}
    rdata = {VTABLE - RDATA: struct.pack("<II", BASE + 0x380, BASE + 0x3A0),
             VBT - RDATA: struct.pack("<ii", 0, offset)}
    relocations = {VTABLE, VTABLE + 4, store + 3, indexed_store + 4, BASE + 0x202}
    return _image_with_rdata(placed, rdata, relocations)


def test_frame_constant_vptr_survives_a_non_negative_virtual_base_store():
    evidence = callee_preserves(_floor_image(8), BASE + 0x100, "ebx")
    assert [(row.get("vtable"), row.get("target")) for row in evidence] == [(VTABLE, BASE + 0x3A0)]
    # A negative virtual-base offset in any installed vbtable: the store may hit the slot.
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(_floor_image(-8), BASE + 0x100, "ebx")


# ASSUMPTION (c): a frame object's known vptr changes only in that object's own
# constructor or destructor (a call receiving its address in ECX whose callee
# installs a vtable or vbtable).
# mov dword [ecx],OTHER8; ret  -- installs a vtable: a constructor/destructor
CTOR = bytes.fromhex("c7 01") + struct.pack("<I", OTHER8) + b"\xc3"


def _with_call(prefix: bytes, callee: bytes, relocate_callee_store=False) -> PeImage:
    """SAVED_HEAD; mov [ebp-8],VTABLE; <prefix>; call 0x5C0; SAVED_CALL."""
    start = BASE + 0x100 + len(SAVED_HEAD) + len(STORE_VTABLE) + len(prefix)
    body = SAVED_HEAD + STORE_VTABLE + prefix + _call(start, BASE + 0x5C0) + SAVED_CALL
    extra = (BASE + 0x5C2,) if relocate_callee_store else ()
    image = _virtual(body, targets={0x5C0: callee}, extra=extra)
    return image


def test_known_vptr_survives_unrelated_calls_and_pointer_stores():
    lea_this = bytes.fromhex("8d 4d f8")  # lea ecx,[ebp-8]: the callee receives the object
    for prefix, callee in ((lea_this, RET0), (b"", CTOR), (bytes.fromhex("89 06"), RET0)):
        evidence = callee_preserves(_with_call(prefix, callee, callee is CTOR), BASE + 0x100, "ebx")
        assert [(row.get("vtable"), row.get("target")) for row in evidence] == [(VTABLE, BASE + 0x3A0)]


def test_known_vptr_is_forgotten_by_the_objects_own_constructor_or_destructor():
    lea_this = bytes.fromhex("8d 4d f8")
    # The callee re-installs OTHER8 into the object it receives: by ASSUMPTION
    # (e1) the object now carries OTHER8, whose slot 1 pops 8 bytes.
    with pytest.raises(ValueError, match="releases its saved ebx slot"):
        callee_preserves(_with_call(lea_this, CTOR, True), BASE + 0x100, "ebx")
    # An object starting above the known slot is a different object: its
    # constructor keeps the fact. One starting below may contain the slot.
    above = bytes.fromhex("8d 4d fc")  # lea ecx,[ebp-4]
    callee_preserves(_with_call(above, CTOR, True), BASE + 0x100, "ebx")
    below = bytes.fromhex("8d 4d f4")  # lea ecx,[ebp-0Ch]
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(_with_call(below, CTOR, True), BASE + 0x100, "ebx")
    # A vtable install through a pointer forgets every fact.
    install = bytes.fromhex("c7 06") + struct.pack("<I", OTHER0)  # mov dword [esi],OTHER0
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(_virtual(SAVED_HEAD + STORE_VTABLE + install + SAVED_CALL,
                                  extra=(BASE + 0x100 + len(SAVED_HEAD) + len(STORE_VTABLE) + 2,)),
                         BASE + 0x100, "ebx")


# ASSUMPTION (e1): after the object's own constructor its vptr is the last
# vtable that constructor installs at [this].
def test_constructor_result_becomes_the_known_vptr():
    lea_this = bytes.fromhex("8d 4d f8")
    # mov esi,ecx; mov dword [esi],OTHER0; mov dword [esi],VTABLE; mov dword [esi+4],OTHER8; ret
    ctor = (bytes.fromhex("8b f1 c7 06") + struct.pack("<I", OTHER0)
            + bytes.fromhex("c7 06") + struct.pack("<I", VTABLE)
            + bytes.fromhex("c7 46 04") + struct.pack("<I", OTHER8) + b"\xc3")
    image = _virtual(SAVED_HEAD + STORE_VTABLE + lea_this
                     + _call(BASE + 0x100 + len(SAVED_HEAD) + len(STORE_VTABLE) + 3, BASE + 0x5C0)
                     + SAVED_CALL,
                     targets={0x5C0: ctor},
                     extra=(BASE + 0x5C4, BASE + 0x5CA, BASE + 0x5D1))
    evidence = callee_preserves(image, BASE + 0x100, "ebx")
    assert [(row.get("vtable"), row.get("target")) for row in evidence] == [(VTABLE, BASE + 0x3A0)]
    # Two returns leaving different vtables: no known result, the call stays unprovable.
    # mov esi,ecx; test eax,eax; jz +7; mov [esi],OTHER0; ret; mov [esi],VTABLE; ret
    split = (bytes.fromhex("8b f1 85 c0 74 07 c7 06") + struct.pack("<I", OTHER0) + b"\xc3"
             + bytes.fromhex("c7 06") + struct.pack("<I", VTABLE) + b"\xc3")
    image = _virtual(SAVED_HEAD + STORE_VTABLE + lea_this
                     + _call(BASE + 0x100 + len(SAVED_HEAD) + len(STORE_VTABLE) + 3, BASE + 0x5C0)
                     + SAVED_CALL,
                     targets={0x5C0: split}, extra=(BASE + 0x5C8, BASE + 0x5CF))
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(image, BASE + 0x100, "ebx")


# ASSUMPTION (e2): a known frame-object vptr is carried into a callee that
# receives the object as `this`.
def test_known_vptr_is_carried_into_a_callee_receiving_this():
    def image(lea: bytes) -> PeImage:
        # push ebp; mov ebp,esp; sub esp,8; mov [ebp-8],VTABLE; <lea ecx>; call 0x5C0;
        # mov esp,ebp; pop ebp; ret
        head = bytes.fromhex("55 8b ec 83 ec 08") + STORE_VTABLE + lea
        body = head + _call(BASE + 0x100 + len(head), BASE + 0x5C0) + bytes.fromhex("8b e5 5d c3")
        # push ebx; xor ebx,ebx; push 1; mov eax,[ecx]; call [eax+4]; pop ebx; ret
        return _virtual(body, targets={0x5C0: HEAP_CALL})
    callee_preserves(image(bytes.fromhex("8d 4d f8")), BASE + 0x100, "ebx")
    # The callee alone, or receiving an object whose vptr is unknown, refuses.
    with pytest.raises(ValueError, match="unknown stack depth"):
        callee_preserves(image(bytes.fromhex("8d 4d f8")), BASE + 0x5C0, "ebx")
    with pytest.raises(ValueError, match="overwrites ebx|unknown stack depth"):
        callee_preserves(image(bytes.fromhex("8d 4d fc")), BASE + 0x100, "ebx")
