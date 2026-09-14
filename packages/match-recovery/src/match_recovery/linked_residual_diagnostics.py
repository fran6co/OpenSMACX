"""Bounded explanations of residual JSON differences, never proof inputs."""

import json

from .model import canonical, digest


MAX_DIFFERENCES = 32
MAX_DIAGNOSTIC_BYTES = 16 * 1024
MAX_DETAIL_CHARS = 512


def _key(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False)


def _detail(value):
    encoded = _key(value)
    if len(encoded) <= MAX_DETAIL_CHARS:
        # Detached observations must not expose a mutable canonical input.
        return {"value": json.loads(encoded)}
    return {"json_prefix": encoded[:MAX_DETAIL_CHARS], "sha256": digest(value),
            "truncated": True}


def residual_differences(baseline, current):
    """Count differing leaves, missing subtrees and type/order changes exactly.

    Missing subtrees and type changes each count once, without enumerating
    descendants. Matched nodes use their supplied identities; every other
    array remains positional. Limits affect displayed entries only.
    """
    rows = []
    count = 0
    remaining = MAX_DIAGNOSTIC_BYTES - 1024  # Reserve the summary envelope.

    def difference(kind, path, identity=None):
        nonlocal count, remaining
        count += 1
        if len(rows) == MAX_DIFFERENCES:
            return
        row = {"kind": kind, "path": _detail(path)}
        if identity is not None:
            row["identity"] = _detail(identity[0])
        cost = len(canonical(row)) + 1
        if cost <= remaining:
            rows.append(row)
            remaining -= cost

    def walk(old, new, path, identity=None):
        if type(old) is not type(new):
            # Accepted JSON-compatible scalar subclasses may serialize to
            # the same value. Describe only differences the comparator sees.
            if _key(old) != _key(new):
                difference("type_changed", path, identity)
        elif isinstance(old, dict):
            for key in sorted(old.keys() | new.keys()):
                child = path + [key]
                if key not in old:
                    difference("added", child, identity)
                elif key not in new:
                    difference("removed", child, identity)
                else:
                    walk(old[key], new[key], child, identity)
        elif isinstance(old, list):
            for index in range(max(len(old), len(new))):
                child = path + [index]
                if index >= len(old):
                    difference("added", child, identity)
                elif index >= len(new):
                    difference("removed", child, identity)
                else:
                    walk(old[index], new[index], child, identity)
        elif _key(old) != _key(new):
            difference("value_changed", path, identity)

    def indexed(value):
        nodes = value.get("nodes")
        if not isinstance(nodes, list):
            return None
        result = {}
        for row in nodes:
            if not isinstance(row, dict) or "identity" not in row:
                return None
            key = _key(row["identity"])
            if key in result:
                return None
            result[key] = row
        return result

    old_nodes, new_nodes = indexed(baseline), indexed(current)
    if old_nodes is None or new_nodes is None:
        # Preserve explanations for every canonical dict the comparator
        # already accepts, without adding validation or hiding malformed rows.
        walk(baseline, current, [])
    else:
        walk({key: value for key, value in baseline.items() if key != "nodes"},
             {key: value for key, value in current.items() if key != "nodes"}, [])
        old_order = [key for key in old_nodes if key in new_nodes]
        new_order = [key for key in new_nodes if key in old_nodes]
        if old_order != new_order:
            difference("order_changed", ["nodes"])
        for key in sorted(old_nodes.keys() | new_nodes.keys()):
            row = old_nodes.get(key, new_nodes.get(key))
            identity = (row["identity"],)
            if key not in old_nodes:
                difference("node_added", ["nodes"], identity)
            elif key not in new_nodes:
                difference("node_removed", ["nodes"], identity)
            else:
                # For identified nodes, the path is relative to the node row.
                walk(old_nodes[key], new_nodes[key], [], identity)
    return {"advisory_only": True, "difference_count": count,
            "omitted_difference_count": count - len(rows),
            "truncated": count > len(rows), "differences": rows,
            "entry_limit": MAX_DIFFERENCES, "byte_limit": MAX_DIAGNOSTIC_BYTES}
