"""PDB evidence belongs to the same live native build as its linked image."""

import os
import struct

import pytest

from match_recovery.build_check import Build, check_build
from match_recovery.linked_initializers import _artifacts, observe_build_initializers
from test_build_check import command, project, write_config
from test_link_file_evidence import stamp
from test_linked_initializers import build
from test_msf import small_msf
from test_pdb_fixups import parts
from test_linked_initializers import update_artifact


def add_pdb(context, raw=b"container not yet inspected", *, name="app.pdb"):
    path = context.directory / name
    path.write_bytes(raw)
    context.report["outputs"].append({"path": name, "format": "msvc-pdb",
                                      "result": "pass", **stamp(raw)})
    context.pin_outputs()
    return path


def add_bound_pdb(context, *, wrong_signature=False):
    """Keep the existing linked table; add physical NB10 and its PDB sidecar."""
    reference, streams = parts(())
    payload = reference[0x400:]
    raw = bytearray((context.directory / "app.exe").read_bytes())
    payload_at = len(raw)
    raw.extend(payload)
    struct.pack_into("<I", raw, 0x98 + 60, 0x200)
    struct.pack_into("<II", raw, 0x98 + 96 + 6 * 8, 0x1100, 28)
    struct.pack_into("<IIHHIIII", raw, 0x300, 0, 0, 0, 0, 2, len(payload), 0, payload_at)
    streams[5] = bytes(raw[0x178:0x1C8])
    if wrong_signature:
        info = bytearray(streams[1])
        struct.pack_into("<I", info, 4, 0)
        streams[1] = bytes(info)
    update_artifact(context, "app.exe", bytes(raw))
    return add_pdb(context, small_msf(streams))


def test_pdb_is_optional_for_existing_native_observation(tmp_path):
    context = build(tmp_path / "project")
    assert "msvc-pdb" not in _artifacts(context)
    report = observe_build_initializers(context)
    assert report["result"] == "pass"
    assert "msvc-pdb" not in report["artifacts"]


def test_native_observation_rejects_duplicate_pdb_artifacts(tmp_path):
    context = build(tmp_path / "project")
    add_pdb(context)
    add_pdb(context, name="other.pdb")
    report = observe_build_initializers(context)
    assert report["result"] == "unknown"
    assert "one native artifact of format msvc-pdb" in report["reason"]


@pytest.mark.parametrize("damage", ["contents", "missing", "symlink", "hardlink"])
def test_pdb_uses_native_physical_file_and_content_guards(tmp_path, damage):
    context = build(tmp_path / "project")
    path = add_pdb(context)
    if damage == "contents":
        path.write_bytes(b"different contents")
    elif damage == "missing":
        path.unlink()
    else:
        other = path.with_suffix(".original")
        path.rename(other)
        if damage == "symlink":
            path.symlink_to(other)
        else:
            os.link(other, path)
    with pytest.raises(ValueError, match="contents changed|unaliased physical files"):
        _artifacts(context)


def test_pdb_cannot_escape_retained_build_directory(tmp_path):
    context = build(tmp_path / "project")
    add_pdb(context)
    context.report["outputs"][-1]["path"] = "../app.pdb"
    with pytest.raises(ValueError, match="not a relative path"):
        _artifacts(context)


def test_pdb_changes_expire_live_context_before_observation(tmp_path):
    context = build(tmp_path / "project")
    path = add_pdb(context)
    path.write_bytes(b"changed after build")
    with pytest.raises(ValueError, match="artifact changed"):
        observe_build_initializers(context)


def test_pdb_is_bound_to_actual_image_before_code_observation(tmp_path):
    context = build(tmp_path / "project")
    add_bound_pdb(context, wrong_signature=True)
    report = observe_build_initializers(context)
    assert report["result"] == "unknown"
    assert "signature/age disagree" in report["reason"]


def test_paired_pdb_is_passed_to_roots_and_dependency_graph(tmp_path, monkeypatch):
    from match_recovery import linked_initializers
    from match_recovery.pdb_fixups import BoundFixups

    context = build(tmp_path / "project")
    path = add_bound_pdb(context)
    seen = []
    for name in ("_function", "observe_code_graph"):
        original = getattr(linked_initializers, name)
        def observe(*args, _original=original, _name=name, **kwargs):
            fixups = kwargs["fixups"]
            assert isinstance(fixups, BoundFixups)
            assert fixups.image_sha256 == stamp((context.directory / "app.exe").read_bytes())["sha256"]
            assert fixups.pdb_sha256 == stamp(path.read_bytes())["sha256"]
            seen.append((_name, fixups))
            return _original(*args, **kwargs)
        monkeypatch.setattr(linked_initializers, name, observe)
    report = observe_build_initializers(context)
    assert report["result"] == "pass"
    assert report["artifacts"]["msvc-pdb"] == stamp(path.read_bytes())
    assert {name for name, _ in seen} == {"_function", "observe_code_graph"}
    assert all(item is seen[0][1] for _, item in seen)
    assert not report["ready_for_collection"]


def test_paired_pdb_change_during_observation_expires_whole_result(tmp_path, monkeypatch):
    from match_recovery import linked_initializers

    context = build(tmp_path / "project")
    path = add_bound_pdb(context)
    original = linked_initializers._observe
    def observe(*args, **kwargs):
        report = original(*args, **kwargs)
        path.write_bytes(path.read_bytes() + b"changed during observation")
        return report
    monkeypatch.setattr(linked_initializers, "_observe", observe)
    with pytest.raises(ValueError, match="artifact changed"):
        observe_build_initializers(context)


@pytest.mark.parametrize("raw,expected", [(small_msf([None, b"container only"]), "pass"),
                                         (b"not a PDB container", "unknown")])
def test_fresh_build_accepts_supported_pdb_container_only(project, raw, expected):
    write_config(project, commands=[command(
        "from pathlib import Path; import sys; "
        "(Path(sys.argv[1])/'app.pdb').write_bytes(bytes.fromhex(sys.argv[2]))",
        "{build}", raw.hex())], outputs=[{"path": "app.pdb", "format": "msvc-pdb"}])
    assert Build.read(project).outputs == (("app.pdb", "msvc-pdb"),)
    report = check_build(project)
    assert report["result"] == expected
    assert report["outputs"][0]["result"] == expected
    assert not report["ready_for_collection"]
    assert not report["persisted"]
    # A successful container build has no image identity or fixup proof.
    assert "code_dependencies" not in report


def test_declared_pdb_cannot_be_satisfied_by_a_previous_build(project):
    (project / "build").mkdir()
    (project / "build/app.pdb").write_bytes(small_msf([None, b"old container"]))
    write_config(project, commands=[command("pass", "{build}")],
                 outputs=[{"path": "app.pdb", "format": "msvc-pdb"}])
    report = check_build(project)
    assert report["result"] == "unknown"
    assert report["outputs"][0]["result"] == "unknown"
