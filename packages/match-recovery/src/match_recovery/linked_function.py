"""One owner-qualified COFF function versus its actual linked placement.

The enclosing native caller owns physical input selection and fresh artifacts;
parsed objects or a saved report alone cannot establish those prerequisites.
"""

import hashlib

from .binary import compare_bound_code
from .model import Result, Span, digest


def verify_function(obj, symbol, owner, mapping, image):
    subject = obj.function(symbol.name)
    if subject.index != symbol.index or len(obj.code_sections(subject)) != 1:
        raise ValueError("linked function requires one independently bounded code COMDAT")
    return _verify_region(obj, subject, owner, mapping, image)


def _verify_region(obj, subject, owner, mapping, image):
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
    targets, references = {}, []
    for relocation in relocations:
        target = obj.symbols[int(relocation.target)]
        if target.section == subject.section and 0 <= target.value < len(code):
            address, identity = placed.address + target.value, {"self_offset": target.value}
        else:
            if target.storage not in (2, 3) or target.section < 0:
                raise ValueError("unsupported linked function relocation identity")
            pool = mapping.publics if target.storage == 2 else mapping.statics or ()
            matches = [row for row in pool if row.symbol == target.name and row.segment
                       and (target.storage == 2 or row.owner.casefold() == owner.casefold())]
            if len(matches) != 1:
                raise ValueError("linked reference has no unique independent map placement")
            address = matches[0].address
            identity = {"external": target.name} if target.storage == 2 else {"local_owner": owner, "symbol": target.name}
        targets[relocation.target] = address
        references.append({"offset": relocation.offset, "kind": relocation.kind,
                           "identity": identity, "address": address})
    span = Span(placed.address, placed.address + len(code))
    proof = compare_bound_code(span, image.read(span), code, relocations, targets,
                               original_absolute_sites=image.absolute_sites(span), alignment=alignment)
    fingerprint = digest({"code_sha256": hashlib.sha256(code).hexdigest(), "size": len(code),
                          "flags": obj.sections[subject.section - 1].flags,
                          "references": [{key: row[key] for key in ("offset", "kind", "identity")} for row in references]})
    return {"symbol": subject.name, "owner": owner, "address": placed.address, "size": len(code),
            "body_fingerprint": fingerprint, "code": proof, "references": references,
            "target_contents": Result.UNKNOWN, "result": proof["result"]}
