"""Physical shared-entry tests; section correspondence is not function bounds."""

from hashlib import sha256
import json
import struct

import pytest

from match_recovery.linked_code_graph import compare_code_graphs
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.pe_coff import Coff, PeImage
from test_linked_code_graph import direct_graph
from test_linked_code_region import physical
from test_linked_initializers import stamp, update_artifact
from test_msf import small_msf
from test_pdb_fixups import parts


def replace_input(context, name, raw):
    path = context.directory / name
    path.write_bytes(raw)
    receipt = json.loads((context.directory / "link.json").read_bytes())
    row = next(row for row in receipt["files"] if row["path"] == str(path))
    row["before"] = row["after"] = stamp(raw)
    update_artifact(context, "link.json", json.dumps(receipt).encode())


def shared(root, *, entry=8, other=24, map_damage=None, private=False):
    context = physical(root, anchor=entry, external=(("_other", other, 32, 2),), cross_section=private)
    mapping = (context.directory / "app.map").read_text()
    mapping = mapping.replace(" 0001:00000100 _helper 00401100 f helper.obj\n",
        f" 0001:{0x100 + entry:08x} _helper {0x401100 + entry:08x} f helper.obj\n")
    row = f" 0001:{0x100 + other:08x} _other {0x401100 + other:08x} f helper.obj\n"
    if map_damage == "missing":
        row = ""
    elif map_damage == "owner":
        row = row.replace("helper.obj", "entry.obj")
    elif map_damage == "type":
        row = row.replace(" f helper.obj", " helper.obj")
    elif map_damage == "conflict":
        row = f" 0001:{0x104 + other:08x} _other {0x401104 + other:08x} f helper.obj\n"
    elif map_damage == "duplicate":
        row += row
    elif map_damage == "absolute":
        row = " 0000:00000000 _other 00000000 f helper.obj\n"
    mapping = mapping.replace(" entry point at", row + " entry point at")
    update_artifact(context, "app.map", mapping.encode())
    context.graph_roots = [{"owner": "helper.obj", "symbol": "_helper"}]
    return context


def roots(context, names):
    context.graph_roots = [{"owner": "helper.obj", "symbol": name} for name in names]
    return context


@pytest.mark.parametrize("entry,other", [(0, 16), (8, 24), (16, 22)])
@pytest.mark.parametrize("order", [("_helper", "_other"), ("_other", "_helper")])
def test_every_entry_observes_one_complete_shared_section_in_both_orders(tmp_path, entry, other, order):
    report = direct_graph(roots(shared(tmp_path / "shared", entry=entry, other=other), order))
    assert report["result"] == "pass", report["gaps"]
    assert [node["symbol"] for node in report["nodes"]] == list(order)
    offsets = {"_helper": entry, "_other": other}
    for node in report["nodes"]:
        proof = node["proof"]
        assert proof["address"] == 0x401100 and proof["size"] == 48
        assert proof["entry_offset"] == offsets[node["symbol"]]
        assert proof["entry_address"] == 0x401100 + offsets[node["symbol"]]
        assert proof["other_sections"] == "unknown"
        assert {row["name"] for row in proof["section_symbols"]} == {
            "_helper", "_other", "inside", "$L1", "vector", ".text"}
    assert report["remaining_linked_effects"] == "unknown" and not report["ready_for_collection"]


@pytest.mark.parametrize("offset", [0, 7, 47])
def test_bytes_before_first_entry_and_at_end_are_never_cut_away(tmp_path, offset):
    context = shared(tmp_path / "damage")
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[512 + 0x100 + offset] ^= 1
    update_artifact(context, "app.exe", raw)
    report = direct_graph(context)
    assert report["result"] == "fail" and report["graph_fingerprint"] is None
    assert report["nodes"][0]["proof"]["code"]["bytes"] == "fail"


@pytest.mark.parametrize("damage", ["missing", "owner", "type", "conflict", "duplicate", "absolute"])
def test_each_external_entry_requires_consistent_unique_code_map_evidence(tmp_path, damage):
    context = shared(tmp_path / damage, map_damage=damage)
    if damage == "duplicate":
        with pytest.raises(ValueError, match="duplicate MSVC link map"):
            direct_graph(context)
        return
    report = direct_graph(context)
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None


@pytest.mark.parametrize("other", [8, 24])
def test_requested_entry_identity_changes_graph_and_body_even_for_same_offset_aliases(tmp_path, other):
    context = shared(tmp_path / "alias", other=other)
    before = direct_graph(roots(context, ("_helper",)))
    after = direct_graph(roots(context, ("_other",)))
    assert before["result"] == after["result"] == "pass", (before["gaps"], after["gaps"])
    assert before["nodes"][0]["proof"]["body_fingerprint"] != after["nodes"][0]["proof"]["body_fingerprint"]
    assert compare_code_graphs(before, after)["result"] == "fail"


def test_same_offset_named_entries_are_not_collapsed_into_one_root(tmp_path):
    report = direct_graph(roots(shared(tmp_path / "aliases", other=8), ("_helper", "_other")))
    assert report["result"] == "pass", report["gaps"]
    assert report["roots"] == [0, 1] and len(report["nodes"]) == 2
    assert {node["proof"]["entry_address"] for node in report["nodes"]} == {0x401108}


def calls_entry(root, addend):
    context = shared(root)
    path = context.directory / "main.obj"
    raw = bytearray(path.read_bytes())
    obj = Coff(bytes(raw))
    subject = obj.function("_$E1")
    code_at, = struct.unpack_from("<I", raw, 20 + (subject.section - 1) * 40 + 20)
    struct.pack_into("<i", raw, code_at + 1, addend)
    replace_input(context, "main.obj", bytes(raw))
    image = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<i", image, 513, 0x401108 + addend - 0x401005)
    update_artifact(context, "app.exe", image)
    context.graph_roots = [{"owner": "main.obj", "symbol": "_$E1"}]
    return context


@pytest.mark.parametrize("addend", [-8, -1, 0, 1, 39])
def test_effective_calls_use_entry_offset_plus_signed_addend_inside_whole_section(tmp_path, addend):
    report = direct_graph(calls_entry(tmp_path / "inside", addend))
    assert report["result"] == "pass", report["gaps"]
    helper = next(node for node in report["nodes"] if node["symbol"] == "_helper")
    assert helper["proof"]["entry_offset"] == 8


@pytest.mark.parametrize("addend", [-9, 40, 48])
def test_effective_calls_cannot_escape_section_even_when_operand_matches(tmp_path, addend):
    report = direct_graph(calls_entry(tmp_path / "outside", addend))
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None
    assert all(node["proof"]["code"]["result"] == "pass" for node in report["nodes"])
    assert any("effective code reference" in gap["reason"] for gap in report["gaps"])


@pytest.mark.parametrize("site", [0x1101, 0x1109])
def test_nonzero_entry_private_fixup_uses_section_origin_not_entry_address(tmp_path, site):
    context = shared(tmp_path / "private", private=True)
    raw = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<I", raw, 512 + 0x101, 0x402100)
    template, streams = parts(((6, 0, site, 0x2100),))
    payload = template[1024:]
    at = len(raw)
    raw.extend(payload)
    struct.pack_into("<I", raw, 0x98 + 60, 512)
    struct.pack_into("<II", raw, 0x98 + 96 + 6 * 8, 0x2180, 28)
    struct.pack_into("<IIHHIIII", raw, 1024 + 0x180, 0, 0, 0, 0, 2, len(payload), 0, at)
    streams[5] = bytes(raw[0x178:0x1C8])
    update_artifact(context, "app.exe", raw)
    image = PeImage(bytes(raw), sha256(raw).hexdigest())
    fixups = bound_fixups(image, small_msf(streams))
    report = direct_graph(context, fixups=fixups)
    if site == 0x1101:
        assert report["result"] == "pass", report["gaps"]
        proof = report["nodes"][0]["proof"]
        assert proof["address"] == 0x401100 and proof["entry_address"] == 0x401108
        assert proof["references"][0]["private_binding"]["placement"] == "pdb_fixup"
        assert report["data_references"][0]["contents"] == "unknown"
    else:
        assert report["result"] == "unknown" and report["graph_fingerprint"] is None


def test_strict_function_api_still_rejects_shared_ordinary_sections(tmp_path):
    context = shared(tmp_path / "strict")
    obj = Coff((context.directory / "helper.obj").read_bytes())
    for name in ("_helper", "_other"):
        with pytest.raises(ValueError, match="code COMDAT"):
            obj.function(name)
