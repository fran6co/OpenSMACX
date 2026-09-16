"""Certified ICF fold groups compose shared code allocations, never waive them."""

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.ordinary_allocation_coverage import observe_ordinary_allocations
from test_ordinary_allocation_coverage import fixture, sections


def certificate(group_id, input_identity, object_sha256, section, address, size,
                contribution, *, flags=0x60500020, pdb_flags=0x60500020, owner="unit.obj"):
    member = {"input": dict(input_identity), "object_sha256": object_sha256, "owner": owner,
              "root": {"section": section, "name": "_entry", "index": 3, "storage": 2,
                       "type": 32, "value": 0}}
    return {"check": "native_comdat_folded_members", "group_id": group_id,
            "members": [member],
            "final_storage": {"owner": owner, "address": address, "size": size,
                              "module": 0, "contribution": contribution,
                              "object_flags": flags, "pdb_flags": pdb_flags,
                              "characteristics_agreement": "pass"}}


def code_row(report):
    rows = [row for row in report["sections"] if row["classification"] == "code"]
    assert len(rows) == 1
    return rows[0]


def test_certified_member_binds_shared_storage_without_its_own_module_join(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        plain = observe_ordinary_allocations(live)
        row = code_row(plain)
        group = certificate("g1", row["input"], row["object_sha256"], row["object_section"],
                            row["envelope"]["address"], row["size"], row["contribution"])
        report = observe_ordinary_allocations(live, folded_groups=[group])
    folded = code_row(report)
    assert folded["fold_group_id"] == "g1"
    assert folded["retention"] == "pass" and folded["allocation_isolation"] == "pass"
    assert report["result"] == "pass"
    assert not any("fold" in item["reason"] for item in report["gaps"])


def test_certified_storage_must_agree_with_the_measured_envelope(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        plain = observe_ordinary_allocations(live)
        row = code_row(plain)
        group = certificate("g1", row["input"], row["object_sha256"], row["object_section"],
                            row["envelope"]["address"] + 8, row["size"], 0)
        report = observe_ordinary_allocations(live, folded_groups=[group])
    assert any("fold storage disagrees" in item["reason"] for item in report["gaps"])
    folded = code_row(report)
    assert folded["retention"] == "unknown" and "fold_group_id" not in folded
    assert report["result"] == "unknown"


def test_unexplained_producer_characteristics_still_gap_for_folded_members(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        plain = observe_ordinary_allocations(live)
        row = code_row(plain)
        group = certificate("g1", row["input"], row["object_sha256"], row["object_section"],
                            row["envelope"]["address"], row["size"], row["contribution"],
                            flags=row["flags"] ^ 0x20)
        report = observe_ordinary_allocations(live, folded_groups=[group])
    assert any("characteristics remain unexplained" in item["reason"] for item in report["gaps"])
    # Retention stands; unexplained producer flags keep the domain open exactly
    # like an own-module join with the same disagreement.
    assert code_row(report)["allocation_isolation"] == "unknown"
    assert report["result"] == "unknown"


def test_one_member_may_not_claim_two_certified_groups(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as live:
        plain = observe_ordinary_allocations(live)
        row = code_row(plain)
        first = certificate("g1", row["input"], row["object_sha256"], row["object_section"],
                            row["envelope"]["address"], row["size"], row["contribution"])
        second = certificate("g2", row["input"], row["object_sha256"], row["object_section"],
                             row["envelope"]["address"], row["size"], row["contribution"])
        with pytest.raises(ValueError, match="two certified groups"):
            observe_ordinary_allocations(live, folded_groups=[first, second])


def test_folded_query_returns_the_exact_certified_group(tmp_path):
    from match_recovery.linked_comdat_selection import comdat_selection
    from test_linked_comdat_icf import fixture as icf_fixture

    context = icf_fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.loser("dependency.obj", 2) is not None
        group, = selected.folded_groups()
        for owner in ("unit0.obj", "dependency.obj"):
            member = selected.folded(owner, 1)
            assert member["disposition"] == "certified_icf_coalescence"
            assert member["group_id"] == group["group_id"]
            assert member["final_storage"]["address"] == group["final_storage"]["address"]
        # An ordinary section outside the certified members resolves nothing.
        assert selected.folded("dependency.obj", 2) is None
