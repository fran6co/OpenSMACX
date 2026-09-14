"""Physical byte accounting never normalizes linked output or grants scope."""

from hashlib import sha256
import struct

import pytest

from match_recovery.pe_coff import PeImage
from match_recovery.pe_output_residual import compare_same_layout_pe
from test_binary import pe_fixture


def image(raw=None):
    if raw is None:
        data = bytearray(pe_fixture() + bytes(0x400) + b"overlay evidence")
        struct.pack_into("<H", data, 0x86, 3)
        struct.pack_into("<II", data, 0x98 + 32, 0x1000, 0x200)
        struct.pack_into("<II", data, 0x98 + 56, 0x4000, 0x200)
        for at, name, virtual, rva, size, offset, flags in (
                (0x178, b".text", 0x180, 0x1000, 0x200, 0x200, 0x60000020),
                (0x1A0, b"", 0x300, 0x2000, 0x200, 0x400, 0xC0000040),
                (0x1C8, b".bss", 0x100, 0x3000, 0, 0, 0xC0000080)):
            struct.pack_into("<8sIIIIIIHHI", data, at, name, virtual, rva, size, offset, 0, 0, 0, 0, flags)
        data[0x470:0x484] = b"NB10" + struct.pack("<III", 0, 123, 1) + b"x\0\0\0"
        struct.pack_into("<II", data, 0x98 + 96 + 6 * 8, 0x2040, 28)
        struct.pack_into("<IIHHIIII", data, 0x440, 0, 456, 0, 0, 2, 20, 0x2070, 0x470)
        raw = bytes(data)
    return PeImage(bytes(raw), sha256(raw).hexdigest())


def scope(address=0x401000, size=4, entity_id="entity-one"):
    return [{"entity_id": entity_id, "address": address, "size": size}]


def changed(before, offset, value=None):
    raw = bytearray(before.data)
    raw[offset] = raw[offset] ^ 1 if value is None else value
    return image(raw)


def compare(a, b, before=None, after=None, **options):
    before = scope() if before is None else before
    return compare_same_layout_pe(a, b, before, before if after is None else after, **options)


def test_identical_or_only_scoped_bytes_change_is_bounded_observation():
    before = image()
    for current in (before, changed(before, 0x200), changed(before, 0x203)):
        result = compare(before, current)
        assert result["outside_bytes"] == result["same_layout"] == "pass"
        assert result["different_file_bytes"] == result["difference_range_count"] == 0
        assert result["scoped_file_bytes"] == 4
        assert result["compared_file_bytes"] == len(before.data) - 4
        assert result["unchanged_zero_fill_bytes"] == 0x200
        assert result["result"] == result["outside_scope_verification"] == "unknown"
        assert result["ready_for_collection"] is result["persisted"] is False


@pytest.mark.parametrize("offset", [0x40, 0x88, 0x98 + 64, 0x204, 0x3FF,
                                     0x400, 0x444, 0x47C, 0x650, 0x800])
def test_headers_timestamps_directories_nb10_unnamed_data_gaps_overlay_are_literal(offset):
    before = image()
    result = compare(before, changed(before, offset))
    assert result["outside_bytes"] == "fail"
    assert result["different_file_bytes"] == result["difference_range_count"] == 1
    assert result["difference_ranges"] == [{"offset": offset, "size": 1}]
    assert result["result"] == result["outside_scope_verification"] == "unknown"


def test_added_and_removed_overlay_bytes_are_not_lost_to_zip():
    before = image()
    larger = image(before.data + b"more")
    for a, b in ((before, larger), (larger, before)):
        result = compare(a, b)
        assert result["outside_bytes"] == "fail"
        assert result["different_file_bytes"] == 4
        assert result["compared_file_bytes"] == len(larger.data) - 4
        assert result["difference_ranges"] == [{"offset": len(before.data), "size": 4}]


@pytest.mark.parametrize("offset,value", [(0x178 + 8, 0x170), (0x1A0 + 8, 0x301),
                                           (0x178 + 36, 0xE0000020), (0x98 + 56, 0x5000)])
def test_exact_virtual_size_flags_and_image_geometry_must_agree(offset, value):
    before = image()
    data = bytearray(before.data)
    struct.pack_into("<I", data, offset, value)
    result = compare(before, image(data))
    assert result["same_layout"] == result["outside_bytes"] == "unknown"
    assert "geometry differs" in result["reason"]


@pytest.mark.parametrize("offset,value", [(0x1A0 + 20, 0x200), (0x178 + 20, 0),
                                           (0x178 + 12, 0), (0x98 + 60, 0x180),
                                           (0x98 + 32, 0), (0x98 + 56, 0x2000)])
def test_aliased_raw_backing_header_aliases_and_unbounded_geometry_refuse(offset, value):
    data = bytearray(image().data)
    struct.pack_into("<I", data, offset, value)
    bad = image(data)
    result = compare(bad, bad)
    assert result["outside_bytes"] == "unknown"
    assert "reason" in result


@pytest.mark.parametrize("after", [scope(size=3), scope(address=0x401001), scope(entity_id="other"), []])
def test_changed_scope_identity_placement_extent_cannot_hide_changes(after):
    before = image()
    result = compare(before, before, after=after)
    assert result["outside_bytes"] == "unknown"


@pytest.mark.parametrize("rows", [scope(size=0), scope(size=-1), scope(size=True), scope(address=True),
                                  scope(address=0x402000), scope(address=0x403000),
                                  scope(address=0x4011FF, size=2), scope() * 2,
                                  scope() + scope(address=0x401002, entity_id="second"),
                                  [{"entity_id": "one", "address": 0x401000}], scope(entity_id="")])
def test_invalid_overlapping_or_unbacked_bodies_refuse(rows):
    before = image()
    assert compare(before, before, before=rows)["outside_bytes"] == "unknown"


def test_range_limit_never_stops_counting_and_zero_limit_is_supported():
    before = image()
    raw = bytearray(before.data)
    for offset in range(0x500, 0x600, 2):
        raw[offset] ^= 1
    for limit in (0, 2, 256):
        result = compare(before, image(raw), max_difference_ranges=limit)
        assert result["different_file_bytes"] == result["difference_range_count"] == 128
        assert len(result["difference_ranges"]) == min(limit, 128)
        assert result["difference_ranges_truncated"] is (limit < 128)


def test_adjacent_scopes_and_long_difference_crossing_scan_block():
    before = image(image().data + bytes(70000))
    raw = bytearray(before.data)
    raw[0x500:] = b"x" * (len(raw) - 0x500)
    rows = scope() + scope(address=0x401004, entity_id="second")
    result = compare(before, image(raw), before=rows, after=list(reversed(rows)))
    assert result["outside_bytes"] == "fail"
    assert result["scoped_file_bytes"] == 8
    assert result["compared_file_bytes"] == len(raw) - 8
    # Count independently rather than equating every overlay byte with zero.
    assert result["different_file_bytes"] == sum(a != b for a, b in zip(before.data, raw))
    assert any(row["size"] > 65536 for row in result["difference_ranges"])


def test_no_scope_compares_whole_file_and_results_do_not_alias_inputs():
    before = image()
    rows = scope()
    result = compare(before, changed(before, 0x200), before=[], after=[])
    assert result["outside_bytes"] == "fail" and result["compared_file_bytes"] == len(before.data)
    report = compare(before, before, before=rows)
    report["scoped_bodies"][0]["size"] = 100
    assert rows == scope()
    assert compare(before, before, before=rows)["scoped_file_bytes"] == 4


def test_mutated_parsed_image_and_invalid_diagnostic_bound_refuse():
    before = image()
    before.section_flags = (0, *before.section_flags[1:])
    assert compare(before, before)["outside_bytes"] == "unknown"
    for bound in (True, -1, 100001, "2"):
        assert compare(image(), image(), max_difference_ranges=bound)["outside_bytes"] == "unknown"


def test_boolean_parsed_state_cannot_equal_integer_zero():
    before = image()
    before.directories = ((False, False), *before.directories[1:])
    result = compare(before, before)
    assert result["outside_bytes"] == "unknown"
    assert "parsed PE state" in result["reason"]


@pytest.mark.parametrize("index,address", [(0, 0x1000), (1, 0x1000), (4, 0x200), (12, 0x1000)])
def test_directory_alias_in_either_version_never_hides_body_change(index, address):
    raw = bytearray(image().data)
    struct.pack_into("<II", raw, 0x98 + 96 + index * 8, address, 4)
    aliased = image(raw)
    for a, b in ((aliased, changed(aliased, 0x200)), (image(), aliased), (aliased, image())):
        result = compare(a, b)
        assert result["outside_bytes"] == "unknown"
        assert "aliases directory" in result["reason"]


@pytest.mark.parametrize("mapped", [True, False])
def test_debug_payload_cannot_alias_scoped_code_even_outside_directory(mapped):
    raw = bytearray(image().data)
    raw[0x200:0x204] = b"NB10"
    struct.pack_into("<IIHHIIII", raw, 0x440, 0, 456, 0, 0, 2, 4, 0x1000 if mapped else 0, 0x200)
    before = image(raw)
    result = compare(before, changed(before, 0x201))
    assert result["outside_bytes"] == "unknown"
    assert "aliases directory" in result["reason"] if mapped else "overlaps mapped" in result["reason"]


def test_overlaid_debug_payload_remains_literal_outside_bytes():
    raw = bytearray(image().data)
    struct.pack_into("<IIHHIIII", raw, 0x440, 0, 456, 0, 0, 2, 4, 0, 0x800)
    raw[0x800:0x804] = b"NB10"
    before = image(raw)
    result = compare(before, changed(before, 0x801))
    assert result["outside_bytes"] == "fail" and result["different_file_bytes"] == 1


def test_security_directory_is_a_file_offset_and_metadata_is_not_normalized():
    raw = bytearray(image().data)
    struct.pack_into("<II", raw, 0x98 + 96 + 4 * 8, 0x800, 8)
    before = image(raw)
    assert compare(before, before)["outside_bytes"] == "pass"
    assert compare(before, changed(before, 0x800))["outside_bytes"] == "fail"


@pytest.mark.parametrize("index,address,size", [(0, 0, 4), (0, 0x1000, 0),
                                                (0, 0x3000, 4), (4, 0x9000, 4),
                                                (6, 0x2040, 27)])
def test_unbacked_or_malformed_metadata_cannot_grant_byte_residual(index, address, size):
    raw = bytearray(image().data)
    struct.pack_into("<II", raw, 0x98 + 96 + index * 8, address, size)
    before = image(raw)
    assert compare(before, before)["outside_bytes"] == "unknown"
