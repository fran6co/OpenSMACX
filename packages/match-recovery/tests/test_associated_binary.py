"""Synthetic controls for explicit code-associated switch data."""

import hashlib
import struct

import pytest

from match_recovery.associated_binary import compare_associated_code
from match_recovery.associated_binary import (
    _ENGINE, _guard_for_dispatch, _memory_operand, _prove_dispatch_extent,
)
from match_recovery.associated_source import AssociatedDataBinding
from match_recovery.model import Origin, Result, Span
from match_recovery.pe_coff import Coff, PeImage


BASE = 0x401000
BODY = bytes.fromhex("83 f9 00 77 10 31 d2 8a 91 1c 10 40 00 ff 24 95 18 10 40 00 90 c3")
GAP = b"\x8b\xff"
TABLE_TARGET = BASE + 21


def _original_envelope():
    return BODY + GAP + struct.pack("<I", TABLE_TARGET) + b"\0"


def _pe_fixture():
    original_envelope = _original_envelope()
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
    sites = (9, 16, 24)
    entries = [0x3000 + site for site in sites]
    block_size = 8 + 2 * len(entries)
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1100, block_size)
    struct.pack_into("<II", raw, 768, 0x1000, block_size)
    for index, entry in enumerate(entries):
        struct.pack_into("<H", raw, 776 + index * 2, entry)
    return bytes(raw)


def _coff_fixture(*, cell_addend=0, selector_byte=0, extra_relocation=False,
                  candidate_gap=GAP):
    # The exact body and data bytes are represented by compiler output with
    # zeroed absolute fields plus physical same-section COFF relocations.
    data = bytearray(BODY + candidate_gap + bytes(4) + bytes((selector_byte,)))
    data += b"\x90" * (-len(data) % 16)
    data[9:13] = bytes(4)
    data[16:20] = bytes(4)
    struct.pack_into("<i", data, 24, cell_addend)
    relocations = [(9, 4, 6), (16, 3, 6), (24, 5, 6)]
    if extra_relocation:
        relocations.append((28, 4, 6))
    relocation_blob = b"".join(struct.pack("<IIH", *row) for row in relocations)

    symbols = bytearray()
    symbols += b"_f\0\0\0\0\0\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
    symbols += b".text\0\0\0" + struct.pack("<IhHBB", 0, 1, 0, 3, 1)
    # IMAGE_AUX_SYMBOL_SECTION_DEF, selection=ANY; length/count checked by Coff.
    symbols += struct.pack("<IHHIHB", len(data), len(relocations), 0, 0, 0, 2) + bytes(3)
    symbols += b".Lswitch" + struct.pack("<IhHBB", 24, 1, 0, 6, 0)
    symbols += b".Lselect" + struct.pack("<IhHBB", 28, 1, 0, 6, 0)
    symbols += b".Lcase\0\0" + struct.pack("<IhHBB", 21, 1, 0, 6, 0)

    data_at = 60
    relocation_at = data_at + len(data)
    symbol_at = relocation_at + len(relocation_blob)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbol_at, 6, 0, 0)
    flags = 0x60501020  # code, execute/read, 16-byte alignment, COMDAT
    section = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(data), data_at,
                                                relocation_at, 0, len(relocations), 0, flags))
    return header + section + data + relocation_blob + symbols + struct.pack("<I", 4)


def fixture(*, cell_addend=0, selector_byte=0, extra_relocation=False,
            candidate_gap=GAP):
    """Return (object, function, body span, pinned image, bindings, targets)."""
    raw = _pe_fixture()
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(raw, pin)
    obj = Coff(_coff_fixture(cell_addend=cell_addend, selector_byte=selector_byte,
                            extra_relocation=extra_relocation,
                            candidate_gap=candidate_gap))
    subject = obj.function("_f")
    source_span = Span(BASE, BASE + len(BODY))
    owner = Origin(pin, "pe:va", BASE)
    bindings = (
        AssociatedDataBinding(owner, Span(BASE + 24, BASE + 28), "switch_targets",
                              Origin(pin, "pe:va", BASE + 13), "fixture.cpp", 2),
        AssociatedDataBinding(owner, Span(BASE + 28, BASE + 29), "switch_selector",
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
    with pytest.raises(ValueError, match="gap"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_original_table_target_inside_instruction_is_not_an_entry(monkeypatch):
    monkeypatch.setitem(globals(), "TABLE_TARGET", BASE + 1)
    obj, subject, span, image, bindings, targets = fixture()
    with pytest.raises(ValueError, match="instruction boundary"):
        compare_associated_code(obj, subject, span, image, bindings, targets)
