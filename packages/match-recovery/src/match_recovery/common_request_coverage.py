"""Live duplicate declaration projection, not storage or linked-effect coverage.

Only the native application-object producer can establish redundant requests.
The consumer removes exact symbol rows after authorized startup projection;
surviving references and every other compiler effect remain strict.
"""

from copy import deepcopy
from weakref import WeakKeyDictionary, ref

from .coff_replica import object_signature


class DuplicateCommonRequestCoverage:
    __slots__ = ("__weakref__",)

    def __new__(cls, *args, **kwargs):
        raise TypeError("common request coverage requires a live native producer")

    def __reduce__(self):
        raise TypeError("common request coverage is not serializable")


_live = WeakKeyDictionary()


def _requests(obj, projection):
    """Exact common occurrences reached by actually removed startup roots."""
    indices = {row["target"] for row in projection.get("covered_root_references", ())}
    result = []
    for index in sorted(indices):
        symbol = obj.symbols[index]
        if symbol.section != 0 or symbol.storage != 2 or symbol.value <= 0:
            continue
        if (symbol.type != 0 or obj.auxiliary.get(index)
                or sum(other.name == symbol.name for other in obj.symbols.values()) != 1):
            raise ValueError("common request needs one exact plain external occurrence")
        result.append(index)
    return tuple(result)


def _mint_requests(obj, startup, indices, fresh):
    """Private producer hook; a serialized observation is never authority."""
    fresh()
    value = object.__new__(DuplicateCommonRequestCoverage)
    _live[value] = (ref(obj), object_signature(obj), deepcopy(obj.__dict__), startup, tuple(indices), fresh)
    fresh()
    return value


def _project_requests(coverage, obj, value, projection, *, startup):
    if type(coverage) is not DuplicateCommonRequestCoverage or coverage not in _live:
        raise ValueError("common request projection requires authentic live native coverage")
    owner, signature, snapshot, original_startup, indices, fresh = _live[coverage]
    try:
        fresh()
        if (owner() is not obj or object_signature(obj) != signature or obj.__dict__ != snapshot
                or startup is not original_startup):
            raise ValueError("common request coverage belongs to another or changed compiler/startup object")
        if not indices or not set(indices) <= set(_requests(obj, projection)):
            raise ValueError("common request lacks its actual removed startup-root reference")
        removed = []
        for index in indices:
            symbol = obj.symbols[index]
            name = symbol.name
            # Identity includes request size. Never normalize undefined/common
            # edges or hide a same-name alias by filtering just one index.
            if any(target[0] == name for section in value["sections"] for _, target, _ in section[5]):
                raise ValueError("common request still has a surviving relocation reference")
            if any(aux[0] == "weak" and aux[1][0] == name for _, aux in value["symbols"]):
                raise ValueError("common request still has a surviving weak fallback reference")
            row = ((name, symbol.value, "0", 0, 2), ("none",))
            if value["symbols"].count(row) != 1:
                raise ValueError("common request lacks one exact residual declaration row")
            removed.append(row)
        result = deepcopy(value)
        for row in removed:
            result["symbols"].remove(row)
        return result
    finally:
        fresh()
