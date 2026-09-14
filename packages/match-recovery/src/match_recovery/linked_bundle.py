"""Verify complete associated COFF regions at independent linker placements.

Association establishes emitted ownership, not C++ hierarchy or lifetime. Every
region and relocation remains observable; missing placements are not guessed
from neighboring sections, matching bytes, or pointer operands.
"""

from hashlib import sha256

from .binary import AbsoluteConstant, compare_bound_code
from .linked_fixups import check_image, check_selected, target_address, private_data_binding
from .linked_fpo import verify_fpo
from .linked_function import _constant_binding
from .model import Result, Span, digest
from .pe_coff import unaddressed_end_label


def _owned(obj, root):
    if root in obj.associations:
        raise ValueError("associated child cannot masquerade as an independent function root")
    owned = {root}
    while True:
        added = {child for child, parent in obj.associations.items() if parent in owned} - owned
        if not added:
            break
        owned.update(added)
    if not 1 < len(owned) <= 256:
        raise ValueError("associated region verification requires a bounded nonempty family")
    return (root, *sorted(owned - {root}))


def verify_bundle(obj, subject, owner, mapping, image, *, objects, fixups, folded_frames=None, codeview=None):
    check_image(fixups, image)
    selected = check_selected(obj, owner, objects)
    family = _owned(obj, subject.section)
    metadata = tuple(number for number in family if obj.sections[number - 1].name in (".debug$F", ".debug$S"))
    owned = tuple(number for number in family if number not in metadata)
    ordinals = {number: index for index, number in enumerate(owned)}
    for number in owned:
        section = obj.sections[number - 1]
        if (not section.data or len(section.data) != obj.section_sizes[number - 1]
                or len(section.data) > 16 * 1024 * 1024 or section.flags & (0x80 | 0x800)
                or not section.flags & (0x20 | 0x40)
                or bool(section.flags & 0x20) != bool(section.flags & 0x20000000)):
            raise ValueError("unsupported associated code/data section form")
        obj.section_subject(number)  # Reject malformed/unsupported relocation forms.
    pool = mapping.publics if subject.storage == 2 else mapping.statics or ()
    roots = [row for row in pool if row.symbol == subject.name and row.function and row.segment
             and row.owner.casefold() == owner.casefold()]
    if len(roots) != 1:
        raise ValueError("associated family has no unique owner-qualified root placement")
    places = {subject.section: roots[0].address}
    verify_frame = verify_fpo if folded_frames is None else folded_frames.verify
    debug_regions = []
    for number in metadata:
        if obj.sections[number - 1].name == ".debug$F":
            proof = verify_frame(obj, number, subject, image, roots[0].address, fixups)
        else:
            from .linked_codeview import LinkedProcedures
            if type(codeview) is not LinkedProcedures or folded_frames is not None:
                raise ValueError("associated CodeView requires native non-folded module/type correspondence")
            proof = codeview.verify(obj, number, subject, owner, image, roots[0].address, fixups)
        debug_regions.append({"parent": 0, **proof})
    # Only independently placed source sites may supply another region's base.
    # All paths must agree; unused/unreachable companions still need placement.
    while True:
        before = dict(places)
        for number, address in before.items():
            for offset, index, kind in obj.sections[number - 1].relocations:
                target = obj.symbols[index]
                if kind == 0 or target.section not in ordinals:
                    continue
                if not 0 <= target.value < obj.section_sizes[target.section - 1]:
                    raise ValueError("associated symbol lies outside its owned section")
                base = target_address(fixups, image, address, offset, kind) - target.value
                if target.section in places and places[target.section] != base:
                    raise ValueError("independent linker paths disagree on associated placement")
                places[target.section] = base
        if places == before:
            break
    if set(places) != set(owned):
        raise ValueError("associated regions lack independent linker placement evidence")
    for symbol in obj.symbols.values():
        if symbol.section not in ordinals or symbol.name == obj.sections[symbol.section - 1].name:
            continue
        pool = mapping.publics if symbol.storage == 2 else mapping.statics or ()
        existing = [row for row in pool if row.symbol == symbol.name
                    and (symbol.storage == 2 or row.owner.casefold() == owner.casefold())]
        if existing and (len(existing) != 1 or not existing[0].segment
                         or existing[0].address != places[symbol.section] + symbol.value):
            raise ValueError("owned symbol map evidence contradicts associated placement")
    spans = {number: Span(places[number], places[number] + len(obj.sections[number - 1].data)) for number in owned}
    for number, span in spans.items():
        if any(other != number and span.start < bound.end and bound.start < span.end for other, bound in spans.items()):
            raise ValueError("associated regions have overlapping linked extents")
        regions = [(region, flags) for (region, _, _), flags in zip(image.sections, image.section_flags)
                   if region.start <= span.start and span.end <= region.end]
        if (len(regions) != 1 or bool(regions[0][1] & 0x20) != bool(obj.sections[number - 1].flags & 0x20)
                or bool(regions[0][1] & 0x20000000) != bool(obj.sections[number - 1].flags & 0x20000000)):
            raise ValueError("associated placement has incompatible image section kind")
    names = sorted({obj.symbols[index].name for number in owned
                    for _, index, kind in obj.sections[number - 1].relocations
                    if kind and obj.symbols[index].storage == 2})
    constants = objects.resolve_constants(names, optional=True) if names else None
    regions, private = [], {}
    for number in owned:
        code, relocations, alignment = obj.section_subject(number)
        references, targets = [], {}
        for relocation in relocations:
            target = obj.symbols[int(relocation.target)]
            constant = _constant_binding(obj, target, mapping, constants)
            row = {"offset": relocation.offset, "kind": relocation.kind}
            if constant is not None:
                targets[relocation.target] = AbsoluteConstant(constant["value"])
                row.update(identity={"absolute_constant": {"symbol": target.name, "value": constant["value"]}},
                           constant_binding=constant)
            else:
                address = target_address(fixups, image, places[number], relocation.offset,
                                         6 if relocation.kind == "absolute32" else 20)
                if target.section in ordinals:
                    if address != places[target.section] + target.value:
                        raise ValueError("associated target disagrees with its independently placed section")
                    row["identity"] = {"owned_region": ordinals[target.section], "offset": target.value}
                else:
                    candidates = [item for item in (mapping.publics if target.storage == 2 else mapping.statics or ())
                                  if item.symbol == target.name
                                  and (target.storage == 2 or item.owner.casefold() == owner.casefold())]
                    if not candidates and target.storage == 3:
                        if any(item.symbol == target.name for item in (*mapping.publics, *(mapping.statics or ()))):
                            raise ValueError("private fallback conflicts with existing linked map evidence")
                        binding = private_data_binding(obj, target, selected, image, address)
                        if target.name in private and private[target.name] != binding:
                            raise ValueError("private fixup targets disagree on symbol placement")
                        private[target.name] = binding
                        row["private_binding"] = binding
                    elif (target.storage not in (2, 3) or target.section < 0
                            or len(candidates) != 1 or not candidates[0].segment
                            or candidates[0].address != address):
                        raise ValueError("associated outward target lacks a consistent independent map identity")
                    row["identity"] = ({"external": target.name} if target.storage == 2 else
                                       {"local_owner": owner, "symbol": target.name})
                targets[relocation.target] = address
                row["address"] = address
            references.append(row)
        span = spans[number]
        section = obj.sections[number - 1]
        raw = image.read(span) if section.flags & 0x20 else image.read_initial(span)
        proof = compare_bound_code(span, raw, code, relocations, targets,
                                   original_absolute_sites=image.absolute_sites(span), alignment=alignment)
        symbols = [{"offset": item.value, "type": item.type, "storage": item.storage,
                    **({"external": item.name} if item.storage == 2 else {})}
                   for item in obj.symbols.values() if item.section == number
                   and item.name != section.name]
        if any(not (0 <= item.value < len(code) or unaddressed_end_label(obj, item))
               for item in obj.symbols.values() if item.section == number and item.name != section.name):
            raise ValueError("associated family has unbounded symbol aliases")
        regions.append({"section": number, "ordinal": ordinals[number],
                        "parent": ordinals.get(obj.associations.get(number)),
                        "address": span.start, "size": len(code), "flags": section.flags,
                        "code_sha256": sha256(code).hexdigest(), "code": proof,
                        "references": references, "symbols": sorted(symbols, key=digest)})
    results = [row["code"]["result"] for row in regions]
    result = Result.FAIL if Result.FAIL in results else Result.UNKNOWN if Result.UNKNOWN in results else Result.PASS
    canonical = [{key: row[key] for key in ("ordinal", "parent", "flags", "code_sha256", "symbols")}
                 | {"references": [{key: ref[key] for key in ("offset", "kind", "identity")}
                                   for ref in row["references"]]} for row in regions]
    return {"symbol": subject.name, "owner": owner, "address": places[subject.section],
            "size": len(obj.sections[subject.section - 1].data), "regions": regions,
            "debug_regions": debug_regions,
            "body_fingerprint": digest({"regions": canonical, "debug_regions": debug_regions}), "result": result,
            "code": {"result": result}, "references": regions[0]["references"],
            "extent_kind": "associated_coff_regions", "target_contents": Result.UNKNOWN}
