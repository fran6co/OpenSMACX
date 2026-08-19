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


def test_a_body_that_stops_matching_loses_its_claim(tmp_path):
    """A claim that outlives its body is worse than no claim: every later
    run trusts it."""
    record = record_in(tmp_path, MATCHED)
    assert record.byte_exact
    after = stamped(record, verdict(Tier.MISMATCH))
    assert not after.byte_exact


def test_a_lever_that_stops_working_becomes_a_ruled_out(tmp_path):
    """Each lesson is legal on exactly one kind of body: LEVER on one that
    matches, RULED-OUT on one that does not. The prose stays true either
    way - it just stopped being the thing that worked."""
    record = record_in(tmp_path, MATCHED)
    assert record.levers
    after = stamped(record, verdict(Tier.SHAPE_EXACT))
    assert after.levers == ()
    assert "flipped the condition and swapped the arms" in after.ruled_out


@pytest.mark.parametrize("tier", [t for t in Tier if t is not Tier.BYTE_EXACT])
def test_only_byte_exact_is_a_claim(tmp_path, tier):
    """NO_COMPILE, SHARED_TAIL and REFUSED are not proofs either."""
    assert not stamped(record_in(tmp_path, MATCHED), verdict(tier)).byte_exact


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


def test_record_match_removes_a_claim_that_stopped_reproducing(tmp_path,
                                                               monkeypatch):
    import decomp.record as module
    record = record_in(tmp_path, MATCHED)
    monkeypatch.setattr(module, "compare_record",
                        lambda *a, **k: verdict(Tier.MISMATCH))
    record_match(record, "exe", ["cl"], ("/c /O2",))
    assert "BYTE_EXACT" not in (tmp_path / "x.cpp").read_text()
