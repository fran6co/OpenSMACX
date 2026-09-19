"""Physical PE damage controls for the new generated-storage assignment."""

import struct

import pytest

from match_recovery.linker_generated_storage import assign_linker_contributions
from test_imports import pe
from test_linked_import_thunks import image_thunk


def assign(raw, *, rows=None, modules=None):
    return assign_linker_contributions(
        rows if rows is not None else [{"index": 0, "module": 0, "address": 0x402000, "size": 6,
                                       "section": 2, "offset": 0, "flags": 0x60000020}],
        modules if modules is not None else [{"index": 0, "name": "* Linker *",
                                             "kind": "linker_generated", "object_name": ""}],
        pe(bytes(raw)),
    )


@pytest.mark.parametrize("options", [{}, {"ordinal": True}, {"bound": True}, {"lookup": False}])
def test_physical_pe_positive_control(options):
    report = assign(image_thunk(**options))
    assert report["assigned"] == 1
    assert not report["ready_for_collection"]
    assert report["records"][0]["assignment"]["ordinal"] == (7 if options.get("ordinal") else None)


@pytest.mark.parametrize("damage", ["missing_fixup", "wrong_fixup", "nonexecutable_thunk", "executable_iat",
                                   "opcode", "unbound_operand", "cut_relocation", "iat_relocation"])
def test_damaged_pe_cannot_assign_generated_thunk(damage):
    raw = bytearray(image_thunk())
    if damage == "missing_fixup":
        struct.pack_into("<H", raw, 0x308, 0)
    elif damage == "wrong_fixup":
        struct.pack_into("<H", raw, 0x308, 0x3001)
    elif damage == "nonexecutable_thunk":
        struct.pack_into("<I", raw, 0x1A0 + 36, 0x40000040)
    elif damage == "executable_iat":
        struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)
    elif damage == "opcode":
        raw[1025] = 0x15
    elif damage == "unbound_operand":
        struct.pack_into("<I", raw, 1026, 0x401164)
    elif damage == "cut_relocation":
        struct.pack_into("<H", raw, 0x308, 0x3003)
    else:
        struct.pack_into("<I", raw, 0x300, 0x1000)
        struct.pack_into("<H", raw, 0x308, 0x3160)
    report = assign(raw)
    assert report["assigned"] == 0, report
    assert report["result"] != "pass", report


@pytest.mark.parametrize("field,value", [("section", 1), ("offset", 1), ("flags", 0x40000040),
                                        ("size", 0), ("size", 5), ("size", 7)])
def test_contribution_extent_and_characteristics_are_not_inferred_from_bytes(field, value):
    row = {"index": 0, "module": 0, "address": 0x402000, "size": 6,
           "section": 2, "offset": 0, "flags": 0x60000020, field: value}
    report = assign(image_thunk(), rows=[row])
    assert report["assigned"] == 0 and report["unassigned"] == 1


@pytest.mark.parametrize("field,value", [("object_name", "real.obj"), ("kind", "unresolved")])
def test_linker_name_alone_is_not_generated_module_identity(field, value):
    module = {"index": 0, "name": "* Linker *", "kind": "linker_generated", "object_name": "", field: value}
    assert assign(image_thunk(), modules=[module])["assigned"] == 0


def test_ordinary_modules_are_not_recipe_candidates():
    module = {"index": 0, "name": "real.obj", "kind": "standalone_object", "object_name": "real.obj"}
    report = assign(image_thunk(), modules=[module])
    assert report["records"] == []
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("offset", [0, 2, 5])
def test_duplicate_or_intersecting_contribution_stays_unassigned(offset):
    first = {"index": 0, "module": 0, "address": 0x402000, "size": 6,
             "section": 2, "offset": 0, "flags": 0x60000020}
    second = {**first, "index": 1, "address": first["address"] + offset, "offset": offset}
    report = assign(image_thunk(), rows=[first, second])
    assert report["assigned"] == 0 and report["unassigned"] == 2
