import json
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.cli import main, parser
from match_recovery.project import initialize


def test_new_target_needs_no_library_registration_and_never_overwrites(tmp_path, capsys):
    assert main(["init", "--root", str(tmp_path), "--target", "never-seen-before"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert not report["ready_for_recovery"]
    config = json.loads((tmp_path / "recovery/driver.json").read_text())
    assert config["target"] == "never-seen-before"
    adapter = tmp_path / "recovery/adapter.py"
    adapter.write_text("user-owned adapter\n")
    with pytest.raises(ValueError, match="overwrite"):
        initialize(tmp_path, "other", sys.executable)
    assert adapter.read_text() == "user-owned adapter\n"
    assert json.loads((tmp_path / "recovery/driver.json").read_text()) == config


def test_starter_check_is_unknown_and_context_consumes_json(tmp_path):
    initialize(tmp_path, "unregistered", sys.executable)
    command = [sys.executable, "recovery/adapter.py"]
    request = json.dumps({"task": {"editable_files": ["code/owner.cc"]}})
    check = subprocess.run([*command, "check"], cwd=tmp_path, input=request, capture_output=True, text=True)
    assert check.returncode == 3 and json.loads(check.stdout)["result"] == "unknown"
    context = subprocess.run([*command, "context"], cwd=tmp_path, input=request, capture_output=True, text=True)
    assert context.returncode == 0 and json.loads(context.stdout)["source_homes"] == ["code/owner.cc"]


def test_public_source_commands_use_only_local_json_configuration(tmp_path, capsys):
    initialize(tmp_path, "unregistered", sys.executable)
    assert main(["inventory", "--root", str(tmp_path)]) == 3
    assert json.loads(capsys.readouterr().out)["result"] == "unknown"
    # Replacing a local adapter needs no imports, registry or change to recover.
    (tmp_path / "recovery/adapter.py").write_text(
        'import json, sys\nr = json.load(sys.stdin)\n'
        'assert r["arguments"]["command"] == "inventory"\n'
        'print(json.dumps({"output": {"local_source_inventory": True}}))\n')
    assert main(["inventory", "--root", str(tmp_path)]) == 0
    assert json.loads(capsys.readouterr().out) == {"local_source_inventory": True}


def test_setup_does_not_follow_a_recovery_directory_symlink(tmp_path):
    outside = tmp_path / "outside"
    outside.mkdir()
    root = tmp_path / "project"
    root.mkdir()
    (root / "recovery").symlink_to(outside, target_is_directory=True)
    with pytest.raises(ValueError, match="inside"):
        initialize(root, "fixture", sys.executable)
    assert list(outside.iterdir()) == []


def test_public_cli_exposes_workflow_and_source_commands():
    help_text = parser().format_help()
    assert "init" in help_text and "run" in help_text
    assert "inventory" in help_text and "partition" in help_text


def test_base_api_and_cli_run_without_site_packages(tmp_path):
    """Isolation tests dependencies, not the spelling of comments or modules."""
    import match_recovery
    package_root = Path(match_recovery.__file__).resolve().parent.parent
    script = '''import sys
sys.path.insert(0, sys.argv[1])
from pathlib import Path
from match_recovery.source import read_text, write
from match_recovery.codeview import class_layout
from match_recovery.generated_binary import check_generated
from match_recovery.cli import main
text = "// ORIGINAL: 0x00401000 ?f@@YAXXZ 0x00401000-0x00401001\\nvoid f() {}\\n"
records = read_text(text, Path("unit.cc"))
assert len(records) == 1 and records[0].address == 0x401000
assert write(text, records) == text
raise SystemExit(main(["init", "--target", "isolated-target", "--root", sys.argv[2]]))
'''
    result = subprocess.run([sys.executable, "-I", "-S", "-c", script, str(package_root), str(tmp_path)],
                            cwd=tmp_path, capture_output=True, text=True, timeout=30)
    assert result.returncode == 0, result.stderr
    assert json.loads(result.stdout)["ready_for_recovery"] is False


LOCAL_ADAPTER = '''import json, sys
from pathlib import Path
request = json.load(sys.stdin)
source = Path("code/owner.cc")
action = sys.argv[1]
if action == "context":
    print(json.dumps({"source": str(source)}))
elif action == "worker":
    assert request["editable_files"] == [str(source)]
    source.write_text("finished synthetic fixture\\n")
    print(json.dumps({"discoveries": []}))
elif action == "reviewer":
    assert source.read_text() == "finished synthetic fixture\\n"
    print(json.dumps({"snapshot": request["snapshot"], "decision": "approve",
                      "evidence": ["synthetic fixture reviewed"], "feedback": "fixture agrees"}))
else:
    passed = source.read_text() == "finished synthetic fixture\\n"
    result = "pass" if passed else "fail"
    report = {"result": result}
    if action == "acceptance":
        report.update(scope="full_project", **{key: result for key in
                      ("build", "structure", "references", "obligations", "runtime")})
    print(json.dumps(report))
    raise SystemExit(0 if passed else 1)
'''


def test_external_project_commands_drive_recovery_without_python_integration(tmp_path, capsys):
    # These synthetic local commands import only stdlib and run as real child
    # processes. They stand in for a new project's trusted measurement tools.
    root = tmp_path / "standalone"
    root.mkdir()
    initialize(root, "independent-target", sys.executable)
    (root / "recovery/adapter.py").write_text(LOCAL_ADAPTER)
    (root / "code").mkdir()
    (root / "code/owner.cc").write_text("unfinished\n")
    (root / ".gitignore").write_text("build/\n")
    command = lambda name: {"argv": [sys.executable, "recovery/adapter.py", name], "timeout": 10}
    (root / "recovery/driver.json").write_text(json.dumps({"schema_version": 1, "target": "independent-target",
        "context": command("context"), "check": command("check"), "acceptance": command("acceptance")}))
    for role in ("worker", "reviewer"):
        (root / "recovery" / (role + ".json")).write_text(json.dumps({"author": role, "command": command(role)}))
    def git(*args):
        return subprocess.run(["git", *args], cwd=root, capture_output=True, text=True, check=True).stdout.strip()
    git("init")
    git("config", "user.name", "Synthetic")
    git("config", "user.email", "fixture@example.invalid")
    git("add", ".")
    git("commit", "-m", "synthetic baseline")
    decision = {"schema_version": 1, "task_id": "owner", "target": "independent-target", "author": "coordinator",
                "intent": "Test local adapter protocol", "base_revision": git("rev-parse", "HEAD"),
                "subjects": [{"image_sha256": "a" * 64, "address_space": "custom:offset", "address": 16}],
                "editable_files": ["code/owner.cc"], "evidence": ["synthetic local mapping"]}
    task_file = tmp_path / "decision.json"
    task_file.write_text(json.dumps(decision))
    assert main(["tasks", "create", "--root", str(root), "--file", str(task_file)]) == 0
    capsys.readouterr()
    git("add", ".")
    git("commit", "-m", "task decision")
    # Invoke from outside the project: default configs resolve under --root.
    assert main(["run", "owner", "--root", str(root), "--reviewer", "recovery/reviewer.json"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["ready_for_collection"] and report["worker_rounds"] == 1
    assert report["acceptance"]["scope"] == "full_project"
