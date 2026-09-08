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
