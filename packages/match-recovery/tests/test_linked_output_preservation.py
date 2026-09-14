"""Physical composition controls, not a task-authorization or compiler double.

Synthetic native receipts establish real COFF/PE/PDB/command inputs. All linked
verifiers run normally; the caller-owned body list is a unit-test scope, not an
application permission or a claim that a recovered game works.
"""

from copy import deepcopy
from pathlib import Path
import json
import struct

import pytest

from match_recovery import linked_output_preservation as output
from match_recovery.linked_context import linked_build
from match_recovery.msf import SmallMsf
from test_linked_pe_metadata import locator_fixture
from test_linked_modules import set_link
from test_link_file_evidence import stamp
from test_linked_initializers import update_artifact
from test_ordinary_contributions import set_contributions
from test_pdb_contributions import contribution
from test_slot_local_initializer_order import replace_object
from test_msf import small_msf


def fixture(root, *, body_change=False, damage=None, policy="/OPT:NOICF", sidecar="app.pdb"):
    context = locator_fixture(root)
    description = deepcopy(context.report["compile_link"]["command_bindings"]["link"]["description"])
    evidence = json.loads((context.directory / "link.json").read_bytes())
    if policy:
        evidence["arguments"].append(policy)
    set_link(context, evidence, (context.directory / "link.log").read_text())
    context.report["compile_link"]["command_bindings"]["link"]["description"] = description
    set_contributions(context, contribution(offset=0, size=16, flags=0x60501020))
    if body_change:
        change_code(context, 0, 0x200)
    if sidecar != "app.pdb":
        rename_sidecar(context, sidecar)
    if damage:
        damage(context)
    return context


def change_code(context, index, image_offset):
    path = context.objects[index]
    raw = bytearray(path.read_bytes())
    start, = struct.unpack_from("<I", raw, 40)
    raw[start:start + 16] = b"\x90\xc3".ljust(16, b"\xcc")
    replace_object(context, bytes(raw), index=index)
    for row in context.report["compile_link"]["compilations"]:
        if row["output"] == str(path):
            row["compilation"]["output"]["after"] = stamp(raw)
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[image_offset:image_offset + 16] = raw[start:start + 16]
    update_artifact(context, "app.exe", bytes(image))


def image_damage(context, offset):
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[offset] ^= 1
    update_artifact(context, "app.exe", bytes(raw))


def rename_sidecar(context, name):
    old, new = context.directory / "app.pdb", context.directory / name
    old.rename(new)
    for row in context.report["outputs"]:
        if row["format"] == "msvc-pdb":
            row["path"] = name
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"] = [word.replace(str(old), str(new)) for word in evidence["arguments"]]
    description = deepcopy(context.report["compile_link"]["command_bindings"]["link"]["description"])
    for operand in description["operands"]:
        if operand["path"] == str(old):
            operand["path"] = str(new)
            operand["end"] += len(str(new)) - len(str(old))
    set_link(context, evidence, (context.directory / "link.log").read_text())
    context.report["compile_link"]["command_bindings"]["link"]["description"] = description
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[0x610:] = ("Q:\\" + "\\".join(new.parts[1:])).encode() + b"\0"
    struct.pack_into("<I", raw, 0x4D0, len(raw) - 0x600)
    update_artifact(context, "app.exe", bytes(raw))
    context.pin_outputs()


def observed(native):
    graph = native.code([{"owner": "unit0.obj", "symbol": "_$E1"},
                         {"owner": "dependency.obj", "symbol": "_dep"}])
    assert graph["result"] == "pass", graph
    node = next(row for row in graph["nodes"] if row["symbol"] == "_$E1")
    return graph, [{"node_id": node["id"], "entity_id": "test-owned-body",
                   "address": node["proof"]["address"], "size": node["proof"]["size"]}]


def compare(old, new):
    with linked_build(old) as baseline, linked_build(new) as current:
        graphs, bodies = {}, {}
        for version, native in (("baseline", baseline), ("current", current)):
            graphs[version], bodies[version] = observed(native)
        return output.compare_linked_output(baseline, current, graphs, bodies)


@pytest.mark.parametrize("body_change", [False, True])
def test_real_native_composition_qualifies_identical_or_owned_body_change(tmp_path, body_change):
    old = fixture(tmp_path / "old")
    new = fixture(tmp_path / "new", body_change=body_change)
    report = compare(old, new)
    assert report["result"] == report["outside_scope_verification"] == "pass", report
    assert report["outside_bytes"]["scoped_file_bytes"] == (16 if body_change else 0)
    assert report["outside_bytes"]["different_file_bytes"] == 0
    assert report["metadata"]["baseline"]["locator_identity"] == {"format": "msvc-pdb", "path": "app.pdb"}
    assert not report["ready_for_collection"]
    assert all("projected_bytes" not in row for row in report["metadata"].values())


@pytest.mark.parametrize("kind", ["unowned_code", "unowned_data", "import_name", "import_hint", "overlay", "header"])
def test_non_owned_code_data_and_nonidentification_bytes_remain_failures(tmp_path, kind):
    def damage(context):
        if kind == "unowned_code":
            change_code(context, 1, 0x3E0)
        elif kind == "overlay":
            raw = (context.directory / "app.exe").read_bytes() + b"unowned overlay"
            update_artifact(context, "app.exe", raw)
        else:
            image_damage(context, {"unowned_data": 0x450, "import_name": 0x592,
                                   "import_hint": 0x590, "header": 0x40}[kind])
    result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new", body_change=True, damage=damage))
    assert result["result"] == result["outside_scope_verification"] == "fail", result
    assert result["outside_bytes"]["different_file_bytes"] > 0


def test_authenticated_timestamps_and_pdb_ids_are_the_only_metadata_exceptions(tmp_path):
    def damage(context):
        raw = bytearray((context.directory / "app.exe").read_bytes())
        struct.pack_into("<I", raw, 0x88, 987)
        struct.pack_into("<I", raw, 0x4C4, 654)
        struct.pack_into("<II", raw, 0x608, 123, 3)
        original = SmallMsf((context.directory / "app.pdb").read_bytes())
        streams = [original.stream(index) for index in range(original.stream_count)]
        info, dbi = bytearray(streams[1]), bytearray(streams[3])
        struct.pack_into("<II", info, 4, 123, 3)
        struct.pack_into("<I", dbi, 8, 3)
        streams[1], streams[3] = bytes(info), bytes(dbi)
        update_artifact(context, "app.pdb", small_msf(streams))
        update_artifact(context, "app.exe", bytes(raw))
    result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new", body_change=True, damage=damage))
    assert result["result"] == "pass", result
    assert result["metadata"]["baseline"]["image_sha256"] != result["metadata"]["current"]["image_sha256"]


@pytest.mark.parametrize("policy", [None, "/DEBUG", "/OPT:ICF"])
def test_unqualified_folding_policy_still_refuses_composition(tmp_path, policy):
    result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new", body_change=True, policy=policy))
    assert result["result"] == result["outside_scope_verification"] == "unknown"
    assert "NOICF" in result["reason"]


def test_changed_metadata_field_extent_is_not_covered_by_locator_normalization(tmp_path):
    result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "longer"))
    assert result["result"] == "unknown", result
    assert "fields or logical sidecar" in result["reason"]


def test_different_logical_sidecar_is_not_equated_even_with_same_encoded_length(tmp_path):
    result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new", sidecar="alt.pdb"))
    assert result["result"] == "unknown", result
    assert "fields or logical sidecar" in result["reason"]


@pytest.mark.parametrize("damage", ["scope", "graph", "epoch"])
def test_last_comparison_cannot_bypass_final_scope_and_epoch_guards(tmp_path, monkeypatch, damage):
    old, new = fixture(tmp_path / "old"), fixture(tmp_path / "new")
    with linked_build(old) as baseline, linked_build(new) as current:
        ga, ba = observed(baseline)
        gb, bb = observed(current)
        graphs, bodies = {"baseline": ga, "current": gb}, {"baseline": ba, "current": bb}
        original = output.compare_same_layout_pe
        def change(*args, **kwargs):
            report = original(*args, **kwargs)  # Actual native byte comparison, never a mocked PASS.
            if damage == "scope":
                ba[0]["entity_id"] = "unapproved-replacement"
            elif damage == "graph":
                ga["unexpected"] = "changed after comparison"
            else:
                old.alive = False
            return report
        monkeypatch.setattr(output, "compare_same_layout_pe", change)
        try:
            with pytest.raises(ValueError, match="scope or graph changed|expired"):
                output.compare_linked_output(baseline, current, graphs, bodies)
        finally:
            old.alive = True  # Test teardown only; no result was admitted.


def test_report_remains_defensive_without_replaying_it_as_proof(tmp_path):
    old, new = fixture(tmp_path / "old"), fixture(tmp_path / "new", body_change=True)
    first = compare(old, new)
    expected = deepcopy(first)
    first["body_qualification"]["baseline"]["bodies"][0]["size"] = 10000
    first["metadata"]["baseline"]["fields"].clear()
    assert compare(old, new) == expected
