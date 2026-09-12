"""Real preservation producer/consumer with synthetic application objects.

These tests exercise native history, per-file controls and residual comparisons;
compiler output is controlled, not a live game/compiler acceptance claim.
"""

from dataclasses import replace
import json
from pathlib import Path
import re

import pytest

from match_recovery.build_graph import ApplicationGraph
from match_recovery.source_history import revision_files
from match_recovery.source_replica import replica_snapshot
from match_recovery.tasks import TaskStore
from test_binary import coff_fixture
from test_build_graph_commands import Context, stamp
from test_preservation import project, Compiler, check
from test_prepared_context import prepared
from test_prepared_unit_preservation import compiler_calls, configure, change_presence, NEW_UNIT, check_unchanged
from test_function_witness import object_bytes


def application(root, *, task_name="owner", object_for=None):
    store = TaskStore(root)
    task, decisions = store.read(task_name), store.decisions(task_name)
    versions = {"baseline": revision_files(root, task.base_revision, ("code",)),
                "current": {name: raw for name, raw in replica_snapshot(root, ("code",)).items() if raw is not None}}
    contexts = {}
    for version, files in versions.items():
        context = Context(root / "build" / ("app-" + version), sorted(name for name in files if name.endswith(".cc")))
        context.policy["compile_link"]["source_extensions"] = [".cc"]
        context.save_policy()
        for name, raw in files.items():
            path = context.root / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(raw)
        for row in context.proof["compilations"]:
            source = Path(row["source"])
            name = source.relative_to(context.root).as_posix()
            raw = Compiler(root).output(root / name, files) if object_for is None else object_for(name, files)
            Path(row["output"]).write_bytes(raw)
            row["compilation"]["output"]["after"] = stamp(raw)
            capture = next(item for item in row["compilation"]["inputs"] if item["path"] == str(source))
            capture["before"] = capture["after"] = stamp(source.read_bytes())
        contexts[version] = context

    def fresh():
        try:
            contexts["baseline"].check_current()
        finally:
            contexts["current"].check_current()

    return ApplicationGraph(root, task_name, task.base_revision, ("code",), contexts["baseline"], contexts["current"],
                            versions, {"result": "unknown"},
                            {"task": task.to_dict(), "decisions": decisions, "integration": None}, fresh)


@pytest.mark.parametrize("changed", [False, True])
def test_preservation_captures_actual_baseline_and_current_compiles(project, changed):
    if changed:
        path = project / "code/owner.cc"
        path.write_text(path.read_text().replace("return 1", "return 2"))
    graph = application(project)
    report = check(project, application=graph)
    assert report["result"] == "pass"
    assert report["compile_attempts"] >= 6  # No current object relabeled as baseline.
    for row in report["units"]:
        objects = row["application_objects"]
        assert [item["version"] for item in objects] == ["baseline", "current"]
        assert all(item["result"] == "pass" for item in objects)
        assert all(item["flags"] == row["flags"] and item["unit"] == row["unit"] for item in objects)
        assert all(item["linked_effects"] == "unknown" for item in objects)
    assert not report["ready_for_collection"] and report["outside_scope_verification"] == "unknown"
    assert '"raw"' not in json.dumps(report)


def test_bound_live_objects_do_not_waive_outside_code_regression(project):
    header = project / "code/model.h"
    header.write_text(header.read_text() + "// padding\n")
    report = check(project, application=application(project))
    assert report["result"] == "fail"
    assert all(item["result"] == "pass" for row in report["units"] for item in row["application_objects"])
    assert any(row["compiled_comparison"] == "fail" for row in report["units"])


def test_each_trial_keeps_its_own_object_even_when_another_matches_the_link(project):
    path = project / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["flag_sets"] = [["optimize"], ["different"]]
    path.write_text(json.dumps(config))

    class Trials(Compiler):
        def __call__(self, source, command, flags):
            return self.overlay(source, {}, command, flags)

        def overlay(self, source, replacements, command, flags):
            raw = super().overlay(source, replacements, command, flags)
            return coff_fixture(b"\x90\xc3") if flags == ("different",) else raw

    report = check(project, compiler=Trials(project), application=application(project))
    assert report["result"] == "pass"  # Actual outside bytes agree within each trial.
    assert len(report["units"]) == 4
    for row in report["units"]:
        expected = "pass" if row["flags"] == ["optimize"] else "unknown"
        assert all(item["result"] == expected for item in row["application_objects"])
        assert all(item["flags"] == row["flags"] for item in row["application_objects"])


@pytest.mark.parametrize("field,value", [("task_name", "another"), ("baseline_revision", "1" * 40),
                                       ("roots", ("code", "extra")), ("_authorization", {"task": "other"})])
def test_mismatched_live_graph_refuses_before_any_preservation_compile(project, field, value):
    compiler = Compiler(project)
    with pytest.raises(ValueError, match="differs from preservation"):
        check(project, compiler=compiler, application=replace(application(project), **{field: value}))
    assert not compiler.calls


def test_missing_neighbor_source_is_not_a_matching_graph_version(project):
    graph = application(project)
    sources = graph.source_versions
    del sources["baseline"]["code/model.h"]
    compiler = Compiler(project)
    with pytest.raises(ValueError, match="source versions"):
        check(project, compiler=compiler, application=replace(graph, _sources=sources))
    assert not compiler.calls


def test_failed_application_guard_cannot_return_an_earlier_preservation_report(project):
    graph = application(project)

    class Damage(Compiler):
        def __call__(self, source, command, flags):
            raw = super().__call__(source, command, flags)
            graph.current.fail_at = graph.current.checks + 1
            return raw

    with pytest.raises(ValueError, match="context changed"):
        check(project, compiler=Damage(project), application=graph)


@pytest.mark.parametrize("remove,diagnostic", [(False, False), (True, False), (False, True)])
def test_prepared_added_removed_tu_has_only_its_actual_present_version_trial(prepared, compiler_calls, remove, diagnostic):
    base = configure(prepared, initially_present=remove)
    change_presence(prepared, remove=remove)

    def emitted(name, files):
        symbols = re.findall(rb"// ORIGINAL: 0x[0-9A-Fa-f]+ (\S+)", files[name])
        return object_bytes([(symbol.decode(), b"\xc3", ()) for symbol in symbols])

    graph = application(prepared, task_name="prepared-owner", object_for=emitted)
    report = check_unchanged(prepared, base, application=graph, diagnostic=diagnostic)
    assert report["result"] == ("unknown" if diagnostic else "pass"), report
    assert compiler_calls
    row, = [item for item in report["units"] if item["unit"] == NEW_UNIT]
    present = "baseline" if remove else "current"
    assert [item["version"] for item in row["application_objects"]] == [present]
    assert row["application_objects"][0]["result"] == "pass"
    for unit in report["units"]:
        assert all(item["result"] == "pass" for item in unit["application_objects"])
        assert all(item["linked_effects"] == "unknown" for item in unit["application_objects"])
