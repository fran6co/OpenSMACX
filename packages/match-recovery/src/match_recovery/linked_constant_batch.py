"""Invocation-local optional constant lookups inside a guarded graph epoch.

Only absence from the complete native population's constant/weak-path interest
set is accelerated. This is neither a persisted verdict nor selection authority;
the enclosing observer must retain its live source, image and artifact guards.
"""

from contextlib import contextmanager
from copy import deepcopy
from pathlib import Path

from .compile_file_evidence import _identity, _physical
from .linked_objects import SelectedObjects, _constant_names


@contextmanager
def constant_lookups(objects):
    """Yield an expiring callable, or no acceleration for nonnative objects.

The full physical population and immutable indices are checked at entry/exit,
including exceptional exit. Present constants and required lookups retain the
ordinary resolver's complete checks. No partial result survives a refused epoch.
"""
    if type(objects) is not SelectedObjects:
        yield None
        return
    from .common_allocation import _policy
    from .linked_context import _same

    records, _ = objects._constant_population()
    interest = frozenset(objects._constant_interest)
    physical = deepcopy(objects._constant_physical)
    caches = {"_files", "_archives", "_resolved", "_constant_policy_gaps"}

    def indices():
        return {key: value for key, value in objects.__dict__.items() if key not in caches}

    original = deepcopy(indices())
    original_policy = deepcopy(objects._constant_policy_gaps)
    # This expected value permits the existing resolver's lazy policy cache,
    # never an arbitrary replacement that could conceal unsupported directives.
    expected_policy = (original_policy if original_policy is not None else
                       _policy(deepcopy(objects._constant_evidence), deepcopy(records)))
    live = True

    def fresh():
        try:
            if not _same(indices(), original):
                raise ValueError("constant lookup population or indices changed during its batch")
            policy = objects._constant_policy_gaps
            if (original_policy is not None or policy is not None) and not _same(policy, expected_policy):
                raise ValueError("constant lookup policy changed during its batch")
        finally:
            # Do not consult a potentially replaced/cleared native witness map.
            for name, expected in physical.items():
                if _identity(_physical(Path(name))) != expected:
                    raise ValueError("constant input population changed during its lookup batch")

    def lookup(names, *, optional=True):
        if not live:
            raise ValueError("constant lookup batch has expired")
        names = _constant_names(names, optional)
        if optional and not any(name in interest for name in names):
            return {name: None for name in names}
        return objects.resolve_constants(names, optional=optional)

    try:
        yield lookup
    finally:
        live = False
        fresh()
