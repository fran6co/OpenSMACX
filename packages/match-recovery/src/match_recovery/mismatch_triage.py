"""Advisory triage of already-observed function mismatch evidence.

This module classifies clues; it does not compare machine code, resolve a
binding, or establish semantic equivalence. Incomplete inputs intentionally
remain unknown.
"""

from itertools import islice
import re
from collections import Counter
from typing import Mapping


_BRANCH = re.compile(r"^(?:j[a-z]+|loop[a-z]*|call|ret|retn|jmp)$", re.I)
_MEMORY = re.compile(
    r"(?:(byte|word|dword|qword|tbyte|xmmword)\s+ptr\s*)?\[([^\]]+)\]", re.I)
_REGISTER = re.compile(
    r"\b(?:e?[abcd]x|e?[sd]i|e?[sb]p|r(?:[0-9]+[dwb]?|[abcd]x)|"
    r"[abcd][lh]|[sd]il|[sd]pl|[xyz]mm[0-9]+|mm[0-9]+)\b", re.I)
_NUMBER = re.compile(r"(?<![\w])(?:0x[0-9a-f]+|[0-9]+)(?![\w])", re.I)
_MAX_NODES = 1200
_MAX_LIST = 256
_MAX_INSTRUCTIONS = 512
_MAX_STRING = 2048
_MAX_OBSERVATIONS = 64

_NEXT = {
    "behavior_control_flow_constants":
        "Compare the complete pinned branch paths and literal operands; do not infer runtime behavior from this local difference alone.",
    "member_width_offset":
        "Trace the access to its owning object/member using original instructions and producer/caller evidence; an address displacement alone is not a member proof.",
    "lifetime":
        "Verify the full constructor/destructor or call sequence and ownership path against the pinned span and callers.",
    "register_schedule":
        "Inspect def-use and instruction order in the full spans; register-only resemblance is not semantic equivalence.",
    "relocation_targets":
        "Resolve each relocation through independent source/object bindings and strict reference comparison; symbol text alone is not target proof.",
    "unknown":
        "Obtain complete, bounded original and candidate evidence before classifying this mismatch.",
}


def _unknown(reason, *, comparison=None, original=None, candidate=None):
    omitted = [0]
    measurements = _bounded_copy(comparison, omitted) if isinstance(comparison, Mapping) else None
    if comparison is None and (original is not None or candidate is not None):
        measurements = {
            "original": _bounded_copy(original, omitted),
            "candidate": _bounded_copy(candidate, omitted),
        }
    return {
        "schema_version": 1,
        "status": "unknown",
        "classifications": ["unknown"],
        "reason_codes": [reason],
        "observations": [],
        "measurements": measurements,
        "omitted_measurement_items": omitted[0],
        "diagnostic_state": {
            "original_truncated": _truncated(original),
            "candidate_truncated": _truncated(candidate),
        },
        "next_investigation": [_NEXT["unknown"]],
        "limitations": [
            "advisory mismatch triage only; not semantic equivalence, structural proof, or acceptance",
            "missing, ambiguous, or truncated evidence remains unknown",
        ],
    }


def _bounded_copy(value, omitted, budget=None):
    """Copy evidence under hard output bounds, counting omitted values."""
    if budget is None:
        budget = [_MAX_NODES]
    if budget[0] <= 0:
        omitted[0] += 1
        return "<omitted:output_limit>"
    budget[0] -= 1
    if isinstance(value, str):
        if len(value) > _MAX_STRING:
            omitted[0] += 1
            suffix = "<omitted:tail>"
            return value[:_MAX_STRING - len(suffix)] + suffix
        return value
    if isinstance(value, Mapping):
        output = {}
        for key, item in islice(value.items(), _MAX_LIST):
            if not isinstance(key, str):
                omitted[0] += 1
                continue
            output[key[:128]] = _bounded_copy(item, omitted, budget)
        if len(value) > _MAX_LIST:
            omitted[0] += len(value) - _MAX_LIST
        return output
    if isinstance(value, (list, tuple)):
        output = [_bounded_copy(item, omitted, budget) for item in value[:_MAX_LIST]]
        if len(value) > _MAX_LIST:
            omitted[0] += len(value) - _MAX_LIST
        return output
    if value is None or type(value) in (bool, int, float):
        return value
    omitted[0] += 1
    return f"<omitted:unsupported:{type(value).__name__}>"


def _truncated(value):
    return bool(isinstance(value, Mapping) and value.get("truncated", False))


def _section(report):
    """Return a single complete diagnostic section, or a stable refusal code."""
    if not isinstance(report, Mapping):
        return None, "diagnostic_missing"
    if report.get("status") != "observed":
        return None, "diagnostic_unavailable"
    sections = report.get("sections")
    if not isinstance(sections, list) or len(sections) != 1 or report.get("omitted_sections", 0):
        return None, "section_ownership_ambiguous"
    part = sections[0]
    if not isinstance(part, Mapping):
        return None, "diagnostic_malformed"
    complete = (
        not report.get("truncated", False)
        and part.get("decode_status") == "linear_complete"
        and part.get("omitted_bytes", 1) == 0
        and part.get("undecoded_reported_bytes", 1) == 0
        and part.get("omitted_relocations", 1) == 0
        and part.get("omitted_decoded_instructions", 1) == 0
        and part.get("raw_size") == part.get("reported_bytes")
    )
    instructions, relocations = part.get("instructions"), part.get("relocations")
    if not isinstance(instructions, list) or not isinstance(relocations, list):
        return None, "diagnostic_malformed"
    if len(instructions) > _MAX_INSTRUCTIONS or len(relocations) > _MAX_INSTRUCTIONS:
        return None, "diagnostic_input_limit"
    return (part, instructions, relocations, complete), None


def _instruction_rows(rows):
    result = []
    for row in rows:
        if (not isinstance(row, Mapping) or not isinstance(row.get("mnemonic"), str)
                or not isinstance(row.get("operands_unlinked"), str)
                or len(row["mnemonic"]) > 64 or len(row["operands_unlinked"]) > _MAX_STRING
                or type(row.get("offset")) is not int
                or not isinstance(row.get("relocation_offsets", []), list)):
            return None
        result.append(row)
    return result


def _memory_shapes(operands):
    shapes = []
    for match in _MEMORY.finditer(operands):
        width, expression = match.groups()
        address_expression = re.sub(r"\s+", "", expression.lower())
        shapes.append((width.lower() if width else None, address_expression))
    return tuple(shapes)


def _add(observations, reasons, classifications, code, classification, details):
    reasons.add(code)
    classifications.add(classification)
    observations.append({"reason_code": code, **details})


def triage_mismatch(*, comparison=None, original=None, candidate=None):
    """Return stable advisory clues for a strict comparison or paired diagnostics.

    `comparison` is a strict comparison row (for example the result of
    `compare_bound_code`). Alternatively, `original` and `candidate` are one
    trial entry each from the existing bounded compiled-diagnostic report.
    Multi-section or truncated diagnostic pairs are not heuristically aligned.
    Input measurements are retained in bounded form, with omissions counted.
    """
    if comparison is not None:
        if not isinstance(comparison, Mapping):
            return _unknown("comparison_malformed", comparison=comparison)
        observations, reasons, classes = [], set(), set()
        omitted = [0]
        preserved = _bounded_copy(dict(comparison), omitted)
        if comparison.get("boundary") == "fail":
            _add(observations, reasons, classes, "strict_extent_failed", "unknown",
                 {"boundary": comparison.get("boundary"),
                  "original_size": comparison.get("original_size"),
                  "candidate_size": comparison.get("candidate_size")})
        if comparison.get("references") == "fail":
            _add(observations, reasons, classes, "strict_reference_mismatch", "relocation_targets",
                 {"relocations": _bounded_copy(comparison.get("relocations", []), omitted)})
        elif comparison.get("references") == "unknown":
            _add(observations, reasons, classes, "strict_references_unknown", "unknown",
                 {"references": "unknown"})
        if comparison.get("bytes") == "fail":
            _add(observations, reasons, classes, "strict_bytes_differ", "unknown",
                 {"differing_offsets": _bounded_copy(comparison.get("differing_offsets"), omitted)})
        if not observations:
            return {
                "schema_version": 1,
                "status": "unknown",
                "classifications": ["unknown"],
                "reason_codes": ["no_classifiable_mismatch"],
                "observations": [],
                "measurements": preserved,
                "omitted_measurement_items": omitted[0],
                "diagnostic_state": {"original_truncated": False, "candidate_truncated": False},
                "next_investigation": [_NEXT["unknown"]],
                "limitations": ["advisory mismatch triage only; no equivalence or acceptance claim"],
            }
        bounded_observations = _bounded_copy(observations[:_MAX_OBSERVATIONS], omitted)
        if len(observations) > _MAX_OBSERVATIONS:
            omitted[0] += len(observations) - _MAX_OBSERVATIONS
        return {
            "schema_version": 1,
            "status": "advisory",
            "classifications": sorted(classes),
            "reason_codes": sorted(reasons),
            "observations": bounded_observations,
            "measurements": preserved,
            "omitted_measurement_items": omitted[0],
            "diagnostic_state": {"original_truncated": False, "candidate_truncated": False},
            "next_investigation": [_NEXT[key] for key in sorted(classes)],
            "limitations": [
                "strict comparison facts are retained; byte/reference failure does not identify a C++ cause",
                "advisory only; not semantic equivalence, structural proof, or acceptance",
            ],
        }

    original_section, original_error = _section(original)
    candidate_section, candidate_error = _section(candidate)
    if original_error or candidate_error:
        reason = original_error or candidate_error
        return _unknown(reason, original=original, candidate=candidate)

    opart, original_rows, original_relocations, original_complete = original_section
    cpart, candidate_rows, candidate_relocations, candidate_complete = candidate_section
    original_rows = _instruction_rows(original_rows)
    candidate_rows = _instruction_rows(candidate_rows)
    if original_rows is None or candidate_rows is None:
        return _unknown("diagnostic_malformed", original=original, candidate=candidate)

    observations, reasons, classes = [], set(), set()
    if not original_complete or not candidate_complete:
        reasons.add("diagnostic_truncated_or_incomplete")
        classes.add("unknown")
        observations.append({
            "reason_code": "diagnostic_truncated_or_incomplete",
            "original_decode_status": opart.get("decode_status"),
            "candidate_decode_status": cpart.get("decode_status"),
            "original_reported_bytes": opart.get("reported_bytes"),
            "original_raw_size": opart.get("raw_size"),
            "candidate_reported_bytes": cpart.get("reported_bytes"),
            "candidate_raw_size": cpart.get("raw_size"),
        })

    complete_pair = original_complete and candidate_complete
    if len(original_rows) != len(candidate_rows):
        _add(observations, reasons, classes, "listed_instruction_count_differs", "unknown",
             {"original_count": len(original_rows), "candidate_count": len(candidate_rows)})

    # Ordinal comparison is deliberately limited to equal complete instruction
    # lists. Insertions make local pairing ambiguous, so only coarse evidence is
    # reported in that case.
    offsets_aligned = (len(original_rows) == len(candidate_rows) and
                       all(left["offset"] == right["offset"]
                           for left, right in zip(original_rows, candidate_rows)))
    if complete_pair and len(original_rows) == len(candidate_rows) and not offsets_aligned:
        _add(observations, reasons, classes, "instruction_alignment_ambiguous", "unknown",
             {"reason": "paired instruction offsets differ; ordinal branch operands are not compared"})
    if complete_pair and offsets_aligned:
        for index, (left, right) in enumerate(zip(original_rows, candidate_rows)):
            lm, rm = left["mnemonic"].lower(), right["mnemonic"].lower()
            lo, ro = left["operands_unlinked"], right["operands_unlinked"]
            has_reloc = bool(left.get("relocation_offsets")) or bool(right.get("relocation_offsets"))
            if has_reloc and (lm != rm or lo != ro):
                _add(observations, reasons, classes, "relocation_covered_operand_differs", "relocation_targets",
                     {"instruction_index": index, "evidence_level": "unlinked_relocation_covered_operand"})
                continue
            if lm != rm:
                _add(observations, reasons, classes, "opcode_or_control_flow_differs", "behavior_control_flow_constants",
                     {"instruction_index": index, "original_mnemonic": lm, "candidate_mnemonic": rm,
                      "original_offset": left["offset"], "candidate_offset": right["offset"]})
                continue
            if _BRANCH.match(lm) and lm != "call" and lo != ro:
                _add(observations, reasons, classes, "branch_operand_differs", "behavior_control_flow_constants",
                     {"instruction_index": index, "mnemonic": lm, "original_operands": lo,
                      "candidate_operands": ro})
            if lm == "call" and lo != ro:
                _add(observations, reasons, classes, "call_operand_differs_unresolved", "unknown",
                     {"instruction_index": index, "evidence_level": "unlinked_call_operand",
                      "original_operands_unlinked": lo, "candidate_operands_unlinked": ro})
            lmem, rmem = _memory_shapes(lo), _memory_shapes(ro)
            if lmem != rmem:
                _add(observations, reasons, classes, "memory_address_expression_differs", "member_width_offset",
                     {"instruction_index": index, "original_memory_shapes": lmem,
                      "candidate_memory_shapes": rmem,
                      "evidence_level": "unlinked_address_expression; scale/index/displacement are ambiguous"})
            lnums, rnums = _NUMBER.findall(_MEMORY.sub("", lo)), _NUMBER.findall(_MEMORY.sub("", ro))
            if lnums != rnums:
                _add(observations, reasons, classes, "immediate_operand_differs", "behavior_control_flow_constants",
                     {"instruction_index": index, "original_immediates": lnums,
                      "candidate_immediates": rnums})
            if lo != ro and lmem == rmem and lnums == rnums and not (_BRANCH.match(lm) or lm == "call"):
                normalized_left, normalized_right = _REGISTER.sub("<reg>", lo.lower()), _REGISTER.sub("<reg>", ro.lower())
                if normalized_left == normalized_right:
                    _add(observations, reasons, classes, "register_operand_or_schedule_differs", "register_schedule",
                         {"instruction_index": index, "mnemonic": lm,
                          "original_operands": lo, "candidate_operands": ro})
                elif not lmem and not rmem:
                    _add(observations, reasons, classes, "nonregister_operand_differs", "unknown",
                         {"instruction_index": index, "mnemonic": lm})

    left_relocs = [(row.get("offset"), row.get("symbol"), row.get("coff_type"), row.get("raw_addend"))
                   for row in original_relocations if isinstance(row, Mapping)]
    right_relocs = [(row.get("offset"), row.get("symbol"), row.get("coff_type"), row.get("raw_addend"))
                    for row in candidate_relocations if isinstance(row, Mapping)]
    if complete_pair and left_relocs != right_relocs:
        _add(observations, reasons, classes, "relocation_identity_or_addend_differs", "relocation_targets",
             {"original_relocations": left_relocs, "candidate_relocations": right_relocs,
              "evidence_level": "unlinked_symbolic_observation"})

    if not observations:
        classes.add("unknown")
        reasons.add("no_local_difference_observed")
        observations.append({"reason_code": "no_local_difference_observed"})
    if not original_complete or not candidate_complete:
        classes.add("unknown")
    omitted = [0]
    bounded_observations = _bounded_copy(observations[:_MAX_OBSERVATIONS], omitted)
    if len(observations) > _MAX_OBSERVATIONS:
        omitted[0] += len(observations) - _MAX_OBSERVATIONS
    copied_measurements = {
        "original": _bounded_copy(dict(original), omitted),
        "candidate": _bounded_copy(dict(candidate), omitted),
    }
    return {
        "schema_version": 1,
        "status": "unknown" if not original_complete or not candidate_complete else "advisory",
        "classifications": sorted(classes),
        "reason_codes": sorted(reasons),
        "observations": bounded_observations,
        "measurements": copied_measurements,
        "omitted_measurement_items": omitted[0],
        "diagnostic_state": {
            "original_truncated": _truncated(original) or not original_complete,
            "candidate_truncated": _truncated(candidate) or not candidate_complete,
            "original_decode_status": opart.get("decode_status"),
            "candidate_decode_status": cpart.get("decode_status"),
        },
        "next_investigation": [_NEXT[key] for key in sorted(classes)],
        "limitations": [
            "instruction rows are unlinked linear observations and may include padding/data",
            "memory displacements are not proof of C++ member offsets or object ownership",
            "symbolic relocation differences are not resolved target differences",
            "advisory only; no semantic equivalence, structure, lifetime, or acceptance claim",
        ],
    }


def triage_report(report):
    """Adapt a strict row, paired diagnostics, or existing candidate-deck report.

    Candidate-deck rows are joined only by their existing name/flags fields.
    The baseline diagnostic for the same flag set is compared with each
    candidate diagnostic; absent or omitted diagnostics stay unknown. The
    adapter does not rank, select, or promote a candidate.
    """
    if not isinstance(report, Mapping):
        row = triage_mismatch(comparison=report)
        return _report([{"name": "input", "flags": [], "entity_id": None, "triage": row}], 0)
    if "original" in report or "candidate" in report:
        row = triage_mismatch(original=report.get("original"), candidate=report.get("candidate"))
        return _report([{"name": report.get("name", "pair") if isinstance(report.get("name", "pair"), str) else "pair",
                         "flags": report.get("flags", []),
                         "entity_id": report.get("entity_id"), "triage": row}], 0)
    if "boundary" in report or "bytes" in report or "references" in report:
        row = triage_mismatch(comparison=report)
        return _report([{"name": report.get("name", "comparison") if isinstance(report.get("name", "comparison"), str) else "comparison",
                         "flags": report.get("flags", []),
                         "entity_id": report.get("entity_id"), "triage": row}], 0)
    if report.get("operation") != "candidate_deck":
        row = _unknown("report_shape_unrecognized", comparison=report)
        return _report([{"name": "report", "flags": [], "entity_id": report.get("entity_id"),
                         "triage": row}], 0)

    diagnostics = report.get("compiled_diagnostics")
    diagnostic_entries = (diagnostics.get("entries", [])
                          if isinstance(diagnostics, Mapping) and isinstance(diagnostics.get("entries"), list)
                          else [])
    baselines = report.get("baseline", [])
    candidates = report.get("candidates", [])
    if not isinstance(baselines, list):
        baselines = []
    if not isinstance(candidates, list):
        candidates = []
    baseline_diag = {}
    candidate_diag = {}
    for entry in diagnostic_entries[:_MAX_LIST]:
        entry_flags = _valid_flags(entry.get("flags")) if isinstance(entry, Mapping) else None
        if not isinstance(entry, Mapping) or entry_flags is None:
            continue
        key = tuple(entry_flags)
        if entry.get("name") == "baseline":
            baseline_diag[key] = entry
        else:
            candidate_diag[(entry.get("name"), key)] = entry

    result_entries = []
    omitted_count = max(0, len(candidates) - _MAX_LIST)
    diagnostic_omissions = diagnostics.get("omitted_trials", 0) if isinstance(diagnostics, Mapping) else 0
    if type(diagnostic_omissions) is not int or diagnostic_omissions < 0:
        diagnostic_omissions = 0
    for candidate_row in candidates[:_MAX_LIST]:
        if not isinstance(candidate_row, Mapping):
            omitted_count += 1
            continue
        raw_name = candidate_row.get("name", "candidate")
        name = raw_name if isinstance(raw_name, str) and len(raw_name) <= 256 else "candidate"
        raw_flags = candidate_row.get("flags", [])
        flags = _valid_flags(raw_flags)
        key = tuple(flags) if flags is not None else None
        original_diag = baseline_diag.get(key)
        candidate_entry = candidate_diag.get((name, key)) if key is not None else None
        if flags is None:
            triage = _unknown("candidate_flags_malformed", original=original_diag,
                              candidate=candidate_entry)
        elif original_diag is not None and candidate_entry is not None:
            triage = triage_mismatch(original=original_diag, candidate=candidate_entry)
        else:
            target = candidate_row.get("target")
            if isinstance(target, Mapping) and any(field in target for field in ("boundary", "bytes", "references")):
                triage = triage_mismatch(comparison=target)
            else:
                triage = _unknown("compiled_diagnostic_missing", original=original_diag,
                                  candidate=candidate_entry)
        result_entries.append({
            "name": name, "flags": flags if flags is not None else [],
            "entity_id": candidate_row.get("target", {}).get("entity_id", report.get("entity_id"))
            if isinstance(candidate_row.get("target"), Mapping) else report.get("entity_id"),
            "triage": triage,
        })
    result = _report(result_entries, omitted_count)
    result["omitted_diagnostic_trials"] = diagnostic_omissions
    return result


def _report(entries, omitted):
    counts = Counter()
    for entry in entries:
        triage = entry.get("triage", {})
        counts[triage.get("status", "unknown")] += 1
        for category in triage.get("classifications", ["unknown"]):
            counts["classification:" + category] += 1
    omitted_details = [0]
    bounded_entries = _bounded_copy(entries[:_MAX_LIST], omitted_details)
    return {
        "schema_version": 1,
        "status": "advisory",
        "advisory_only": True,
        "entries": bounded_entries,
        "omitted_entries": omitted + max(0, len(entries) - _MAX_LIST),
        "omitted_detail_items": omitted_details[0],
        "counts": dict(sorted(counts.items())),
        "acceptance": "not_evaluated",
        "limitations": ["triage is descriptive only; it does not score or promote a candidate",
                        "unknown and truncated measurements remain unknown"],
    }


def _valid_flags(value):
    if not isinstance(value, list) or len(value) > 64:
        return None
    if any(not isinstance(flag, str) or len(flag) > 128 for flag in value):
        return None
    return value
