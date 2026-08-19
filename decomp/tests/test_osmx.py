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
