"""UNBOUND mappings stay visible as unknowns and never acquire a body by name."""

import json
from pathlib import Path

import pytest

from match_recovery.match_check import check_matches
from match_recovery.recording import record as record_public
from match_recovery.source import Mode, State, Tier, read_file
from match_recovery.source.asm import (compare_source, compare_unit, compiled_asm)
from match_recovery.source.recording import record_claims
from match_recovery.source_measure import measure_source
from test_claim_integrity import configuration
from test_recording import calls, compiler, project


def mark_unbound(root, *, claim=True):
    source = root / "engine/family.cc"
    raw = source.read_text()
    marker = "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001"
    replacement = marker + (" UNBOUND BYTE_EXACT" if claim else " UNBOUND")
    assert raw.count(marker) == 1
    source.write_text(raw.replace(marker, replacement))
    return source


def configured_match_project(root):
    (root / "build").mkdir(exist_ok=True)
    (root / "build/commands.json").write_text(json.dumps([
        {"directory": str(root), "file": "engine/family.cc", "arguments": ["compiler", "engine/family.cc"]}
    ]))
    configuration(root, lambda value: value.update(mappings={
        "compile_commands": "build/commands.json", "forwarder_macros": []}))


def test_unbound_public_measurement_is_unknown_without_compiling(project):
    mark_unbound(project)
    row = next(row for row in read_file(project / "engine/family.cc") if row.name == "_f")
    assert (row.mode, row.state) == (Mode.UNBOUND, State.UNBOUND)
    report = measure_source(project, ("_f",), image=Path("original/input.exe"))
    measured, = report["measurements"]
    assert report["result"] == "unknown"
    assert report["counts"] == {"unknown": 1}
    assert report["compile_attempts"] == report["measurement_groups"] == 0
    assert measured["result"] == "unknown" and measured["verdict"] == "UNRESOLVED"
    assert measured["historical_byte_obligation"] and measured["symbol"] == ""
    assert "UNBOUND" in measured["diagnostic"]
    assert not calls(project)


def test_mixed_measurement_keeps_unbound_unknown_despite_same_name_coff_symbol(project):
    source = mark_unbound(project)
    assert source.relative_to(project).as_posix() == "engine/family.cc"
    report = measure_source(project, ("_f", "_h"), image=Path("original/input.exe"))
    rows = {row["name"]: row for row in report["measurements"]}
    assert report["result"] == "unknown"
    assert report["counts"] == {"unknown": 1, "pass": 1}
    assert report["compile_attempts"] == 1
    assert rows["_f"]["result"] == "unknown" and rows["_f"]["symbol"] == ""
    assert rows["_h"]["result"] == "pass"
    # The fixture compiler emits `_f`, but that incidental symbol is not a
    # source binding for an UNBOUND original identity.
    assert calls(project)[0]["source"] == "engine/family.cc"


def test_check_matches_keeps_unclaimed_unbound_visible_and_blocks_whole_pass(project):
    source = mark_unbound(project, claim=False)
    # Give the ordinary sibling a real obligation so this specifically proves
    # that an unclaimed UNBOUND row cannot disappear behind a passing claim.
    source.write_text(source.read_text().replace(
        "// ORIGINAL: 0x00401001 _h 0x00401001-0x00401004",
        "// ORIGINAL: 0x00401001 _h 0x00401001-0x00401004 BYTE_EXACT"))
    configured_match_project(project)
    report = check_matches(project, Path("original/input.exe"))
    unbound, = [row for row in report["measurements"] if row["source_mode"] == "unbound"]
    assert report["claims"] == report["verified"] == 1
    assert report["result"] == "unknown" and report["counts"] == {"pass": 1, "unknown": 1}
    assert unbound["result"] == "unknown" and unbound["verdict"] == "UNBOUND"
    assert not unbound["byte_obligation"] and not unbound["shape_obligation"]
    assert unbound["symbol"] == "" and unbound["compile_context"] is None and not unbound["built"]
    assert any(row["address"] == unbound["address"] for row in report["unverifiable"])


def test_low_level_comparers_refuse_unbound_before_compiler_lookup(project):
    source = mark_unbound(project)
    row = next(row for row in read_file(source) if row.name == "_f")
    observed = []

    def compiler_call(*args):
        observed.append(args)
        return b"not an object"

    result = compare_source(row, project / "original/input.exe", source, (), ("trial",),
                            compiler=compiler_call)
    assert result.verdict is Tier.REFUSED and "UNBOUND" in result.diagnostic
    unit = compare_unit((row,), project / "original/input.exe", source, (), ("trial",),
                        compiler=compiler_call)
    assert unit.compile_attempts == 0 and unit.reused_attempts == 0
    assert unit.results[0][1].verdict is Tier.REFUSED
    with pytest.raises(ValueError, match="UNBOUND"):
        compiled_asm(row, (), ("trial",), compiler=compiler_call)
    assert not observed


def test_recording_paths_refuse_unbound_before_validation_or_measurement(project):
    source = mark_unbound(project)
    assert source.relative_to(project).as_posix() == "engine/family.cc"
    before = source.read_bytes()
    row = next(row for row in read_file(source) if row.name == "_f")
    observed = []

    with pytest.raises(ValueError, match="UNBOUND"):
        record_claims((row,), measure=lambda: observed.append("measure"),
                      validate=lambda: observed.append("validate"),
                      staging_directory=project / "build/recovery/publications")
    with pytest.raises(ValueError, match="UNBOUND"):
        record_public(project, ("_f",), image=Path("original/input.exe"))
    assert not observed and not calls(project)
    assert source.read_bytes() == before
    assert not (project / "build/recovery/publications").exists()
