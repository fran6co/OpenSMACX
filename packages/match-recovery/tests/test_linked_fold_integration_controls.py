"""Adversarial native fold enrollment, resource bounds and final epoch guards."""

from copy import deepcopy
from dataclasses import replace
from hashlib import sha256
from pathlib import Path
import struct

import pytest

from match_recovery.linked_code_graph import observe_code_graph
from match_recovery.linked_folding import bind_folding
from match_recovery.linked_objects import SelectedObjects
from test_link_file_evidence import stamp
from test_linked_folded_graph import fixture, graph, private_fixture
from test_linked_objects import inventory


@pytest.mark.parametrize("reverse", [False, True])
def test_prequeued_duplicate_physical_member_aliases_refuse_instead_of_changing_graph_identity(tmp_path, reverse):
    roots = [{"owner": "app.obj", "symbol": "_g"},
             {"owner": "RUNTIME:runtime.obj", "symbol": "_g"}]
    if reverse:
        roots.reverse()
    report = graph(fixture(tmp_path), roots)
    assert report["result"] == "unknown"
    assert report["graph_fingerprint"] is None
    assert any("prequeued" in row["reason"] or "alias" in row["reason"] for row in report["gaps"])


def test_repeated_same_root_keeps_occurrences_without_duplicating_original_members(tmp_path):
    root = {"owner": "app.obj", "symbol": "_g"}
    report = graph(fixture(tmp_path), [root, dict(root)])
    assert report["result"] == "pass", report["gaps"]
    assert report["roots"] == [0, 0]
    assert len(report["nodes"]) == 2


def test_group_expansion_cannot_return_a_passing_prefix_at_function_limit(tmp_path):
    values = fixture(tmp_path)
    objects, mapping, image, fixups, _ = values
    report = observe_code_graph(objects, mapping, image,
        [{"owner": "app.obj", "symbol": "_f"}], max_functions=1,
        fixups=fixups, folding=bind_folding(*values))
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None
    assert any("resource bound" in row["reason"] for row in report["gaps"])


@pytest.mark.parametrize("first", ["caller", "selected", "replaced"])
def test_private_reference_discovery_keeps_the_entire_group_in_all_request_orders(tmp_path, first):
    roots = [{"owner": "RUNTIME:runtime.obj", "symbol": "_caller"},
             {"owner": "app.obj", "symbol": "_f"}]
    if first == "selected":
        roots.reverse()
    elif first == "replaced":
        roots = [{"owner": "RUNTIME:runtime.obj", "symbol": "_g"}, roots[0]]
    report = graph(private_fixture(tmp_path), roots)
    assert report["result"] == "pass", report["gaps"]
    assert len(report["nodes"]) == 3
    assert sorted(row["symbol"] for row in report["nodes"]) == ["_caller", "_f", "_g"]
    assert not report["data_references"]


def test_input_mutation_during_frame_consumption_invalidates_the_whole_graph(tmp_path, monkeypatch):
    from match_recovery import linked_fold_frames
    values = fixture(tmp_path)
    original = linked_fold_frames.validate_fpo_source
    calls = 0
    def changed(*args, **kwargs):
        nonlocal calls
        result = original(*args, **kwargs)
        calls += 1
        if calls == 2:
            (tmp_path / "runtime.lib").write_bytes(b"changed during complete frame observation")
        return result
    monkeypatch.setattr(linked_fold_frames, "validate_fpo_source", changed)
    with pytest.raises(ValueError, match="population changed"):
        graph(values)


def test_public_and_private_same_spelling_cannot_collapse_distinct_fold_members(tmp_path):
    objects, mapping, image, fixups, log = private_fixture(tmp_path)
    # Two physically different roots share a spelling, but not storage class or
    # original owner. The selected public root has deliberately wrong code;
    # enrolling only the private replacement would hide that counterexample.
    path = tmp_path / "app.obj"
    raw = bytearray(path.read_bytes().replace(b"_f\0\0\0\0\0\0", b"_g\0\0\0\0\0\0"))
    code_at, = struct.unpack_from("<I", raw, 20 + 20)
    raw[code_at] = 0x91
    path.write_bytes(raw)
    log = log.replace(b"_f from app.obj", b"_g from app.obj")
    evidence = deepcopy(objects._constant_evidence)
    evidence["link_log_sha256"] = sha256(log).hexdigest()
    for row in evidence["files"]:
        row["before"] = row["after"] = stamp(Path(row["path"]).read_bytes())
    objects = SelectedObjects(*inventory(evidence, log.decode()))
    mapping = replace(mapping, publics=tuple(replace(row, symbol="_g") if row.symbol == "_f" else row
                                             for row in mapping.publics))
    report = graph((objects, mapping, image, fixups, log),
                   [{"owner": "RUNTIME:runtime.obj", "symbol": "_caller"}])
    assert report["result"] != "pass"
    assert report["graph_fingerprint"] is None
