"""Advisory mismatch clues never substitute for strict evidence."""

from copy import deepcopy

from match_recovery.mismatch_triage import triage_mismatch
from match_recovery.mismatch_triage import triage_report
from test_variants import measure, project


def diagnostic(*instructions, truncated=False, omitted_bytes=0,
               decode_status="linear_complete", relocations=()):
    rows = [
        {"offset": index * 3, "mnemonic": mnemonic,
         "operands_unlinked": operands, "relocation_offsets": []}
        for index, (mnemonic, operands) in enumerate(instructions)
    ]
    return {
        "status": "observed",
        "truncated": truncated,
        "omitted_sections": 0,
        "sections": [{
            "raw_size": len(rows) * 3 + omitted_bytes,
            "reported_bytes": len(rows) * 3 + omitted_bytes,
            "omitted_bytes": omitted_bytes,
            "undecoded_reported_bytes": 0,
            "omitted_relocations": 0,
            "omitted_decoded_instructions": 0,
            "decode_status": decode_status,
            "instructions": rows,
            "relocations": list(relocations),
        }],
    }


def test_near_exact_opcode_profile_still_reports_member_displacement_clue():
    original = diagnostic(("mov", "eax, dword ptr [esi + 0x10]"), ("ret", ""))
    candidate = diagnostic(("mov", "eax, dword ptr [esi + 0x14]"), ("ret", ""))
    result = triage_mismatch(original=original, candidate=candidate)

    assert result["status"] == "advisory"
    assert result["classifications"] == ["member_width_offset"]
    assert "memory_address_expression_differs" in result["reason_codes"]
    assert result["measurements"] == {"original": original, "candidate": candidate}
    assert "no semantic equivalence" in result["limitations"][-1]


def test_register_only_difference_is_not_equivalence_or_behavior_proof():
    original = diagnostic(("mov", "eax, ecx"), ("ret", ""))
    candidate = diagnostic(("mov", "eax, edx"), ("ret", ""))
    result = triage_mismatch(original=original, candidate=candidate)

    assert result["classifications"] == ["register_schedule"]
    assert result["reason_codes"] == ["register_operand_or_schedule_differs"]
    assert "no semantic equivalence" in " ".join(result["limitations"])


def test_strict_wrong_target_is_preserved_as_reference_evidence_not_cause():
    comparison = {
        "result": "mismatch", "boundary": "pass", "bytes": "pass",
        "references": "fail", "original_size": 5, "candidate_size": 5,
        "relocations": [{"offset": 1, "original_target": "_good", "candidate_target": "_wrong"}],
    }
    before = deepcopy(comparison)
    result = triage_mismatch(comparison=comparison)

    assert result["classifications"] == ["relocation_targets"]
    assert "strict_reference_mismatch" in result["reason_codes"]
    assert result["measurements"] == before
    assert comparison == before
    assert "does not identify a C++ cause" in " ".join(result["limitations"])


def test_truncated_diagnostics_remain_unknown_and_keep_raw_measurements():
    original = diagnostic(("nop", ""), truncated=True, omitted_bytes=6,
                         decode_status="instruction_limit")
    candidate = diagnostic(("nop", ""))
    result = triage_mismatch(original=original, candidate=candidate)

    assert result["status"] == "unknown"
    assert "unknown" in result["classifications"]
    assert result["diagnostic_state"]["original_truncated"] is True
    assert result["measurements"] == {"original": original, "candidate": candidate}
    assert "diagnostic_truncated_or_incomplete" in result["reason_codes"]


def test_incomplete_rows_produce_only_unknown_coarse_observation():
    original = diagnostic(("mov", "eax, [esi+0x10]"), truncated=True)
    candidate = diagnostic(("mov", "eax, [esi-0x10]"))
    result = triage_mismatch(original=original, candidate=candidate)

    assert result["status"] == "unknown"
    assert "member_width_offset" not in result["classifications"]
    assert "memory_address_expression_differs" not in result["reason_codes"]


def test_signed_memory_displacements_remain_distinct_and_ambiguous():
    original = diagnostic(("mov", "eax, dword ptr [esi + 4]"), ("ret", ""))
    candidate = diagnostic(("mov", "eax, dword ptr [esi - 4]"), ("ret", ""))
    result = triage_mismatch(original=original, candidate=candidate)

    obs, = [row for row in result["observations"] if row["reason_code"] == "memory_address_expression_differs"]
    assert obs["original_memory_shapes"] != obs["candidate_memory_shapes"]
    assert "scale/index/displacement are ambiguous" in obs["evidence_level"]


def test_relocation_covered_branch_operand_is_not_misreported_as_constant():
    original = diagnostic(("call", "0x10"), ("ret", ""))
    candidate = diagnostic(("call", "0x20"), ("ret", ""))
    original["sections"][0]["instructions"][0]["relocation_offsets"] = [1]
    candidate["sections"][0]["instructions"][0]["relocation_offsets"] = [1]
    result = triage_mismatch(original=original, candidate=candidate)

    assert result["classifications"] == ["relocation_targets"]
    assert "relocation_covered_operand_differs" in result["reason_codes"]
    assert "immediate_operand_differs" not in result["reason_codes"]
    assert "branch_operand_differs" not in result["reason_codes"]


def test_call_operand_difference_does_not_invent_lifetime_classification():
    original = diagnostic(("call", "0x10"), ("ret", ""))
    candidate = diagnostic(("call", "0x20"), ("ret", ""))
    result = triage_mismatch(original=original, candidate=candidate)

    assert "lifetime" not in result["classifications"]
    assert "call_operand_differs_unresolved" in result["reason_codes"]
    assert "unknown" in result["classifications"]


def test_offset_shift_prevents_ordinal_branch_target_alignment():
    original = diagnostic(("nop", ""), ("jne", "0x6"), ("ret", ""))
    candidate = diagnostic(("nop", ""), ("nop", ""), ("jne", "0x9"))
    candidate["sections"][0]["instructions"][1]["offset"] = 1
    candidate["sections"][0]["instructions"][2]["offset"] = 4
    result = triage_mismatch(original=original, candidate=candidate)

    assert "instruction_alignment_ambiguous" in result["reason_codes"]
    assert "branch_operand_differs" not in result["reason_codes"]


def test_large_measurement_copy_is_bounded_and_reports_omissions():
    comparison = {"boundary": "fail", "bytes": "fail", "references": "unknown",
                  "differing_offsets": list(range(5000))}
    result = triage_mismatch(comparison=comparison)

    assert result["omitted_measurement_items"] > 0
    assert len(result["measurements"]["differing_offsets"]) <= 256
    assert result["status"] == "advisory"


def test_observation_limit_counts_omitted_rows_once():
    original = diagnostic(*[("add", "eax, 1") for _ in range(100)])
    candidate = diagnostic(*[("add", "eax, 2") for _ in range(100)])
    result = triage_mismatch(original=original, candidate=candidate)

    assert len(result["observations"]) == 64
    assert result["omitted_measurement_items"] == 36


def test_long_operands_refuse_heuristic_and_report_raw_copy_omissions():
    original = diagnostic(("mov", "eax, " + "x" * 3000))
    candidate = diagnostic(("mov", "eax, " + "y" * 3000))
    result = triage_mismatch(original=original, candidate=candidate)

    assert result["status"] == "unknown"
    assert result["reason_codes"] == ["diagnostic_malformed"]
    assert result["omitted_measurement_items"] > 0


def test_triage_report_adapts_existing_candidate_deck_shape(project):
    report = measure(project, compiled_diagnostics=True)
    triaged = triage_report(report)

    assert triaged["advisory_only"] is True
    assert triaged["acceptance"] == "not_evaluated"
    assert [entry["name"] for entry in triaged["entries"]] == ["exact", "different"]
    assert all(set(entry) == {"name", "flags", "entity_id", "triage"}
               for entry in triaged["entries"])
    assert triaged["counts"].get("advisory", 0) + triaged["counts"].get("unknown", 0) == 2
    report["compiled_diagnostics"]["omitted_trials"] = 3
    triaged_with_omissions = triage_report(report)
    assert triaged_with_omissions["omitted_diagnostic_trials"] == 3
    assert triaged_with_omissions["omitted_entries"] == 0


def test_candidate_deck_rejects_unhashable_flag_values_as_unknown(project):
    report = measure(project, compiled_diagnostics=True)
    report["candidates"][0]["flags"] = [{"not": "a compiler flag"}]
    triaged = triage_report(report)

    assert triaged["entries"][0]["triage"]["status"] == "unknown"
    assert triaged["entries"][0]["triage"]["reason_codes"] == ["candidate_flags_malformed"]


def test_adapter_bounds_explicit_metadata_fields():
    original = diagnostic(("ret", ""))
    candidate = diagnostic(("ret", ""))
    huge = "x" * 10000
    report = triage_report({"name": huge, "flags": [huge], "entity_id": huge,
                            "original": original, "candidate": candidate})

    entry = report["entries"][0]
    assert len(entry["name"]) <= 2048
    assert len(entry["entity_id"]) <= 2048
    assert len(entry["flags"][0]) <= 2048
    assert report["omitted_detail_items"] > 0


def test_triage_report_accepts_explicit_pair_and_strict_row():
    original = diagnostic(("mov", "eax, ecx"), ("ret", ""))
    candidate = diagnostic(("mov", "eax, edx"), ("ret", ""))
    paired = triage_report({"name": "typed", "flags": ["-O2"], "entity_id": "owner",
                            "original": original, "candidate": candidate})
    strict = triage_report({"boundary": "pass", "bytes": "fail", "references": "pass",
                            "differing_offsets": [3]})

    assert paired["entries"][0]["name"] == "typed"
    assert paired["entries"][0]["triage"]["classifications"] == ["register_schedule"]
    assert strict["entries"][0]["triage"]["reason_codes"] == ["strict_bytes_differ"]



def test_missing_and_ambiguous_evidence_stays_unknown():
    missing = triage_mismatch(original=None, candidate=None)
    ambiguous = diagnostic(("ret", ""))
    ambiguous["sections"].append(deepcopy(ambiguous["sections"][0]))
    result = triage_mismatch(original=ambiguous, candidate=diagnostic(("ret", "")))

    assert missing["status"] == "unknown"
    assert missing["classifications"] == ["unknown"]
    assert result["status"] == "unknown"
    assert result["reason_codes"] == ["section_ownership_ambiguous"]
    assert result["measurements"]["original"] == ambiguous


def test_unlinked_relocation_difference_is_explicitly_not_resolved_target_proof():
    left_reloc = {"offset": 1, "symbol": "_a", "coff_type": 6, "raw_addend": 0}
    right_reloc = {"offset": 1, "symbol": "_b", "coff_type": 6, "raw_addend": 0}
    original = diagnostic(("call", "0"), relocations=(left_reloc,))
    candidate = diagnostic(("call", "0"), relocations=(right_reloc,))
    result = triage_mismatch(original=original, candidate=candidate)

    assert result["classifications"] == ["relocation_targets"]
    assert result["observations"][0]["evidence_level"] == "unlinked_symbolic_observation"
    assert "not resolved target differences" in " ".join(result["limitations"])

