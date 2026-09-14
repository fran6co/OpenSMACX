"""Physical native whole-output controls; no passing verifier doubles."""
from copy import deepcopy
import json
import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery import linked_output_preservation as output
from test_linked_output_preservation import fixture as original_fixture, observed, image_damage
from test_linked_initializers import update_artifact
from test_linked_modules import set_link

def fixture(root, *, body_change=False, policy="/OPT:ICF", executable_debug=False, fold=False):
    context = original_fixture(root, body_change=body_change, policy=policy)
    raw = bytearray((context.directory / "app.exe").read_bytes())
    rva, = struct.unpack_from("<I", raw, 0x98 + 96 + 6 * 8)
    assert rva == 0x20C0  # Ordinary metadata lives in nonexecuting data.
    if executable_debug:
        raw[0x300:0x31C] = raw[0x4C0:0x4DC]
        raw[0x4C0:0x4DC] = bytes(28)
        struct.pack_into("<I", raw, 0x98 + 96 + 6 * 8, 0x1100)
    update_artifact(context, "app.exe", bytes(raw))
    if fold:
        mapping = (context.directory / "app.map").read_bytes()
        mapping = mapping.replace(b"0001:000001e0 _dep 004011e0 f dependency.obj", b"0001:00000000 _dep 00401000 f unit0.obj")
        update_artifact(context, "app.map", mapping)
        evidence = json.loads((context.directory / "link.json").read_bytes())
        description = deepcopy(context.report["compile_link"]["command_bindings"]["link"]["description"])
        log = (context.directory / "link.log").read_text().replace("Start Pass2", "Selected symbol:\n_$E1 from unit0.obj\nReplaced symbol(s):\n_dep from dependency.obj\nICF total savings: 16 bytes\nStart Pass2")
        set_link(context, evidence, log)
        context.report["compile_link"]["command_bindings"]["link"]["description"] = description
    return context


def compare(a, b, *, alter_graph=None):
    with linked_build(a) as old, linked_build(b) as new:
        graphs, bodies = {}, {}
        for version, native in (("baseline", old), ("current", new)):
            graphs[version], bodies[version] = observed(native)
        if alter_graph:
            alter_graph(graphs, bodies)
        return output.compare_linked_output(old, new, graphs, bodies)


@pytest.mark.parametrize("fold", [False, True])
def test_icf_whole_equality_has_no_body_waiver(tmp_path, fold):
    a, b = fixture(tmp_path / "old", fold=fold), fixture(tmp_path / "new", fold=fold)
    if fold:
        with linked_build(a) as native:
            graph, _ = observed(native)
            assert len(native.inputs.folding.group("unit0.obj", "_$E1").members) == 2
            assert any(row.get("folded_group") is not None for row in graph["nodes"])
    result = compare(a, b)
    assert result["result"] == "pass", result
    assert result["proof_basis"] == "whole_projected_image_equality"
    assert result["outside_bytes"]["scoped_file_bytes"] == 0
    assert result["outside_bytes"]["scoped_bodies"] == []
    assert "body_qualification" not in result
    assert not result["ready_for_collection"]
    assert all("runtime_protection" not in row for row in result["metadata"].values())
    assert all(row["runtime_protection_count"] > 0 for row in result["metadata"].values())


@pytest.mark.parametrize("policy,expected", [("/OPT:ICF", "unknown"), ("/OPT:NOICF", "pass")])
def test_differing_body_requires_old_qualification(tmp_path, policy, expected):
    result = compare(fixture(tmp_path / "old", policy=policy), fixture(tmp_path / "new", policy=policy, body_change=True))
    assert result["result"] == expected, result
    assert result["whole_output"]["outside_bytes"] == "fail"
    if expected == "pass":
        assert result["proof_basis"] == "source_scoped_output_residual"
        assert result["outside_bytes"]["scoped_file_bytes"] == 16
    else:
        assert "NOICF" in result["reason"]


def test_projected_field_in_unobserved_executable_storage_refuses(tmp_path):
    result = compare(fixture(tmp_path / "old", executable_debug=True), fixture(tmp_path / "new", executable_debug=True))
    assert result["result"] == "unknown"
    assert "executable storage" in result["reason"]


@pytest.mark.parametrize("damage", ["data", "overlay", "import", "header"])
def test_no_body_projection_can_hide_other_bytes(tmp_path, damage):
    a, b = fixture(tmp_path / "old"), fixture(tmp_path / "new")
    if damage == "overlay":
        update_artifact(b, "app.exe", (b.directory / "app.exe").read_bytes() + b"extra")
    else:
        image_damage(b, {"data": 0x450, "import": 0x590, "header": 0x40}[damage])
    result = compare(a, b)
    assert result["result"] == "unknown", result
    assert result["whole_output"]["different_file_bytes"] > 0


def test_task_scope_is_not_used_as_a_whole_equality_waiver(tmp_path):
    result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new"),
                     alter_graph=lambda graphs, bodies: bodies["baseline"].clear())
    assert result["result"] == "pass", result
    assert result["outside_bytes"]["scoped_file_bytes"] == 0


def test_authenticated_identification_changes_still_have_zero_body_waiver(tmp_path):
    a, b = fixture(tmp_path / "old"), fixture(tmp_path / "new")
    raw = bytearray((b.directory / "app.exe").read_bytes())
    struct.pack_into("<I", raw, 0x88, 42)
    struct.pack_into("<I", raw, 0x4C4, 71)
    update_artifact(b, "app.exe", bytes(raw))
    result = compare(a, b)
    assert result["result"] == "pass", result
    assert result["proof_basis"] == "whole_projected_image_equality"
    assert result["outside_bytes"]["scoped_file_bytes"] == 0
    assert result["metadata"]["baseline"]["image_sha256"] != result["metadata"]["current"]["image_sha256"]


def test_different_identification_field_shape_stays_unknown(tmp_path):
    result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "longer"))
    assert result["result"] == "unknown", result
    assert "fields or logical sidecar" in result["reason"]


def test_missing_runtime_extent_is_not_negative_alias_evidence(tmp_path):
    def damage(graphs, bodies):
        graphs["current"]["nodes"][0]["proof"]["regions"] = []
    result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new"), alter_graph=damage)
    assert result["result"] == "unknown"
    assert "missing runtime regions" in result["reason"]


def test_associated_data_protection_cannot_be_omitted_by_empty_task_scope(tmp_path):
    def damage(graphs, bodies):
        # Adversarial supplied final graph: protection must not ignore its data
        # region just because the caller's task-body list is empty.
        for version, graph in graphs.items():
            graph["nodes"][0]["proof"]["regions"] = [{"address": 0x4020C0, "size": 28}]
            bodies[version] = []
    result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new"), alter_graph=damage)
    assert result["result"] == "unknown"
    assert "observed runtime region" in result["reason"]


@pytest.mark.parametrize("damage", ["graph", "epoch"])
def test_whole_fast_return_keeps_final_guards(tmp_path, monkeypatch, damage):
    a, b = fixture(tmp_path / "old"), fixture(tmp_path / "new")
    with linked_build(a) as old, linked_build(b) as new:
        ga, ba = observed(old)
        gb, bb = observed(new)
        real = output.compare_same_layout_pe
        def compare_then_damage(*args, **kwargs):
            result = real(*args, **kwargs)
            if damage == "graph":
                ga["late"] = True
            else:
                a.alive = False
            return result
        monkeypatch.setattr(output, "compare_same_layout_pe", compare_then_damage)
        try:
            with pytest.raises(ValueError, match="scope or graph changed|expired"):
                output.compare_linked_output(old, new, {"baseline": ga, "current": gb}, {"baseline": ba, "current": bb})
        finally:
            a.alive = True  # Teardown only; no accepted observation escaped.
