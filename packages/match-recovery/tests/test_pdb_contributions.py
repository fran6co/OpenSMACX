"""Physical image-bound V60 records, without COFF ownership guessing."""

from dataclasses import FrozenInstanceError
from hashlib import sha256
import struct

import pytest

from match_recovery.pdb_contributions import bound_contributions
from match_recovery.pe_coff import PeImage
from test_msf import small_msf
from test_pdb_fixups import parts as fixup_parts


def contribution(section=1, offset=16, size=8, flags=0x60302020, module=0, data_crc=0, relocation_crc=0):
    return struct.pack("<H2xiiIH2xII", section, offset, size, flags, module, data_crc, relocation_crc)


def module(name=b"app.obj", object_name=b"app.obj", *, first=None, stream=0xFFFF, lengths=(0, 0, 0)):
    header = bytearray(64)
    header[4:32] = contribution(section=0xFFFF) if first is None else first
    struct.pack_into("<Hiii", header, 34, stream, *lengths)
    raw = bytes(header) + name + b"\0" + object_name + b"\0"
    return raw + bytes((-len(raw)) % 4)


def parts(modules=None, contributions=None):
    raw, streams = fixup_parts()
    mods = module() if modules is None else modules
    records = contribution() if contributions is None else contributions
    hdr = bytearray(streams[3][:64])
    struct.pack_into("<II", hdr, 24, len(mods), 4 + len(records))
    streams[3] = bytes(hdr) + mods + struct.pack("<I", 0xF12EBA2D) + records + streams[3][64:]
    return raw, streams


def bind(raw, streams, **kwargs):
    return bound_contributions(PeImage(raw, sha256(raw).hexdigest()), small_msf(streams), **kwargs)


def test_full_table_preserved_separately_from_module_first_contribution():
    first = contribution()
    second = contribution(offset=32, size=4, data_crc=123, relocation_crc=456)
    value = bind(*parts(module(first=first), first + second))
    assert len(value.modules) == 1 and len(value.contributions) == 2
    assert value.modules[0].first == value.contributions[0]
    assert value.contributions[1].offset == 32 and value.contributions[1].data_crc == 123
    assert value.contributions[1].relocation_crc == 456
    assert value.contributions[0].flags == 0x60302020  # No undocumented bit masking.
    report = value.report()
    assert report["enumeration"] == "pass"
    assert report["result"] == report["allocation_isolation"] == "unknown"
    assert report["physical_object_binding"] == report["retained_storage_coverage"] == "unknown"
    report["modules"].clear()
    report["contributions"][0]["size"] = 200
    assert len(value.report()["modules"]) == 1 and value.report()["contributions"][0]["size"] == 8
    with pytest.raises(FrozenInstanceError): value.modules[0].name = "other"
    with pytest.raises(FrozenInstanceError): value.contributions[0].size = 200


def test_duplicate_intersecting_intervals_and_names_keep_multiplicity():
    records = contribution() + contribution() + contribution(offset=20, module=1)
    value = bind(*parts(module() + module(), records))
    assert len(value.modules) == 2 and len(value.contributions) == 3
    assert value.contributions[0] == value.contributions[1]
    assert value.report()["allocation_isolation"] == "unknown"


@pytest.mark.parametrize("has_module", [False, True])
def test_present_empty_table_is_not_missing_or_complete_storage(has_module):
    value = bind(*parts(module() if has_module else b"", b""))
    assert value.contributions == ()
    assert value.report()["enumeration"] == "pass" and value.report()["retained_storage_coverage"] == "unknown"


def test_nil_module_first_does_not_erase_its_live_table_entries():
    value = bind(*parts())
    assert value.modules[0].first is None and len(value.contributions) == 1


@pytest.mark.parametrize("field,values", [("section", [0, 2, 0xFFFF]), ("offset", [-1, 512, 0x7FFFFFFF]),
    ("size", [0, -1, 513, 0x7FFFFFFF]), ("module", [1, 0xFFFF])])
def test_invalid_section_module_and_signed_extents_refuse(field, values):
    for value in values:
        with pytest.raises(ValueError): bind(*parts(contributions=contribution(**{field: value})))


def test_an_in_bounds_row_cannot_point_at_another_modules_first_entry():
    mods = module(first=contribution(module=1)) + module()
    with pytest.raises(ValueError, match="another module"): bind(*parts(mods))


@pytest.mark.parametrize("limit", [True, False, 0, -1, "1", 1.0, None])
@pytest.mark.parametrize("key", ["max_modules", "max_contributions"])
def test_limits_are_exact_positive_bounded_integers(key, limit):
    with pytest.raises(ValueError, match="limits"): bind(*parts(), **{key: limit})


def test_exhaustion_refuses_without_a_partial_inventory():
    with pytest.raises(ValueError, match="module table"):
        bind(*parts(module() + module()), max_modules=1)
    with pytest.raises(ValueError, match="contribution table"):
        bind(*parts(contributions=contribution() * 2), max_contributions=1)


def test_module_stream_sizes_and_binding_are_checked_without_parsing_symbols():
    raw, streams = parts(module(stream=6, lengths=(4, 2, 1)))
    streams.append(bytes(8))
    value = bind(raw, streams)
    assert value.modules[0].stream == 6 and value.modules[0].symbol_bytes == 4
    streams[6] = bytes(6)
    with pytest.raises(ValueError, match="backing"): bind(raw, streams)


def test_old_pointers_and_alignment_padding_are_never_interpreted_as_offsets():
    mods = bytearray(module(name=b"x", object_name=b""))
    struct.pack_into("<I", mods, 0, 0xFFFFFFFF)
    struct.pack_into("<I", mods, 52, 0xFFFFFFFF)
    mods[-1] = 0xA5
    assert bind(*parts(bytes(mods))).modules[0].name == "x"
