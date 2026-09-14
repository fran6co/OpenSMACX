"""Physical all-root ICF normalization, without bypassing peer verification."""

import json

import pytest

from match_recovery.application_code_roots import combine_roots
from match_recovery.linked_context import linked_build
from test_generated_residual_folded_roots import context, fold_dependency, CALL
from test_linked_initializers import stamp, update_artifact
from test_slot_local_initializer_order import physical_context


ROOTS = [{"owner": "unit0.obj", "symbol": "_$E1"},
         {"owner": "unit1.obj", "symbol": "_$E1"},
         {"owner": "unit1.obj", "symbol": "_dep"},
         {"owner": "dependency.obj", "symbol": "_dep"}]


@pytest.mark.parametrize("reverse", [False, True])
def test_full_physical_roots_and_rewritten_aliases_keep_every_member(monkeypatch, tmp_path, reverse):
    physical = context(monkeypatch, tmp_path / "project")
    roots = ROOTS[::-1] if reverse else ROOTS
    with linked_build(physical) as live:
        original = live.code(roots)
        assert original["result"] == "unknown"
        assert any("prequeued" in gap["reason"] for gap in original["gaps"])
        normalized = combine_roots(live, roots)
        assert len(normalized) == len(roots)
        assert normalized.count({"owner": "unit1.obj", "symbol": "_dep"}) == 2
        graph = live.code(normalized)
        assert graph["result"] == "pass", graph
        assert len(graph["nodes"]) == 3
        peer, = [node for node in graph["nodes"] if node["symbol"] == "_dep"]
        assert peer["source_owner"] == "dependency.obj"
        assert len(graph["roots"]) == len(roots)


def test_normalization_never_hides_corrupted_original_folded_peer(monkeypatch, tmp_path):
    physical = context(monkeypatch, tmp_path / "project", corrupt_peer=True)
    with linked_build(physical) as live:
        graph = live.code(combine_roots(live, ROOTS))
        assert graph["result"] == "fail", graph
        peer, = [node for node in graph["nodes"] if node["symbol"] == "_dep"]
        assert peer["result"] == "fail"


def test_repeated_ordinary_root_requests_are_not_deduplicated(monkeypatch, tmp_path):
    physical = context(monkeypatch, tmp_path / "project")
    roots = [ROOTS[0], dict(ROOTS[0])]
    with linked_build(physical) as live:
        assert combine_roots(live, roots) == roots
        graph = live.code(combine_roots(live, roots))
        assert graph["result"] == "pass"
        assert graph["roots"] == [0, 0]


@pytest.mark.parametrize("root", [None, {}, {"owner": "unit0.obj"},
    {"owner": "", "symbol": "_$E1"}, {"owner": "unit0.obj", "symbol": "_$E1", "address": 0x401000}])
def test_invalid_root_identity_is_not_silently_dropped(monkeypatch, tmp_path, root):
    physical = context(monkeypatch, tmp_path / "project")
    with linked_build(physical) as live:
        with pytest.raises(ValueError, match="exact owner and symbol"):
            combine_roots(live, [root])


def test_root_binding_expires_with_native_context(monkeypatch, tmp_path):
    physical = context(monkeypatch, tmp_path / "project")
    with linked_build(physical) as live:
        combine_roots(live, ROOTS)
    with pytest.raises(ValueError, match="expired"):
        combine_roots(live, ROOTS)


def test_mutation_during_root_iteration_refuses_final_epoch(monkeypatch, tmp_path):
    physical = context(monkeypatch, tmp_path / "project")
    def roots():
        yield ROOTS[0]
        path = physical.objects[-1]
        path.write_bytes(path.read_bytes() + b"changed during roots")
        yield ROOTS[-1]
    with pytest.raises(ValueError, match="changed"):
        with linked_build(physical) as live:
            combine_roots(live, roots())


@pytest.mark.parametrize("damage", ["overlapping_group", "missing_peer"])
def test_partial_or_overlapping_fold_cannot_normalize_a_passing_prefix(monkeypatch, tmp_path, damage):
    def configure(physical):
        fold_dependency(physical)
        path = physical.directory / "link.log"
        log = path.read_text()
        if damage == "overlapping_group":
            log = log.replace("ICF total savings:", "Selected symbol:\n_$E1 from unit1.obj\n"
                              "Replaced symbol(s):\n_dep from dependency.obj\nICF total savings:")
        else:
            log = log.replace("ICF total savings:", "_absent from dependency.obj\nICF total savings:")
        update_artifact(physical, "link.log", log.encode())
        receipt = json.loads((physical.directory / "link.json").read_bytes())
        receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
        update_artifact(physical, "link.json", json.dumps(receipt).encode())
    physical = physical_context(monkeypatch, tmp_path / "project", codes=(CALL, b"\xc3"),
                                references=("_dep", None), damage=configure)
    with linked_build(physical) as live:
        with pytest.raises(ValueError, match="unique|ambiguous"):
            combine_roots(live, ROOTS)
