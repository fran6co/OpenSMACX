"""Pure historical-claim transitions; publishing is tested at the recorder."""

from dataclasses import replace
from pathlib import Path

import pytest

from match_recovery.source import Tier, read_text
from match_recovery.source.record import stamped


@pytest.fixture
def record():
    return read_text(
        "// ORIGINAL: 0x00401000 f 0x00401000-0x00401008\n"
        "// LEVER: jl/jge  flipped the condition and swapped the arms\n"
        "void f() {}\n", Path("unit.cpp"))[0]


def test_exact_transition_sets_claim_and_removes_superseded_state(record):
    before = replace(record, semantic=True, ruled_out=("plain immediate", "bitfield"))
    after = stamped(before, Tier.BYTE_EXACT)
    assert after.byte_exact and not after.semantic
    assert after.ruled_out == ()
    assert after.levers == before.levers
    assert not before.byte_exact


@pytest.mark.parametrize("tier", [t for t in Tier if t is not Tier.BYTE_EXACT])
def test_no_other_tier_sets_a_claim_or_erases_partial_progress(record, tier):
    after = stamped(record, tier)
    assert not after.byte_exact
    assert after.levers == record.levers


@pytest.mark.parametrize("tier", [t for t in Tier if t is not Tier.BYTE_EXACT])
def test_no_tier_lowers_a_claim_without_an_explicit_decision(record, tier):
    before = replace(record, byte_exact=True)
    kept = stamped(before, tier)
    assert kept.byte_exact
    assert not kept.levers
    assert kept.ruled_out == ("jl/jge flipped the condition and swapped the arms",)
    down = stamped(before, tier, demote=True)
    assert not down.byte_exact
    assert down.ruled_out == kept.ruled_out


@pytest.mark.parametrize("tier,demote", [("BYTE_EXACT", False), (None, False),
                                          (Tier.MISMATCH, 1), (Tier.MISMATCH, "yes")])
def test_malformed_transition_is_not_a_measurement(record, tier, demote):
    with pytest.raises(ValueError):
        stamped(record, tier, demote)
