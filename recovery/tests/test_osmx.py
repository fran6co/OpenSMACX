"""Transitional command semantics over the installed source API.

Tested through typer's runner rather than a subprocess: the point is what
the command decides, and a subprocess would mostly be testing that Python
starts. The environment's facts are given explicitly, as the tool requires
of every caller.
"""

import json
import os
import subprocess
import sys
from pathlib import Path

import pytest
import match_recovery.source.asm as source_asm
from typer.testing import CliRunner

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
import osmx                                                  # noqa: E402

runner = CliRunner()
claim_coverage = osmx._claim_coverage


@pytest.fixture(autouse=True)
def isolated_build_database(tmp_path, monkeypatch):
    """Synthetic gate verdicts must never overwrite the real measurement memo.

    The CLI's default database used to point into the application build. A test
    run replaced its warm cache with mocked results for temporary source paths,
    forcing the next real gate to remeasure every file. Explicit test databases
    still override this default; configuration behavior is tested separately.
    """
    database = tmp_path / "synthetic-build/compile_commands.json"
    database.parent.mkdir()
    # Match the legacy command's explicitly chosen borrowing anchor, without
    # consulting the real database or providing an executable compiler.
    database.write_text(json.dumps([{"file": str(osmx.REPO_ROOT / "src/buffer.cpp"),
                                     "directory": str(database.parent),
                                     "arguments": ["not-a-compiler", "/c", str(osmx.REPO_ROOT / "src/buffer.cpp")]}]))
    monkeypatch.setenv("OPENSMACX_COMPILE_COMMANDS", str(database))
    return database


@pytest.fixture(autouse=True)
def project_audits(monkeypatch):
    """Synthetic command tests must not build/audit the actual application.

    The full gate and its damage suite exercise those processes separately.
    Keep their exit codes controllable here so refusal propagation is tested.
    """
    failures = set()
    actual = subprocess.run
    def invoke(argv, *args, **kwargs):
        command = [str(word) for word in argv]
        audit = (len(command) > 1 and command[0] == sys.executable
                 and Path(command[1]).parent == osmx.REPO_ROOT / "tools")
        native_audit = command[:5] == [sys.executable, "-m", "match_recovery.cli", "audits", "check"]
        source_check = command[:4] == [sys.executable, "-m", "match_recovery.cli", "check-source"]
        mapping_check = command[:4] == [sys.executable, "-m", "match_recovery.cli", "check-mappings"]
        build = command[:2] == ["cmake", "--build"]
        if audit or native_audit or source_check or mapping_check or build:
            key = (command[3] if source_check or mapping_check else command[5] if native_audit
                   else Path(command[1]).name if audit else "build")
            output = (json.dumps({"audit": key, "result": "fail" if key in failures else "pass",
                                  "total": 0, "rules": []}) if native_audit else "")
            if native_audit and key == "storage":
                from match_recovery.cpp_storage_audit import STORAGE_RULES
                bad = int(key in failures)
                output = json.dumps({"audit": key, "result": "fail" if bad else "pass", "total": bad,
                                     "files_scanned": 1, "persisted": False,
                                     "rules": [{"name": name, "count": bad if i == 0 else 0, "ceiling": 0,
                                                "status": "growth" if bad and i == 0 else "exact"}
                                               for i, name in enumerate(STORAGE_RULES)]})
            if source_check:
                from match_recovery.source_integrity import RULES
                output = json.dumps({"check": "source_annotation_integrity", "schema_version": 1,
                                     "result": "fail" if key in failures else "pass",
                                     "annotations": 2, "files_scanned": 1, "persisted": False,
                                     "ready_for_collection": False, "input_fingerprint": "a" * 64,
                                     "errors": [], "rules": [{"name": rule, "failures": int(key in failures)}
                                                              for rule in RULES]})
            if mapping_check:
                from match_recovery.mapping_check import RULES
                output = json.dumps({"check": "source_build_mappings", "schema_version": 1,
                                     "result": "fail" if key in failures else "pass",
                                     "annotations": 2, "entities": 2, "build_units": 1,
                                     "seed_alternative_groups": 0, "forwarder_sites": 0,
                                     "persisted": False, "ready_for_collection": False,
                                     "input_fingerprint": "a" * 64, "errors": [],
                                     "rules": [{"name": rule, "failures": int(key in failures)} for rule in RULES]})
            return subprocess.CompletedProcess(argv, int(key in failures), output, "")
        return actual(argv, *args, **kwargs)
    monkeypatch.setattr(osmx.subprocess, "run", invoke)
    return failures


ANNOTATED = """// ORIGINAL: 0x00401000 ?a@@YAXXZ 0x00401000-0x00401040 BYTE_EXACT
void a() {
}
// ORIGINAL: 0x00402000 ?b@@YAXXZ 0x00402000-0x00402010
void b() {
}
// ORIGINAL: 0x00403000 ?c@@YAXXZ 0x00403000-0x00403008 EXCLUDED S1
void c() {
}
"""


@pytest.fixture
def tree(tmp_path):
    (tmp_path / "x.cpp").write_text(ANNOTATED)
    return tmp_path


def run(*args):
    return runner.invoke(osmx.app, list(args))


@pytest.fixture(autouse=True)
def synthetic_claim_coverage(monkeypatch):
    report = {"result": "pass", "missing": [], "claims": 2, "historical_requirements": 2}
    monkeypatch.setattr(osmx, "_claim_coverage", lambda src: report)
    return report


@pytest.mark.parametrize("suffix", [".h", ".hpp", ".cc", ".cxx", ".hh"])
@pytest.mark.parametrize("token", ["BYTE_EXACT", "SEMANTIC"])
def test_legacy_coverage_refuses_claims_outside_its_measurement_walk(tmp_path, monkeypatch, suffix, token):
    source = tmp_path / "src"
    source.mkdir()
    (source / ("claimed" + suffix)).write_text(
        f"// ORIGINAL: 0x00401000 _first 0x00401000-0x00401008 {token}\nint first();\n")
    monkeypatch.setattr(osmx, "REPO_ROOT", tmp_path)
    monkeypatch.setattr(osmx.SourceConfiguration, "read", lambda root: osmx.SourceConfiguration("src/PIN", "pe:va"))
    monkeypatch.setattr(osmx, "check_claims", lambda root: pytest.fail("unsupported coverage must refuse"))
    result = claim_coverage(source)
    assert result["result"] == "unknown"
    assert "does not cover claimed source hosts" in result["error"]


def test_status_counts_each_population(tree):
    result = run("status", "--src", str(tree),
                 "--compile-commands", "/nonexistent", "--json")
    assert result.exit_code == 0, result.output
    rows = json.loads(result.output)["populations"]
    assert rows["CLAIMED"]["count"] == 1
    assert rows["NOT_MATCHING"]["count"] == 1
    assert rows["EXCLUDED"]["count"] == 1
    assert rows["NO_BODY"]["count"] == 0


def test_a_claim_is_not_counted_as_work(tree):
    """`State` says a claimed body is IMPLEMENTED like any other; what
    separates them is the ratchet claim, and that is the whole reason
    `Population` exists beside `State`."""
    result = run("status", "--src", str(tree), "--json")
    rows = json.loads(result.output)["populations"]
    assert rows["CLAIMED"]["count"] == 1
    assert rows["NOT_MATCHING"]["count"] == 1


def test_size_is_the_primary_span(tree):
    result = run("status", "--src", str(tree),
                 "--compile-commands", "/nonexistent", "--json")
    rows = json.loads(result.output)["populations"]
    assert rows["CLAIMED"]["median_bytes"] == 0x40
    assert rows["NOT_MATCHING"]["median_bytes"] == 0x10


def test_an_unreadable_build_database_is_unknown_not_zero(tree):
    """Reporting 0 reachable on a checkout with no configured build would
    read as "nothing can be compiled", which is a different and false
    statement."""
    result = run("status", "--src", str(tree),
                 "--compile-commands", "/nonexistent", "--json")
    assert json.loads(result.output)["populations"]["CLAIMED"]["reachable"] is None
    assert "unknown, not zero" in run(
        "status", "--src", str(tree),
        "--compile-commands", "/nonexistent").output


def test_a_tree_with_no_annotations_fails_rather_than_reporting_zeros(tmp_path):
    """The failure this project can actually have is silence: a reader that
    finds nothing returns `[]` and every count computed from it is zero,
    which looks like an answer."""
    result = run("status", "--src", str(tmp_path))
    assert result.exit_code == 1
    assert "no annotations" in result.output


def test_status_is_a_report_and_not_a_gate(tree):
    """It exits 0 even with 4,045 records of outstanding work; `check` is
    the command that fails."""
    assert run("status", "--src", str(tree)).exit_code == 0


# ----------------------------------------------------------------- show

NAMED = """// ORIGINAL: 0x0045F950 _WinMain@16 0x0045F950-0x0045F958
void a() {
}
// ORIGINAL: 0x005DAC70 ?set_font@Buffer@@QAEHPAVFont@@000@Z 0x005DAC70-0x005DAC78
void b() {
}
// ORIGINAL: 0x00401000 ?init@Alpha@@QAEXXZ 0x00401000-0x00401008
void c() {
}
// ORIGINAL: 0x00402000 ?init@Beta@@QAEXXZ 0x00402000-0x00402008
void d() {
}
// ORIGINAL: 0x00403000 ?init@Gamma@@QAEXXZ 0x00403000-0x00403008
void e() {
}
// ORIGINAL: 0x00404000 ?init@Delta@@QAEXXZ 0x00404000-0x00404008
void f() {
}
// ORIGINAL: 0x00405000 ?init@Epsilon@@QAEXXZ 0x00405000-0x00405008
void g() {
}
// ORIGINAL: 0x00406000 ?close@Alpha@@QAEXXZ 0x00406000-0x00406008
void h() {
}
// ORIGINAL: 0x00406000 ?close@Alpha@@QAEXXZ 0x00406000-0x00406008 FILE
void i() {
}
"""


@pytest.fixture
def named(tmp_path):
    (tmp_path / "n.cpp").write_text(NAMED)
    return tmp_path


def show(tree, target, *extra):
    return run("show", target, "--src", str(tree),
               "--exe", "/nonexistent", *extra)


def test_a_name_finds_it_through_its_decoration(named):
    """`WinMain` is `_WinMain@16` here: the underscore is stdcall
    decoration, so a word-boundary match would miss it entirely."""
    result = show(named, "WinMain", "--json")
    assert result.exit_code == 0, result.output
    assert json.loads(result.output)[0]["name"] == "_WinMain@16"


def test_a_scoped_name_finds_the_member(named):
    result = show(named, "Buffer::set_font", "--json")
    assert json.loads(result.output)[0]["address"] == "0x005DAC70"


def test_an_address_finds_it(named):
    for spelling in ("0x0045F950", "0045F950", "45f950"):
        assert json.loads(show(named, spelling, "--json").output)[0][
            "name"] == "_WinMain@16"


def test_a_short_hex_looking_name_is_read_as_a_name(named):
    """`add`, `face` and `dead` are all valid hex. Winning that argument
    silently is how someone asking about a function gets the bytes at
    0x000ADD instead, so an address takes six digits or an `0x`."""
    assert not osmx._looks_like_an_address("add")
    assert not osmx._looks_like_an_address("face")
    assert osmx._looks_like_an_address("0xadd")
    assert osmx._looks_like_an_address("0045F950")


def test_a_name_many_pieces_share_lists_rather_than_dumps(named):
    """Five classes have an `init`. Choosing one silently would answer a
    question nobody asked; printing five disassemblies answers too much."""
    result = show(named, "init")
    assert result.exit_code == 1
    assert "5 pieces match" in result.output
    for owner in ("Alpha", "Beta", "Gamma", "Delta", "Epsilon"):
        assert f"?init@{owner}@@QAEXXZ" in result.output


def test_an_address_two_annotations_claim_shows_both(named):
    """34 addresses in the real tree are annotated twice, usually a body in
    `src/` and the same piece in the proved store. Both are the answer."""
    result = show(named, "0x00406000")
    assert result.exit_code == 0
    assert "2 annotations match" in result.output
    assert result.output.count("0x00406000") >= 2


def test_an_unmatched_target_fails(named):
    result = show(named, "NotAThing")
    assert result.exit_code == 1
    assert "matches" in result.output


def test_without_an_image_it_still_answers_from_the_map(named):
    """The map is on disk; the image may not be. What the annotation says
    is worth having either way."""
    result = show(named, "WinMain")
    assert result.exit_code == 0
    assert "no image to read" in result.output


# --------------------------------------------------------------- measure


def test_measure_names_exactly_one_piece(named):
    """`measure init` cannot be answered: five pieces match. Exit 2 is a
    usage error, distinct from exit 1, which means "measured, not exact"."""
    result = run("measure", "init", "--src", str(named))
    assert result.exit_code == 2
    assert "measure names one" in result.output


def test_measure_exits_zero_only_for_byte_exact(named, monkeypatch):
    """The contract every other tier is measured against, and what lets
    this be a shell loop condition."""
    from match_recovery.source import Tier
    from match_recovery.source.asm import AsmComparison

    for tier in Tier:
        monkeypatch.setattr(
            osmx, "compare_source",
            lambda *a, tier=tier, **k: AsmComparison(verdict=tier))
        result = run("measure", "WinMain", "--src", str(named))
        expected = 0 if tier is Tier.BYTE_EXACT else 1
        assert result.exit_code == expected, f"{tier} gave {result.exit_code}"


def test_measure_reports_a_compile_failure_rather_than_crashing(named,
                                                                monkeypatch):
    from match_recovery.source.asm import CompileFailed

    def refuse(*args, **kwargs):
        raise CompileFailed("x.cpp(3) : error C2065: 'g_x' : undeclared")

    monkeypatch.setattr(osmx, "compare_source", refuse)
    result = run("measure", "WinMain", "--src", str(named))
    assert result.exit_code == 2
    assert "C2065" in result.output


def test_measure_writes_nothing(named, monkeypatch):
    """`record` is the one that stamps a claim. Keeping them apart is what
    lets an agent be given this and not that."""
    from match_recovery.source import Tier
    from match_recovery.source.asm import AsmComparison
    before = (named / "n.cpp").read_text()
    monkeypatch.setattr(osmx, "compare_source",
                        lambda *a, **k: AsmComparison(verdict=Tier.BYTE_EXACT))
    run("measure", "WinMain", "--src", str(named))
    assert (named / "n.cpp").read_text() == before


# ------------------------------------------------- candidates, not the tree


def a_verdict(tier):
    from match_recovery.source.asm import AsmComparison
    return AsmComparison(verdict=tier)


def test_body_scores_the_file_it_is_given(named, tmp_path, monkeypatch):
    """The whole point: pose the question without editing `src/`."""
    from match_recovery.source import Tier
    candidate = tmp_path / "candidate.cpp"
    candidate.write_text("void f() {}\n")
    asked = []
    monkeypatch.setattr(
        osmx, "compare_source",
        lambda record, exe, source, *a, **k: (
            asked.append(source), a_verdict(Tier.BYTE_EXACT))[1])
    result = run("measure", "WinMain", "--src", str(named),
                 "--body", str(candidate))
    assert result.exit_code == 0, result.output
    assert asked == [candidate]


def test_without_a_candidate_the_tree_is_the_candidate(named, monkeypatch):
    """`measure` with no `--body` must still ask about the committed body,
    or the plain form silently starts answering about something else."""
    from match_recovery.source import Tier
    asked = []
    monkeypatch.setattr(
        osmx, "compare_source",
        lambda record, exe, source, *a, **k: (
            asked.append(source), a_verdict(Tier.BYTE_EXACT))[1])
    run("measure", "WinMain", "--src", str(named))
    assert asked == [named / "n.cpp"]


def test_scoring_a_candidate_writes_nothing(named, tmp_path, monkeypatch):
    from match_recovery.source import Tier
    candidate = tmp_path / "candidate.cpp"
    candidate.write_text("void f() {}\n")
    before = (named / "n.cpp").read_text()
    monkeypatch.setattr(osmx, "compare_source",
                        lambda *a, **k: a_verdict(Tier.BYTE_EXACT))
    run("measure", "WinMain", "--src", str(named), "--body", str(candidate))
    assert (named / "n.cpp").read_text() == before


def test_one_candidate_or_a_directory_of_them_but_not_both(named, tmp_path):
    result = run("measure", "WinMain", "--src", str(named),
                 "--body", str(tmp_path / "a.cpp"), "--dir", str(tmp_path))
    assert result.exit_code == 2
    assert "pass one" in result.output


def test_a_directory_ranks_best_first_against_the_tree(named, tmp_path,
                                                       monkeypatch):
    """"Best" with no baseline is not an answer anyone can act on: what a
    candidate has to beat is what is committed."""
    from match_recovery.source import Tier
    variants = tmp_path / "variants"
    variants.mkdir()
    for name in ("a.cpp", "b.cpp"):
        (variants / name).write_text("void f() {}\n")
    tiers = {"a.cpp": Tier.MISMATCH, "b.cpp": Tier.BYTE_EXACT,
             "n.cpp": Tier.NO_COMPILE}
    monkeypatch.setattr(
        osmx, "compare_source",
        lambda record, exe, source, *a, **k: a_verdict(tiers[source.name]))
    result = run("measure", "WinMain", "--src", str(named),
                 "--dir", str(variants), "--jobs", "1")
    assert result.exit_code == 0, result.output
    # The table rows, which are the ones naming a file. The winner's full
    # verdict is printed under them and repeats its tier.
    order = [line.split()[0] for line in result.output.splitlines()
             if line.startswith("  ") and line.split()[0].isupper()
             and line.rstrip().endswith((".cpp", "<- the tree"))]
    assert order == ["BYTE_EXACT", "MISMATCH", "NO_COMPILE"]
    assert "n.cpp  <- the tree" in result.output


def test_a_directory_with_no_best_exits_one(named, tmp_path, monkeypatch):
    from match_recovery.source import Tier
    variants = tmp_path / "variants"
    variants.mkdir()
    (variants / "a.cpp").write_text("void f() {}\n")
    monkeypatch.setattr(osmx, "compare_source",
                        lambda *a, **k: a_verdict(Tier.MISMATCH))
    result = run("measure", "WinMain", "--src", str(named),
                 "--dir", str(variants), "--jobs", "1")
    assert result.exit_code == 1


def test_an_empty_directory_says_so(named, tmp_path):
    empty = tmp_path / "variants"
    empty.mkdir()
    result = run("measure", "WinMain", "--src", str(named),
                 "--dir", str(empty))
    assert result.exit_code == 2
    assert "no *.cpp" in result.output


def test_a_candidate_that_misses_the_subject_is_a_row_not_a_crash():
    """Half the point of ranking a directory is that some of it is wrong; a
    misnamed subject would otherwise take the other answers down with it."""
    from match_recovery.source import Tier
    import match_recovery.source.asm as asm

    def refuse(*args, **kwargs):
        raise ValueError("?f@@YAXXZ not found among the object's .text")

    original = asm.compare_source
    osmx.compare_source = refuse
    try:
        source, result, note = osmx._score_candidate(
            (Path("a.cpp"), None, None, [], (), frozenset()))
    finally:
        osmx.compare_source = original
    assert result is None
    assert "not found" in note
    assert Tier                      # the import is the point of the name


def test_legacy_cli_has_no_claim_mutation_commands():
    for command in ("record", "semantic"):
        result = run(command, "0x00401000")
        assert result.exit_code != 0 and "No such command" in result.output


# ----------------------------------------------------------------- check


CLAIMS = """// ORIGINAL: 0x00401000 ?a@@YAXXZ 0x00401000-0x00401008 BYTE_EXACT
void a() {
}
// ORIGINAL: 0x00402000 ?b@@YAXXZ 0x00402000-0x00402008 BYTE_EXACT
void b() {
}
// ORIGINAL: 0x00403000 ?c@@YAXXZ 0x00403000-0x00403008
void c() {
}
"""


@pytest.fixture
def claimed(tmp_path):
    (tmp_path / "c.cpp").write_text(CLAIMS)
    return tmp_path


def fake_file_result(verdicts):
    """Stand in for the worker, which needs a compiler."""
    def worker(job):
        _path, records, *_rest = job
        return [(osmx._claim_key(r), verdicts.get(r.address, "BYTE_EXACT"), "")
                for r in records]
    return worker


def check(tree, *extra):
    # `--jobs 1` runs the worker inline: a pool cannot take the local
    # function these tests substitute for it, and a serial run is what any
    # failure here would be debugged with anyway.
    return run("check", "--src", str(tree), "--exe", "/nonexistent",
               "--jobs", "1", *extra)


def test_check_measures_only_claims(claimed, monkeypatch):
    """`?c@@YAXXZ` carries no claim, so the ratchet has nothing to say
    about it - the floor is the number of claims."""
    monkeypatch.setattr(osmx, "_check_one_file", fake_file_result({}))
    result = check(claimed, "--json")
    assert json.loads(result.stdout)["claims"] == 2


def test_synthetic_gate_preserves_the_real_measurement_cache(claimed, monkeypatch, isolated_build_database):
    real_cache = osmx.REPO_ROOT / "build/osmx_check_cache.json"
    before = real_cache.read_bytes() if real_cache.exists() else None
    monkeypatch.setattr(osmx, "_check_one_file", fake_file_result({}))
    result = check(claimed)
    assert result.exit_code == 0, result.output
    memo = json.loads(isolated_build_database.with_name("osmx_check_cache.json").read_text())
    assert set(memo["files"]) == {str(claimed / "c.cpp")}
    assert (real_cache.read_bytes() if real_cache.exists() else None) == before


def test_check_passes_when_every_claim_holds(claimed, monkeypatch):
    monkeypatch.setattr(osmx, "_check_one_file", fake_file_result({}))
    assert check(claimed).exit_code == 0


def test_a_regression_exits_one_and_names_it(claimed, monkeypatch):
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "MISMATCH"}))
    result = check(claimed)
    assert result.exit_code == 1
    assert "REGRESSED 0x00402000" in result.output


def test_an_unverifiable_claim_is_not_a_regression(claimed, monkeypatch):
    """A wall is not a miss. Reporting NO_COMPILE as a regression sends
    someone to look at a body that is fine - measured on the real tree, 6
    regressions against 534 unverifiable."""
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "NO_COMPILE"}))
    result = check(claimed)
    assert result.exit_code == 3, "a different job, so a different code"
    assert "REGRESSED 0x" not in result.output, "nothing is blamed on a body"
    assert "0 REGRESSED" in result.output
    assert "1 NO_COMPILE" in result.output
    assert "INCOMPLETE: unverifiable claims present (not green)" in result.output


def test_no_compile_in_a_build_input_is_a_regression(claimed, monkeypatch,
                                                     tmp_path):
    """A wall someone just built is not a wall that was always there.

    Making `Buffer::dib_` private put `width_` out of reach of `win.cpp`,
    and 28 claims that had been BYTE_EXACT went NO_COMPILE. The gate
    printed "0 REGRESSED" and exited 3 - exactly what it prints when the
    tree is healthy - because the classification could not tell a file
    CMake builds from one it has never built. The build database naming a
    file is the project asserting that it compiles.
    """
    database = tmp_path / "cc.json"
    database.write_text(json.dumps([{
        "directory": str(claimed),
        "file": str(claimed / "c.cpp"),
        "command": "cl /c /I. c.cpp",
    }]))
    monkeypatch.setattr(osmx, "_fresh_compile_commands",
                        lambda *_args, **_kw: None)
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "NO_COMPILE"}))
    result = check(claimed, "--compile-commands", str(database))
    assert result.exit_code == 1, "the tree is broken, not unmeasurable"
    assert "REGRESSED 0x00402000" in result.output


def test_unresolved_in_a_build_input_is_a_regression(claimed, monkeypatch,
                                                     tmp_path):
    """A symbol that stopped being emitted is not an unaskable question.

    The class passes rename methods, declare them virtual and re-point
    `// symbol` facts - exactly the operations that can make a CLAIMED
    symbol vanish from the object. UNRESOLVED took the unaskable branch
    unconditionally, so such a loss folded into the standing unverifiable
    tally and the gate printed its healthy exit 3 over a broken tree.
    Same wall as NO_COMPILE, same rule: in a file CMake builds, it is a
    regression.
    """
    database = tmp_path / "cc.json"
    database.write_text(json.dumps([{
        "directory": str(claimed),
        "file": str(claimed / "c.cpp"),
        "command": "cl /c /I. c.cpp",
    }]))
    monkeypatch.setattr(osmx, "_fresh_compile_commands",
                        lambda *_args, **_kw: None)
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "UNRESOLVED"}))
    result = check(claimed, "--compile-commands", str(database))
    assert result.exit_code == 1, "the claim was lost, not unaskable"
    assert "REGRESSED 0x00402000" in result.output


def test_a_failed_compile_is_not_retried_under_every_flag_set(
        claimed, monkeypatch):
    """The eight sets differ only in /O1|/O2, /Oy- and /Ob0 - all back-end.

    MEASURED: all 992 files carrying a claim were compiled under all eight,
    and the 313 that fail failed with a byte-identical diagnostic every
    time; none disagreed. Those retries were 2,504 of a sweep's 4,831
    compiles, 3.7 of its 12.7 worker-minutes.
    """
    tried = []

    def refuse(path, command, flags):
        tried.append(" ".join(flags))
        raise osmx.CompileFailed("c.cpp(1) : error C2065: 'x' : undeclared")

    monkeypatch.setattr(osmx, "compile_object", refuse)
    monkeypatch.setattr(source_asm, "original_asm", lambda *a, **k: None)
    monkeypatch.setattr(source_asm, "span_refusal", lambda *a, **k: None)
    records = osmx.read(claimed)
    path = next(iter(osmx._claims_by_file(records)))
    osmx._check_one_file((path, osmx._claims_by_file(records)[path],
                          Path("/nonexistent"), ["cl"], osmx.FLAG_SETS, frozenset()))
    assert tried == [osmx.FLAG_SETS[0]], tried


def test_an_internal_compiler_error_is_retried(claimed, monkeypatch):
    """C1001 IS the optimiser falling over, so the flags really can change
    it - which is why the rule needs an exception rather than a comment."""
    tried = []

    def refuse(path, command, flags):
        tried.append(" ".join(flags))
        raise osmx.CompileFailed("fatal error C1001: INTERNAL COMPILER ERROR")

    monkeypatch.setattr(osmx, "compile_object", refuse)
    monkeypatch.setattr(source_asm, "original_asm", lambda *a, **k: None)
    monkeypatch.setattr(source_asm, "span_refusal", lambda *a, **k: None)
    records = osmx.read(claimed)
    path = next(iter(osmx._claims_by_file(records)))
    osmx._check_one_file((path, osmx._claims_by_file(records)[path],
                          Path("/nonexistent"), ["cl"], osmx.FLAG_SETS, frozenset()))
    assert tried == list(osmx.FLAG_SETS), tried


def test_a_regression_outranks_an_unverifiable_claim(claimed, monkeypatch):
    monkeypatch.setattr(osmx, "_check_one_file", fake_file_result(
        {0x00401000: "MISMATCH", 0x00402000: "NO_COMPILE"}))
    assert check(claimed).exit_code == 1


def test_check_writes_nothing(claimed, monkeypatch):
    """What makes it a gate rather than a pass of `record` that fails: one
    that can add a claim moves the floor as a side effect of measuring it."""
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "MISMATCH"}))
    before = (claimed / "c.cpp").read_text()
    check(claimed)
    assert (claimed / "c.cpp").read_text() == before


def test_shared_span_added_in_an_unmeasured_file_invalidates_the_gate_cache(claimed, monkeypatch):
    seen = []
    def measured(job):
        seen.append(job[-1])
        return fake_file_result({})(job)
    monkeypatch.setattr(osmx, "_check_one_file", measured)
    assert check(claimed).exit_code == 0
    assert seen == [frozenset()]
    assert check(claimed).exit_code == 0
    assert len(seen) == 1  # Warm invocation really did reuse the old answer.
    (claimed / "another.cpp").write_text(
        "// ORIGINAL: 0x00404000 ?other@@YAXXZ 0x00401000-0x00401008\nvoid other() {}\n")
    assert check(claimed).exit_code == 0
    assert len(seen) == 2 and (0x00401000, 0x00401008) in seen[-1]


def test_json_is_only_json(claimed, monkeypatch):
    monkeypatch.setattr(osmx, "_check_one_file", fake_file_result({}))
    result = check(claimed, "--json")
    json.loads(result.stdout)
    assert "pin was not verified" in result.stderr


@pytest.mark.parametrize("audit", ["build", "compiler-work", "stale_references.py",
    "marker_symbols.py", "check-mappings", "class-debt", "storage",
    "data_values.py", "check-source", "header_offsets.py"])
def test_failed_project_audit_prevents_acceptance(claimed, monkeypatch, project_audits, audit):
    monkeypatch.setattr(osmx, "_check_one_file", fake_file_result({}))
    project_audits.add(audit)
    assert check(claimed).exit_code == 1


@pytest.mark.parametrize("report", ["", "not JSON", "{}", '{"audit":"wrong"}',
                                    '{"audit":"compiler-work","result":"fail","total":0,"rules":[]}'])
@pytest.mark.parametrize("audit_name", ["compiler-work", "class-debt", "storage"])
def test_invalid_native_audit_report_cannot_pass_the_gate(claimed, monkeypatch, report, audit_name):
    actual = osmx.subprocess.run
    def invoke(argv, *args, **kwargs):
        if list(argv)[:6] == [sys.executable, "-m", "match_recovery.cli", "audits", "check", audit_name]:
            return subprocess.CompletedProcess(argv, 0, report, "")
        return actual(argv, *args, **kwargs)
    monkeypatch.setattr(osmx.subprocess, "run", invoke)
    monkeypatch.setattr(osmx, "_check_one_file", fake_file_result({}))
    result = check(claimed)
    assert result.exit_code == 1
    expected = ("FAILED: the tree does more of the compiler's work" if audit_name == "compiler-work" else
                "FAILED: storage audit" if audit_name == "storage" else "FAILED: semantic debt grew")
    assert expected in result.output


@pytest.mark.parametrize("damage", [
    lambda r: r.update(rules=[]),
    lambda r: r["rules"].pop(),
    lambda r: r["rules"].append(dict(r["rules"][0])),
    lambda r: r["rules"][0].update(name=[]),
    lambda r: r["rules"][0].update(count=-1),
    lambda r: r["rules"][0].update(count=True),
    lambda r: r["rules"][0].update(ceiling=-1),
    lambda r: r["rules"][0].update(status="growth"),
    lambda r: r["rules"][0].update(count=1),
    lambda r: r.update(total=1),
    lambda r: r.update(files_scanned=0),
    lambda r: r.update(files_scanned=True),
    lambda r: r.update(persisted=True),
    lambda r: (r["rules"][0].update(count=1, status="growth"), r.update(total=1)),
])
def test_storage_gate_refuses_incomplete_or_inconsistent_coverage(damage):
    from match_recovery.cpp_storage_audit import STORAGE_RULES
    report = {"audit": "storage", "result": "pass", "total": 0,
              "files_scanned": 1, "persisted": False,
              "rules": [{"name": name, "count": 0, "ceiling": 0, "status": "exact"}
                        for name in STORAGE_RULES]}
    def outcome():
        return osmx.subprocess.CompletedProcess([], 0, json.dumps(report), "")
    assert osmx._native_audit_result(outcome(), "storage", "storage audit", STORAGE_RULES).returncode == 0
    damage(report)
    assert osmx._native_audit_result(outcome(), "storage", "storage audit", STORAGE_RULES).returncode == 3


@pytest.mark.parametrize("state,code", [("fail", 1), ("unknown", 3)])
def test_history_identity_failures_and_unknowns_propagate(claimed, monkeypatch, synthetic_claim_coverage, state, code):
    monkeypatch.setattr(osmx, "_check_one_file", fake_file_result({}))
    synthetic_claim_coverage.update(result=state, error="missing historical configuration",
                                    missing=[{"address": "0x00401000", "dimension": "bytes"}])
    result = check(claimed, "--json")
    assert result.exit_code == code
    output = json.loads(result.stdout)
    assert output["claim_identities"]["result"] == state
    assert output["failed"]["claim_identities"] == (state == "fail")


@pytest.mark.parametrize("damage", ["not-json", "wrong-check", "missing-rule", "duplicate-rule",
                                    "pass-with-errors", "pass-with-failures", "empty-coverage",
                                    "invalid-fingerprint", "exit-disagreement"])
@pytest.mark.parametrize("command", ["check-source", "check-mappings"])
def test_invalid_native_consistency_evidence_fails_the_gate(claimed, monkeypatch, damage, command):
    actual = osmx.subprocess.run

    def invoke(argv, *args, **kwargs):
        outcome = actual(argv, *args, **kwargs)
        if list(argv)[:4] != [sys.executable, "-m", "match_recovery.cli", command]:
            return outcome
        report = json.loads(outcome.stdout)
        if damage == "not-json":
            return subprocess.CompletedProcess(argv, 0, "not JSON", "")
        if damage == "wrong-check":
            report["check"] = "another-check"
        elif damage == "missing-rule":
            report["rules"].pop()
        elif damage == "duplicate-rule":
            report["rules"][-1] = report["rules"][0]
        elif damage == "pass-with-errors":
            report["errors"] = [{"reason": "could not read a file"}]
        elif damage == "pass-with-failures":
            report["rules"][0]["failures"] = 1
        elif damage == "empty-coverage":
            report["annotations"] = 0
        elif damage == "invalid-fingerprint":
            report["input_fingerprint"] = "stale"
        return subprocess.CompletedProcess(argv, 3 if damage == "exit-disagreement" else 0,
                                           json.dumps(report), "")

    monkeypatch.setattr(osmx.subprocess, "run", invoke)
    monkeypatch.setattr(osmx, "_check_one_file", fake_file_result({}))
    assert check(claimed).exit_code == 1


def test_a_tree_with_no_claims_says_so(tmp_path):
    (tmp_path / "c.cpp").write_text(
        "// ORIGINAL: 0x00401000 ?a@@YAXXZ 0x00401000-0x00401008\nvoid a(){}\n")
    result = check(tmp_path)
    assert result.exit_code == 3
    assert "no BYTE_EXACT claims" in result.output


def test_concurrency_is_capped_by_the_wine_prefix():
    """Eight concurrent CL is where this tree measured the knee against the
    one shared prefix; more queue on the wineserver and win nothing."""
    assert osmx.WINE_CEILING == 8


# ------------------------------------------------------------- configure


def test_a_missing_database_is_generated(tmp_path, monkeypatch):
    calls = []
    monkeypatch.setattr(osmx, "_configure",
                        lambda cc, wipe=True: calls.append(("configure", wipe)))
    osmx._fresh_compile_commands(tmp_path / "compile_commands.json")
    assert calls == [("configure", True)]


def test_an_input_newer_than_the_database_reconfigures(tmp_path, monkeypatch):
    """Measuring against a database that describes a build nobody has is the
    quiet kind of wrong: every verdict comes back, about the wrong includes."""
    db = tmp_path / "compile_commands.json"
    db.write_text("[]")
    newer = tmp_path / "CMakeLists.txt"
    newer.write_text("")
    os.utime(newer, (db.stat().st_mtime + 10,) * 2)
    monkeypatch.setattr(osmx, "_configure_inputs", lambda: [newer])
    calls = []
    monkeypatch.setattr(osmx, "_configure",
                        lambda cc, wipe=True: calls.append(cc))
    osmx._fresh_compile_commands(db)
    assert calls == [db]


def test_a_fresh_database_is_left_alone(tmp_path, monkeypatch):
    db = tmp_path / "compile_commands.json"
    db.write_text("[]")
    older = tmp_path / "CMakeLists.txt"
    older.write_text("")
    os.utime(older, (db.stat().st_mtime - 10,) * 2)
    monkeypatch.setattr(osmx, "_configure_inputs", lambda: [older])
    monkeypatch.setattr(osmx, "_configure", lambda *a, **k: pytest.fail(
        "reconfigured a database that was already current"))
    osmx._fresh_compile_commands(db)


def test_reconfigure_forces_it_regardless(tmp_path, monkeypatch):
    db = tmp_path / "compile_commands.json"
    db.write_text("[]")
    monkeypatch.setattr(osmx, "_configure_inputs", list)
    calls = []
    monkeypatch.setattr(osmx, "_configure",
                        lambda cc, wipe=True: calls.append(cc))
    osmx._fresh_compile_commands(db, reconfigure=True)
    assert calls == [db]


def test_the_cache_is_cleared_not_the_build_tree(tmp_path, monkeypatch):
    """Objects survive a reconfigure; only what cmake remembers goes."""
    build = tmp_path / "build"
    (build / "CMakeFiles").mkdir(parents=True)
    (build / "CMakeCache.txt").write_text("stale")
    (build / "CMakeFiles" / "x").write_text("")
    kept = build / "OpenSMACX.exe"
    kept.write_text("an object nobody should have to rebuild")
    db = build / "compile_commands.json"

    def fake_cmake(argv, **kwargs):
        db.write_text("[]")
        return subprocess.CompletedProcess(argv, 0, "", "")

    monkeypatch.setattr(osmx.subprocess, "run", fake_cmake)
    osmx._configure(db)
    assert not (build / "CMakeCache.txt").exists()
    assert not (build / "CMakeFiles").exists()
    assert kept.read_text() == "an object nobody should have to rebuild"


def test_two_annotations_of_one_address_are_two_claims(tmp_path,
                                                        monkeypatch):
    """34 addresses in this tree are annotated twice - the body in `src/`
    and the same piece preserved beside it - and they are measured in two
    translation units, so they can disagree. Keyed by address the second
    result overwrote the first, and the twin it never showed was counted
    among the reproduced."""
    marker = ("// ORIGINAL: 0x00402000 ?f@@YAXXZ 0x00402000-0x00402010 "
              "BYTE_EXACT\nvoid f() {\n}\n")
    (tmp_path / "body.cpp").write_text(marker)
    preserved = tmp_path / "preserved"
    preserved.mkdir()
    (preserved / "00402000.cpp").write_text(marker)

    def worker(job):
        path, records, *_rest = job
        verdict = "MISMATCH" if path.name == "body.cpp" else "BYTE_EXACT"
        return [(osmx._claim_key(r), verdict, "") for r in records]

    monkeypatch.setattr(osmx, "_check_one_file", worker)
    result = check(tmp_path)
    assert result.exit_code == 1
    assert "2 claims" in result.output
    assert "1,009 verified" not in result.output
    assert "1 REGRESSED" in result.output
    assert "body.cpp" in result.output


# ------------------------------------------- a body that lives in a header

HEADER_BODIED = """/*
// ORIGINAL: 0x00404000 ??0Thing@@QAE@XZ 0x00404000-0x00404010
// body      src/thing.h
// kind      game
*/
"""


def test_a_marker_with_no_definition_says_where_the_body_is(tmp_path):
    """27 pieces are written in-class so the image's own inlining
    reproduces. Their marker cannot sit beside them - a header is neither
    globbed nor compiled - so without the fact `show` reports "no closing
    brace within the file", which is true and useless."""
    (tmp_path / "thing.cpp").write_text(HEADER_BODIED)
    result = show(tmp_path, "0x00404000")
    assert result.exit_code == 0, result.output
    assert "body       src/thing.h" in result.output
    assert "no region" not in result.output


def test_the_body_fact_reaches_the_record(tmp_path):
    from match_recovery.source import read
    (tmp_path / "thing.cpp").write_text(HEADER_BODIED)
    record = read(tmp_path)[0]
    assert record.body == "src/thing.h"
    assert record.kind == "game"


def test_a_marker_with_no_body_fact_still_reports_the_missing_region(tmp_path):
    """The fact suppresses the warning; its absence must not."""
    (tmp_path / "thing.cpp").write_text(
        HEADER_BODIED.replace("// body      src/thing.h\n", ""))
    assert "no region" in show(tmp_path, "0x00404000").output


def test_the_most_specific_in_fragment_wins(tmp_path):
    """`--in src/` matched both `src/text.cpp` and
    `src/unrecovered/00608c00.cpp`, so naming one file exactly still refused
    with "2 pieces match"."""
    marker = ("// ORIGINAL: 0x00402000 ?f@@YAXXZ 0x00402000-0x00402010\n"
              "void f() {\n}\n")
    (tmp_path / "body.cpp").write_text(marker)
    kept = tmp_path / "unrecovered"
    kept.mkdir()
    (kept / "00402000.cpp").write_text(marker)
    from match_recovery.source import read
    records = read(tmp_path)
    assert len(osmx._in_file(records, "unrecovered")) == 1
    assert len(osmx._in_file(records, "body.cpp")) == 1
    assert len(osmx._in_file(records, str(tmp_path))) == 2


def test_a_body_fact_that_leads_nowhere_fails_the_gate(tmp_path,
                                                        monkeypatch):
    """The fact has to be checked or it is a comment with a parser: nothing
    about MEASURING the piece depends on it, so the only thing keeping it
    true is this."""
    (tmp_path / "x.cpp").write_text(
        "// ORIGINAL: 0x00402000 ??0Thing@@QAE@XZ 0x00402000-0x00402010 "
        "BYTE_EXACT\n// body      src/gone.h\n")
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "BYTE_EXACT"}))
    result = check(tmp_path)
    assert result.exit_code == 1
    assert "DANGLING" in result.output
    assert "does not exist" in result.output


def test_a_body_fact_whose_file_lost_the_piece_fails_too(tmp_path,
                                                          monkeypatch):
    tree = tmp_path / "src"
    tree.mkdir()
    (tmp_path / "there.h").write_text("class Other { };\n")
    (tree / "x.cpp").write_text(
        "// ORIGINAL: 0x00402000 ??0Thing@@QAE@XZ 0x00402000-0x00402010 "
        "BYTE_EXACT\n// body      ./there.h\n")
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "BYTE_EXACT"}))
    result = check(tree)
    assert result.exit_code == 1
    assert "does not mention Thing" in result.output


def test_a_body_fact_that_resolves_is_silent(tmp_path, monkeypatch):
    tree = tmp_path / "src"
    tree.mkdir()
    (tmp_path / "there.h").write_text("class Thing { Thing() { } };\n")
    (tree / "x.cpp").write_text(
        "// ORIGINAL: 0x00402000 ??0Thing@@QAE@XZ 0x00402000-0x00402010 "
        "BYTE_EXACT\n// body      ./there.h\n")
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "BYTE_EXACT"}))
    result = check(tree)
    assert result.exit_code == 0, result.output
    assert "DANGLING" not in result.output


CTORS = """// ORIGINAL: 0x00407000 ??0Thing@@QAE@XZ 0x00407000-0x00407008
void a() {
}
// ORIGINAL: 0x00407100 ??1Thing@@QAE@XZ 0x00407100-0x00407108
void b() {
}
// ORIGINAL: 0x00407200 ?init@Thing@@QAEXXZ 0x00407200-0x00407208
void c() {
}
"""


def test_a_scoped_name_finds_a_constructor_and_a_destructor(tmp_path):
    """`Text::Text` and `Text::~Text` mangle to `??0Text@@` and `??1Text@@`,
    with no member name at all - the two spellings a person is most likely
    to type for a header-defined piece found nothing."""
    (tmp_path / "x.cpp").write_text(CTORS)
    from match_recovery.source import read
    records = read(tmp_path)
    assert [r.address for r in osmx._matching(records, "Thing::Thing")] == [
        0x00407000]
    assert [r.address for r in osmx._matching(records, "Thing::~Thing")] == [
        0x00407100]
    assert [r.address for r in osmx._matching(records, "Thing::init")] == [
        0x00407200]


def test_an_overload_is_named_by_its_address_or_its_full_name(tmp_path):
    """`--in` separates a piece annotated in two FILES; it cannot separate
    two overloads that share one. `prefs_get` is two functions on lines 1326
    and 1439 of src/alpha.cpp, so offering `--in` there sends someone
    somewhere with no answer in it."""
    (tmp_path / "x.cpp").write_text(
        "// ORIGINAL: 0x00408000 ?f@@YAHH@Z 0x00408000-0x00408008\n"
        "void a() {\n}\n"
        "// ORIGINAL: 0x00409000 ?f@@YAHPAD@Z 0x00409000-0x00409008\n"
        "void b() {\n}\n")
    result = run("show", "f", "--src", str(tmp_path), "--exe", "/nonexistent")
    assert result.exit_code == 0
    assert "2 annotations match" in result.output
    result = run("measure", "f", "--src", str(tmp_path))
    assert result.exit_code == 2
    assert "name one of these exactly, or its address" in result.output
    assert "--in <path fragment>" not in result.output
    assert "0x00408000" in result.output and "0x00409000" in result.output


def test_one_address_annotated_twice_in_one_file_fails_the_gate(tmp_path,
                                                                monkeypatch):
    """Two markers in one unit describe one piece twice, so they compile to
    the same subject and can only agree by luck: twenty were found here and
    seventeen disagreed about whether the body reproduced. The same address
    in two FILES is the deliberate body-and-preserved-copy pattern and must
    stay allowed."""
    (tmp_path / "x.cpp").write_text(
        "// ORIGINAL: 0x00402000 ?f@@YAXXZ 0x00402000-0x00402010 BYTE_EXACT\n"
        "void f() {\n}\n"
        "// ORIGINAL: 0x00402000 ?f@@YAXXZ 0x00402000-0x00402010\n"
        "void g() {\n}\n")
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "BYTE_EXACT"}))
    result = check(tmp_path)
    assert result.exit_code == 1
    assert "DUPLICATE 0x00402000" in result.output
    assert "also annotated at line 1" in result.output


def test_the_same_address_in_two_files_is_allowed(tmp_path, monkeypatch):
    """The body in `src/` and the piece preserved beside it. Only ONE of
    them claims here: two CLAIMS on one address is the artifact being a
    copy, which `redundant_artifacts` refuses."""
    marker = "// ORIGINAL: 0x00402000 ?f@@YAXXZ 0x00402000-0x00402010"
    (tmp_path / "x.cpp").write_text(marker + " BYTE_EXACT\nvoid f() {\n}\n")
    kept = tmp_path / "unrecovered"
    kept.mkdir()
    (kept / "00402000.cpp").write_text(marker + "\nvoid f() {\n}\n")
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "BYTE_EXACT"}))
    result = check(tmp_path)
    assert result.exit_code == 0, result.output
    assert "DUPLICATE" not in result.output


def test_a_subject_one_flag_set_does_not_emit_stays_outstanding(tmp_path,
                                                                 monkeypatch):
    """`check` keeps its own copy of the flag-set loop for the per-file
    compile, and only `match_recovery.source.asm` learned this: an `inline` function is
    folded under /O2 and emitted under /Ob0, so giving up on the first set
    took `say_num` from BYTE_EXACT to unverifiable between two sweeps."""
    from match_recovery.source import read
    (tmp_path / "x.cpp").write_text(
        "// ORIGINAL: 0x00402000 ?f@@YAXXZ 0x00402000-0x00402010 BYTE_EXACT\n"
        "void f() {\n}\n")
    records = read(tmp_path)
    monkeypatch.setattr(osmx, "compile_object", lambda *a, **k: b"object")
    monkeypatch.setattr(source_asm, "original_asm", lambda *a, **k: None)
    monkeypatch.setattr(source_asm, "span_refusal", lambda *a, **k: None)
    tries = iter([ValueError("not found among the object's .text symbols"),
                  "second set emits it"])
    def maybe(obj, record, flags):
        got = next(tries)
        if isinstance(got, ValueError):
            raise got
        return got
    monkeypatch.setattr(source_asm, "subject_asm", maybe)
    from match_recovery.source import Tier
    from match_recovery.source.asm import AsmComparison
    monkeypatch.setattr(source_asm, "compare_subject",
                        lambda *a, **k: AsmComparison(verdict=Tier.BYTE_EXACT))
    out = osmx._check_one_file((tmp_path / "x.cpp", records, "exe", ["cl"],
                                ("/O2", "/O2 /Ob0"), frozenset()))
    assert [v for _k, v, _n in out] == ["BYTE_EXACT"]


def test_an_artifact_claiming_what_product_source_claims_is_redundant(
        tmp_path, monkeypatch):
    """A preserved unit proves a body reproduces SOMEWHERE; once the shipped
    source reproduces the same address, that proof is a copy. Eighteen
    accumulated in one session, each created by fixing a body and forgetting
    its artifact."""
    marker = "// ORIGINAL: 0x00402000 ?f@@YAXXZ 0x00402000-0x00402010"
    (tmp_path / "body.cpp").write_text(marker + " BYTE_EXACT\nvoid f() {\n}\n")
    kept = tmp_path / "recovered"
    kept.mkdir()
    (kept / "00402000.cpp").write_text(marker + " BYTE_EXACT\nvoid f() {\n}\n")
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "BYTE_EXACT"}))
    result = check(tmp_path)
    assert result.exit_code == 1
    assert "the artifact is a copy" in result.output


def test_an_artifact_whose_twin_does_not_reproduce_stays(tmp_path,
                                                          monkeypatch):
    """That one is the only evidence there is - which is why the test is on
    the CLAIM and not on the address."""
    marker = "// ORIGINAL: 0x00402000 ?f@@YAXXZ 0x00402000-0x00402010"
    (tmp_path / "body.cpp").write_text(marker + "\nvoid f() {\n}\n")
    kept = tmp_path / "recovered"
    kept.mkdir()
    (kept / "00402000.cpp").write_text(marker + " BYTE_EXACT\nvoid f() {\n}\n")
    monkeypatch.setattr(osmx, "_check_one_file",
                        fake_file_result({0x00402000: "BYTE_EXACT"}))
    result = check(tmp_path)
    assert result.exit_code == 0, result.output
    assert "copy" not in result.output
