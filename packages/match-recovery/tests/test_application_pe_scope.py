"""Physical integration controls; no compiler, native permission or game acceptance."""

import struct

import pytest

from match_recovery import application_residual, generated_code_residual, pe_output_residual
from test_binary import coff_fixture
from test_linked_initializers import update_artifact
from test_slot_local_initializer_order import physical_context, replace_object


def contexts(monkeypatch, root, *, header_damage=False):
    def prepare(context, changed):
        image = bytearray((context.directory / "app.exe").read_bytes())
        # Populate the actual PE geometry fields required by literal coverage.
        struct.pack_into("<II", image, 0x98 + 32, 0x1000, 0x200)
        struct.pack_into("<II", image, 0x98 + 56, 0x3000, 0x200)
        if changed:
            raw = bytearray(coff_fixture(b"\x90\xc3"))
            at, = struct.unpack_from("<I", raw, 8)
            raw[at:at + 8] = b"_dep\0\0\0\0"
            replace_object(context, bytes(raw), index=1)
            image[0x3E0:0x3F0] = b"\x90\xc3".ljust(16, b"\xcc")
            if header_damage:
                image[0x88] ^= 1
        update_artifact(context, "app.exe", bytes(image))
    old = physical_context(monkeypatch, root / "old", damage=lambda value: prepare(value, False))
    new = physical_context(monkeypatch, root / "new", damage=lambda value: prepare(value, True))
    return old, new


def compare(old, new):
    scope = {"src/dependency.cpp": {"_dep": "source-owned-task-function"}}
    return generated_code_residual._compare_contexts(old, new, [],
        application_scope={"baseline": scope, "current": scope})


def test_native_scoped_extent_reaches_literal_pe_comparison_without_promotion(monkeypatch, tmp_path):
    result = compare(*contexts(monkeypatch, tmp_path))
    assert result["result"] == "pass", result
    output = result["pe_output_residual"]
    assert output["same_layout"] == output["outside_bytes"] == "pass", output
    assert output["scoped_bodies"] == [{"entity_id": "source-owned-task-function",
        "address": 0x4011E0, "size": 16, "file_offset": 0x3E0}]
    assert output["different_file_bytes"] == 0
    assert output["result"] == output["outside_scope_verification"] == "unknown"
    assert result["remaining_linked_effects"] == "unknown" and not result["ready_for_collection"]


def test_actual_header_difference_is_visible_but_never_relabels_code_observation(monkeypatch, tmp_path):
    result = compare(*contexts(monkeypatch, tmp_path, header_damage=True))
    assert result["result"] == "pass", result
    output = result["pe_output_residual"]
    assert output["outside_bytes"] == "fail"
    assert output["difference_ranges"] == [{"offset": 0x88, "size": 1}]
    assert output["result"] == output["outside_scope_verification"] == "unknown"


def test_final_graph_scope_rechecks_an_alias_not_present_in_initial_graph(monkeypatch, tmp_path):
    old, new = contexts(monkeypatch, tmp_path)
    original = application_residual._owned_bodies
    calls = []

    def later_alias(context, graph, bindings):
        calls.append(context)
        if len(calls) == 3:
            # Refusal injection only: mutate an otherwise complete graph after
            # both initial scope checks. No synthetic passing proof is granted.
            subject = next(node for node in graph["nodes"] if node["symbol"] == "_dep")
            other = next(node for node in graph["nodes"] if node is not subject)
            other["proof"]["address"] = subject["proof"]["address"]
        return original(context, graph, bindings)

    monkeypatch.setattr(application_residual, "_owned_bodies", later_alias)
    result = compare(old, new)
    assert len(calls) == 3
    assert result["result"] == "unknown" and "overlaps" in result["reason"]
    assert "pe_output_residual" not in result


def test_final_native_guard_rejects_expiry_during_literal_comparison(monkeypatch, tmp_path):
    old, new = contexts(monkeypatch, tmp_path)
    original = pe_output_residual.compare_same_layout_pe

    def expired(*args, **kwargs):
        report = original(*args, **kwargs)
        old.alive = False
        return report

    monkeypatch.setattr(pe_output_residual, "compare_same_layout_pe", expired)
    with pytest.raises(ValueError, match="expired"):
        compare(old, new)
