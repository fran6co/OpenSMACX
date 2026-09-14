"""Gate build hook routing; native graph identity, not report authority.

The bound graph below is an explicit protocol fixture, not a physical linked
preservation proof. No compiler or acceptance verdict is mocked into a native
graph producer. Artifact bytes and the identity/lifetime guards are exercised.
"""

from contextlib import contextmanager
from dataclasses import replace
import hashlib

import pytest

from match_recovery.application_context import ApplicationBuild
from match_recovery.build_graph import ApplicationGraph, _bound_graph
from test_project_gate import (COMPILED, configured, gate, mapped, observed,
                               preparation, project, run)


@contextmanager
def bound(root, *, foreign=False):
    private = root / "build/private-replica"
    directory = private / "build/product"
    directory.mkdir(parents=True)
    raw = b"exact synthetic build output"
    (directory / "sample.exe").write_bytes(raw)
    report = {"result": "pass", "directory": "build/product", "outputs": [
        {"path": "sample.exe", "sha256": hashlib.sha256(raw).hexdigest(), "format": "file"}]}

    def guard():
        if (directory / "sample.exe").read_bytes() != raw:
            raise ValueError("native fixture artifact changed")

    current = ApplicationBuild(private, directory, report, guard)
    graph = ApplicationGraph(root / "different" if foreign else root, "owner", "baseline", ("engine",),
                             current, current, {}, {"result": "pass"}, {}, guard)
    with _bound_graph(graph):
        yield graph


def test_hook_acquired_only_after_preparation_and_kept_through_sweeps(configured, observed):
    preparation(configured)
    events, acquisitions = [], []
    with bound(configured) as graph:
        original = graph.current.report

        def acquire():
            assert {"check": "preparation", "event": "finish", "result": "pass"} in events
            assert (configured / "build/preparation").is_dir()
            acquisitions.append(graph)
            return graph

        report = run(configured, retain_build=True, _application_build=acquire, progress=events.append)
        built = next(row["report"] for row in report["checks"] if row["name"] == "build")
        assert report["result"] == "pass" and acquisitions == [graph]
        assert built["directory"] == graph.current.directory.relative_to(configured).as_posix()
        assert built["outputs"] == original["outputs"]
        assert graph.current.report == original  # Rebasing never changes the native receipt.
    assert not any(name == "check_build" for name, _, _ in observed)
    assert all(any(name == expected for name, _, _ in observed) for expected in COMPILED)


def test_failed_preparation_never_acquires_a_native_build(configured, observed):
    preparation(configured, [{"argv": ["false", "{build}"], "timeout": 10}])
    def acquire():
        pytest.fail("acquired graph before preparation passed")
    assert run(configured, retain_build=True, _application_build=acquire)["result"] != "pass"
    assert not observed


@pytest.mark.parametrize("kind", ["report", "copied", "expired", "foreign"])
def test_unbound_or_wrong_native_build_never_reaches_compiler_sweeps(configured, observed, kind):
    with bound(configured, foreign=kind == "foreign") as graph:
        if kind == "report":
            candidate = graph.current.report
        elif kind == "copied":
            candidate = replace(graph)
        else:
            candidate = graph
        if kind != "expired":
            report = run(configured, retain_build=True, _application_build=lambda: candidate)
    if kind == "expired":
        report = run(configured, retain_build=True, _application_build=lambda: candidate)
    build = next(row for row in report["checks"] if row["name"] == "build")
    assert build["result"] == report["result"] == "unknown"
    assert not any(name in COMPILED for name, _, _ in observed)


def test_gate_rechecks_bound_artifact_after_each_sweep(configured, observed, monkeypatch):
    with pytest.raises(ValueError, match="artifact changed"):
        with bound(configured) as graph:
            def mutate(*args, **kwargs):
                (graph.current.directory / "sample.exe").write_bytes(b"substituted artifact")
                return {"result": "pass"}
            monkeypatch.setattr(gate, "check_matches", mutate)
            report = run(configured, retain_build=True, _application_build=lambda: graph)
            assert report["result"] == "unknown"
            assert report["checks"][-1]["name"] == "freshness"
            assert not any(name == "check_symbols" for name, _, _ in observed)
