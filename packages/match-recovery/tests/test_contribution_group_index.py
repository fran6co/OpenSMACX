"""Grouping acceleration preserves the old relation, not fold authority."""

from collections import Counter
from copy import deepcopy

import pytest

from match_recovery.ordinary_allocation_coverage import _contribution_groups


@pytest.mark.parametrize("groups,expected", [(["a", "a"], [True, True]),
    (["a", "b"], [False, False]), (["a", None], [False, False]),
    ([None, None], [False, False]), (["a", "a", "b"], [False, False, False])])
@pytest.mark.parametrize("failed_assigned", [False, True])
def test_group_index_preserves_every_assigned_competitor(groups, expected, failed_assigned):
    sections = [{"contribution": 7, "fold_group_id": group, "intersections": [7],
                 "input_consistency": "fail" if failed_assigned and i == 0 else "pass"}
                for i, group in enumerate(groups)]
    # A failed join still contributes an intersection user. It does not
    # acquire an assigned group merely because another claimant has one.
    sections.append({"intersections": [7], "input_consistency": "unknown"})
    before = deepcopy(sections)
    index = _contribution_groups(sections)
    observed = []
    for row in sections[:-1]:
        old = {other.get("fold_group_id") for other in sections
               if other.get("contribution") == row["contribution"]}
        assert index[row["contribution"]] == old
        observed.append(row.get("fold_group_id") is not None and len(old) == 1 and None not in old)
    assert observed == expected
    assert sections == before
    users = Counter(number for row in sections for number in row.get("intersections", ()))
    assert users[7] == len(groups) + 1  # Group sets never replace multiplicity.


def test_many_contributions_need_one_scan_and_keep_independent_groups():
    class Counted(list):
        scans = 0
        def __iter__(self):
            self.scans += 1
            return super().__iter__()
    sections = Counted({"contribution": i // 2, "fold_group_id": f"group-{i // 2}"}
                       for i in range(8192))
    index = _contribution_groups(sections)
    assert sections.scans == 1
    assert len(index) == 4096
    assert all(index[number] == {f"group-{number}"} for number in range(4096))
