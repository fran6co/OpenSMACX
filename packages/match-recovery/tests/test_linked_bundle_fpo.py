"""Native selected-object graph consumption of physical associated FPO data."""

from hashlib import sha256
import struct

import pytest

from match_recovery.linked_code_graph import compare_code_graphs
from match_recovery.linked_objects import SelectedObjects
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.pe_coff import Coff, PeImage
from test_linked_bundle import fixture as base_fixture, graph
from test_linked_fpo import object_bytes as fpo_object_bytes
from test_linked_objects import inventory
from test_link_input_coverage import inputs
from test_link_file_evidence import stamp
from test_multispan import object_fixture
from test_msf import small_msf
from test_pdb_fixups import parts


def fixture(root, *, family=True, locals_count=3, damage=None):
    _, _, image, _, mapping = base_fixture(root)
    code_size = 6 if family else 16
    payload = struct.pack("<IIIHH", 0, code_size, locals_count, 2, 0x0201)
    if family:
        raw_obj = bytearray(object_fixture(extra=(b".debug$F", payload, ((0, 2, 7),), 1, 5, 0x42101048)))
        symbols_at, = struct.unpack_from("<I", raw_obj, 8)
        # A real FPO section defines only its section+auxiliary, not the
        # generic fixture's extra local code-label record.
        del raw_obj[symbols_at + 8 * 18:symbols_at + 9 * 18]
        struct.pack_into("<I", raw_obj, 12, 8)
    else:
        raw_obj = bytearray(fpo_object_bytes(payload))
    if damage == "unknown_child":
        section_at = 20 + (2 if family else 1) * 40
        raw_obj[section_at:section_at + 8] = b".debug$S"
        symbols_at, = struct.unpack_from("<I", raw_obj, 8)
        symbol_index = 6 if family else 3
        raw_obj[symbols_at + symbol_index * 18:symbols_at + symbol_index * 18 + 8] = b".debug$S"
    evidence, log = inputs(root)
    path = root / "app.obj"
    path.write_bytes(raw_obj)
    row = next(row for row in evidence["files"] if row["path"] == str(path))
    row["before"] = row["after"] = stamp(raw_obj)
    objects = SelectedObjects(*inventory(evidence, log))
    raw = bytearray(image.data)
    if not family:
        obj = Coff(bytes(raw_obj))
        raw[512:528] = obj.sections[0].data
    image = PeImage(bytes(raw), sha256(raw).hexdigest())
    _, streams = parts(((20, 0, 0x1001, 0x1020),) if family else ())
    streams[5] = bytes(raw[0x178:0x1A0])
    dbi = bytearray(streams[3])
    if damage != "missing_stream":
        struct.pack_into("<H", dbi, 64, 6)
    streams[3] = bytes(dbi)
    record = struct.pack("<I", 0x1000) + payload[4:]
    if damage == "missing_record":
        record = b""
    elif damage == "duplicate":
        record *= 2
    elif damage == "mismatch":
        record = struct.pack("<IIIHH", 0x1000, code_size, locals_count + 1, 2, 0x0201)
    elif damage == "other_root":
        record = struct.pack("<I", 0x1020) + payload[4:]
    streams.append(record)
    fixups = bound_fixups(image, small_msf(streams))
    return Coff(bytes(raw_obj)), objects, image, fixups, mapping


@pytest.mark.parametrize("family", [False, True])
def test_associated_fpo_is_consumed_as_metadata_not_a_guessed_loaded_region(tmp_path, family):
    result = graph(fixture(tmp_path, family=family))
    assert result["result"] == "pass", result["gaps"]
    proof = result["nodes"][0]["proof"]
    size = 6 if family else 16
    assert proof["debug_regions"] == [{"parent": 0, "format": "fpo_data",
                                       "data": struct.pack("<IIHH", size, 3, 2, 0x0201).hex(), "code_size": size}]
    assert len(proof["regions"]) == (2 if family else 1)
    assert all(row["code"]["result"] == "pass" for row in proof["regions"])
    assert result["remaining_linked_effects"] == "unknown"
    assert not result["ready_for_collection"]
    assert proof["target_contents"] == "unknown"


@pytest.mark.parametrize("family", [False, True])
@pytest.mark.parametrize("damage", ["missing_stream", "missing_record", "duplicate", "mismatch", "other_root", "unknown_child"])
def test_unverified_debug_metadata_cannot_be_dropped_from_owned_family(tmp_path, family, damage):
    result = graph(fixture(tmp_path, family=family, damage=damage))
    assert result["result"] == "unknown", result
    assert result["graph_fingerprint"] is None
    assert not result["ready_for_collection"]


@pytest.mark.parametrize("family", [False, True])
def test_valid_fpo_metadata_changes_survive_cross_build_graph_comparison(tmp_path, family):
    left, right = tmp_path / "left", tmp_path / "right"
    left.mkdir()
    right.mkdir()
    baseline = graph(fixture(left, family=family))
    current = graph(fixture(right, family=family, locals_count=4))
    assert baseline["result"] == current["result"] == "pass"
    left_proof, right_proof = baseline["nodes"][0]["proof"], current["nodes"][0]["proof"]
    assert left_proof["regions"] == right_proof["regions"]
    assert left_proof["debug_regions"] != right_proof["debug_regions"]
    assert left_proof["body_fingerprint"] != right_proof["body_fingerprint"]
    assert baseline["graph_fingerprint"] != current["graph_fingerprint"]
    assert compare_code_graphs(baseline, current)["result"] == "fail"
