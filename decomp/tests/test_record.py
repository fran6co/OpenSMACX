"""What a measurement does to an annotation.

The environment's facts live in the tests that need them; these need none -
`stamped` is pure, and `record_match` is exercised against a stubbed
measurement so the rule can be tested without a compiler.
"""

from dataclasses import replace
from pathlib import Path

import pytest

from decomp import Tier, read_file
from decomp.asm import AsmComparison
from decomp.record import record_match, stamped

MARKED = """// ORIGINAL: 0x00401000 ?f@C@@QAEXXZ 0x00401000-0x00401008
// LEVER: jl/jge  flipped the condition and swapped the arms
void C::f() {
}
"""
MATCHED = MARKED.replace("0x00401000-0x00401008",
                         "0x00401000-0x00401008 BYTE_EXACT")


def record_in(tmp_path, text):
    path = tmp_path / "x.cpp"
    path.write_text(text)
    return read_file(path)[0]


def verdict(tier):
    return AsmComparison(verdict=tier)


def test_a_proof_sets_the_claim_and_refutes_the_ruled_out(tmp_path):
    record = replace(record_in(tmp_path, MARKED),
                     ruled_out=("plain immediate", "a 1-bit bitfield"))
    after = stamped(record, verdict(Tier.BYTE_EXACT))
    assert after.byte_exact
    # the match refutes every spelling that was recorded as not working
    assert after.ruled_out == ()
    assert after.levers == record.levers


def test_an_upgrade_drops_the_weaker_semantic_claim(tmp_path):
    """BYTE_EXACT subsumes SEMANTIC, so gaining it must clear the old token.

    Without this, recording a SEMANTIC body that has become byte-exact
    writes `BYTE_EXACT SEMANTIC` onto one marker and the reader reports
    both flags true for one body. Measured on 0x005F2330 (2026-08-25),
    the tree's only such marker - the first SEMANTIC claim to be upgraded
    since the writer was rewritten, which is why nothing had caught it.
    """
    record = replace(record_in(tmp_path, MARKED), semantic=True)
    after = stamped(record, verdict(Tier.BYTE_EXACT))
    assert after.byte_exact
    assert not after.semantic


def test_a_body_that_stops_matching_keeps_its_claim_for_a_human(tmp_path):
    """A claim that outlives its body is bad; a claim that DISAPPEARS is
    worse, because the gate then has nothing to fail on. It is kept, and
    `demote` is how someone says they have looked at it."""
    record = record_in(tmp_path, MATCHED)
    assert record.byte_exact
    assert stamped(record, verdict(Tier.MISMATCH)).byte_exact
    assert not stamped(record, verdict(Tier.MISMATCH), demote=True).byte_exact


def test_a_lever_that_stops_working_becomes_a_tried(tmp_path):
    """Each lesson is legal on exactly one kind of body: LEVER on one that
    matches, TRIED on one that does not. The prose stays true either
    way - it just stopped being the thing that worked.

    THE KEY TRAVELS WITH THE PROSE. A LEVER is `KEY prose` and a TRIED is one
    string, so migrating one to the other must re-join them: dropping the key
    ATE THE FIRST WORD of every migrated line, turning `jl/jge flipped the
    condition` into `flipped the condition` - a note that no longer says which
    divergence it addresses, which is the only part the next agent greps for.
    This test asserted the lossy form and so could not see it."""
    record = record_in(tmp_path, MATCHED)
    assert record.levers
    after = stamped(record, verdict(Tier.SHAPE_EXACT))
    assert after.levers == ()
    assert ("jl/jge flipped the condition and swapped the arms"
            in after.ruled_out)


@pytest.mark.parametrize("tier", [t for t in Tier if t is not Tier.BYTE_EXACT])
def test_only_byte_exact_proves_a_claim(tmp_path, tier):
    """NO_COMPILE, SHARED_TAIL and REFUSED are not proofs either - so none
    of them SETS a claim on a body that had none."""
    assert not stamped(record_in(tmp_path, MARKED), verdict(tier)).byte_exact


@pytest.mark.parametrize("tier", [t for t in Tier if t is not Tier.BYTE_EXACT])
def test_no_tier_lowers_a_claim_on_its_own(tmp_path, tier):
    """Not even REFUSED: a span that cannot be scored today says nothing
    about a proof made when it could be, and lowering the floor on it would
    leave the gate with nothing to check."""
    kept = stamped(record_in(tmp_path, MATCHED), verdict(tier))
    assert kept.byte_exact
    assert not stamped(record_in(tmp_path, MATCHED), verdict(tier),
                       demote=True).byte_exact


def test_record_match_writes_the_claim_it_measured(tmp_path, monkeypatch):
    import decomp.record as module
    record = record_in(tmp_path, MARKED)
    monkeypatch.setattr(module, "compare_record",
                        lambda *a, **k: verdict(Tier.BYTE_EXACT))
    assert record_match(record, "exe", ["cl"], ("/c /O2",)).verdict == "BYTE_EXACT"
    assert "BYTE_EXACT" in (tmp_path / "x.cpp").read_text()
    assert read_file(tmp_path / "x.cpp")[0].byte_exact


def test_record_match_touches_nothing_when_nothing_changed(tmp_path,
                                                           monkeypatch):
    """A run over a tree that has not moved must not rewrite it."""
    import decomp.record as module
    record = record_in(tmp_path, MATCHED)
    before = (tmp_path / "x.cpp").stat().st_mtime_ns
    monkeypatch.setattr(module, "compare_record",
                        lambda *a, **k: verdict(Tier.BYTE_EXACT))
    record_match(record, "exe", ["cl"], ("/c /O2",))
    assert (tmp_path / "x.cpp").stat().st_mtime_ns == before


def test_a_claim_that_stopped_reproducing_is_kept_for_a_human(tmp_path,
                                                              monkeypatch):
    """The floor is the number of claims. Clearing one lowers the floor, and
    the gate then passes because there is nothing left to check - which is
    how a ratchet quietly stops being one. `docs/DECOMP_MAP.md`: either a
    tooling change or a lost scaffolding, and both need a human.
    """
    import decomp.record as module
    record = record_in(tmp_path, MATCHED)
    monkeypatch.setattr(module, "compare_record",
                        lambda *a, **k: verdict(Tier.MISMATCH))
    record_match(record, "exe", ["cl"], ("/c /O2",))
    assert "BYTE_EXACT" in (tmp_path / "x.cpp").read_text()


def test_the_lesson_still_moves_when_the_claim_is_kept(tmp_path, monkeypatch):
    """A `LEVER` the measurement contradicts is wrong whether or not anyone
    has decided what to do about the claim; lessons are not a floor."""
    import decomp.record as module
    record = record_in(tmp_path, MATCHED)
    monkeypatch.setattr(module, "compare_record",
                        lambda *a, **k: verdict(Tier.MISMATCH))
    record_match(record, "exe", ["cl"], ("/c /O2",))
    after = read_file(tmp_path / "x.cpp")[0]
    assert after.byte_exact
    assert after.levers == ()
    # The key travels with the prose - see the migration test above.
    assert ("jl/jge flipped the condition and swapped the arms"
            in after.ruled_out)


def test_demote_is_how_a_caller_says_it_has_looked(tmp_path, monkeypatch):
    import decomp.record as module
    record = record_in(tmp_path, MATCHED)
    monkeypatch.setattr(module, "compare_record",
                        lambda *a, **k: verdict(Tier.MISMATCH))
    record_match(record, "exe", ["cl"], ("/c /O2",), demote=True)
    assert "BYTE_EXACT" not in (tmp_path / "x.cpp").read_text()


def test_a_claim_gained_is_written_without_asking(tmp_path, monkeypatch):
    """Only losing one needs a decision; proving one is the point."""
    import decomp.record as module
    record = record_in(tmp_path, MARKED)
    monkeypatch.setattr(module, "compare_record",
                        lambda *a, **k: verdict(Tier.BYTE_EXACT))
    record_match(record, "exe", ["cl"], ("/c /O2",))
    assert read_file(tmp_path / "x.cpp")[0].byte_exact
