"""Shared node budgets; capacity is not evidence of complete graph coverage."""

DEFAULT_NODES = 4096
MAX_NODES = 100000


def node_limit(value):
    """Validate an explicit budget without coercing booleans or truncating it."""
    if type(value) is not int or not 1 <= value <= MAX_NODES:
        raise ValueError("graph node budget requires an integer from 1 to 100000")
    return value
