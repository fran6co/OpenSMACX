"""Derive bounded witness hosts from literal includes, never infer ABI independence."""

import base64
from collections import defaultdict, deque

from .includes import include_graph


def witness_hosts(snapshot, source, units, expectations):
    texts = {name: base64.b64decode(item["content_base64"]).decode("utf-8")
             for name, item in snapshot["files"].items()}
    graph, _ = include_graph(texts, source.include_paths)
    dependencies = defaultdict(set)
    for edge in graph.edges:
        if edge.relationship == "includes":
            dependencies[edge.dependent].add(edge.prerequisite)
    declarations = defaultdict(set)
    for item in expectations:
        declarations[item.class_name].add(item.path)
    candidates = defaultdict(list)
    for unit in units:
        distance, pending = {unit: 0}, deque([unit])
        while pending:
            parent = pending.popleft()
            for child in sorted(dependencies[parent]):
                if child not in distance:
                    distance[child] = distance[parent] + 1
                    pending.append(child)
        for name, paths in declarations.items():
            visible = paths & distance.keys()
            if visible:
                candidates[name].append((min(distance[path] for path in visible), len(texts[unit]), unit))
    return {name: {"unit": min(candidates[name])[2] if candidates[name] else None,
                   "candidate_units": [unit for _, _, unit in sorted(candidates[name])],
                   "declaration_paths": sorted(paths),
                   "basis": "nearest_literal_include_then_smallest_source"}
            for name, paths in sorted(declarations.items())}
