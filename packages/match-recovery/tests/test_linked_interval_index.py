"""Full scan equivalence, multiplicity and bounded AVL work controls."""

import math
import random

import pytest

from match_recovery import linked_interval_index as implementation
from match_recovery.linked_interval_index import IntervalIndex


def scan(rows, begin, end):
    return [row for row in rows if row[1] < end and begin < row[2]]


def validate(entry):
    if entry is None:
        return [], 0, None, None
    left, lh, lmin, lmax = validate(entry.left)
    right, rh, rmin, rmax = validate(entry.right)
    assert abs(lh - rh) <= 1
    assert entry.height == 1 + max(lh, rh)
    minimum = min(value for value in (entry.begin, lmin, rmin) if value is not None)
    maximum = max(value for value in (entry.end, lmax, rmax) if value is not None)
    assert entry.minimum == minimum and entry.maximum == maximum
    keys = left + [(entry.begin, entry.ordinal)] + right
    assert keys == sorted(keys)
    return keys, entry.height, minimum, maximum


def test_equal_nested_adjacent_and_repeated_ranges_preserve_insertion_order():
    rows = [(7, 50, 100), (1, 0, 200), (7, 50, 100), (3, 100, 101), (9, 60, 70), (2, 0, 50)]
    index = IntervalIndex()
    for row in rows:
        index.insert(*row)
    for begin, end in ((0, 1), (49, 50), (50, 51), (60, 70), (99, 100), (100, 101), (200, 201)):
        assert index.overlaps(begin, end) == scan(rows, begin, end)
    assert index.overlaps(0, 201) == rows
    assert len(index) == len(rows)
    validate(index._root)


@pytest.mark.parametrize("seed", range(6))
def test_randomized_incremental_queries_equal_complete_list_scan(seed):
    randomizer = random.Random(seed)
    index, rows = IntervalIndex(), []
    for _ in range(600):
        begin = randomizer.randrange(500)
        end = begin + randomizer.randrange(1, 500)
        row = randomizer.randrange(20), begin, end
        index.insert(*row)
        rows.append(row)
        left = randomizer.randrange(1200)
        right = left + randomizer.randrange(1, 700)
        assert index.overlaps(left, right) == scan(rows, left, right)
    validate(index._root)


@pytest.mark.parametrize("order", ["ascending", "descending", "equal", "alternating"])
def test_adversarial_insertions_remain_balanced_and_logarithmic(monkeypatch, order):
    count = 4096
    starts = list(range(count))
    if order == "descending":
        starts.reverse()
    elif order == "equal":
        starts = [100] * count
    elif order == "alternating":
        starts = [value for pair in zip(range(count // 2), range(count - 1, count // 2 - 1, -1)) for value in pair]
    operations = 0
    original = implementation._insert
    def measured(*args):
        nonlocal operations
        operations += 1
        return original(*args)
    monkeypatch.setattr(implementation, "_insert", measured)
    index = IntervalIndex()
    rows = [(number, begin, begin + count - number + 1) for number, begin in enumerate(starts)]
    for row in rows:
        index.insert(*row)
    assert operations < count * (2 * math.ceil(math.log2(count)) + 2)
    assert validate(index._root)[1] < 2 * math.ceil(math.log2(count))
    assert index.overlaps(0, 2 * count + 1) == rows


def test_sparse_queries_prune_the_population_instead_of_scanning_it(monkeypatch):
    index = IntervalIndex()
    count = 8192
    for number in range(count):
        index.insert(number, number * 4, number * 4 + 2)
    operations = 0
    original = implementation._collect
    def measured(*args):
        nonlocal operations
        operations += 1
        return original(*args)
    monkeypatch.setattr(implementation, "_collect", measured)
    for number in (0, count // 3, count // 2, count - 1):
        before = operations
        assert index.overlaps(number * 4, number * 4 + 1) == [(number, number * 4, number * 4 + 2)]
        assert operations - before < 4 * math.ceil(math.log2(count))
        before = operations
        assert index.overlaps(number * 4 + 2, number * 4 + 4) == []
        assert operations - before < 4 * math.ceil(math.log2(count))
    before = operations
    assert index.overlaps(count * 4, count * 4 + 1) == []
    assert operations - before == 1


@pytest.mark.parametrize("begin,end", [(0, 0), (5, 4), (-1, 4), (False, 4), (0, True),
    (0.0, 4), (0, "4"), (0, (1 << 64) + 1)])
def test_invalid_or_empty_intervals_refuse_without_mutation(begin, end):
    index = IntervalIndex()
    index.insert(1, 0, 4)
    with pytest.raises(ValueError):
        index.insert(2, begin, end)
    with pytest.raises(ValueError):
        index.overlaps(begin, end)
    assert len(index) == 1 and index.overlaps(0, 4) == [(1, 0, 4)]


@pytest.mark.parametrize("node", [True, -1, 1.0, "1", None])
def test_invalid_node_ids_refuse_without_mutation(node):
    index = IntervalIndex()
    with pytest.raises(ValueError):
        index.insert(node, 0, 1)
    assert len(index) == 0 and index.overlaps(0, 1) == []


@pytest.mark.parametrize("options", [{"max_intervals": True}, {"max_intervals": 0},
    {"max_intervals": 1000001}, {"coordinate_limit": True}, {"coordinate_limit": 0},
    {"coordinate_limit": (1 << 64) + 1}])
def test_resource_configuration_has_exact_bounds(options):
    with pytest.raises(ValueError):
        IntervalIndex(**options)


def test_capacity_and_coordinate_edges_preserve_every_existing_interval():
    index = IntervalIndex(max_intervals=2, coordinate_limit=16)
    index.insert(0, 0, 16)
    index.insert(0, 15, 16)
    with pytest.raises(ValueError, match="capacity"):
        index.insert(0, 15, 16)
    with pytest.raises(ValueError, match="interval"):
        index.insert(1, 16, 17)
    assert index.overlaps(15, 16) == [(0, 0, 16), (0, 15, 16)]
    rows = index.overlaps(0, 16)
    rows.clear()
    assert len(index.overlaps(0, 16)) == 2
