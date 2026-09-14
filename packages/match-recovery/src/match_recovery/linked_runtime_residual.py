"""Private cross-build runtime view; never source or output acceptance.

Only verified CodeView procedure type graphs are separated from the strict
residual. Their source-to-own-PDB correspondence remains mandatory. The caller
owns fresh native graph production, source identities and authorized omissions.
"""
from copy import deepcopy

from .graph_limits import node_limit
from .linked_code_residual import normalize_residual, compare_residual
from .linked_context import LinkedBuild
from .model import Result, digest


def _types(value, locals_count):
    if (not isinstance(value, dict) or set(value) != {"format", "roots", "nodes"}
            or value["format"] != "codeview5_type_graph"
            or not isinstance(value["roots"], list) or len(value["roots"]) != 1 + locals_count
            or not isinstance(value["nodes"], list) or len(value["nodes"]) > 100000):
        raise ValueError("unsupported or malformed verified CodeView type graph")
    nodes = value["nodes"]

    def target(row):
        if not isinstance(row, dict) or len(row) != 1:
            raise ValueError("invalid CodeView type target")
        if "node" in row and type(row["node"]) is int and 0 <= row["node"] < len(nodes):
            return 0x1000 + row["node"]
        if "primitive" in row and type(row["primitive"]) is int and 0 <= row["primitive"] < 0x1000:
            return row["primitive"]
        raise ValueError("invalid CodeView type target")

    # Semantic decoding happened in the native LinkedProcedures verifier.
    # Check only its exact bounded serialized shape here, not a second TPI
    # decode per procedure. Native proof equality below is mandatory.
    for index, row in enumerate(nodes):
        if (not isinstance(row, dict) or set(row) != {"id", "payload", "references"}
                or type(row["id"]) is not int or row["id"] != index
                or not isinstance(row["payload"], str) or not isinstance(row["references"], list)):
            raise ValueError("invalid CodeView type node")
        for edge in row["references"]:
            if (not isinstance(edge, dict) or set(edge) != {"offset", "target"}
                    or type(edge["offset"]) is not int or edge["offset"] < 0):
                raise ValueError("invalid CodeView type reference")
            target(edge["target"])
    for row in value["roots"]:
        target(row)


def compare_runtime_code(live, graphs, producers, omitted, *, max_nodes):
    """Compare fresh residuals modulo exact recognized procedure type metadata.

No procedure metadata, locals, FPO, code/data/reference/alias/fold evidence is
otherwise changed. A passing result requires separate source/output composition.
"""
    limit = node_limit(max_nodes)
    versions = {"baseline", "current"}
    if any(not isinstance(value, dict) or set(value) != versions for value in (live, graphs, producers, omitted)):
        raise ValueError("runtime residual requires both exact live versions")
    if any(type(value) is not LinkedBuild for value in live.values()):
        raise ValueError("runtime residual requires native linked contexts")
    if any(not isinstance(value, set) or any(type(index) is not int for index in value) for value in omitted.values()):
        raise ValueError("runtime residual requires exact omission sets")
    if any(not isinstance(value, dict) or any(type(index) is not int for index in value) for value in producers.values()):
        raise ValueError("runtime residual requires exact producer indices")
    def snapshot():
        return digest({"graphs": graphs, "producers": {version: sorted(value.items()) for version, value in producers.items()},
                       "omitted": {version: sorted(value) for version, value in omitted.items()}})
    before = snapshot()
    def fresh():
        try:
            live["baseline"].check_current()
        finally:
            live["current"].check_current()
        if snapshot() != before:
            raise ValueError("runtime residual graph, identities or omissions changed")
    try:
        fresh()
        projected, counts, checked = {}, {}, set()
        for version in ("baseline", "current"):
            original = graphs[version]
            # Validate all reference/identity algebra before making a copy.
            normalize_residual(original, producers[version], omitted[version], max_nodes=limit)
            view = deepcopy(original)
            count = 0
            for index, (node, body) in enumerate(zip(original["nodes"], view["canonical_nodes"], strict=True)):
                debug = body.get("debug_regions")
                if debug is None:
                    continue
                proof = node.get("proof")
                if (body.get("extent_kind") != "associated_coff_regions" or not isinstance(debug, list)
                        or not isinstance(proof, dict) or proof.get("result") != Result.PASS
                        or proof.get("extent_kind") != "associated_coff_regions"
                        or digest(debug) != digest(proof.get("debug_regions"))):
                    raise ValueError("runtime metadata lacks exact final native debug proof")
                for region in debug:
                    if not isinstance(region, dict):
                        raise ValueError("malformed runtime debug region")
                    if region.get("format") == "fpo_data":
                        continue  # Retained byte-for-byte, never projected.
                    if region.get("format") != "codeview5_procedure" or not isinstance(region.get("locals"), list):
                        raise ValueError("unsupported runtime debug region")
                    types = region.get("types")
                    key = digest({"types": types, "locals_count": len(region["locals"])})
                    if key not in checked:
                        _types(types, len(region["locals"]))
                        checked.add(key)
                    del region["types"]
                    if index not in omitted[version]:
                        count += 1
            projected[version] = normalize_residual(view, producers[version], omitted[version], max_nodes=limit)
            counts[version] = count
        result = compare_residual(projected["baseline"], projected["current"])
        return {**result, "check": "native_runtime_code_residual",
                "model": "runtime residual modulo verified CodeView procedure type graphs",
                "projected_type_graphs": counts, "persisted": False,
                "source_authority": "caller_owned", "outside_scope_verification": Result.UNKNOWN,
                "limitations": ["not cross-version debugger type identity or source ABI correctness",
                                "requires separate qualified output preservation and source authorization"]}
    finally:
        fresh()
