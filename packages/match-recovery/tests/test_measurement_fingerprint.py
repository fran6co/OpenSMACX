"""Snapshot path optimization must retain content and namespace coverage."""

import hashlib
import builtins
import io
import json
import os

import pytest

from match_recovery.fingerprint import input_snapshot, _content_digest
from match_recovery.measurement import Measurement
from test_symbol_check import project


def test_nested_unicode_names_binary_content_and_missing_inputs_keep_exact_identity(tmp_path):
    path = tmp_path / "owner with spaces" / "déclarations.h"
    path.parent.mkdir()
    path.write_bytes(b"\xff\x00\r\n")
    name = path.relative_to(tmp_path).as_posix()
    report = input_snapshot(tmp_path, [name, "missing.h", name], {"flags": ["-types"]})
    assert report["inputs"] == {name: hashlib.sha256(path.read_bytes()).hexdigest(), "missing.h": None}
    assert list(report["inputs"]) == sorted(report["inputs"])
    assert report["configuration"] == {"flags": ["-types"]}


@pytest.mark.parametrize("kind", ["file", "directory", "dangling"])
def test_symlink_escape_to_same_prefix_sibling_is_refused_before_reading(tmp_path, monkeypatch, kind):
    root, sibling = tmp_path / "project", tmp_path / "project-other"
    root.mkdir()
    sibling.mkdir()
    (sibling / "outside.h").write_text("outside")
    if kind == "directory":
        (root / "escape").symlink_to(sibling, target_is_directory=True)
        name = "escape/outside.h"
    else:
        (root / "escape.h").symlink_to(sibling / ("outside.h" if kind == "file" else "missing.h"))
        name = "escape.h"
    def forbidden(*args, **kwargs):
        pytest.fail("an escaping input must not be opened")
    monkeypatch.setattr(type(root), "open", forbidden)
    monkeypatch.setattr(builtins, "open", forbidden)
    with pytest.raises(ValueError, match="outside the worktree"):
        input_snapshot(root, [name], {})


def test_internal_symlinks_and_an_aliased_root_keep_source_names(tmp_path):
    root = tmp_path / "project"
    root.mkdir()
    (root / "environment").mkdir()
    (root / "environment/compiler.py").write_bytes(b"compiler script")
    (root / "environment/compiler").symlink_to("compiler.py")
    (root / "alias").symlink_to("environment", target_is_directory=True)
    outer = tmp_path / "project-alias"
    outer.symlink_to(root, target_is_directory=True)
    names = ["environment/compiler", "alias/compiler.py"]
    expected = {name: hashlib.sha256(b"compiler script").hexdigest() for name in names}
    assert input_snapshot(root, names, {})["inputs"] == expected
    assert input_snapshot(outer, names, {})["inputs"] == expected


def test_reused_buffer_never_hashes_bytes_left_by_a_previous_input(tmp_path):
    contents = {"a.bin": b"large\x00" * 100_003, "b.bin": b"short", "c.bin": b"",
                "d.bin": b"boundary" * 32_768, "e.bin": b"last"}
    for name, raw in contents.items():
        (tmp_path / name).write_bytes(raw)
    expected = {name: hashlib.sha256(raw).hexdigest() for name, raw in contents.items()}
    assert input_snapshot(tmp_path, contents, {})["inputs"] == expected


def test_short_reads_are_not_mistaken_for_end_of_input():
    raw = b"short reads followed by more content\x00\xff"

    class ShortReads(io.BytesIO):
        def readinto(self, buffer):
            return super().readinto(memoryview(buffer)[:3])

    assert _content_digest(ShortReads(raw), bytearray(16)) == hashlib.sha256(raw).hexdigest()


def test_would_block_is_not_a_successful_partial_digest():
    class WouldBlock:
        def readinto(self, buffer):
            return None

    with pytest.raises(BlockingIOError, match="would block"):
        _content_digest(WouldBlock(), bytearray(16))


def test_directory_link_is_resolved_again_for_each_input(tmp_path, monkeypatch):
    root, outside = tmp_path / "project", tmp_path / "project-other"
    root.mkdir()
    outside.mkdir()
    inside = root / "inside"
    inside.mkdir()
    (inside / "a.h").write_bytes(b"first")
    (outside / "b.h").write_bytes(b"must not be read")
    link = root / "alias"
    link.symlink_to(inside, target_is_directory=True)
    original_open = builtins.open
    opened = []

    def changing_open(path, *args, **kwargs):
        opened.append(os.fspath(path))
        assert os.fspath(path) == os.fspath(root / "alias/a.h")
        stream = original_open(path, *args, **kwargs)
        link.unlink()
        link.symlink_to(outside, target_is_directory=True)
        return stream

    monkeypatch.setattr(builtins, "open", changing_open)
    with pytest.raises(ValueError, match="outside the worktree"):
        input_snapshot(root, ["alias/a.h", "alias/b.h"], {})
    assert len(opened) == 1


def test_read_errors_are_not_missing_inputs(tmp_path, monkeypatch):
    (tmp_path / "input.h").write_bytes(b"unreadable")

    def denied(*args, **kwargs):
        raise PermissionError("input read denied")

    monkeypatch.setattr(builtins, "open", denied)
    with pytest.raises(PermissionError, match="denied"):
        input_snapshot(tmp_path, ["input.h"], {})


def test_symlink_targets_are_resolved_again_on_every_snapshot(tmp_path):
    root, sibling = tmp_path / "project", tmp_path / "project-other"
    root.mkdir()
    sibling.mkdir()
    (root / "first.h").write_bytes(b"first")
    (root / "other.h").write_bytes(b"other")
    link = root / "input.h"
    link.symlink_to("first.h")
    first = input_snapshot(root, ["input.h"], {})
    link.unlink()
    link.symlink_to("other.h")
    assert input_snapshot(root, ["input.h"], {}) != first
    link.unlink()
    link.symlink_to(sibling / "missing.h")
    with pytest.raises(ValueError, match="outside the worktree"):
        input_snapshot(root, ["input.h"], {})


@pytest.mark.parametrize("name", ["a//b", "a/./b", "a/../b", "a/", "../project-other/outside.h", "a\\b"])
def test_noncanonical_input_names_are_not_normalized_into_acceptance(tmp_path, name):
    with pytest.raises(ValueError, match="normalized relative input path"):
        input_snapshot(tmp_path, [name], {})


def test_added_renamed_and_deleted_include_files_are_reenumerated(project):
    settings = Measurement.read(project)
    before = settings.snapshot(project, {})
    directory = project / "code/detail with spaces"
    directory.mkdir()
    header = directory / "member.inc"
    header.write_bytes(b"int field;")
    added = settings.snapshot(project, {})
    assert added != before and "code/detail with spaces/member.inc" in added["inputs"]
    header.rename(directory / "renamed.inc")
    renamed = settings.snapshot(project, {})
    assert renamed != added and "code/detail with spaces/member.inc" not in renamed["inputs"]
    (directory / "renamed.inc").unlink()
    assert settings.snapshot(project, {}) == before


def test_hidden_and_non_cpp_include_inputs_are_still_enumerated(project):
    directory = project / "code/.generated"
    directory.mkdir()
    (directory / "compiler-input").write_bytes(b"hidden include")
    report = Measurement.read(project).snapshot(project, {})
    assert report["inputs"]["code/.generated/compiler-input"] == hashlib.sha256(b"hidden include").hexdigest()


def test_enumerated_file_aliases_are_hashed_and_external_retargets_refused(project, tmp_path_factory):
    target = project / "code/actual.h"
    target.write_bytes(b"original include")
    alias = project / "code/alias.h"
    alias.symlink_to("actual.h")
    settings = Measurement.read(project)
    report = settings.snapshot(project, {})
    assert report["inputs"]["code/alias.h"] == report["inputs"]["code/actual.h"]
    outside = tmp_path_factory.mktemp("outside") / "other.h"
    outside.write_bytes(b"outside include")
    alias.unlink()
    alias.symlink_to(outside)
    with pytest.raises(ValueError, match="outside the worktree"):
        settings.snapshot(project, {})


def test_same_size_and_timestamp_edits_are_rehashed_in_measurement(project):
    path = project / "code/member.h"
    path.write_bytes(b"int first;")
    settings = Measurement.read(project)
    original_stat = path.stat()
    before = settings.snapshot(project, {})
    path.write_bytes(b"int other;")
    os.utime(path, ns=(original_stat.st_atime_ns, original_stat.st_mtime_ns))
    after = settings.snapshot(project, {})
    assert path.stat().st_size == original_stat.st_size
    assert after["inputs"]["code/member.h"] == hashlib.sha256(b"int other;").hexdigest()
    assert after != before


def test_declared_build_input_alias_is_fingerprinted_under_its_configured_name(project):
    directory = project / "build-inputs"
    directory.mkdir()
    (directory / "policy.inc").write_bytes(b"policy")
    (project / "policy-alias").symlink_to(directory, target_is_directory=True)
    path = project / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["input_roots"] = ["policy-alias"]
    path.write_text(json.dumps(config))
    report = Measurement.read(project).snapshot(project, {})
    assert report["inputs"]["policy-alias/policy.inc"] == hashlib.sha256(b"policy").hexdigest()
    assert "build-inputs/policy.inc" not in report["inputs"]


def test_new_or_changed_checker_code_invalidates_implementation_identity(project, monkeypatch, tmp_path_factory):
    import match_recovery.measurement as module
    package = tmp_path_factory.mktemp("engine")
    path = package / "nested" / "parser.py"
    path.parent.mkdir()
    path.write_bytes(b"old")
    monkeypatch.setattr(module, "__file__", str(package / "measurement.py"))
    settings = Measurement.read(project)
    before = settings.snapshot(project, {})
    saved = path.stat()
    path.write_bytes(b"new")
    os.utime(path, ns=(saved.st_atime_ns, saved.st_mtime_ns))
    changed = settings.snapshot(project, {})
    assert changed["inputs"] == before["inputs"]
    assert changed["configuration"]["implementation"] != before["configuration"]["implementation"]
    path.rename(path.with_name("other.py"))
    assert settings.snapshot(project, {}) != changed
