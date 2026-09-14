"""Pure residual graph algebra, not source authority or acceptance evidence.

The caller must supply fresh complete native observations and independently
verified identities/omissions, including dependencies orphaned by removed roots.
This module cannot establish any of those facts. Root order belongs to the
separate initializer-order comparison; no reachability pruning occurs here.
"""

import json

from .model import Result, digest
from .graph_limits import DEFAULT_NODES, node_limit


def _copy(value):
    def check(item, depth=0):
        if depth > 64:
            raise ValueError("residual JSON nesting exceeds its bound")
        if isinstance(item, dict):
            if any(not isinstance(key, str) for key in item):
                raise ValueError("residual JSON object keys must be strings")
            for child in item.values():
                check(child, depth + 1)
        elif isinstance(item, list):
            for child in item:
                check(child, depth + 1)
        elif item is not None and not isinstance(item, (str, int, float, bool)):
            raise ValueError("residual values must be JSON")
    check(value)
    try:
        return json.loads(json.dumps(value, allow_nan=False))
    except (TypeError, ValueError, OverflowError) as error:
        raise ValueError("residual values must be finite JSON") from error


def _key(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False)


def normalize_residual(observed_graph, identities: dict[int, object], omitted: set[int], *, max_nodes=DEFAULT_NODES):
    """Omit approved bodies only, retaining every other node and relationship.

IDs are supplied for *all* nodes, including omitted bodies: incoming references
and alias/fold membership must still identify those definitions. Saved reports
or an algebraic PASS confer no permission and establish no native freshness.
"""
    max_nodes = node_limit(max_nodes)
    if (not isinstance(observed_graph, dict) or observed_graph.get("result") != Result.PASS
            or observed_graph.get("gaps") != []):
        raise ValueError("residual comparison requires a complete passing code graph")
    nodes, bodies = observed_graph.get("nodes"), observed_graph.get("canonical_nodes")
    if (not isinstance(nodes, list) or not isinstance(bodies, list)
            or len(nodes) != len(bodies) or len(nodes) > max_nodes):
        raise ValueError("residual comparison requires bounded canonical nodes")
    indexes = set(range(len(nodes)))
    if (not isinstance(identities, dict) or any(type(key) is not int for key in identities)
            or set(identities) != indexes or not isinstance(omitted, set)
            or any(type(key) is not int for key in omitted) or not omitted <= indexes):
        raise ValueError("residual identities and omissions must identify exact observed nodes")
    stable = {index: _copy(identity) for index, identity in identities.items()}
    if len({_key(value) for value in stable.values()}) != len(stable):
        raise ValueError("residual node identities must be unique")
    bodies = _copy(bodies)
    for index, (node, body) in enumerate(zip(nodes, bodies)):
        if (not isinstance(node, dict) or type(node.get("id")) is not int
                or node["id"] != index or node.get("result") != Result.PASS
                or not isinstance(body, dict)):
            raise ValueError("residual canonical node is incomplete")

    def node_id(value):
        if type(value) is not int or value not in indexes:
            raise ValueError("residual reference has an unknown node")
        return value

    roots = observed_graph.get("roots")
    if not isinstance(roots, list):
        raise ValueError("residual graph lacks its observed roots")
    for index in roots:
        node_id(index)

    # Alias representatives depend on traversal order. Retain the equivalence
    # classes, including omitted members when they share with preserved code.
    parents = list(range(len(nodes)))

    def root(index):
        while parents[index] != index:
            index = parents[index]
        return index

    folds, slot_uses, slot_imports = {}, {}, {}
    for index, body in enumerate(bodies):
        if "address_alias" in body:
            alias = node_id(body["address_alias"])
            parents[root(index)] = root(alias)
        group = body.get("folded_group")
        if group is not None:
            if type(group) is not int or group < 0:
                raise ValueError("residual folding group is invalid")
            folds.setdefault(group, []).append(index)
    aliases = {}
    for index in indexes:
        aliases.setdefault(root(index), []).append(index)

    def members(values):
        return sorted((stable[index] for index in values), key=_key)

    def walk(value, index, path, replace=False):
        if isinstance(value, list):
            return [walk(child, index, path + [position], replace) for position, child in enumerate(value)]
        if not isinstance(value, dict):
            return value
        result = {}
        for key, child in value.items():
            if key == "target" and isinstance(child, dict):
                target = dict(child)
                if "node" in target:
                    target_id = node_id(target["node"])
                    if replace:
                        target["node"] = stable[target_id]
                if "slot" in target or "loader_import" in target:
                    slot = target.get("slot")
                    if (type(slot) is not int or slot < 0
                            or not isinstance(target.get("loader_import"), dict)
                            or "node" in target):
                        raise ValueError("residual import slot is invalid")
                    imported = _key(target["loader_import"])
                    if slot in slot_imports and slot_imports[slot] != imported:
                        raise ValueError("residual import slot has conflicting identities")
                    slot_imports[slot] = imported
                    if replace:
                        target["slot"] = sorted(slot_uses[slot], key=_key)
                    elif index not in omitted:
                        slot_uses.setdefault(slot, []).append({"node": stable[index], "path": path + [key]})
                result[key] = target
            else:
                result[key] = walk(child, index, path + [key], replace)
        return result

    # Validate references even in omitted bodies; their evidence must be whole.
    for index, body in enumerate(bodies):
        walk(body, index, [])
    result = []
    for index, body in enumerate(bodies):
        if index in omitted:
            continue
        normalized = walk(body, index, [], True)
        if "address_alias" in body:
            normalized["address_alias"] = members(aliases[root(index)])
        if body.get("folded_group") is not None:
            normalized["folded_group"] = members(folds[body["folded_group"]])
        result.append({"identity": stable[index], "body": normalized})
    canonical = {"nodes": sorted(result, key=lambda row: _key(row["identity"]))}
    return {"check": "linked_code_residual_algebra", "result": Result.PASS,
            "canonical": canonical, "fingerprint": digest(canonical),
            "remaining_linked_effects": Result.UNKNOWN, "ready_for_collection": False}


def compare_residual(baseline, current):
    """Compare normalized algebra values; never accept them as a capability."""
    for value in (baseline, current):
        if (not isinstance(value, dict) or value.get("result") != Result.PASS
                or not isinstance(value.get("canonical"), dict)):
            raise ValueError("residual comparison requires two normalized passing values")
        canonical = _copy(value["canonical"])
        if value.get("fingerprint") != digest(canonical):
            raise ValueError("residual canonical value changed after normalization")
    # Python container equality aliases True/1 and 1/1.0. Canonical JSON does
    # not: retaining all metadata includes its actual JSON value types.
    equal = _key(baseline["canonical"]) == _key(current["canonical"])
    return {"check": "linked_code_residual_comparison", "result": Result.PASS if equal else Result.FAIL,
            "equal": equal, "remaining_linked_effects": Result.UNKNOWN, "ready_for_collection": False}
