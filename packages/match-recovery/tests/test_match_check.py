"""Whole-project match coverage with unrelated paths and a real compiler client."""

from dataclasses import replace
import json
from pathlib import Path

import pytest

from match_recovery import match_check
from match_recovery.cli import main
from match_recovery.source import read_file
from match_recovery.source.asm import AsmComparison, UnitComparison
from match_recovery.source.model import Tier
from test_recording import project, compiler, calls, run
from test_claim_integrity import configuration, commit, git


@pytest.fixture
def mapped(project):
    (project / "build").mkdir(exist_ok=True)
    database(project, ["engine/family.cc"])
    configuration(project, lambda value: value.update(mappings={
        "compile_commands": "build/commands.json", "forwarder_macros": []}))
    return project


def database(root, names):
    (root / "build/commands.json").write_text(json.dumps([
        {"directory": str(root), "file": name, "arguments": ["compiler", name]} for name in names]))


def check(root, **kwargs):
    return match_check.check_matches(root, Path("original/input.exe"), **kwargs)


def test_check_measures_a_whole_family_without_writing_or_banking(mapped, capsys):
    run(mapped)
    commit(mapped, "Measured source")
    source = mapped / "engine/family.cc"
    before = source.read_bytes(), source.stat().st_ino, source.stat().st_mtime_ns
    code = main(["check-matches", "--root", str(mapped), "--image", "original/input.exe"])
    report = json.loads(capsys.readouterr().out)
    assert code == 0 and report["claims"] == report["verified"] == 2
    assert report["compile_attempts"] == report["translation_units"] == 1
    assert report["reused_attempts"] == 1 and len(report["measurements"]) == 2
    assert report["counts"] == {"pass": 2}
    assert not report["persisted"] and not report["ready_for_collection"]
    assert report["verification"] == "historical_masked_bytes_only"
    assert before == (source.read_bytes(), source.stat().st_ino, source.stat().st_mtime_ns)
    assert not git(mapped, "status", "--porcelain").strip()


def test_no_claims_does_not_pass_or_bank_discoveries(mapped):
    before = (mapped / "engine/family.cc").read_bytes()
    report = check(mapped)
    assert report["result"] == "unknown" and report["claims"] == 0
    assert len(report["free"]) == 2 and len(report["measurements"]) == 2
    assert (mapped / "engine/family.cc").read_bytes() == before


def test_fresh_calls_never_reuse_a_previous_invocations_verdict(mapped):
    run(mapped)
    check(mapped)
    previous = len(calls(mapped))
    compiler(mapped, [("_f", b"\x33\xc0\xc3"), ("_h", b"\x33\xc0\xc3")])
    report = check(mapped)
    assert report["result"] == "fail" and report["verified"] == 1
    assert report["regressed"][0]["symbol"] == "_f"
    assert len(calls(mapped)) > previous
    assert all(row.byte_exact for row in read_file(mapped / "engine/family.cc"))


@pytest.mark.parametrize("seed", [False, True])
@pytest.mark.parametrize("failure", ["compile", "missing-symbol"])
def test_built_subject_loss_fails_but_unbuilt_seed_loss_remains_unknown(mapped, seed, failure):
    run(mapped)
    if seed:
        (mapped / "engine/family.cc").rename(mapped / "seeds/family.cc")
        (mapped / "engine/family.cc").write_text("// A real but currently unmapped product TU\n")
    if failure == "compile":
        compiler(mapped, failure=True)
    else:
        compiler(mapped, [("_unrelated", b"\xc3")])
    report = check(mapped)
    assert report["claims"] == 2 and report["verified"] == 0
    assert report["result"] == ("unknown" if seed else "fail")
    rows = report["unverifiable"] if seed else report["regressed"]
    assert len(rows) == 2 and all(row["seed"] == seed for row in rows)


def test_header_claims_are_visible_and_unmeasurable_not_dropped(mapped):
    run(mapped)
    (mapped / "engine/family.cc").rename(mapped / "engine/moved.hpp")
    (mapped / "engine/family.cc").write_text("// Unit still builds\n")
    report = check(mapped)
    assert report["result"] == "unknown" and report["claims"] == len(report["unsupported_hosts"]) == 2
    assert report["compile_attempts"] == 0
    assert all(row["path"] == "engine/moved.hpp" for row in report["unverifiable"])


@pytest.mark.parametrize("jobs", [1, 2])
def test_source_and_seed_attempts_do_not_overwrite_each_other(mapped, jobs):
    run(mapped)
    (mapped / "seeds/copy.cc").write_bytes((mapped / "engine/family.cc").read_bytes())
    report = check(mapped, jobs=jobs)
    assert report["claims"] == report["verified"] == 4
    assert report["translation_units"] == report["compile_attempts"] == 2
    assert {row["path"] for row in report["measurements"]} == {"engine/family.cc", "seeds/copy.cc"}
    # Mapping consistency rejects conflicting owners separately; this check
    # must ask every historical attempt instead of counting one address twice.


def test_repeated_markers_in_one_file_do_not_silently_collapse(mapped):
    run(mapped)
    path = mapped / "engine/family.cc"
    path.write_bytes(path.read_bytes() * 2)
    report = check(mapped)
    assert report["claims"] == len(report["measurements"]) == 4
    assert len({row["line"] for row in report["measurements"]}) == 4


def test_shape_evidence_cannot_satisfy_a_simultaneous_byte_obligation(mapped):
    record = replace(read_file(mapped / "engine/family.cc")[0], byte_exact=True, semantic=True)
    observed = AsmComparison(Tier.MNEMONIC_ONLY, allocation_only="")
    settings = match_check.Measurement.read(mapped)
    row = match_check._observation(mapped, record, observed, built={"engine/family.cc"}, source=settings.source)
    assert row["result"] == "fail"
    shape = match_check._observation(mapped, replace(record, byte_exact=False), observed,
                                     built={"engine/family.cc"}, source=settings.source)
    assert shape["result"] == "pass"


@pytest.mark.parametrize("change", ["header", "new-header", "database", "image", "compiler", "history"])
def test_changed_inputs_cannot_supply_a_completed_measurement(mapped, monkeypatch, change):
    run(mapped)
    original = match_check._unit
    def mutate(job):
        result = original(job)
        if change == "header":
            (mapped / "engine/types.hpp").write_text("struct Value { int shifted[2]; };\n")
        elif change == "new-header":
            (mapped / "engine/new.inc").write_text("// Includes may shadow earlier input\n")
        elif change == "database":
            database(mapped, ["engine/family.cc", "engine/types.hpp"])
        elif change == "image":
            with (mapped / "original/input.exe").open("ab") as stream:
                stream.write(b"x")
        elif change == "compiler":
            compiler(mapped, failure=True)
        else:
            commit(mapped, "Concurrent revision")
        return result
    monkeypatch.setattr(match_check, "_unit", mutate)
    with pytest.raises(ValueError, match="changed|pinned original"):
        check(mapped)


@pytest.mark.parametrize("damage", ["dropped", "duplicated", "swapped"])
def test_worker_must_cover_the_exact_selected_mapping_list(mapped, monkeypatch, damage):
    original = match_check._unit
    def corrupt(job):
        report = original(job)
        rows = report.results
        if damage == "dropped":
            rows = rows[:1]
        elif damage == "duplicated":
            rows = (rows[0], rows[0])
        else:
            rows = rows[::-1]
        return UnitComparison(rows, report.compile_attempts, report.reused_attempts)
    monkeypatch.setattr(match_check, "_unit", corrupt)
    with pytest.raises(ValueError, match="did not cover"):
        check(mapped)


@pytest.mark.parametrize("kind", ["wrong-image", "missing-image", "missing-input", "missing-root", "alias"])
def test_invalid_inputs_refuse_before_any_compiler(mapped, kind):
    if kind == "wrong-image":
        (mapped / "original/input.exe").write_bytes(b"wrong ruler")
    elif kind == "missing-image":
        (mapped / "original/input.exe").unlink()
    elif kind == "missing-input":
        configuration(mapped, lambda value: value["measurement"].update(inputs=["missing.txt"]))
    elif kind == "missing-root":
        configuration(mapped, lambda value: value["measurement"].update(input_roots=["missing"]))
    else:
        (mapped / "engine/alias.hpp").symlink_to(mapped / "engine/types.hpp")
    with pytest.raises((ValueError, OSError)):
        check(mapped)
    assert not calls(mapped)


@pytest.mark.parametrize("jobs", [0, -1, True, 1.5])
def test_invalid_compiler_concurrency_refused(mapped, jobs):
    with pytest.raises(ValueError, match="positive integer"):
        check(mapped, jobs=jobs)


def test_summary_retains_regression_identity(mapped, capsys):
    run(mapped)
    compiler(mapped, [("_f", b"\x33\xc0\xc3"), ("_h", b"\x33\xc0\xc3")])
    assert main(["check-matches", "--root", str(mapped), "--image", "original/input.exe", "--summary"]) == 1
    report = json.loads(capsys.readouterr().out)
    assert report["measured"] == 2 and "measurements" not in report
    assert report["regressed"][0]["path"] == "engine/family.cc"
