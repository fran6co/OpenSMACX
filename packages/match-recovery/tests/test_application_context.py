"""Isolated application-build lifetime controls using a synthetic build client."""

from copy import deepcopy
import json

import pytest

from match_recovery.application_context import application_build
from match_recovery.driver import working_state
from test_build_check import project, command, write_config
from test_task_source import git


GRAPH = "BuildGraph.txt"
SCRIPT = '''import json,subprocess,sys
from pathlib import Path
root, output = Path(sys.argv[1]), Path(sys.argv[2])
sources = {path.relative_to(root).as_posix(): path.read_text()
           for path in sorted((root / 'code').rglob('*')) if path.is_file()}
head = subprocess.run(['git','rev-parse','HEAD'], cwd=root, check=True, capture_output=True, text=True).stdout.strip()
(output / 'app').write_text(json.dumps({'sources': sources, 'graph': (root / 'BuildGraph.txt').read_text(), 'head': head}))
'''


@pytest.fixture
def configured(project):
    (project / GRAPH).write_text("code/main.c\n")
    script = project / "recovery/application.py"
    script.write_text(SCRIPT)
    # Intentionally unusable preparation: application contexts must not invoke it.
    path = project / "recovery/project.json"
    value = json.loads(path.read_text())
    value["preparation"] = "recovery/absent-preparation.json"
    path.write_text(json.dumps(value))
    write_config(project, versioned_inputs=[GRAPH], inputs=["recovery/application.py"],
                 environment=["APPLICATION_CONTEXT_OPTION"],
                 commands=[command("import runpy,sys; sys.argv=sys.argv[1:]; runpy.run_path(sys.argv[0],run_name='__main__')",
                                   "{root}/recovery/application.py", "{root}", "{build}")])
    return project


def files(root):
    return {path.relative_to(root).as_posix(): path.read_bytes() for path in (root / "code").rglob("*") if path.is_file()}


def context(root, supplied=None, *, versioned=None):
    return application_build(root, ("code",), files(root) if supplied is None else supplied,
                             versioned_files={GRAPH: (root / GRAPH).read_bytes()} if versioned is None else versioned)


def test_complete_source_versions_build_privately_without_preparation_and_expire(configured, monkeypatch):
    root = configured
    monkeypatch.setattr("match_recovery.preparation.prepare_project",
                        lambda *a, **k: pytest.fail("application build must not run preparation"))
    current = files(root)
    historical = {name: data for name, data in current.items() if name != "code/main.c"}
    historical["code/old.c"] = b"historical source not present in current checkout\n"
    (root / "build").mkdir(exist_ok=True)
    unrelated_cache = root / "build/unrelated-cache"
    unrelated_cache.write_bytes(b"not an application input")
    before = working_state(root)
    for source, graph in ((current, b"code/main.c\n"), (historical, b"code/old.c\n")):
        original = deepcopy(source)
        with context(root, source, versioned={GRAPH: graph}) as application:
            assert application.root != root and application.root.is_dir()
            assert application.directory.is_dir() and application.directory.is_relative_to(application.root)
            assert not (application.root / "build/unrelated-cache").exists()
            assert application.report["result"] == "pass"
            observed = json.loads((application.directory / "app").read_bytes())
            assert observed == {"sources": {name: data.decode() for name, data in source.items()},
                                "graph": graph.decode(), "head": before["head"].decode().strip()}
            assert files(application.root) == source and source == original
            assert (application.root / GRAPH).read_bytes() == graph
            application.check_current()
        assert not application.root.exists() and not application.directory.exists()
        with pytest.raises(ValueError, match="(expired|closed|active|ended)"):
            application.check_current()
        assert working_state(root) == before
        assert unrelated_cache.read_bytes() == b"not an application input"
    assert not (root / "code/old.c").exists()


def test_report_is_a_defensive_copy_not_mutable_verification_state(configured):
    with context(configured) as application:
        original = application.report
        changed = application.report
        changed["result"] = "fail"
        changed["outputs"][0]["sha256"] = "0" * 64
        changed["commands"].clear()
        assert application.report == original and changed != original
        application.check_current()


@pytest.mark.parametrize("damage", ["source", "source_added", "configuration", "output", "versioned", "index"])
@pytest.mark.parametrize("explicit_check", [False, True])
def test_private_mutations_cannot_escape_context_freshness(configured, damage, explicit_check):
    root = configured
    before = working_state(root)
    with pytest.raises(ValueError, match="(changed|invalid|missing|modified)"):
        with context(root) as application:
            if damage == "index":
                git(application.root, "add", "code")
                # Private files match this fixture's committed source, so stage a real change.
                (application.root / "code/new.c").write_bytes(b"new private source")
                git(application.root, "add", "code/new.c")
            else:
                path = (application.directory / "app" if damage == "output" else application.root / {
                            "source": "code/main.c", "source_added": "code/new.c",
                            "configuration": "recovery/project.json", "versioned": GRAPH}[damage])
                suffix = b"\n " if damage == "configuration" else b"\nchanged\n"
                path.write_bytes((path.read_bytes() if path.exists() else b"") + suffix)
            if explicit_check:
                application.check_current()
    assert working_state(root) == before


@pytest.mark.parametrize("damage", ["source", "configuration", "versioned", "index"])
def test_original_mutations_are_preserved_and_refused(configured, damage):
    root = configured
    changed = None
    with pytest.raises(ValueError, match="(changed|invalid|modified)"):
        with context(root) as application:
            if damage == "index":
                git(root, "add", GRAPH)
            else:
                changed = root / {"source": "code/main.c", "configuration": "recovery/project.json", "versioned": GRAPH}[damage]
                changed.write_bytes(changed.read_bytes() + b"\n ")
            application.check_current()
    if changed is not None:
        assert changed.read_bytes().endswith(b"\n ")
    else:
        assert GRAPH in git(root, "diff", "--cached", "--name-only")


def test_environment_changes_during_context_refuse(configured, monkeypatch):
    monkeypatch.setenv("APPLICATION_CONTEXT_OPTION", "original")
    with pytest.raises(ValueError, match="changed"):
        with context(configured) as application:
            monkeypatch.setenv("APPLICATION_CONTEXT_OPTION", "changed")
            application.check_current()


@pytest.mark.parametrize("damage", ["command", "missing_artifact", "invalid_artifact"])
def test_failed_application_build_cannot_yield_usable_context(configured, damage):
    if damage == "command":
        write_config(configured, commands=[command("raise SystemExit(1)", "{build}")])
    elif damage == "missing_artifact":
        write_config(configured, commands=[command("pass", "{build}")])
    else:
        write_config(configured, outputs=[{"path": "app", "format": "pe32-i386"}])
    before = working_state(configured)
    with pytest.raises(ValueError):
        with context(configured):
            pytest.fail("failed build yielded application evidence")
    assert working_state(configured) == before


@pytest.mark.parametrize("versioned", [{}, {GRAPH: b"old\n", "extra": b"not authorized"}, {GRAPH: "not bytes"}])
def test_exact_declared_versioned_file_set_is_required(configured, versioned):
    before = working_state(configured)
    with pytest.raises(ValueError):
        with context(configured, versioned=versioned):
            pytest.fail("invalid versioned input substitution admitted")
    assert working_state(configured) == before


@pytest.mark.parametrize("damage", ["outside", "not_bytes", "file_directory_collision"])
def test_invalid_supplied_source_tree_is_not_partially_materialized(configured, damage):
    supplied = files(configured)
    if damage == "outside": supplied["recovery/project.json"] = b"unapproved policy replacement"
    elif damage == "not_bytes": supplied["code/main.c"] = "not bytes"
    else: supplied["code/main.c/child"] = b"conflicting child"
    before = working_state(configured)
    with pytest.raises(ValueError):
        with context(configured, supplied):
            pytest.fail("invalid complete source tree admitted")
    assert working_state(configured) == before


@pytest.mark.parametrize("declaration", ["input_root", "input_file"])
def test_explicit_ignored_non_source_build_inputs_are_copied_unchanged(configured, declaration):
    root = configured
    if declaration == "input_root":
        name = "support/ignored.dat"
        ignore = root / ".gitignore"
        ignore.write_text(ignore.read_text() + "/support/ignored.dat\n")
        write_config(root, input_roots=["code", "support"])
    else:
        name = "build/support.dat"
        write_config(root, inputs=["recovery/application.py", name])
    path = root / name
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(b"declared ignored input\x00\xff")
    assert name in git(root, "check-ignore", name)
    unrelated = root / "build/unrelated-cache"
    unrelated.parent.mkdir(exist_ok=True)
    unrelated.write_bytes(b"not a declared input")
    before = working_state(root)
    with context(root) as application:
        assert (application.root / name).read_bytes() == path.read_bytes()
        assert not (application.root / "build/unrelated-cache").exists()
        application.check_current()
    assert working_state(root) == before
    assert path.read_bytes() == b"declared ignored input\x00\xff"
    assert unrelated.read_bytes() == b"not a declared input"


def test_missing_empty_non_source_input_directory_refuses_before_build(configured, monkeypatch):
    root = configured
    (root / "support/empty").mkdir(parents=True)
    (root / "support/input.dat").write_bytes(b"materializes parent, not empty child")
    write_config(root, input_roots=["code", "support"])
    before = working_state(root)
    monkeypatch.setattr("match_recovery.application_context.generate_build",
                        lambda *a, **k: pytest.fail("incomplete non-source replica must not build"))
    with pytest.raises(ValueError, match="exact non-source build inputs"):
        with context(root):
            pytest.fail("missing declared directory yielded application evidence")
    assert (root / "support/empty").is_dir()
    assert working_state(root) == before


@pytest.mark.parametrize("policy", ["project", "build"])
def test_source_roots_cannot_substitute_project_or_build_policy(configured, monkeypatch, policy):
    root = configured
    if policy == "project":
        extra_root = "recovery"
    else:
        extra_root = "policy"
        (root / extra_root).mkdir()
        (root / "policy/build.json").write_bytes((root / "recovery/build.json").read_bytes())
        project_path = root / "recovery/project.json"
        value = json.loads(project_path.read_text())
        value["build"] = "policy/build.json"
        project_path.write_text(json.dumps(value))
    supplied = files(root)
    supplied.update({path.relative_to(root).as_posix(): path.read_bytes()
                     for path in (root / extra_root).rglob("*") if path.is_file()})
    before = working_state(root)
    monkeypatch.setattr("match_recovery.application_context.generate_build",
                        lambda *a, **k: pytest.fail("source-root policy substitution must not build"))
    with pytest.raises(ValueError, match="source roots cannot substitute recovery/build policy"):
        with application_build(root, ("code", extra_root), supplied,
                               versioned_files={GRAPH: (root / GRAPH).read_bytes()}):
            pytest.fail("source roots admitted policy even with unchanged supplied bytes")
    assert working_state(root) == before


@pytest.mark.parametrize("side", ["original", "private"])
def test_ignored_source_outside_build_input_roots_is_checked_immediately(configured, side):
    from match_recovery.build_check import Build

    root = configured
    (root / "extra").mkdir()
    name = "extra/ignored.cc"
    (root / name).write_bytes(b"original ignored source")
    ignore = root / ".gitignore"
    ignore.write_text(ignore.read_text() + "/extra/\n")
    assert name in git(root, "check-ignore", name)
    supplied = {**files(root), name: (root / name).read_bytes()}
    before = working_state(root)
    with pytest.raises(ValueError, match="application inputs or Git state changed"):
        with application_build(root, ("code", "extra"), supplied,
                               versioned_files={GRAPH: (root / GRAPH).read_bytes()}) as application:
            changed_root = root if side == "original" else application.root
            build = Build.read(changed_root)
            prior_inputs, prior_git = build.snapshot(changed_root), working_state(changed_root)
            (changed_root / name).write_bytes(b"new ignored source")
            # Neither declared build inputs nor Git notices this source mutation.
            assert build.snapshot(changed_root) == prior_inputs
            assert working_state(changed_root) == prior_git
            with pytest.raises(ValueError, match="application inputs or Git state changed"):
                application.check_current()
    assert working_state(root) == before
    assert (root / name).read_bytes() == (b"new ignored source" if side == "original" else b"original ignored source")


def test_historical_source_cannot_replace_declared_compiler_helper(configured, monkeypatch):
    root = configured
    helper = root / "code/compiler_helper.py"
    helper.write_bytes(b"current compiler helper\n")
    project_path = root / "recovery/project.json"
    value = json.loads(project_path.read_text())
    value["compiler"] = {"command": command("pass"), "inputs": ["code/compiler_helper.py"], "environment": []}
    project_path.write_text(json.dumps(value))
    supplied = files(root)
    supplied["code/compiler_helper.py"] = b"historical compiler helper\n"
    before = working_state(root)
    monkeypatch.setattr("match_recovery.application_context.generate_build",
                        lambda *a, **k: pytest.fail("changed compiler policy must refuse before building"))
    monkeypatch.setattr("match_recovery.compiler.invoke",
                        lambda *a, **k: pytest.fail("compiler snapshots must not invoke the compiler"))
    with pytest.raises(ValueError, match="changed compiler or toolchain policy inputs"):
        with context(root, supplied):
            pytest.fail("source substitution replaced declared compiler policy")
    assert working_state(root) == before
    assert helper.read_bytes() == b"current compiler helper\n"


def test_substituted_toolchain_discovery_is_refused_before_its_first_execution(configured, tmp_path_factory, monkeypatch):
    from match_recovery.toolchain import snapshot

    root = configured
    external = tmp_path_factory.mktemp("application-context-toolchain")
    (external / "compiler").write_bytes(b"synthetic compiler identity, never executed")
    (external / "sdk").mkdir()
    (external / "sdk/header.h").write_bytes(b"synthetic header")
    marker = external / "substituted-discovery-executed"
    description = {"schema_version": 1, "files": {"compiler": str(external / "compiler")},
                   "directories": {"sdk": str(external / "sdk")}}
    discovery = ("import json,sys\n"
                 "assert json.load(sys.stdin) == {'schema_version':1,'operation':'toolchain_inputs'}\n"
                 f"print(json.dumps({description!r}))\n")
    helper = root / "code/discover.py"
    helper.write_text(discovery)
    project_path = root / "recovery/project.json"
    value = json.loads(project_path.read_text())
    value["toolchain"] = {
        "command": command("import runpy; runpy.run_path('code/discover.py',run_name='__main__')"),
        "inputs": ["code/discover.py"], "environment": []}
    project_path.write_text(json.dumps(value))
    # Exercise the real discovery protocol first: refusal below must not stem
    # from a broken original discovery or an unsupported external inventory.
    assert set(snapshot(root)["inputs"]) == {"compiler", "sdk"}
    supplied = files(root)
    supplied["code/discover.py"] = (
        f"from pathlib import Path\nPath({str(marker)!r}).write_bytes(b'executed')\n" + discovery).encode()
    before = working_state(root)
    monkeypatch.setattr("match_recovery.application_context.generate_build",
                        lambda *a, **k: pytest.fail("substituted discovery must refuse before building"))
    with pytest.raises(ValueError, match="changed compiler or toolchain policy inputs"):
        with context(root, supplied):
            pytest.fail("substituted toolchain discovery yielded application evidence")
    assert not marker.exists(), "private snapshot executed substituted discovery before checking its bytes"
    assert helper.read_text() == discovery
    assert working_state(root) == before
