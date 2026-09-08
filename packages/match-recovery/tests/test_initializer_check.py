"""Pinned initial-storage migration lens, without a compiler or game fixture."""

import hashlib
import json
from pathlib import Path
import struct

import pytest

from match_recovery.cli import main
from match_recovery.initializer_check import check_initializers
from match_recovery.pe_coff import PeImage
from test_binary import pe_fixture


@pytest.fixture
def project(tmp_path):
    (tmp_path / "engine").mkdir()
    (tmp_path / "recovery").mkdir()
    data = bytearray(pe_fixture((9).to_bytes(4, "little")))
    struct.pack_into("<I", data, 0x178 + 36, 0xC0000040)
    struct.pack_into("<I", data, 0x180, 1024)  # virtual tail after raw data
    (tmp_path / "input.exe").write_bytes(data)
    (tmp_path / "engine/PIN").write_text("sha256 " + hashlib.sha256(data).hexdigest() + "\n")
    (tmp_path / "engine/global.cc").write_text("int Counter = 9; // 0x00401000\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1, "source": {"roots": ["engine"], "extensions": [".cc", ".hpp"],
            "seed_roots": ["engine/seeds"], "image_pin": "engine/PIN", "address_space": "pe:va"},
        "initializers": {"ruleset": "cpp-trailing-address-v1", "probe_bytes": 4,
            "scalar_types": ["int", "CustomWord"], "unit_extensions": [".cc"], "exclude_roots": []}}))
    return tmp_path


def check(root):
    return check_initializers(root, Path("input.exe"))


def test_missing_initializer_is_detected_and_restoration_is_readonly(project):
    path = project / "engine/global.cc"
    assert check(project)["result"] == "pass"
    path.write_text("int Counter; // 0x00401000\n")
    before = path.read_bytes(), path.stat().st_mtime_ns
    report = check(project)
    assert report["result"] == "fail" and report["counts"]["checked"] == 1
    assert report["findings"][0]["symbol"] == "Counter"
    assert not report["persisted"] and not report["ready_for_collection"]
    assert before == (path.read_bytes(), path.stat().st_mtime_ns)
    path.write_text("int Counter = 9; // 0x00401000\n")
    assert check(project)["result"] == "pass"


@pytest.mark.parametrize("initializer", ["0", "NULL", "nullptr", "{}", "{0}", "0u", "false"])
def test_explicit_zero_spellings_are_not_nonzero_initialization(project, initializer):
    (project / "engine/global.cc").write_text(f"int Counter = {initializer}; // 0x00401000\n")
    assert check(project)["result"] == "fail"


def test_virtual_zero_fill_and_raw_boundary_are_bounded(project):
    path = project / "engine/global.cc"
    path.write_text("int Tail; // 0x00401200\nint Boundary; // 0x004011FE\n")
    assert check(project)["counts"]["checked"] == 2
    assert check(project)["result"] == "pass"
    path.write_text("int Outside; // 0x004013FF\n")
    assert check(project)["result"] == "unknown"


def test_mixed_failure_and_unknown_never_passes(project):
    (project / "engine/global.cc").write_text("int Counter; // 0x00401000\nint Outside; // 0x00999999\n")
    report = check(project)
    assert report["result"] == "fail" and report["findings"] and report["errors"]


@pytest.mark.parametrize("damage", ["missing", "pin", "malformed", "code"])
def test_missing_unpinned_or_unsupported_original_never_passes(project, damage):
    image = project / "input.exe"
    if damage == "missing":
        image.unlink()
    elif damage == "pin":
        image.write_bytes(image.read_bytes() + b"changed")
    else:
        data = bytearray(image.read_bytes())
        if damage == "malformed": data[:2] = b"XX"
        else: struct.pack_into("<I", data, 0x178 + 36, 0x60000020)
        image.write_bytes(data)
        (project / "engine/PIN").write_text("sha256 " + hashlib.sha256(data).hexdigest() + "\n")
    if damage == "code":
        assert check(project)["result"] == "unknown"
    else:
        with pytest.raises((OSError, ValueError)):
            check(project)


def test_seeds_comments_and_unknown_types_do_not_become_coverage(project):
    (project / "engine/seeds").mkdir()
    (project / "engine/seeds/old.cc").write_text("int Bad; // 0x00401000\n")
    path = project / "engine/global.cc"
    path.write_text("/*\nint Comment; // 0x00401000\n*/\nUnknownType Opaque; // 0x00401000\n")
    report = check(project)
    assert report["result"] == "unknown" and report["counts"]["unsupported_types"] == 1
    path.write_text("CustomWord Counter = 9; // 0x00401000\n")
    assert check(project)["result"] == "pass"


@pytest.mark.parametrize("damage", ["image", "source", "config"])
def test_mutation_during_probe_is_refused(project, monkeypatch, damage):
    original = PeImage.read_initial
    def changed(image, span):
        result = original(image, span)
        path = project / {"image": "input.exe", "source": "engine/global.cc", "config": "recovery/project.json"}[damage]
        path.write_bytes(path.read_bytes() + b"\n")
        return result
    monkeypatch.setattr(PeImage, "read_initial", changed)
    with pytest.raises(ValueError, match="changed"):
        check(project)


def test_installed_cli_uses_root_and_preserves_failure_details(project, capsys):
    args = ["check-initializers", "--root", str(project), "--image", "input.exe", "--summary"]
    assert main(args) == 0
    assert json.loads(capsys.readouterr().out)["counts"]["checked"] == 1
    (project / "engine/global.cc").write_text("int Counter; // 0x00401000\n")
    assert main(args) == 1
    assert json.loads(capsys.readouterr().out)["findings"]
    (project / "input.exe").unlink()
    assert main(args) == 3
