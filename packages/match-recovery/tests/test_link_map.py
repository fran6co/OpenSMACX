"""Synthetic map evidence: never silently discard a malformed public row."""

import hashlib
import struct

import pytest

from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pe_coff import PeImage
from test_binary import pe_fixture


MAP = ("demo\n Preferred load address is 00400000\n"
       " Address         Publics by Value              Rva+Base     Lib:Object\n\n"
       " 0001:00000000 _f 00401000 f RUNTIME:runtime.obj\n"
       " 0001:00000020 _g 00401020 f application.obj\n"
       " entry point at 0001:00000000\n Static symbols\n")


def executable(code=b"\xe8\x1b\0\0\0\xc3", *, absolute_site=None):
    raw = bytearray(pe_fixture(code, absolute_site=absolute_site))
    struct.pack_into("<I", raw, 0x98 + 16, 0x1000)
    struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)
    return bytes(raw)


def parsed_image(raw):
    return PeImage(raw, hashlib.sha256(raw).hexdigest())


def test_map_preserves_owner_labels_and_checks_all_coordinates():
    result = parse_link_map(MAP.replace("\n", "\r\n").encode())
    assert result.publics[0].owner == "RUNTIME:runtime.obj"
    assert result.publics[1].owner == "application.obj"
    result.validate(parsed_image(executable()))


@pytest.mark.parametrize("damage", [
    lambda text: text.replace("00400000", "not-a-base"),
    lambda text: text.replace(" Preferred load", " Preferred wrong load"),
    lambda text: text + " Preferred load address is 00400000\n",
    lambda text: text.replace(" _g ", " _f "),
    lambda text: text.replace(" f RUNTIME", " f f RUNTIME"),
    lambda text: text.replace(" f RUNTIME", " x RUNTIME"),
    lambda text: text.replace("00401020", "bad-address"),
    lambda text: text.replace(" application.obj", ""),
    lambda text: text.split(" entry point")[0],
    lambda text: text.replace(" entry point at 0001:00000000", ""),
    lambda text: text.replace(" _g ", " \x00_g "),
    lambda text: text.replace(" _g ", " é_g "),
])
def test_corrupt_maps_do_not_yield_partial_public_evidence(damage):
    with pytest.raises(ValueError):
        parse_link_map(damage(MAP).encode())


@pytest.mark.parametrize("old,new", [
    ("00400000", "00500000"), ("00401020", "00401021"),
    ("0001:00000020", "0002:00000020"), ("0001:00000020", "0001:00000200"),
    ("entry point at 0001:00000000", "entry point at 0001:00000001"),
])
def test_map_must_agree_with_actual_pe_sections_and_entrypoint(old, new):
    mapping = parse_link_map(MAP.replace(old, new).encode())
    with pytest.raises(ValueError):
        mapping.validate(parsed_image(executable()))


def test_distinct_alias_names_are_retained_without_guessing_ownership():
    mapping = parse_link_map(MAP.replace("0001:00000020 _g 00401020", "0001:00000000 _g 00401000").encode())
    mapping.validate(parsed_image(executable()))
    assert len(mapping.publics) == 2


def test_absolute_symbols_are_retained_as_unplaced_not_image_sections():
    mapping = parse_link_map(MAP.replace("0001:00000020 _g 00401020", "0000:00000020 _g 00000020").encode())
    mapping.validate(parsed_image(executable()))
    assert mapping.publics[1].segment == 0
