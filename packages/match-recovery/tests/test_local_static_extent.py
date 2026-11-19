"""VC6 function-local statics: no CodeView data record, COFF-bounded extent only."""

from dataclasses import replace
import hashlib

import pytest

from match_recovery.codeview_data import data_layout, member_data_layout
from match_recovery.model import Origin, Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import ObjectBinding
from match_recovery.pe_coff import Coff, PeImage, Symbol
from test_object_data import data_fixture, data_image

# `static char part_speech[6][20]` inside `int parse_string(char *, char *)`.
ROWS = "_?part_speech@?IF@??parse_string@@YAHPAD0@Z@4PAY0BE@DA"


def local_static(name=ROWS, size=128, neighbour=120, *, storage=3, flags=None, value=b""):
    """One `.data` definition with the debug$S record unbound from it, as VC6 emits."""
    obj = Coff(data_fixture(value.ljust(size, b"\0"), storage=storage))
    obj.symbols[0] = replace(obj.symbols[0], name=name)
    obj.sections = (obj.sections[0] if flags is None else replace(obj.sections[0], flags=flags),
                    obj.sections[1], replace(obj.sections[2], relocations=()))
    if neighbour is not None:
        obj.symbols[2] = Symbol(2, "$SG18199", neighbour, 1, 0, 3)
    return obj


def test_local_static_array_takes_its_extent_up_to_the_next_definition():
    observed = data_layout(local_static(), ROWS)
    assert observed == {"symbol": ROWS, "source_name": "part_speech", "section": 1, "offset": 0,
                        "size": 120, "type_index": None, "extent_basis": "coff_local_static"}


def test_local_static_array_at_the_section_end_takes_the_section_size():
    assert data_layout(local_static(size=120, neighbour=None), ROWS)["size"] == 120


@pytest.mark.parametrize("size,neighbour,expected", [(8, 4, 4), (12, 8, 4), (4, None, 4)])
def test_local_static_scalar_uses_its_decorated_size(size, neighbour, expected):
    name = "_?count@?1??tick@@YAXXZ@4HA"
    assert data_layout(local_static(name, size, neighbour), name)["size"] == expected


@pytest.mark.parametrize("damage", [
    "pointer_or_vector",  # `PADA`: char * and char[] decorate alike
    "class_type", "file_static", "external", "comdat", "alias", "padding_ambiguous",
    "not_a_multiple", "scalar_overflows", "scalar_too_much_padding", "unbound_debug_reference"])
def test_local_static_extent_refuses_ambiguous_or_foreign_evidence(damage):
    name, kwargs = ROWS, {}
    if damage == "pointer_or_vector":
        name = "_?rows@?1??f@@YAXXZ@4PADA"
    elif damage == "class_type":
        name = "_?rows@?1??f@@YAXXZ@4VThing@@A"
    elif damage == "file_static":
        name = "?part_speech@@3PAY0BE@DA"
    elif damage == "external":
        kwargs = {"storage": 2}
    elif damage == "comdat":
        kwargs = {"flags": 0xC0301040}
    elif damage == "alias":
        kwargs = {"neighbour": 0}
    elif damage == "padding_ambiguous":
        name, kwargs = "_?rows@?1??f@@YAXXZ@4PAY02DA", {"neighbour": 12}  # char[][3] before a 4-aligned neighbour
    elif damage == "not_a_multiple":
        kwargs = {"neighbour": 112}
    elif damage == "scalar_overflows":
        name, kwargs = "_?count@?1??f@@YAXXZ@4NA", {"neighbour": 4}
    elif damage == "scalar_too_much_padding":
        name, kwargs = "_?count@?1??f@@YAXXZ@4HA", {"size": 12, "neighbour": None}
    obj = local_static(name, **kwargs)
    if damage == "unbound_debug_reference":
        # Any debug relocation naming the definition (here from a public
        # record, not S_LDATA32) leaves its extent to CodeView, not COFF.
        debug = obj.sections[2]
        data = bytearray(debug.data)
        data[6:8] = (0x1009).to_bytes(2, "little")
        obj.sections = (*obj.sections[:2], replace(debug, data=bytes(data),
                                                   relocations=((12, 0, 11), (16, 0, 10))))
    with pytest.raises(ValueError, match="requires one"):
        data_layout(obj, name)


def test_local_static_extent_supplies_no_member_or_slice_types():
    obj = local_static()
    with pytest.raises(ValueError, match="CodeView-typed parent"):
        member_data_layout(obj, ROWS, ("[1]",))
    with pytest.raises(ValueError, match="CodeView-typed parent"):
        member_data_layout(obj, ROWS, ("[0:2]",))


def test_bounded_local_static_initializer_is_checked_over_the_coff_extent():
    rows = b" de ".ljust(20, b"\0") * 6
    raw = data_image(rows)
    image = PeImage(raw, hashlib.sha256(raw).hexdigest())
    origin = Origin(hashlib.sha256(raw).hexdigest(), "pe:va", 0x401000)
    binding = ObjectBinding(origin, ROWS, "code/parse.cc", 1, Span(0x401000, 0x401000 + 120), origin)
    row, = check_initial_data((binding,), {binding.unit: local_static(value=rows)}, image, {})
    assert row["definition"] == "pass" and row["content"] == "pass"
    assert row["compiler_storage"]["extent_basis"] == "coff_local_static"
    short = ObjectBinding(origin, ROWS, "code/parse.cc", 1, Span(0x401000, 0x401000 + 100), origin)
    row, = check_initial_data((short,), {short.unit: local_static(value=rows)}, image, {})
    assert row["content"] != "pass"
