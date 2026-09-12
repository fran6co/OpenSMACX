"""Synthetic embedded PE COFF debug symbols, never owner/placement authority."""

from dataclasses import FrozenInstanceError
import hashlib
import struct

import pytest

from match_recovery.pe_coff import PeImage, Symbol
from match_recovery.pe_debug import linked_coff_symbols
from test_binary import pe_fixture


DIRECTORY = 0x98 + 96 + 6 * 8
DEBUG = 0x300
PAYLOAD = 0x400


def embedded_image(symbols=None, *, mapped=False, lines=(), header_table=True):
    """Rows are (name, value_RVA, section_ordinal, type, storage, aux_bytes)."""
    if symbols is None:
        symbols = [("_private", 0x1000, 1, 0, 3, b"")]
    strings, records = bytearray(), bytearray()
    for name, value, section, kind, storage, auxiliary in symbols:
        assert len(auxiliary) % 18 == 0
        encoded = name.encode("ascii")
        if len(encoded) > 8:
            label = struct.pack("<II", 0, 4 + len(strings))
            strings.extend(encoded + b"\0")
        else:
            label = encoded.ljust(8, b"\0")
        records.extend(label + struct.pack("<IhHBB", value, section, kind, storage, len(auxiliary) // 18) + auxiliary)
    count = len(records) // 18
    string_table = struct.pack("<I", 4 + len(strings)) + strings
    lines_at = 32 + len(records) + len(string_table) if lines else 0
    payload = (struct.pack("<8I", count, 32, len(lines), lines_at, 0, 0, 0x1000, 0x1200)
               + records + string_table + b"".join(struct.pack("<IH", *row) for row in lines))
    raw = bytearray(pe_fixture())
    raw.extend(bytes(PAYLOAD + len(payload) - len(raw)))
    raw[PAYLOAD:] = payload
    struct.pack_into("<I", raw, 0x98 + 60, 0x200)
    struct.pack_into("<I", raw, 0x178 + 36, 0xC0000040)
    if mapped:
        size = len(raw) - 0x200
        struct.pack_into("<I", raw, 0x178 + 8, size)
        struct.pack_into("<I", raw, 0x178 + 16, size)
    struct.pack_into("<II", raw, DIRECTORY, 0x1100, 28)
    struct.pack_into("<IIHHIIII", raw, DEBUG, 0, 7, 0, 0, 1, len(payload), 0x1200 if mapped else 0, PAYLOAD)
    if header_table and count:
        struct.pack_into("<II", raw, 0x80 + 12, PAYLOAD + 32, count)
    return bytes(raw)


def parse(raw):
    return linked_coff_symbols(PeImage(raw, hashlib.sha256(raw).hexdigest()))


@pytest.mark.parametrize("mapped", [False, True])
@pytest.mark.parametrize("header_table", [False, True])
def test_overlay_and_mapped_payloads_keep_linked_rvas(mapped, header_table):
    assert parse(embedded_image(mapped=mapped, header_table=header_table)) == (Symbol(0, "_private", 0x1000, 1, 0, 3),)


def test_long_names_duplicates_weak_and_nonplacement_records_are_not_reinterpreted():
    rows = [("a_very_long_private_symbol", 0x1001, 1, 0, 3, b""),
            ("same", 0x1002, 1, 32, 2, b""), ("same", 0x1003, 1, 32, 105, b""),
            ("absolute", 0xFFFFFFFF, -1, 0, 2, b""), ("undefined", 0, 0, 0, 2, b"")]
    parsed = parse(embedded_image(rows))
    assert [(s.name, s.value, s.section, s.storage) for s in parsed] == [(r[0], r[1], r[2], r[4]) for r in rows]
    with pytest.raises(FrozenInstanceError):
        parsed[0].value = 0


def test_file_auxiliaries_are_not_symbols_or_owner_scopes():
    rows = [(".file", 0, -2, 0, 103, b"file.cpp\0".ljust(36, b"\0")),
            ("_private", 0x1000, 1, 0, 3, b"")]
    parsed = parse(embedded_image(rows, lines=((3, 0), (0x1000, 1))))
    assert parsed == (Symbol(0, ".file", 0, -2, 0, 103), Symbol(3, "_private", 0x1000, 1, 0, 3))
    assert all(not hasattr(row, "owner") for row in parsed)


def test_absent_debug_and_valid_empty_table_have_no_symbols():
    assert parse(pe_fixture()) == ()
    assert parse(embedded_image([])) == ()
    raw = bytearray(pe_fixture())
    struct.pack_into("<I", raw, 0x98 + 92, 0)
    assert parse(bytes(raw)) == ()


@pytest.mark.parametrize("offset,value", [
    (DIRECTORY, 0), (DIRECTORY + 4, 0), (DIRECTORY + 4, 27), (DIRECTORY + 4, 28 * 4097),
    (DIRECTORY, 0xFFFFFFFF), (DEBUG, 1), (DEBUG + 16, 31), (DEBUG + 16, 0xFFFFFFFF),
    (DEBUG + 20, 0x1000), (DEBUG + 24, 0), (DEBUG + 24, 0xFFFFFFFF),
    (PAYLOAD, 1_000_001), (PAYLOAD + 4, 0), (PAYLOAD + 4, 31), (PAYLOAD + 4, 0xFFFFFFFF),
    (PAYLOAD + 8, 1_000_001), (PAYLOAD + 12, 32),
    (PAYLOAD + 32 + 18, 0), (PAYLOAD + 32 + 18, 3), (PAYLOAD + 32 + 18, 0xFFFFFFFF),
    (0x80 + 12, 0), (0x80 + 12, PAYLOAD + 33), (0x80 + 16, 0), (0x80 + 16, 2),
])
def test_corrupt_extents_identity_or_tables_refuse_without_partial_symbols(offset, value):
    raw = bytearray(embedded_image())
    struct.pack_into("<I", raw, offset, value)
    with pytest.raises(ValueError):
        parse(bytes(raw))


@pytest.mark.parametrize("value,section", [(0x400000, 1), (0xFFF, 1), (0x1200, 1), (0xFFFFFFFF, 1), (0x1000, 2), (0, -3)])
def test_positive_section_values_are_rvas_inside_that_actual_section(value, section):
    with pytest.raises(ValueError):
        parse(embedded_image([("_private", value, section, 0, 3, b"")]))


def test_mapped_auxiliary_metadata_remains_explicitly_unsupported():
    with pytest.raises(ValueError, match="auxiliary metadata"):
        parse(embedded_image([("_private", 0x1000, 1, 0, 3, bytes(18))]))


def test_truncated_auxiliary_record_cannot_become_a_symbol():
    raw = bytearray(embedded_image())
    raw[PAYLOAD + 32 + 17] = 1
    with pytest.raises(ValueError, match="auxiliary extent"):
        parse(bytes(raw))


@pytest.mark.parametrize("offset", [0, 3, 0xFFFFFFFF])
def test_long_name_offsets_require_the_declared_string_table(offset):
    raw = bytearray(embedded_image())
    struct.pack_into("<II", raw, PAYLOAD + 32, 0, offset)
    with pytest.raises(ValueError, match="string offset"):
        parse(bytes(raw))


@pytest.mark.parametrize("name", [b"a\0bad\0\0\0", b"\xffinvalid"])
def test_short_symbol_names_are_unambiguous_lossless_ascii(name):
    raw = bytearray(embedded_image())
    raw[PAYLOAD + 32:PAYLOAD + 40] = name
    with pytest.raises(ValueError, match="symbol name|padding"):
        parse(bytes(raw))


@pytest.mark.parametrize("name", ["\x7fADVAPI32_NULL_THUNK_DATA", "\x01local", "a name"])
def test_ascii_control_and_space_names_are_preserved_without_inferred_identity(name):
    rows = parse(embedded_image([(name, 0x1000, 1, 0, 2, b"")]))
    assert rows == (Symbol(0, name, 0x1000, 1, 0, 2),)


def test_unterminated_long_name_cannot_read_outside_payload():
    raw = bytearray(embedded_image([("a_very_long_private_name", 0x1000, 1, 0, 3, b"")]))
    raw[-1] = 65
    with pytest.raises(ValueError, match="unterminated"):
        parse(bytes(raw))


@pytest.mark.parametrize("record", [(1, 0), (0x1200, 9)])
def test_line_records_cannot_reference_missing_symbols_or_unmapped_rvas(record):
    with pytest.raises(ValueError, match="line"):
        parse(embedded_image(lines=(record,)))


def test_line_record_cannot_target_file_auxiliary():
    with pytest.raises(ValueError, match="auxiliary symbol"):
        parse(embedded_image([(".file", 0, -2, 0, 103, bytes(18))], lines=((1, 0),)))


def test_line_table_overlap_refuses():
    raw = bytearray(embedded_image(lines=((0, 0),)))
    struct.pack_into("<I", raw, PAYLOAD + 12, PAYLOAD - 1)
    with pytest.raises(ValueError):
        parse(bytes(raw))


def test_multiple_distinct_coff_payloads_are_ambiguous_even_when_equal():
    raw = bytearray(embedded_image())
    second_at = len(raw)
    payload = raw[PAYLOAD:]
    raw.extend(payload)
    struct.pack_into("<II", raw, DIRECTORY, 0x1100, 56)
    struct.pack_into("<IIHHIIII", raw, DEBUG + 28, 0, 0, 0, 0, 1, len(payload), 0, second_at)
    with pytest.raises(ValueError, match="multiple"):
        parse(bytes(raw))


def test_duplicate_payload_alias_is_not_a_second_independent_table():
    raw = bytearray(embedded_image())
    struct.pack_into("<II", raw, DIRECTORY, 0x1100, 56)
    raw[DEBUG + 28:DEBUG + 56] = raw[DEBUG:DEBUG + 28]
    with pytest.raises(ValueError, match="overlapping"):
        parse(bytes(raw))


def test_other_debug_format_is_not_misread_as_coff():
    raw = bytearray(embedded_image(header_table=False))
    struct.pack_into("<I", raw, DEBUG + 12, 2)
    assert parse(bytes(raw)) == ()


def test_pe_header_cannot_hide_a_coff_table_by_removing_debug_kind():
    raw = bytearray(embedded_image())
    struct.pack_into("<I", raw, DEBUG + 12, 2)
    with pytest.raises(ValueError, match="lack.*COFF debug"):
        parse(bytes(raw))


def test_overlay_payload_cannot_alias_runtime_section_bytes():
    raw = bytearray(embedded_image(mapped=True))
    struct.pack_into("<I", raw, DEBUG + 20, 0)
    with pytest.raises(ValueError, match="unmapped.*overlaps"):
        parse(bytes(raw))


@pytest.mark.parametrize("offset", [0x40, 0x1A0])
def test_overlay_payload_cannot_alias_pe_headers_or_declared_padding(offset):
    raw = bytearray(embedded_image())
    payload = raw[PAYLOAD:]
    raw[offset:offset + len(payload)] = payload
    struct.pack_into("<I", raw, DEBUG + 24, offset)
    struct.pack_into("<I", raw, 0x80 + 12, offset + 32)
    with pytest.raises(ValueError, match="overlapping PE debug metadata"):
        parse(bytes(raw))


@pytest.mark.parametrize("size", [0, 0x19F, 0x201, 0xFFFFFFFF])
def test_invalid_declared_pe_header_extent_refuses(size):
    raw = bytearray(embedded_image())
    struct.pack_into("<I", raw, 0x98 + 60, size)
    with pytest.raises(ValueError, match="header"):
        parse(bytes(raw))
