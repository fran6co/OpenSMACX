"""Generation isolation/refusal controls; real compiler controls live separately."""

import json
import sys

import pytest

from match_recovery.driver import working_state
from match_recovery.measurement import Measurement
from match_recovery.preparation import prepare_project
from match_recovery.prepared_context import prepared_compiler, source_roots
from test_preservation import project
from test_project_replica import files


GENERATOR = '''import json, subprocess, sys
from pathlib import Path
root, output = Path(sys.argv[1]), Path(sys.argv[2])
head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=root, check=True, capture_output=True).stdout
(output / "head").write_bytes(head)
(output / "generated.h").write_bytes((root / "code/model.h").read_bytes())
(output / "compile_commands.json").write_text(json.dumps([
    {"directory": str(root), "file": str(root / "code/owner.cc"), "arguments": ["compiler", "/FI" + str(output / "generated.h")]}
]))
'''


@pytest.fixture
def prepared(project):
    path = project / "recovery/project.json"
    config = json.loads(path.read_text())
    config.update(preparation="recovery/prepare.json", mappings={"compile_commands": "build/commands.json"},
                  compiler={"command": {"argv": [sys.executable, "-c", "raise RuntimeError('not invoked by this test')"], "timeout": 10},
                            "inputs": ["recovery/generate.py"], "environment": []})
    path.write_text(json.dumps(config))
    (project / "recovery/generate.py").write_text(GENERATOR)
    (project / "recovery/prepare.json").write_text(json.dumps({
        "schema_version": 1, "artifacts": "build/preparation", "input_roots": ["code"],
        "inputs": ["recovery/generate.py"], "environment": [],
        "commands": [{"argv": [sys.executable, "recovery/generate.py", "{root}", "{build}"], "timeout": 10}],
        "outputs": [{"path": "compile_commands.json", "format": "compilation-database"}]}))
    assert prepare_project(project)["published"]
    return project


def test_each_source_version_generates_its_own_headers_and_git_context(prepared):
    settings = Measurement.read(prepared)
    roots = source_roots(prepared, settings)
    assert roots == ("code",)
    current = files(prepared)
    historical = {**current, "code/model.h": b"historical model\n"}
    before = working_state(prepared)
    original_cache = (prepared / "build/commands.json").read_bytes()
    for source in (current, historical):
        with prepared_compiler(prepared, roots, source, settings) as compiler:
            rows = json.loads((compiler.root / "build/commands.json").read_text())
            generated = next((compiler.root / "build/preparation").glob("build-*/generated.h"))
            assert generated.read_bytes() == source["code/model.h"]
            assert generated.with_name("head").read_bytes() == before["head"]
            assert rows[0]["file"] == str(compiler.root / "code/owner.cc")
            assert str(compiler.root) in rows[0]["arguments"][1]
        assert not compiler.root.exists()
    assert working_state(prepared) == before
    assert (prepared / "build/commands.json").read_bytes() == original_cache


@pytest.mark.parametrize("damage", ["outside_recipe", "generator_failure", "generator_edit", "generated_edit"])
def test_generated_context_refusals_preserve_original_project(prepared, damage):
    script = prepared / "recovery/generate.py"
    if damage == "outside_recipe":
        script.write_text(GENERATOR.replace('str(root / "code/owner.cc")', repr(str(prepared / "code/owner.cc"))))
    elif damage == "generator_failure":
        script.write_text("raise SystemExit(1)\n")
    elif damage == "generator_edit":
        script.write_text(GENERATOR + '\n(root / "code/model.h").write_text("generator damage")\n')
    before = working_state(prepared)
    settings = Measurement.read(prepared)
    with pytest.raises(ValueError):
        with prepared_compiler(prepared, source_roots(prepared, settings), files(prepared), settings) as compiler:
            assert damage == "generated_edit"
            generated = next((compiler.root / "build/preparation").glob("build-*/generated.h"))
            generated.write_text("changed context\n")
    assert working_state(prepared) == before


def test_generated_artifact_roots_cannot_overlap_historical_source(prepared):
    path = prepared / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["input_roots"] = ["build"]
    path.write_text(json.dumps(config))
    with pytest.raises(ValueError, match="separate from historical"):
        source_roots(prepared, Measurement.read(prepared))


def versioned(prepared):
    graph = prepared / "BuildGraph.txt"
    graph.write_text("code/owner.cc\n")
    path = prepared / "recovery/prepare.json"
    policy = json.loads(path.read_text())
    policy["versioned_inputs"] = ["BuildGraph.txt"]
    path.write_text(json.dumps(policy))
    script = '''import json,sys
from pathlib import Path
root, output = Path(sys.argv[1]), Path(sys.argv[2])
units = (root / "BuildGraph.txt").read_text().splitlines()
(output / "compile_commands.json").write_text(json.dumps([
    {"directory": str(root), "file": str(root / unit), "arguments": ["compiler", str(root / unit)]}
    for unit in units
]))
'''
    (prepared / "recovery/generate.py").write_text(script)
    return Measurement.read(prepared)


def test_explicit_current_and_historical_build_graphs_generate_real_distinct_membership(prepared):
    settings = versioned(prepared)
    current = files(prepared)
    historical = {name: raw for name, raw in current.items() if name != "code/owner.cc"}
    historical["code/old.cc"] = b"void old_unit() {}\n"
    before = working_state(prepared)
    for source, graph, expected in ((current, b"code/owner.cc\n", "code/owner.cc"),
                                    (historical, b"code/old.cc\n", "code/old.cc")):
        with prepared_compiler(prepared, source_roots(prepared, settings), source, settings,
                               versioned_files={"BuildGraph.txt": graph}) as compiler:
            assert compiler.units == frozenset([expected])
            assert (compiler.root / expected).is_file()
            assert (compiler.root / "BuildGraph.txt").read_bytes() == graph
            missing = "code/old.cc" if expected == "code/owner.cc" else "code/owner.cc"
            assert not (compiler.root / missing).exists()
            assert (compiler.root / "recovery/generate.py").read_bytes() == (prepared / "recovery/generate.py").read_bytes()
    assert working_state(prepared) == before


def test_default_prepared_context_uses_current_explicit_graph(prepared):
    settings = versioned(prepared)
    with prepared_compiler(prepared, source_roots(prepared, settings), files(prepared), settings) as compiler:
        assert compiler.units == frozenset(["code/owner.cc"])


@pytest.mark.parametrize("graph", [b"code/missing.cc\n", b"code/owner.cc\ncode/owner.cc\n"])
def test_missing_or_duplicate_recipe_sources_refuse(prepared, graph):
    settings = versioned(prepared)
    with pytest.raises(ValueError, match="canonical existing source|duplicate prepared"):
        with prepared_compiler(prepared, source_roots(prepared, settings), files(prepared), settings,
                               versioned_files={"BuildGraph.txt": graph}):
            pytest.fail("invalid fresh recipe membership admitted")


@pytest.mark.parametrize("values", [{}, {"BuildGraph.txt": b"code/owner.cc\n", "extra": b"extra"}])
def test_preparation_does_not_fill_in_missing_or_extra_versioned_inputs(prepared, values):
    settings = versioned(prepared)
    with pytest.raises(ValueError, match="exactly their declared files"):
        with prepared_compiler(prepared, source_roots(prepared, settings), files(prepared), settings, versioned_files=values):
            pytest.fail("partial build graph substitution admitted")


@pytest.mark.parametrize("location", ["graph", "database"])
def test_prepared_membership_and_versioned_graph_are_freshly_guarded(prepared, location):
    settings = versioned(prepared)
    before = working_state(prepared)
    with pytest.raises(ValueError, match="changed"):
        with prepared_compiler(prepared, source_roots(prepared, settings), files(prepared), settings) as compiler:
            name = "BuildGraph.txt" if location == "graph" else "build/commands.json"
            (compiler.root / name).write_text("changed\n")
            compiler.check_current()
    assert working_state(prepared) == before


def test_prepared_compiler_refuses_unlisted_source_before_invoking_compiler(prepared, monkeypatch):
    import match_recovery.prepared_context as module
    settings = versioned(prepared)
    seen = []

    class ControlledCompiler:
        def __init__(self, root, **kwargs): self.root = root
        def __call__(self, source, command, flags):
            seen.append((source, command, flags))
            return b"compiler protocol result"

    monkeypatch.setattr(module, "Compiler", ControlledCompiler)
    with prepared_compiler(prepared, source_roots(prepared, settings), files(prepared), settings) as compiler:
        assert compiler(compiler.root / "code/owner.cc", arguments=("argument",), flags=("flag",)) == b"compiler protocol result"
        with pytest.raises(ValueError, match="unique freshly prepared recipe"):
            compiler(compiler.root / "code/model.h", (), ())
        assert len(seen) == 1


@pytest.mark.parametrize("damage", ["graph", "source", "database"])
def test_compiler_cannot_return_output_from_mutated_prepared_context(prepared, monkeypatch, damage):
    import match_recovery.prepared_context as module
    settings = versioned(prepared)

    class MutatingCompiler:
        def __init__(self, root, **kwargs): self.root = root
        def __call__(self, source, arguments, flags):
            name = {"graph": "BuildGraph.txt", "source": "code/owner.cc", "database": "build/commands.json"}[damage]
            (self.root / name).write_text("changed during compiler interaction\n")
            return b"must not be usable output"

    monkeypatch.setattr(module, "Compiler", MutatingCompiler)
    before = working_state(prepared)
    with pytest.raises(ValueError, match="changed"):
        with prepared_compiler(prepared, source_roots(prepared, settings), files(prepared), settings) as compiler:
            compiler(compiler.root / "code/owner.cc", (), ())
            pytest.fail("mutated compiler output returned")
    assert working_state(prepared) == before


def test_aliased_generated_recipe_source_refuses(prepared):
    settings = versioned(prepared)
    script = prepared / "recovery/generate.py"
    script.write_text(script.read_text().replace(
        'units = (root / "BuildGraph.txt").read_text().splitlines()',
        '(output / "alias.cc").symlink_to(root / "code/owner.cc"); units = [str(output / "alias.cc")]'))
    settings = Measurement.read(prepared)
    with pytest.raises(ValueError, match="canonical existing source"):
        with prepared_compiler(prepared, source_roots(prepared, settings), files(prepared), settings):
            pytest.fail("aliased recipe admitted")


def test_nested_recipe_output_resolves_relative_inputs_against_build_directory(prepared):
    policy_path = prepared / "recovery/prepare.json"
    policy = json.loads(policy_path.read_text())
    policy["outputs"][0]["path"] = "nested/compile_commands.json"
    policy_path.write_text(json.dumps(policy))
    (prepared / "recovery/generate.py").write_text('''import json,sys
from pathlib import Path
root, output = Path(sys.argv[1]), Path(sys.argv[2])
(output / "nested").mkdir()
(output / "generated.cc").write_text("int generated_value;\\n")
(output / "nested/compile_commands.json").write_text(json.dumps([
    {"directory": ".", "file": "generated.cc", "arguments": ["compiler", "generated.cc"]}
]))
''')
    settings = Measurement.read(prepared)
    with prepared_compiler(prepared, source_roots(prepared, settings), files(prepared), settings) as compiler:
        generated = next((compiler.root / "build/preparation").glob("build-*/generated.cc"))
        assert compiler.units == frozenset([generated.relative_to(compiler.root).as_posix()])
        assert generated.is_file() and not (generated.parent / "nested/generated.cc").exists()
