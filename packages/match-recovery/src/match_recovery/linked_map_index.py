"""Invocation-local duplicate-preserving indexes, never placement evidence.

Each original or ICF owner-rewritten map keeps its own index. Native callers
still own entry/final map/artifact guards; no index survives the observation.
"""

from collections import defaultdict


class MapIndex:
    def __init__(self, mapping):
        self.mapping = mapping
        public, static, names = defaultdict(list), defaultdict(list), set()
        for row in mapping.publics:
            public[row.symbol].append(row)
            names.add(row.symbol)
        for row in mapping.statics or ():
            static[row.owner.casefold(), row.symbol].append(row)
            names.add(row.symbol)
        self.publics = {key: tuple(rows) for key, rows in public.items()}
        self.statics = {key: tuple(rows) for key, rows in static.items()}
        self.names = frozenset(names)

    def require(self, mapping):
        if self.mapping is not mapping:
            raise ValueError("linked map index belongs to another physical or owner-rewritten view")
        return self


class MapIndexes:
    """Retain the exact view along with id-keyed entries to prevent id reuse."""

    def __init__(self):
        self._views = {}

    def for_map(self, mapping):
        key = id(mapping)
        if key not in self._views:
            self._views[key] = MapIndex(mapping)
        return self._views[key].require(mapping)


def index_for(mapping, supplied=None):
    if supplied is None:
        return MapIndex(mapping)
    if type(supplied) is not MapIndex:
        raise ValueError("linked map lookup requires its exact native observation index")
    return supplied.require(mapping)
