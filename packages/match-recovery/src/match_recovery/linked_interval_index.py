"""Deterministic interval lookup preserving every insertion and overlap.

An AVL tree gives logarithmic insertion and prunes queries by subtree extents.
Hits are sorted by insertion ordinal, costing O(k log k) for k results; this is
not an interval ownership, deduplication or alias-selection mechanism.
"""

from dataclasses import dataclass


@dataclass(slots=True)
class _Entry:
    node: int
    begin: int
    end: int
    ordinal: int
    left: object = None
    right: object = None
    height: int = 1
    minimum: int = 0
    maximum: int = 0


def _height(entry):
    return 0 if entry is None else entry.height


def _refresh(entry):
    entry.height = 1 + max(_height(entry.left), _height(entry.right))
    entry.minimum = entry.left.minimum if entry.left is not None else entry.begin
    entry.maximum = max(entry.end, entry.left.maximum if entry.left is not None else entry.end,
                        entry.right.maximum if entry.right is not None else entry.end)
    return entry


def _rotate_left(entry):
    pivot = entry.right
    entry.right, pivot.left = pivot.left, entry
    _refresh(entry)
    return _refresh(pivot)


def _rotate_right(entry):
    pivot = entry.left
    entry.left, pivot.right = pivot.right, entry
    _refresh(entry)
    return _refresh(pivot)


def _balance(entry):
    _refresh(entry)
    difference = _height(entry.left) - _height(entry.right)
    if difference > 1:
        if _height(entry.left.left) < _height(entry.left.right):
            entry.left = _rotate_left(entry.left)
        return _rotate_right(entry)
    if difference < -1:
        if _height(entry.right.right) < _height(entry.right.left):
            entry.right = _rotate_right(entry.right)
        return _rotate_left(entry)
    return entry


def _insert(entry, added):
    if entry is None:
        return added
    # Ordinals strictly increase: a new equal-start interval belongs after all
    # previous equal-start intervals, including after balancing rotations.
    if added.begin < entry.begin:
        entry.left = _insert(entry.left, added)
    else:
        entry.right = _insert(entry.right, added)
    return _balance(entry)


def _collect(entry, begin, end, hits):
    if entry is None or entry.maximum <= begin or entry.minimum >= end:
        return
    _collect(entry.left, begin, end, hits)
    if entry.begin < end and begin < entry.end:
        hits.append(entry)
    _collect(entry.right, begin, end, hits)


class IntervalIndex:
    """Bounded half-open integer intervals; queries return inserted triples.

    Equal ranges, repeated node IDs and repeated insertions remain distinct.
    The coordinate ceiling is exclusive for starts and inclusive for ends.
    Empty or malformed intervals refuse without changing the index.
    """

    def __init__(self, *, max_intervals=1000000, coordinate_limit=1 << 64):
        if type(max_intervals) is not int or not 1 <= max_intervals <= 1000000:
            raise ValueError("interval index requires a bounded positive capacity")
        if type(coordinate_limit) is not int or not 1 <= coordinate_limit <= 1 << 64:
            raise ValueError("interval index requires a bounded positive coordinate limit")
        self._root = None
        self._count = 0
        self._limit = max_intervals
        self._coordinate_limit = coordinate_limit

    def __len__(self):
        return self._count

    def _interval(self, begin, end):
        if (type(begin) is not int or type(end) is not int
                or not 0 <= begin < end <= self._coordinate_limit):
            raise ValueError("interval index requires a positive bounded half-open interval")

    def insert(self, node, begin, end):
        self._interval(begin, end)
        if type(node) is not int or node < 0:
            raise ValueError("interval index requires a nonnegative integer node identity")
        if self._count >= self._limit:
            raise ValueError("interval index exceeds its capacity")
        added = _Entry(node, begin, end, self._count, minimum=begin, maximum=end)
        self._root = _insert(self._root, added)
        self._count += 1

    def overlaps(self, begin, end):
        self._interval(begin, end)
        hits = []
        _collect(self._root, begin, end, hits)
        hits.sort(key=lambda entry: entry.ordinal)
        return [(entry.node, entry.begin, entry.end) for entry in hits]
