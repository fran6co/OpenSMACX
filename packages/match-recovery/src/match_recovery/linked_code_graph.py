"""Relocation-reachable linked function code, not memory or startup semantics.

Roots and every discovered code target are bound to selected physical inputs.
Following an address operand includes callbacks and tail-call helpers; it does
not imply that the operand executes, or discover indirect targets through data.
"""

from collections import Counter
import hashlib

from .linked_function import verify_function
from .model import Result, digest
from .pe_coff import Coff


MAX_FUNCTIONS = 4096


def observe_code_graph(objects, mapping, image, roots, *, max_functions=MAX_FUNCTIONS):
    """Observe a deterministic rooted graph inside the caller's live epoch.

Node numbering follows root/relocation order, not local compiler spellings.
Shared targets and exact linked-address aliases remain shared; identical bytes
never merge distinct definitions. External symbol identity remains explicit.
"""
    if type(max_functions) is not int or not 1 <= max_functions <= MAX_FUNCTIONS:
        raise ValueError("linked code graph requires a bounded positive function limit")
    mapping.validate(image)
    nodes, keys, parsed, canonical, gaps = [], {}, {}, [], []
    storage, code_targets = [], []

    def add(owner, symbol):
        if not isinstance(owner, str) or not owner or not isinstance(symbol, str) or not symbol:
            raise ValueError("linked code graph requires owner-qualified function roots")
        key = owner.casefold(), symbol
        if key not in keys:
            if len(nodes) >= max_functions:
                raise ValueError("linked code graph function resource bound exceeded")
            keys[key] = len(nodes)
            nodes.append({"id": len(nodes), "owner": owner, "symbol": symbol, "result": Result.UNKNOWN})
            canonical.append(None)
        return keys[key]

    root_ids = []
    for root in roots:
        root_ids.append(add(root["owner"], root["symbol"]))
    public = {}
    static = {}
    for row in mapping.publics:
        public.setdefault(row.symbol, []).append(row)
    for row in mapping.statics or ():
        static.setdefault((row.owner.casefold(), row.symbol), []).append(row)
    spans = []
    cursor = 0
    while cursor < len(nodes):
        node = nodes[cursor]
        try:
            item = objects.resolve(node["owner"])
            if (item["owner"].casefold() != node["owner"].casefold()
                    or hashlib.sha256(item["raw"]).hexdigest() != item["sha256"]):
                raise ValueError("linked code object differs from its selected physical identity")
            if item["sha256"] not in parsed:
                parsed[item["sha256"]] = Coff(item["raw"])
            obj = parsed[item["sha256"]]
            subject = obj.function(node["symbol"])
            proof = verify_function(obj, subject, item["owner"], mapping, image)
            node.update(input=item["identity"], object_sha256=item["sha256"],
                        section=subject.section, proof=proof, result=proof["result"])
            code, relocations, _ = obj.subject(subject)
            begin, end = proof["address"], proof["address"] + len(code)
            overlaps = [(index, left, right) for index, left, right in spans if left < end and begin < right]
            if any((left, right) != (begin, end) for _, left, right in overlaps):
                raise ValueError("linked code graph has overlapping, differently bounded functions")
            alias = overlaps[0][0] if overlaps else cursor
            spans.append((cursor, begin, end))
            edges = []
            for relocation in sorted(relocations, key=lambda row: row.offset):
                target = obj.symbols[int(relocation.target)]
                addend = int.from_bytes(code[relocation.offset:relocation.offset + 4], "little", signed=True)
                edge = {"offset": relocation.offset, "kind": relocation.kind}
                if target.section == subject.section and 0 <= target.value < len(code):
                    if not 0 <= target.value + addend < len(code):
                        raise ValueError("effective self code reference is outside its observed function span")
                    edge["target"] = {"self_offset": target.value}
                else:
                    if target.storage not in (2, 3) or target.section < 0:
                        raise ValueError("unsupported linked code target identity")
                    matches = [row for row in (public.get(target.name, ()) if target.storage == 2 else
                               static.get((item["owner"].casefold(), target.name), ())) if row.segment]
                    if len(matches) != 1:
                        raise ValueError("linked code target has no unique independent placement")
                    placed, = matches
                    if placed.function:
                        edge["target"] = {"node": add(placed.owner, placed.symbol)}
                        code_targets.append((cursor, edge["target"]["node"], addend))
                        if addend:
                            edge["target"]["addend"] = addend
                        if target.storage == 2:
                            edge["target"]["external"] = target.name
                    else:
                        if target.type & 0x20:
                            raise ValueError("declared code target has a non-function linked placement")
                        identity = ({"external": target.name} if target.storage == 2 else
                                    {"local_owner": item["owner"], "symbol": target.name})
                        edge["target"] = {"data": identity}
                        storage.append({"node": cursor, **edge, "address": placed.address,
                                        "contents": Result.UNKNOWN})
                edges.append(edge)
            node["edges"] = edges
            canonical[cursor] = {"code_sha256": hashlib.sha256(code).hexdigest(), "size": len(code),
                                 "flags": obj.sections[subject.section - 1].flags,
                                 "address_alias": alias, "edges": edges}
        except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
            # An observed code counterexample is not erased by a later gap in
            # the same function's target closure.
            node.update(result=Result.FAIL if node["result"] == Result.FAIL else Result.UNKNOWN, reason=str(error))
            gaps.append({"node": cursor, "owner": node["owner"], "symbol": node["symbol"], "reason": str(error)})
        cursor += 1
    # A relocation can name a real function while its addend actually lands
    # somewhere else. Observe the effective destination, not just the symbol.
    for source, target, addend in code_targets:
        proof = nodes[target].get("proof")
        if proof is not None and not 0 <= addend < proof["size"]:
            node = nodes[source]
            reason = "effective code reference is outside its observed target function span"
            node.update(result=Result.FAIL if node["result"] == Result.FAIL else Result.UNKNOWN, reason=reason)
            gaps.append({"node": source, "owner": node["owner"], "symbol": node["symbol"], "reason": reason})
    counts = Counter(row["result"] for row in nodes)
    result = Result.FAIL if counts[Result.FAIL] else Result.UNKNOWN if counts[Result.UNKNOWN] else Result.PASS
    return {"check": "linked_relocation_code_graph", "result": result, "roots": root_ids,
            "nodes": nodes, "counts": dict(counts), "gaps": gaps, "data_references": storage,
            "graph_fingerprint": digest({"roots": root_ids, "nodes": canonical}) if result == Result.PASS else None,
            "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
            "limitations": ["code reached by explicit COFF relocations only; data contents and indirect targets are unproved",
                            "code agreement is not guard, callback registration, storage or runtime effect equivalence",
                            "one independently bounded COMDAT per function; unsupported selections/definitions refuse",
                            "shared definitions, relocation order and linked address aliasing are retained"]}


def compare_code_graphs(baseline, current):
    """Compose in-epoch observations, never accept this report as a waiver."""
    results = [baseline["result"], current["result"]]
    complete = all(result == Result.PASS for result in results)
    equal = complete and baseline["graph_fingerprint"] == current["graph_fingerprint"]
    result = (Result.FAIL if Result.FAIL in results else Result.UNKNOWN if not complete else
              Result.PASS if equal else Result.FAIL)
    return {"check": "application_initializer_code_dependencies", "result": result,
            "baseline": baseline, "current": current,
            "graph_equal": equal if complete else None,
            "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False}
