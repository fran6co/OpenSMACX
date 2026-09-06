"""Dependencies shared by scheduling, context invalidation and migration scope."""

from __future__ import annotations

from collections import defaultdict, deque
from dataclasses import dataclass
from typing import Iterable

from .model import require_text


@dataclass(frozen=True, order=True)
class Edge:
    dependent: str
    prerequisite: str
    relationship: str
    evidence: str

    def __post_init__(self) -> None:
        for value in (self.dependent, self.prerequisite, self.relationship, self.evidence):
            require_text(value, "edge field")


class DependencyGraph:
    def __init__(self, edges: Iterable[Edge] = ()):
        self.edges = frozenset(edges)
        self._dependents: dict[str, set[str]] = defaultdict(set)
        self._prerequisites: dict[str, set[str]] = defaultdict(set)
        for edge in self.edges:
            self._dependents[edge.prerequisite].add(edge.dependent)
            self._prerequisites[edge.dependent].add(edge.prerequisite)

    @staticmethod
    def _closure(roots: Iterable[str], adjacency: dict[str, set[str]]) -> frozenset[str]:
        seen = set(roots)
        pending = deque(sorted(seen))
        while pending:
            for node in sorted(adjacency.get(pending.popleft(), ())):
                if node not in seen:
                    seen.add(node)
                    pending.append(node)
        return frozenset(seen)

    def impacted(self, changed: Iterable[str]) -> frozenset[str]:
        return self._closure(changed, self._dependents)

    def prerequisites(self, subjects: Iterable[str]) -> frozenset[str]:
        return self._closure(subjects, self._prerequisites)

    def with_discovery(self, edges: Iterable[Edge]) -> DependencyGraph:
        return DependencyGraph(self.edges | frozenset(edges))
