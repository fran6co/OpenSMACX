from concurrent.futures import ThreadPoolExecutor
from dataclasses import replace
import json

import pytest

from match_recovery.graph import DependencyGraph, Edge
from match_recovery.migration import GateExit, Migration, Review, check_obligations
from match_recovery.model import (Component, Delivery, Dimension, Measurement,
                                  Obligation, Origin, Ownership, Result, Span,
                                  canonical, digest)
from match_recovery.store import CorruptObject, ObjectStore


PIN = "a" * 64
SNAPSHOT = "b" * 64


def test_identity_separates_images_and_overlays():
    origin = Origin(PIN, "overlay:0", 0x1000)
    assert len({origin.entity_id, replace(origin, address_space="overlay:1").entity_id,
                replace(origin, image_sha256="c" * 64).entity_id}) == 3
    assert origin.entity_id == Origin(PIN, "overlay:0", 0x1000).entity_id


@pytest.mark.parametrize("address", [-1, True, 1.5, "123"])
def test_reject_invalid_addresses(address):
    with pytest.raises(ValueError):
        Origin(PIN, "va", address)


@pytest.mark.parametrize("bounds", [(1, 1), (2, 1), (-1, 1), (True, 2)])
def test_reject_invalid_extents(bounds):
    with pytest.raises(ValueError):
        Span(*bounds)


def test_canonical_encoding_is_deterministic_and_rejects_nan():
    assert digest({"b": 1, "a": 2}) == digest({"a": 2, "b": 1})
    with pytest.raises(ValueError):
        canonical({"confidence": float("nan")})
    assert canonical(frozenset({"b", "a"})) == canonical(["a", "b"])
    with pytest.raises(ValueError):
        canonical({1: "ambiguous", "1": "key"})


@pytest.mark.parametrize(("owner", "delivery", "workflow"), [
    (Ownership.APPLICATION, Delivery.RECOVERED_SOURCE, "recover"),
    (Ownership.THIRD_PARTY, Delivery.UPSTREAM_SOURCE, "verify_dependency"),
    (Ownership.RUNTIME, Delivery.TOOLCHAIN, "verify_dependency"),
    (Ownership.SYSTEM, Delivery.SYSTEM_IMPORT, "verify_dependency"),
    (Ownership.GENERATED, Delivery.GENERATED, "recover_owner"),
    (Ownership.UNKNOWN, Delivery.UNRESOLVED, "identify"),
    (Ownership.THIRD_PARTY, Delivery.UNRESOLVED, "resolve_dependency"),
])
def test_classification_selects_workflow_not_exemption(owner, delivery, workflow):
    assert Component("component", owner, delivery, ("observation",)).workflow == workflow


def test_classification_requires_evidence():
    with pytest.raises(ValueError):
        Component("library", Ownership.THIRD_PARTY, Delivery.UPSTREAM_SOURCE, ())


def test_store_preserves_observations_and_detects_corruption(tmp_path):
    store = ObjectStore(tmp_path)
    first = store.put({"observation": "candidate matched", "snapshot": "old"})
    store.put({"observation": "candidate regressed", "snapshot": "new"})
    assert store.get(first)["observation"] == "candidate matched"
    path = tmp_path / first[:2] / (first[2:] + ".json")
    path.write_text(json.dumps({"observation": "rewritten history"}))
    with pytest.raises(CorruptObject):
        store.get(first)
    with pytest.raises(CorruptObject):
        store.put({"observation": "candidate matched", "snapshot": "old"})


def test_store_concurrent_publication_is_idempotent(tmp_path):
    store = ObjectStore(tmp_path)
    with ThreadPoolExecutor(max_workers=8) as workers:
        ids = list(workers.map(lambda _: store.put({"same": "evidence"}), range(32)))
    assert len(set(ids)) == 1
    assert store.get(ids[0]) == {"same": "evidence"}
    assert not list(tmp_path.rglob(".publish-*"))


def test_store_rejects_path_traversal(tmp_path):
    with pytest.raises(ValueError):
        ObjectStore(tmp_path).get("../../outside")


def test_discovery_invalidates_transitive_dependents_and_handles_cycles():
    graph = DependencyGraph([Edge("ctor", "class", "layout", "e1"),
                             Edge("caller", "ctor", "call", "e2"),
                             Edge("class", "ctor", "evidence", "e3")])
    assert graph.impacted(["class"]) == {"class", "ctor", "caller"}
    extended = graph.with_discovery([Edge("other", "caller", "call", "e4")])
    assert extended.impacted(["class"]) == {"class", "ctor", "caller", "other"}
    assert graph.impacted(["class"]) == {"class", "ctor", "caller"}
    assert extended.prerequisites(["other"]) == {"other", "caller", "ctor", "class"}


@pytest.fixture
def case():
    obligations = (Obligation("constructor", Dimension.BYTES, "historical-match"),
                   Obligation("caller", Dimension.REFERENCES, "historical-bindings"))
    current = {item.obligation_id: SNAPSHOT for item in obligations}
    measurements = [Measurement(item.obligation_id, SNAPSHOT, Result.PASS, "measured")
                    for item in obligations]
    migration = Migration("hierarchy", "worker", frozenset({"constructor", "caller"}),
                          obligations, frozenset({obligations[0].obligation_id}),
                          ("original-construction-order",))
    checks = dict(snapshot=SNAPSHOT, changed=frozenset({"constructor"}),
                  review=Review("worker", "reviewer", SNAPSHOT, Result.PASS, ("review",)),
                  build=Result.PASS, structural_checks=Result.PASS, references=Result.PASS,
                  measurements=measurements, current=current)
    return migration, checks


def test_supported_regression_is_admitted_but_not_green_or_closed(case):
    migration, checks = case
    checks["measurements"][0] = replace(checks["measurements"][0], result=Result.FAIL)
    report = migration.admit(**checks)
    assert report.exit_code == GateExit.TRACKED_REGRESSIONS != 0
    with pytest.raises(ValueError, match="restore every"):
        migration.close(**checks)
    checks["measurements"][0] = replace(checks["measurements"][0], result=Result.PASS)
    assert migration.close(**checks).exit_code == GateExit.PASSED
    assert len(migration.protected) == 2


@pytest.mark.parametrize("defect", ["missing", "stale", "unknown", "unexpected"])
def test_regression_allowance_never_hides_missing_or_unexpected_results(case, defect):
    migration, checks = case
    if defect == "missing":
        checks["measurements"].pop(0)
    elif defect == "stale":
        checks["measurements"][0] = replace(checks["measurements"][0], fingerprint="c" * 64)
    elif defect == "unknown":
        checks["measurements"][0] = replace(checks["measurements"][0], result=Result.UNKNOWN)
    else:
        checks["measurements"][1] = replace(checks["measurements"][1], result=Result.FAIL)
    with pytest.raises(ValueError, match="not admissible"):
        migration.admit(**checks)


@pytest.mark.parametrize("defect", ["self-review", "stale-review", "scope", "build", "structure", "references"])
def test_migration_admission_checks(case, defect):
    migration, checks = case
    if defect == "self-review":
        checks["review"] = replace(checks["review"], reviewer="worker")
    elif defect == "stale-review":
        checks["review"] = replace(checks["review"], snapshot="c" * 64)
    elif defect == "scope":
        checks["changed"] = frozenset({"unrelated"})
    elif defect == "build":
        checks["build"] = Result.FAIL
    elif defect == "references":
        checks["references"] = Result.UNKNOWN
    else:
        checks["structural_checks"] = Result.UNKNOWN
    with pytest.raises(ValueError):
        migration.admit(**checks)


def test_unrelated_new_match_cannot_replace_protected_match(case):
    migration, checks = case
    checks["measurements"][0] = Measurement("new-obligation", SNAPSHOT, Result.PASS, "new")
    assert check_obligations(migration.protected, checks["measurements"],
                             checks["current"]).exit_code == GateExit.UNMEASURABLE


def test_duplicate_measurements_are_not_last_writer_wins(case):
    migration, checks = case
    checks["measurements"].append(checks["measurements"][0])
    with pytest.raises(ValueError, match="ambiguous"):
        migration.admit(**checks)


def test_migration_can_be_fingerprinted_and_preserves_reference_checks(case):
    migration, checks = case
    assert len(digest(migration)) == 64
    with pytest.raises(ValueError, match="only matching"):
        replace(migration, allowed_regressions=frozenset(
            {migration.protected[1].obligation_id}))
