"""Native same-layout output preservation within caller-owned source scope.

Only independently qualified function bodies and authenticated build identifiers
can differ. This load-image model does not promise debugger/file self-inspection
identity or runtime correctness. The enclosing task verifier owns authorization.
"""

from hashlib import sha256

from .linked_body_scope import qualify_body_aliases
from .linked_pe_metadata import qualify_metadata
from .model import Result, digest
from .pe_coff import PeImage
from .pe_output_residual import compare_same_layout_pe


def _runtime_regions(graph):
    """Extract protection-only extents, never body omission authority."""
    nodes = graph.get("nodes")
    if (graph.get("result") != Result.PASS or graph.get("gaps") != []
            or not isinstance(nodes, list) or len(nodes) > 100000):
        raise ValueError("metadata protection requires a complete bounded runtime graph")
    result, seen, visited = [], set(), 0
    for node in nodes:
        proof = node.get("proof")
        if node.get("result") != Result.PASS or not isinstance(proof, dict) or proof.get("result") != Result.PASS:
            raise ValueError("metadata protection has incomplete runtime proof")
        regions = proof.get("regions", [proof])
        if not isinstance(regions, list) or not regions:
            raise ValueError("metadata protection has missing runtime regions")
        for region in regions:
            visited += 1
            if visited > 1000000:
                raise ValueError("metadata runtime region scan budget exceeded")
            address, size = region.get("address"), region.get("size")
            if (type(address) is not int or type(size) is not int or address <= 0
                    or size <= 0 or address + size > 0x100000000):
                raise ValueError("metadata protection has invalid runtime extent")
            if (address, size) not in seen:
                if len(result) >= 100000:
                    raise ValueError("metadata runtime region budget exceeded")
                seen.add((address, size))
                result.append({"address": address, "size": size})
    return result


def compare_linked_output(baseline, current, graphs, bodies):
    """Compare the actual paired native artifacts; never consume saved proof."""
    native = {"baseline": baseline, "current": current}
    report = {"check": "native_same_layout_output_preservation", "result": Result.UNKNOWN,
              "outside_scope_verification": Result.UNKNOWN, "ready_for_collection": False,
              "persisted": False, "source_authority": "caller_owned",
              "model": "same-layout PE outside task bodies modulo authenticated build identification",
              "limitations": ["not debugger identity or self-inspection of executable files/mapped build metadata",
                              "not runtime acceptance",
                              "changed layout and unsupported metadata refuse; differing folded bodies remain unsupported"]}
    before = digest({"graphs": graphs, "bodies": bodies})

    def fresh():
        try:
            baseline.check_current()
        finally:
            current.check_current()
        if digest({"graphs": graphs, "bodies": bodies}) != before:
            raise ValueError("native output source scope or graph changed during comparison")

    try:
        fresh()
        if set(graphs) != set(native) or set(bodies) != set(native):
            raise ValueError("native output needs both original source-scoped graphs")
        qualified, metadata, images = {}, {}, {}
        for version, linked in native.items():
            regions = _runtime_regions(graphs[version])
            value = qualify_metadata(linked, [], project_locator=True, runtime_regions=regions)
            value["protected_runtime_regions"] = len(regions)
            raw = value.pop("projected_bytes")
            # Protected ranges have already been checked against every body.
            # Keep their count/digest without expanding every import table cell
            # in an agent's acceptance request.
            ranges = value.pop("protected_ranges")
            value.update(protected_ranges_count=len(ranges), protected_ranges_sha256=digest(ranges))
            runtime = value.pop("runtime_protection")
            value.update(runtime_protection_count=len(runtime), runtime_protection_sha256=digest(runtime))
            metadata[version] = value
            images[version] = PeImage(raw, sha256(raw).hexdigest())
        report.update(metadata=metadata)
        if (metadata["baseline"]["fields"] != metadata["current"]["fields"]
                or metadata["baseline"]["locator_identity"] != metadata["current"]["locator_identity"]):
            raise ValueError("native build-identification fields or logical sidecar outputs differ")
        fresh()
        whole = compare_same_layout_pe(images["baseline"], images["current"], [], [])
        report["whole_output"] = whole
        if whole["same_layout"] == Result.PASS and whole["outside_bytes"] == Result.PASS:
            report.update(result=Result.PASS, outside_scope_verification=Result.PASS,
                          outside_bytes=whole, proof_basis="whole_projected_image_equality")
            return report
        # Only differing images need source-owned omission ranges. Protection
        # above covered every final graph runtime extent, independent of scope.
        for version, linked in native.items():
            qualified[version] = qualify_body_aliases(linked, graphs[version], bodies[version])
            # Body omissions owe protection against every indirect metadata
            # range, not just the build-ID fields protected in the zero-waiver
            # attempt. Requalify under the same guarded native epoch.
            qualify_metadata(linked, qualified[version]["bodies"], project_locator=True,
                             runtime_regions=_runtime_regions(graphs[version]))
        report.update(body_qualification=qualified, proof_basis="source_scoped_output_residual")
        comparison = compare_same_layout_pe(images["baseline"], images["current"],
                                            qualified["baseline"]["bodies"], qualified["current"]["bodies"])
        report["outside_bytes"] = comparison
        if comparison["same_layout"] == Result.PASS:
            report.update(result=comparison["outside_bytes"], outside_scope_verification=comparison["outside_bytes"])
        else:
            report["reason"] = comparison.get("reason", "native output layout remains unqualified")
        return report
    except (OSError, ValueError, KeyError, TypeError, AttributeError) as error:
        return {**report, "result": Result.UNKNOWN, "outside_scope_verification": Result.UNKNOWN, "reason": str(error)}
    finally:
        fresh()
