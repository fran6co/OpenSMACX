"""Portable source-check behavior and independent writer-damage controls."""

import json
import os
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.cli import main
import match_recovery.source_integrity as integrity


TEXT = """// ORIGINAL: 0x00401000 f 0x00401000-0x00401008 BYTE_EXACT
// LEVER: branch preserve this evidence
//              and this wrapped qualification
int f() { return 1; }
// Unrelated prose must survive, too.
// ORIGINAL: 0x00402000 g 0x00402000-0x00402008
// TRIED: an earlier spelling
int g() { return 2; }
"""


@pytest.fixture
def project(tmp_path):
    (tmp_path / "recovery").mkdir()
    (tmp_path / "engine/seeds").mkdir(parents=True)
    (tmp_path / "engine/window.cxx").write_text(TEXT)
    (tmp_path / "engine/window.hpp").write_text("struct Window {};\n")
    (tmp_path / "engine/seeds/old.cxx").write_text(TEXT.replace("0x0040", "0x0050"))
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1, "source": {
            "image_pin": "original/PIN", "address_space": "custom:offset",
            "roots": ["engine"], "extensions": [".cxx", ".hpp"],
            "seed_roots": ["engine/seeds"], "include_paths": ["engine"],
        },
    }))
    return tmp_path


def test_configured_sources_headers_and_seeds_without_git_or_compiler(project):
    before = {path: (path.read_bytes(), path.stat().st_mtime_ns) for path in project.rglob("*") if path.is_file()}
    report = integrity.check_source(project)
    assert report["result"] == "pass"
    assert report["annotations"] == 4 and report["files_scanned"] == 3
    assert not report["persisted"] and not report["ready_for_collection"]
    assert before == {path: (path.read_bytes(), path.stat().st_mtime_ns) for path in project.rglob("*") if path.is_file()}


def test_parallel_check_has_the_same_evidence(project):
    assert integrity.check_source(project, jobs=2) == integrity.check_source(project)


@pytest.mark.parametrize("rule", ["noop_identity", "keyword_isolation", "demotion_line_count",
                                   "demotion_lessons", "lesson_deletion"])
def test_writer_damage_is_detected_even_when_record_roundtrips_could_agree(monkeypatch, rule):
    real = integrity.write

    def damaged(text, records):
        after = real(text, records)
        if rule == "noop_identity" and len(records) == 2:
            return after.replace("Unrelated prose", "Rewritten prose")
        if len(records) == 1 and records[0].address == 0x00401000:
            record = records[0]
            if rule == "keyword_isolation" and not record.byte_exact and record.levers:
                return after.replace("return 1", "return 9")
            if not record.byte_exact and not record.levers and record.ruled_out:
                if rule == "demotion_line_count":
                    return after + "\n"
                if rule == "demotion_lessons":
                    return after.replace("TRIED: branch", "TRIED: dropped")
            if rule == "lesson_deletion" and not record.levers and not record.ruled_out:
                # Same number of lines, wrong surviving source: the old line
                # count-only deletion invariant could not catch this damage.
                return after.replace("return 1", "return 9")
        return after

    monkeypatch.setattr(integrity, "write", damaged)
    report = integrity.check_annotation_text(Path("unit.cxx"), TEXT)
    assert rule in {finding["rule"] for finding in report["findings"]}


def test_independent_marker_witness_catches_silent_parser_loss(monkeypatch):
    monkeypatch.setattr(integrity, "read_text", lambda *args: [])
    report = integrity.check_annotation_text(Path("unit.cxx"), TEXT)
    assert [row["line"] for row in report["findings"]] == [1, 6]
    assert {row["rule"] for row in report["findings"]} == {"marker_coverage"}


def test_malformed_marker_and_unsupported_source_encoding_cannot_pass(project):
    (project / "engine/window.hpp").write_text("// ORIGINAL: not-an-address\n")
    report = integrity.check_source(project)
    assert report["result"] == "fail"
    assert report["findings"][0]["rule"] == "marker_coverage"
    (project / "engine/window.hpp").write_bytes(b"\xff")
    report = integrity.check_source(project)
    assert report["result"] == "unknown" and report["errors"]


@pytest.mark.parametrize("contents", ["", "int no_annotations;\n"])
def test_no_annotations_do_not_establish_coverage(project, contents):
    for path in (project / "engine").rglob("*"):
        if path.is_file():
            path.write_text(contents)
    assert integrity.check_source(project)["result"] == "unknown"


def test_directory_alias_cannot_hide_source(project):
    (project / "engine/alias").symlink_to(project / "engine/seeds", target_is_directory=True)
    with pytest.raises(ValueError, match="symlinks"):
        integrity.check_source(project)


@pytest.mark.parametrize("change", ["source", "add", "configuration"])
def test_census_refuses_inputs_changed_while_checking(project, monkeypatch, change):
    real = integrity._one_file

    def alter(item):
        result = real(item)
        if change == "source":
            (project / "engine/window.hpp").write_text("struct Changed {};\n")
        elif change == "add":
            (project / "engine/added.hpp").write_text("int added;\n")
        else:
            path = project / "recovery/project.json"
            path.write_bytes(path.read_bytes() + b"\n")
        return result

    monkeypatch.setattr(integrity, "_one_file", alter)
    with pytest.raises(ValueError, match="changed"):
        integrity.check_source(project)


@pytest.mark.parametrize("jobs", [0, -1, True, 1.5])
def test_invalid_concurrency_is_refused(project, jobs):
    with pytest.raises(ValueError, match="positive integer"):
        integrity.check_source(project, jobs=jobs)


def test_cli_status_and_summary(project, capsys):
    assert main(["check-source", "--root", str(project), "--summary"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["annotations"] == 4 and "findings" not in report
    (project / "engine/window.hpp").write_text("// ORIGINAL: bad\n")
    assert main(["check-source", "--root", str(project)]) == 1
    assert json.loads(capsys.readouterr().out)["findings"]


def test_command_runs_without_site_packages_or_project_python(project):
    import match_recovery
    environment = dict(os.environ, PYTHONPATH=str(Path(match_recovery.__file__).parents[1]))
    result = subprocess.run([sys.executable, "-S", "-m", "match_recovery.cli", "check-source",
                             "--root", str(project), "--summary"],
                            env=environment, cwd=project, capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    assert json.loads(result.stdout)["annotations"] == 4
