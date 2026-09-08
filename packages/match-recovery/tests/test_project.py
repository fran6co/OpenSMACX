import hashlib
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
    assert config["acceptance_scope"] == "change"
    assert "check-acceptance" in config["acceptance"]["argv"]
    project = json.loads((tmp_path / "recovery/project.json").read_text())
    assert project["mappings"] == {"compile_commands": "build/compile_commands.json", "forwarder_macros": []}
    config_file = tmp_path / "recovery/build.json"
    config_file.write_text("user-owned build policy\n")
    with pytest.raises(ValueError, match="overwrite"):
        initialize(tmp_path, "other", sys.executable)
    assert config_file.read_text() == "user-owned build policy\n"
    assert json.loads((tmp_path / "recovery/driver.json").read_text()) == config


def test_unconfigured_native_starter_cannot_claim_success(tmp_path, capsys, monkeypatch):
    from match_recovery.build_check import Build
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement

    initialize(tmp_path, "unregistered", sys.executable)
    assert main(["inventory", "--root", str(tmp_path)]) == 3
    assert "IMAGE_PIN" in capsys.readouterr().err
    assert Build.read(tmp_path).outputs == (("application.exe", "pe32-i386"),)
    assert Measurement.read(tmp_path).flag_sets == ((),)  # No guessed compiler flags.
    monkeypatch.delenv("RECOVERY_COMPILER", raising=False)
    unit = tmp_path / "src/f.cpp"
    unit.write_text("void f() {}\n")
    with pytest.raises(ValueError, match="RECOVERY_COMPILER"):
        Compiler(tmp_path)(unit, (), ())


def test_initializer_pins_only_identity_and_native_inventory_reads_existing_source(tmp_path, capsys):
    (tmp_path / "code").mkdir()
    unit = tmp_path / "code/owner.cc"
    text = "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\nvoid f() {}\n"
    unit.write_text(text)
    original = tmp_path / "original.bin"
    original.write_bytes(b"our identity-only test input; not a PE fixture")
    assert main(["init", "--root", str(tmp_path), "--target", "unregistered", "--source-root", "code",
                 "--image", "original.bin"]) == 0
    setup = json.loads(capsys.readouterr().out)
    pin = hashlib.sha256(original.read_bytes()).hexdigest()
    assert setup["image_sha256"] == pin and not setup["ready_for_recovery"]
    assert (tmp_path / "code/IMAGE_PIN").read_text().splitlines()[-1] == "sha256 " + pin
    assert unit.read_text() == text
    assert main(["inventory", "--root", str(tmp_path), "--image", str(original)]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["image_sha256"] == pin and report["image_verified"]
    assert report["entities"] == report["annotations"] == 1 and report["verification"] == "not_measured"
    original.write_bytes(b"different input")
    assert main(["inventory", "--root", str(tmp_path), "--image", str(original)]) == 3
    assert "digest mismatch" in capsys.readouterr().err


def test_generated_context_and_obligations_commands_work_without_project_code(tmp_path):
    from match_recovery.driver import Command, invoke
    from match_recovery.model import Origin
    from match_recovery.tasks import Task, TaskStore

    original = tmp_path / "original.bin"
    original.write_bytes(b"source context identity, not a binary proof")
    report = initialize(tmp_path, "native-target", sys.executable, image=original, source_root="code")
    (tmp_path / "code/owner.cc").write_text(
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\nvoid f() {}\n")
    (tmp_path / ".gitignore").write_text("build/\noriginal.bin\n")
    def git(*args):
        return subprocess.run(["git", *args], cwd=tmp_path, check=True, capture_output=True, text=True).stdout.strip()
    git("init")
    git("config", "user.name", "Native starter fixture")
    git("config", "user.email", "fixture@example.invalid")
    git("add", ".")
    git("commit", "-m", "source-owned seed and native configuration")
    task = Task("owner", "native-target", "coordinator", "Recover ownership", git("rev-parse", "HEAD"),
                (Origin(report["image_sha256"], "pe:va", 0x401000),), ("code/owner.cc",), ("source:code/owner.cc",))
    TaskStore(tmp_path).create(task)
    git("add", "recovery")
    git("commit", "-m", "authorize owner task")
    config = json.loads((tmp_path / "recovery/driver.json").read_text())
    code, context = invoke(Command.read(config["context"]), tmp_path, "owner", {"task": task.to_dict()})
    assert code == 0 and Task.from_dict(context["task"]) == task
    assert len(context["subjects"]) == 1 and not context["persisted"]
    code, obligations = invoke(Command.read(config["obligations"]), tmp_path, "owner", {"task": task.to_dict()})
    assert code == 0 and context["restoration"] == obligations
    assert not git("status", "--porcelain")


def test_obsolete_dispatch_configuration_is_refused_not_executed(tmp_path, capsys):
    initialize(tmp_path, "native-target", sys.executable)
    path = tmp_path / "recovery/project.json"
    config = json.loads(path.read_text())
    config["adapter"] = {"argv": [sys.executable, "-c",
                                  "from pathlib import Path; Path('ran-obsolete-dispatch').touch()"], "timeout": 10}
    path.write_text(json.dumps(config))
    assert main(["inventory", "--root", str(tmp_path)]) == 3
    assert "invalid local project command configuration" in capsys.readouterr().err
    assert not (tmp_path / "ran-obsolete-dispatch").exists()


def test_new_empty_inventory_is_a_census_not_permission_for_empty_recovery(tmp_path, capsys):
    from match_recovery.source_inventory import source_snapshot
    original = tmp_path / "original.bin"
    original.write_bytes(b"identity-only input")
    initialize(tmp_path, "empty-target", sys.executable, image=original)
    assert main(["inventory", "--root", str(tmp_path), "--image", str(original)]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["entities"] == report["annotations"] == 0
    assert report["image_verified"] and report["verification"] == "not_measured"
    with pytest.raises(ValueError, match="empty migration"):
        source_snapshot(tmp_path)
    assert main(["partition", "--root", str(tmp_path)]) == 3
    assert "empty migration" in capsys.readouterr().err


@pytest.mark.parametrize("source", ["../outside", "recovery", "recovery/src", "build/code", ".git/src"])
def test_initializer_refuses_unsafe_source_roots_before_writing(tmp_path, source):
    with pytest.raises(ValueError):
        initialize(tmp_path, "fixture", sys.executable, source_root=source)
    assert list(tmp_path.iterdir()) == []


@pytest.mark.parametrize("damage", ["missing_image", "existing_pin", "symlink_source", "file_parent"])
def test_initializer_input_and_path_refusals_leave_existing_files_untouched(tmp_path, damage):
    image = tmp_path / "original.bin"
    image.write_bytes(b"identity")
    source = tmp_path / "code"
    source.mkdir()
    if damage == "missing_image":
        image = tmp_path / "missing.bin"
    elif damage == "existing_pin":
        (source / "IMAGE_PIN").write_text("user-owned pin\n")
    elif damage == "symlink_source":
        (tmp_path / "alias").symlink_to(source, target_is_directory=True)
        source = tmp_path / "alias"
    else:
        (source / "nested").write_text("not a directory")
        source = source / "nested" / "src"
    before = {str(path): path.read_bytes() for path in tmp_path.rglob("*") if path.is_file()}
    with pytest.raises((ValueError, OSError)):
        initialize(tmp_path, "fixture", sys.executable, image=image, source_root=source.relative_to(tmp_path).as_posix())
    assert not (tmp_path / "recovery").exists()
    assert before == {str(path): path.read_bytes() for path in tmp_path.rglob("*") if path.is_file()}


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


LOCAL_ADAPTER = '''import hashlib, json, sys
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
        serialized = json.dumps(request, sort_keys=True, ensure_ascii=False, separators=(",", ":")) + "\\n"
        report.update(request_fingerprint=hashlib.sha256(serialized.encode()).hexdigest(),
                      measurements=[], current={})
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
