"""Fresh per-version recipe coverage with the real configured VC6 interaction.

Synthetic C++ only. Retained compiler metadata is deliberately not waived, so
this control proves coverage and detection, not acceptance of a TU split.
"""

import json
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.model import Origin
from match_recovery.preparation import prepare_project
from match_recovery.preservation import check_preservation
from match_recovery.tasks import Task, TaskStore


def test_project_preparation_versions_build_graph_and_disables_pch_generation():
    from match_recovery.build_check import Build
    repository = Path(__file__).resolve().parents[2]
    preparation = Build.read(repository, key="preparation")
    assert preparation.versioned_inputs == ("CMakeLists.txt",)
    command, = preparation.commands
    assert command.argv.count("-DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON") == 1


def test_real_prepared_added_unit_cannot_hide_unowned_storage(tmp_path, monkeypatch):
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    root = tmp_path
    repository = Path(__file__).resolve().parents[2]
    for name in ("code", "recovery", "build"):
        (root / name).mkdir()
    (root / ".gitignore").write_text("build/\n")
    (root / "code/PIN").write_text("sha256 " + "a" * 64 + "\n")
    (root / "code/owner.cpp").write_text(
        '// ORIGINAL: 0x00401000 _owner 0x00401000-0x00401006\n'
        'extern "C" int owner() { return 1; }\n')
    (root / "BuildGraph.txt").write_text("code/owner.cpp\n")
    (root / "recovery/generate.py").write_text(
        "import json,sys\nfrom pathlib import Path\n"
        "root,output,compiler = Path(sys.argv[1]),Path(sys.argv[2]),sys.argv[3]\n"
        "rows = [{'directory':str(root),'file':str(root / name),"
        "'arguments':[compiler,'/nologo','/c',str(root / name)]} "
        "for name in (root / 'BuildGraph.txt').read_text().splitlines()]\n"
        "(output / 'compile_commands.json').write_text(json.dumps(rows))\n")
    config = {
        "schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code"], "include_paths": ["code"]},
        "preparation": "recovery/prepare.json",
        "mappings": {"compile_commands": "build/compile_commands.json"},
        "compiler": {"command": {"argv": [sys.executable, str(repository / "recovery/environment/adapter.py")], "timeout": 60},
                     "inputs": [], "environment": ["VC6_PREFIX", "VC6_ROOT", "WINEARCH"]},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "application",
                        "flag_sets": [["/c", "/O2", "/Gy"]], "layout_flags": [], "unit_extensions": [".cpp"],
                        "inputs": ["build/compile_commands.json"], "input_roots": []}}
    (root / "recovery/project.json").write_text(json.dumps(config))
    (root / "recovery/prepare.json").write_text(json.dumps({
        "schema_version": 1, "artifacts": "build/preparation", "input_roots": ["code"],
        "inputs": ["recovery/generate.py"], "versioned_inputs": ["BuildGraph.txt"], "environment": [],
        "commands": [{"argv": [sys.executable, "recovery/generate.py", "{root}", "{build}",
                                str(repository / "recovery/environment/vc6-cl")], "timeout": 60}],
        "outputs": [{"path": "compile_commands.json", "format": "compilation-database"}]}))

    def git(*args):
        return subprocess.run(["git", "-c", "user.name=Compiler fixture", "-c", "user.email=fixture@example.invalid", *args],
                              cwd=root, check=True, capture_output=True, text=True).stdout.strip()

    git("init", "-q", "--initial-branch=work")
    git("add", ".")
    git("commit", "-qm", "Synthetic explicit build graph baseline")
    baseline = git("rev-parse", "HEAD")
    TaskStore(root).create(Task(
        "owner", "synthetic", "coordinator", "Exercise added TU coverage", baseline,
        (Origin("a" * 64, "pe:va", 0x401000), Origin("a" * 64, "pe:va", 0x401010)),
        ("code/owner.cpp", "code/added.cpp", "BuildGraph.txt"), ("synthetic source",)))
    git("add", ".")
    git("commit", "-qm", "Authorize fixture task")
    assert prepare_project(root)["published"]
    assert check_preservation(root, "owner")["result"] == "pass"
    (root / "code/added.cpp").write_text(
        '// ORIGINAL: 0x00401010 _added 0x00401010-0x00401016\n'
        'extern "C" int added() { return 2; }\nint unowned_storage = 7;\n')
    (root / "BuildGraph.txt").write_text("code/owner.cpp\ncode/added.cpp\n")
    assert prepare_project(root)["published"]
    before = git("diff")
    report = check_preservation(root, "owner")
    assert report["base_revision"] == baseline
    assert report["recipe_membership"] == {"baseline": ["code/owner.cpp"],
                                            "current": ["code/added.cpp", "code/owner.cpp"]}
    assert not report["preparation_error"] and not report["unsupported_file_set_changes"]
    assert report["result"] == "fail", report
    added = next(row for row in report["units"] if row["unit"] == "code/added.cpp")
    assert added["replica"] == "pass" and added["compiled_comparison"] == "fail"
    assert added["unit_presence"] == {"baseline": False, "current": True}
    assert any(row["symbol"] == "?unowned_storage@@3HA" for row in added["symbol_changes"])
    assert all(control["result"] == "pass" for control in report["controls"])
    assert git("diff") == before and TaskStore(root).read("owner").base_revision == baseline
    assert not report["ready_for_collection"] and not report["persisted"]
