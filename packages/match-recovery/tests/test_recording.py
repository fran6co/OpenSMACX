"""Installed production recording through a real synthetic compiler protocol."""

import base64
import hashlib
import json
from pathlib import Path
import sys

import pytest

from match_recovery.cli import main
from match_recovery import recording
from match_recovery.source import read_file
from test_claim_integrity import git, commit, configuration
from test_binary import pe_fixture
from source.test_unit_comparison import coff


FIRST = "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\n// TRIED: seed observation\nvoid f() {}\n"
SECOND = "// ORIGINAL: 0x00401001 _h 0x00401001-0x00401004\nint h() {return 0;}\n"


def compiler(root, functions=None, *, effect="", failure=False):
    obj = coff(functions if functions is not None else [("_f", b"\xc3"), ("_h", b"\x33\xc0\xc3")])
    (root / "recovery/compiler.py").write_text(
        "import json, sys\nfrom pathlib import Path\nr = json.load(sys.stdin)\n"
        "assert r['schema_version'] == 2 and r['profile'] == 'fixture'\n"
        "Path('build').mkdir(exist_ok=True)\n"
        "with Path('build/calls.jsonl').open('a') as f: f.write(json.dumps(r)+'\\n')\n"
        + effect + "\n" + ("print(json.dumps({'error': 'synthetic compile failure'})); sys.exit(3)\n" if failure else
                           f"print(json.dumps({{'object_base64': {base64.b64encode(obj).decode()!r}}}))\n"))


@pytest.fixture
def project(tmp_path):
    git(tmp_path, "init", "-b", "main")
    git(tmp_path, "config", "user.name", "Control")
    git(tmp_path, "config", "user.email", "control@example.test")
    for name in ("engine", "seeds", "original", "recovery"):
        (tmp_path / name).mkdir()
    (tmp_path / ".gitignore").write_text("build/\noriginal/input.exe\n")
    (tmp_path / "engine/family.cc").write_text(FIRST + SECOND)
    (tmp_path / "engine/types.hpp").write_text("struct Value { int field; };\n")
    (tmp_path / "original/input.exe").write_bytes(pe_fixture(b"\xc3\x33\xc0\xc3"))
    (tmp_path / "original/PIN").write_text("sha256 " + hashlib.sha256((tmp_path / "original/input.exe").read_bytes()).hexdigest() + "\n")
    compiler(tmp_path)
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "original/PIN", "address_space": "pe:va", "roots": ["engine", "seeds"],
                   "extensions": [".cc", ".hpp"], "seed_roots": ["seeds"]},
        "compiler": {"command": {"argv": [sys.executable, "recovery/compiler.py"], "timeout": 10},
                     "inputs": ["recovery/compiler.py"], "environment": ["RECORDING_CONTROL"]},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "fixture",
                        "flag_sets": [["first", "/Dvalue=two words"], ["second"]], "layout_flags": [],
                        "unit_extensions": [".cc"], "inputs": [], "input_roots": []}}))
    baseline = commit(tmp_path, "Initial synthetic source")
    configuration(tmp_path, lambda value: value.update(source_history={
        "baseline_revision": baseline, "evidence": ["Starting source checkpoint"]}))
    commit(tmp_path, "Declare history policy")
    return tmp_path


def run(root, *targets, **kwargs):
    return recording.record(root, targets or ("_f", "_h"), image=Path("original/input.exe"), **kwargs)


def calls(root):
    path = root / "build/calls.jsonl"
    return [json.loads(line) for line in path.read_text().splitlines()] if path.exists() else []


def test_installed_cli_measures_and_publishes_a_family_once(project, capsys):
    code = main(["record", "_f", "0x00401001", "--root", str(project), "--image", "original/input.exe"])
    report = json.loads(capsys.readouterr().out)
    assert code == 0 and report["selected"] == len(report["measured"]) == len(report["updated"]) == 2
    assert report["compile_attempts"] == report["translation_units"] == 1 and report["reused_attempts"] == 1
    assert report["verification"] == "historical_masked_bytes_only" and not report["ready_for_collection"]
    assert all(row.byte_exact for row in read_file(project / "engine/family.cc"))
    assert "TRIED: seed observation" in (project / "engine/family.cc").read_text()
    assert calls(project)[0]["flags"] == ["first", "/Dvalue=two words"]
    assert not list((project / "build/recovery/publications").iterdir())


def test_noop_keeps_source_inode_timestamp_and_claims(project):
    run(project)
    source = project / "engine/family.cc"
    before = source.read_bytes(), source.stat().st_ino, source.stat().st_mtime_ns
    code, report = run(project)
    assert code == 0 and report["changed_files"] == report["updated"] == []
    assert before == (source.read_bytes(), source.stat().st_ino, source.stat().st_mtime_ns)
    assert len(calls(project)) == 2  # No object/result reuse across invocations.


@pytest.mark.parametrize("targets", [("_f",), ("_f", "_h")])
def test_even_byte_identical_placeholder_cannot_bank_a_claim(project, targets, capsys):
    source = project / "engine/family.cc"
    source.write_text(source.read_text().replace("void f() {}", "void f() {\n// BODY GOES HERE.\n}"))
    before = source.read_bytes()
    # Both the synthetic image and compiler emit RET for f. Matching that is
    # insufficient while the source still declares the body unfinished.
    code = main(["record", *targets, "--root", str(project), "--image", "original/input.exe"])
    assert code == 3 and "unfinished placeholder" in capsys.readouterr().err
    assert source.read_bytes() == before and not calls(project)
    assert not any(row.byte_exact for row in read_file(source))
    # An intentional empty implementation is allowed; no permanent stub ban.
    source.write_text(source.read_text().replace("// BODY GOES HERE.\n", ""))
    assert run(project, *targets)[0] == 0


def test_regression_keeps_existing_claim_and_notes(project):
    run(project)
    commit(project, "Measured byte obligations")
    source = project / "engine/family.cc"
    before = source.read_bytes()
    compiler(project, [("_f", b"\x33\xc0\xc3"), ("_h", b"\x33\xc0\xc3")])
    code, report = run(project)
    assert code == 1 and report["regressed"][0]["address"] == "0x00401000"
    assert source.read_bytes() == before


def test_unclaimed_miss_does_not_publish(project):
    source = project / "engine/family.cc"
    before = source.read_bytes()
    compiler(project, [("_f", b"\x33\xc0\xc3")])
    code, report = run(project, "_f")
    assert code == 1 and report["updated"] == [] and source.read_bytes() == before


def test_missing_subject_does_not_erase_successful_sibling(project):
    compiler(project, [("_f", b"\xc3")])
    code, report = run(project)
    assert code == 3 and len(report["unresolved"]) == len(report["updated"]) == 1
    assert [row.byte_exact for row in read_file(project / "engine/family.cc")] == [True, False]


def test_compile_failure_is_unknown_and_shared_by_the_unit(project):
    compiler(project, failure=True)
    code, report = run(project)
    assert code == 3 and len(report["unresolved"]) == 2 and report["regressed"] == []
    assert len(calls(project)) == report["compile_attempts"] == 2
    assert report["reused_attempts"] == 2
    assert not any(row.byte_exact for row in read_file(project / "engine/family.cc"))


@pytest.mark.parametrize("targets", [("_f", "0x00401000"), ("_absent",)])
def test_invalid_selection_refuses_before_compiling(project, targets):
    with pytest.raises(ValueError, match="selected more than once|requires one source mapping"):
        run(project, *targets)
    assert not calls(project)


def test_duplicate_seed_requires_exact_host_selection(project):
    (project / "seeds/family.cc").write_text(FIRST)
    with pytest.raises(ValueError, match="select an exact --in path"):
        run(project, "_f")
    assert run(project, "_f", in_file="engine/family.cc")[0] == 0
    assert not read_file(project / "seeds/family.cc")[0].byte_exact


@pytest.mark.parametrize("name", ["f", "add", "face"])
def test_hexadecimal_looking_function_name_is_not_an_address(project, name):
    source = FIRST.replace(" _f ", f" {name} ").replace("\n// TRIED", "\n// symbol _f\n// TRIED")
    (project / "engine/family.cc").write_text(source + SECOND)
    code, report = run(project, name)
    assert code == 0 and report["measured"][0]["address"] == "0x00401000"


def test_unprefixed_address_still_selects_when_no_symbol_has_that_name(project):
    assert run(project, "00401000")[0] == 0


@pytest.mark.parametrize("jobs", [1, 2])
def test_multiple_units_and_source_roots_use_configured_compiler(project, jobs):
    (project / "engine/family.cc").write_text(FIRST)
    (project / "seeds/other.cc").write_text(SECOND)
    code, report = run(project, jobs=jobs)
    assert code == 0 and report["translation_units"] == report["compile_attempts"] == 2
    assert {row["source"] for row in calls(project)} == {"engine/family.cc", "seeds/other.cc"}


@pytest.mark.parametrize("damage", ["wrong", "missing"])
def test_missing_or_wrong_image_refuses_before_compiler(project, damage):
    image = project / "original/input.exe"
    image.unlink() if damage == "missing" else image.write_bytes(b"wrong")
    with pytest.raises((ValueError, OSError)):
        run(project)
    assert not calls(project)


def test_historical_claim_loss_refuses_before_compiler(project):
    run(project)
    commit(project, "Bank obligations")
    (project / "engine/family.cc").write_text(FIRST + SECOND)
    before = len(calls(project))
    with pytest.raises(ValueError, match="historical source obligations are missing"):
        run(project)
    assert len(calls(project)) == before


@pytest.mark.parametrize("effect", [
    "Path('engine/types.hpp').write_text('struct Changed {};')",
    "Path('engine/new.hpp').write_text('new header')",
    "Path('engine/family.cc').write_text(Path('engine/family.cc').read_text()+'// concurrent edit\\n')",
    "Path('original/input.exe').write_bytes(b'changed image')",
    "Path('.gitignore').write_text('')",
])
def test_compiler_side_effects_refuse_publication_without_reverting_edits(project, effect):
    compiler(project, effect=effect)
    with pytest.raises(ValueError):
        run(project)
    assert not any(row.byte_exact for row in read_file(project / "engine/family.cc"))


def test_explicit_compiler_arguments_preserve_tokens(project):
    arguments = ["chosen compiler", "/Ipath with spaces", '/Dquoted="words"']
    (project / "recovery/arguments.json").write_text(json.dumps(arguments))
    assert run(project, arguments_file=Path("recovery/arguments.json"))[0] == 0
    assert calls(project)[0]["arguments"] == arguments


def test_changed_explicit_arguments_refuse_publication(project):
    (project / "recovery/arguments.json").write_text("[]")
    compiler(project, effect="Path('recovery/arguments.json').write_text('[\"different\"]')")
    with pytest.raises(ValueError, match="inputs changed"):
        run(project, arguments_file=Path("recovery/arguments.json"))


@pytest.mark.parametrize("arguments", [{"argv": []}, [1], ["contains\u0000nul"]])
def test_invalid_arguments_refuse_before_compiling(project, arguments):
    (project / "recovery/arguments.json").write_text(json.dumps(arguments))
    with pytest.raises(ValueError):
        run(project, arguments_file=Path("recovery/arguments.json"))
    assert not calls(project)


@pytest.mark.parametrize("damage", ["head", "environment"])
def test_coordinator_input_changes_after_measurement_refuse(project, monkeypatch, damage):
    actual = recording._unit
    def changed(job):
        result = actual(job)
        if damage == "head":
            git(project, "commit", "--allow-empty", "-m", "Concurrent checkpoint")
        else:
            monkeypatch.setenv("RECORDING_CONTROL", "changed")
        return result
    monkeypatch.setattr(recording, "_unit", changed)
    with pytest.raises(ValueError, match="inputs changed"):
        run(project)
    assert not any(row.byte_exact for row in read_file(project / "engine/family.cc"))


@pytest.mark.parametrize("damage", ["unignored", "overlap", "missing-input", "header-host", "outside-host"])
def test_invalid_input_or_publication_policy_refuses(project, damage):
    kwargs = {}
    if damage == "unignored":
        (project / ".gitignore").write_text("")
    elif damage == "overlap":
        configuration(project, lambda value: value["measurement"].update(input_roots=["build"]))
        (project / "build").mkdir()
    elif damage == "missing-input":
        configuration(project, lambda value: value["measurement"].update(inputs=["missing.txt"]))
    elif damage == "header-host":
        (project / "engine/family.cc").rename(project / "engine/family.hpp")
    else:
        kwargs["in_file"] = "../elsewhere.cc"
    with pytest.raises(ValueError):
        run(project, **kwargs)
    assert not calls(project)


def test_no_demotion_or_saved_verdict_input_in_the_cli(project):
    with pytest.raises(SystemExit) as error:
        main(["record", "_f", "--root", str(project), "--image", "original/input.exe", "--demote"])
    assert error.value.code == 2
    assert not calls(project)
