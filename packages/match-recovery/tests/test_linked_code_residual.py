"""Synthetic algebra controls, not native selection or permission evidence."""

from copy import deepcopy

import pytest

from match_recovery.linked_code_residual import compare_residual, normalize_residual


def body(index, edges=()):
    return {"code_sha256": "code", "size": 8, "flags": 32,
            "extent_kind": "function_comdat", "entry_offset": 0,
            "address_alias": index, "folded_group": None, "folded_frames": None,
            "edges": list(edges)}


def edge(target, offset=0):
    return {"offset": offset, "kind": "absolute32", "target": target}


def graph(bodies, roots=None):
    return {"result": "pass", "gaps": [], "canonical_nodes": bodies,
            "nodes": [{"id": index, "result": "pass"} for index in range(len(bodies))],
            "roots": list(range(len(bodies))) if roots is None else roots}


def normalized(value, identities=None, omitted=None):
    return normalize_residual(value, identities if identities is not None else
                              {index: {"unit": f"unit{index}.cpp", "symbol": "f"}
                               for index in range(len(value["nodes"]))},
                              omitted if omitted is not None else set())


def test_traversal_root_and_reference_numbering_are_not_identity():
    old = graph([body(0, [edge({"node": 1, "addend": 2})]), body(1)])
    new = graph([body(0), body(1, [edge({"node": 0, "addend": 2})])], [1, 0, 1])
    assert compare_residual(normalized(old), normalized(new, {
        0: {"unit": "unit1.cpp", "symbol": "f"},
        1: {"unit": "unit0.cpp", "symbol": "f"}}))["result"] == "pass"


def test_omitting_root_never_prunes_orphaned_node():
    old = graph([body(0, [edge({"node": 1})]), body(1)])
    new = graph([body(0)])
    identity = {1: {"unit": "unit1.cpp", "symbol": "f"}}
    before = normalized(old, omitted={0})
    after = normalized(new, {0: identity[1]})
    assert compare_residual(before, after)["result"] == "pass"
    new["canonical_nodes"][0]["code_sha256"] = "damaged helper"
    assert compare_residual(before, normalized(new, {0: identity[1]}))["result"] == "fail"
    assert compare_residual(before, normalized(graph([])))["result"] == "fail"


def test_incoming_reference_to_omitted_body_remains_bound():
    observed = graph([body(0), body(1, [edge({"node": 0})])])
    before = normalized(observed, omitted={0})
    observed["canonical_nodes"][0]["code_sha256"] = "approved new body"
    assert compare_residual(before, normalized(observed, omitted={0}))["result"] == "pass"
    ids = {0: "another approved root", 1: {"unit": "unit1.cpp", "symbol": "f"}}
    assert compare_residual(before, normalized(observed, ids, {0}))["result"] == "fail"


@pytest.mark.parametrize("field", ["address_alias", "folded_group"])
def test_shared_alias_or_folded_peer_cannot_disappear(field):
    observed = graph([body(0), body(1)])
    for item in observed["canonical_nodes"]:
        item[field] = 0
    before = normalized(observed, omitted={0})
    after = normalized(graph([body(0)]), {0: {"unit": "unit1.cpp", "symbol": "f"}})
    assert compare_residual(before, after)["result"] == "fail"


def test_alias_representative_and_fold_numbers_normalize_but_split_does_not():
    old = graph([body(0), body(0)])
    for item in old["canonical_nodes"]:
        item["folded_group"] = 4
    new = deepcopy(old)
    for item in new["canonical_nodes"]:
        item.update(address_alias=1, folded_group=37)
    assert compare_residual(normalized(old), normalized(new))["result"] == "pass"
    new["canonical_nodes"][0]["address_alias"] = 0
    assert compare_residual(normalized(old), normalized(new))["result"] == "fail"


def imported(slot):
    return {"loader_import": {"module": "api.dll", "name": "entry", "ordinal": None, "symbol": "_entry"},
            "slot": slot}


def test_import_slot_renumbering_preserves_sharing_and_distinguishes_split():
    old = graph([body(0, [edge(imported(0)), edge(imported(0), 4)])])
    new = graph([body(0, [edge(imported(88)), edge(imported(88), 4)])])
    assert compare_residual(normalized(old), normalized(new))["result"] == "pass"
    new["canonical_nodes"][0]["edges"][1]["target"]["slot"] = 89
    assert compare_residual(normalized(old), normalized(new))["result"] == "fail"


def test_import_slots_referenced_only_by_omitted_bodies_do_not_change_numbering():
    old = graph([body(0, [edge(imported(3))]), body(1, [edge(imported(4))])])
    new = graph([body(0, [edge(imported(0))])])
    assert compare_residual(normalized(old, omitted={0}), normalized(new, {
        0: {"unit": "unit1.cpp", "symbol": "f"}}))["result"] == "pass"


def test_import_sharing_is_stable_across_node_traversal_and_distinct_equal_imports():
    old = graph([body(0, [edge(imported(0)), edge(imported(1), 4)]),
                 body(1, [edge(imported(0))])])
    new = graph([body(0, [edge(imported(88))]),
                 body(1, [edge(imported(88)), edge(imported(77), 4)])])
    ids = {0: {"unit": "unit1.cpp", "symbol": "f"},
           1: {"unit": "unit0.cpp", "symbol": "f"}}
    assert compare_residual(normalized(old), normalized(new, ids))["result"] == "pass"
    new["canonical_nodes"][1]["edges"][1]["target"]["slot"] = 88
    assert compare_residual(normalized(old), normalized(new, ids))["result"] == "fail"


def test_metadata_json_types_do_not_alias_python_boolean_and_integer():
    observed = graph([body(0)])
    observed["canonical_nodes"][0]["extra_metadata"] = True
    before = normalized(observed)
    observed["canonical_nodes"][0]["extra_metadata"] = 1
    assert compare_residual(before, normalized(observed))["result"] == "fail"


@pytest.mark.parametrize("field,value", [("entry_offset", 4), ("flags", 64),
    ("section_symbols", [{"name": "entry", "offset": 4}]),
    ("folded_frames", [{"symbol": "peer", "frame": {"data": "changed"}}])])
def test_outside_metadata_remains_exact(field, value):
    observed = graph([body(0)])
    before = normalized(observed)
    observed["canonical_nodes"][0][field] = value
    assert compare_residual(before, normalized(observed))["result"] == "fail"


def test_associated_regions_references_and_debug_metadata_are_preserved():
    observed = graph([{"extent_kind": "associated_coff_regions", "folded_group": None,
        "regions": [{"ordinal": 0, "parent": None, "flags": 32, "code_sha256": "code",
                     "symbols": [], "edges": [edge({"node": 1})]}],
        "debug_regions": [{"parent": 0, "format": "fpo_data", "data": "frame", "code_size": 8}]}, body(1)])
    before = normalized(observed)
    observed["canonical_nodes"][0]["debug_regions"][0]["data"] = "other frame"
    assert compare_residual(before, normalized(observed))["result"] == "fail"


@pytest.mark.parametrize("omitted", [set(), {0}])
def test_codeview_type_nodes_are_not_function_nodes_even_in_omitted_bodies(omitted):
    observed = graph([body(0)])
    types = {"format": "codeview5_type_graph", "nodes": [
        {"edges": [{"offset": 4, "target": {"node": 7}}]}]}
    observed["canonical_nodes"][0]["debug_regions"] = [{"types": types}]
    before = normalized(observed, omitted=omitted)
    if not omitted:
        retained = before["canonical"]["nodes"][0]["body"]["debug_regions"][0]["types"]
        assert retained == types
        types["nodes"][0]["edges"][0]["target"]["node"] = 8
        assert compare_residual(before, normalized(observed))["result"] == "fail"


def test_type_format_label_cannot_hide_a_dangling_runtime_edge():
    observed = graph([body(0, [edge({"node": 7})])])
    observed["canonical_nodes"][0]["format"] = "codeview5_type_graph"
    with pytest.raises(ValueError, match="unknown node"):
        normalized(observed, omitted={0})


@pytest.mark.parametrize("damage", ["unknown", "gaps", "missing_body", "node_id", "node_unknown",
    "dangling", "invalid_alias", "invalid_fold", "conflicting_import"])
def test_incomplete_or_invalid_observations_refuse(damage):
    observed = graph([body(0, [edge({"node": 0})])])
    if damage == "unknown": observed["result"] = "unknown"
    elif damage == "gaps": observed["gaps"] = [{"reason": "missing"}]
    elif damage == "missing_body": observed["canonical_nodes"][0] = None
    elif damage == "node_id": observed["nodes"][0]["id"] = True
    elif damage == "node_unknown": observed["nodes"][0]["result"] = "unknown"
    elif damage == "dangling": observed["canonical_nodes"][0]["edges"][0]["target"]["node"] = 3
    elif damage == "invalid_alias": observed["canonical_nodes"][0]["address_alias"] = 3
    elif damage == "invalid_fold": observed["canonical_nodes"][0]["folded_group"] = True
    else:
        observed["canonical_nodes"][0]["edges"] = [edge(imported(0)), edge(imported(0), 4)]
        observed["canonical_nodes"][0]["edges"][1]["target"]["loader_import"]["name"] = "other"
    with pytest.raises(ValueError):
        normalized(observed, omitted={0})


@pytest.mark.parametrize("identities,omitted", [({}, set()), ({True: "x"}, set()),
    ({0: "x"}, {True}), ({0: "x"}, {9}), ({0: {1: "invalid"}}, set()),
    ({0: float("nan")}, set())])
def test_identity_and_omission_inputs_are_exact(identities, omitted):
    with pytest.raises(ValueError):
        normalized(graph([body(0)]), identities, omitted)


def test_duplicate_identities_refuse_and_normalization_does_not_mutate_inputs():
    observed = graph([body(0), body(1)])
    with pytest.raises(ValueError):
        normalized(observed, {0: {"same": "node"}, 1: {"same": "node"}})
    snapshot = deepcopy(observed)
    result = normalized(observed)
    assert observed == snapshot
    result["canonical"]["nodes"][0]["body"]["size"] = 99
    assert observed == snapshot
    with pytest.raises(ValueError):
        compare_residual(result, normalized(observed))
