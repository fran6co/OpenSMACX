"""Private task-body shape checks, not native evidence or authorization.

The caller owns the exact source bindings, selected physical COFF identity and
fresh complete linked graph. This helper never proves linking from a report.
Only the root's complete raw code extent is returned; metadata is not omitted.
"""

from dataclasses import asdict
from hashlib import sha256
import struct

from .model import Result
from .pe_coff import weak_external


def _extent(proof):
    address, size = proof.get("address"), proof.get("size")
    if (type(address) is not int or type(size) is not int or address <= 0
            or size <= 0 or address + size > 0x100000000):
        raise ValueError("task body requires a positive bounded linked extent")
    return address, size


def validate_body_node(obj, subject, node, graph):
    """Validate one selected task root against the final, complete native graph.

No associated runtime section, folded definition or observed alias is a whole
function permission. Verified direct FPO/CodeView leaves remain metadata, not
returned omission ranges. Call again after graph expansion, not before it only.
"""
    nodes, canonical = graph.get("nodes"), graph.get("canonical_nodes")
    index = node.get("id")
    if (graph.get("result") != Result.PASS or graph.get("gaps") != []
            or not isinstance(nodes, list) or not 0 < len(nodes) <= 100000
            or not isinstance(canonical, list) or len(canonical) != len(nodes)
            or type(index) is not int or not 0 <= index < len(nodes) or nodes[index] is not node
            or obj.symbols.get(subject.index) != subject or obj.function(subject.name) != subject
            or subject.section in obj.associations or obj.code_sections(subject) != (subject.section,)
            or node.get("symbol") != subject.name or node.get("section") != subject.section):
        raise ValueError("task body requires its exact independent function and complete graph")
    proof, body = node.get("proof"), canonical[index]
    if (not isinstance(proof, dict) or not isinstance(body, dict)
            or node.get("result") != Result.PASS or proof.get("result") != Result.PASS
            or proof.get("code", {}).get("result") != Result.PASS
            or proof.get("symbol") != subject.name or proof.get("owner") != node.get("owner")
            or node.get("folded_group") is not None or body.get("folded_group") is not None):
        raise ValueError("task body has incomplete, folded or mismatched native proof")
    address, size = _extent(proof)
    section = obj.sections[subject.section - 1]
    code_hash = sha256(section.data).hexdigest()
    if size != len(section.data) or size != obj.section_sizes[subject.section - 1]:
        raise ValueError("task body proof does not account for its complete raw code section")
    for other in obj.symbols.values():
        if (other.section == subject.section and other.index != subject.index
                and (other.storage == 2 or other.type & 0x20)):
            raise ValueError("task body shares code with another definition")
        if other.storage == 105 and weak_external(obj, other)[0].section == subject.section:
            raise ValueError("task body has a weak external address alias")

    children = sorted(child for child, parent in obj.associations.items() if parent == subject.section)
    if children:
        if (proof.get("extent_kind") != "associated_coff_regions"
                or body.get("extent_kind") != "associated_coff_regions"
                or not isinstance(proof.get("regions"), list) or len(proof["regions"]) != 1
                or not isinstance(body.get("regions"), list) or len(body["regions"]) != 1
                or not isinstance(proof.get("debug_regions"), list)
                or len(proof["debug_regions"]) != len(children)
                or body.get("debug_regions") != proof["debug_regions"]):
            raise ValueError("task body companions lack complete single-runtime-region proof")
        region, = proof["regions"]
        canonical_region, = body["regions"]
        if (region.get("section") != subject.section or region.get("ordinal") != 0
                or region.get("parent") is not None or _extent(region) != (address, size)
                or region.get("code", {}).get("result") != Result.PASS
                or any(row.get("flags") != section.flags or row.get("code_sha256") != code_hash
                       for row in (region, canonical_region))):
            raise ValueError("task body runtime proof differs from its exact physical section")
        for number, metadata in zip(children, proof["debug_regions"]):
            child = obj.sections[number - 1]
            if (child.name not in (".debug$S", ".debug$F") or child.flags != 0x42101048
                    or number in obj.associations.values() or metadata.get("parent") != 0):
                raise ValueError("task body has runtime, nested or unsupported companions")
            if child.name == ".debug$F":
                if (len(child.data) != 16 or metadata.get("format") != "fpo_data"
                        or metadata.get("data") != child.data[4:].hex()
                        or metadata.get("code_size") != struct.unpack_from("<I", child.data, 4)[0]):
                    raise ValueError("task body FPO proof differs from its physical metadata")
            else:
                from .linked_codeview import source_procedure
                source = source_procedure(obj, number, subject)
                fields = ("kind", "length", "debug_start", "debug_end", "flags", "name")
                locals_ = [{key: value for key, value in asdict(local).items() if key != "type_index"}
                           for local in source.locals]
                if (metadata.get("format") != "codeview5_procedure"
                        or any(metadata.get(field) != getattr(source, field) for field in fields)
                        or metadata.get("locals") != locals_
                        or metadata.get("types", {}).get("format") != "codeview5_type_graph"):
                    raise ValueError("task body CodeView proof differs from its physical metadata")
    elif (proof.get("regions") or proof.get("debug_regions")
          or body.get("extent_kind") != "function_comdat" or body.get("size") != size
          or body.get("flags") != section.flags or body.get("code_sha256") != code_hash):
        raise ValueError("task body proof does not describe its independent code COMDAT")

    # Check every runtime region, including associated data and aliases whose
    # canonical representative happens to be this root. No reachability prune.
    for other in nodes:
        other_proof = other.get("proof")
        if other.get("result") != Result.PASS or not isinstance(other_proof, dict):
            raise ValueError("task body graph contains an incomplete native node")
        if other is node:
            continue
        regions = other_proof.get("regions", [other_proof])
        if not isinstance(regions, list) or not regions:
            raise ValueError("task body graph has missing runtime extents")
        for region in regions:
            other_address, other_size = _extent(region)
            if address < other_address + other_size and other_address < address + size:
                raise ValueError("task body overlaps another observed runtime definition")
    return {"address": address, "size": size}
