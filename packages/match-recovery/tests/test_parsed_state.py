"""Shared exact parsed-state guards retain scalar types and ordered records."""

from copy import deepcopy
from dataclasses import dataclass

import pytest

from match_recovery.parsed_state import same


@pytest.mark.parametrize("left,right", [
    (False, 0), (True, 1), (1, 1.0), ([1], (1,)), ({1}, {True}),
    ({1: "value"}, {True: "value"}),
    ({"first": 1, "second": 2}, {"second": 2, "first": 1}),
])
def test_python_value_equality_does_not_hide_changed_parsed_types_or_order(left, right):
    assert left == right or isinstance(left, list)
    assert not same(left, right)


def test_deep_parsed_state_copies_match_but_nested_types_do_not():
    @dataclass
    class Parsed:
        count: int
    original = {"records": (Parsed(1),), "bytes": b"immutable", "flags": frozenset({1, 2})}
    copy = deepcopy(original)
    assert same(original, copy)
    copy["records"][0].count = True
    assert not same(original, copy)


def test_recursive_state_terminates_and_preserves_other_differences():
    original = {"count": 1}
    original["self"] = original
    copy = deepcopy(original)
    assert same(original, copy)
    copy["count"] = True
    assert not same(original, copy)
