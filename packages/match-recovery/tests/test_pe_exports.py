"""Physical PE export-table coverage, including unnamed entries and holes."""

from dataclasses import FrozenInstanceError
import hashlib
import struct

import pytest

from match_recovery.pe_coff import PeImage
from match_recovery.pe_exports import PeExport, exports
from test_binary import pe_fixture


DIRECTORY = 0x98 + 96
HEADER = 0x400  # RVA 0x1200.
EAT = 0x440
NAMES = 0x480
ORDINALS = 0x4C0
STRINGS = 0x500
MODULE = 0x5F0
FORWARDER = 0x600


def export_image(values=(0x1000, 0x1008), *, names=(("Alpha", 0), ("Beta", 1)),
                 base=1, forwarder=b"other.Target\0", absolute_site=None):
    raw = bytearray(pe_fixture(absolute_site=absolute_site))
    raw.extend(bytes(0x1200 - len(raw)))
    # One section: 0x1000 raw bytes and an extra 0x200 of virtual storage.
    struct.pack_into("<IIII", raw, 0x180, 0x1200, 0x1000, 0x1000, 0x200)
    struct.pack_into("<I", raw, 0x178 + 36, 0xC0000040)
    struct.pack_into("<II", raw, DIRECTORY, 0x1200, 0x600)
    struct.pack_into("<IIHHIIIIIII", raw, HEADER, 0, 0, 0, 0, 0x13F0, base, len(values), len(names),
                     0x1240 if values else 0, 0x1280 if names else 0, 0x12C0 if names else 0)
    for index, value in enumerate(values):
        struct.pack_into("<I", raw, EAT + index * 4, value)
    at = STRINGS
    for index, (name, ordinal) in enumerate(names):
        encoded = name.encode("ascii") + b"\0"
        raw[at:at + len(encoded)] = encoded
        struct.pack_into("<I", raw, NAMES + index * 4, at + 0xE00)
        struct.pack_into("<H", raw, ORDINALS + index * 2, ordinal)
        at += len(encoded)
    raw[MODULE:MODULE + 12] = b"fixture.exe\0"
    raw[FORWARDER:FORWARDER + len(forwarder)] = forwarder
    return bytes(raw)


def parse(raw):
    return exports(PeImage(raw, hashlib.sha256(raw).hexdigest()))


def test_complete_named_exports_preserve_ordinals_and_addresses():
    rows = parse(export_image(base=7))
    assert rows == (PeExport(0, 7, ("Alpha",), 0x1000, 0x401000, None),
                    PeExport(1, 8, ("Beta",), 0x1008, 0x401008, None))
    with pytest.raises(FrozenInstanceError):
        rows[0].address = 0


def test_named_aliases_and_repeated_targets_are_not_collapsed():
    rows = parse(export_image((0x1000, 0x1000), names=(("Alias", 0), ("Alpha", 0), ("Other", 1))))
    assert len(rows) == 2 and rows[0].names == ("Alias", "Alpha") and rows[1].names == ("Other",)
    assert rows[0].address == rows[1].address == 0x401000


def test_ordinal_only_exports_and_holes_are_all_retained():
    rows = parse(export_image((0, 0x1000, 0, 0x1009), names=(), base=0))
    assert [row.ordinal for row in rows] == [0, 1, 2, 3]
    assert [row.address for row in rows] == [None, 0x401000, None, 0x401009]
    assert all(row.names == () for row in rows)


def test_mixed_named_and_ordinal_only_exports_are_complete():
    rows = parse(export_image((0x1000, 0x1008, 0x1010), names=(("Only", 1),)))
    assert [row.names for row in rows] == [(), ("Only",), ()]
    assert len(rows) == 3


@pytest.mark.parametrize("target", [0x1000, 0x1FFF, 0x2000, 0x21FF])
def test_direct_data_or_virtual_storage_targets_do_not_require_code_bytes(target):
    row, = parse(export_image((target,), names=()))
    assert row.address == 0x400000 + target


@pytest.mark.parametrize("forwarder", [b"other.Target\0", b"other.#27\0", b"other.#0\0"])
def test_forwarders_are_bounded_strings_not_local_targets(forwarder):
    row, = parse(export_image((0x1400,), names=(), forwarder=forwarder))
    assert row.rva == 0x1400 and row.forwarder == forwarder[:-1].decode()
    assert row.address is None


def test_empty_valid_table_and_absent_directory_are_empty():
    assert parse(export_image((), names=())) == ()
    assert parse(pe_fixture()) == ()
    raw = bytearray(pe_fixture())
    struct.pack_into("<I", raw, 0x98 + 92, 0)
    assert parse(bytes(raw)) == ()


@pytest.mark.parametrize("at,value", [
    (DIRECTORY, 0), (DIRECTORY + 4, 0), (DIRECTORY + 4, 39),
    (DIRECTORY, 0xFFFFFFF0), (DIRECTORY + 4, 0xFFFFFFFF),
    (HEADER, 1), (HEADER + 12, 0), (HEADER + 12, 0x2000),
    (HEADER + 16, 0xFFFFFFFF), (HEADER + 20, 65537), (HEADER + 24, 65537),
    (HEADER + 28, 0), (HEADER + 28, 0x1FFE), (HEADER + 32, 0),
    (HEADER + 32, 0x1FFE), (HEADER + 36, 0), (HEADER + 36, 0x1FFF),
    (NAMES, 0), (NAMES, 0x2000), (EAT, 0x2200), (EAT, 0xFFFFFFFF),
    (EAT, 0xFF), (EAT, 0),
])
def test_corrupt_or_unbacked_metadata_never_becomes_partial_or_empty_success(at, value):
    raw = bytearray(export_image())
    struct.pack_into("<I", raw, at, value)
    with pytest.raises(ValueError):
        parse(bytes(raw))


def test_name_ordinal_must_index_the_complete_eat():
    raw = bytearray(export_image())
    struct.pack_into("<H", raw, ORDINALS, 2)
    with pytest.raises(ValueError, match="ordinal.*outside"):
        parse(bytes(raw))


@pytest.mark.parametrize("names", [(("Same", 0), ("Same", 1)), (("Zulu", 0), ("Alpha", 1))])
def test_export_name_array_must_be_unique_and_sorted(names):
    with pytest.raises(ValueError, match="unique.*ordered"):
        parse(export_image(names=names))


@pytest.mark.parametrize("at", [MODULE, STRINGS])
@pytest.mark.parametrize("value", [b"\0", b"\xff\0", b"\x1f\0", b"\x7f\0"])
def test_names_must_be_nonempty_printable_ascii(at, value):
    raw = bytearray(export_image())
    raw[at:at + len(value)] = value
    with pytest.raises(ValueError):
        parse(bytes(raw))


def test_string_must_terminate_before_end_of_raw_backing():
    raw = bytearray(export_image())
    struct.pack_into("<I", raw, NAMES, 0x1FFC)
    raw[-4:] = b"ABCD"
    with pytest.raises(ValueError, match="unterminated"):
        parse(bytes(raw))


@pytest.mark.parametrize("text", [b"NoSeparator\0", b".Target\0", b"other.\0", b"other.#\0",
                                  b"other.#-1\0", b"other.#65536\0", b"other.#12x\0"])
def test_malformed_forwarders_refuse_the_entire_inventory(text):
    with pytest.raises(ValueError, match="forwarder"):
        parse(export_image((0x1000, 0x1400), forwarder=text))


def test_forwarder_string_cannot_terminate_beyond_export_directory():
    raw = bytearray(export_image((0x1400,), names=()))
    struct.pack_into("<II", raw, DIRECTORY, 0x1200, 0x204)
    with pytest.raises(ValueError, match="unterminated"):
        parse(bytes(raw))


@pytest.mark.parametrize("at", [HEADER + 20, EAT, NAMES, ORDINALS, MODULE, STRINGS, FORWARDER])
def test_loader_relocations_cannot_change_interpreted_export_metadata(at):
    raw = export_image((0x1000, 0x1400), absolute_site=at - 0x200)
    with pytest.raises(ValueError, match="loader base relocations"):
        parse(raw)


@pytest.mark.parametrize("count,names,field", [(0, (), 28), (0, (), 32), (0, (), 36), (1, (), 32), (1, (), 36)])
def test_absent_array_counts_cannot_hide_nonzero_array_pointers(count, names, field):
    raw = bytearray(export_image((0x1000,) * count, names=names))
    struct.pack_into("<I", raw, HEADER + field, 0x1500)
    with pytest.raises(ValueError, match="contradictory"):
        parse(bytes(raw))


def test_ordinal_array_and_eat_can_live_outside_export_directory():
    raw = bytearray(export_image())
    # Only forwarder strings are mandated to remain in the declared export
    # directory; ordinary arrays may be separately placed and raw-backed.
    raw[0xA00:0xA08] = raw[EAT:EAT + 8]
    raw[0xA10:0xA14] = raw[ORDINALS:ORDINALS + 4]
    struct.pack_into("<I", raw, HEADER + 28, 0x1800)
    struct.pack_into("<I", raw, HEADER + 36, 0x1810)
    assert len(parse(bytes(raw))) == 2


def test_relocation_to_separately_placed_eat_is_still_rejected():
    raw = bytearray(export_image(absolute_site=0x800))
    raw[0xA00:0xA08] = raw[EAT:EAT + 8]
    struct.pack_into("<I", raw, HEADER + 28, 0x1800)
    with pytest.raises(ValueError, match="loader base relocations"):
        parse(bytes(raw))
