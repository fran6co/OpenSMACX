"""Physical COFF records exercise the pure conservative no-fold witness."""

from copy import deepcopy
from dataclasses import replace
import struct

import pytest

from match_recovery.pe_coff import CoffRecords


from match_recovery.linked_comdat_nofold import sections_distinct


def physical(code, relocations=(), *, name=b".text", flags=0x60501020):
    """Decode complete actual COFF bytes, with two independent target records."""
    relocation_bytes = b"".join(struct.pack("<IIH", *row) for row in relocations)
    symbols = b"".join(label.ljust(8, b"\0") + struct.pack("<IhHBB", 0, section, 0, storage, 0)
                       for label, section, storage in ((name, 1, 3), (b"_a", 0, 2), (b"_b", 0, 2)))
    raw = (struct.pack("<HHIIIHH", 0x14C, 1, 0, 60 + len(code) + len(relocation_bytes), 3, 0, 0)
           + name.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(code), 60 if code else 0,
               60 + len(code) if relocations else 0, 0, len(relocations), 0, flags)
           + code + relocation_bytes + symbols + struct.pack("<I", 4))
    obj = CoffRecords(raw)
    return obj.sections[0], obj.section_sizes[0]


def distinct(left, right):
    return sections_distinct(*left, *right)


@pytest.mark.parametrize("left,right", [
    (b"\x55\xc3", b"\x56\xc3"),
    (b"\x55\xc3\x90", b"\x56\xc3\xcc\0"),
    (b"\x90\xc3", b"\x91\xc3"),
])
def test_immutable_fixed_prefix_mismatch_is_positive_and_symmetric(left, right):
    a, b = physical(left), physical(right)
    before = deepcopy((a, b))
    assert distinct(a, b) is True
    assert distinct(b, a) is True
    assert (a, b) == before


@pytest.mark.parametrize("left,right", [
    (b"\x55\xc3", b"\x55\xc3"),
    (b"\x55", b"\x55\xc3"),
    (b"\xc3\x90", b"\xc3\xcc\0"),
    (b"\xc3", b"\xc3\0\x90\xcc"),
    (b"\0\x90\xcc", b"\x55\xc3"),
    (b"\0", b"\x90\xcc"),
    (b"", b"\x55\xc3"),
    (b"", b""),
])
def test_equal_prefix_empty_and_possible_padding_do_not_distinguish(left, right):
    assert distinct(physical(left), physical(right)) is False
    assert distinct(physical(right), physical(left)) is False


def test_names_flags_targets_and_addends_grant_no_distinction():
    left = physical(b"\xe8\x11\x22\x33\x44\xc3", ((1, 1, 20),))
    right = physical(b"\xe8\xaa\xbb\xdd\xee\xc3", ((1, 2, 20),),
                     name=b".text$x", flags=0x60301020)
    assert distinct(left, right) is False


def test_both_operand_masks_are_unioned_not_intersected():
    left = physical(b"\x55\x11\x22\x33\x44\x55\x66\x77\x88\xc3", ((1, 1, 6),))
    right = physical(b"\x55\xaa\xbb\xdd\xee\x99\xaa\xbb\xdd\xc3", ((5, 2, 7),))
    assert distinct(left, right) is False
    assert distinct(right, left) is False


@pytest.mark.parametrize("kind,width", [(6, 4), (7, 4), (10, 2), (11, 4), (13, 1), (20, 4)])
def test_documented_write_widths_mask_complete_containing_bytes(kind, width):
    left = physical(b"\x55" + b"\x11" * width + b"\xc3", ((1, 1, kind),))
    right = physical(b"\x55" + b"\xee" * width + b"\xc3")
    assert distinct(left, right) is False
    changed = physical(b"\x56" + b"\xee" * width + b"\xc3")
    assert distinct(left, changed) is True


def test_absolute_is_noop_and_overlapping_write_intervals_remain_conservative():
    assert distinct(physical(b"\x55\xc3", ((0, 1, 0),)), physical(b"\x56\xc3")) is True
    assert distinct(physical(b"\x11" * 6 + b"\xc3", ((0, 1, 6), (2, 2, 6))),
                    physical(b"\xee" * 6 + b"\xc3")) is False


@pytest.mark.parametrize("kind", [1, 2, 9, 12, 15, 0xFFFF])
@pytest.mark.parametrize("reverse", [False, True])
def test_unknown_or_unsupported_kind_refuses_even_after_early_mismatch(kind, reverse):
    pair = (physical(b"\x55\x11\x22\x33\x44\xc3", ((1, 1, kind),)), physical(b"\x56\xc3"))
    with pytest.raises(ValueError, match="unsupported.*relocation"):
        distinct(*(reversed(pair) if reverse else pair))


@pytest.mark.parametrize("offset,kind", [(1, 6), (2, 10), (3, 13), (3, 0), (0xFFFFFFFF, 20)])
def test_out_of_bounds_physical_relocation_refuses(offset, kind):
    with pytest.raises(ValueError, match="outside.*extent"):
        distinct(physical(b"\x55\xc3", ((offset, 1, kind),)), physical(b"\x56\xc3"))


@pytest.mark.parametrize("size", [-1, 0, 1, 3, True, 2.0])
def test_missing_excess_and_nonexact_declared_extents_refuse(size):
    section, _ = physical(b"\x55\xc3")
    with pytest.raises(ValueError, match="physical raw extent"):
        sections_distinct(section, size, *physical(b"\x56\xc3"))


def test_truncated_raw_data_and_mutable_bytes_refuse():
    section, size = physical(b"\x55\xc3")
    for data in (section.data[:-1], bytearray(section.data)):
        with pytest.raises(ValueError, match="physical raw extent"):
            sections_distinct(replace(section, data=data), size, *physical(b"\x56\xc3"))


@pytest.mark.parametrize("rows", [((True, 1, 6),), ((-1, 1, 6),), ((0, -1, 6),),
                                  ((0, 1),), ([0, 1, 6],), [(0, 1, 6)]])
def test_malformed_relocation_metadata_refuses(rows):
    section, size = physical(b"\x55\x11\x22\x33\x44\xc3")
    with pytest.raises(ValueError):
        sections_distinct(replace(section, relocations=rows), size, *physical(b"\x56\xc3"))


def test_empty_other_side_does_not_hide_invalid_late_metadata():
    with pytest.raises(ValueError, match="unsupported.*relocation"):
        distinct(physical(b""), physical(b"\xc3", ((0, 1, 0xFFFF),)))
