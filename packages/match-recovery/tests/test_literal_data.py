"""Private literal bindings must prove storage before resolving code operands."""

from dataclasses import replace
import hashlib
import struct

import pytest

from match_recovery.application_binary import check_compiled
from match_recovery.binary import BoundedTarget
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.literal_data import check_literals, literal_storage
from match_recovery.literal_source import LiteralBinding
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from test_pooled_string_constants import image_with_literal


NAME = "??_C@_0"


def literal_object(contents=b"rb\0", *, addend=0, flags=0xC0301040, storage=2,
                   selection=2, extra_symbol=False, relocated=False, defined=True,
                   symbol_name=NAME, symbol_offset=0, symbol_type=0,
                   section_name=None, wrong_key=False):
    """Physical independent data COMDAT with its exact definition/key records."""
    code = b"\xa1" + struct.pack("<I", addend) + b"\x90" * 11
    zero_fill = bool(flags & 0x80)
    raw = b"" if zero_fill else contents
    associative = selection == 5
    reloc = struct.pack("<IIH", 1, 3 + 2 * int(associative) + int(wrong_key), 6)
    data_reloc = struct.pack("<IIH", 0, 0, 6) if relocated else b""
    name = section_name.encode() if section_name else b".bss" if zero_fill else b".data"
    encoded = symbol_name.encode()
    strings = struct.pack("<I", 4)
    if len(encoded) > 8:
        token = struct.pack("<II", 0, 4)
        strings = struct.pack("<I", 5 + len(encoded)) + encoded + b"\0"
    else:
        token = encoded.ljust(8, b"\0")
    text_definition = (b".text\0\0\0" + struct.pack("<IhHBB", 0, 1, 0, 3, 1)
                       + struct.pack("<IHHIHB3s", 16, 1, 0, 0, 0, 2, bytes(3))) if associative else b""
    symbols = (text_definition
               + b"_f".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
               + name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, 2, 0, 3, 1)
               + struct.pack("<IHHIHB3s", len(contents), int(relocated), 0, 0,
                             1 if selection == 5 else 0, selection, bytes(3)))
    if wrong_key:
        symbols += b"_other\0\0" + struct.pack("<IhHBB", 0, 2, 0, 2, 0)
    symbols += token + struct.pack("<IhHBB", symbol_offset, 2 if defined else 0,
                                   symbol_type, storage, 0)
    if extra_symbol:
        symbols += b"_alias\0\0" + struct.pack("<IhHBB", 0, 2, 0, 2, 0)
    base = 100
    payload = code + raw + reloc + data_reloc
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, base + len(payload),
                         4 + 2 * int(associative) + int(extra_symbol) + int(wrong_key), 0, 0)
    text = b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, 16, base,
        base + 16 + len(raw), 0, 1, 0, 0x60501020)
    data = name.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(contents),
        0 if zero_fill else base + 16, base + 16 + len(raw) + 10 if relocated else 0,
        0, int(relocated), 0, flags)
    return header + text + data + payload + symbols + strings


def fixture(contents=b"rb\0", *, twice=True, target=0x402000, **object_options):
    original = b"\xa1" + struct.pack("<I", target) + b"\x90" * 11
    raw = image_with_literal(original, contents[:-1], twice=twice)
    pin = hashlib.sha256(raw).hexdigest()
    origin = Origin(pin, "pe:va", 0x401000)
    literal = LiteralBinding(Origin(pin, "pe:va", 0x402000), Span(0x402000, 0x402000 + len(contents)),
                             NAME, "code/a.cc", 1, origin)
    function = FunctionBinding(origin, (Span(0x401000, 0x401010),), "_f", literal.unit)
    return literal, function, {literal.unit: Coff(literal_object(contents, **object_options))}, PeImage(raw, pin)


@pytest.mark.parametrize("contents,zero_fill", [(b"rb\0", False), (b"a\0b\0", False),
                                               (b"r\0b\0\0\0", False), (b"\0", False), (b"\0", True)])
def test_complete_emitted_extent_not_first_nul(contents, zero_fill):
    literal, function, objects, image = fixture(contents, flags=0xC0301080 if zero_fill else 0xC0301040)
    targets, rows = check_literals([literal], objects, image)
    assert targets == {(literal.unit, NAME): BoundedTarget(literal.span)}
    assert rows[0]["definition"] == rows[0]["content"] == "pass"
    row, = check_objects([function], objects, image, literal_targets=targets)
    assert row["result"] == "pass"
    assert literal_storage(objects[literal.unit], NAME)[0] == contents


@pytest.mark.parametrize("damage", ["contents", "extent", "missing", "undefined", "alias", "static", "relocated", "executable", "ordinary"])
def test_bad_literal_never_supplies_a_target_or_falls_back_to_content_search(damage):
    literal, function, objects, image = fixture(twice=False)
    if damage == "contents":
        objects[literal.unit] = Coff(literal_object(b"wb\0"))
    elif damage == "extent":
        literal = replace(literal, span=Span(0x402000, 0x402002))
    elif damage == "missing":
        objects = {}
    else:
        options = {"undefined": {"defined": False}, "alias": {"extra_symbol": True},
                   "static": {"storage": 3}, "relocated": {"relocated": True},
                   "executable": {"flags": 0x60301060}, "ordinary": {"flags": 0xC0300040, "selection": 0}}
        objects[literal.unit] = Coff(literal_object(**options[damage]))
    targets, rows = check_literals([literal], objects, image)
    assert targets == {(literal.unit, NAME): None}
    assert rows[0]["content"] != "pass"
    row, = check_objects([function], objects, image, literal_targets=targets)
    assert row["result"] != "pass", row


def test_identical_names_in_two_tus_need_two_independent_bindings():
    literal, function, objects, image = fixture()
    second = replace(literal, origin=Origin(literal.origin.image_sha256, "pe:va", 0x402003),
                     span=Span(0x402003, 0x402006), unit="code/b.cc")
    objects[second.unit] = Coff(literal_object())
    targets, rows = check_literals([literal, second], objects, image)
    assert all(row["content"] == "pass" for row in rows)
    first, = check_objects([function], objects, image, literal_targets=targets)
    other, = check_objects([replace(function, unit=second.unit)], objects, image, literal_targets=targets)
    assert first["result"] == "pass" and other["references"] == "fail"
    alone, = check_objects([replace(function, unit=second.unit)], objects, image,
                           literal_targets={(literal.unit, NAME): BoundedTarget(literal.span)})
    assert alone["references"] == "unknown"


@pytest.mark.parametrize("damage", ["target", "addend", "loader"])
def test_proven_literal_cannot_hide_code_reference_damage(damage):
    literal, function, objects, image = fixture(target=0x402003 if damage == "target" else 0x402000,
                                               addend=1 if damage == "addend" else 0)
    if damage == "loader":
        raw = bytearray(image.data)
        struct.pack_into("<H", raw, 776, 0)  # Remove original HIGHLOW at code operand.
        pin = hashlib.sha256(raw).hexdigest()
        image = PeImage(bytes(raw), pin)
        function = replace(function, origin=replace(function.origin, image_sha256=pin))
        literal = replace(literal, origin=replace(literal.origin, image_sha256=pin), evidence=function.origin)
    targets, rows = check_literals([literal], objects, image)
    assert rows[0]["content"] == "pass"
    row, = check_objects([function], objects, image, literal_targets=targets)
    assert row["result"] == row["references"] == "fail"


def test_declared_binding_uses_source_address_even_when_same_bytes_repeat():
    literal, function, objects, image = fixture()
    moved = replace(literal, origin=replace(literal.origin, address=0x402003), span=Span(0x402003, 0x402006))
    rows, _, _, _, _, proofs = check_compiled([function], (), (), objects, image, {}, literal_bindings=[moved])
    assert proofs[0]["content"] == "pass"  # Equal data does not select a different occurrence.
    assert rows[0]["references"] == "fail"


def test_global_channel_cannot_alias_a_private_binding():
    literal, function, objects, image = fixture()
    with pytest.raises(ValueError, match="also has a global"):
        check_compiled([function], (), (), objects, image, {NAME: 0x402000}, literal_bindings=[literal])


@pytest.mark.parametrize("target,address,addend", [(0x402000, 0x402003, 0xFFFFFFFD),
                                                  (0x402004, 0x402000, 4)])
def test_addend_cannot_escape_literal_even_when_encoded_original_address_matches(target, address, addend):
    literal, function, objects, image = fixture(target=target, addend=addend)
    declared = replace(literal, origin=replace(literal.origin, address=address), span=Span(address, address + 3))
    rows, _, _, _, _, proofs = check_compiled([function], (), (), objects, image, {}, literal_bindings=[declared])
    assert proofs[0]["content"] == "pass"
    assert rows[0]["result"] == rows[0]["references"] == "fail"


@pytest.mark.parametrize("addend", [0, 1, 2, 3])
def test_valid_pointer_addends_include_interior_and_one_past(addend):
    literal, function, objects, image = fixture(target=0x402000 + addend, addend=addend)
    rows, _, _, _, _, proofs = check_compiled([function], (), (), objects, image, {}, literal_bindings=[literal])
    assert proofs[0]["content"] == rows[0]["references"] == rows[0]["result"] == "pass"


REAL8 = "__real@8@3ff88102040810204000"
REAL8_BYTES = struct.pack("<d", 1.0 / 127.0)
REAL8_FLAGS = 0x40401040


def numeric_fixture(*, contents=REAL8_BYTES, symbol_name=REAL8, target=0x402000,
                    original_flags=0x40000040, **options):
    """Two explicitly placed double copies; neither names nor contents choose one."""
    original = b"\xa1" + struct.pack("<I", target) + b"\x90" * 11
    raw = bytearray(image_with_literal(original, b""))
    struct.pack_into("<IIII", raw, 0x1A8, 16, 0x2000, 16, 1024)
    struct.pack_into("<I", raw, 0x1A0 + 36, original_flags)
    raw[1024:1040] = REAL8_BYTES * 2
    pin = hashlib.sha256(raw).hexdigest()
    origin = Origin(pin, "pe:va", 0x401000)
    literal = LiteralBinding(Origin(pin, "pe:va", 0x402000), Span(0x402000, 0x402008),
                             symbol_name, "code/a.cc", 1, origin)
    function = FunctionBinding(origin, (Span(0x401000, 0x401010),), "_f", literal.unit)
    settings = dict(flags=REAL8_FLAGS, section_name=".rdata", symbol_name=symbol_name)
    settings.update(options)
    return literal, function, {literal.unit: Coff(literal_object(contents, **settings))}, PeImage(bytes(raw), pin)


def test_vc6_double_literal_uses_complete_physical_storage_and_explicit_origin():
    literal, function, objects, image = numeric_fixture()
    contents, storage = literal_storage(objects[literal.unit], literal.symbol)
    assert contents == REAL8_BYTES and storage["size"] == 8 and storage["offset"] == 0
    targets, rows = check_literals([literal], objects, image)
    assert targets == {(literal.unit, REAL8): BoundedTarget(literal.span)}
    assert rows[0]["definition"] == rows[0]["content"] == "pass"
    row, = check_objects([function], objects, image, literal_targets=targets)
    assert row["result"] == row["references"] == "pass"


def test_numeric_zero_is_eight_initialized_bytes_not_first_nul_or_bss():
    name = "__real@8@00000000000000000000"
    obj = Coff(literal_object(bytes(8), flags=REAL8_FLAGS, symbol_name=name, section_name=".rdata"))
    contents, storage = literal_storage(obj, name)
    assert contents == bytes(8) and storage["size"] == 8


@pytest.mark.parametrize("damage", [
    "content", "empty", "four_bytes", "seven_bytes", "nine_bytes", "ten_bytes", "sixteen_bytes",
    "symbol_offset", "static", "function_type", "undefined", "writable", "zero_fill", "unreadable",
    "executable", "tls", "relocated", "peers", "wrong_key", "ordinary", "selection1", "selection3",
    "selection4", "associative", "selection6", "original_writable", "original_zero_fill", "original_extent",
])
def test_bad_numeric_storage_reserves_unresolved_and_never_supplies_a_target(damage):
    choices = {
        "symbol_offset": {"symbol_offset": 1}, "static": {"storage": 3},
        "function_type": {"symbol_type": 0x20}, "undefined": {"defined": False},
        "writable": {"flags": REAL8_FLAGS | 0x80000000},
        "zero_fill": {"flags": (REAL8_FLAGS & ~0x40) | 0x80},
        "unreadable": {"flags": REAL8_FLAGS & ~0x40000000},
        "executable": {"flags": REAL8_FLAGS | 0x20000020},
        "tls": {"section_name": ".tls"}, "relocated": {"relocated": True},
        "peers": {"extra_symbol": True}, "wrong_key": {"wrong_key": True},
        "ordinary": {"flags": REAL8_FLAGS & ~0x1000, "selection": 0},
        "selection1": {"selection": 1}, "selection3": {"selection": 3},
        "selection4": {"selection": 4}, "associative": {"selection": 5},
        "selection6": {"selection": 6}, "original_writable": {"original_flags": 0xC0000040},
        "original_zero_fill": {"original_flags": 0x40000080},
    }
    lengths = {"empty": 0, "four_bytes": 4, "seven_bytes": 7, "nine_bytes": 9,
               "ten_bytes": 10, "sixteen_bytes": 16}
    options = choices.get(damage, {})
    if damage in lengths:
        options = {"contents": bytes(lengths[damage])}
    elif damage == "content":
        options = {"contents": struct.pack("<d", 0.5)}
    literal, function, objects, image = numeric_fixture(**options)
    if damage == "original_extent":
        literal = replace(literal, span=Span(0x402000, 0x402007))
    targets, rows = check_literals([literal], objects, image)
    assert targets == {(literal.unit, literal.symbol): None}
    assert rows[0]["content"] != "pass"
    row, = check_objects([function], objects, image, literal_targets=targets)
    assert row["result"] != "pass" and row["references"] == "unknown", row


@pytest.mark.parametrize("name", ["ordinary", "__real@4@3f800000", "__real@10@" + "0" * 20,
                                  "__real@8@" + "0" * 19, "__real@8@" + "0" * 21,
                                  "__real@8@" + "g" * 20, REAL8 + "suffix"])
def test_unsupported_numeric_symbol_families_remain_unresolved(name):
    literal, _, objects, image = numeric_fixture(symbol_name=name)
    targets, rows = check_literals([literal], objects, image)
    assert targets == {(literal.unit, name): None} and rows[0]["definition"] == "unknown"


def test_numeric_names_and_equal_contents_do_not_infer_or_unify_origins():
    literal, function, objects, image = numeric_fixture()
    other = replace(literal, origin=replace(literal.origin, address=0x402008),
                    span=Span(0x402008, 0x402010), unit="code/b.cc")
    objects[other.unit] = Coff(literal_object(REAL8_BYTES, flags=REAL8_FLAGS,
                                            symbol_name=REAL8, section_name=".rdata"))
    targets, rows = check_literals([literal, other], objects, image)
    assert all(r["content"] == "pass" for r in rows)
    current, = check_objects([function], objects, image, literal_targets=targets)
    wrong_origin, = check_objects([replace(function, unit=other.unit)], objects, image, literal_targets=targets)
    unbound_unit, = check_objects([replace(function, unit="code/c.cc")],
                                 {"code/c.cc": objects[literal.unit]}, image, literal_targets=targets)
    assert current["result"] == "pass" and wrong_origin["references"] == "fail"
    assert unbound_unit["references"] == "unknown"
    with pytest.raises(ValueError, match="duplicate TU-private"):
        check_literals([literal, replace(other, unit=literal.unit)], objects, image)
    with pytest.raises(ValueError, match="also has a global"):
        check_compiled([function], (), (), objects, image, {REAL8: 0x402000}, literal_bindings=[literal])


@pytest.mark.parametrize("damage", ["original_target", "candidate_addend", "original_highlow"])
def test_qualified_numeric_data_does_not_hide_bad_instruction_operands(damage):
    literal, function, objects, image = numeric_fixture(
        target=0x402008 if damage == "original_target" else 0x402000,
        addend=1 if damage == "candidate_addend" else 0)
    if damage == "original_highlow":
        raw = bytearray(image.data)
        struct.pack_into("<H", raw, 776, 0)
        pin = hashlib.sha256(raw).hexdigest()
        image = PeImage(bytes(raw), pin)
        function = replace(function, origin=replace(function.origin, image_sha256=pin))
        literal = replace(literal, origin=replace(literal.origin, image_sha256=pin), evidence=function.origin)
    targets, rows = check_literals([literal], objects, image)
    assert rows[0]["definition"] == rows[0]["content"] == "pass"
    row, = check_objects([function], objects, image, literal_targets=targets)
    assert row["references"] == row["result"] == "fail"


@pytest.mark.parametrize("offset", [4, 8])
def test_numeric_original_requires_full_raw_backing_not_initialized_virtual_tail(offset):
    # Both original intervals are readable and in an initialized section,
    # but cross or follow its raw file extent. read_initial would zero-extend.
    expected = (REAL8_BYTES[4:] + bytes(4)) if offset == 4 else bytes(8)
    literal, function, objects, image = numeric_fixture(contents=expected, target=0x402000 + offset)
    raw = bytearray(image.data)
    struct.pack_into("<I", raw, 0x1A0 + 16, 8)  # Keep virtual size16, raw size8.
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(bytes(raw), pin)
    function = replace(function, origin=replace(function.origin, image_sha256=pin))
    literal = replace(literal, origin=Origin(pin, "pe:va", 0x402000 + offset),
                      span=Span(0x402000 + offset, 0x402008 + offset), evidence=function.origin)
    assert image.read_initial(literal.span) == expected
    with pytest.raises(ValueError, match="backed by image file bytes"):
        image.read(literal.span)
    targets, rows = check_literals([literal], objects, image)
    assert targets == {(literal.unit, literal.symbol): None}
    assert rows[0]["content"] != "pass" and "backed by image file bytes" in rows[0]["reason"]
    row, = check_objects([function], objects, image, literal_targets=targets)
    assert row["references"] == "unknown"


def test_legacy_string_initial_zero_tail_behavior_is_unchanged():
    literal, _, objects, image = fixture(contents=b"\0")
    raw = bytearray(image.data)
    struct.pack_into("<IIII", raw, 0x1A8, 16, 0x2000, 8, 1024)
    pin = hashlib.sha256(raw).hexdigest()
    image = PeImage(bytes(raw), pin)
    literal = replace(literal, origin=Origin(pin, "pe:va", 0x402008),
                      span=Span(0x402008, 0x402009), evidence=replace(literal.evidence, image_sha256=pin))
    targets, rows = check_literals([literal], objects, image)
    assert rows[0]["definition"] == rows[0]["content"] == "pass"
    assert targets == {(literal.unit, literal.symbol): BoundedTarget(literal.span)}


def test_merged_literal_name_may_coincide_only_with_its_own_folded_object_target():
    literal, function, objects, image = fixture()
    targets, rows = check_literals([literal], objects, image)
    assert rows[0]["content"] == "pass"
    # Without a caller-validated merge, a same-name global still refuses.
    with pytest.raises(ValueError, match="also has a global"):
        check_objects([function], objects, image, external_targets={NAME: 0x402000},
                      literal_targets=targets)
    row, = check_objects([function], objects, image, external_targets={NAME: 0x402000},
                         literal_targets=targets, merged_literal_names=(NAME,))
    assert row["result"] == "pass", row
    # A released name must still agree with the object's placement.
    with pytest.raises(ValueError, match="also has a global"):
        check_objects([function], objects, image, external_targets={NAME: 0x402003},
                      literal_targets=targets, merged_literal_names=(NAME,))
    # Function and import namespaces are never released.
    with pytest.raises(ValueError, match="also has a global"):
        check_objects([function], objects, image, import_targets={NAME: 0x402000},
                      literal_targets=targets, merged_literal_names=(NAME,))


def test_unproved_merged_literal_never_borrows_the_object_target():
    literal, function, objects, image = fixture(target=0x402003)
    unresolved = {(literal.unit, NAME): None}
    row, = check_objects([function], objects, image, external_targets={NAME: 0x402003},
                         literal_targets=unresolved, merged_literal_names=(NAME,))
    assert row["result"] != "pass" and row["references"] != "pass", row


def static_string_object(data=b"xy\0\0rb\0\0", labels=(0, 4), target=1, *, section=b".data",
                         flags=0xC0300040, data_relocation=None, raw=True):
    """Unpooled VC6 TU: ``$SGn`` static labels inside one multi-entry data section."""
    code = b"\xa1" + bytes(4) + b"\x90" * 11
    base = 100
    stored = data if raw else b""
    reloc = struct.pack("<IIH", 1, 3 + target, 6)
    data_reloc = struct.pack("<IIH", *data_relocation, 6) if data_relocation else b""
    payload = code + stored + reloc + data_reloc
    symbols = (b"_f".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
               + section.ljust(8, b"\0") + struct.pack("<IhHBB", 0, 2, 0, 3, 1)
               + struct.pack("<IHHIHB3s", len(data), int(bool(data_relocation)), 0, 0, 0, 0, bytes(3)))
    for number, value in enumerate(labels, 1):
        symbols += f"$SG{number}".encode().ljust(8, b"\0") + struct.pack("<IhHBB", value, 2, 0, 3, 0)
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, base + len(payload), 3 + len(labels), 0, 0)
    text = b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, 16, base,
        base + 16 + len(stored), 0, 1, 0, 0x60501020)
    section_header = section.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(data),
        base + 16 if raw else 0, base + 26 + len(stored) if data_relocation else 0,
        0, int(bool(data_relocation)), 0, flags)
    return header + text + section_header + payload + symbols + struct.pack("<I", 4)


def static_fixture(size=3, symbol="$SG2", **options):
    original = b"\xa1" + struct.pack("<I", 0x402000) + b"\x90" * 11
    raw = image_with_literal(original, b"rb\0")  # .rdata "rb\0\0X\0": padded like the TU.
    pin = hashlib.sha256(raw).hexdigest()
    origin = Origin(pin, "pe:va", 0x401000)
    literal = LiteralBinding(Origin(pin, "pe:va", 0x402000), Span(0x402000, 0x402000 + size),
                             symbol, "code/a.cc", 1, origin)
    function = FunctionBinding(origin, (Span(0x401000, 0x401010),), "_f", literal.unit)
    return literal, function, {literal.unit: Coff(static_string_object(**options))}, PeImage(raw, pin)


@pytest.mark.parametrize("size", [3, 4])
def test_unpooled_static_string_label_resolves_within_multi_entry_data(size):
    literal, function, objects, image = static_fixture(size)
    targets, rows = check_literals([literal], objects, image)
    assert rows[0]["definition"] == rows[0]["content"] == "pass", rows
    assert rows[0]["compiler_storage"]["basis"] == "static_string_label"
    assert rows[0]["compiler_storage"]["offset"] == 4
    assert targets == {(literal.unit, "$SG2"): BoundedTarget(literal.span)}
    row, = check_objects([function], objects, image, literal_targets=targets)
    assert row["result"] == "pass", row


def test_empty_static_string_label_in_bss_is_zero_filled():
    literal, _, objects, _ = static_fixture()
    obj = Coff(static_string_object(bytes(1), labels=(0,), target=0, section=b".bss",
                                    flags=0xC0300080, raw=False))
    contents, storage = literal_storage(obj, "$SG1", 1)
    assert contents == b"\0" and storage["basis"] == "static_string_label"


@pytest.mark.parametrize("damage", ["truncated", "overlong", "unterminated", "wide_tail", "alias",
                                    "relocated", "other_section", "executable", "external", "unbound"])
def test_bad_static_string_label_never_supplies_a_target(damage):
    size, symbol, options = 3, "$SG2", {}
    if damage == "truncated":
        size = 2  # Declared bound would hide the terminator.
    elif damage == "overlong":
        options = {"data": b"xy\0\0rb\0\0", "labels": (0, 4, 7)}
        size = 4  # Crosses the next label.
    elif damage == "unterminated":
        options = {"data": b"xy\0\0rbcd", "labels": (0, 4)}
        size = 4
    elif damage == "wide_tail":
        options = {"data": b"xy\0\0rb\0\0" + bytes(8) + b"q\0\0\0", "labels": (0, 4, 16)}
    elif damage == "alias":
        options = {"labels": (0, 4, 4)}
    elif damage == "relocated":
        options = {"data_relocation": (4, 0)}
    elif damage == "other_section":
        options = {"section": b".CRT$XCU"}
    elif damage == "executable":
        options = {"flags": 0x60300020}
    elif damage == "unbound":
        symbol = "$SG7"
    literal, function, objects, image = static_fixture(size, symbol, **options)
    if damage == "external":
        raw = bytearray(static_string_object())
        raw[-4 - 18 + 16] = 2  # $SG2 storage class: external, not a static label.
        objects[literal.unit] = Coff(bytes(raw))
    targets, rows = check_literals([literal], objects, image)
    assert targets == {(literal.unit, symbol): None}, rows
    assert rows[0]["content"] != "pass"
    row, = check_objects([function], objects, image, literal_targets=targets)
    assert row["result"] != "pass", row
