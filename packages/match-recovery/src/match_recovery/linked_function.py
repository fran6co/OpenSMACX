"""One owner-qualified COFF function versus its actual linked placement.

The enclosing native caller owns physical input selection and fresh artifacts;
parsed objects or a saved report alone cannot establish those prerequisites.
"""

import hashlib

from .binary import AbsoluteConstant, compare_bound_code
from .model import Result, Span, digest


def verify_function(obj, symbol, owner, mapping, image, *, objects=None):
    subject = obj.function(symbol.name)
    if subject.index != symbol.index or len(obj.code_sections(subject)) != 1:
        raise ValueError("linked function requires one independently bounded code COMDAT")
    return _verify_region(obj, subject, owner, mapping, image, objects=objects)


def _constant_binding(obj, target, mapping, bindings):
    """Resolve only a physically selected external constant, never an operand."""
    if bindings is None or target.storage != 2:
        return None
    placed = [row for row in mapping.publics if row.symbol == target.name]
    binding = bindings[target.name]
    if binding is None:
        if target.section == -1:
            raise ValueError("absolute referring declaration has no physical constant definition")
        return None
    if placed:
        raise ValueError("absolute constant conflicts with a linked map identity")
    if (target.type != 0 or target.section not in (0, -1) or obj.auxiliary.get(target.index)
            or target.section == 0 and target.value):
        raise ValueError("unsupported referring declaration of an absolute constant")
    if binding["symbol"] != target.name or target.section == -1 and binding["value"] != target.value:
        raise ValueError("constant binding differs from its referring object declaration")
    return binding


def _verify_region(obj, subject, owner, mapping, image, *, objects=None):
    """Verify a caller-bounded zero-origin code region; does not infer ownership."""
    placements = mapping.statics if subject.storage == 3 else mapping.publics
    matches = [row for row in placements or () if row.symbol == subject.name
               and row.owner.casefold() == owner.casefold() and row.function and row.segment]
    if len(matches) != 1:
        raise ValueError("linked function has no unique selected owner-qualified placement")
    placed, = matches
    if not image.section_flags[placed.segment - 1] & 0x20:
        raise ValueError("linked function placement is outside code")
    code, relocations, alignment = obj.subject(subject)
    names = sorted({obj.symbols[int(row.target)].name for row in relocations
                    if obj.symbols[int(row.target)].storage == 2})
    # One before/final physical-population guard per code region, not per
    # relocation operand. Every named candidate still receives the same checks.
    bindings = objects.resolve_constants(names, optional=True) if objects is not None and names else None
    private_names = sorted({target.name for row in relocations
        if (target := obj.symbols[int(row.target)]).storage == 3 and target.section != subject.section
        and not any(item.symbol == target.name and item.segment and item.owner.casefold() == owner.casefold()
                    for item in mapping.statics or ())})
    if objects is not None and any(row.symbol in private_names
            for row in (*mapping.publics, *(mapping.statics or ()))):
        raise ValueError("private fallback conflicts with existing linked map evidence")
    private = (objects.resolve_local_symbols(owner, private_names, image)
               if objects is not None and private_names else {})
    targets, references = {}, []
    for relocation in relocations:
        target = obj.symbols[int(relocation.target)]
        constant = _constant_binding(obj, target, mapping, bindings)
        if constant is not None:
            targets[relocation.target] = AbsoluteConstant(constant["value"])
            references.append({"offset": relocation.offset, "kind": relocation.kind,
                               "identity": {"absolute_constant": {"symbol": target.name, "value": constant["value"]}},
                               "constant_binding": constant})
            continue
        if target.section == subject.section and 0 <= target.value < len(code):
            address, identity = placed.address + target.value, {"self_offset": target.value}
        else:
            if target.storage not in (2, 3) or target.section < 0:
                raise ValueError("unsupported linked function relocation identity")
            pool = mapping.publics if target.storage == 2 else mapping.statics or ()
            matches = [row for row in pool if row.symbol == target.name and row.segment
                       and (target.storage == 2 or row.owner.casefold() == owner.casefold())]
            if not matches and target.storage == 3 and target.name in private:
                binding = private[target.name]
                if (binding["object_index"] != target.index or binding["object_section"] != target.section
                        or binding["object_offset"] != target.value or target.type != 0
                        or obj.auxiliary.get(target.index)):
                    raise ValueError("private binding differs from the referring selected object")
                address = binding["address"]
            elif len(matches) == 1:
                address = matches[0].address
            else:
                raise ValueError("linked reference has no unique independent map placement")
            identity = {"external": target.name} if target.storage == 2 else {"local_owner": owner, "symbol": target.name}
        targets[relocation.target] = address
        references.append({"offset": relocation.offset, "kind": relocation.kind,
                           "identity": identity, "address": address})
        if target.name in private and target.storage == 3 and target.section != subject.section:
            references[-1]["private_binding"] = private[target.name]
    span = Span(placed.address, placed.address + len(code))
    proof = compare_bound_code(span, image.read(span), code, relocations, targets,
                               original_absolute_sites=image.absolute_sites(span), alignment=alignment)
    fingerprint = digest({"code_sha256": hashlib.sha256(code).hexdigest(), "size": len(code),
                          "flags": obj.sections[subject.section - 1].flags,
                          "references": [{key: row[key] for key in ("offset", "kind", "identity")} for row in references]})
    return {"symbol": subject.name, "owner": owner, "address": placed.address, "size": len(code),
            "body_fingerprint": fingerprint, "code": proof, "references": references,
            "target_contents": Result.UNKNOWN, "result": proof["result"]}
