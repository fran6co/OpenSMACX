"""Source-aware, advisory opcode examples for recovery work planning.

This adapter selects only explicitly requested exemplar mappings, measures
those current source bodies afresh, and uses the pinned image for candidate
opcode sequences. Its rankings are context, never equivalence or ownership
proof. The similarity implementation is imported lazily so this adapter can be
developed alongside that worker API without a second scoring implementation.
"""

from __future__ import annotations

import re
from dataclasses import asdict
from hashlib import sha256
from pathlib import Path
from typing import Callable, Sequence

from match_recovery.model import Span, digest
from match_recovery.ownership_source import project_view
from match_recovery.pe_coff import PeImage
from match_recovery.source_config import SourceConfiguration
from match_recovery.source_inventory import (assert_snapshot_current,
                                             source_snapshot)
from match_recovery.source_selection import (select_records,
                                             selected_source_records,
                                             snapshot_mappings)
from match_recovery.source_measure import measure_source as _measure_source
from match_recovery.source_quality import quality_report


MAX_CANDIDATES = 10000
MAX_REFERENCES = 10000
MAX_UNRESOLVED_DETAIL = 200
MAX_SPAN_BYTES = 65536
MAX_TOTAL_ORIGINAL_BYTES = 4 * 1024 * 1024
MAX_EXCERPT_CHARS = 1200
_RAW_OR_ASM_KINDS = {
    "manual virtual dispatch representation",
    "hardcoded address-like literal",
    "unfinished body or fixed-address forwarder",
    "native placeholder classification",
}
_GENERATED_PREFIXES = ("??_G", "??_E", "??__E", "??__F")


def _source_fingerprint(snapshot: dict, root: Path) -> str:
    """Bind source content (including uncommitted edits) and worktree identity."""
    return digest({
        "root": str(root.resolve()),
        "revision": snapshot.get("revision"),
        "files": {name: row["sha256"] for name, row in sorted(snapshot["files"].items())},
    })


def _identity_id(fact: dict) -> str:
    return fact["entity_id"]


def _line_excerpt(record, maximum: int = MAX_EXCERPT_CHARS) -> str:
    region = getattr(record, "region", "") or ""
    region = region.strip()
    if len(region) <= maximum:
        return region
    return region[:maximum].rstrip() + " …[excerpt capped]"


def _opcode_tokens(listing) -> tuple[str, ...]:
    """Return API-valid mnemonic tokens while preserving x86 RET pop width."""
    tokens = []
    for instruction in listing.instructions:
        mnemonic = instruction.mnemonic.lower()
        operand = instruction.op_str.strip()
        # match_recovery.similarity accepts normalized mnemonic-like tokens,
        # not whole disassembly lines. Encode `ret 4` distinctly as `ret_4`.
        if mnemonic.startswith("ret") and operand:
            operand_token = re.sub(r"[^a-zA-Z0-9_.]+", "_", operand).strip("_").lower()
            mnemonic = f"{mnemonic}_{operand_token}"
        tokens.append(mnemonic)
    return tuple(tokens)


def _decode_record(pe: PeImage, spans, *, consumed: int) -> tuple[tuple[str, ...], int]:
    from match_recovery.source.asm import Listing

    all_tokens = []
    for low, high in spans:
        size = high - low
        if size <= 0 or size > MAX_SPAN_BYTES:
            raise ValueError(f"span size {size} is outside the 1..{MAX_SPAN_BYTES} decode bound")
        if consumed + size > MAX_TOTAL_ORIGINAL_BYTES:
            raise ValueError("original opcode decode budget exhausted")
        data = pe.read(Span(low, high))
        consumed += len(data)
        listing = Listing(data, low)
        decoded_size = sum(len(instruction.bytes) for instruction in listing.instructions)
        if decoded_size != len(data):
            raise ValueError(f"declared span at 0x{low:08X} is not completely decodable")
        all_tokens.extend(_opcode_tokens(listing))
    return tuple(all_tokens), consumed


def _mapping_fact(snapshot: dict, path: str, line: int) -> dict:
    matches = [row for row in snapshot["records"]
               if row["path"] == path and row["line"] == line]
    if len(matches) != 1:
        raise ValueError(f"source mapping at {path}:{line} is not unique in its snapshot")
    return matches[0]


def _owner_selectors(view: dict) -> dict[str, str]:
    return {entity["entity_id"]: group["selector"]
            for group in view["groups"] for entity in group["entities"]}


def _quality_by_entity(report: dict) -> dict[str, dict]:
    return {row["entity_id"]: row for row in report.get("functions", [])}


def _generated_mapping(record, fact: dict) -> bool:
    symbol = fact.get("emitted_symbol_hint") or fact.get("catalogued_symbol") or ""
    return (symbol.startswith(_GENERATED_PREFIXES)
            or re.search(r"@@\$[0-9A-Z]", symbol) is not None
            or "thunk" in symbol.lower())


def _reference_exclusion(record, fact: dict, owner: str | None,
                         quality: dict | None) -> str | None:
    if fact.get("candidate_kind") != "source":
        return "seed mapping is context only; no current source body selected"
    if owner and owner.startswith("component:"):
        return "third-party/component mapping is not an application exemplar"
    if fact.get("source_role") == "scaffold":
        return "current source mapping is explicitly classified as scaffold"
    state = getattr(getattr(record, "state", None), "value", getattr(record, "state", None))
    if state != "implemented":
        return f"current source body state is {state or 'unknown'}"
    if not getattr(record, "region", "").strip():
        return "no authored source body region was resolved"
    if _generated_mapping(record, fact):
        return "compiler-generated/lifetime or thunk mapping has no authored exemplar body"
    findings = quality.get("findings", []) if quality else []
    kinds = {finding.get("kind") for finding in findings}
    if "native placeholder classification" in kinds or "unfinished body or fixed-address forwarder" in kinds:
        return "source-quality report identifies placeholder/forwarder reconstruction debt"
    if re.search(r"\b(?:__asm|asm)\b", record.region):
        return "inline assembly source is not a C++ recovery exemplar"
    if kinds & _RAW_OR_ASM_KINDS:
        return "source-quality report flags raw-address/dispatch scaffold-like code"
    return None


def _report_measurement_kind(report: dict, requested: str) -> str:
    if requested == "masked":
        # Current source_measure reports this explicitly. A custom callable is
        # accepted only when it doesn't contradict the requested mode.
        verification = report.get("verification")
        if verification not in (None, "historical_masked_bytes_only", "source_masked_bytes_and_component_binary"):
            raise ValueError("measurement callable returned non-masked verification for masked mode")
        return "masked"
    if report.get("measurement_kind") != "strict":
        raise ValueError("strict mode requires a measurement report explicitly marked measurement_kind='strict'")
    return "strict"


def recommend_examples(
    root: Path,
    image: Path,
    targets: Sequence[str] = (),
    reference_targets: Sequence[str] = (),
    *,
    all_candidates: bool = False,
    limit: int = 50,
    top_k: int = 5,
    measure_source: Callable | None = None,
    measurement_kind: str = "masked",
) -> dict:
    """Build advisory similarity context from current source and pinned bytes.

    ``targets`` are candidate mappings; an empty list is permitted only with
    ``all_candidates=True``. ``reference_targets`` are explicit source
    mappings to freshly measure. `limit` caps returned owner candidates, not
    the input universe; pool and omission counts are returned by the worker.

    Masked mode uses the native source measurement API and labels its result as
    masked-byte exact, never strict reference equivalence. Strict mode is
    reserved for an injected measurement callable which explicitly reports
    ``measurement_kind='strict'`` for each successful measurement.
    """
    root = root.resolve()
    image = (image if image.is_absolute() else root / image).resolve()
    if not isinstance(targets, (list, tuple)) or not isinstance(reference_targets, (list, tuple)):
        raise TypeError("targets and reference_targets must be sequences of selectors")
    if type(all_candidates) is not bool:
        raise TypeError("all_candidates must be bool")
    if not targets and not all_candidates:
        raise ValueError("supply targets or set all_candidates=True")
    if targets and all_candidates:
        raise ValueError("explicit targets and all_candidates are mutually exclusive")
    if type(limit) is not int or not 0 <= limit <= MAX_CANDIDATES:
        raise ValueError(f"limit must be between 0 and {MAX_CANDIDATES}")
    if type(top_k) is not int or not 0 <= top_k <= 100:
        raise ValueError("top_k must be between 0 and 100")
    if measurement_kind not in ("masked", "strict"):
        raise ValueError("measurement_kind must be 'masked' or 'strict'")
    if len(reference_targets) > MAX_REFERENCES:
        raise ValueError(f"reference target count exceeds {MAX_REFERENCES}")

    configuration = SourceConfiguration.read(root)
    snapshot = source_snapshot(root, image)
    source_fingerprint = _source_fingerprint(snapshot, root)
    image_fingerprint = snapshot["image_sha256"]
    if configuration.address_space != "pe:va":
        raise ValueError("opcode similarity currently requires PE virtual-address mappings")

    # project_view provides current owner-group context. Its separate snapshot
    # must agree with our captured source snapshot before it is used.
    view, view_snapshot = project_view(root, image)
    if (_source_fingerprint(view_snapshot, root) != source_fingerprint
            or view_snapshot["image_sha256"] != image_fingerprint):
        raise ValueError("source/project ownership view changed during context preparation")
    owners = _owner_selectors(view)
    mappings = snapshot_mappings(root, snapshot)
    facts_by_entity = {}
    for fact in snapshot["records"]:
        facts_by_entity.setdefault(fact["entity_id"], []).append(fact)

    unresolved = []
    selected_mappings = []
    if all_candidates:
        source_facts = [fact for fact in snapshot["records"] if fact["candidate_kind"] == "source"]
        for entity_id, facts in sorted({eid: rows for eid, rows in facts_by_entity.items()}.items()):
            current = [fact for fact in facts if fact["candidate_kind"] == "source"]
            if len(current) != 1:
                if current:
                    unresolved.append({"entity_id": entity_id, "status": "ambiguous current source mapping",
                                       "candidates": [{"path": row["path"], "line": row["line"]} for row in current],
                                       "seed_alternatives": sum(row["candidate_kind"] != "source" for row in facts)})
                continue
            fact = current[0]
            if owners.get(entity_id, "").startswith("component:"):
                unresolved.append({"entity_id": entity_id, "status": "excluded third-party/component mapping",
                                   "path": fact["path"], "line": fact["line"],
                                   "owner_group": owners[entity_id]})
                continue
            host = configuration.path(root, fact["path"])
            try:
                one = select_records(mappings, [f"0x{fact['origin']['address']:08X}"], host,
                                     configuration.symbol_format)
                selected_mappings.extend(one)
            except ValueError as error:
                unresolved.append({"entity_id": entity_id, "status": "unresolved source mapping",
                                   "path": fact["path"], "line": fact["line"], "reason": str(error),
                                   "seed_alternatives": sum(row["candidate_kind"] != "source" for row in facts)})
        # Keep the number of retained candidates explicit when the ranker limit
        # is lower than the source pool. Do not silently claim full coverage.
        selected.sort(key=lambda row: (row.path.as_posix(), row.line, row.address))
        candidate_universe_size = len(selected_mappings)
    else:
        candidate_universe_size = len(targets)
        for target in targets:
            try:
                one = select_records(mappings, [target], None, configuration.symbol_format)
                selected_mappings.extend(one)
            except ValueError as error:
                unresolved.append({"target": target, "status": "unresolved or ambiguous target", "reason": str(error)})

    if len({(row.path, row.line) for row in selected_mappings}) != len(selected_mappings):
        raise ValueError("candidate selectors resolve to the same source mapping more than once")
    try:
        selected = selected_source_records(root, snapshot, selected_mappings)
    except ValueError as error:
        if selected_mappings:
            unresolved.append({"status": "selected candidate source batch failed snapshot/body validation",
                               "count": len(selected_mappings), "reason": str(error)})
        selected = []
    if len(selected) > MAX_CANDIDATES:
        raise ValueError(f"candidate pool has {len(selected)} mappings, exceeding the {MAX_CANDIDATES} worker bound; shard with explicit selectors")

    application_selected = []
    for record in selected:
        fact = _mapping_fact(snapshot, record.path.relative_to(root).as_posix(), record.line)
        owner = owners.get(_identity_id(fact), "unassigned")
        if owner.startswith("component:"):
            unresolved.append({"entity_id": _identity_id(fact),
                               "status": "excluded third-party/component candidate",
                               "path": fact["path"], "line": fact["line"],
                               "owner_group": owner})
            continue
        application_selected.append(record)
    selected = application_selected
    if len(selected) > MAX_CANDIDATES:
        raise ValueError(f"candidate pool exceeds the {MAX_CANDIDATES} worker limit")

    # Quality results are advisory filters for exemplar selection only. They
    # never certify a candidate's structure or disqualify it as recovery work.
    quality = _quality_by_entity(quality_report(root))
    selected_facts = [_mapping_fact(snapshot, row.path.relative_to(root).as_posix(), row.line)
                      for row in selected]

    image_data = image.read_bytes()
    if sha256(image_data).hexdigest() != image_fingerprint:
        raise ValueError("pinned image changed while original opcodes were being read")
    pe = PeImage(image_data, image_fingerprint)
    references, eligible_reference_ids, reference_context = [], [], {}
    consumed = 0
    if reference_targets:
        if measurement_kind == "strict" and measure_source is None:
            raise ValueError("strict measurement mode requires an explicitly injected strict measurement callable")
        ref_mappings = []
        selected_ref_targets = []
        for target in reference_targets:
            try:
                mapping = select_records(mappings, [target], None, configuration.symbol_format)[0]
                ref_mappings.append(mapping)
                selected_ref_targets.append(mapping.address_hex)
            except ValueError as error:
                unresolved.append({"target": target, "status": "unresolved or ambiguous reference target",
                                   "reason": str(error)})
        if len({(row.path, row.line) for row in ref_mappings}) != len(ref_mappings):
            raise ValueError("reference selectors resolve to the same source mapping more than once")
        try:
            ref_records = selected_source_records(root, snapshot, ref_mappings)
        except ValueError as error:
            unresolved.append({"status": "selected reference source batch failed snapshot/body validation",
                               "count": len(ref_mappings), "reason": str(error)})
            ref_records = []
            selected_ref_targets = []
        measure_records = []
        for record in ref_records:
            fact = _mapping_fact(snapshot, record.path.relative_to(root).as_posix(), record.line)
            entity_id = _identity_id(fact)
            owner = owners.get(entity_id)
            reason = _reference_exclusion(record, fact, owner, quality.get(entity_id))
            if reason is None:
                measure_records.append(record)
            else:
                reference_context[entity_id] = {"eligible": False, "reason": reason,
                                                "path": fact["path"], "line": fact["line"],
                                                "owner_group": owner}
                unresolved.append({"entity_id": entity_id, "status": "reference excluded",
                                   "path": fact["path"], "line": fact["line"], "reason": reason})
        if len(selected) * len(measure_records) > 100_000:
            raise ValueError(f"candidate/reference batch requires {len(selected) * len(measure_records)} comparisons, above the 100000 comparison bound; shard targets or references")
        selected_ref_targets = [f"0x{record.address:08X}" for record in measure_records]
        # Measure the selected references once as a bounded batch using the
        # package API; no candidate source body is compiled by this adapter.
        measurement = {}
        effective_kind = measurement_kind
        if measure_records:
            measure_fn = measure_source or _measure_source
            measurement = measure_fn(root, selected_ref_targets, image=image)
            effective_kind = _report_measurement_kind(measurement, measurement_kind)
            if measurement.get("result") != "pass":
                unresolved.append({"status": "reference measurement batch not fully passing",
                                   "result": measurement.get("result"),
                                   "counts": measurement.get("counts", {})})
        result_by_location = {(row.get("path"), row.get("line")): row
                              for row in measurement.get("measurements", [])}
        for record in measure_records:
            fact = _mapping_fact(snapshot, record.path.relative_to(root).as_posix(), record.line)
            entity_id = _identity_id(fact)
            owner = owners.get(entity_id)
            measured = result_by_location.get((fact["path"], fact["line"]))
            reason = None
            if measured is None:
                reason = "fresh measurement did not return this exact source mapping"
            if reason is None and (measured.get("result") != "pass"
                                   or measured.get("verdict") != "BYTE_EXACT"):
                reason = ("fresh strict measurement not byte exact" if effective_kind == "strict"
                          else "fresh masked-byte measurement not byte exact")
            if reason is not None:
                reference_context[entity_id] = {"eligible": False, "reason": reason,
                                                "path": fact["path"], "line": fact["line"],
                                                "owner_group": owner}
                unresolved.append({"entity_id": entity_id, "status": "reference excluded",
                                   "path": fact["path"], "line": fact["line"], "reason": reason})
                continue
            try:
                opcodes, consumed = _decode_record(pe, fact["spans"], consumed=consumed)
            except ValueError as error:
                reference_context[entity_id] = {"eligible": False,
                                                "reason": f"pinned original opcodes unresolved: {error}",
                                                "path": fact["path"], "line": fact["line"],
                                                "owner_group": owner}
                unresolved.append({"entity_id": entity_id, "status": "reference excluded",
                                   "path": fact["path"], "line": fact["line"],
                                   "reason": f"pinned original opcodes unresolved: {error}"})
                continue
            if not opcodes:
                reference_context[entity_id] = {"eligible": False, "reason": "empty pinned opcode sequence",
                                                "path": fact["path"], "line": fact["line"],
                                                "owner_group": owner}
                unresolved.append({"entity_id": entity_id, "status": "reference excluded",
                                   "path": fact["path"], "line": fact["line"],
                                   "reason": "empty pinned opcode sequence"})
                continue
            from match_recovery.similarity import OpcodeReference
            reference_id = entity_id
            provenance = (f"fresh {effective_kind} BYTE_EXACT source measurement; "
                          f"provisional worktree {root} revision {snapshot.get('revision') or 'unknown'}")
            reference = OpcodeReference.create(
                reference_id, opcodes,
                source_fingerprint=source_fingerprint, image_fingerprint=image_fingerprint,
                provenance=provenance, verified=True, scaffold=False,
                historical_claim_only=False, provisional_provenance=True,
                ownership_group=owner or "unassigned")
            references.append(reference)
            eligible_reference_ids.append(reference_id)
            reference_context[reference_id] = {
                "eligible": True, "verification_level": f"{effective_kind}_byte_exact",
                "measurement_kind": effective_kind, "path": fact["path"], "line": fact["line"],
                "address": f"0x{fact['origin']['address']:08X}",
                "fresh_measurement": measured,
                "owner_group": owner or "unassigned", "excerpt": _line_excerpt(record),
                "provisional_worktree_provenance": provenance,
            }

    candidates, candidate_groups, candidate_context = [], {}, {}
    from match_recovery.similarity import OpcodeCandidate, rank_ownership_groups
    for record, fact in zip(selected, selected_facts, strict=True):
        entity_id = _identity_id(fact)
        try:
            opcodes, consumed = _decode_record(pe, fact["spans"], consumed=consumed)
        except ValueError as error:
            unresolved.append({"entity_id": entity_id, "status": "original opcode sequence unresolved",
                               "path": fact["path"], "line": fact["line"], "reason": str(error)})
            continue
        if not opcodes:
            unresolved.append({"entity_id": entity_id, "status": "empty original opcode sequence",
                               "path": fact["path"], "line": fact["line"]})
            continue
        group = owners.get(entity_id, "unassigned")
        provenance = (f"pinned original opcodes; provisional current worktree {root} "
                      f"revision {snapshot.get('revision') or 'unknown'}")
        candidate = OpcodeCandidate.create(entity_id, opcodes,
                                           source_fingerprint=source_fingerprint,
                                           image_fingerprint=image_fingerprint,
                                           provenance=provenance)
        candidates.append(candidate)
        candidate_groups[entity_id] = group
        candidate_context[entity_id] = {
            "path": fact["path"], "line": fact["line"], "address": f"0x{fact['origin']['address']:08X}",
            "owner_group": group, "source_role": fact.get("source_role"),
            "candidate_kind": fact.get("candidate_kind"), "historical_byte_claim": fact.get("legacy_byte_exact", False),
            "scaffold_investigation_target": fact.get("source_role") == "scaffold",
            "quality_findings": quality.get(entity_id, {}).get("findings", []),
            "excerpt": _line_excerpt(record),
            "provisional_worktree_provenance": provenance,
        }

    assert_snapshot_current(root, snapshot)
    if not candidates:
        visible_unresolved = unresolved[:MAX_UNRESOLVED_DETAIL]
        return {
            "schema_version": 1, "operation": "recommend_examples", "result": "unknown",
            "advisory_only": True, "equivalence_established": False, "acceptance_established": False,
            "source_fingerprint": source_fingerprint, "image_fingerprint": image_fingerprint,
            "measurement_kind": measurement_kind, "candidate_universe_size": candidate_universe_size,
            "candidate_pool_size": 0, "returned_count": 0, "omitted_count": candidate_universe_size,
            "eligible_reference_count": len(eligible_reference_ids),
            "candidates": [], "references": [], "rankings": None,
            "eligible_reference_ids": [], "unresolved": visible_unresolved,
            "unresolved_count": len(unresolved),
            "unresolved_omitted_count": max(0, len(unresolved) - len(visible_unresolved)),
            "candidate_context": {}, "reference_context": {},
            "limitations": ["no candidate body had a completely decodable pinned source span"],
        }
    eligible_count = len(eligible_reference_ids)
    ranking = rank_ownership_groups(
        candidates, references, candidate_groups=candidate_groups,
        eligible_reference_ids=eligible_reference_ids,
        source_fingerprint=source_fingerprint, image_fingerprint=image_fingerprint,
        top_k=top_k, limit=limit)
    ranked_candidates = [asdict(row) for row in ranking.candidates]
    returned_ids = {row["candidate_id"] for row in ranked_candidates}
    matched_reference_ids = {match["reference_id"] for row in ranked_candidates
                             for match in row["matches"]}
    visible_unresolved = unresolved[:MAX_UNRESOLVED_DETAIL]
    return {
        "schema_version": 1, "operation": "recommend_examples", "result": "advisory",
        "advisory_only": ranking.advisory_only,
        "equivalence_established": ranking.equivalence_established,
        "acceptance_established": ranking.acceptance_established,
        "source_fingerprint": source_fingerprint, "image_fingerprint": image_fingerprint,
        "measurement_kind": measurement_kind,
        "candidate_universe_size": candidate_universe_size,
        "candidate_pool_size": ranking.candidate_pool_size,
        "returned_count": ranking.returned_count, "omitted_count": ranking.omitted_count,
        "excluded_reference_count": ranking.excluded_reference_count,
        "eligible_reference_count": eligible_count,
        "rankings": ranked_candidates,
        "candidate_context": {key: candidate_context[key] for key in sorted(returned_ids)},
        "reference_context": {key: reference_context[key] for key in sorted(matched_reference_ids)
                              if key in reference_context},
        "unresolved": visible_unresolved, "unresolved_count": len(unresolved),
        "unresolved_omitted_count": max(0, len(unresolved) - len(visible_unresolved)),
        "limitations": [
            "opcode similarity is advisory and does not establish source equivalence or acceptance",
            "owner groups are current source/project hints, not structural certification or historical-TU proof",
            "masked BYTE_EXACT means relocation-masked body comparison only, not strict reference equivalence",
            "worktree/source provenance is provisional; excerpts are bounded context, never source authority",
            "candidate scopes are selected explicitly or from current source mappings; unresolved/ambiguous records remain reported",
        ],
    }
