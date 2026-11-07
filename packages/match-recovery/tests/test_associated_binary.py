"""Synthetic controls for explicit code-associated switch data."""

import hashlib
import struct

import pytest

from match_recovery.associated_binary import compare_associated_code
from match_recovery.associated_binary import (
    _ENGINE, _guard_for_dispatch, _is_canonical_lea_identity_padding,
    _is_intergroup_hotpatch_gap, _is_intergroup_lea_identity_gap,
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


def _with_fpo_child(raw, *, code_size=None):
    """Add one exact associative FPO child to the synthetic root COMDAT."""
    header = bytearray(raw[:20])
    section_header = raw[20:60]
    raw_pointer, = struct.unpack_from("<I", section_header, 20)
    relocation_pointer, = struct.unpack_from("<I", section_header, 24)
    root_size, = struct.unpack_from("<I", section_header, 16)
    relocation_count, = struct.unpack_from("<H", section_header, 32)
    symbol_pointer, symbol_count = struct.unpack_from("<II", raw, 8)
    root_data = raw[raw_pointer:raw_pointer + root_size]
    root_relocations = raw[relocation_pointer:relocation_pointer + relocation_count * 10]
    symbol_records = raw[symbol_pointer:symbol_pointer + symbol_count * 18]
    string_table = raw[symbol_pointer + symbol_count * 18:]
    if len(string_table) != 4:
        raise AssertionError("fixture expected an empty COFF string table")

    code_size = len(BODY) + len(GAP) + 5 if code_size is None else code_size
    payload = struct.pack("<IIIHH", 0, code_size, 0, 0, 0)
    child_relocation = struct.pack("<IIH", 0, 0, 7)
    child_symbol = b".debug$F" + struct.pack("<IhHBB", 0, 2, 0, 3, 1)
    child_auxiliary = struct.pack("<IHHIHB3x", len(payload), 1, 0, 0, 1, 5)

    new_header_end = 20 + 2 * 40
    root_data_at = new_header_end
    root_relocations_at = root_data_at + root_size
    child_data_at = root_relocations_at + len(root_relocations)
    child_relocations_at = child_data_at + len(payload)
    symbols_at = child_relocations_at + len(child_relocation)
    struct.pack_into("<H", header, 2, 2)
    struct.pack_into("<II", header, 8, symbols_at, symbol_count + 2)

    root_header = bytearray(section_header)
    struct.pack_into("<I", root_header, 20, root_data_at)
    struct.pack_into("<I", root_header, 24, root_relocations_at)
    child_header = (b".debug$F" + struct.pack("<IIIIIIHHI", 0, 0, len(payload),
        child_data_at, child_relocations_at, 0, 1, 0, 0x42101048))
    return bytes(header + root_header + child_header + root_data + root_relocations
                 + payload + child_relocation + symbol_records + child_symbol
                 + child_auxiliary + string_table)


MULTI_BODY = bytes.fromhex(
    "83 f9 00 77 10 31 d2 0f b6 91 00 00 00 00 ff 24 95 00 00 00 00 "
    "83 fe 00 77 10 31 db 0f b6 9e 00 00 00 00 ff 24 9d 00 00 00 00 c3"
)


def multi_fixture(*, groups=2, original_change=None, candidate_change=None,
                  candidate_gap2=None, candidate_selector2=0, target1=None,
                  default2=None, omit=(), duplicate_evidence=False,
                  retarget_candidate=False, wrong_selector_anchor=False,
                  candidate_cell_targets=None, candidate_selector_values=None,
                  extra_tail=b"", extra_relocation=False,
                  unknown_indirect=False, call_target=None, gap2=None):
    """Build two or more independently guarded selector/table groups."""
    if groups not in (2, 4):
        raise ValueError("synthetic multi-dispatch fixture supports two or four groups")
    guard_codes = (1, 6, 7, 5)   # ecx, esi, edi, ebp
    index_codes = (2, 3, 0, 1)   # edx, ebx, eax, ecx
    code = bytearray()
    selector_fields, table_fields, selector_evidence, table_evidence = [], [], [], []
    for group, (guard, index) in enumerate(zip(guard_codes[:groups], index_codes[:groups])):
        start = len(code)
        code.extend((0x83, 0xF8 | guard, 0, 0x77, 0x10,
                     0x31, (index << 3) | index,
                     0x0F, 0xB6, 0x80 | (index << 3) | guard))
        selector_fields.append(start + 10)
        code.extend(bytes(4))
        code.extend((0xFF, 0x24, 0x85 | (index << 3)))
        table_fields.append(start + 17)
        code.extend(bytes(4))
        selector_evidence.append(start + 7)
        table_evidence.append(start + 14)
    if unknown_indirect:
        code.extend(b"\xff\xe0")
    else:
        if call_target is not None:
            code.extend(b"\xe8" + struct.pack("<i", call_target - (len(code) + 5)))
        code.append(0xC3)
    table_offsets, selector_offsets, gaps = [], [], []
    cursor = len(code)
    first_table = (cursor + 3) & ~3
    gaps.append(b"\x90" * (first_table - cursor))
    cursor = first_table
    for group in range(groups):
        table_offsets.append(cursor)
        selector_offsets.append(cursor + 4)
        cursor += 5
        if group + 1 < groups:
            next_table = (cursor + 3) & ~3
            gap = b"\x90" * (next_table - cursor)
            if group == 0 and gap2 is not None:
                # Pinned original spelling of the first inter-group gap; the
                # candidate reuses it unless candidate_gap2 mutates it.
                if len(gap2) != len(gap):
                    raise ValueError("gap2 must preserve the fixture extent")
                gap = gap2
            if group == 0 and candidate_gap2 is not None:
                # Candidate gap spelling is a same-length mutation of this
                # first inter-group gap; original source data stays pinned.
                if len(candidate_gap2) != len(gap):
                    raise ValueError("candidate_gap2 must preserve the fixture extent")
            gaps.append(gap)
            cursor = next_table

    # Absolute original operand values; these offsets deliberately preserve
    # the historical two-group fixture's code anchors at 7/14 and 28/35.
    for field, target in zip(selector_fields, selector_offsets):
        struct.pack_into("<I", code, field, BASE + target)
    for field, target in zip(table_fields, table_offsets):
        struct.pack_into("<I", code, field, BASE + target)
    if original_change is not None:
        offset, value = original_change
        code[offset] = value

    first_target = BASE + 21 if target1 is None else target1
    table_targets = [BASE + ((group + 1) * 21) for group in range(groups)]
    table_targets[0] = first_target
    if groups > 1 and default2 is not None:
        table_targets[1] = default2
    envelope_tail = bytearray(gaps[0])
    for group in range(groups):
        envelope_tail.extend(struct.pack("<I", table_targets[group]))
        envelope_tail.append(0)
        if group + 1 < groups:
            envelope_tail.extend(gaps[group + 1])
    envelope = bytes(code) + bytes(envelope_tail)

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
    struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)
    raw[512:512 + len(envelope)] = envelope
    fixups = (*selector_fields, *table_fields, *table_offsets)
    entries = [0x3000 + offset for offset in fixups]
    block_size = 8 + 2 * len(entries)
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1100, block_size)
    struct.pack_into("<II", raw, 768, 0x1000, block_size)
    for index, entry in enumerate(entries):
        struct.pack_into("<H", raw, 776 + index * 2, entry)

    candidate_code = bytearray(code)
    for field in (*selector_fields, *table_fields):
        candidate_code[field:field + 4] = bytes(4)
    if candidate_change is not None:
        offset, value = candidate_change
        candidate_code[offset] = value
    selectors = dict(candidate_selector_values or {})
    if groups > 1:
        selectors.setdefault(1, candidate_selector2)
    candidate_tail = bytearray(gaps[0])
    for group in range(groups):
        candidate_tail.extend(bytes(4))
        candidate_tail.append(selectors.get(group, 0))
        if group + 1 < groups:
            candidate_gap = gaps[group + 1]
            if group == 0 and candidate_gap2 is not None:
                candidate_gap = candidate_gap2
            candidate_tail.extend(candidate_gap)
    candidate_tail.extend(extra_tail)
    data = candidate_code + candidate_tail
    data += b"\x90" * (-len(data) % 16)

    symbol_indices = {}
    symbols = bytearray()
    symbols += b"_f\0\0\0\0\0\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
    symbols += b".text\0\0\0" + struct.pack("<IhHBB", 0, 1, 0, 3, 1)
    symbols += struct.pack("<IHHIHB", len(data), len(fixups) + int(extra_relocation),
                           0, 0, 0, 2) + bytes(3)
    for group in range(groups):
        for role, value in (("tbl", table_offsets[group]),
                            ("sel", selector_offsets[group]),
                            ("def", (group + 1) * 21)):
            name = f"{role}{group}"
            symbol_indices[name] = 3 + group * 3 + (0 if role == "tbl" else 1 if role == "sel" else 2)
            symbols += name.encode().ljust(8, b"\0") + struct.pack("<IhHBB", value, 1, 0, 6, 0)

    cell_targets = dict(candidate_cell_targets or {})
    if retarget_candidate:
        cell_targets[0] = min(1, groups - 1)
    relocations = []
    for group, field in enumerate(selector_fields):
        target_group = 1 if wrong_selector_anchor and group == 0 else group
        relocations.append((field, symbol_indices[f"sel{target_group}"], 6))
    for group, field in enumerate(table_fields):
        relocations.append((field, symbol_indices[f"tbl{group}"], 6))
    original_target_groups = {BASE + ((group + 1) * 21): group
                              for group in range(groups)}
    for group, table_offset in enumerate(table_offsets):
        original_target_group = original_target_groups.get(table_targets[group], group)
        target_group = cell_targets.get(group, original_target_group)
        relocations.append((table_offset, symbol_indices[f"def{target_group}"], 6))
    if extra_relocation:
        relocations.append((len(envelope) + 3, symbol_indices["def0"], 6))
    relocation_blob = b"".join(struct.pack("<IIH", *row) for row in relocations)
    data_at = 60
    relocation_at = data_at + len(data)
    symbol_at = relocation_at + len(relocation_blob)
    symbol_count = 3 + groups * 3
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbol_at, symbol_count, 0, 0)
    flags = 0x60501020
    section = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(data), data_at,
                                                relocation_at, 0, len(relocations), 0, flags))
    obj = Coff(header + section + data + relocation_blob + symbols + struct.pack("<I", 4))
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(bytes(raw), pin)
    owner = Origin(pin, "pe:va", BASE)
    rows = []
    for group in range(groups):
        rows.append(AssociatedDataBinding(
            owner, Span(BASE + table_offsets[group], BASE + table_offsets[group] + 4),
            "switch_targets", Origin(pin, "pe:va", BASE + table_evidence[group]),
            "fixture.cpp", group * 2 + 1))
        rows.append(AssociatedDataBinding(
            owner, Span(BASE + selector_offsets[group], BASE + selector_offsets[group] + 1),
            "switch_selector", Origin(pin, "pe:va", BASE + selector_evidence[group]),
            "fixture.cpp", group * 2 + 2))
    if duplicate_evidence:
        rows[-1] = AssociatedDataBinding(owner, rows[-1].span, rows[-1].kind,
                                         rows[1].evidence, rows[-1].unit, rows[-1].line)
    rows = [row for index, row in enumerate(rows) if index not in omit]
    return obj, obj.function("_f"), Span(BASE, BASE + len(code)), image, tuple(rows), {}


def test_explicit_same_comdat_table_and_selector_compare_exactly():
    obj, subject, span, image, bindings, targets = fixture()
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.PASS, (report["bytes"], report["references"],
                                              report.get("differing_offsets"), report.get("relocations"))
    assert report["source_body_span"] == {"start": BASE, "end": BASE + len(BODY)}
    assert report["comparison_span"] == {"start": BASE, "end": BASE + 29}
    assert report["linked_selection"] == report["runtime"] == Result.UNKNOWN


def test_one_exact_associated_fpo_child_is_retained_as_unknown_metadata():
    obj, subject, span, image, bindings, targets = fixture()
    obj = Coff(_with_fpo_child(obj.raw_data))
    report = compare_associated_code(obj, obj.function(subject.name), span, image, bindings, targets)
    assert report["result"] == Result.PASS
    witness = report["associated_metadata"]
    assert witness["format"] == "fpo_data"
    assert witness["code_size"] == report["comparison_span"]["end"] - span.start
    assert witness["section"] == 2 and witness["association_parent"] == 1
    assert witness["section_bytes"] == struct.pack(
        "<IIIHH", 0, witness["code_size"], 0, 0, 0).hex()
    assert report["original_metadata"] == report["linked_selection"] == Result.UNKNOWN


def test_fpo_child_must_describe_the_full_declared_code_and_table_envelope():
    obj, subject, span, image, bindings, targets = fixture()
    candidate = Coff(_with_fpo_child(obj.raw_data, code_size=30))
    with pytest.raises(ValueError, match="FPO metadata extent"):
        compare_associated_code(candidate, candidate.function(subject.name), span,
                                image, bindings, targets)


@pytest.mark.parametrize("damage", ["wrong_name", "wrong_flags", "wrong_relocation"])
def test_associated_proof_rejects_non_fpo_or_malformed_debug_children(damage):
    obj, subject, span, image, bindings, targets = fixture()
    raw = bytearray(_with_fpo_child(obj.raw_data))
    if damage == "wrong_name":
        raw[60:68] = b".xdata$x"
    elif damage == "wrong_flags":
        struct.pack_into("<I", raw, 60 + 36, 0x62501040)
    else:
        child_relocation, = struct.unpack_from("<I", raw, 60 + 24)
        struct.pack_into("<H", raw, child_relocation + 8, 6)
    with pytest.raises(ValueError):
        candidate = Coff(bytes(raw))
        compare_associated_code(candidate, candidate.function(subject.name), span,
                                image, bindings, targets)


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


def test_two_adjacent_pairs_require_and_prove_each_dispatch_independently():
    obj, subject, span, image, bindings, targets = multi_fixture()
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.PASS, (report.get("bytes"), report.get("references"),
                                               report.get("reason"))
    assert len(report["dispatch_extents"]) == 2
    assert [row["mode"] for row in report["dispatch_extents"]] == ["mapped-index"] * 2
    assert [row["target_count"] for row in report["dispatch_extents"]] == [1, 1]
    assert report["comparison_span"] == {"start": BASE, "end": BASE + 57}
    assert [row.span.start - BASE for row in bindings if row.kind == "switch_targets"] == [44, 52]


def test_two_group_default2_parameter_does_not_move_historical_data_offsets():
    obj, subject, span, image, bindings, targets = multi_fixture(default2=BASE + 21)
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.PASS
    assert [row.span.start - BASE for row in bindings if row.kind == "switch_targets"] == [44, 52]


def test_four_adjacent_pairs_compare_one_complete_envelope():
    obj, subject, span, image, bindings, targets = multi_fixture(groups=4)
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.PASS, (report.get("bytes"), report.get("references"),
                                               report.get("reason"))
    assert len(report["dispatch_extents"]) == 4
    assert [row["target_count"] for row in report["dispatch_extents"]] == [1, 1, 1, 1]
    assert report["comparison_span"] == {"start": BASE, "end": BASE + 117}
    assert [row.span.start - BASE for row in bindings if row.kind == "switch_targets"] == [88, 96, 104, 112]


def test_intergroup_hotpatch_gap_requires_exact_position_bytes_and_alignment():
    assert _is_intergroup_hotpatch_gap(b"\x8b\xff", b"\x8b\xff",
                                       0x1002, 0x1004, {0x1002}, {0x1004})
    damage = [
        (b"\x8b\xfe", b"\x8b\xfe", 0x1002, 0x1004, {0x1002}, {0x1004}),  # wrong register
        (b"\x66\x90", b"\x66\x90", 0x1002, 0x1004, {0x1002}, {0x1004}),  # prefixed NOP
        (b"\x8b\xff", b"\x89\xff", 0x1002, 0x1004, {0x1002}, {0x1004}),  # candidate byte
        (b"\x8b\xff", b"\x8b\xff", 0x1002, 0x1004, {0x1000}, {0x1004}),  # wrong location
        (b"\x8b\xff", b"\x8b\xff", 0x1003, 0x1005, {0x1003}, {0x1005}),  # bad alignment
    ]
    for original, candidate, left, right, selectors, tables in damage:
        assert not _is_intergroup_hotpatch_gap(original, candidate, left, right,
                                               selectors, tables)


def test_intergroup_lea_identity_gap_requires_exact_position_bytes_and_alignment():
    lea = bytes.fromhex("8d 49 00")
    assert _is_intergroup_lea_identity_gap(lea, lea, 0x1005, 0x1008, {0x1005}, {0x1008})
    damage = [
        (bytes.fromhex("8d 49 01"), bytes.fromhex("8d 49 01"), 0x1005, 0x1008, {0x1005}, {0x1008}),  # nonzero disp
        (bytes.fromhex("8d 4a 00"), bytes.fromhex("8d 4a 00"), 0x1005, 0x1008, {0x1005}, {0x1008}),  # not identity
        (lea, bytes.fromhex("8d 40 00"), 0x1005, 0x1008, {0x1005}, {0x1008}),  # candidate byte
        (lea, b"\x90\x90\x90", 0x1005, 0x1008, {0x1005}, {0x1008}),  # candidate NOP fill
        (lea, lea, 0x1005, 0x1008, {0x1004}, {0x1008}),  # left is not a selector end
        (lea, lea, 0x1005, 0x1008, {0x1005}, {0x100c}),  # right is not a table start
        (lea, lea, 0x1006, 0x1009, {0x1006}, {0x1009}),  # unaligned table
        (lea + b"\x90", lea + b"\x90", 0x1004, 0x1008, {0x1004}, {0x1008}),  # extra fill
    ]
    for original, candidate, left, right, selectors, tables in damage:
        assert not _is_intergroup_lea_identity_gap(original, candidate, left, right,
                                                   selectors, tables)


def test_intergroup_lea_identity_gap_between_selector_and_table_compares_exactly():
    lea = bytes.fromhex("8d 49 00")
    obj, subject, span, image, bindings, targets = multi_fixture(gap2=lea)
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.PASS, (report.get("bytes"), report.get("references"))
    assert report["comparison_span"] == {"start": BASE, "end": BASE + 57}


@pytest.mark.parametrize("candidate_gap", [bytes.fromhex("8d 40 00"), b"\x90\x90\x90"])
def test_intergroup_lea_identity_gap_candidate_mutation_never_passes(candidate_gap):
    obj, subject, span, image, bindings, targets = multi_fixture(
        gap2=bytes.fromhex("8d 49 00"), candidate_gap2=candidate_gap)
    with pytest.raises(ValueError, match="not a supported alignment form"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_intergroup_noncanonical_lea_gap_refuses():
    obj, subject, span, image, bindings, targets = multi_fixture(gap2=bytes.fromhex("8d 49 01"))
    with pytest.raises(ValueError, match="not a supported alignment form"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("omit", [(2, 3), (6, 7)])
def test_four_group_middle_and_last_group_omission_refuses(omit):
    obj, subject, span, image, bindings, targets = multi_fixture(groups=4, omit=omit)
    with pytest.raises(ValueError, match="cover every indexed"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("selector_row", [1, 3, 5, 7])
def test_four_group_selector_omission_refuses_for_each_group(selector_row):
    obj, subject, span, image, bindings, targets = multi_fixture(groups=4, omit=(selector_row,))
    with pytest.raises(ValueError, match="require one adjacent selector"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("group", [0, 1, 2, 3])
def test_four_group_cell_or_selector_damage_is_checked_per_group(group):
    obj, subject, span, image, bindings, targets = multi_fixture(
        groups=4, candidate_cell_targets={group: (group + 1) % 4})
    cell_report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert cell_report["result"] != Result.PASS
    obj, subject, span, image, bindings, targets = multi_fixture(
        groups=4, candidate_selector_values={group: 1})
    selector_report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert selector_report["result"] != Result.PASS


@pytest.mark.parametrize("omit", [(2, 3), (2,), (3,)])
def test_missing_group_or_selector_never_proves_the_remaining_dispatches(omit):
    obj, subject, span, image, bindings, targets = multi_fixture(omit=omit)
    with pytest.raises(ValueError, match="cover every indexed|require one adjacent|orphan"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_ambiguous_selector_evidence_is_rejected():
    obj, subject, span, image, bindings, targets = multi_fixture(duplicate_evidence=True)
    with pytest.raises(ValueError, match="evidence is ambiguous"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("original_change,reason", [
    ((3, 0x76), "guard"),       # signed JBE does not establish unsigned extent
    ((9, 0x96), "selector"),    # selector uses ESI rather than guarded ECX
    ((10, 0x31), "data start"), # selector absolute operand points elsewhere
    ((24, 0x76), "guard"),      # damage only the second group's guard
])
def test_each_group_requires_its_own_guard_register_and_anchor(original_change, reason):
    obj, subject, span, image, bindings, targets = multi_fixture(original_change=original_change)
    with pytest.raises(ValueError, match=reason):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_target_cell_must_land_on_original_instruction_boundary():
    obj, subject, span, image, bindings, targets = multi_fixture(target1=BASE + 22)
    with pytest.raises(ValueError, match="instruction boundary"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("target", [BASE + 28, BASE + 35])
def test_indirect_table_edge_cannot_bypass_another_groups_guard(target):
    obj, subject, span, image, bindings, targets = multi_fixture(target1=target)
    with pytest.raises(ValueError, match="unambiguous unsigned dominating guard"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_unmodeled_register_indirect_jump_refuses_the_complete_cfg():
    obj, subject, span, image, bindings, targets = multi_fixture(unknown_indirect=True)
    with pytest.raises(ValueError, match="unmodeled indirect JMP"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("kwargs", [
    {"retarget_candidate": True},
    {"wrong_selector_anchor": True},
    {"candidate_selector2": 1},
    {"candidate_change": (3, 0x76)},
    {"candidate_change": (9, 0x96)},
    {"candidate_gap2": b"\x90\x90\xcc"},
    {"extra_tail": b"\xc3"},
    {"extra_relocation": True},
])
def test_all_groups_and_envelope_remain_strict_under_candidate_damage(kwargs):
    obj, subject, span, image, bindings, targets = multi_fixture(**kwargs)
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] != Result.PASS, (kwargs, report)


def direct_multi_fixture(*, groups=2, omit=(), original_change=None,
                         candidate_change=None, target_override=None,
                         missing_highlow=None, cell_damage=None, cell_group=0,
                         candidate_shift=0, truncate=False,
                         clobber_group=None, unknown_indirect=False):
    """Independent four-cell direct dispatches, with no selector-map bytes."""
    assert groups in (1, 2, 4)
    registers = (2, 0, 3, 1)  # EDX, EAX, EBX, ECX
    body = bytearray()
    starts, jumps, fields = [], [], []
    for group, register in enumerate(registers[:groups]):
        starts.append(len(body))
        body.extend((0x83, 0xF8 | register, 3, 0x77, 0))
        if group == clobber_group:
            body.append(0x40 | register)  # INC guarded/index register
        jumps.append(len(body))
        body.extend((0xFF, 0x24, 0x85 | (register << 3)))
        fields.append(len(body))
        body.extend(bytes(4))
    if unknown_indirect:
        body.extend(b"\xff\xe0")
    cases = []
    for value in range(4):
        cases.append(len(body))
        body.extend(b"\xb8" + struct.pack("<I", value) + b"\xc3")
    for start in starts:
        body[start + 4] = cases[-1] - (start + 5)
    body_size = len(body)
    gap = b"\x90" * (-body_size % 4)
    table_start = body_size + len(gap)
    table_offsets = [table_start + group * 16 for group in range(groups)]
    for field, offset in zip(fields, table_offsets):
        struct.pack_into("<I", body, field, BASE + offset)
    if original_change is not None:
        offset, value = original_change
        if isinstance(value, bytes):
            body[offset:offset + len(value)] = value
        else:
            body[offset] = value
    destinations = []
    for group in range(groups):
        # Reenter another group at its CMP, never after its range check.
        values = [starts[group + 1] if group + 1 < groups else cases[0],
                  cases[1], cases[2], cases[2]]
        if target_override is not None and group == 0:
            values[0] = target_override
        destinations.append(values)
    original = bytes(body) + gap + b"".join(
        struct.pack("<I", BASE + value) for values in destinations for value in values)
    fixups = [*fields, *(offset + cell * 4 for offset in table_offsets for cell in range(4))]
    if missing_highlow is not None:
        fixups.remove(missing_highlow)
    pe = bytearray(1024)
    pe[:2] = b"MZ"
    struct.pack_into("<I", pe, 0x3C, 0x80)
    pe[0x80:0x84] = b"PE\0\0"
    struct.pack_into("<HHIIIHH", pe, 0x84, 0x14C, 1, 0, 0, 0, 224, 0)
    struct.pack_into("<H", pe, 0x98, 0x10B)
    struct.pack_into("<I", pe, 0x98 + 28, 0x400000)
    struct.pack_into("<I", pe, 0x98 + 92, 16)
    pe[0x178:0x180] = b".text\0\0\0"
    struct.pack_into("<IIII", pe, 0x180, 512, 0x1000, 512, 512)
    struct.pack_into("<I", pe, 0x178 + 36, 0x60000020)
    pe[512:512 + len(original)] = original
    struct.pack_into("<II", pe, 0x98 + 96 + 5 * 8, 0x1100, 8 + 2 * len(fixups))
    struct.pack_into("<II", pe, 768, 0x1000, 8 + 2 * len(fixups))
    for index, offset in enumerate(fixups):
        struct.pack_into("<H", pe, 776 + 2 * index, 0x3000 + offset)
    image_raw = bytes(pe)
    pin = hashlib.sha256(image_raw).hexdigest()
    image = PeImage(image_raw, pin)

    data = bytearray(body)
    for field in fields:
        data[field:field + 4] = bytes(4)
    if candidate_change is not None:
        data[candidate_change[0]] = candidate_change[1]
    data.extend(gap + b"\x90" * candidate_shift + bytes(16 * groups))
    shifted_tables = [offset + candidate_shift for offset in table_offsets]
    data.extend(b"\x90" * (-len(data) % 16))
    if truncate:
        data = data[:len(original) - 1]
    locals_ = [(f"tbl{group}", offset) for group, offset in enumerate(shifted_tables)]
    locals_ += [(f"dst{offset}", offset) for offset in sorted(set(sum(destinations, [])))]
    indices = {name: index + 3 for index, (name, _) in enumerate(locals_)}
    relocations = [(field, indices[f"tbl{group}"], 6) for group, field in enumerate(fields)]
    for group, offset in enumerate(shifted_tables):
        for cell, destination in enumerate(destinations[group]):
            relocations.append((offset + cell * 4, indices[f"dst{destination}"], 6))
    cell_at = len(fields) + cell_group * 4
    if cell_damage == "missing":
        del relocations[cell_at]
    elif cell_damage == "duplicate":
        relocations.append(relocations[cell_at])
    elif cell_damage == "relative":
        offset, index, _ = relocations[cell_at]
        relocations[cell_at] = (offset, index, 20)
    elif cell_damage == "nonlocal":
        offset, _, kind = relocations[cell_at]
        relocations[cell_at] = (offset, 0, kind)
    elif cell_damage == "retarget":
        offset, _, kind = relocations[cell_at]
        relocations[cell_at] = (offset, indices[f"dst{cases[1]}"], kind)
    symbols = bytearray(b"_f\0\0\0\0\0\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0))
    symbols.extend(b".text\0\0\0" + struct.pack("<IhHBB", 0, 1, 0, 3, 1))
    symbols.extend(struct.pack("<IHHIHB3x", len(data), len(relocations), 0, 0, 0, 2))
    for name, offset in locals_:
        symbols.extend(name.encode().ljust(8, b"\0") + struct.pack("<IhHBB", offset, 1, 0, 6, 0))
    relocation_blob = b"".join(struct.pack("<IIH", *row) for row in relocations)
    relocation_at = 60 + len(data)
    symbol_at = relocation_at + len(relocation_blob)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbol_at, 3 + len(locals_), 0, 0)
    section = b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(data), 60,
        relocation_at, 0, len(relocations), 0, 0x60501020)
    obj = Coff(header + section + data + relocation_blob + symbols + struct.pack("<I", 4))
    owner = Origin(pin, "pe:va", BASE)
    bindings = tuple(AssociatedDataBinding(owner, Span(BASE + offset, BASE + offset + 16),
        "switch_targets", Origin(pin, "pe:va", BASE + jumps[group]), "fixture.cpp", group + 1)
        for group, offset in enumerate(table_offsets) if group not in omit)
    return obj, obj.function("_f"), Span(BASE, BASE + body_size), image, bindings, {}


@pytest.mark.parametrize("groups", [2, 4])
def test_multiple_independent_direct_tables_prove_all_guards_and_cells(groups):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(groups=groups)
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.PASS, report
    assert report["source_body_span"] == {"start": span.start, "end": span.end}
    assert report["comparison_span"]["end"] == bindings[-1].span.end
    assert len(report["dispatch_extents"]) == groups
    assert all(row["mode"] == "direct-index" and row["target_count"] == 4
               and row["selector_count"] == 0 for row in report["dispatch_extents"])
    assert report["linked_selection"] == report["runtime"] == Result.UNKNOWN


@pytest.mark.parametrize("group", range(4))
def test_direct_group_omission_cannot_borrow_another_groups_proof(group):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(groups=4, omit=(group,))
    with pytest.raises(ValueError, match="cover every indexed"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("group", range(4))
@pytest.mark.parametrize("damage", ["signed", "bound", "clobber", "missing"])
def test_each_direct_group_requires_its_own_unsigned_unchanged_index(group, damage):
    change = (None if damage == "clobber" else
              (group * 12, b"\x90" * 3) if damage == "missing" else
              (group * 12 + (3 if damage == "signed" else 2),
               0x7F if damage == "signed" else 2))
    obj, subject, span, image, bindings, targets = direct_multi_fixture(
        groups=4, original_change=change, clobber_group=group if damage == "clobber" else None)
    with pytest.raises(ValueError, match="guard|index"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("target", [15, 17])  # second guard's branch / indexed jump
def test_direct_table_edge_cannot_bypass_another_groups_range_check(target):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(target_override=target)
    with pytest.raises(ValueError, match="unambiguous unsigned dominating guard"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("site", [8, 20, *range(48, 80, 4)])
def test_direct_anchor_and_every_original_cell_require_highlow(site):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(missing_highlow=site)
    with pytest.raises(ValueError, match="HIGHLOW"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("target,reason", [(25, "instruction boundary"), (1000, "escapes")])
def test_direct_cell_targets_are_bounded_instruction_entries(target, reason):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(target_override=target)
    with pytest.raises(ValueError, match=reason):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("damage", ["missing", "duplicate", "relative", "nonlocal"])
@pytest.mark.parametrize("group", range(4))
def test_direct_candidate_cell_evidence_is_mandatory_for_pass(damage, group):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(
        groups=4, cell_damage=damage, cell_group=group)
    reason = {"missing": "COFF relocation", "duplicate": "overlapping",
              "relative": "ABS32", "nonlocal": "same-section"}[damage]
    with pytest.raises(ValueError, match=reason):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("kwargs", [{"candidate_change": (25, 9)}, {"cell_damage": "retarget"}])
def test_direct_candidate_code_or_cell_damage_is_measured_fail(kwargs):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(**kwargs)
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == Result.FAIL


@pytest.mark.parametrize("groups", [2, 4])
def test_complete_shifted_direct_tables_are_boundary_fail_not_new_layout(groups):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(
        groups=groups, candidate_shift=4)
    report = compare_associated_code(obj, subject, span, image, bindings, targets)
    assert report["result"] == report["boundary"] == Result.FAIL
    assert report["bytes"] == report["references"] == Result.UNKNOWN
    assert report["candidate_relocations_outside_original"]
    assert report["comparison_span"]["end"] == bindings[-1].span.end
    assert any("layout remains unproved" in text for text in report["limitations"])


@pytest.mark.parametrize("kwargs", [{"truncate": True},
    {"candidate_shift": 4, "cell_damage": "duplicate"}])
def test_direct_boundary_failure_does_not_hide_short_or_malformed_candidates(kwargs):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(**kwargs)
    with pytest.raises(ValueError, match="ABS32|out-of-bound relocation"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("kwargs", [{"original_change": (3, 0x7F)},
    {"target_override": 15}, {"missing_highlow": 48}, {"omit": (1,)}])
def test_shifted_direct_candidate_never_bypasses_original_proofs(kwargs):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(candidate_shift=4, **kwargs)
    with pytest.raises(ValueError):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_direct_population_still_refuses_unmodeled_indirect_jumps():
    obj, subject, span, image, bindings, targets = direct_multi_fixture(unknown_indirect=True)
    with pytest.raises(ValueError, match="unmodeled indirect JMP"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_partial_selector_population_does_not_become_a_mixed_direct_population():
    obj, subject, span, image, bindings, targets = multi_fixture(omit=(1,))
    with pytest.raises(ValueError, match="require one adjacent selector"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_omitting_every_selector_does_not_turn_mapped_indices_into_direct_indices():
    obj, subject, span, image, bindings, targets = multi_fixture(omit=(1, 3))
    with pytest.raises(ValueError):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def anchor_fixture(mode):
    if mode == "mapped-single":
        return fixture()
    if mode == "mapped-multi":
        return multi_fixture()
    return direct_multi_fixture(groups=1 if mode == "direct-single" else 2)


def changed_anchor(obj, subject, binding, damage):
    """Mutate disposable synthetic COFF metadata, never qualified inputs."""
    raw = bytearray(obj.raw_data)
    section_header = 20 + (subject.section - 1) * 40
    data_at, relocation_at = struct.unpack_from("<II", raw, section_header + 20)
    symbol_at, = struct.unpack_from("<I", raw, 8)
    offset = binding.evidence.address - binding.owner.address
    instruction = next(_ENGINE.disasm(obj.sections[subject.section - 1].data[offset:], offset))
    field = offset + instruction.disp_offset
    relocations = obj.sections[subject.section - 1].relocations
    position, (_, target, _) = next((n, row) for n, row in enumerate(relocations) if row[0] == field)
    symbol = obj.symbols[target]
    if damage == "storage":
        raw[symbol_at + target * 18 + 16] = 2  # external, not a compiler-local label
    elif damage == "section":
        struct.pack_into("<h", raw, symbol_at + target * 18 + 12, 0)
    elif damage == "index":
        struct.pack_into("<I", raw, relocation_at + position * 10 + 4, subject.index)
        struct.pack_into("<I", raw, data_at + field, symbol.value)
    elif damage == "missing":
        struct.pack_into("<H", raw, relocation_at + position * 10 + 8, 0)
    elif damage == "duplicate":
        copied = raw[relocation_at + position * 10:relocation_at + (position + 1) * 10]
        raw[symbol_at:symbol_at] = copied
        struct.pack_into("<I", raw, 8, symbol_at + 10)
        struct.pack_into("<H", raw, section_header + 32, len(relocations) + 1)
        # These fixtures place the root section definition's auxiliary record at2.
        struct.pack_into("<H", raw, symbol_at + 10 + 2 * 18 + 4, len(relocations) + 1)
    elif damage == "local-addend":
        struct.pack_into("<I", raw, symbol_at + target * 18 + 8, symbol.value - 4)
        struct.pack_into("<I", raw, data_at + field, 4)
        raw[symbol_at + target * 18:symbol_at + target * 18 + 8] = b"variant\0"
    else:
        raise AssertionError(damage)
    return Coff(bytes(raw))


@pytest.mark.parametrize("mode", ["direct-single", "direct-multi", "mapped-single", "mapped-multi"])
@pytest.mark.parametrize("damage", ["storage", "section", "index", "missing", "duplicate"])
def test_unsupported_candidate_anchors_never_borrow_equal_address_proof(mode, damage):
    obj, subject, span, image, bindings, targets = anchor_fixture(mode)
    obj = changed_anchor(obj, subject, bindings[0], damage)
    if damage in ("storage", "index"):
        # These changes preserve every resolved operand byte. They used to
        # pass solely because the physical anchor's exception was discarded.
        with pytest.raises(ValueError, match="anchor proof is unsupported"):
            compare_associated_code(obj, obj.function(subject.name), span, image, bindings, targets)
    else:
        try:
            report = compare_associated_code(obj, obj.function(subject.name), span, image, bindings, targets)
        except ValueError:
            return
        assert report["result"] != Result.PASS
        assert report["candidate_anchors"] == Result.UNKNOWN
        assert report["unsupported_candidate_anchors"]


@pytest.mark.parametrize("mode", ["direct-single", "direct-multi", "mapped-single", "mapped-multi"])
def test_legitimate_local_anchor_names_and_addends_still_pass(mode):
    obj, subject, span, image, bindings, targets = anchor_fixture(mode)
    obj = changed_anchor(obj, subject, bindings[0], "local-addend")
    report = compare_associated_code(obj, obj.function(subject.name), span, image, bindings, targets)
    assert report["result"] == Result.PASS


def test_known_code_mismatch_retains_fail_and_unsupported_anchor_dimension():
    obj, subject, span, image, bindings, targets = direct_multi_fixture(candidate_change=(25, 9))
    obj = changed_anchor(obj, subject, bindings[0], "storage")
    report = compare_associated_code(obj, obj.function(subject.name), span, image, bindings, targets)
    assert report["result"] == report["bytes"] == Result.FAIL
    assert report["candidate_anchors"] == report["references"] == Result.UNKNOWN
    assert "local same-section" in report["unsupported_candidate_anchors"][0]["reason"]


@pytest.mark.parametrize("groups", [1, 2, 4])
@pytest.mark.parametrize("entry", ["branch", "dispatch"])
def test_direct_call_cannot_enter_after_another_dispatchs_range_check(groups, entry):
    guard = 12 if groups > 1 else 0
    target = guard + (3 if entry == "branch" else 5)
    call = groups * 12 + 6
    obj, subject, span, image, bindings, targets = direct_multi_fixture(
        groups=groups, original_change=(call, b"\xe8" + struct.pack("<i", target - (call + 5))))
    with pytest.raises(ValueError, match="dominating guard"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("groups", [1, 2, 4])
@pytest.mark.parametrize("entry", ["compare", "ordinary-case"])
def test_direct_call_to_range_check_or_ordinary_code_does_not_blanket_refuse(groups, entry):
    guard = 12 if groups > 1 else 0
    target = guard if entry == "compare" else groups * 12 + 12
    call = groups * 12 + 6
    obj, subject, span, image, bindings, targets = direct_multi_fixture(
        groups=groups, original_change=(call, b"\xe8" + struct.pack("<i", target - (call + 5))))
    assert compare_associated_code(obj, subject, span, image, bindings, targets)["result"] == Result.PASS


@pytest.mark.parametrize("target", [24, 35])  # second JA / indexed JMP
@pytest.mark.parametrize("groups", [2, 4])
def test_selector_paired_dispatch_also_refuses_direct_call_guard_bypass(groups, target):
    obj, subject, span, image, bindings, targets = multi_fixture(groups=groups, call_target=target)
    with pytest.raises(ValueError, match="dominating guard"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("groups", [2, 4])
@pytest.mark.parametrize("entry", ["compare", "ordinary-return"])
def test_selector_pairs_allow_direct_calls_to_guard_or_ordinary_return(groups, entry):
    target = 21 if entry == "compare" else groups * 21 + 5
    obj, subject, span, image, bindings, targets = multi_fixture(groups=groups, call_target=target)
    assert compare_associated_code(obj, subject, span, image, bindings, targets)["result"] == Result.PASS


@pytest.mark.parametrize("opcode", [0xE0, 0xE1, 0xE2])  # LOOPNE/LOOPE/LOOP
@pytest.mark.parametrize("entry", [15, 17])  # second JA / indexed JMP
@pytest.mark.parametrize("groups", [2, 4])
def test_loop_family_cannot_enter_another_dispatch_after_its_guard(opcode, entry, groups):
    source = groups * 12 + 6
    code = bytes((opcode, (entry - (source + 2)) & 255, 0x90, 0x90, 0x90))
    obj, subject, span, image, bindings, targets = direct_multi_fixture(
        groups=groups, original_change=(source, code))
    with pytest.raises(ValueError, match="dominating guard"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


@pytest.mark.parametrize("opcode", [0xE0, 0xE1, 0xE2])
@pytest.mark.parametrize("position", ["before", "after"])
def test_loop_family_is_a_control_transfer_in_each_guard_corridor(opcode, position):
    dispatch = bytes.fromhex("ff 24 85 00 00 00 00")
    loop = bytes((opcode, 0))
    if position == "before":
        code = bytes.fromhex("83 f8 00") + loop + bytes((0x77, len(dispatch))) + dispatch + b"\xc3"
    else:
        code = bytes.fromhex("83 f8 00") + bytes((0x77, len(loop) + len(dispatch))) + loop + dispatch + b"\xc3"
    instructions = list(_ENGINE.disasm(code, BASE))
    table_jump = next(item for item in instructions if item.mnemonic == "jmp")
    with pytest.raises(ValueError, match="guard"):
        _guard_for_dispatch(instructions, table_jump, Span(BASE, BASE + len(code)))


@pytest.mark.parametrize("target", [13, 14, 16, 18, 23])
@pytest.mark.parametrize("opcode", [0xE8, 0xE9])  # near CALL / JMP
def test_incoming_direct_control_cannot_create_an_overlapping_guard_stream(target, opcode):
    obj, subject, span, image, bindings, targets = direct_multi_fixture(
        original_change=(30, bytes((opcode,)) + struct.pack("<i", target - 35)))
    with pytest.raises(ValueError, match="instruction boundary"):
        compare_associated_code(obj, subject, span, image, bindings, targets)


def test_ordinary_direct_call_outside_parent_body_retains_supported_proof():
    obj, subject, span, image, bindings, targets = direct_multi_fixture(
        original_change=(30, b"\xe8" + struct.pack("<i", 0x2000 - 35)))
    assert compare_associated_code(obj, subject, span, image, bindings, targets)["result"] == Result.PASS


def test_far_direct_call_requires_segment_context_not_guessed_linear_entry():
    code = (bytes.fromhex("83 f8 00 77 07 ff 24 85 00 00 00 00 c3")
            + b"\x9a" + struct.pack("<IH", BASE + 5, 0x23))
    instructions = list(_ENGINE.disasm(code, BASE))
    dispatch = next(item for item in instructions if item.mnemonic == "jmp")
    with pytest.raises(ValueError, match="segment-context"):
        _guard_for_dispatch(instructions, dispatch, Span(BASE, BASE + len(code)),
                            indirect_targets=frozenset())


def test_ordinary_indirect_callback_outside_guard_corridors_is_not_blanket_refused():
    obj, subject, span, image, bindings, targets = direct_multi_fixture(
        original_change=(30, bytes.fromhex("ff d0 90 90 90")))
    assert compare_associated_code(obj, subject, span, image, bindings, targets)["result"] == Result.PASS
