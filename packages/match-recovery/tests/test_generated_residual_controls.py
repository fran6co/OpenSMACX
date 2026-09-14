"""Adversarial physical code closure and live residual evidence controls."""

from dataclasses import replace
import struct
from types import MappingProxyType

import pytest

from match_recovery.generated_code_residual import _compare_contexts
from match_recovery.linked_context import linked_build
from match_recovery.pdb_fixups import Fixup
from test_linked_code_graph import function, object_bytes
from test_linked_initializers import build, update_artifact
from test_pdb_build_binding import add_bound_pdb
from test_projected_initializer_order import selection
from test_slot_local_initializer_order import physical_context, replace_object


CALL = b"\xe8" + bytes(4) + b"\xc3"
CHANGES = [selection(unit="src/unit0.cpp", version=version) for version in ("baseline", "current")]


def dependency(linked, *, code=b"\xc3", calls_leaf=False, hide=False):
    """Different coherent physical link, before native receipt/epoch capture."""
    functions = [function("_dep", 0x1E0, CALL if calls_leaf else code, public=True,
                          edges=((1, "_leaf", 20),) if calls_leaf else ()),
                 function("_leaf", 0x140, public=True)]
    replace_object(linked, object_bytes(functions, (), {}), index=-1)
    image = bytearray((linked.directory / "app.exe").read_bytes())
    body = bytearray((CALL if calls_leaf else code).ljust(16, b"\xcc"))
    if calls_leaf:
        struct.pack_into("<i", body, 1, 0x401140 - 0x4011E5)
    image[512 + 0x1E0:512 + 0x1F0] = body
    image[512 + 0x140:512 + 0x150] = b"\xc3" + b"\xcc" * 15
    update_artifact(linked, "app.exe", image)
    mapping = (linked.directory / "app.map").read_text().replace(
        " entry point", " 0001:00000140 _leaf 00401140 f dependency.obj\n entry point")
    if hide:
        mapping = "".join(line for line in mapping.splitlines(keepends=True) if " _dep " not in line)
    update_artifact(linked, "app.map", mapping.encode())


def pair(monkeypatch, tmp_path, *, new_code=b"\xc3", leaf=False, hide=False):
    old = physical_context(monkeypatch, tmp_path / "old", codes=(CALL,), references=("_dep",),
                           damage=lambda context: dependency(context))
    new = physical_context(monkeypatch, tmp_path / "new", codes=(b"\xc3",),
                           damage=lambda context: dependency(context, code=new_code, calls_leaf=leaf, hide=hide))
    return old, new


def test_omitted_root_does_not_prune_its_now_unreachable_unchanged_helper(monkeypatch, tmp_path):
    result = _compare_contexts(*pair(monkeypatch, tmp_path), CHANGES)
    assert result["result"] == "pass", result
    assert result["preserved_producers"] == 1
    assert result["omitted_bodies"] == {"baseline": 1, "current": 1}
    assert result["remaining_linked_effects"] == "unknown" and not result["ready_for_collection"]


def test_now_unreachable_changed_helper_is_reverified_and_still_fails(monkeypatch, tmp_path):
    result = _compare_contexts(*pair(monkeypatch, tmp_path, new_code=b"\x90\xc3"), CHANGES)
    assert result["result"] == "fail", result
    assert result["code_residual"]["equal"] is False and result["preserved_producers"] == 1


def test_missing_current_placement_of_old_helper_is_unknown_not_implicit_discard(monkeypatch, tmp_path):
    result = _compare_contexts(*pair(monkeypatch, tmp_path, hide=True), CHANGES)
    assert result["result"] == "unknown", result
    assert "missing, discarded" in result["reason"]


@pytest.mark.parametrize("rounds", [1, 2])
def test_dependencies_newly_discovered_while_rerooting_require_fixed_point(monkeypatch, tmp_path, rounds):
    result = _compare_contexts(*pair(monkeypatch, tmp_path, leaf=True), CHANGES, max_rounds=rounds)
    if rounds == 1:
        assert result["result"] == "unknown" and "did not converge" in result["reason"], result
    else:
        assert result["result"] == "fail" and result["rounds"] == 2, result
        assert result["preserved_producers"] == 2


@pytest.mark.parametrize("damage", ["records", "record_field", "fpo", "pdb_digest", "groups",
                                    "fixup_replacement", "folding_replacement", "folding_objects",
                                    "folding_fixups", "folding_map", "folding_image"])
@pytest.mark.parametrize("at_close", [False, True])
def test_frozen_pdb_and_fold_state_cannot_replace_same_epoch_physical_evidence(tmp_path, damage, at_close):
    context = build(tmp_path / "project")
    add_bound_pdb(context)
    with pytest.raises(ValueError):
        with linked_build(context) as live:
            value = live.inputs
            assert live.code([{"owner": "unit0.obj", "symbol": "_$E1"}])["result"] == "pass"
            if damage == "records":
                object.__setattr__(value.fixups, "_records", MappingProxyType({0x1001: Fixup(20, 0x1140)}))
            elif damage == "record_field":
                # Even an apparently frozen nested record is not a new PDB observation.
                row = Fixup(20, 0x1140)
                object.__setattr__(row, "target_rva", 0x1180)
                object.__setattr__(value.fixups, "_records", MappingProxyType({0x1001: row}))
            elif damage == "fpo":
                object.__setattr__(value.fixups, "fpo_records", (bytes(16),))
            elif damage == "pdb_digest":
                object.__setattr__(value.fixups, "pdb_sha256", "a" * 64)
            elif damage == "groups":
                object.__setattr__(value.folding, "_groups", ((("unit0.obj", "_$E1"), ("dependency.obj", "_dep")),))
            elif damage == "fixup_replacement":
                object.__setattr__(value, "fixups", replace(value.fixups))
            elif damage == "folding_replacement":
                object.__setattr__(value, "folding", replace(value.folding))
            elif damage == "folding_objects":
                from match_recovery.linked_objects import SelectedObjects
                object.__setattr__(value.folding, "_objects", SelectedObjects(value.evidence, value.coverage, value.records))
            elif damage == "folding_fixups":
                object.__setattr__(value.folding, "_fixups", replace(value.fixups))
            elif damage == "folding_map":
                object.__setattr__(value.folding, "_mapping", replace(value.mapping))
            else:
                from match_recovery.pe_coff import PeImage
                object.__setattr__(value.folding, "_image", PeImage(value.image.data, value.fixups.image_sha256))
            if not at_close:
                with pytest.raises(ValueError): live.check_current()
