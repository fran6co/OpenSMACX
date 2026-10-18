"""Pending source identities cannot borrow body, component or fragment proof."""

from copy import deepcopy
from dataclasses import replace
import json

import pytest

from match_recovery.application_binary import select_bindings, verify_task
from match_recovery.cli import main
from match_recovery.ownership_source import project_view
from test_application_commands import combined_object, project
from test_component_original_routing import (component_route_project,
                                              _install_component, _manifest)


def pending(project, claim="", *, keep_product=False, seed=False):
    """Keep a same-named real function available, but do not bind it by chance."""
    if not keep_product:
        (project / "code/unit.cc").write_text("void f() {}\n")
    path = project / "code/unit.hpp"
    if seed:
        configuration = project / "recovery/project.json"
        data = json.loads(configuration.read_text())
        data["source"]["seed_roots"] = ["code/hints"]
        configuration.write_text(json.dumps(data))
        (project / "code/hints").mkdir()
        path = project / "code/hints/pending.hpp"
        (project / "code/unit.hpp").write_text("// No unrelated member-layout work.\n")
    path.write_text(
        f"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001 UNBOUND{claim}\n")
    return path


@pytest.mark.parametrize("claim", ["", " BYTE_EXACT", " SEMANTIC", " BYTE_EXACT SEMANTIC"])
def test_strict_task_retains_pending_identity_without_compiling(project, claim):
    path = pending(project, claim)
    before = path.read_bytes()
    calls = []
    def incidental_same_symbol(*args):
        calls.append(args)
        return combined_object()  # Contains precisely the original _f RET.
    report = verify_task(project, "owner", project / "original.exe",
                         compiler=incidental_same_symbol)
    assert report["result"] == "unknown"
    assert report["compile_attempts"] == 0 and calls == []
    assert report["selected_entities"] == 1
    row, = report["functions"]
    assert row["result"] == "unknown" and "unbound" in row["reason"]
    assert report["ready_for_collection"] is False
    assert path.read_bytes() == before


def test_reviewed_rebinding_restores_strict_measurement_not_incidental_symbol(project):
    from match_recovery.source import Mode, Recipe, State, read_file, remove, write
    from match_recovery.source.publication import publish
    from match_recovery.source_inventory import assert_snapshot_current, source_snapshot
    path = pending(project, " BYTE_EXACT SEMANTIC")
    row, = read_file(path)
    calls = []
    def compiler(*args):
        calls.append(args)
        return combined_object()
    before = verify_task(project, "owner", project / "original.exe", compiler=compiler)
    assert before["result"] == "unknown" and calls == []
    body_path = project / "code/unit.cc"
    body = body_path.read_text()
    restored = replace(row, path=body_path, line=1, mode=Mode.BODY,
                       state=State.IMPLEMENTED, recipe=Recipe.CENSUS, region=body)
    expected = {path: path.read_bytes(), body_path: body_path.read_bytes()}
    replacements = {path: remove(path.read_text(), [row]).encode(),
                    body_path: write(body, [restored]).encode()}
    snapshot = source_snapshot(project)
    publish(expected, replacements,
            validate=lambda: assert_snapshot_current(project, snapshot),
            staging_directory=project / "build/recovery/publications")
    bound, = read_file(body_path)
    assert bound.mode is Mode.BODY and bound.image_spans == row.image_spans
    assert (bound.address, bound.name) == (row.address, row.name)
    assert bound.byte_exact and bound.semantic and read_file(path) == []
    after = verify_task(project, "owner", project / "original.exe", compiler=compiler)
    assert after["result"] == "pass" and after["compile_attempts"] == len(calls) == 1
    assert after["functions"][0]["result"] == "pass"
    # Fresh physical COFF/PE fixture comparison, not compiler, runtime, original
    # boundary, historical ownership or full-game acceptance proof.
    assert after["ready_for_collection"] is False


@pytest.mark.parametrize("action", ["check-binary", "check"])
def test_public_task_command_cannot_pass_pending_unclaimed_body(project, capsys, monkeypatch, action):
    from match_recovery.compiler import Compiler
    pending(project)
    calls = []
    def no_compile(*args):
        calls.append(args)
        raise AssertionError("pending identity was submitted as a compiler body")
    monkeypatch.setattr(Compiler, "__call__", no_compile)
    assert main(["tasks", action, "owner", "--root", str(project),
                 "--image", "original.exe"]) == 3
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == "unknown" and report["ready_for_collection"] is False
    assert calls == []


@pytest.mark.parametrize("seed", [False, True])
def test_real_body_does_not_silently_resolve_competing_pending_identity(project, seed):
    pending(project, " BYTE_EXACT", keep_product=True, seed=seed)
    if seed:
        # Canonical pending originals are not competing legacy body attempts.
        # A misplaced UNBOUND record refuses instead of letting the real body
        # silently consume a historical obligation from a legacy seed root.
        with pytest.raises(ValueError, match="unbound candidate"):
            project_view(project)
        return
    view, snapshot = project_view(project)
    entity = snapshot["records"][0]["entity_id"]
    bindings, refused, targets = select_bindings(view, frozenset({entity}))
    assert bindings == [] and "_f" not in targets
    assert len(refused) == 1 and "unbound" in refused[0]["reason"]


def test_pending_identity_cannot_export_any_component_alias_by_address(project):
    pending(project)
    view, snapshot = project_view(project)
    entity = snapshot["records"][0]["entity_id"]
    bindings, refused, targets = select_bindings(view, frozenset({entity}),
        component_targets={"_f": 0x401000, "_other_alias": 0x401000,
                           "_unrelated": 0x402000})
    assert bindings == [] and len(refused) == 1
    assert targets == {"_unrelated": 0x402000}


def test_malformed_pending_candidate_cannot_supply_fragment_parent_support(project):
    from match_recovery.application_binary import select_fragment_bindings
    from match_recovery.model import Origin
    from match_recovery.partition import Candidate, partition
    from match_recovery.model import Span
    pin = (project / "code/PIN").read_text().split()[1]
    parent = Origin(pin, "pe:va", 0x401000)
    child = Origin(pin, "pe:va", 0x401001)
    view = partition([
        Candidate(parent, (Span(0x401000, 0x401001),), "code/unit.cc", 1, False, "_f"),
        Candidate(child, (Span(0x401001, 0x401002),), "code/unit.cc", 2, False, "_tail",
                  function_fragment={"parent_address": 0x401000, "parent_symbol": "_f",
                                     "reason": "fixture adjacency", "evidence": ["fixture"]}),
    ])
    # A native reader refuses UNBOUND+symbol/fragment. The lower-level selector
    # must refuse too if a caller manufactures an inconsistent dictionary.
    corrupt = deepcopy(view)
    parent_entity = next(entity for group in corrupt["groups"] for entity in group["entities"]
                         if entity["entity_id"] == parent.entity_id)
    parent_entity["candidates"][0]["unbound"] = True
    bindings, _, _ = select_bindings(corrupt, frozenset({child.entity_id}))
    selected, fragments, support, refused = select_fragment_bindings(
        corrupt, frozenset({child.entity_id}), bindings)
    assert selected == [] and fragments == [] and support == set()
    assert refused and all(row["result"] == "unknown" for row in refused)


def test_variant_rebinding_retains_pending_disposition_and_no_emitted_fallback(project):
    from match_recovery.measurement import Measurement
    from match_recovery.variants import _trial_bindings
    pending(project)
    view, snapshot = project_view(project)
    entity = snapshot["records"][0]["entity_id"]
    source = "code/unit.cc"
    original = (project / source).read_bytes()
    bindings, refused, targets, changes = _trial_bindings(
        view, {source: original}, {source: original.replace(b"{}", b"{ /* spelling */ }")},
        frozenset({entity}), Measurement.read(project), snapshot["image_sha256"])
    assert bindings == [] and changes == [] and "_f" not in targets
    assert len(refused) == 1 and "unbound" in refused[0]["reason"]


def test_shared_body_alias_cannot_supply_pending_canonical_identity(project):
    from match_recovery.model import Origin, Span
    from match_recovery.shared_source import SharedFunctionBinding, validate_shared
    pending(project)
    view, snapshot = project_view(project)
    origin = Origin(snapshot["image_sha256"], "pe:va", 0x401000)
    shared = SharedFunctionBinding(origin, Span(0x401000, 0x401001), "_shared",
                                   origin, "code/unit.cc", 1)
    with pytest.raises(ValueError, match="unbound original identity"):
        validate_shared((shared,), view)


def test_unbound_row_does_not_acquire_component_owner_from_identical_span(component_route_project):
    from match_recovery.component_original_routing import (
        component_record_identity_candidates, exact_component_record_bindings)
    root = component_route_project
    manifest, archive = _manifest((root / "original/input.exe").read_bytes())
    _install_component(root, manifest, archive,
        marker="// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001 UNBOUND BYTE_EXACT\n")
    view, snapshot = project_view(root)
    entity = snapshot["records"][0]["entity_id"]
    assert exact_component_record_bindings(snapshot) == {}
    # The raw collision stays visible; it does not grant callable ownership.
    assert ("engine/runtime.recovery.h", entity) in component_record_identity_candidates(snapshot)
    bindings, refused, targets = select_bindings(view, frozenset({entity}),
        component_targets={"_f": 0x401000})
    assert bindings == [] and "_f" not in targets and len(refused) == 1
    assert "unbound" in refused[0]["reason"]
