"""Actual synthetic COFF/map/PE loader endpoints, not DLL behavior proofs."""

import json
import struct

import pytest

from match_recovery.pe_coff import Coff
from test_linked_code_graph import function, graph_build
from test_linked_initializers import compare, stamp, update_artifact


def imported_graph(root, *, module="SYSTEM.dll", export="Call", ordinal=None,
                   slot=0x20C0, addend=0, relative=False, hint=32, second=False, shared=False):
    code = ((b"\xe9" if relative else b"\xff\x15") + struct.pack("<i", addend) + b"\xc3")
    edges = [(1 if relative else 2, "_slot", 20 if relative else 6)]
    data = {"_slot": 0x400000 + slot}
    if second:
        assert not relative
        code = code[:-1] + b"\xff\x15\0\0\0\0\xc3"
        edges.append((8, "_slot2", 6))
        data["_slot2"] = 0x400000 + slot + (0 if shared else 4)
    context = graph_build(root, [function("_$E1", 0, code, edges=tuple(edges))], data=data)
    # VC6 uses a function type hint even on loader-owned pointer declarations.
    path = context.directory / "main.obj"
    obj = Coff(path.read_bytes())
    symbol, = [symbol for symbol in obj.symbols.values() if symbol.name == "_slot"]
    raw_object = bytearray(path.read_bytes())
    table, = struct.unpack_from("<I", raw_object, 8)
    struct.pack_into("<H", raw_object, table + 18 * symbol.index + 14, hint)
    path.write_bytes(raw_object)
    receipt = json.loads((context.directory / "link.json").read_bytes())
    row, = [row for row in receipt["files"] if row["path"] == str(path)]
    row["before"] = row["after"] = stamp(raw_object)
    update_artifact(context, "link.json", json.dumps(receipt).encode())
    raw = bytearray((context.directory / "app.exe").read_bytes())
    # Independent loader metadata, not inferred from the call operand.
    struct.pack_into("<II", raw, 0x98 + 96 + 8, 0x2040, 40)
    struct.pack_into("<IIIII", raw, 1024 + 0x40, 0x2080, 0, 0, 0x20A0, slot)
    entry = 0x80000000 | ordinal if ordinal is not None else 0x2100
    entries = (entry, entry, 0) if second and not shared else (entry, 0)
    struct.pack_into("<" + "I" * len(entries), raw, 1024 + 0x80, *entries)
    struct.pack_into("<" + "I" * len(entries), raw, 1024 + slot - 0x2000, *entries)
    raw[1024 + 0xA0:1024 + 0xA0 + len(module) + 1] = module.encode() + b"\0"
    raw[1024 + 0x100:1024 + 0x100 + len(export) + 3] = b"\0\0" + export.encode() + b"\0"
    update_artifact(context, "app.exe", bytes(raw))
    return context


@pytest.mark.parametrize("hint", [0, 32])
def test_loader_endpoint_uses_actual_import_and_survives_slot_move(tmp_path, hint):
    old = imported_graph(tmp_path / "old", hint=hint)
    new = imported_graph(tmp_path / "new", slot=0x20E0, module="system.DLL", hint=hint)
    report = compare(old, new)
    assert report["result"] == report["input_consistency"] == "pass", report
    graph = report["code_dependencies"]["current"]
    assert len(graph["nodes"]) == 1 and not graph["data_references"]
    endpoint, = graph["import_references"]
    assert endpoint["target"] == {"loader_import": {
        "module": "system.dll", "name": "Call", "ordinal": None, "symbol": "_slot"}, "slot": 0}
    assert endpoint["implementation"] == endpoint["selection"] == "unknown"
    assert report["remaining_linked_effects"] == "unknown"


@pytest.mark.parametrize("change", [{"module": "OTHER.dll"}, {"export": "Kill"}, {"ordinal": 7}])
def test_identical_referring_code_cannot_hide_a_different_loader_identity(tmp_path, change):
    report = compare(imported_graph(tmp_path / "old"), imported_graph(tmp_path / "new", **change))
    assert report["ordered_code_correspondence"] == report["input_consistency"] == "pass", report
    assert report["code_dependencies"]["result"] == report["result"] == "fail"


def test_same_ordinal_import_remains_an_explicit_loader_endpoint(tmp_path):
    report = compare(imported_graph(tmp_path / "old", ordinal=7),
                     imported_graph(tmp_path / "new", ordinal=7, slot=0x20E0))
    assert report["result"] == "pass", report


def test_same_export_identity_does_not_hide_changed_slot_sharing(tmp_path):
    report = compare(imported_graph(tmp_path / "old", second=True, shared=True),
                     imported_graph(tmp_path / "new", second=True))
    assert report["ordered_code_correspondence"] == report["input_consistency"] == "pass", report
    assert report["code_dependencies"]["result"] == report["result"] == "fail"
    slots = lambda version: [row["target"]["slot"] for row in report["code_dependencies"][version]["import_references"]]
    assert slots("baseline") == [0, 0] and slots("current") == [0, 1]


@pytest.mark.parametrize("change", [{"addend": 1}, {"addend": -1}, {"relative": True}])
def test_nonpointer_loader_slot_references_stay_unproved(tmp_path, change):
    report = compare(imported_graph(tmp_path / "old"), imported_graph(tmp_path / "new", **change))
    assert report["code_dependencies"]["current"]["result"] == "unknown", report
    assert not report["code_dependencies"]["current"]["import_references"]


def test_actual_operand_must_match_independent_map_slot(tmp_path):
    old, new = imported_graph(tmp_path / "old"), imported_graph(tmp_path / "new")
    raw = bytearray((new.directory / "app.exe").read_bytes())
    struct.pack_into("<I", raw, 512 + 2, 0x4020C4)
    update_artifact(new, "app.exe", bytes(raw))
    report = compare(old, new)
    assert report["input_consistency"] == report["result"] == "fail", report


@pytest.mark.parametrize("damage_root", [False, True])
def test_malformed_import_directory_cannot_hide_a_measured_root_failure(tmp_path, damage_root):
    old, new = imported_graph(tmp_path / "old"), imported_graph(tmp_path / "new")
    raw = bytearray((new.directory / "app.exe").read_bytes())
    struct.pack_into("<I", raw, 1024 + 0x40 + 16, 0)
    if damage_root:
        raw[512] = 0x90
    update_artifact(new, "app.exe", bytes(raw))
    report = compare(old, new)
    assert report["result"] == report["input_consistency"] == ("fail" if damage_root else "unknown"), report
    assert "missing PE import address table" in report["current"]["code_dependencies"]["reason"]
