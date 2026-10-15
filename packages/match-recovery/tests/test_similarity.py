from __future__ import annotations

import pytest

from match_recovery.similarity import (
    OpcodeCandidate,
    OpcodeReference,
    StaleSimilarityEvidence,
    normalize_opcodes,
    rank_ownership_groups,
    rank_opcode_sequences,
)


SOURCE = "source:abc123"
IMAGE = "image:def456"


def candidate(instructions=("PUSH", "MOV", "RET")):
    return OpcodeCandidate.create(
        "candidate", instructions, source_fingerprint=SOURCE,
        image_fingerprint=IMAGE, provenance="current candidate build",
    )


def reference(reference_id, instructions, **kwargs):
    return OpcodeReference.create(
        reference_id, instructions, source_fingerprint=kwargs.pop("source_fingerprint", SOURCE),
        image_fingerprint=kwargs.pop("image_fingerprint", IMAGE),
        provenance=kwargs.pop("provenance", "verified native measurement"),
        **kwargs,
    )


def rank(candidate_value, refs, eligible, top_k=10, **kwargs):
    return rank_opcode_sequences(
        candidate_value, refs, eligible_reference_ids=eligible,
        source_fingerprint=SOURCE, image_fingerprint=IMAGE, top_k=top_k,
        **kwargs,
    )


def test_normalizes_opcode_tokens_and_rejects_instruction_lines():
    assert normalize_opcodes((" MOV ", "JNE", "RET ")) == ("mov", "jne", "ret")
    with pytest.raises(ValueError, match="one normalized mnemonic"):
        normalize_opcodes(("00401000: mov eax, ecx",))
    with pytest.raises(TypeError, match="must be text"):
        normalize_opcodes(("mov", None))


def test_requires_explicit_eligibility_and_excludes_untrusted_reference_kinds():
    refs = [
        reference("verified", ("push", "mov", "ret"), verified=True),
        reference("unverified", ("push", "mov", "ret")),
        reference("scaffold", ("push", "mov", "ret"), verified=True, scaffold=True),
        reference("claim", ("push", "mov", "ret"), verified=True,
                  historical_claim_only=True),
    ]
    result = rank(candidate(), refs, ("verified", "unverified", "scaffold", "claim"))
    assert [match.reference_id for match in result.matches] == ["verified"]
    assert result.excluded == (
        ("unverified", "reference is not verified"),
        ("scaffold", "scaffold reference"),
        ("claim", "historical claim without current verification"),
    )
    assert result.advisory_only and not result.equivalence_established
    assert not result.acceptance_established


def test_rejects_stale_candidate_or_selected_reference_evidence():
    stale_candidate = OpcodeCandidate.create(
        "old", ("push",), source_fingerprint="old-source",
        image_fingerprint=IMAGE, provenance="old build",
    )
    with pytest.raises(StaleSimilarityEvidence, match="candidate source fingerprint"):
        rank(stale_candidate, [], ())

    stale_reference = reference("old", ("push", "mov"), verified=True,
                                image_fingerprint="old-image")
    with pytest.raises(StaleSimilarityEvidence, match="reference 'old' image fingerprint"):
        rank(candidate(), [stale_reference], ("old",))


def test_unmatched_and_near_miss_references_remain_in_the_ranked_pool():
    refs = [
        reference("near", ("push", "mov", "nop", "ret"), verified=True),
        reference("unmatched", ("xor", "add"), verified=True),
        reference("exact", ("push", "mov", "ret"), verified=True),
    ]
    result = rank(candidate(), refs, ("near", "unmatched", "exact"), top_k=3)
    assert result.eligible_pool_size == 3
    assert [match.reference_id for match in result.matches] == ["exact", "near", "unmatched"]
    assert result.matches[-1].score == 0.0
    assert result.matches[-1].prefiltered
    assert result.omitted_count == 0

    same_length_disjoint = rank(candidate(("div",)),
                                [reference("cmp", ("cmp",), verified=True)],
                                ("cmp",))
    assert same_length_disjoint.matches[0].score == 0.0
    assert same_length_disjoint.matches[0].prefiltered


def test_top_k_is_stable_and_reports_omitted_candidates_instead_of_hiding_pool_size():
    refs = [
        reference("z", ("push", "ret"), verified=True),
        reference("b", ("push", "ret"), verified=True),
        reference("a", ("xor",), verified=True),
    ]
    result = rank(candidate(("push", "ret")), refs, ("z", "b", "a"), top_k=1)
    assert result.eligible_pool_size == 3
    assert result.returned_count == 1
    assert result.omitted_count == 2
    assert result.matches[0].reference_id == "b"  # tie-break by ID


def test_unknown_selected_id_and_duplicate_reference_ids_refuse():
    with pytest.raises(ValueError, match="absent from pool"):
        rank(candidate(), [], ("missing",))
    duplicate = reference("same", ("push",), verified=True)
    with pytest.raises(ValueError, match="must be unique"):
        rank(candidate(), [duplicate, duplicate], ("same",))


def test_zero_top_k_keeps_accounting_and_empty_sequence_prefilter_is_exact():
    refs = [reference("empty", (), verified=True)]
    ranked = rank(OpcodeCandidate.create(
        "empty-candidate", (), source_fingerprint=SOURCE,
        image_fingerprint=IMAGE, provenance="empty source sequence",
    ), refs, ("empty",), top_k=0)
    assert ranked.eligible_pool_size == 1
    assert ranked.matches == ()
    assert ranked.omitted_count == 1


def test_owner_batch_scores_every_target_then_spreads_groups_and_counts_unmatched():
    candidates = [
        OpcodeCandidate.create("a-hit", ("push", "mov", "ret"),
                               source_fingerprint=SOURCE, image_fingerprint=IMAGE,
                               provenance="candidate build"),
        OpcodeCandidate.create("a-miss", ("xor", "add"),
                               source_fingerprint=SOURCE, image_fingerprint=IMAGE,
                               provenance="candidate build"),
        OpcodeCandidate.create("b-hit", ("push", "mov", "nop", "ret"),
                               source_fingerprint=SOURCE, image_fingerprint=IMAGE,
                               provenance="candidate build"),
        OpcodeCandidate.create("c-miss", ("div",),
                               source_fingerprint=SOURCE, image_fingerprint=IMAGE,
                               provenance="candidate build"),
    ]
    refs = [
        reference("a-ref", ("push", "mov", "ret"), verified=True,
                  ownership_group="owner-a"),
        reference("b-ref", ("push", "mov", "ret"), verified=True,
                  ownership_group="owner-b"),
        reference("unused", ("cmp",), verified=True, ownership_group="other"),
        reference("unselected", ("xor",), verified=True, ownership_group="owner-a"),
    ]
    result = rank_ownership_groups(
        candidates, refs,
        candidate_groups={"a-hit": "owner-a", "a-miss": "owner-a",
                          "b-hit": "owner-b", "c-miss": "owner-c"},
        eligible_reference_ids=("a-ref", "b-ref", "unused"),
        source_fingerprint=SOURCE, image_fingerprint=IMAGE,
        top_k=2, limit=4, spread_groups=True,
    )
    assert result.candidate_pool_size == 4
    assert result.returned_count == 4 and result.omitted_count == 0
    assert [item.candidate_id for item in result.candidates] == [
        "a-hit", "b-hit", "c-miss", "a-miss",
    ]
    assert result.candidates[0].matches[0].reference_id == "a-ref"
    assert result.candidates[1].matches[0].reference_id == "a-ref"
    assert result.candidates[1].matches[0].reference_group == "owner-a"
    assert result.candidates[2].unmatched and result.candidates[3].unmatched
    assert result.group_counts == (("owner-a", 2, 1), ("owner-b", 1, 1), ("owner-c", 1, 0))
    assert result.excluded_reference_count == 1
    assert result.excluded_references == (
        ("unselected", "not explicitly eligible"),
    )
    assert result.advisory_only and not result.equivalence_established


def test_owner_batch_limit_reports_targets_not_emitted():
    candidate_items = [
        OpcodeCandidate.create(item, ("mov", "ret"), source_fingerprint=SOURCE,
                               image_fingerprint=IMAGE, provenance="batch")
        for item in ("first", "second", "third")
    ]
    result = rank_ownership_groups(
        candidate_items, [],
        candidate_groups={item: "no-siblings" for item in ("first", "second", "third")},
        eligible_reference_ids=(), source_fingerprint=SOURCE,
        image_fingerprint=IMAGE, limit=2,
    )
    assert result.candidate_pool_size == 3
    assert result.returned_count == 2 and result.omitted_count == 1
    assert all(item.unmatched and not item.matches for item in result.candidates)


def test_owner_batch_searches_verified_references_across_owner_groups():
    target = OpcodeCandidate.create(
        "owner-b-target", ("push", "mov", "ret"),
        source_fingerprint=SOURCE, image_fingerprint=IMAGE, provenance="candidate",
    )
    source_owner_reference = reference(
        "owner-a-reference", ("push", "mov", "ret"), verified=True,
        ownership_group="owner-a",
    )
    result = rank_ownership_groups(
        [target], [source_owner_reference],
        candidate_groups={"owner-b-target": "owner-b"},
        eligible_reference_ids=("owner-a-reference",),
        source_fingerprint=SOURCE, image_fingerprint=IMAGE,
    )
    assert result.candidates[0].ownership_group == "owner-b"
    assert result.candidates[0].matches[0].reference_group == "owner-a"
    assert result.candidates[0].matches[0].score == 1.0


def test_owner_batch_refuses_stale_eligible_reference_even_if_group_has_no_target():
    target = candidate()
    ref = reference("stale-other-owner", ("push", "ret"), verified=True,
                    image_fingerprint="old-image", ownership_group="other")
    with pytest.raises(StaleSimilarityEvidence, match="reference 'stale-other-owner' image fingerprint"):
        rank_ownership_groups(
            [target], [ref], candidate_groups={"candidate": "owner"},
            eligible_reference_ids=("stale-other-owner",), source_fingerprint=SOURCE,
            image_fingerprint=IMAGE,
        )


def test_tentative_and_provisional_provenance_are_explicit_and_not_verified():
    ref = reference("provisional", ("push", "ret"), verified=False,
                    provisional_provenance=True)
    result = rank(candidate(("push", "ret")), [ref], (), tentative_reference_ids=("provisional",))
    assert result.matches[0].tentative is True
    assert result.matches[0].provisional_provenance is True
    assert result.matches[0].score == 1.0

    bad_ref = reference("bad-tentative", ("push",), verified=True)
    rejected = rank(candidate(), [bad_ref], (), tentative_reference_ids=("bad-tentative",))
    assert rejected.matches == ()
    assert rejected.excluded == (("bad-tentative", "tentative reference must be explicitly unverified"),)


def test_boolean_evidence_flags_are_strict_and_self_reference_is_excluded():
    fake = reference("candidate", ("push", "mov", "ret"), verified=True)
    with pytest.raises(TypeError, match="verified must be bool"):
        bad = OpcodeReference(
            "candidate", fake.opcodes, SOURCE, IMAGE, "not evidence",
            verified="false",  # type: ignore[arg-type]
        )
        rank(candidate(), [bad], ("candidate",))
    self_ref = reference("candidate", ("push", "mov", "ret"), verified=True)
    result = rank(candidate(), [self_ref], ("candidate",))
    assert result.matches == ()
    assert result.excluded == (("candidate", "self reference"),)


def test_length_upper_bound_prunes_only_after_top_k_score_proves_it_cannot_enter():
    refs = [
        reference("exact", ("push", "mov", "ret"), verified=True),
        reference("long", ("push", "mov", "ret", "nop", "nop"), verified=True),
    ]
    result = rank(candidate(), refs, ("exact", "long"), top_k=1)
    assert [match.reference_id for match in result.matches] == ["exact"]
    assert result.pruned_reference_count == 1
    assert result.eligible_pool_size == 2 and result.omitted_count == 1

    tied = [
        reference("z-exact", ("push", "mov", "ret"), verified=True),
        reference("a-exact", ("push", "mov", "ret"), verified=True),
    ]
    tie_result = rank(candidate(), tied, ("z-exact", "a-exact"), top_k=1)
    assert tie_result.matches[0].reference_id == "a-exact"
    assert tie_result.pruned_reference_count == 0


def test_candidate_groups_and_resource_bounds_fail_closed():
    item = candidate()
    with pytest.raises(ValueError, match="map exactly every candidate"):
        rank_ownership_groups(
            [item], [], candidate_groups={}, eligible_reference_ids=(),
            source_fingerprint=SOURCE, image_fingerprint=IMAGE,
        )
    with pytest.raises(ValueError, match=r"top_k \* limit"):
        rank_ownership_groups(
            [item], [], candidate_groups={"candidate": "owner"},
            eligible_reference_ids=(), source_fingerprint=SOURCE,
            image_fingerprint=IMAGE, top_k=100, limit=200,
        )
    with pytest.raises(TypeError, match="bool"):
        rank_ownership_groups(
            [item], [], candidate_groups={"candidate": "owner"},
            eligible_reference_ids=(), source_fingerprint=SOURCE,
            image_fingerprint=IMAGE, spread_groups=1,  # type: ignore[arg-type]
        )


def test_opcode_sequence_limit_is_enforced_during_normalization():
    with pytest.raises(ValueError, match="opcode sequence exceeds"):
        normalize_opcodes("nop" for _ in range(20_001))
