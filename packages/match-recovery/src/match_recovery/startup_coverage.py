"""Ephemeral startup coverage capabilities, never report-based authority.

The native application-object producer owns authorization and linked evidence.
This module binds its selected slots to one exact mutable COFF instance and
its live verification epoch. Serializing or copying a capability cannot retain
authority, and an equal second object cannot borrow another trial's coverage.
"""

from copy import deepcopy
from weakref import WeakKeyDictionary, ref

from .coff_replica import object_signature


class StartupCoverage:
    __slots__ = ("__weakref__",)

    def __new__(cls, *args, **kwargs):
        raise TypeError("startup coverage is produced only by a live native application context")

    def __reduce__(self):
        raise TypeError("startup coverage is not serializable")


_live_coverage = WeakKeyDictionary()


def _mint_startup(obj, slots, fresh, *, fpo=(), trial=None):
    """Private producer hook; parsed reports must never call this directly."""
    fresh()
    value = object.__new__(StartupCoverage)
    _live_coverage[value] = (ref(obj), object_signature(obj), deepcopy(obj.__dict__), tuple(slots),
                             deepcopy(tuple(fpo)), fresh, trial)
    fresh()
    return value


def _consume(coverage, obj, *, producer=None, trial=None):
    if type(coverage) is not StartupCoverage or coverage not in _live_coverage:
        raise ValueError("startup coverage requires an authentic live native capability")
    owner, signature, snapshot, slots, fpo, fresh, original_trial = _live_coverage[coverage]
    try:
        fresh()
        if producer is not None and fresh is not producer:
            raise ValueError("startup coverage belongs to another native producer epoch")
        if producer is not None and (trial is None or original_trial is None or trial != original_trial):
            raise ValueError("startup coverage belongs to another native object trial")
        if owner() is not obj or obj.__dict__ != snapshot or object_signature(obj) != signature:
            raise ValueError("startup coverage belongs to another or changed compiler object")
        return slots, deepcopy(fpo)
    finally:
        fresh()


def _consume_for_producer(coverage, obj, producer, *, trial):
    """Private composition must not borrow another graph's broader permission."""
    return _consume(coverage, obj, producer=producer, trial=trial)


def consume_startup(coverage, obj):
    """Read exact immutable slots only while their original epoch is live."""
    return _consume(coverage, obj)[0]


def consume_startup_fpo(coverage, obj):
    """Read exact native-bound child selections, not a debug-name exemption."""
    return _consume(coverage, obj)[1]
