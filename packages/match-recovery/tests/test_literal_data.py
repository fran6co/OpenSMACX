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
                   selection=2, extra_symbol=False, relocated=False, defined=True):
    """Physical independent data COMDAT with its exact definition/key records."""
    code = b"\xa1" + struct.pack("<I", addend) + b"\x90" * 11
    zero_fill = bool(flags & 0x80)
    raw = b"" if zero_fill else contents
    reloc = struct.pack("<IIH", 1, 3, 6)
    data_reloc = struct.pack("<IIH", 0, 0, 6) if relocated else b""
    name = b".bss" if zero_fill else b".data"
    symbols = (b"_f".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
               + name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, 2, 0, 3, 1)
               + struct.pack("<IHHIHB3s", len(contents), int(relocated), 0, 0, 0, selection, bytes(3))
               + NAME.encode().ljust(8, b"\0") + struct.pack("<IhHBB", 0, 2 if defined else 0, 0, storage, 0))
    if extra_symbol:
        symbols += b"_alias\0\0" + struct.pack("<IhHBB", 0, 2, 0, 2, 0)
    base = 100
    payload = code + raw + reloc + data_reloc
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, base + len(payload), 4 + int(extra_symbol), 0, 0)
    text = b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, 16, base,
        base + 16 + len(raw), 0, 1, 0, 0x60501020)
    data = name.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(contents),
        0 if zero_fill else base + 16, base + 16 + len(raw) + 10 if relocated else 0,
        0, int(relocated), 0, flags)
    return header + text + data + payload + symbols + struct.pack("<I", 4)


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
