"""Exact equality for guarded parsed inputs, independent of native linking."""


def same(left, right, seen=None):
    """Exact parsed-state equality, including scalar types (False is not 0)."""
    if type(left) is not type(right):
        return False
    if left is right:
        return True
    if seen is None:
        seen = set()
    pair = id(left), id(right)
    if pair in seen:
        return True
    seen.add(pair)
    if isinstance(left, dict):
        return len(left) == len(right) and all(same(a, b, seen) and same(x, y, seen)
            for (a, x), (b, y) in zip(left.items(), right.items()))
    if isinstance(left, (list, tuple)):
        return len(left) == len(right) and all(same(a, b, seen) for a, b in zip(left, right))
    if isinstance(left, (set, frozenset)):
        return {(type(value), value) for value in left} == {(type(value), value) for value in right}
    if hasattr(left, "__dict__"):
        return same(left.__dict__, right.__dict__, seen)
    return left == right

