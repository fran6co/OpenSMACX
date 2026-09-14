"""Physical ICF alias controls for native generated-code union re-rooting."""

import json
import struct

import pytest

from match_recovery import linked_context
from match_recovery.generated_code_residual import _compare_contexts
from match_recovery.linked_code_graph import observe_code_graph
from match_recovery.linked_code_ownership import identities, resolve
from match_recovery.linked_context import linked_build
from test_linked_initializers import stamp, update_artifact
from test_pdb_build_binding import add_bound_pdb
from test_slot_local_initializer_order import physical_context, replace_object


CALL = b"\xe8" + bytes(4) + b"\xc3"


def fold_dependency(context, *, corrupt_peer=False):
    """Same code, but retain distinct original compiled physical producers."""
    log = (context.directory / "link.log").read_text().replace("Start Pass2\n",
        "Selected symbol:\n_$E1 from unit1.obj\nReplaced symbol(s):\n_dep from dependency.obj\n"
        "ICF total savings: 16 bytes\nStart Pass2\n")
    update_artifact(context, "link.log", log.encode())
    receipt = json.loads((context.directory / "link.json").read_text())
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(context, "link.json", json.dumps(receipt).encode())
    mapping = (context.directory / "app.map").read_text().replace(
        "0001:000001e0 _dep 004011e0 f dependency.obj", "0001:00000020 _dep 00401020 f unit1.obj")
    update_artifact(context, "app.map", mapping.encode())
    image = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<i", image, 513, 0x401020 - 0x401005)
    update_artifact(context, "app.exe", image)
    if corrupt_peer:
        raw = bytearray(context.objects[-1].read_bytes())
        raw[60:62] = b"\x90\xc3"
        replace_object(context, bytes(raw), index=-1)
    add_bound_pdb(context)


def context(monkeypatch, root, *, corrupt_peer=False):
    return physical_context(monkeypatch, root, codes=(CALL, b"\xc3"), references=("_dep", None),
                            damage=lambda linked: fold_dependency(linked, corrupt_peer=corrupt_peer))


def test_same_source_initial_graph_contains_selected_alias_and_original_producer(monkeypatch, tmp_path):
    physical = context(monkeypatch, tmp_path / "project")
    with linked_build(physical) as live:
        observed = live.initializers()
        assert observed["result"] == observed["code_dependencies"]["result"] == "pass", observed
        graph = observed["code_dependencies"]
        peer, = [node for node in graph["nodes"] if node["symbol"] == "_dep"]
        assert peer["owner"] == "unit1.obj" and peer["source_owner"] == "dependency.obj"
        stable = identities(physical, graph["nodes"])
        assert stable[peer["id"]] == {"kind": "compiled", "unit": "src/dependency.cpp", "symbol": "_dep"}
        roots = resolve(physical, stable.values(), live.inputs.objects, live.inputs.mapping, live.inputs.folding)
        assert {"owner": "dependency.obj", "symbol": "_dep"} in roots
        # Prove the old strategy's actual failure. The ordinary graph's refusal
        # remains essential; the residual driver must avoid unnecessary aliases.
        old_roots = [row["function"] for row in observed["sequence"]] + list(roots)
        expanded = observe_code_graph(live.inputs.objects, live.inputs.mapping, live.inputs.image, old_roots,
                                     fixups=live.inputs.fixups, folding=live.inputs.folding)
        assert expanded["result"] == "unknown", expanded
        assert any("prequeued" in row["reason"] or "preallocated" in row["reason"]
                   for row in expanded["gaps"]), expanded["gaps"]


def test_union_reroot_does_not_requeue_already_verified_folded_producers(monkeypatch, tmp_path):
    old = context(monkeypatch, tmp_path / "old")
    new = context(monkeypatch, tmp_path / "new")
    roots = []
    original = linked_context.observe_code_graph
    def observe(*args, **kwargs):
        roots.append(list(args[3]))
        return original(*args, **kwargs)
    monkeypatch.setattr(linked_context, "observe_code_graph", observe)
    result = _compare_contexts(old, new, [], max_rounds=1)
    assert result["result"] == "pass", result
    assert result["preserved_producers"] == 3 and result["rounds"] == 1
    assert result["omitted_bodies"] == {"baseline": 0, "current": 0}
    # No new producer needs placement: reuse the fresh complete initial graph,
    # rather than rerunning it with selected/original alias spellings prequeued.
    assert roots == []
    assert result["remaining_linked_effects"] == "unknown" and not result["ready_for_collection"]


@pytest.mark.parametrize("version", ["baseline", "current"])
def test_present_folded_peer_is_still_independently_verified_not_skipped(monkeypatch, tmp_path, version):
    old = context(monkeypatch, tmp_path / "old", corrupt_peer=version == "baseline")
    new = context(monkeypatch, tmp_path / "new", corrupt_peer=version == "current")
    result = _compare_contexts(old, new, [])
    assert result["result"] == "fail", result
    assert result["reason"] == "initializer or reachable code observation is incomplete"
    graph = result["initializers"][version]["code_dependencies"]
    peer, = [node for node in graph["nodes"] if node["symbol"] == "_dep"]
    assert peer["result"] == "fail"
