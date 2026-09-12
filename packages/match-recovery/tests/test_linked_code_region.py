"""Physical ordinary code-section controls, never guessed function bounds."""

import json
import struct

import pytest

from match_recovery.linked_code_graph import compare_code_graphs
from match_recovery.pe_coff import Coff
from test_linked_code_graph import direct_graph, function, graph_build
from test_linked_initializers import stamp, update_artifact


def ordinary_object(*, aliases=(), anchor=0, external=(), local_offset=32, raw_missing=False,
                    cross_section=False, relocations=None, addend=0):
    """One ordinary section with interior code labels and pointer-vector data."""
    code = bytearray(b"\xcc" * 48)
    code[:6] = b"\x68\0\0\0\0\xc3"
    struct.pack_into("<i", code, 1, addend)
    code[16:20] = bytes(4)
    code[32] = code[36] = 0xC3
    relocations = [(1, 1, 6), (16, 2, 6)] if relocations is None else relocations
    symbols = [("_helper", anchor, 1, 32, 2, b""),
               ("inside", local_offset, 1, 32, 3, b""),
               ("$L1", 36, 1, 0, 6, b""),
               ("vector", 16, 1, 0, 3, b""),
               (".text", 0, 1, 0, 3, struct.pack("<IHHIHB3x", 48, len(relocations), 0, 0, 0, 0)),
               *[(name, value, 1, kind, storage, b"") for name, value, kind, storage in (*aliases, *external)]]
    specs = [(".text", bytes(code), relocations, 0x60500020)]
    if cross_section:
        symbols[1] = ("inside", 0, 2, 0, 3, b"")
        specs.append((".data", bytes(4), (), 0xC0300040))
    records = bytearray()
    for name, value, section, kind, storage, auxiliary in symbols:
        assert len(name) <= 8
        records.extend(name.encode().ljust(8, b"\0") + struct.pack("<IhHBB", value, section, kind,
                                                                    storage, len(auxiliary) // 18) + auxiliary)
    start = 20 + len(specs) * 40
    headers, payload = [], bytearray()
    for name, data, edges, flags in specs:
        at = start + len(payload)
        payload.extend(data)
        relocation_at = start + len(payload) if edges else 0
        for edge in edges:
            payload.extend(struct.pack("<IIH", *edge))
        headers.append(name.encode().ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(data),
                         0 if raw_missing and name == ".text" else at, relocation_at, 0, len(edges), 0, flags))
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(payload), len(records) // 18, 0, 0)
            + b"".join(headers) + payload + records + struct.pack("<I", 4))


def physical(root, **options):
    code = bytearray(b"\xcc" * 48)
    code[:6] = b"\x68" + struct.pack("<I", 32) + b"\xc3"
    code[16:20] = struct.pack("<I", 36)
    code[32] = code[36] = 0xC3
    context = graph_build(root, [
        function("_$E1", 0, b"\xe8\0\0\0\0\xc3", edges=((1, "_helper", 20),)),
        function("_helper", 0x100, bytes(code), owner="helper.obj", public=True,
                 edges=((1, "_helper", 6), (16, "_helper", 6)))])
    path = context.directory / "helper.obj"
    path.write_bytes(ordinary_object(**options))
    receipt = json.loads((context.directory / "link.json").read_text())
    row = next(row for row in receipt["files"] if row["path"] == str(path))
    row["before"] = row["after"] = stamp(path.read_bytes())
    update_artifact(context, "link.json", json.dumps(receipt).encode())
    return context


def helper(report):
    return next(node for node in report["nodes"] if node["symbol"] == "_helper")


def test_ordinary_section_with_internal_function_peers_and_jump_vectors_is_verified(tmp_path):
    context = physical(tmp_path / "ordinary")
    report = direct_graph(context)
    assert report["result"] == "pass", report["gaps"]
    node = helper(report)
    assert node["proof"]["extent_kind"] == "selected_code_section"
    assert node["proof"]["size"] == 48
    assert node["proof"]["other_sections"] == "unknown"
    assert {row["name"] for row in node["proof"]["section_symbols"]} == {"_helper", "inside", "$L1", "vector", ".text"}
    assert node["proof"]["code"]["result"] == node["proof"]["code"]["references"] == "pass"
    assert [edge["target"] for edge in node["edges"]] == [{"self_offset": 32}, {"self_offset": 36}]
    assert [node["symbol"] for node in report["nodes"]] == ["_$E1", "_helper"]
    assert report["remaining_linked_effects"] == "unknown" and not report["ready_for_collection"]
    assert '"raw"' not in json.dumps(report)


def test_strict_function_measurement_api_still_refuses_ordinary_code():
    obj = Coff(ordinary_object())
    with pytest.raises(ValueError, match="code COMDAT"):
        obj.function("_helper")


@pytest.mark.parametrize("damage", ["instruction", "jump-vector", "missing-fixup", "extra-fixup"])
def test_selected_section_pe_damage_is_never_hidden_by_local_labels(tmp_path, damage):
    context = physical(tmp_path / damage)
    raw = bytearray((context.directory / "app.exe").read_bytes())
    if damage == "instruction":
        raw[512 + 0x100 + 32] = 0x90
    elif damage == "jump-vector":
        struct.pack_into("<I", raw, 512 + 0x100 + 16, 0x401020)
    elif damage == "missing-fixup":
        # First relocation page contains exactly the helper's two words.
        struct.pack_into("<H", raw, 512 + 0x180 + 8, 0)
    else:
        # Move one fixup off its operand while retaining a valid block.
        struct.pack_into("<H", raw, 512 + 0x180 + 8, 0x3108)
    update_artifact(context, "app.exe", raw)
    report = direct_graph(context)
    assert report["result"] == "fail", report
    assert helper(report)["proof"]["code"]["result"] == "fail"


@pytest.mark.parametrize("external", [("alias", 0, 32, 2), ("another", 8, 32, 2), ("exported", 16, 0, 2)])
def test_additional_external_entries_or_aliases_refuse_whole_section_claim(tmp_path, external):
    report = direct_graph(physical(tmp_path / "extra", external=(external,)))
    assert report["result"] == helper(report)["result"] == "unknown"
    assert report["graph_fingerprint"] is None


@pytest.mark.parametrize("options", [{"anchor": 4}, {"anchor": 48}, {"local_offset": 48},
                                    {"local_offset": 0xFFFFFFFF}, {"raw_missing": True}, {"cross_section": True}])
def test_unsupported_section_boundaries_or_unplaced_targets_refuse(tmp_path, options):
    report = direct_graph(physical(tmp_path / "unsupported", **options))
    assert report["result"] == helper(report)["result"] == "unknown", report
    assert report["graph_fingerprint"] is None


def test_internal_alias_multiplicity_is_not_collapsed_by_equal_addresses_or_bytes(tmp_path):
    old = direct_graph(physical(tmp_path / "before"))
    new = direct_graph(physical(tmp_path / "after", aliases=(("alias", 32, 32, 3),)))
    assert old["result"] == new["result"] == "pass", (old["gaps"], new["gaps"])
    result = compare_code_graphs(old, new)
    assert result["result"] == "fail" and result["graph_equal"] is False
    assert result["remaining_linked_effects"] == "unknown"


@pytest.mark.parametrize("alias", [("label", 40, 0, 6), ("entry", 40, 32, 3)])
def test_unreferenced_internal_entry_identity_is_retained(tmp_path, alias):
    old = direct_graph(physical(tmp_path / "before", aliases=(alias,)))
    moved = (alias[0], 44, alias[2], alias[3])
    new = direct_graph(physical(tmp_path / "after", aliases=(moved,)))
    assert old["result"] == new["result"] == "pass", (old["gaps"], new["gaps"])
    assert compare_code_graphs(old, new)["result"] == "fail"


def test_identical_ordinary_section_graph_survives_physical_build_relocation(tmp_path):
    old = direct_graph(physical(tmp_path / "before"))
    new = direct_graph(physical(tmp_path / "after"))
    assert old["result"] == new["result"] == "pass", (old["gaps"], new["gaps"])
    assert compare_code_graphs(old, new)["result"] == "pass"


@pytest.mark.parametrize("relocations", [((47, 1, 6), (16, 2, 6)), ((1, 1, 6), (1, 2, 6)),
                                        ((1, 1, 7), (16, 2, 6)), ((1, 1, 6), (3, 2, 6))])
def test_malformed_or_unsupported_ordinary_relocations_cannot_pass(tmp_path, relocations):
    report = direct_graph(physical(tmp_path / "relocations", relocations=relocations))
    assert report["result"] != "pass" and report["graph_fingerprint"] is None


@pytest.mark.parametrize("addend", [-33, 16, 256])
def test_effective_local_target_must_stay_inside_verified_section(tmp_path, addend):
    context = physical(tmp_path / "addend", addend=addend)
    raw = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<I", raw, 512 + 0x100 + 1, 0x401100 + 32 + addend)
    update_artifact(context, "app.exe", raw)
    report = direct_graph(context)
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None
