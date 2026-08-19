"""The command line over `decomp`.

Tested through typer's runner rather than a subprocess: the point is what
the command decides, and a subprocess would mostly be testing that Python
starts. The environment's facts are given explicitly, as the tool requires
of every caller.
"""

import json
import sys
from pathlib import Path

import pytest
from typer.testing import CliRunner

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
import osmx                                                  # noqa: E402

runner = CliRunner()

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
