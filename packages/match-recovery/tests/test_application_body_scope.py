"""Ownership shape tests; no source authorization or acceptance grant."""

from copy import deepcopy
from dataclasses import replace
from hashlib import sha256
import struct

import pytest

from match_recovery.application_body_scope import validate_body_node
from match_recovery.pe_coff import Coff
from match_recovery.linked_codeview import procedure_correspondence
from test_linked_code_graph import graph_build, direct_graph, function
from test_linked_bundle import graph as bundle_graph, fixture as runtime_fixture
from test_linked_bundle_fpo import fixture as fpo_fixture
from test_linked_codeview import object_bytes as cv_object, corresponding
from test_codeview_procedures import procedure


def plain(root, *, aliases=False, code=b"\xc3"):
    functions = [function("_f", 0, code)]
    if aliases:
        functions.append(function("_g", 0, code))
    context = graph_build(root / "physical", functions, roots=[row["name"] for row in functions])
    graph = direct_graph(context)
    assert graph["result"] == "pass", graph
    obj = Coff(context.objects[0].read_bytes())
    return obj, graph


def fpo(root):
    values = fpo_fixture(root, family=False)
    graph = bundle_graph(values)
    assert graph["result"] == "pass", graph
    return values[0], graph


def checked(values, index=0):
    obj, graph = values
    node = graph["nodes"][index]
    return validate_body_node(obj, obj.function(node["symbol"]), node, graph)


def test_independent_physical_code_graph_returns_only_full_root_extent(tmp_path):
    values = plain(tmp_path)
    assert checked(values) == {"address": 0x401000, "size": 16}
    assert values[1]["remaining_linked_effects"] == "unknown"


def test_physical_fpo_pair_does_not_expand_root_omission(tmp_path):
    values = fpo(tmp_path)
    before = deepcopy(values[1])
    assert checked(values) == {"address": 0x401000, "size": 16}
    assert values[1] == before
    assert values[1]["nodes"][0]["proof"]["debug_regions"][0]["format"] == "fpo_data"


def codeview_shape(root):
    # Unit-only structural seam. Linking the typed child is covered by the
    # real /Z7 fixture; this assembled view is not native selection evidence.
    local = (0x1006, struct.pack("<iI", 8, 0x74) + b"\x05input")
    obj = Coff(cv_object(procedure(type_index=0x74, locals_=[local])))
    _, graph = plain(root, code=obj.sections[0].data)
    node = graph["nodes"][0]
    source, target = corresponding()
    metadata = {"parent": 0, **procedure_correspondence(source, target, {}, {})}
    proof = node["proof"]
    proof.update(extent_kind="associated_coff_regions", debug_regions=[metadata], regions=[{
        "section": 1, "ordinal": 0, "parent": None, "address": proof["address"],
        "size": proof["size"], "flags": obj.sections[0].flags,
        "code_sha256": sha256(obj.sections[0].data).hexdigest(), "code": proof["code"]}])
    graph["canonical_nodes"][0] = {"extent_kind": "associated_coff_regions",
        "regions": deepcopy(proof["regions"]), "debug_regions": deepcopy(proof["debug_regions"])}
    return obj, graph


def test_typed_codeview_structural_view_retains_metadata_outside_range(tmp_path):
    assert checked(codeview_shape(tmp_path)) == {"address": 0x401000, "size": 32}


@pytest.mark.parametrize("damage", ["missing", "extra", "format", "parent", "bytes", "canonical"])
def test_missing_or_mismatched_fpo_cannot_qualify_companions(tmp_path, damage):
    obj, graph = fpo(tmp_path)
    proof = graph["nodes"][0]["proof"]
    if damage == "missing":
        proof["debug_regions"] = []
    elif damage == "extra":
        proof["debug_regions"] *= 2
    elif damage == "canonical":
        graph["canonical_nodes"][0]["debug_regions"] = []
    else:
        metadata = proof["debug_regions"][0]
        metadata[{"format": "format", "parent": "parent", "bytes": "data"}[damage]] = "wrong"
    with pytest.raises(ValueError):
        checked((obj, graph))


@pytest.mark.parametrize("field", ["format", "name", "length", "locals", "types"])
def test_codeview_metadata_mismatch_refuses_even_with_a_passing_root(tmp_path, field):
    obj, graph = codeview_shape(tmp_path)
    metadata = graph["nodes"][0]["proof"]["debug_regions"][0]
    metadata[field] = {} if field == "types" else [] if field == "locals" else "wrong"
    graph["canonical_nodes"][0]["debug_regions"] = deepcopy([metadata])
    with pytest.raises(ValueError):
        checked((obj, graph))


def test_complete_native_runtime_child_proof_still_is_not_one_body(tmp_path):
    values = runtime_fixture(tmp_path)
    graph = bundle_graph(values)
    assert graph["result"] == "pass"
    with pytest.raises(ValueError):
        checked((values[0], graph))


@pytest.mark.parametrize("damage", ["data", "executable", "nested"])
def test_section_names_do_not_waive_runtime_or_nested_companions(tmp_path, damage):
    obj, graph = fpo(tmp_path)
    if damage == "nested":
        obj.associations[1] = 2
    else:
        obj.sections = (obj.sections[0], replace(obj.sections[1],
            name=".xdata" if damage == "data" else ".debug$F",
            flags=0xC0301040 if damage == "data" else 0x60501020))
    with pytest.raises(ValueError):
        checked((obj, graph))


@pytest.mark.parametrize("index", [0, 1])
def test_native_same_address_alias_is_refused_for_either_representative(tmp_path, index):
    values = plain(tmp_path, aliases=True)
    with pytest.raises(ValueError, match="overlaps"):
        checked(values, index)


def test_alias_added_after_initial_scope_check_requires_final_graph_recheck(tmp_path):
    values = plain(tmp_path)
    assert checked(values)["size"] == 16
    graph = values[1]
    later = deepcopy(graph["nodes"][0])
    later.update(id=1, symbol="_later", owner="later.obj")
    graph["nodes"].append(later)
    graph["canonical_nodes"].append(deepcopy(graph["canonical_nodes"][0]))
    with pytest.raises(ValueError, match="overlaps"):
        checked(values)


@pytest.mark.parametrize("where", ["node", "canonical"])
@pytest.mark.parametrize("group", [0, 1])
def test_folded_roots_never_receive_whole_body_scope(tmp_path, where, group):
    obj, graph = plain(tmp_path)
    graph["nodes" if where == "node" else "canonical_nodes"][0]["folded_group"] = group
    with pytest.raises(ValueError, match="folded"):
        checked((obj, graph))


@pytest.mark.parametrize("field,value", [("address", None), ("address", -1), ("address", True),
    ("address", 0x100000000), ("size", None), ("size", 0), ("size", -1), ("size", True), ("size", 15)])
def test_missing_negative_or_partial_root_extents_refuse(tmp_path, field, value):
    obj, graph = plain(tmp_path)
    graph["nodes"][0]["proof"][field] = value
    with pytest.raises(ValueError):
        checked((obj, graph))


@pytest.mark.parametrize("damage", ["graph", "node", "proof", "code", "hash", "subject"])
def test_incomplete_or_mismatched_node_evidence_refuses(tmp_path, damage):
    obj, graph = plain(tmp_path)
    node = graph["nodes"][0]
    if damage == "hash":
        graph["canonical_nodes"][0]["code_sha256"] = "0" * 64
    elif damage == "subject":
        node["section"] += 1
    else:
        value = {"graph": graph, "node": node, "proof": node["proof"], "code": node["proof"]["code"]}[damage]
        value["result"] = "unknown"
    with pytest.raises(ValueError):
        checked((obj, graph))
