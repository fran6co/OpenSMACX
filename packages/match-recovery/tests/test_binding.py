"""New identities are source decisions, not executable/matching proof."""

import hashlib
import json
import os
import subprocess
import sys

import pytest

from match_recovery.binding import bind
from match_recovery.binding_source import parse, render
from match_recovery.cli import main
from match_recovery.model import Span
from match_recovery.project import initialize
from match_recovery.source import read_text, write
from match_recovery.source_integrity import check_annotation_text
from match_recovery.source_inventory import source_snapshot
from test_binary import pe_fixture
from test_image_survey import executable


@pytest.fixture
def project(tmp_path):
    image = tmp_path / "original.exe"
    image.write_bytes(executable(pe_fixture(b"\xc3\xc3")))
    initialize(tmp_path, "binding-control", sys.executable, image=image)
    (tmp_path / "src/f.cpp").write_text("int f() { return 1; }\n")
    (tmp_path / ".gitignore").write_text("build/\noriginal.exe\n")
    for args in (("init",), ("config", "user.name", "Binding fixture"), ("config", "user.email", "fixture@example.invalid"),
                 ("add", "."), ("commit", "-m", "source and original identity")):
        subprocess.run(["git", *args], cwd=tmp_path, check=True, capture_output=True)
    return tmp_path


def parameters(root):
    return {"image": root / "original.exe", "address": 0x401000, "name": "_f", "spans": [Span(0x401000, 0x401001)],
            "source": "src/f.cpp", "line": 1, "source_sha256": hashlib.sha256((root / "src/f.cpp").read_bytes()).hexdigest(),
            "reason": "Controlled fixture has an independently known one-byte reference body.",
            "evidence": ["test-owned original fixture", "source:src/f.cpp"]}


def test_binding_changes_only_comments_and_reaches_source_inventory(project):
    path = project / "src/f.cpp"
    before = path.read_bytes()
    report = bind(project, **parameters(project))
    text = path.read_text()
    assert text.splitlines(keepends=True)[2:] == before.decode().splitlines(keepends=True)
    assert report["binding_added"] and report["verification"] == report["boundary_verification"] == "not_measured"
    assert not report["ready_for_collection"]
    record, = read_text(text, path)
    assert record.name == "_f" and not record.byte_exact and not record.semantic
    assert parse(text, [record])[1]["evidence"] == parameters(project)["evidence"]
    assert write(text, [record]) == text
    assert not check_annotation_text(path, text)["findings"]
    row, = source_snapshot(project)["records"]
    assert row["binding_evidence"]["reason"] == parameters(project)["reason"]


@pytest.mark.parametrize("damage", ["stale", "duplicate", "overlap", "nonexec", "bad_name", "claim_name", "bad_line", "bad_span"])
def test_refusals_do_not_change_source(project, damage):
    args = parameters(project)
    if damage == "stale":
        args["source_sha256"] = "a" * 64
    elif damage == "duplicate":
        bind(project, **args)
        args = parameters(project)
        args["line"] = 3
    elif damage == "overlap":
        (project / "src/other.cpp").write_text("// ORIGINAL: 0x00400FFF _other 0x00400FFF-0x00401001\nvoid other() {}\n")
    elif damage == "nonexec":
        image = project / "original.exe"
        image.write_bytes(executable(pe_fixture(), executable=False))
        (project / "src/IMAGE_PIN").write_text("sha256 " + hashlib.sha256(image.read_bytes()).hexdigest() + "\n")
    elif damage == "bad_name":
        args["name"] = "_f\n// invented marker"
    elif damage == "claim_name":
        args["name"] = "BYTE_EXACT"
    elif damage == "bad_line":
        args["line"] = 2
    else:
        args["spans"] = [Span(0x401001, 0x401002)]
    before = {path: path.read_bytes() for path in (project / "src").rglob("*") if path.is_file()}
    with pytest.raises(ValueError):
        bind(project, **args)
    assert before == {path: path.read_bytes() for path in (project / "src").rglob("*") if path.is_file()}


def test_exact_seed_identity_can_seed_a_new_product_without_erasing_its_evidence(project):
    from match_recovery.mapping_check import check_mappings
    config_path = project / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["source"]["seed_roots"] = ["src/seeds"]
    config["mappings"] = {"compile_commands": "build/compilation.json", "forwarder_macros": []}
    config_path.write_text(json.dumps(config))
    (project / "build").mkdir(exist_ok=True)
    (project / "build/compilation.json").write_text(json.dumps([
        {"directory": str(project), "file": "src/f.cpp", "arguments": ["compiler-not-run", "src/f.cpp"]}]))
    (project / "src/seeds").mkdir()
    seed = project / "src/seeds/f.cpp"
    text = "// ORIGINAL: 0x00401000 _old_hint 0x00401000-0x00401001\nint old_hint() { return 0; }\n"
    seed.write_text(text)
    bind(project, **parameters(project))
    assert seed.read_text() == text
    assert len(source_snapshot(project)["records"]) == 2
    mappings = check_mappings(project)
    assert mappings["result"] == "pass" and not mappings["ready_for_collection"]
    assert mappings["seed_alternatives"][0]["product_owner"]["path"] == "src/f.cpp"


def test_neighboring_claim_and_lessons_survive_insertion(project):
    path = project / "src/f.cpp"
    # Test fixture only: the production binder has no claim-setting interface.
    existing = "// ORIGINAL: 0x00401001 _g 0x00401001-0x00401002 BYTE_EXACT\n// LEVER: fixture Existing lesson.\nint g() { return 2; }\n"
    path.write_text("int f() { return 1; }\n\n" + existing)
    bind(project, **parameters(project))
    assert path.read_text().endswith(existing)
    first, second = read_text(path.read_text(), path)
    assert not first.byte_exact and second.byte_exact and second.levers == (("fixture", "Existing lesson."),)


@pytest.mark.parametrize("damage", ["source", "image", "configuration"])
def test_publication_rechecks_relevant_binding_inputs(project, monkeypatch, damage):
    import match_recovery.binding as module
    real_publish = module.publish
    path = project / "src/f.cpp"
    before = path.read_bytes()
    def raced(*args, **kwargs):
        target = path if damage == "source" else project / ("original.exe" if damage == "image" else "recovery/project.json")
        target.write_bytes(target.read_bytes() + b"\n")
        return real_publish(*args, **kwargs)
    monkeypatch.setattr(module, "publish", raced)
    # Configuration whitespace alone is semantically equivalent, but the source
    # snapshot has no reason to refuse it; use an actual source selection change.
    if damage == "configuration":
        def changed_config(*args, **kwargs):
            target = project / "recovery/project.json"
            value = json.loads(target.read_text())
            value["source"]["symbol_format"] = "msvc"
            target.write_text(json.dumps(value))
            return real_publish(*args, **kwargs)
        monkeypatch.setattr(module, "publish", changed_config)
    with pytest.raises(ValueError, match="changed"):
        bind(project, **parameters(project))
    assert path.read_bytes() == before + (b"\n" if damage == "source" else b"")


@pytest.mark.parametrize("link", ["symlink", "hardlink"])
def test_linked_output_is_refused(project, link):
    path = project / "src/f.cpp"
    alias = project / "src/alias.cpp"
    if link == "symlink":
        alias.symlink_to(path)
        args = {**parameters(project), "source": "src/alias.cpp"}
    else:
        os.link(path, alias)
        args = parameters(project)
    before = path.read_bytes()
    with pytest.raises(ValueError):
        bind(project, **args)
    assert path.read_bytes() == alias.read_bytes() == before


def test_cli_binding_then_native_task_creation_and_context(project, capsys):
    args = parameters(project)
    assert main(["bind", "0x401000", "--root", str(project), "--image", "original.exe", "--in", "src/f.cpp",
                 "--line", "1", "--name", "_f", "--span", "0x401000-0x401001", "--source-sha256", args["source_sha256"],
                 "--reason", args["reason"], "--evidence", "source:src/f.cpp"]) == 0
    assert json.loads(capsys.readouterr().out)["binding_added"]
    for command in (("add", "src/f.cpp"), ("commit", "-m", "establish original binding")):
        subprocess.run(["git", *command], cwd=project, check=True, capture_output=True)
    assert main(["tasks", "create", "owner", "--root", str(project), "--group", "source:src/f.cpp",
                 "--author", "coordinator", "--intent", "Recover real function", "--evidence", "source:src/f.cpp"]) == 0
    capsys.readouterr()
    assert main(["tasks", "context", "owner", "--root", str(project)]) == 0
    context = json.loads(capsys.readouterr().out)
    assert context["binding_evidence"][0]["evidence"] == ["source:src/f.cpp"]
    assert context["binding_evidence"][0]["reason"] == args["reason"]


@pytest.mark.parametrize("metadata", ["// RECOVERY_BINDING: v2 {}", "// RECOVERY_BINDING: v1 {}",
                                       render("example", ["source:src/f.cpp"])])
def test_malformed_or_orphaned_binding_metadata_fails_source_integrity(project, metadata):
    text = metadata + "\nint f() { return 1; }\n"
    report = check_annotation_text(project / "src/f.cpp", text)
    assert any(row["rule"] == "binding_evidence" for row in report["findings"])
