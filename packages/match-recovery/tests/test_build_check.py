"""Configuration-only fresh builds, real compiler damage and failure controls."""

import json
import shutil
import sys

import pytest

from match_recovery.build_check import Build, check_build
from match_recovery.cli import main
from test_task_source import git


def write_config(root, **changes):
    path = root / "recovery/build.json"
    value = json.loads(path.read_text())
    value.update(changes)
    path.write_text(json.dumps(value))


def command(script, *args, timeout=5):
    return {"argv": [sys.executable, "-c", script, *args], "timeout": timeout}


@pytest.fixture
def project(tmp_path):
    (tmp_path / "code").mkdir()
    (tmp_path / "recovery").mkdir()
    (tmp_path / ".gitignore").write_text("build/\n")
    (tmp_path / "code/values.h").write_text("#define VALUE 0\n")
    (tmp_path / "code/main.c").write_text('#include "values.h"\nint main(void) { return VALUE; }\n')
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1, "build": "recovery/build.json"}))
    (tmp_path / "recovery/build.json").write_text(json.dumps({"schema_version": 1,
        "artifacts": "build/checks", "input_roots": ["code"], "inputs": [], "environment": [],
        "commands": [command("from pathlib import Path; import sys; p=Path(sys.argv[1]); "
                             "assert list(p.iterdir()) == []; (p/'app').write_bytes(b'synthetic output')", "{build}")],
        "outputs": [{"path": "app", "format": "file"}]}))
    git(tmp_path, "init", "--initial-branch=work")
    git(tmp_path, "add", ".")
    git(tmp_path, "commit", "-m", "Synthetic build configuration")
    return tmp_path


def test_native_cli_build_needs_no_source_map_or_project_python(project, capsys):
    assert main(["check-build", "--root", str(project)]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == "pass"
    assert report["fresh_directory"] and not report["persisted"] and not report["ready_for_collection"]
    assert report["outputs"][0]["size"] == len(b"synthetic output")
    assert list((project / "build/checks").iterdir()) == []
    assert git(project, "status", "--porcelain") == ""


def test_real_compiler_detects_header_damage_even_with_existing_output(project):
    cc = shutil.which("cc")
    if cc is None:
        pytest.skip("host C compiler unavailable")
    write_config(project, commands=[{"argv": [cc, "{root}/code/main.c", "-o", "{build}/app"], "timeout": 10}])
    first = check_build(project)
    assert first["result"] == "pass"
    (project / "build/app").write_bytes(b"stale artifact cannot satisfy this check")
    header = project / "code/values.h"
    header.write_text("#error deliberately damaged header\n")
    damaged = check_build(project)
    assert damaged["result"] == "fail" and damaged["outputs"] == []
    assert "deliberately damaged" in damaged["commands"][0]["stderr"]
    assert header.read_text() == "#error deliberately damaged header\n"
    header.write_text("#define VALUE 1\n")
    restored = check_build(project)
    assert restored["result"] == "pass"
    assert first["input_fingerprint"] != restored["input_fingerprint"]
    assert first["outputs"][0]["sha256"] != restored["outputs"][0]["sha256"]


def test_success_without_fresh_artifacts_is_unknown(project):
    (project / "build").mkdir()
    (project / "build/app").write_text("previous output")
    write_config(project, commands=[command("pass", "{build}")])
    report = check_build(project)
    assert report["result"] == "unknown"
    assert report["outputs"][0]["result"] == "unknown"
    assert (project / "build/app").read_text() == "previous output"


def test_real_linker_detects_a_missing_cross_unit_definition(project):
    cc = shutil.which("cc")
    if cc is None:
        pytest.skip("host C compiler unavailable")
    (project / "code/main.c").write_text("extern int helper(void); int main(void) { return helper(); }\n")
    helper = project / "code/helper.c"
    helper.write_text("int unrelated(void) { return 0; }\n")
    write_config(project, commands=[{"argv": [cc, "{root}/code/main.c", "{root}/code/helper.c", "-o", "{build}/app"],
                                     "timeout": 10}])
    report = check_build(project)
    assert report["result"] == "fail"
    assert "helper" in report["commands"][0]["stderr"]
    helper.write_text("int helper(void) { return 0; }\n")
    assert check_build(project)["result"] == "pass"


@pytest.mark.parametrize("kind", ["empty", "symlink", "directory", "invalid_pe"])
def test_invalid_artifacts_are_not_a_success(project, kind):
    action = {"empty": "p.touch()", "symlink": "p.symlink_to(Path(sys.argv[2])/'code/main.c')",
              "directory": "p.mkdir()", "invalid_pe": "p.write_bytes(b'not a PE image')"}[kind]
    write_config(project, commands=[command("from pathlib import Path; import sys; p=Path(sys.argv[1])/'app'; " + action,
                                            "{build}", "{root}")],
                 outputs=[{"path": "app", "format": "pe32-i386" if kind == "invalid_pe" else "file"}])
    report = check_build(project)
    assert report["result"] == "unknown"
    assert list((project / "build/checks").iterdir()) == []


@pytest.mark.parametrize("failure", ["missing", "timeout", "exit"])
def test_later_failure_does_not_inherit_an_earlier_success(project, failure):
    failing = ({"argv": ["missing-build-executable-92ae", "{build}"], "timeout": 1} if failure == "missing" else
               command("import time; time.sleep(5)", timeout=1) if failure == "timeout" else
               command("raise SystemExit(1)"))
    write_config(project, commands=[command("pass", "{build}"), failing,
                                   command("raise AssertionError('must not run')")])
    report = check_build(project)
    assert report["result"] == ("fail" if failure == "exit" else "unknown")
    assert len(report["commands"]) == 2 and not report["outputs"]
    assert list((project / "build/checks").iterdir()) == []


@pytest.mark.parametrize("damage", ["source", "unrelated", "head"])
def test_source_and_git_mutations_are_preserved_and_refused(project, damage):
    script = ("from pathlib import Path; Path('code/values.h').write_text('changed')" if damage == "source" else
              "from pathlib import Path; Path('unrelated.txt').write_text('changed')" if damage == "unrelated" else
              "import subprocess; subprocess.run(['git','add','.'],check=True)")
    # A config change gives the Git-index damage command something to stage.
    write_config(project, commands=[command(script, "{build}")])
    with pytest.raises(ValueError, match="changed"):
        check_build(project)
    assert list((project / "build/checks").iterdir()) == []
    if damage == "source":
        assert (project / "code/values.h").read_text() == "changed"
    elif damage == "unrelated":
        assert (project / "unrelated.txt").read_text() == "changed"


@pytest.mark.parametrize("damage", [
    {"commands": []}, {"commands": [command("pass")]}, {"outputs": []}, {"outputs": [{"path": "../app", "format": "file"}]},
    {"outputs": [{"path": "app", "format": "guess"}]}, {"input_roots": []}, {"inputs": ["../secret"]},
    {"input_roots": ["."]}, {"environment": [[]]}, {"environment": ["BAD=VALUE"]}, {"schema_version": True},
])
def test_invalid_build_configuration_is_refused(project, damage):
    write_config(project, **damage)
    with pytest.raises(ValueError):
        Build.read(project)


@pytest.mark.parametrize("damage", ["unignored", "tracked", "overlap", "input_symlink", "output_symlink"])
def test_unsafe_build_paths_are_refused(project, damage):
    if damage == "unignored":
        (project / ".gitignore").write_text("")
    elif damage == "tracked":
        (project / "build/checks").mkdir(parents=True)
        (project / "build/checks/kept").write_text("user artifact")
        git(project, "add", "-f", "build/checks/kept")
    elif damage == "overlap":
        write_config(project, artifacts="code/build")
    elif damage == "input_symlink":
        (project / "code/alias").symlink_to(project / "code/values.h")
    else:
        (project / "build").symlink_to(project / "code", target_is_directory=True)
    with pytest.raises(ValueError):
        check_build(project)


def test_environment_values_are_fingerprinted_without_disclosing_them(project, monkeypatch):
    write_config(project, environment=["BUILD_TEST_TOKEN"])
    monkeypatch.setenv("BUILD_TEST_TOKEN", "first secret")
    first = Build.read(project).snapshot(project)
    assert "first secret" not in json.dumps(first)
    monkeypatch.setenv("BUILD_TEST_TOKEN", "second secret")
    assert first != Build.read(project).snapshot(project)


def test_explicit_local_file_alias_is_pinned_with_its_target(project):
    alias = project / "recovery/compiler-alias"
    alias.symlink_to("../code/main.c")
    write_config(project, inputs=["recovery/compiler-alias"])
    first = Build.read(project).snapshot(project)
    assert first["configuration"]["links"]["recovery/compiler-alias"] == {
        "link": "../code/main.c", "resolved": "code/main.c"}
    alias.unlink()
    alias.symlink_to("../code/values.h")
    assert first != Build.read(project).snapshot(project)
    alias.unlink()
    alias.symlink_to(sys.executable)
    with pytest.raises(ValueError, match="build input"):
        check_build(project)


def test_summary_keeps_command_failures_and_artifact_status(project, capsys):
    assert main(["check-build", "--root", str(project), "--summary"]) == 0
    output = json.loads(capsys.readouterr().out)
    assert output["commands"][0]["exit_code"] == 0
    assert "stdout" not in output["commands"][0] and "stderr" not in output["commands"][0]
