"""Physical ICF peers, complete FPO multisets, and native graph consumption."""

from copy import deepcopy
from dataclasses import replace
from hashlib import sha256
import struct

import pytest

from match_recovery.linked_code_graph import observe_code_graph, compare_code_graphs
from match_recovery.linked_folding import bind_folding
from match_recovery.linked_objects import SelectedObjects
from match_recovery.pe_coff import PeImage
from match_recovery.pdb_fixups import bound_fixups
from test_binary import coff_fixture
from test_linked_folding import fixture as folding_fixture
from test_linked_fpo import object_bytes
from test_linked_objects import inventory
from test_link_file_evidence import stamp
from test_msf import small_msf
from test_multispan import object_fixture
from test_pdb_fixups import parts


def fixture(root, *, peer_params=4, damage=None, extra_groups="", second_address=0x401000,
            companions=False, peer_extent=16):
    first_frame = struct.pack("<IIIHH", 0, 6 if companions else 16, 3, 2, 0x0201)
    peer_frame = struct.pack("<IIIHH", 0, 6 if companions else peer_extent, 3, peer_params, 0x0201)
    code = b"\xe8\0\0\0\0\xc3" + b"\xcc" * 10 if companions else b"\x90" * 15 + b"\xc3"
    peer_code = b"\x91" + code[1:] if damage == "peer_source" else code
    def emitted(frame, body):
        if not companions:
            return object_bytes(frame, code=body)
        value = bytearray(object_fixture(extra=(b".debug$F", frame, ((0, 2, 7),), 1, 5, 0x42101048)))
        symbol_at, = struct.unpack_from("<I", value, 8)
        del value[symbol_at + 8 * 18:symbol_at + 9 * 18]
        struct.pack_into("<I", value, 12, 8)
        return bytes(value)
    first = emitted(first_frame, code)
    second = emitted(peer_frame, peer_code + b"\xcc" * (peer_extent - 16)).replace(b"_f\0\0\0\0\0\0", b"_g\0\0\0\0\0\0")
    objects, mapping, original, _, log = folding_fixture(root, raw_first=first, raw_second=second,
        extra_groups=extra_groups, second_address=second_address)
    raw = bytearray(original.data)
    raw[512:528] = code
    if companions:
        struct.pack_into("<i", raw, 513, 0x1B)
        raw[544:560] = b"\xc3" + b"\xcc" * 15
    if damage == "linked_code":
        raw[512] = 0x91
    image = PeImage(bytes(raw), sha256(raw).hexdigest())
    _, streams = parts(((20, 0, 0x1001, 0x1020),) if companions else ())
    streams[5] = bytes(raw[0x178:0x1C8])
    dbi = bytearray(streams[3])
    struct.pack_into("<H", dbi, 64, 6)
    streams[3] = bytes(dbi)
    first_record = struct.pack("<I", 0x1000) + first_frame[4:]
    peer_record = struct.pack("<I", 0x1000) + peer_frame[4:]
    records = first_record + peer_record
    if damage == "missing_fpo":
        records = first_record
    elif damage == "extra_fpo":
        records += peer_record
    elif damage == "duplicate_wrong_fpo":
        records = first_record * 2
    elif damage == "wrong_fpo_root":
        records = first_record + struct.pack("<I", 0x1020) + peer_record[4:]
    elif damage == "reordered_fpo":
        records = peer_record + first_record
    streams.append(records)
    fixups = bound_fixups(image, small_msf(streams))
    return objects, mapping, image, fixups, log


def graph(values, roots=None, *, folding=None):
    objects, mapping, image, fixups, _ = values
    return observe_code_graph(objects, mapping, image,
        roots or [{"owner": "app.obj", "symbol": "_f"}], fixups=fixups,
        folding=bind_folding(*values) if folding is None else folding)


@pytest.mark.parametrize("owner,symbol", [("app.obj", "_f"), ("app.obj", "_g"), ("RUNTIME:runtime.obj", "_g")])
def test_any_requested_fold_member_verifies_every_original_owner_and_distinct_frame(tmp_path, owner, symbol):
    result = graph(fixture(tmp_path), [{"owner": owner, "symbol": symbol}])
    assert result["result"] == "pass", result["gaps"]
    assert len(result["nodes"]) == 2
    nodes = {node["symbol"]: node for node in result["nodes"]}
    assert nodes["_f"]["source_owner"] == "app.obj"
    assert nodes["_g"]["source_owner"] == "RUNTIME:runtime.obj"
    assert nodes["_g"]["input"]["member"] == "runtime.obj"
    assert nodes["_g"]["input"]["path"] == str(tmp_path / "runtime.lib")
    assert nodes["_f"]["folded_group"] == nodes["_g"]["folded_group"]
    assert all(node["result"] == "pass" and len(node["proof"]["regions"]) == 1 for node in nodes.values())
    assert nodes["_f"]["proof"]["debug_regions"][0]["data"] != nodes["_g"]["proof"]["debug_regions"][0]["data"]
    assert result["remaining_linked_effects"] == "unknown" and not result["ready_for_collection"]


@pytest.mark.parametrize("damage", ["missing_fpo", "extra_fpo", "duplicate_wrong_fpo", "wrong_fpo_root"])
def test_folding_requires_complete_exact_frame_multiset_not_any_matching_record(tmp_path, damage):
    result = graph(fixture(tmp_path, damage=damage))
    assert result["result"] == "unknown" and result["graph_fingerprint"] is None
    assert any("every original member exactly" in gap["reason"] for gap in result["gaps"])


def test_identical_frame_duplicates_require_the_actual_number_of_original_members(tmp_path):
    result = graph(fixture(tmp_path, peer_params=2))
    assert result["result"] == "pass", result["gaps"]
    assert len(result["nodes"]) == 2


def test_physical_fpo_record_order_does_not_change_member_identity(tmp_path):
    left, right = tmp_path / "left", tmp_path / "right"
    left.mkdir()
    right.mkdir()
    baseline = graph(fixture(left))
    current = graph(fixture(right, damage="reordered_fpo"))
    assert baseline["result"] == current["result"] == "pass"
    assert compare_code_graphs(baseline, current)["result"] == "pass"


@pytest.mark.parametrize("damage", ["peer_source", "linked_code"])
def test_matching_selected_root_or_frame_cannot_hide_peer_code_damage(tmp_path, damage):
    result = graph(fixture(tmp_path, damage=damage))
    assert result["result"] == "fail", result["gaps"]
    assert result["graph_fingerprint"] is None
    assert len(result["nodes"]) == 2
    nodes = {node["symbol"]: node for node in result["nodes"]}
    assert nodes["_g"]["result"] == "fail"
    if damage == "peer_source":
        assert nodes["_f"]["result"] == "pass"


def test_peer_metadata_change_remains_in_cross_build_canonical_graph(tmp_path):
    left, right = tmp_path / "left", tmp_path / "right"
    left.mkdir()
    right.mkdir()
    baseline = graph(fixture(left))
    current = graph(fixture(right, peer_params=5))
    assert baseline["result"] == current["result"] == "pass"
    assert [node["proof"]["regions"] for node in baseline["nodes"]] == [node["proof"]["regions"] for node in current["nodes"]]
    assert compare_code_graphs(baseline, current)["result"] == "fail"


def test_distinct_member_addresses_are_not_an_icf_shared_root(tmp_path):
    result = graph(fixture(tmp_path, second_address=0x401010))
    assert result["result"] == "unknown"
    assert any("common linked address" in gap["reason"] for gap in result["gaps"])


def test_unrelated_unsupported_group_is_not_used_to_complete_requested_group(tmp_path):
    extra = "Selected symbol:\n_missing from app.obj\nReplaced symbol(s):\n_other from app.obj\n"
    values = fixture(tmp_path, extra_groups=extra)
    result = graph(values)
    assert result["result"] == "pass" and len(result["nodes"]) == 2
    requested = graph(values, [{"owner": "app.obj", "symbol": "_missing"}])
    assert requested["result"] == "unknown" and requested["graph_fingerprint"] is None


def test_unrelated_same_address_body_cannot_join_verified_group_by_equal_bytes(tmp_path):
    objects, mapping, image, fixups, log = fixture(tmp_path)
    raw = coff_fixture(b"\x90" * 15 + b"\xc3").replace(b"_f\0\0\0\0\0\0", b"_h\0\0\0\0\0\0")
    path = tmp_path / "app.exp"
    path.write_bytes(raw)
    evidence = deepcopy(objects._constant_evidence)
    next(row for row in evidence["generated_exports"] if row["path"] == str(path))["after"] = stamp(raw)
    objects = SelectedObjects(*inventory(evidence, log.decode()))
    unrelated = replace(mapping.publics[0], symbol="_h", owner="app.exp")
    mapping = replace(mapping, publics=(*mapping.publics, unrelated))
    values = objects, mapping, image, fixups, log
    result = graph(values, [{"owner": "app.obj", "symbol": "_f"}, {"owner": "app.exp", "symbol": "_h"}])
    assert result["result"] == "unknown" and result["graph_fingerprint"] is None
    assert any("overlap" in gap["reason"] for gap in result["gaps"])


@pytest.mark.parametrize("options", [{"companions": True}, {"peer_extent": 32}])
def test_same_group_cannot_authorize_shared_companions_or_unequal_root_extents(tmp_path, options):
    result = graph(fixture(tmp_path, **options))
    assert result["result"] == "unknown", result["gaps"]
    assert result["graph_fingerprint"] is None
    assert any("overlap" in gap["reason"] for gap in result["gaps"])


@pytest.mark.parametrize("damage", ["objects", "image", "fixups", "mapping"])
def test_prebound_folding_cannot_be_reused_with_another_consumer_epoch(tmp_path, damage):
    values = list(fixture(tmp_path))
    folding = bind_folding(*values)
    if damage == "objects":
        values[0] = SelectedObjects(*inventory(deepcopy(values[0]._constant_evidence), values[4].decode()))
    elif damage == "image":
        values[2] = PeImage(values[2].data, sha256(values[2].data).hexdigest())
    elif damage == "fixups":
        values[3] = replace(values[3])
    else:
        values[1] = replace(values[1], publics=values[1].publics[:1])
    with pytest.raises(ValueError, match="exact live input/image binding"):
        graph(values, folding=folding)


def private_caller_object():
    """One physical object owns private _g/FPO and independent public caller."""
    original = bytearray(object_bytes().replace(b"_f\0\0\0\0\0\0", b"_g\0\0\0\0\0\0"))
    symbol_at, = struct.unpack_from("<I", original, 8)
    original[symbol_at + 2 * 18 + 16] = 3  # Real static function, never data.
    sections = bytearray(original[20:100])
    for start in (0, 40):
        for field in (20, 24):
            pointer, = struct.unpack_from("<I", sections, start + field)
            if pointer:
                struct.pack_into("<I", sections, start + field, pointer + 40)
    caller = b"\xe8\0\0\0\0\xc3" + b"\xcc" * 10
    caller_at = symbol_at + 40
    caller_header = b".text$c\0" + struct.pack("<IIIIIIHHI", 0, 0, 16, caller_at, caller_at + 16, 0, 1, 0, 0x60501020)
    caller_symbols = (b".text$c\0" + struct.pack("<IhHBB", 0, 3, 0, 3, 1)
                      + struct.pack("<IHHIHB3x", 16, 1, 0, 0, 0, 2)
                      + b"_caller\0" + struct.pack("<IhHBB", 0, 3, 32, 2, 0))
    header = struct.pack("<HHIIIHH", 0x14C, 3, 0, caller_at + 26, 8, 0, 0)
    return (header + sections + caller_header + original[100:symbol_at] + caller
            + struct.pack("<IIH", 1, 2, 20) + original[symbol_at:-4] + caller_symbols + original[-4:])


def private_fixture(root, *, damage=None):
    group = None
    if damage == "missing_group":
        group = "Selected symbol:\n_f from app.obj\nReplaced symbol(s):\n_other from app.obj\n"
    elif damage == "wrong_original_owner":
        group = "Selected symbol:\n_f from app.obj\nReplaced symbol(s):\n_g from app.obj\n"
    elif damage == "wrong_selected_owner":
        group = "Selected symbol:\n_f from absent.obj\nReplaced symbol(s):\n_g from RUNTIME.lib(runtime.obj)\n"
    objects, mapping, original, _, log = folding_fixture(root, raw_second=private_caller_object(), group=group)
    raw = bytearray(original.data)
    raw[512:528] = b"\x90" * 15 + b"\xc3"
    operand = -0x44 if damage == "wrong_call_operand" else -0x45
    raw[576:592] = b"\xe8" + struct.pack("<i", operand) + b"\xc3" + b"\xcc" * 10
    image = PeImage(bytes(raw), sha256(raw).hexdigest())
    target = replace(mapping.publics[1])  # Map rewrites private target owner to winner app.obj.
    caller = replace(mapping.publics[0], symbol="_caller", owner="RUNTIME:runtime.obj", address=0x401040, offset=0x40)
    mapping = replace(mapping, publics=(mapping.publics[0], caller), statics=(target,))
    _, streams = parts(((20, 0, 0x1041, 0x1000),))
    streams[5] = bytes(raw[0x178:0x1C8])
    dbi = bytearray(streams[3])
    struct.pack_into("<H", dbi, 64, 6)
    streams[3] = bytes(dbi)
    streams.append(struct.pack("<IIIHH", 0x1000, 16, 3, 2, 0x0201) * 2)
    fixups = bound_fixups(image, small_msf(streams))
    return objects, mapping, image, fixups, log


def test_private_code_reference_uses_original_fold_owner_and_adds_every_peer(tmp_path):
    values = private_fixture(tmp_path)
    result = graph(values, [{"owner": "RUNTIME:runtime.obj", "symbol": "_caller"}])
    assert result["result"] == "pass", result["gaps"]
    nodes = {node["symbol"]: node for node in result["nodes"]}
    assert set(nodes) == {"_caller", "_f", "_g"}
    assert nodes["_g"]["source_owner"] == "RUNTIME:runtime.obj"
    assert nodes["_f"]["source_owner"] == "app.obj"
    edge, = nodes["_caller"]["edges"]
    assert edge["target"] == {"node": nodes["_g"]["id"]}
    reference, = nodes["_caller"]["proof"]["references"]
    assert "private_binding" not in reference and "private_data_binding" not in reference
    assert result["data_references"] == []
    assert result["remaining_linked_effects"] == "unknown"


@pytest.mark.parametrize("damage", ["missing_group", "wrong_original_owner", "wrong_selected_owner"])
def test_private_folded_code_without_exact_original_icf_ownership_refuses(tmp_path, damage):
    result = graph(private_fixture(tmp_path, damage=damage), [{"owner": "RUNTIME:runtime.obj", "symbol": "_caller"}])
    assert result["result"] == "unknown", result["gaps"]
    assert result["graph_fingerprint"] is None
    assert result["data_references"] == []


def test_valid_private_folding_metadata_cannot_hide_wrong_call_operand(tmp_path):
    result = graph(private_fixture(tmp_path, damage="wrong_call_operand"), [{"owner": "RUNTIME:runtime.obj", "symbol": "_caller"}])
    assert result["result"] == "fail", result["gaps"]
    assert result["graph_fingerprint"] is None and result["data_references"] == []
