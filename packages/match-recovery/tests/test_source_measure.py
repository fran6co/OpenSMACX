"""Diagnostic measurement shares compilation, not recording authority."""

import json
from pathlib import Path

import pytest

from match_recovery.cli import main
from match_recovery.source_measure import measure_source
from test_recording import project, compiler, calls, FIRST
from test_claim_integrity import configuration


def measure(root, *targets, **kwargs):
    return measure_source(root, targets or ("_f", "_h"), image=Path("original/input.exe"), **kwargs)


def test_cli_batches_subjects_without_banking_or_changing_source(project, capsys):
    path = project / "engine/family.cc"
    before = path.read_bytes(), path.stat().st_mtime_ns
    assert main(["measure", "_f", "_h", "--root", str(project), "--image", "original/input.exe"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["selected"] == 2 and report["counts"] == {"pass": 2}
    assert report["compile_attempts"] == report["translation_units"] == 1 and report["reused_attempts"] == 1
    assert before == (path.read_bytes(), path.stat().st_mtime_ns)
    assert not (project / "build/recovery/publications").exists()
    assert not report["ready_for_collection"] and report["verification"] == "historical_masked_bytes_only"


def test_mismatch_retains_detailed_diagnostic_and_existing_claim(project):
    path = project / "engine/family.cc"
    path.write_text(path.read_text().replace("0x00401001\n", "0x00401001 BYTE_EXACT\n"))
    before = path.read_bytes()
    compiler(project, [("_f", b"\x33\xc0\xc3"), ("_h", b"\x33\xc0\xc3")])
    report = measure(project)
    assert report["result"] == "fail" and report["counts"] == {"fail": 1, "pass": 1}
    assert report["measurements"][0]["context"] and report["measurements"][0]["historical_byte_obligation"]
    assert path.read_bytes() == before


def test_unavailable_compiler_is_unknown_not_a_measured_miss(project):
    compiler(project, failure=True)
    report = measure(project)
    assert report["result"] == "unknown" and report["counts"] == {"unknown": 2}


def test_placeholder_does_not_pass_even_if_an_empty_body_would_match(project):
    path = project / "engine/family.cc"
    path.write_text(path.read_text().replace("void f() {}", "void f() {\n// BODY GOES HERE.\n}"))
    report = measure(project, "_f")
    assert report["result"] == "unknown" and report["compile_attempts"] == 0
    assert "placeholder" in report["measurements"][0]["diagnostic"] and not calls(project)


def test_diagnostics_do_not_require_satisfied_historical_claims(project):
    configuration(project, lambda c: c.pop("source_history"))
    assert measure(project)["result"] == "pass"


@pytest.mark.parametrize("selection", [("_f", "_f"), ("missing",), ()])
def test_invalid_selection_refuses_before_compilation(project, selection):
    with pytest.raises(ValueError):
        measure_source(project, selection, image=Path("original/input.exe"))
    assert not calls(project)


def test_seed_mapping_is_not_silently_displaced_by_product_source(project):
    (project / "seeds/f.cc").write_text(FIRST)
    with pytest.raises(ValueError, match="requires one source mapping"):
        measure(project, "_f")
    assert measure(project, "_f", in_file="engine/family.cc")["result"] == "pass"


def test_changed_header_refuses_mixed_measurement_epoch(project):
    compiler(project, effect="Path('engine/types.hpp').write_text('struct Changed {};')")
    with pytest.raises(ValueError, match="changed during measurement"):
        measure(project)


def test_partial_unit_result_cannot_pass(project, monkeypatch):
    import match_recovery.source_measure as module
    from dataclasses import replace
    actual = module._unit
    def missing(job):
        result = actual(job)
        return replace(result, results=result.results[:1])
    monkeypatch.setattr(module, "_unit", missing)
    with pytest.raises(ValueError, match="cover exactly"):
        measure(project)


def test_measurement_never_reuses_a_previous_invocation_result(project):
    measure(project)
    measure(project)
    assert len(calls(project)) == 2


def test_unselected_mapping_still_refuses_a_shared_original_span(project):
    (project / "seeds/shared.cc").write_text(
        "// ORIGINAL: 0x00401010 _other 0x00401000-0x00401001\nvoid other() {}\n")
    report = measure(project, "_f", in_file="engine/family.cc")
    assert report["result"] == "unknown"
    assert report["measurements"][0]["verdict"] == "SHARED_TAIL"


def test_warm_inventory_reparses_only_selected_hosts_and_never_caches_measurement(project, monkeypatch):
    from match_recovery.source_inventory import source_snapshot
    from match_recovery.source import reader

    monkeypatch.setenv("RECOVERY_PARSE_CACHE", "on")
    (project / "seeds/other.cc").write_text(
        "// ORIGINAL: 0x00401010 _other 0x00401010-0x00401011\nvoid other() {}\n")
    source_snapshot(project)  # Populate the existing disposable parse memo.
    original, parsed = reader._read_text, []

    def observe(text, path):
        parsed.append(Path(path))
        return original(text, path)

    monkeypatch.setattr(reader, "_read_text", observe)
    first = measure(project)
    assert first["result"] == "pass"
    assert parsed == [project / "engine/family.cc"]
    parsed.clear()
    second = measure(project)
    assert first == second
    assert parsed == [project / "engine/family.cc"]
    assert len(calls(project)) == 2


def test_measurement_preserves_replacement_decoding_for_invalid_source_bytes(project):
    path = project / "engine/family.cc"
    path.write_bytes(path.read_bytes() + b"\n// provenance: \xff\n")
    assert measure(project)["result"] == "pass"


def test_file_mode_measurement_keeps_crlf_source_and_compiles_the_same_host(project):
    path = project / "engine/family.cc"
    text = FIRST.replace("\n", " FILE\n", 1)
    path.write_bytes(text.encode())
    before = measure(project, "_f")
    raw = text.replace("\n", "\r\n").encode()
    path.write_bytes(raw)
    after = measure(project, "_f")
    assert before["result"] == after["result"] == "pass"
    assert before["measurements"] == after["measurements"]
    assert path.read_bytes() == raw and len(calls(project)) == 2


def test_changed_mapping_cannot_reuse_an_inventory_entry_at_same_timestamp(project):
    import os

    path = project / "engine/family.cc"
    before = path.stat()
    assert measure(project, "_f")["result"] == "pass"
    path.write_bytes(path.read_bytes().replace(b"_f", b"_g"))
    os.utime(path, ns=(before.st_atime_ns, before.st_mtime_ns))
    with pytest.raises(ValueError, match="found 0"):
        measure(project, "_f")
    assert len(calls(project)) == 1
