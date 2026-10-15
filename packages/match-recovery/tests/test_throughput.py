"""Explicit, bounded descriptive throughput reports."""

import copy

import pytest

from match_recovery.throughput import (source_measurement_batch, summarize_attempts,
                                       throughput_report)


IDS = [{"identity": f"fn-{i}", "original_fingerprint": f"orig-{i}"} for i in range(6)]


def batch(source, outcomes, *, image="image-a", toolchain="toolchain-a"):
    return {
        "source_fingerprint": source,
        "original_image_fingerprint": image,
        "toolchain_fingerprint": toolchain,
        "observations": [
            {"identity": f"fn-{i}", "original_fingerprint": f"orig-{i}",
             "source_fingerprint": source, "result": result}
            for i, result in outcomes.items()
        ],
    }


def test_separates_exact_movement_implementation_coverage_and_scaffolds():
    before = batch("source-before", {0: "mismatch", 1: "strict_exact", 2: "strict_exact", 3: "unknown", 4: "mismatch"})
    after = batch("source-after", {0: "strict_exact", 1: "mismatch", 2: "strict_exact", 3: "strict_exact", 4: "strict_exact"})
    report = throughput_report(
        IDS, before, after,
        implementation_before={"fn-0": "genuine", "fn-1": "genuine", "fn-2": "scaffold"},
        implementation_after={"fn-0": "genuine", "fn-1": "genuine", "fn-2": "scaffold",
                             "fn-3": "genuine", "fn-4": "placeholder"},
        timings={"worker_elapsed_seconds": 20, "wall_elapsed_seconds": 40,
                 "compile_elapsed_seconds": 7},
    )
    assert report["denominator"]["original_identity_count"] == 6
    assert report["measurements"]["newly_verified_exact_genuine_identities"] == ["fn-0"]
    assert report["measurements"]["lost_exact_identities"] == ["fn-1"]
    assert report["measurements"]["current_strict_exact_genuine_identities"] == ["fn-0", "fn-3"]
    assert report["implementation_coverage"]["after"]["genuine_implemented"] == 3
    assert report["implementation_coverage"]["after"]["scaffolds_remaining"] == {
        "count": 1, "identities": ["fn-2"]}
    assert report["implementation_coverage"]["after"]["all_original_identity_denominator"] == 6
    assert report["implementation_coverage"]["after"]["coverage_fraction_of_all_original_identities"] == 0.5
    assert report["timing"]["compile_elapsed_seconds"] == 7
    assert report["rates"]["newly_verified_exact_genuine_per_worker_second"] == pytest.approx(0.05)
    assert report["rates"]["newly_verified_exact_genuine_per_wall_second"] == pytest.approx(0.025)
    assert report["persisted"] is False and report["ready_for_collection"] is False
    assert "historical BYTE_EXACT claims are not current measurements and are not consumed" in report["limitations"]


def test_missing_unknown_and_stale_rows_remain_visible_in_original_denominator():
    before = batch("src", {0: "mismatch", 1: "unknown", 2: "strict_exact", 3: "strict_exact"})
    after = batch("src2", {0: "strict_exact", 1: "strict_exact", 2: "strict_exact", 3: "mismatch"})
    after["observations"][0]["source_fingerprint"] = "stale-row"
    after["observations"][1]["original_fingerprint"] = "stale-original"
    report = throughput_report(IDS, before, after)
    assert report["denominator"]["original_identity_count"] == 6
    assert report["comparison"]["row_statuses"] == {
        "fn-0": "stale", "fn-1": "stale", "fn-2": "comparable",
        "fn-3": "comparable", "fn-4": "missing", "fn-5": "missing"}
    assert report["comparison"]["row_counts"] == {
        "comparable": 2, "unknown": 0, "stale": 2, "noncomparable": 0, "missing": 2}
    assert report["measurements"]["newly_verified_exact_genuine_count"] == 0
    assert report["rates"]["newly_verified_exact_genuine_per_worker_second"] is None


def test_image_or_toolchain_change_blocks_all_exactness_delta():
    report = throughput_report(IDS[:1], batch("a", {0: "mismatch"}),
                               batch("b", {0: "strict_exact"}, toolchain="toolchain-b"))
    assert report["comparison"]["batches_comparable"] is False
    assert report["comparison"]["row_statuses"]["fn-0"] == "noncomparable"
    assert report["measurements"]["newly_verified_exact_genuine_count"] == 0


def test_elapsed_rates_are_never_inferred_from_compile_time_or_zero():
    report = throughput_report(IDS[:1], batch("a", {0: "mismatch"}),
                               batch("b", {0: "strict_exact"}),
                               implementation_after={"fn-0": "genuine"},
                               timings={"compile_elapsed_seconds": 2,
                                        "worker_elapsed_seconds": 0})
    assert report["rates"] == {
        "newly_verified_exact_genuine_per_worker_second": None,
        "newly_verified_exact_genuine_per_wall_second": None,
        "newly_measured_masked_exact_genuine_per_worker_second": None,
        "newly_measured_masked_exact_genuine_per_wall_second": None,
        "basis": "explicit elapsed seconds only; compile time is separate and is not substituted"}


def test_input_fingerprint_covers_observations_and_implementation_classification():
    old = batch("a", {0: "mismatch"})
    new = batch("b", {0: "strict_exact"})
    first = throughput_report(IDS[:1], old, new, implementation_after={"fn-0": "genuine"})
    changed_observation = copy.deepcopy(new)
    changed_observation["observations"][0]["result"] = "mismatch"
    second = throughput_report(IDS[:1], old, changed_observation,
                               implementation_after={"fn-0": "genuine"})
    changed_implementation = throughput_report(IDS[:1], old, new,
                                                implementation_after={"fn-0": "placeholder"})
    assert first["input_fingerprint"] != second["input_fingerprint"]
    assert first["input_fingerprint"] != changed_implementation["input_fingerprint"]


def test_attempts_repeat_only_with_identical_three_fingerprint_key():
    attempts = [
        {"source_fingerprint": "s", "toolchain_fingerprint": "t", "hypothesis_fingerprint": "h", "outcome": "miss"},
        {"source_fingerprint": "s", "toolchain_fingerprint": "t", "hypothesis_fingerprint": "h", "outcome": "miss"},
        {"source_fingerprint": "s2", "toolchain_fingerprint": "t", "hypothesis_fingerprint": "h", "outcome": "pass"},
    ]
    summary = summarize_attempts(attempts)
    assert summary["attempt_count"] == 3 and summary["fingerprint_group_count"] == 2
    repeated = next(group for group in summary["groups"] if group["source_fingerprint"] == "s")
    assert repeated["attempt_count"] == 2 and repeated["repeat_attempt_count"] == 1
    assert repeated["outcomes"] == {"miss": 2}
    with pytest.raises(ValueError, match="bounds"):
        summarize_attempts(attempts, max_attempts=2)
    with pytest.raises(ValueError, match="too many distinct"):
        summarize_attempts(attempts, max_groups=1)


@pytest.mark.parametrize("bad", [-1, float("inf"), float("nan"), True, "2"])
def test_invalid_timing_refuses_instead_of_emitting_rate(bad):
    with pytest.raises(ValueError, match="worker_elapsed_seconds"):
        throughput_report(IDS[:1], batch("a", {0: "mismatch"}), batch("b", {0: "strict_exact"}),
                          timings={"worker_elapsed_seconds": bad})


@pytest.mark.parametrize("rows", [
    [{"identity": "fn-0", "original_fingerprint": "x"},
     {"identity": "fn-0", "original_fingerprint": "x"}],
])
def test_duplicate_original_identity_denominator_refuses(rows):
    with pytest.raises(ValueError, match="duplicate original identity"):
        throughput_report(rows, batch("a", {}), batch("b", {}))


def test_duplicate_or_out_of_denominator_measurements_refuse():
    duplicate = batch("a", {0: "strict_exact"})
    duplicate["observations"].append(duplicate["observations"][0])
    with pytest.raises(ValueError, match="duplicate before observation"):
        throughput_report(IDS, duplicate, batch("b", {}))
    outside = batch("a", {})
    outside["observations"].append({"identity": "foreign", "original_fingerprint": "x",
                                    "source_fingerprint": "a", "result": "strict_exact"})
    with pytest.raises(ValueError, match="outside the original identity denominator"):
        throughput_report(IDS, outside, batch("b", {}))


def test_native_source_report_adapter_counts_masked_matches_separately_from_strict_exact():
    report = {
        "schema_version": 1,
        "operation": "measure",
        "result": "fail",  # Aggregate status is deliberately not treated as per-function proof.
        "verification": "historical_masked_bytes_only",
        "input_fingerprint": "a" * 64,
        "selected": 2,
        "counts": {"pass": 1, "fail": 1},
        "measurements": [
            {"path": "src/a.cpp", "address": "0x1000", "result": "pass", "verdict": "BYTE_EXACT"},
            {"path": "src/b.cpp", "address": "0x2000", "result": "fail", "verdict": "MISMATCH"},
        ],
    }
    bindings = {
        ("src/a.cpp", "0x1000"): {"identity": "fn-0", "original_fingerprint": "orig-0"},
        ("src/b.cpp", "0x2000"): {"identity": "fn-1", "original_fingerprint": "orig-1"},
    }
    before = source_measurement_batch(report, bindings, source_fingerprint="source-a",
                                     original_image_fingerprint="image-a", toolchain_fingerprint="tc-a")
    after_report = copy.deepcopy(report)
    after_report["input_fingerprint"] = "b" * 64
    after_report["measurements"][1].update(result="pass", verdict="BYTE_EXACT")
    after_report["result"] = "pass"
    after_report["counts"] = {"pass": 2}
    after = source_measurement_batch(after_report, bindings, source_fingerprint="source-b",
                                     original_image_fingerprint="image-a", toolchain_fingerprint="tc-a")
    result = throughput_report(
        [{"identity": "fn-0", "original_fingerprint": "orig-0"},
         {"identity": "fn-1", "original_fingerprint": "orig-1"}],
        before, after, implementation_after={"fn-0": "genuine", "fn-1": "genuine"})
    assert result["measurements"]["newly_verified_exact_genuine_count"] == 0
    assert result["measurements"]["newly_measured_masked_exact_genuine_identities"] == ["fn-1"]
    assert result["comparison"]["batches_comparable"] is True


def test_strict_to_masked_is_loss_of_strict_evidence_not_new_masked_gain():
    before = batch("before", {0: "strict_exact"})
    after = batch("after", {0: "masked_exact"})
    report = throughput_report(IDS[:1], before, after,
                               implementation_after={"fn-0": "genuine"})
    assert report["measurements"]["lost_exact_identities"] == ["fn-0"]
    assert report["measurements"]["strict_to_masked_exact_identities"] == ["fn-0"]
    assert report["measurements"]["newly_measured_masked_exact_genuine_count"] == 0


def test_native_source_adapter_requires_exact_selected_identity_binding_and_valid_receipt():
    report = {"schema_version": 1, "operation": "measure", "result": "pass",
              "verification": "historical_masked_bytes_only", "input_fingerprint": "c" * 64,
              "selected": 1, "counts": {"pass": 1},
              "measurements": [{"path": "src/a.cpp", "address": "0x1000",
                                "result": "pass", "verdict": "BYTE_EXACT"}]}
    with pytest.raises(ValueError, match="same selected rows"):
        source_measurement_batch(report, {("src/a.cpp", "0x1000"): {
            "identity": "fn-0", "original_fingerprint": "o"},
            ("extra.cpp", "0x2000"): {"identity": "fn-1", "original_fingerprint": "p"}}, source_fingerprint="s", original_image_fingerprint="i",
                                toolchain_fingerprint="t")
    report["input_fingerprint"] = "not-a-digest"
    with pytest.raises(ValueError, match="valid input fingerprint"):
        source_measurement_batch(report, {("src/a.cpp", "0x1000"): {
            "identity": "fn-0", "original_fingerprint": "o"}}, source_fingerprint="s",
            original_image_fingerprint="i", toolchain_fingerprint="t")
