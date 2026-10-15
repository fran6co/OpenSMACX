"""Advisory nearest-neighbour ranking for opcode sequences.

This module does not measure equivalence. Callers must supply the reference IDs
whose measurement evidence they have independently verified, and fingerprints
must bind both sides to the same source and image snapshots.
"""

from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from difflib import SequenceMatcher
import re
from typing import Iterable, Mapping, Sequence


_OPCODE = re.compile(r"^[a-z][a-z0-9_.]*$")
MAX_OPCODE_SEQUENCE = 20_000
MAX_REFERENCE_POOL = 10_000
MAX_CANDIDATE_POOL = 10_000
MAX_TOP_K = 100
MAX_OUTPUT_MATCHES = 10_000
MAX_BATCH_COMPARISONS = 100_000


class StaleSimilarityEvidence(ValueError):
    """Raised when a selected reference does not match the requested snapshot."""


@dataclass(frozen=True)
class OpcodeCandidate:
    """A sequence and the provisional evidence identity it was derived from."""

    candidate_id: str
    opcodes: tuple[str, ...]
    source_fingerprint: str
    image_fingerprint: str
    provenance: str

    @classmethod
    def create(
        cls,
        candidate_id: str,
        instructions: Iterable[str],
        *,
        source_fingerprint: str,
        image_fingerprint: str,
        provenance: str,
    ) -> "OpcodeCandidate":
        return cls(candidate_id, normalize_opcodes(instructions),
                   source_fingerprint, image_fingerprint, provenance)


@dataclass(frozen=True)
class OpcodeReference:
    """A possible neighbor; metadata is descriptive, never eligibility by itself."""

    reference_id: str
    opcodes: tuple[str, ...]
    source_fingerprint: str
    image_fingerprint: str
    provenance: str
    verified: bool = False
    scaffold: bool = False
    historical_claim_only: bool = False
    provisional_provenance: bool = False
    ownership_group: str | None = None

    @classmethod
    def create(
        cls,
        reference_id: str,
        instructions: Iterable[str],
        *,
        source_fingerprint: str,
        image_fingerprint: str,
        provenance: str,
        verified: bool = False,
        scaffold: bool = False,
        historical_claim_only: bool = False,
        provisional_provenance: bool = False,
        ownership_group: str | None = None,
    ) -> "OpcodeReference":
        return cls(reference_id, normalize_opcodes(instructions),
                   source_fingerprint, image_fingerprint, provenance,
                   verified, scaffold, historical_claim_only,
                   provisional_provenance, ownership_group)


@dataclass(frozen=True)
class SimilarityMatch:
    reference_id: str
    reference_group: str | None
    score: float
    sequence_ratio: float
    length_ratio: float
    opcode_jaccard: float
    provenance: str
    prefiltered: bool
    provisional_provenance: bool
    tentative: bool


@dataclass(frozen=True)
class SimilarityRanking:
    candidate_id: str
    candidate_provenance: str
    matches: tuple[SimilarityMatch, ...]
    eligible_pool_size: int
    returned_count: int
    omitted_count: int
    excluded: tuple[tuple[str, str], ...]
    pruned_reference_count: int = 0
    advisory_only: bool = True
    equivalence_established: bool = False
    acceptance_established: bool = False


@dataclass(frozen=True)
class OwnershipCandidateRanking:
    candidate_id: str
    ownership_group: str
    best_score: float
    matches: tuple[SimilarityMatch, ...]
    unmatched: bool


@dataclass(frozen=True)
class OwnershipGroupRanking:
    candidates: tuple[OwnershipCandidateRanking, ...]
    candidate_pool_size: int
    returned_count: int
    omitted_count: int
    group_counts: tuple[tuple[str, int, int], ...]
    excluded_reference_count: int
    excluded_references: tuple[tuple[str, str], ...]
    advisory_only: bool = True
    equivalence_established: bool = False
    acceptance_established: bool = False


def normalize_opcodes(instructions: Iterable[str]) -> tuple[str, ...]:
    """Normalize caller-provided mnemonic tokens without parsing disassembly.

    Each input item is one opcode token (not an instruction line). Case and
    surrounding whitespace are normalized; malformed or multi-token entries
    are rejected instead of being guessed at.
    """
    result = []
    for index, instruction in enumerate(instructions):
        if index >= MAX_OPCODE_SEQUENCE:
            raise ValueError(f"opcode sequence exceeds the {MAX_OPCODE_SEQUENCE}-opcode limit")
        if not isinstance(instruction, str):
            raise TypeError(f"opcode {index} must be text")
        opcode = instruction.strip().lower()
        if not _OPCODE.fullmatch(opcode):
            raise ValueError(f"opcode {index} is not one normalized mnemonic: {instruction!r}")
        result.append(opcode)
    return tuple(result)


def _nonempty_identity(value: str, label: str) -> None:
    if not isinstance(value, str) or not value.strip():
        raise ValueError(f"{label} must be a non-empty string")


def _validate_sequence(opcodes: tuple[str, ...], label: str) -> None:
    normalized = normalize_opcodes(opcodes)
    if normalized != opcodes:
        raise ValueError(f"{label} must be created from normalized opcode tokens")
    if len(opcodes) > MAX_OPCODE_SEQUENCE:
        raise ValueError(f"{label} exceeds the {MAX_OPCODE_SEQUENCE}-opcode limit")


def _validate_reference_flags(reference: OpcodeReference) -> None:
    for name in ("verified", "scaffold", "historical_claim_only",
                 "provisional_provenance"):
        if type(getattr(reference, name)) is not bool:
            raise TypeError(f"reference {reference.reference_id!r} {name} must be bool")


def rank_opcode_sequences(
    candidate: OpcodeCandidate,
    references: Sequence[OpcodeReference],
    *,
    eligible_reference_ids: Iterable[str],
    tentative_reference_ids: Iterable[str] = (),
    source_fingerprint: str,
    image_fingerprint: str,
    top_k: int = 10,
) -> SimilarityRanking:
    """Rank eligible references by deterministic opcode-sequence similarity.

    ``eligible_reference_ids`` is an explicit caller assertion based on
    verified evidence, not inferred from a BYTE_EXACT label or provenance
    string. The additional reference flags are fail-closed guards. Selected
    references with stale fingerprints refuse the whole ranking. Every eligible
    reference is considered for top-k using a safe length-ratio upper bound;
    SequenceMatcher is skipped only when the established kth score proves that
    reference cannot enter the result. Disjoint opcode sets also have exact
    zero similarity. Pool and omitted counts include these safely pruned rows.
    """
    if type(top_k) is not int or top_k < 0 or top_k > MAX_TOP_K:
        raise ValueError(f"top_k must be between 0 and {MAX_TOP_K}")
    for value, label in (
        (candidate.candidate_id, "candidate_id"),
        (candidate.provenance, "candidate provenance"),
        (candidate.source_fingerprint, "candidate source fingerprint"),
        (candidate.image_fingerprint, "candidate image fingerprint"),
        (source_fingerprint, "source_fingerprint"),
        (image_fingerprint, "image_fingerprint"),
    ):
        _nonempty_identity(value, label)
    _validate_sequence(candidate.opcodes, "candidate opcode sequence")
    if len(references) > MAX_REFERENCE_POOL:
        raise ValueError(f"reference pool exceeds {MAX_REFERENCE_POOL} references")
    _nonempty_identity(source_fingerprint, "source_fingerprint")
    _nonempty_identity(image_fingerprint, "image_fingerprint")
    if candidate.source_fingerprint != source_fingerprint:
        raise StaleSimilarityEvidence("candidate source fingerprint is stale")
    if candidate.image_fingerprint != image_fingerprint:
        raise StaleSimilarityEvidence("candidate image fingerprint is stale")

    ids = [reference.reference_id for reference in references]
    for reference_id in ids:
        _nonempty_identity(reference_id, "reference_id")
    if len(ids) != len(set(ids)):
        raise ValueError("reference IDs must be unique")
    by_id = {reference.reference_id: reference for reference in references}
    selected_ids = tuple(eligible_reference_ids)
    tentative_ids = tuple(tentative_reference_ids)
    if (len(selected_ids) != len(set(selected_ids)) or
            len(tentative_ids) != len(set(tentative_ids))):
        raise ValueError("reference ID selections must be unique")
    if set(selected_ids) & set(tentative_ids):
        raise ValueError("a reference cannot be both verified-eligible and tentative")
    missing = [reference_id for reference_id in (*selected_ids, *tentative_ids)
               if reference_id not in by_id]
    if missing:
        raise ValueError(f"eligible reference IDs are absent from pool: {missing!r}")

    excluded: list[tuple[str, str]] = []
    usable: list[OpcodeReference] = []
    selected = set(selected_ids) | set(tentative_ids)
    tentative = set(tentative_ids)
    for reference in references:
        reference_id = reference.reference_id
        _validate_reference_flags(reference)
        if reference_id not in selected:
            excluded.append((reference_id, "not explicitly eligible"))
            continue
        if reference_id == candidate.candidate_id:
            excluded.append((reference_id, "self reference"))
            continue
        if reference.scaffold:
            excluded.append((reference_id, "scaffold reference"))
            continue
        if reference.historical_claim_only:
            excluded.append((reference_id, "historical claim without current verification"))
            continue
        if reference_id in selected_ids and reference.verified is not True:
            excluded.append((reference_id, "reference is not verified"))
            continue
        if reference_id in tentative and reference.verified is not False:
            excluded.append((reference_id, "tentative reference must be explicitly unverified"))
            continue
        _nonempty_identity(reference.provenance, f"reference {reference_id!r} provenance")
        _nonempty_identity(reference.source_fingerprint,
                           f"reference {reference_id!r} source fingerprint")
        _nonempty_identity(reference.image_fingerprint,
                           f"reference {reference_id!r} image fingerprint")
        if reference.source_fingerprint != source_fingerprint:
            raise StaleSimilarityEvidence(f"reference {reference_id!r} source fingerprint is stale")
        if reference.image_fingerprint != image_fingerprint:
            raise StaleSimilarityEvidence(f"reference {reference_id!r} image fingerprint is stale")
        _validate_sequence(reference.opcodes,
                           f"reference {reference_id!r} opcode sequence")
        usable.append(reference)

    candidate_set = set(candidate.opcodes)
    feature_rows = []
    for reference in usable:
        ref_set = set(reference.opcodes)
        candidate_length, reference_length = len(candidate.opcodes), len(reference.opcodes)
        length_ratio = (min(candidate_length, reference_length) /
                        max(candidate_length, reference_length)
                        if max(candidate_length, reference_length) else 1.0)
        union = candidate_set | ref_set
        jaccard = len(candidate_set & ref_set) / len(union) if union else 1.0
        prefiltered = (candidate_length == 0 or reference_length == 0 or
                       not candidate_set.intersection(ref_set))
        upper_bound = (2.0 * min(candidate_length, reference_length) /
                       (candidate_length + reference_length)
                       if candidate_length + reference_length else 1.0)
        feature_rows.append((upper_bound, reference, length_ratio, jaccard, prefiltered))

    feature_rows.sort(key=lambda row: (-row[0], row[1].reference_id))
    ranked: list[SimilarityMatch] = []
    pruned_reference_count = 0
    for upper_bound, reference, length_ratio, jaccard, prefiltered in feature_rows:
        ranked.sort(key=lambda item: (-item.score, item.reference_id))
        if top_k == 0 or (len(ranked) >= top_k and upper_bound < ranked[top_k - 1].score):
            pruned_reference_count += 1
            continue
        candidate_length, reference_length = len(candidate.opcodes), len(reference.opcodes)
        if prefiltered:
            sequence_ratio = (
                1.0 if candidate_length == 0 and reference_length == 0 else 0.0
            )
        else:
            sequence_ratio = SequenceMatcher(
                None, candidate.opcodes, reference.opcodes).ratio()
        ranked.append(SimilarityMatch(
            reference_id=reference.reference_id,
            reference_group=reference.ownership_group,
            score=sequence_ratio,
            sequence_ratio=sequence_ratio,
            length_ratio=length_ratio,
            opcode_jaccard=jaccard,
            provenance=reference.provenance,
            prefiltered=prefiltered,
            provisional_provenance=reference.provisional_provenance,
            tentative=reference.reference_id in tentative,
        ))

    ranked.sort(key=lambda item: (-item.score, item.reference_id))
    matches = tuple(ranked[:top_k])
    return SimilarityRanking(
        candidate_id=candidate.candidate_id,
        candidate_provenance=candidate.provenance,
        matches=matches,
        eligible_pool_size=len(usable),
        returned_count=len(matches),
        omitted_count=max(0, len(usable) - len(matches)),
        excluded=tuple(excluded),
        pruned_reference_count=pruned_reference_count,
    )


def rank_ownership_groups(
    candidates: Sequence[OpcodeCandidate],
    references: Sequence[OpcodeReference],
    *,
    candidate_groups: Mapping[str, str],
    eligible_reference_ids: Iterable[str],
    source_fingerprint: str,
    image_fingerprint: str,
    tentative_reference_ids: Iterable[str] = (),
    top_k: int = 5,
    limit: int = 50,
    spread_groups: bool = True,
) -> OwnershipGroupRanking:
    """Schedule a bounded owner batch by verified sibling similarity.

    Every candidate is scored against the full explicitly eligible reference
    pool before ``limit`` is applied. Candidate ownership groups organize the
    target schedule only; reference ownership is retained on each match as
    provenance and does not constrain cross-owner exemplar search. Unmatched
    targets remain in the counted candidate pool. Optional round-robin
    spreading balances candidate owner groups in the bounded result prefix.
    """
    if type(spread_groups) is not bool:
        raise TypeError("spread_groups must be bool")
    if type(top_k) is not int or not 0 <= top_k <= MAX_TOP_K:
        raise ValueError(f"top_k must be between 0 and {MAX_TOP_K}")
    if type(limit) is not int or not 0 <= limit <= MAX_CANDIDATE_POOL:
        raise ValueError(f"limit must be between 0 and {MAX_CANDIDATE_POOL}")
    if top_k * limit > MAX_OUTPUT_MATCHES:
        raise ValueError(f"top_k * limit exceeds {MAX_OUTPUT_MATCHES} output matches")
    if len(candidates) > MAX_CANDIDATE_POOL:
        raise ValueError(f"candidate pool exceeds {MAX_CANDIDATE_POOL} candidates")
    if len(references) > MAX_REFERENCE_POOL:
        raise ValueError(f"reference pool exceeds {MAX_REFERENCE_POOL} references")
    _nonempty_identity(source_fingerprint, "source_fingerprint")
    _nonempty_identity(image_fingerprint, "image_fingerprint")

    candidate_ids = [candidate.candidate_id for candidate in candidates]
    if len(candidate_ids) != len(set(candidate_ids)):
        raise ValueError("candidate IDs must be unique")
    if set(candidate_groups) != set(candidate_ids):
        raise ValueError("candidate_groups must map exactly every candidate ID")
    for candidate_id, group in candidate_groups.items():
        _nonempty_identity(candidate_id, "candidate group key")
        _nonempty_identity(group, f"candidate {candidate_id!r} ownership group")

    reference_ids = [reference.reference_id for reference in references]
    if len(reference_ids) != len(set(reference_ids)):
        raise ValueError("reference IDs must be unique")
    eligible_ids = tuple(eligible_reference_ids)
    tentative_ids = tuple(tentative_reference_ids)
    if (len(eligible_ids) != len(set(eligible_ids)) or
            len(tentative_ids) != len(set(tentative_ids))):
        raise ValueError("reference ID selections must be unique")
    if set(eligible_ids) & set(tentative_ids):
        raise ValueError("a reference cannot be both eligible and tentative")
    reference_id_set = set(reference_ids)
    missing_ids = (set(eligible_ids) | set(tentative_ids)) - reference_id_set
    if missing_ids:
        raise ValueError(f"selected reference IDs are absent: {sorted(missing_ids)!r}")

    exclusions: dict[tuple[str, str], None] = {}
    selected_set = set(eligible_ids) | set(tentative_ids)
    eligible_set = set(eligible_ids)
    tentative_set = set(tentative_ids)
    for reference in references:
        _validate_reference_flags(reference)
        if reference.reference_id in selected_set:
            should_validate_evidence = (
                not reference.scaffold and not reference.historical_claim_only and
                ((reference.reference_id in eligible_set and reference.verified is True) or
                 (reference.reference_id in tentative_set and reference.verified is False))
            )
            if should_validate_evidence:
                _nonempty_identity(reference.provenance,
                                   f"reference {reference.reference_id!r} provenance")
                _nonempty_identity(reference.source_fingerprint,
                                   f"reference {reference.reference_id!r} source fingerprint")
                _nonempty_identity(reference.image_fingerprint,
                                   f"reference {reference.reference_id!r} image fingerprint")
                if reference.source_fingerprint != source_fingerprint:
                    raise StaleSimilarityEvidence(
                        f"reference {reference.reference_id!r} source fingerprint is stale")
                if reference.image_fingerprint != image_fingerprint:
                    raise StaleSimilarityEvidence(
                        f"reference {reference.reference_id!r} image fingerprint is stale")
                _validate_sequence(reference.opcodes,
                                   f"reference {reference.reference_id!r} opcode sequence")
        if reference.reference_id not in selected_set:
            reason = "not explicitly eligible"
            exclusions[(reference.reference_id, reason)] = None
            continue
        if reference.ownership_group is not None:
            _nonempty_identity(reference.ownership_group,
                               f"reference {reference.reference_id!r} ownership group")

    selected_references = [reference for reference in references
                           if reference.reference_id in selected_set]
    comparison_count = len(candidates) * len(selected_references)
    if comparison_count > MAX_BATCH_COMPARISONS:
        raise ValueError(f"candidate/reference batch exceeds {MAX_BATCH_COMPARISONS} comparisons")

    by_group: dict[str, list[OwnershipCandidateRanking]] = defaultdict(list)
    for candidate in candidates:
        group = candidate_groups[candidate.candidate_id]
        result = rank_opcode_sequences(
            candidate,
            references,
            eligible_reference_ids=eligible_ids,
            tentative_reference_ids=tentative_ids,
            source_fingerprint=source_fingerprint,
            image_fingerprint=image_fingerprint,
            top_k=top_k,
        )
        for exclusion in result.excluded:
            exclusions[exclusion] = None
        best_score = result.matches[0].score if result.matches else 0.0
        by_group[group].append(OwnershipCandidateRanking(
            candidate_id=candidate.candidate_id,
            ownership_group=group,
            best_score=best_score,
            matches=result.matches,
            unmatched=(not result.matches or best_score == 0.0),
        ))

    for group in by_group:
        by_group[group].sort(key=lambda item: (-item.best_score, item.candidate_id))
    group_order = sorted(
        by_group,
        key=lambda group: (-max((item.best_score for item in by_group[group]), default=0.0),
                           group),
    )

    ordered: list[OwnershipCandidateRanking] = []
    if spread_groups:
        offsets = {group: 0 for group in group_order}
        while True:
            advanced = False
            for group in group_order:
                offset = offsets[group]
                if offset < len(by_group[group]):
                    ordered.append(by_group[group][offset])
                    offsets[group] += 1
                    advanced = True
            if not advanced:
                break
    else:
        ordered = [item for group in group_order for item in by_group[group]]

    returned = tuple(ordered[:limit])
    group_counts = tuple(
        (group, len(by_group[group]), sum(not item.unmatched for item in by_group[group]))
        for group in group_order
    )
    return OwnershipGroupRanking(
        candidates=returned,
        candidate_pool_size=len(candidates),
        returned_count=len(returned),
        omitted_count=len(candidates) - len(returned),
        group_counts=group_counts,
        excluded_reference_count=len({reference_id for reference_id, _ in exclusions}),
        excluded_references=tuple(sorted(exclusions)),
    )


