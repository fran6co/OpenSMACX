"""Independently anchored whole ordinary data sections in a candidate image.

Section bounds are compiler declarations, never distances between labels. A
positive placement does not prove contents, absence of another retained section,
allocation isolation or any task's authority to change the data.
"""

from collections import defaultdict
from contextlib import contextmanager
from copy import deepcopy

from .common_allocation import _policy
from .model import Result, Span, require_text
from .pe_coff import Coff
from .pe_debug import linked_coff_symbols


def observe_data_section(inputs, owner, number):
    """Observe one selected ordinary section inside a caller-owned live epoch.

    Native callers supply guarded LinkedInputs and recheck them on normal and
    exceptional exit. All physical candidate inputs are independently checked
    here too; neither a supplied COFF object nor a cached placement is accepted.
    COMDAT/associated and special linker-generated storage need other proofs.
    """
    with _data_sections(inputs) as observe:
        return observe(owner, number)


@contextmanager
def _data_sections(inputs):
    """Share native indices during one guarded batch, never cache verdicts."""
    objects, mapping, image = inputs.objects, inputs.mapping, inputs.image
    records, fresh = objects._constant_population()
    try:
        fresh()
        mapping.validate(image)
        debug, declarations = defaultdict(list), defaultdict(list)
        for symbol in linked_coff_symbols(image):
            debug[symbol.name].append(symbol)
        for record in records:
            for symbol in record["symbols"]:
                declarations[symbol["name"]].append((record, symbol))
        index = {"policy": _policy(inputs.evidence, records), "debug": debug,
                 "declarations": declarations, "selected": {}, "caller_anchors": {}, "data_anchors": {}}
        yield lambda owner, number: _observe_data_section(inputs, owner, number, index)
    finally:
        fresh()


def _observe_data_section(inputs, owner, number, index, *, include_data=True, extra_anchors=()):
    objects, mapping, image = inputs.objects, inputs.mapping, inputs.image
    require_text(owner, "link map owner")
    if index["policy"]:
        raise ValueError("ordinary storage has unsupported linker policy or directives")
    key = owner.casefold()
    if key not in index["selected"]:
        selected = objects.resolve(owner)
        index["selected"][key] = (selected, Coff(selected["raw"]))
    selected, obj = index["selected"][key]
    if type(number) is not int or not 1 <= number <= len(obj.sections):
        raise ValueError("ordinary storage requires an exact existing section number")
    section, size = obj.sections[number - 1], obj.section_sizes[number - 1]
    if (section.flags & ~0xC0F000C0 or not section.flags & 0x40000000
            or section.flags & 0xC0 not in (0x40, 0x80)
            or number in obj.associations or number in obj.associations.values()
            or section.name.lower().startswith((".tls", ".debug", ".idata", ".edata", ".rsrc"))
            or not section.name or section.name.startswith("/")):
        raise ValueError("unsupported ordinary non-COMDAT data section form")
    alignment = section.alignment
    if not 0 < size <= 16 * 1024 * 1024:
        raise ValueError("ordinary storage extent is empty or exceeds its resource bound")
    zero_fill = section.flags & 0xC0 == 0x80
    if ((zero_fill and (section.data or section.relocations))
            or not zero_fill and len(section.data) != size):
        raise ValueError("ordinary data extent disagrees with raw or zero-fill representation")
    writes = []
    for offset, _, kind in section.relocations:
        if kind == 0:
            continue  # Explicit no-op, not a storage write.
        if kind not in (6, 20) or not 0 <= offset <= size - 4:
            raise ValueError("ordinary storage relocation has unsupported or unbounded write extent")
        if any(other < offset + 4 and offset < other + 4 for other in writes):
            raise ValueError("ordinary storage has overlapping relocation write extents")
        writes.append(offset)
    symbols = []
    for symbol in obj.symbols.values():
        if symbol.section != number:
            continue
        if symbol.name == section.name and symbol.storage == 3 and symbol.type == 0:
            continue  # Coff has validated the complete section definition.
        if (symbol.storage not in (2, 3) or symbol.type != 0 or obj.auxiliary.get(symbol.index)
                or not symbol.name or not 0 <= symbol.value < size):
            raise ValueError("ordinary data section has an unsupported or unbounded symbol")
        symbols.append(symbol)
    if len({symbol.name for symbol in symbols}) != len(symbols):
        raise ValueError("ordinary storage has ambiguous within-section symbol names")
    for symbol in symbols:
        peers = [other for other in obj.symbols.values() if other.name == symbol.name
                 and other.section > 0 and other.storage in (2, 3)]
        if len(peers) != 1:
            raise ValueError("ordinary anchor name is ambiguous across selected object sections")
    debug, declarations = index["debug"], index["declarations"]
    anchors, private_names = [], []
    for symbol in symbols:
        pool = mapping.publics if symbol.storage == 2 else mapping.statics or ()
        opposite = mapping.statics or () if symbol.storage == 2 else mapping.publics
        if any(row.symbol == symbol.name and row.owner.casefold() == owner.casefold() for row in opposite):
            raise ValueError("ordinary data anchor conflicts with the opposite map namespace")
        matches = [row for row in pool if row.symbol == symbol.name
                   and (symbol.storage == 2 or row.owner.casefold() == owner.casefold())]
        if matches:
            if (len(matches) != 1 or matches[0].function or not matches[0].segment
                    or matches[0].owner.casefold() != owner.casefold()):
                raise ValueError("ordinary data map anchor has a conflicting owner or kind")
            anchors.append({"symbol": symbol.name, "object_index": symbol.index,
                            "object_offset": symbol.value, "address": matches[0].address,
                            "placement": "owner_qualified_map"})
        if not debug.get(symbol.name):
            continue
        if symbol.storage == 3:
            private_names.append(symbol.name)
            continue
        if (symbol.name in objects._constant_imports
                or objects._constant_weak_fallbacks.get(symbol.name)):
            raise ValueError("ordinary external debug anchor conflicts with an import or weak alias")
        # Ordinary unresolved external references do not define another
        # object/owner. Common, weak, private and actual definitions still
        # participate; no competing storage identity is dropped.
        def unresolved(record, declared):
            return (record["format"] == "coff" and record["machine"] == 0x14C
                    and record["optional_header_size"] == 0
                    and declared["declaration"] == "undefined" and declared["storage"] == 2
                    and declared["section"] == declared["value"] == declared["type"] == 0
                    and declared["auxiliary_size"] == 0)
        candidates = [(record, declared) for record, declared in declarations[symbol.name]
                      if not unresolved(record, declared)]
        if len(candidates) != 1 or len(debug[symbol.name]) != 1:
            raise ValueError("ordinary external debug anchor lacks unique physical identity")
        record, declared = candidates[0]
        identity = {key: record[key] for key in ("path", "member", "occurrence", "generated") if key in record}
        placed, = debug[symbol.name]
        if (identity != selected["identity"] or record["sha256"] != selected["sha256"]
                or declared["index"] != symbol.index or placed.storage != 2 or placed.type != 0
                or placed.section <= 0):
            raise ValueError("ordinary external debug anchor differs from its selected definition")
        anchors.append({"symbol": symbol.name, "object_index": symbol.index,
                        "object_offset": symbol.value, "address": image.base + placed.value,
                        "placement": "unique_embedded_coff"})
    if private_names:
        bindings = objects.resolve_local_symbols(owner, tuple(private_names), image)
        for name in private_names:
            row = bindings[name]
            if row["object_section"] != number:
                raise ValueError("private anchor belongs to a different selected section")
            anchors.append({key: row[key] for key in
                            ("symbol", "object_index", "object_offset", "address", "placement")})
    caller_evidence = None
    if getattr(inputs, "fixups", None) is not None:
        from .linked_data_anchors import collect_anchors
        if key not in index["caller_anchors"]:
            index["caller_anchors"][key] = collect_anchors(inputs, selected, obj)
        caller_evidence = index["caller_anchors"][key]
        anchors.extend(deepcopy(caller_evidence["anchors"].get(number, ())))
    data_evidence = None
    if include_data and getattr(inputs, "fixups", None) is not None:
        from .linked_data_anchors import collect_data_anchors
        if key not in index["data_anchors"]:
            def independent(number, pending=()):
                return _observe_data_section(inputs, owner, number, index,
                                             include_data=False, extra_anchors=pending)
            index["data_anchors"][key] = collect_data_anchors(inputs, selected, obj, independent)
        data_evidence = index["data_anchors"][key]
        anchors.extend(deepcopy(data_evidence["anchors"].get(number, ())))
    anchors.extend(deepcopy(extra_anchors))
    if not anchors:
        raise ValueError("ordinary storage has no independent placement anchor; absence is unproved")
    bases = {row["address"] - row["object_offset"] for row in anchors}
    if len(bases) != 1:
        raise ValueError("independent ordinary data anchors disagree on section base")
    base, = bases
    if base < image.base or base + size > 0x100000000 or base % alignment:
        raise ValueError("ordinary data placement has invalid arithmetic or alignment")
    span = Span(base, base + size)
    regions = [(index, flags) for index, ((region, _, _), flags) in enumerate(
        zip(image.sections, image.section_flags), 1) if region.start <= base and span.end <= region.end]
    if (len(regions) != 1 or regions[0][1] & (0x20 | 0x20000000)
            or not regions[0][1] & (0x40 | 0x80)
            or regions[0][1] & 0xC0000000 != section.flags & 0xC0000000):
        raise ValueError("ordinary data envelope lacks one compatible PE data section")
    return {"check": "linked_ordinary_data_envelope", "result": Result.PASS,
            "owner": selected["owner"], "input": dict(selected["identity"]),
            "object_sha256": selected["sha256"], "object_section": number,
            "section_name": section.name, "address": base, "size": size,
            "image_section": regions[0][0], "alignment": alignment, "zero_fill": zero_fill,
            "caller_evidence": deepcopy({key: caller_evidence[key] for key in
                ("callers", "gaps", "caller_coverage", "target_contents", "allocation_isolation")})
                if caller_evidence is not None else None,
            "data_evidence": deepcopy({key: data_evidence[key] for key in
                ("sources", "gaps", "source_coverage", "target_contents", "allocation_isolation")})
                if data_evidence is not None else None,
            "anchors": anchors, "symbols": [{"index": symbol.index, "name": symbol.name,
                "offset": symbol.value, "storage": symbol.storage} for symbol in symbols],
            "placement": Result.PASS, "contents": Result.UNKNOWN, "allocation_isolation": Result.UNKNOWN,
            "retained_data_coverage": Result.UNKNOWN, "remaining_linked_effects": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False}


def common_intersections(inputs, symbols, sections=None, *, context=None):
    """Find positive overlaps against freshly placed requested data sections.

    By default derive the entire physical storage census. An explicit subset
    can refute isolation but cannot establish its absence. Requests contain
    native owner/section identities, never reported extents.
    Completeness of retained ordinary and linker-generated storage is a separate
    prerequisite; even an empty intersection list cannot produce a PASS.
    """
    from .common_allocation import _verify_common_inputs
    from .common_preservation import _symbols

    names = _symbols(symbols)
    if sections is not None and (not isinstance(sections, (tuple, list)) or not 0 < len(sections) <= 4096
            or any(not isinstance(row, dict) or set(row) != {"owner", "section"}
                   or not isinstance(row["owner"], str) or not row["owner"]
                   or type(row["section"]) is not int or row["section"] <= 0 for row in sections)
            or len({(row["owner"].casefold(), row["section"]) for row in sections}) != len(sections)):
        raise ValueError("common intersections require bounded unique owner/section requests")
    # These are observations from the same guarded physical input set, not
    # user-supplied PASS reports or cross-version numerical correspondences.
    commons = _verify_common_inputs(inputs.evidence, inputs.coverage, inputs.records,
                                    inputs.image, inputs.mapping, names)
    regions, gaps, census, pdb_contributions, ordinary = [], [], None, None, None
    if sections is None:
        from .linked_storage_census import observe_storage
        if context is not None and "msvc-pdb" in inputs.artifacts:
            from .linked_contributions import ordinary_contributions
            try:
                ordinary = ordinary_contributions(context, inputs)
                census = ordinary["census"]
            except (OSError, ValueError) as error:
                ordinary = {"result": Result.UNKNOWN, "reason": str(error)}
                gaps.append({"reason": "PDB ordinary contribution evidence refused: " + str(error)})
        if census is None:
            census = observe_storage(inputs)
        regions = census["section_envelopes"]
        gaps.extend(census["gaps"])
        if "msvc-pdb" in inputs.artifacts:
            from .linked_contributions import _common_contributions
            try:
                pdb_contributions = _common_contributions(inputs, commons)
            except (OSError, ValueError) as error:
                pdb_contributions = {"result": Result.UNKNOWN, "reason": str(error)}
                gaps.append({"reason": "PDB common contribution evidence refused: " + str(error)})
    else:
        with _data_sections(inputs) as observe:
            for row in sections:
                try:
                    regions.append(observe(row["owner"], row["section"]))
                except (OSError, ValueError) as error:
                    gaps.append({**row, "reason": str(error)})
    intersections = []
    for common in commons["allocations"]:
        if common["result"] != Result.PASS:
            continue
        begin, end = common["address"], common["address"] + common["size"]
        for region in regions:
            left, right = region["address"], region["address"] + region["size"]
            if begin < right and left < end:
                intersections.append({"symbol": common["symbol"], "owner": region["owner"],
                    "object_section": region["object_section"], "input": region["input"],
                    "address": max(begin, left), "size": min(end, right) - max(begin, left)})
    return {"check": "linked_common_data_intersections",
            "result": Result.FAIL if intersections else Result.UNKNOWN,
            "allocation_isolation": Result.FAIL if intersections else Result.UNKNOWN,
            "common_allocations": commons, "section_envelopes": regions,
            "intersections": intersections, "gaps": gaps,
            "coverage": "complete_physical_candidate_population" if census is not None else "requested_data_sections_only",
            "storage_census": census, "pdb_contributions": pdb_contributions,
            "ordinary_contributions": ordinary,
            "retained_data_coverage": Result.UNKNOWN,
            "remaining_linked_effects": Result.UNKNOWN, "persisted": False, "ready_for_collection": False,
            "limitations": ["a positively placed ordinary section can refute common allocation isolation",
                            "unqueried, unsupported, discarded or linker-generated storage is not proved absent",
                            "no residual projection, task permission or original binding is granted"]}
