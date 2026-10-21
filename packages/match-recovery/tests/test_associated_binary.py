"""Synthetic controls for explicit code-associated switch data."""

import hashlib
import struct

import pytest

from match_recovery.associated_binary import compare_associated_code
from match_recovery.associated_binary import (
    _ENGINE, _guard_for_dispatch, _is_canonical_lea_identity_padding,
    _memory_operand, _prove_dispatch_extent,
)
from match_recovery.associated_source import AssociatedDataBinding
from match_recovery.model import Origin, Result, Span
from match_recovery.pe_coff import Coff, PeImage


BASE = 0x401000
BODY = bytes.fromhex("83 f9 00 77 10 31 d2 8a 91 1c 10 40 00 ff 24 95 18 10 40 00 90 c3")
GAP = b"\x8b\xff"
TABLE_TARGET = BASE + 21


def _table_start() -> int:
    return len(BODY) + len(GAP)


def _original_body():
    body = bytearray(BODY)
    table = BASE + _table_start()
    struct.pack_into("<I", body, 9, table + 4)
    struct.pack_into("<I", body, 16, table)
    return bytes(body)


def _original_envelope(*, gap_relocation=False):
    body = _original_body()
    return body + GAP + struct.pack("<I", TABLE_TARGET) + b"\0"


def _pe_fixture(*, gap_relocation=False):
    original_envelope = _original_envelope(gap_relocation=gap_relocation)
    raw = bytearray(1024)
    raw[:2] = b"MZ"
    struct.pack_into("<I", raw, 0x3C, 0x80)
    raw[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", raw, 0x84, 0x14C, 1, 0, 0, 0, 224, 0)
    struct.pack_into("<H", raw, 0x98, 0x10B)
    struct.pack_into("<I", raw, 0x98 + 28, 0x400000)
    struct.pack_into("<I", raw, 0x98 + 92, 16)
    raw[0x178:0x180] = b".text\0\0\0"
    struct.pack_into("<IIII", raw, 0x180, 512, 0x1000, 512, 512)
    struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)  # executable code/read
    raw[512:512 + len(original_envelope)] = original_envelope
    sites = (9, 16, _table_start())
    if gap_relocation:
        sites = (*sites, len(BODY))
    entries = [0x3000 + site for site in sites]
    block_size = 8 + 2 * len(entries)
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1100, block_size)
    struct.pack_into("<II", raw, 768, 0x1000, block_size)
    for index, entry in enumerate(entries):
        struct.pack_into("<H", raw, 776 + index * 2, entry)
    return bytes(raw)


def _coff_fixture(*, cell_addend=0, selector_byte=0, extra_relocation=False,
                  candidate_gap=None, gap_relocation=False,
                  truncate_candidate_at=None):
    # The exact body and data bytes are represented by compiler output with
    # zeroed absolute fields plus physical same-section COFF relocations.
    if candidate_gap is None:
        candidate_gap = GAP
    table_start = len(BODY) + len(candidate_gap)
    data = bytearray(BODY + candidate_gap + bytes(4) + bytes((selector_byte,)))
    data += b"\x90" * (-len(data) % 16)
    data[9:13] = bytes(4)
    data[16:20] = bytes(4)
    struct.pack_into("<i", data, table_start, cell_addend)
    relocations = [(9, 4, 6), (16, 3, 6), (table_start, 5, 6)]
    if gap_relocation:
        relocations.append((len(BODY), 5, 6))
    if extra_relocation:
        relocations.append((table_start + 4, 4, 6))
    if truncate_candidate_at is not None:
        data = data[:truncate_candidate_at]
    relocation_blob = b"".join(struct.pack("<IIH", *row) for row in relocations)

    symbols = bytearray()
    symbols += b"_f\0\0\0\0\0\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
    symbols += b".text\0\0\0" + struct.pack("<IhHBB", 0, 1, 0, 3, 1)
    # IMAGE_AUX_SYMBOL_SECTION_DEF, selection=ANY; length/count checked by Coff.
    symbols += struct.pack("<IHHIHB", len(data), len(relocations), 0, 0, 0, 2) + bytes(3)
    symbols += b".Lswitch" + struct.pack("<IhHBB", table_start, 1, 0, 6, 0)
    symbols += b".Lselect" + struct.pack("<IhHBB", table_start + 4, 1, 0, 6, 0)
    symbols += b".Lcase\0\0" + struct.pack("<IhHBB", TABLE_TARGET - BASE, 1, 0, 6, 0)

    data_at = 60
    relocation_at = data_at + len(data)
    symbol_at = relocation_at + len(relocation_blob)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbol_at, 6, 0, 0)
    flags = 0x60501020  # code, execute/read, 16-byte alignment, COMDAT
    section = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(data), data_at,
                                                relocation_at, 0, len(relocations), 0, flags))
    return header + section + data + relocation_blob + symbols + struct.pack("<I", 4)


def fixture(*, cell_addend=0, selector_byte=0, extra_relocation=False,
            candidate_gap=None, original_gap_relocation=False,
            candidate_gap_relocation=False, truncate_candidate_at=None):
    """Return (object, function, body span, pinned image, bindings, targets)."""
    raw = _pe_fixture(gap_relocation=original_gap_relocation)
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(raw, pin)
    obj = Coff(_coff_fixture(cell_addend=cell_addend, selector_byte=selector_byte,
                            extra_relocation=extra_relocation,
                            candidate_gap=candidate_gap,
                            gap_relocation=candidate_gap_relocation,
                            truncate_candidate_at=truncate_candidate_at))
    subject = obj.function("_f")
    source_span = Span(BASE, BASE + len(BODY))
    owner = Origin(pin, "pe:va", BASE)
    bindings = (
        AssociatedDataBinding(owner, Span(BASE + _table_start(), BASE + _table_start() + 4), "switch_targets",
                              Origin(pin, "pe:va", BASE + 13), "fixture.cpp", 2),
        AssociatedDataBinding(owner, Span(BASE + _table_start() + 4, BASE + _table_start() + 5), "switch_selector",
                              Origin(pin, "pe:va", BASE + 7), "fixture.cpp", 1),
    )
    return obj, subject, source_span, image, bindings, {}


def test_explicit_same_comdat_table_and_selector_compare_exactly():
    obj, subject, span, image, bindings, targets = fixture()
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.PASS, (report["bytes"], report["references"],
                                              report.get("differing_offsets"), report.get("relocations"))
    assert report["source_body_span"] == {"start": BASE, "end": BASE + len(BODY)}
    assert report["comparison_span"] == {"start": BASE, "end": BASE + 29}
    assert report["linked_selection"] == report["runtime"] == Result.UNKNOWN


@pytest.mark.parametrize("kwargs", [
    {"cell_addend": 1},
    {"selector_byte": 1},
])
def test_candidate_cell_selector_or_relocation_damage_never_passes(kwargs):
    obj, subject, span, image, bindings, targets = fixture(**kwargs)
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] != Result.PASS


def test_changed_original_binding_and_nonroot_subject_are_refused():
    obj, subject, span, image, bindings, targets = fixture()
    changed = (AssociatedDataBinding(bindings[0].owner, Span(BASE + 15, BASE + 19),
                                     bindings[0].kind, bindings[0].evidence,
                                     bindings[0].unit, bindings[0].line), bindings[1])
    with pytest.raises(ValueError):
        compare_associated_code(obj, subject, span, image, changed, targets)
    with pytest.raises(ValueError):
        compare_associated_code(obj, subject, Span(BASE, BASE + len(BODY) - 1),
                                 image, bindings, targets)


def test_candidate_section_cannot_attach_unmodeled_data_relocation():
    obj, subject, span, image, bindings, targets = fixture(extra_relocation=True)
    with pytest.raises(ValueError, match="selector bytes"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("offset,value", [(3, 0x7F), (1, 0xFA)])
def test_signed_or_different_register_guard_is_not_a_range_proof(monkeypatch, offset, value):
    changed = bytearray(BODY)
    changed[offset] = value
    monkeypatch.setitem(globals(), "BODY", bytes(changed))
    obj, subject, span, image, bindings, targets = fixture()
    with pytest.raises(ValueError, match="guard|selector"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("code", [
    bytes.fromhex("83 f9 00 41 77 07 ff 24 8d 20 10 40 00 c3"),
    bytes.fromhex("83 f9 00 f9 77 07 ff 24 8d 20 10 40 00 c3"),
    bytes.fromhex("83 f8 00 77 0a 75 00 ff 24 85 20 10 40 00 90 c3"),
    bytes.fromhex("83 f8 00 7f 7f ff 24 85 20 10 40 00 c3"),
    bytes.fromhex("83 f8 00 77 7f ff 24 85 20 10 40 00 c3"),
])
def test_guard_proof_rejects_alias_writes_flags_clobbers_bypass_and_bad_guards(code):
    instructions = list(_ENGINE.disasm(code, BASE))
    dispatch = next(item for item in instructions if item.mnemonic == "jmp")
    with pytest.raises(ValueError, match="guard"):
        _guard_for_dispatch(instructions, dispatch, Span(BASE, BASE + len(code)))


@pytest.mark.parametrize("position,control", [
    ("before", bytes.fromhex("9a 00 00 00 00 23 00")),  # far call
    ("after", bytes.fromhex("9a 00 00 00 00 23 00")),
    ("before", bytes.fromhex("cd 03")),  # software interrupt
    ("after", bytes.fromhex("cd 03")),
    ("before", bytes.fromhex("cf")),  # IRET
    ("after", bytes.fromhex("cf")),
])
def test_guard_proof_rejects_all_control_transfer_groups(position, control):
    dispatch = bytes.fromhex("ff 24 85 00 00 00 00")
    if position == "before":
        code = bytes.fromhex("83 f8 00") + control + bytes.fromhex("77 07") + dispatch + b"\xc3"
    else:
        displacement = len(control) + len(dispatch)
        code = (bytes.fromhex("83 f8 00") + bytes((0x77, displacement))
                + control + dispatch + b"\xc3")
    instructions = list(_ENGINE.disasm(code, BASE))
    table_jump = next(item for item in instructions if item.mnemonic == "jmp")
    with pytest.raises(ValueError, match="guard"):
        _guard_for_dispatch(instructions, table_jump, Span(BASE, BASE + len(code)))


def test_direct_table_guard_accepts_intervening_safe_x87_without_selector():
    obj, _, _, image, _, _ = fixture()
    code = (bytes.fromhex("83 f8 05") + bytes.fromhex("db 00") * 10
            + bytes.fromhex("77 07 ff 24 85 24 10 40 00 c3"))
    body = Span(BASE, BASE + len(code))
    instructions = list(_ENGINE.disasm(code, BASE))
    dispatch = next(item for item in instructions if item.mnemonic == "jmp")
    operand = _memory_operand(dispatch, "switch_targets")
    owner = Origin(hashlib.sha256(image.data).hexdigest(), "pe:va", BASE)
    table = AssociatedDataBinding(owner, Span(BASE + len(code) + 3,
                                               BASE + len(code) + 27),
                                  "switch_targets", Origin(owner.image_sha256, "pe:va",
                                                            dispatch.address),
                                  "fixture.cpp", 3)
    witness = _prove_dispatch_extent(instructions, body, image, table, dispatch, operand,
                                     None, None, None)
    assert witness["mode"] == "direct-index"
    assert witness["target_count"] == 6 and witness["selector_count"] == 0
    wrong_extent = AssociatedDataBinding(owner, Span(table.span.start, table.span.end + 4),
                                         table.kind, table.evidence, table.unit, table.line)
    with pytest.raises(ValueError, match="guard-derived"):
        _prove_dispatch_extent(instructions, body, image, wrong_extent, dispatch, operand,
                               None, None, None)


@pytest.mark.parametrize("original_gap,candidate_gap", [
    (b"\x90\x90", GAP),  # a separately supported ordinary NOP gap
    (b"\x8b\x90", GAP),  # malformed hotpatch spelling
    (GAP + b"\x90", GAP + b"\x90"),  # too long for the narrow exception
    (GAP, b"\x90\x90"),  # candidate differs from the recorded exception
])
def test_hotpatch_gap_is_exactly_two_bytes_and_same_in_both_images(
        monkeypatch, original_gap, candidate_gap):
    monkeypatch.setitem(globals(), "GAP", original_gap)
    obj, subject, span, image, bindings, targets = fixture(candidate_gap=candidate_gap)
    if original_gap == b"\x90\x90" and candidate_gap == b"\x90\x90":
        report = compare_associated_code(obj, subject, span, image, bindings, targets)
        assert report["result"] == Result.PASS
    else:
        if original_gap != b"\x90\x90":
            if original_gap == bytes.fromhex("8b ff") and candidate_gap != original_gap:
                report = compare_associated_code(obj, subject, span, image, bindings, targets)
                assert report["result"] == Result.FAIL
            else:
                with pytest.raises(ValueError):
                    compare_associated_code(obj, subject, span, image, bindings, targets)
        else:
            report = compare_associated_code(obj, subject, span, image, bindings, targets)
            assert report["result"] == Result.FAIL


def test_hotpatch_exception_requires_body_end_two_mod_four(monkeypatch):
    changed = BODY[:-1] + b"\x90\xc3"
    monkeypatch.setitem(globals(), "BODY", changed)
    obj, subject, span, image, bindings, targets = fixture()
    with pytest.raises(ValueError, match="DWORD-aligned"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_original_table_target_inside_instruction_is_not_an_entry(monkeypatch):
    monkeypatch.setitem(globals(), "TABLE_TARGET", BASE + 1)
    obj, subject, span, image, bindings, targets = fixture()
    with pytest.raises(ValueError, match="instruction boundary"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("register", [0, 1, 2, 3, 5, 6, 7])
def test_canonical_three_byte_lea_identity_predicate(register):
    modrm = 0x40 | (register << 3) | register
    code = bytes((0x8D, modrm, 0))
    decoded = list(_ENGINE.disasm(code, 0))
    assert len(decoded) == 1 and decoded[0].size == 3
    assert decoded[0].mnemonic == "lea"
    assert decoded[0].addr_size == 4 and decoded[0].disp_size == 1
    assert decoded[0].prefix == [0, 0, 0, 0]
    assert _is_canonical_lea_identity_padding(code)


@pytest.mark.parametrize("code", [
    bytes.fromhex("8d 48 00"),  # destination differs from base
    bytes.fromhex("8d 4c 08"),  # indexed SIB address
    bytes.fromhex("8d 49 01"),  # nonzero displacement
    bytes.fromhex("8b 49 00"),  # memory read, not LEA
    bytes.fromhex("8d 09"),     # register-direct, two-byte form
    bytes.fromhex("66 8d 49 00"),  # operand-size prefix
    bytes.fromhex("67 8d 49 00"),  # address-size prefix
    bytes.fromhex("2e 8d 49 00"),  # segment override
    bytes.fromhex("40 8d 49 00"),  # prefix bytes are never admitted
    bytes.fromhex("8d 49 00 90"),  # exact whole-gap decoding required
])
def test_lea_identity_predicate_rejects_noncanonical_shapes(code):
    assert not _is_canonical_lea_identity_padding(code)


def test_identical_lea_gap_between_body_and_aligned_table_is_strictly_compared(monkeypatch):
    body = bytearray(BODY[:-2] + b"\xc3")
    body[4] = 0x0F  # default branch lands on the retained NOP at offset 20.
    monkeypatch.setitem(globals(), "BODY", bytes(body))
    monkeypatch.setitem(globals(), "GAP", bytes.fromhex("8d 49 00"))
    monkeypatch.setitem(globals(), "TABLE_TARGET", BASE + 20)
    obj, subject, span, image, bindings, targets = fixture()
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.PASS
    assert report["comparison_span"] == {"start": BASE, "end": BASE + 29}


def test_changed_candidate_lea_gap_is_a_strict_mismatch_not_a_mask(monkeypatch):
    body = bytearray(BODY[:-2] + b"\xc3")
    body[4] = 0x0F
    monkeypatch.setitem(globals(), "BODY", bytes(body))
    monkeypatch.setitem(globals(), "GAP", bytes.fromhex("8d 49 00"))
    monkeypatch.setitem(globals(), "TABLE_TARGET", BASE + 20)
    obj, subject, span, image, bindings, targets = fixture(
        candidate_gap=bytes.fromhex("8d 48 00"))
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.FAIL


@pytest.mark.parametrize("gap", [
    bytes.fromhex("8d 48 00"),  # LEA, but destination and base differ.
    bytes.fromhex("8b 49 00"),  # same length, but reads memory.
])
def test_original_nonidentity_three_byte_instruction_is_not_alignment(monkeypatch, gap):
    body = bytearray(BODY[:-2] + b"\xc3")
    body[4] = 0x0F
    monkeypatch.setitem(globals(), "BODY", bytes(body))
    monkeypatch.setitem(globals(), "GAP", gap)
    monkeypatch.setitem(globals(), "TABLE_TARGET", BASE + 20)
    obj, subject, span, image, bindings, targets = fixture(candidate_gap=gap)
    with pytest.raises(ValueError, match="supported alignment"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_three_byte_lea_does_not_waive_original_or_candidate_relocations(monkeypatch):
    body = bytearray(BODY[:-2] + b"\xc3")
    body[4] = 0x0F
    monkeypatch.setitem(globals(), "BODY", bytes(body))
    monkeypatch.setitem(globals(), "GAP", bytes.fromhex("8d 49 00"))
    monkeypatch.setitem(globals(), "TABLE_TARGET", BASE + 20)
    obj, subject, span, image, bindings, targets = fixture(original_gap_relocation=True)
    with pytest.raises(ValueError, match="cuts through a relocation"):
        compare_associated_code(obj, subject, span, image, bindings, targets)
    obj, subject, span, image, bindings, targets = fixture(candidate_gap_relocation=True)
    with pytest.raises(ValueError, match="candidate COFF relocation"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_three_byte_gap_requires_actual_alignment_and_complete_candidate(monkeypatch):
    body = bytearray(BODY[:-2] + b"\xc3")
    body[4] = 0x0F
    monkeypatch.setitem(globals(), "BODY", bytes(body))
    monkeypatch.setitem(globals(), "GAP", bytes.fromhex("8d 49 00"))
    monkeypatch.setitem(globals(), "TABLE_TARGET", BASE + 20)
    # The candidate must physically contain the target cell and its relocation.
    obj, subject, span, image, bindings, targets = fixture(truncate_candidate_at=23)
    with pytest.raises(ValueError, match="in-bounds ABS32 relocation"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_three_byte_gap_does_not_make_an_unaligned_table_valid(monkeypatch):
    monkeypatch.setitem(globals(), "GAP", bytes.fromhex("8d 49 00"))
    obj, subject, span, image, bindings, targets = fixture()
    with pytest.raises(ValueError, match="DWORD-aligned"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("body_suffix,gap", [
    (b"\x90\x90\xc3", b"\x90"),
    (b"\x90\xc3", b"\x90\x90"),
])
def test_existing_one_and_two_byte_alignment_fill_still_pass(monkeypatch, body_suffix, gap):
    body = bytearray(BODY[:-2] + body_suffix)
    if len(body) != 22:
        body[4] = 0x0F
    monkeypatch.setitem(globals(), "BODY", bytes(body))
    monkeypatch.setitem(globals(), "GAP", gap)
    monkeypatch.setitem(globals(), "TABLE_TARGET", BASE + 21)
    obj, subject, span, image, bindings, targets = fixture(candidate_gap=gap)
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.PASS


def test_four_byte_nonfill_gap_remains_outside_the_supported_alignment_domain(monkeypatch):
    body = bytearray(BODY[:-2] + b"\x90\x90\x90\xc3")
    body[4] = 0x0F
    monkeypatch.setitem(globals(), "BODY", bytes(body))
    monkeypatch.setitem(globals(), "GAP", bytes.fromhex("8d 49 00 90"))
    monkeypatch.setitem(globals(), "TABLE_TARGET", BASE + 21)
    obj, subject, span, image, bindings, targets = fixture(
        candidate_gap=bytes.fromhex("8d 49 00 90"))
    with pytest.raises(ValueError, match="supported alignment"):
        compare_associated_code(obj, subject, span, image, bindings, targets)
