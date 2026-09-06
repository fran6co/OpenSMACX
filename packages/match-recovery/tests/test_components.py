from dataclasses import replace
import hashlib
import io
import json
import tarfile

import pytest

from match_recovery.cli import main
from match_recovery.components import (SourceFile, VendorBinding, VendorManifest,
                                       local_path, relative_path, source_integrity)
from match_recovery.component_source import parse_component, render_component, write_component, read_components
from match_recovery.model import Component, Delivery, Origin, Ownership, Result, Span, canonical
from match_recovery.records import publish_json, read_json


@pytest.fixture
def vendor(tmp_path):
    source = tmp_path / "vendor"
    source.mkdir()
    contents = {"codec.c": b"int codec(int x) { return x + 1; }\n",
                "codec.h": b"/* Synthetic test fixture license notice */\nint codec(int);\n"}
    archive = tmp_path / "upstream.tar.gz"
    with tarfile.open(archive, "w:gz") as bundle:
        for name, raw in contents.items():
            (source / name).write_bytes(raw)
            info = tarfile.TarInfo("release/" + name)
            info.size = len(raw)
            bundle.addfile(info, io.BytesIO(raw))
    pins = tuple(SourceFile(name, "release/" + name) for name in contents)
    component = Component("codec", Ownership.THIRD_PARTY, Delivery.UPSTREAM_SOURCE,
                          ("synthetic-release",), "1.0")
    binding = VendorBinding(Origin("a" * 64, "overlay:1", 4096), (Span(4096, 4104),),
                            "codec", "codec.c", "historical-test-observation")
    manifest = VendorManifest(component, "vendor", "https://invalid.test/fixture",
                              hashlib.sha256(archive.read_bytes()).hexdigest(), "codec.h", pins,
                              "c", ("codec.c",), ("-O2",), (binding,))
    return tmp_path, manifest, archive


def test_manifest_roundtrip_is_portable_and_source_checks_are_not_binary_claims(vendor):
    root, manifest, archive = vendor
    assert VendorManifest.from_dict(json.loads(canonical(manifest.to_dict()))) == manifest
    report = source_integrity(root, manifest, archive)
    assert report["result"] == Result.PASS
    assert report["archive"]["result"] == Result.PASS
    assert report["binary_verification"] == Result.UNKNOWN
    assert report["application_recoveries"] == 0


def test_changed_vendor_source_is_rejected(vendor):
    root, manifest, archive = vendor
    (root / "vendor" / "codec.c").write_text("int codec(int x) { return x + 2; }\n")
    assert source_integrity(root, manifest, archive)["result"] == Result.FAIL


def test_per_file_hashes_are_not_persisted_and_missing_archive_is_not_green(vendor):
    root, manifest, _ = vendor
    assert all(set(item) == {"path", "archive_member"} for item in manifest.to_dict()["files"])
    assert source_integrity(root, manifest)["result"] == Result.UNKNOWN


def test_wrong_archive_release_is_rejected(vendor):
    root, manifest, archive = vendor
    archive.write_bytes(b"different release")
    assert source_integrity(root, manifest, archive)["result"] == Result.FAIL


def test_missing_source_is_unknown_not_green(vendor):
    root, manifest, _ = vendor
    (root / "vendor" / "codec.c").unlink()
    assert source_integrity(root, manifest)["result"] == Result.UNKNOWN


@pytest.mark.parametrize("name", ["../escape", "/absolute", "a/../b", "a\\b", "C:/other", ".", "a//b"])
def test_vendor_paths_cannot_escape_or_alias(name):
    with pytest.raises(ValueError):
        relative_path(name)


def test_vendor_symlinks_cannot_reach_another_worktree(vendor, tmp_path):
    root, _, _ = vendor
    (root / "vendor" / "linked").symlink_to(tmp_path.parent)
    with pytest.raises(ValueError, match="outside"):
        local_path(root / "vendor", "linked")


def test_duplicate_binding_cannot_double_count_coverage(vendor):
    _, manifest, _ = vendor
    with pytest.raises(ValueError, match="duplicate original"):
        replace(manifest, bindings=manifest.bindings * 2)


def test_application_wrappers_cannot_be_hidden_as_vendor_source(vendor):
    _, manifest, _ = vendor
    with pytest.raises(ValueError, match="application code"):
        replace(manifest, component=replace(manifest.component, ownership=Ownership.APPLICATION))


def test_manifest_rejects_unknown_fields_and_boolean_schema(vendor):
    _, manifest, _ = vendor
    data = json.loads(canonical(manifest.to_dict()))
    with pytest.raises(ValueError, match="fields"):
        VendorManifest.from_dict({**data, "exclude_all_callers": True})
    with pytest.raises(ValueError, match="schema"):
        VendorManifest.from_dict({**data, "schema_version": True})


def test_json_records_refuse_ambiguous_keys_and_do_not_overwrite(tmp_path):
    path = tmp_path / "record.json"
    assert publish_json(path, {"key": "first"})
    assert not publish_json(path, {"key": "first"}, allow_identical=True)
    with pytest.raises(FileExistsError):
        publish_json(path, {"key": "second"}, allow_identical=True)
    assert read_json(path) == {"key": "first"}
    path.write_text('{"key":1,"key":2}')
    with pytest.raises(ValueError, match="duplicate"):
        read_json(path)


def test_component_commands_report_scope_and_exit_codes(vendor, capsys):
    root, manifest, archive = vendor
    configuration = root / "recovery/project.json"
    configuration.parent.mkdir()
    configuration.write_text(json.dumps({"schema_version": 1, "source": {
        "image_pin": "src/IMAGE_PIN", "address_space": "overlay:1"}}))
    write_component(root / "src" / "codec.recovery.h", manifest)
    (root / "src/IMAGE_PIN").write_text("sha256 " + "a" * 64 + "\n")
    assert main(["components", "list", "--root", str(root)]) == 0
    assert json.loads(capsys.readouterr().out)[0]["workflow"] == "verify_dependency"
    arguments = ["components", "check-source", "codec", "--root", str(root), "--archive", str(archive)]
    assert main(arguments) == 0
    assert json.loads(capsys.readouterr().out)["binary_verification"] == "unknown"
    (root / "vendor" / "codec.c").write_text("damaged")
    assert main(arguments) == 1
    capsys.readouterr()
    (root / "vendor" / "codec.c").unlink()
    assert main(arguments) == 3


def test_source_annotation_is_the_only_component_authority(vendor):
    root, manifest, _ = vendor
    path = root / "src" / "codec.recovery.h"
    assert write_component(path, manifest)
    assert not write_component(path, manifest)
    text = path.read_text()
    assert "0x00001000-0x00001008" in text
    assert text.count('"image"') == 1
    assert parse_component(text) == manifest
    assert read_components(root / "src") == [manifest]
    assert not (root / "recovery").exists()
    assert not (root / ".opensmacx").exists()
    with pytest.raises(FileExistsError):
        write_component(path, replace(manifest, flags=("-O0",)))
    with pytest.raises(ValueError, match="duplicate component"):
        (root / "src" / "another.recovery.h").write_text(text)
        read_components(root / "src")


@pytest.mark.parametrize("damage", [
    lambda text: text.replace("BEGIN v1", "BEGIN v2"),
    lambda text: text.replace('//   "kind":', '//   "kind": "duplicate", "kind":'),
    lambda text: text.replace("0x00001000-0x00001008", "0x00001000-4096"),
    lambda text: text.replace('//   "files": [', 'int code;'),
    lambda text: text + text,
])
def test_corrupted_source_annotations_fail_closed(vendor, damage):
    _, manifest, _ = vendor
    with pytest.raises(ValueError):
        parse_component(damage(render_component(manifest)))
