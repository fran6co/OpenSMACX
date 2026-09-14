"""Native body alias qualification, not source permission or PE equivalence.

The private caller supplies task-bound nodes from this live build's complete
code graph. Returned ranges may participate in that same live operation only;
serialized observations cannot authorize a body. This first supported policy
requires explicit NOICF: missing fold events are not proof of no folding.
"""

from bisect import bisect_left
from copy import deepcopy
from hashlib import sha256

from .application_body_scope import validate_body_node
from .common_allocation import _policy, _tokens
from .linked_context import LinkedBuild
from .linked_interval_index import IntervalIndex
from .model import Result, digest, require_text
from .pdb_contributions import bound_contributions
from .pe_coff import Coff
from .pe_exports import exports


def qualify_body_aliases(native, graph, bodies):
    """Qualify exact owned ranges against all native labels/contributions.

No guessed original SC section index, CRC-only binding or missing-record
discard inference. One independently placed exclusive code section must match
one exact physical-module contribution, with no competing contribution or
linked label. The caller separately qualifies metadata and literal PE residual.
"""
    if type(native) is not LinkedBuild:
        raise ValueError("body qualification requires its native linked build")
    records, population_fresh = native.inputs.objects._constant_population()
    before_graph, before_bodies = digest(graph), digest(bodies)
    try:
        native.check_current()
        if (not isinstance(bodies, list) or len(bodies) > 100000
                or graph.get("result") != Result.PASS or graph.get("gaps") != []):
            raise ValueError("body qualification requires bounded scope and a complete native graph")
        inputs = native.inputs
        if _policy(inputs.evidence, records):
            raise ValueError("body qualification has unsupported linker selection policy")
        arguments = [word.lower() for word in inputs.evidence["arguments"]]
        directives = [word.lower() for row in records for entry in row["directives"]
                      for word in _tokens(entry["text"])]
        if (not any(word in ("/opt:noicf", "-opt:noicf") for word in arguments)
                or any(word.startswith(("/opt:icf", "-opt:icf", "/merge:", "-merge:"))
                       for word in arguments + directives)):
            raise ValueError("body qualification requires explicit NOICF and no merging; absence of fold events is not proof")
        pdb = inputs.artifacts.get("msvc-pdb")
        if pdb is None:
            raise ValueError("body qualification requires image-bound PDB section contributions")
        inventory = bound_contributions(inputs.image, pdb)
        modules = native.module_bindings()
        if (modules["image_sha256"] != inventory.image_sha256
                or modules["pdb_sha256"] != inventory.pdb_sha256):
            raise ValueError("body module and contribution observations have different native inputs")
        module_index = {}
        for row in modules["modules"]:
            if row["result"] == Result.PASS:
                module_index.setdefault((digest(row["input"]), row["object_sha256"]), []).append(row)
        labels = sorted((*inputs.mapping.publics, *(inputs.mapping.statics or ())), key=lambda row: row.address)
        addresses = [row.address for row in labels]
        exported = exports(inputs.image)
        spans = IntervalIndex(max_intervals=1000000)
        for index, row in enumerate(inventory.contributions):
            if row.size:
                start = inputs.image.sections[row.section - 1][0].start + row.offset
                spans.insert(index, start, start + row.size)
        firsts = IntervalIndex(max_intervals=65535)
        for index, row in enumerate(inventory.modules):
            if row.first is not None and row.first.size:
                first = row.first
                start = inputs.image.sections[first.section - 1][0].start + first.offset
                firsts.insert(index, start, start + first.size)
        rows, ids, entities, parsed = [], set(), set(), {}
        for body in bodies:
            if not isinstance(body, dict) or set(body) != {"node_id", "entity_id", "address", "size"}:
                raise ValueError("body qualification requires exact node, source identity and extent fields")
            index = body["node_id"]
            require_text(body["entity_id"], "body source identity")
            if (type(index) is not int or not 0 <= index < len(graph["nodes"])
                    or index in ids or body["entity_id"] in entities):
                raise ValueError("body qualification has duplicate or missing source nodes")
            ids.add(index)
            entities.add(body["entity_id"])
            node = graph["nodes"][index]
            selected = inputs.objects.resolve(node["owner"])
            key = digest(selected["identity"]), selected["sha256"]
            if (selected["identity"] != node["input"] or selected["sha256"] != node["object_sha256"]
                    or sha256(selected["raw"]).hexdigest() != selected["sha256"]):
                raise ValueError("body proof differs from its exact selected physical input")
            if key not in parsed:
                parsed[key] = Coff(selected["raw"])
            obj = parsed[key]
            subject = obj.function(node["symbol"])
            extent = validate_body_node(obj, subject, node, graph)
            if digest(extent) != digest({key: body[key] for key in ("address", "size")}):
                raise ValueError("body supplied extent differs from its native code proof")
            begin, end = extent["address"], extent["address"] + extent["size"]
            placed = labels[bisect_left(addresses, begin):bisect_left(addresses, end)]
            if (len(placed) != 1 or not placed[0].segment or not placed[0].function
                    or placed[0].symbol != subject.name or placed[0].address != begin
                    or placed[0].owner.casefold() != selected["owner"].casefold()
                    or placed[0] not in (inputs.mapping.publics if subject.storage == 2 else inputs.mapping.statics or ())):
                raise ValueError("body has an extra, missing or contradictory MAP public/static label")
            if any(row.address is not None and begin <= row.address < end for row in exported):
                raise ValueError("body has an exported entry or interior alias")
            bound = module_index.get(key, [])
            if len(bound) != 1:
                raise ValueError("body lacks one exactly bound physical PDB module")
            overlaps = spans.overlaps(begin, end)
            if len(overlaps) != 1:
                raise ValueError("body has missing, duplicate or competing PDB section contributions")
            contribution_index, start, stop = overlaps[0]
            contribution = inventory.contributions[contribution_index]
            if (start != begin or stop != end or contribution.module != bound[0]["index"]
                    or contribution.section != placed[0].segment
                    or contribution.flags != obj.sections[subject.section - 1].flags):
                raise ValueError("body contribution has a different module, exact extent or section flags")
            if any(inventory.modules[index].first != contribution
                   for index, start, stop in firsts.overlaps(begin, end)):
                raise ValueError("body has contradictory module-first contribution evidence")
            rows.append({"entity_id": body["entity_id"], **extent,
                         "node_id": index, "input": deepcopy(selected["identity"]),
                         "object_sha256": selected["sha256"], "object_section": subject.section,
                         "module": contribution.module, "contribution": contribution_index})
        return {"check": "native_body_alias_qualification", "result": Result.PASS,
                "bodies": [{key: row[key] for key in ("entity_id", "address", "size")} for row in rows],
                "qualified": rows, "image_sha256": inventory.image_sha256, "pdb_sha256": inventory.pdb_sha256,
                "policy": "explicit_noicf_unmerged", "map_labels_scanned": len(labels),
                "contributions_scanned": len(inventory.contributions),
                "source_authority": "caller_owned", "metadata_aliases": Result.UNKNOWN,
                "outside_scope_verification": Result.UNKNOWN, "ready_for_collection": False, "persisted": False}
    finally:
        try:
            population_fresh()
            if digest(graph) != before_graph or digest(bodies) != before_bodies:
                raise ValueError("body graph or source scope changed during native qualification")
        finally:
            native.check_current()
