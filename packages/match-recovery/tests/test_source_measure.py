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
