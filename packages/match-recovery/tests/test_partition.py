from dataclasses import replace
import json
import subprocess

import pytest

from match_recovery.ownership_source import class_hint, project_view, structural_impact
from match_recovery.cli import main
from match_recovery.includes import include_graph
from match_recovery.model import Origin, Span
from match_recovery.partition import Candidate, partition, select_group


PIN = "a" * 64


def candidate(address, path="src/widget.cpp", *, owner="Widget", seed=False):
    return Candidate(Origin(PIN, "pe:va", address), (Span(address, address + 8),),
                     path, 1, seed, "?f@Widget@@QAEXXZ", owner)


def test_class_partition_combines_seeds_without_mechanically_rehoming_them():
    source = candidate(0x401000)
    seed = candidate(0x402000, "src/unrecovered/00402000.cpp", seed=True)
    duplicate = replace(source, path="src/recovered/00401000.cpp", seed=True)
    view = partition([seed, source, duplicate])
    group = select_group(view, "class:Widget")
    assert view["entity_count"] == group["entity_count"] == 2
    assert group["source_homes"] == ["src/widget.cpp"]
    assert group["historical_tu"] == "unproven"
    assert len(group["entities"][0]["candidates"]) == 2
    assert view["persisted"] is False


def test_address_neighbours_are_not_forced_into_a_translation_unit():
    one = candidate(0x401000, owner=None, seed=True)
    two = candidate(0x401008, owner=None, seed=True)
    view = partition([one, two])
    assert select_group(view, "unassigned")["entity_count"] == 2
    assert view["historical_tu_boundaries"] == "not_inferred"


def test_component_bindings_override_coarse_candidate_ownership():
    seed = candidate(0x401000, "src/unrecovered/00401000.cpp", owner=None, seed=True)
    vendor = replace(seed, path="src/vendor/codec.c", seed=False, component="codec")
    group = select_group(partition([seed, vendor]), "component:codec")
    assert group["next_action"] == "verify_dependency"


@pytest.mark.parametrize("change", [
    {"owner_hint": "Another"},
    {"spans": (Span(0x401000, 0x401010),)},
    {"path": "src/another.cpp"},
])
def test_conflicting_sources_are_visible_and_not_automatically_chosen(change):
    first = candidate(0x401000)
    group = select_group(partition([first, replace(first, **change)]), "conflict")
    assert group["entity_count"] == 1
    assert group["entities"][0]["conflicts"]
    assert group["next_action"] == "resolve_conflict"


@pytest.mark.parametrize(("symbol", "owner"), [
    ("?on_redraw@Widget@@QAEXXZ", "Widget"),
    ("??0Widget@@QAE@XZ", "Widget"),
    ("??1Widget@@QAE@XZ", "Widget"),
    ("?f@Inner@Outer@@QAEXXZ", "Outer::Inner"),
    ("?helper@namespace_name@@YAXXZ", None),
    ("??_GWidget@@UAEPAXI@Z", None),
    ("??__EGlobal@@YAXXZ", None),
    ("_free_function", None),
])
def test_class_hints_do_not_turn_namespaces_or_initializers_into_classes(symbol, owner):
    assert class_hint(symbol) == owner


def test_include_closure_handles_cycles_both_conditional_branches_and_relative_lookup():
    sources = {"src/a.cpp": '#include "inc/a.h"\n',
               "src/inc/a.h": '#if A\n#include "b.h"\n#else\n#include "c.h"\n#endif\n',
               "src/inc/b.h": '#include "a.h"\n', "src/inc/c.h": "",
               "src/b.h": "", "src/unrelated.cpp": ""}
    graph, widened = include_graph(sources, ("src",))
    assert graph.prerequisites(["src/a.cpp"]) == {"src/a.cpp", "src/inc/a.h", "src/inc/b.h", "src/inc/c.h"}
    assert graph.impacted(["src/inc/c.h"]) == {"src/inc/a.h", "src/inc/b.h", "src/inc/c.h", "src/a.cpp"}
    assert not widened


@pytest.mark.parametrize("directive", ['#include HEADER', '#include "missing.h"', '#include <external.h>'])
def test_unresolved_includes_overinvalidate_instead_of_dropping_dependencies(directive):
    sources = {"src/a.cpp": directive, "src/a.h": "", "src/b.h": ""}
    graph, widened = include_graph(sources, ("src",))
    assert graph.prerequisites(["src/a.cpp"]) == sources.keys()
    assert widened == ("src/a.cpp",)


def test_continued_include_directive_is_not_missed():
    graph, _ = include_graph({"src/a.cpp": '#inc' + 'lude \\\n"a.h"', "src/a.h": ""}, ("src",))
    assert "src/a.cpp" in graph.impacted(["src/a.h"])


def test_comments_between_preprocessor_tokens_do_not_hide_an_include():
    sources = {"src/a.cpp": '#/**/include /* annotation */ "a.h"\n', "src/a.h": ""}
    graph, widened = include_graph(sources, ("src",))
    assert "src/a.cpp" in graph.impacted(["src/a.h"])
    assert not widened


def test_comment_spelling_in_a_string_does_not_erase_later_directives():
    sources = {"src/a.cpp": 'char *s = "/*";\n#include "a.h"\n// */\n', "src/a.h": ""}
    graph, _ = include_graph(sources, ("src",))
    assert "src/a.cpp" in graph.impacted(["src/a.h"])


@pytest.fixture
def project(tmp_path):
    (tmp_path / "recovery").mkdir()
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1, "source": {
            "image_pin": "src/IMAGE_PIN", "address_space": "pe:va",
            "seed_roots": ["src/recovered", "src/unrecovered"], "symbol_format": "msvc"}}))
    src = tmp_path / "src"
    src.mkdir()
    (src / "IMAGE_PIN").write_text("sha256 " + PIN + "\n")
    (src / "widget.cpp").write_text('''#include "widget.h"
// ORIGINAL: 0x00401000 ?f@Widget@@QAEXXZ 0x00401000-0x00401008
void Widget::f() {}
''')
    (src / "widget.h").write_text("class Widget { public: void f(); };\n")
    (src / "caller.cpp").write_text('''#include "widget.h"
// ORIGINAL: 0x00402000 ?caller@@YAXXZ 0x00402000-0x00402008
void caller() {}
''')
    return tmp_path


def test_real_adapter_connects_class_source_and_structural_recheck_scope(project):
    view, snapshot = project_view(project)
    group = select_group(view, "class:Widget")
    impact = structural_impact(snapshot, group)
    assert impact["root_files"] == ["src/widget.cpp", "src/widget.h"]
    assert "src/caller.cpp" in impact["recheck_files"]
    assert len(impact["recheck_entities"]) == 2
    assert not impact["scope_complete"]


def test_partition_cli_is_read_only_and_does_not_write_a_second_map(project, capsys):
    before = sorted(path.relative_to(project) for path in project.rglob("*"))
    assert main(["partition", "--root", str(project), "--group", "class:Widget", "--impact"]) == 0
    output = json.loads(capsys.readouterr().out)
    assert "entities" not in output["groups"][0]
    assert output["groups"][0]["impact"]["recheck_entities"]
    assert sorted(path.relative_to(project) for path in project.rglob("*")) == before


def test_task_cli_keeps_discoveries_through_scope_expansion_and_context_rebuild(project, capsys):
    def git(*args):
        return subprocess.run(["git", "-c", "user.name=Test", "-c", "user.email=test@invalid.test",
                               "-c", "commit.gpgsign=false", "-c", "core.hooksPath=/dev/null", *args],
                              cwd=project, capture_output=True, check=True)

    git("init", "--initial-branch=master")
    git("add", "src", "recovery/project.json")
    git("commit", "-m", "Synthetic source baseline")
    baseline = git("rev-parse", "HEAD").stdout.decode().strip()
    git("switch", "-c", "framework")
    (project / "README.md").write_text("Tooling-only checkpoint\n")
    git("add", "README.md")
    git("commit", "-m", "Tooling changes must not strand a source baseline")
    common = ["--root", str(project)]
    assert main(["tasks", "create", "widget", *common, "--group", "class:Widget", "--author", "coordinator",
                 "--intent", "Fix member ownership", "--evidence", "source:src/widget.h",
                 "--target", "synthetic", "--base", baseline]) == 0
    assert json.loads(capsys.readouterr().out)["task"]["base_revision"] == baseline
    other = Origin(PIN, "pe:va", 0x402000).entity_id
    assert main(["tasks", "discover", "widget", *common, "--author", "worker", "--request-id", "finding",
                 "--hypothesis", "Caller accesses a member as a global", "--affects", other,
                 "--evidence", "source:src/caller.cpp", "--expected-revision", "0"]) == 0
    capsys.readouterr()
    assert main(["tasks", "status", "widget", *common]) == 0
    assert json.loads(capsys.readouterr().out)["next_action"] == "review_scope_expansion"
    assert main(["tasks", "expand", "widget", *common, "--author", "coordinator", "--request-id", "expand",
                 "--subject", other, "--edit-file", "src/caller.cpp", "--evidence", "source:src/caller.cpp",
                 "--expected-revision", "1"]) == 0
    capsys.readouterr()
    assert main(["tasks", "context", "widget", *common]) == 0
    output = json.loads(capsys.readouterr().out)
    assert len(output["subjects"]) == 2
    assert output["decision_state"]["next_action"] == "evaluate_structural_discoveries"
    assert output["decision_state"]["pending_discoveries"]
    assert not output["persisted"]
    assert main(["tasks", "resolve", "widget", *common, "--author", "reviewer", "--request-id", "resolve",
                 "--discovery-id", "finding", "--disposition", "incorporated",
                 "--source-evidence", "source:src/widget.h", "--expected-revision", "2"]) == 0
    capsys.readouterr()
    assert main(["tasks", "status", "widget", *common]) == 0
    assert not json.loads(capsys.readouterr().out)["ready_for_collection"]
    (project / "src" / "widget.h").write_text("// uncommitted layout change\n")
    assert main(["tasks", "create", "dirty", *common, "--group", "class:Widget", "--author", "coordinator",
                 "--intent", "New migration", "--evidence", "source:src/widget.h", "--target", "synthetic"]) == 3
    assert "checkpoint source" in capsys.readouterr().err
