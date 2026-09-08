"""Pinned inspection, not a second ledger or a successful measurement."""

import hashlib
import json
from pathlib import Path

import pytest

from match_recovery.cli import main
from match_recovery.inspection import inspect_source
from test_binary import pe_fixture
from test_recording import project, calls, FIRST
from test_claim_integrity import configuration


def inspect(root, *targets, **kwargs):
    return inspect_source(root, targets or ("_f",), **kwargs)


def image(root, code):
    (root / "original/input.exe").write_bytes(pe_fixture(code))
    (root / "original/PIN").write_text("sha256 " + hashlib.sha256((root / "original/input.exe").read_bytes()).hexdigest())


def test_source_only_inspection_needs_no_compiler_or_present_image(project):
    (project / "original/input.exe").unlink()
    report = inspect(project)
    row, = report["subjects"]
    assert row["source"]["catalogued_symbol"] == "_f" and "void f()" in row["source_region"]
    assert row["source"]["tried"] == ["seed observation"] and row["original"] is None
    assert not report["image_verified"] and report["result"] == "pass"
    assert report["verification"] == "not_measured" and not report["ready_for_collection"]
    assert not calls(project)


def test_pinned_image_inspection_is_read_only_and_does_not_trim_padding(project):
    path = project / "engine/family.cc"
    path.write_text(FIRST.replace("0x00401001", "0x00401004"))
    image(project, b"\xc3\x90\xcc\x90")
    before = path.read_bytes(), path.stat().st_mtime_ns
    report = inspect(project, image=Path("original/input.exe"))
    original = report["subjects"][0]["original"]
    assert report["result"] == "pass" and report["image_verified"]
    assert original["spans"][0]["decoded_bytes"] == 4 and len(original["spans"][0]["disassembly"]) == 4
    assert before == (path.read_bytes(), path.stat().st_mtime_ns)
    assert not calls(project)


def test_duplicate_seed_selection_is_explicit_and_alternatives_survive(project):
    (project / "seeds/f.cc").write_text(FIRST)
    with pytest.raises(ValueError, match="requires one source mapping"):
        inspect(project)
    report = inspect(project, in_file="engine/family.cc")
    assert report["subjects"][0]["alternatives"][0]["path"] == "seeds/f.cc"


def test_exact_host_is_not_a_path_fragment(project):
    with pytest.raises(ValueError, match="found 0"):
        inspect(project, in_file="family.cc")


def test_calls_keep_repetitions_unknown_targets_and_seed_alternatives(project):
    image(project, b"\xe8\x0b\0\0\0\xe8\x06\0\0\0\xff\xd0\xc3" + b"\x90" * 3 + b"\xc3")
    (project / "engine/family.cc").write_text(
        FIRST.replace("0x00401001", "0x0040100D") +
        "// ORIGINAL: 0x00401010 _callee 0x00401010-0x00401011\nvoid callee() {}\n")
    (project / "seeds/callee.cc").write_text(
        "// ORIGINAL: 0x00401010 _old_label 0x00401010-0x00401011\nvoid old_label() {}\n")
    report = inspect(project, image=Path("original/input.exe"))
    edges = report["subjects"][0]["original"]["calls"]
    assert [e["target"] for e in edges] == [0x401010, 0x401010, None]
    assert {row["catalogued_symbol"] for row in edges[0]["mapped_targets"]} == {"_callee", "_old_label"}
    assert edges[-1]["form"] == "dynamic" and not edges[-1]["mapped_targets"]


def test_jump_between_recorded_fragments_is_not_a_tail_call(project):
    image(project, b"\xe9\x03\0\0\0\x90\x90\x90\xc3")
    (project / "engine/family.cc").write_text(FIRST.replace(
        "0x00401000-0x00401001", "0x00401000-0x00401005;0x00401008-0x00401009"))
    original = inspect(project, image=Path("original/input.exe"))["subjects"][0]["original"]
    assert len(original["spans"]) == 2 and original["calls"] == []


def test_incoming_calls_derive_from_bytes_not_annotation_call_notes(project):
    image(project, b"\xe8\x01\0\0\0\xc3\xc3")
    (project / "engine/family.cc").write_text(FIRST.replace("0x00401001", "0x00401006") +
        "// ORIGINAL: 0x00401006 _callee 0x00401006-0x00401007\nvoid callee() {}\n")
    report = inspect(project, "_callee", image=Path("original/input.exe"), callers=True)
    edge, = report["callers"]["edges"]
    assert edge["at"] == 0x401000 and edge["caller"]["catalogued_symbol"] == "_f"
    assert report["callers"]["coverage"] == "mapped_spans_only" and report["decoded_bytes"] == 7


def test_decode_budget_and_invalid_instructions_keep_evidence_incomplete(project):
    report = inspect(project, "_f", "_h", image=Path("original/input.exe"), max_bytes=1)
    assert report["result"] == "unknown" and report["decoded_bytes"] == 1
    assert "budget" in report["subjects"][1]["original"]["spans"][0]["reason"]
    image(project, b"\x0f")
    report = inspect(project, image=Path("original/input.exe"))
    assert report["result"] == "unknown" and report["decoded_bytes"] == 1


def test_incomplete_caller_coverage_is_not_an_empty_success(project):
    report = inspect(project, image=Path("original/input.exe"), callers=True, max_bytes=1)
    assert report["result"] == "unknown" and report["callers"]["errors"]


@pytest.mark.parametrize("options", [{"max_bytes": 0}, {"max_bytes": True}, {"callers": True}, {"callers": 1}])
def test_invalid_requests_refuse(project, options):
    with pytest.raises(ValueError):
        inspect(project, **options)


def test_wrong_image_pin_refuses_before_disassembly(project):
    with (project / "original/input.exe").open("ab") as stream:
        stream.write(b"wrong")
    with pytest.raises(ValueError, match="digest mismatch"):
        inspect(project, image=Path("original/input.exe"))


def test_cli_image_and_source_inspection_have_distinct_fingerprints(project, capsys):
    assert main(["inspect", "_f", "--root", str(project)]) == 0
    source = json.loads(capsys.readouterr().out)
    assert main(["inspect", "_f", "--root", str(project), "--image", "original/input.exe"]) == 0
    original = json.loads(capsys.readouterr().out)
    assert source["input_fingerprint"] != original["input_fingerprint"]


def test_current_qualified_symbol_selects_without_obsolete_catalogue_name(project):
    configuration(project, lambda c: c["source"].update(symbol_format="msvc"))
    (project / "engine/family.cc").write_text(
        "// ORIGINAL: 0x00401000 ?old@Old@@QAEHXZ 0x00401000-0x00401001\n"
        "// symbol ?run@Current@@QAEHXZ\nint Current::run() {}\n")
    assert inspect(project, "Current::run")["subjects"][0]["source"]["catalogued_symbol"] == "?old@Old@@QAEHXZ"


@pytest.mark.parametrize("target,symbol", [("Thing::Thing", "??0Thing@@QAE@XZ"),
    ("Thing::~Thing", "??1Thing@@QAE@XZ"), ("WinMain", "_WinMain@16"), ("face", "face")])
def test_shared_selector_supports_lifetime_and_decorated_names(project, target, symbol):
    configuration(project, lambda c: c["source"].update(symbol_format="msvc"))
    (project / "engine/family.cc").write_text(FIRST.replace("_f", symbol))
    assert inspect(project, target)["subjects"][0]["source"]["catalogued_symbol"] == symbol


def test_import_calls_are_retained_with_independently_parsed_dll_name(project):
    from test_imports import image_imports
    data = image_imports()
    (project / "original/input.exe").write_bytes(data)
    (project / "original/PIN").write_text("sha256 " + hashlib.sha256(data).hexdigest())
    (project / "engine/family.cc").write_text(FIRST.replace("0x00401001", "0x00401007"))
    edge, = inspect(project, image=Path("original/input.exe"))["subjects"][0]["original"]["calls"]
    assert edge["target"] is None and edge["slot"] == 0x401160
    assert edge["import"]["module"] == "DEMO.dll" and edge["import"]["name"] == "Paint"


def test_image_change_during_inspection_is_refused(project, monkeypatch):
    import match_recovery.inspection as module
    actual = module.assert_snapshot_current
    def changed(root, snapshot):
        actual(root, snapshot)
        with (root / "original/input.exe").open("ab") as stream:
            stream.write(b"changed")
    monkeypatch.setattr(module, "assert_snapshot_current", changed)
    with pytest.raises(ValueError, match="original image changed"):
        inspect(project, image=Path("original/input.exe"))
