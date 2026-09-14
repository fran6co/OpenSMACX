"""Whole ordinary data bytes and references at independently anchored places.

This is selected-input/image correspondence, not target contents, allocation
isolation, cross-build preservation or permission to project a compiler effect.
The native caller owns image/map/live-context guards; physical input population
guards bracket this complete operation on both normal and exceptional exit.
"""

from hashlib import sha256

from .binary import AbsoluteConstant, RelocationKind, compare_bound_code
from .linked_function import _constant_binding
from .linked_storage import _data_sections
from .model import Result, Span, digest
from .pe_coff import Coff


def verify_data_contents(inputs, owner, number):
    """Re-derive a full ordinary envelope and verify every byte and relocation.

    No input report or caller-provided address is accepted. Unplaced private
    targets and special/associated data need separate proofs, not byte searches
    or a synthetic function entry to borrow code-placement assumptions.
    """
    with _data_sections(inputs) as observe:
        return _verify_placed_data(inputs, owner, number, observe)


def _verify_placed_data(inputs, owner, number, observe, *, fixup_target=None):
    """Internal same-epoch verifier; pending anchors are not public evidence.

    The anchor collector supplies only independently seeded source envelopes.
    Its optional target resolver uses image-bound fixups, never operands.
    Nothing may consume those pending bindings until this whole proof passes.
    """
    envelope = observe(owner, number)
    selected = inputs.objects.resolve(owner)
    obj = Coff(selected["raw"])
    if (obj.machine != 0x14C or obj.optional_header_size != 0
            or selected["identity"] != envelope["input"]
            or selected["sha256"] != envelope["object_sha256"]
            or sha256(selected["raw"]).hexdigest() != selected["sha256"]):
        raise ValueError("ordinary contents differ from their independently selected native object")
    raw, relocations, _ = obj.section_subject(number)
    section = obj.sections[number - 1]
    candidate = bytes(envelope["size"]) if envelope["zero_fill"] else raw
    if len(candidate) != envelope["size"]:
        raise ValueError("ordinary contents do not cover their entire declared section")
    image, mapping = inputs.image, inputs.mapping
    names = sorted({obj.symbols[int(row.target)].name for row in relocations
                    if obj.symbols[int(row.target)].storage == 2})
    constants = inputs.objects.resolve_constants(names, optional=True) if names else None
    targets, references, target_envelopes = {}, [], {number: envelope}
    for relocation in relocations:
        target = obj.symbols[int(relocation.target)]
        addend = int.from_bytes(candidate[relocation.offset:relocation.offset + 4], "little", signed=True)
        reference = {"offset": relocation.offset, "kind": relocation.kind, "addend": addend,
                     "object_target": {key: getattr(target, key) for key in
                                       ("index", "name", "section", "value", "storage", "type")},
                     "target_contents": Result.UNKNOWN, "target_selection": Result.UNKNOWN}
        constant = _constant_binding(obj, target, mapping, constants)
        if constant is not None:
            targets[relocation.target] = AbsoluteConstant(constant["value"])
            reference.update(identity={"absolute_constant": {"symbol": target.name, "value": constant["value"]}},
                             constant_binding=constant, target_selection=Result.PASS)
            references.append(reference)
            continue
        if target.section == number:
            if not 0 <= target.value + addend < envelope["size"]:
                raise ValueError("ordinary self reference exceeds its independently bounded section")
            address = envelope["address"] + target.value
            identity = {"self_offset": target.value}
            if target.storage == 2:
                identity["external"] = target.name
            reference["target_selection"] = Result.PASS
        else:
            if target.storage not in (2, 3) or target.section < 0:
                raise ValueError("unsupported ordinary data relocation target declaration")
            if sum(symbol.name == target.name for symbol in obj.symbols.values()) != 1:
                raise ValueError("ordinary data target has ambiguous object symbol identities")
            pool = mapping.publics if target.storage == 2 else mapping.statics or ()
            matches = [row for row in pool if row.symbol == target.name
                       and (target.storage == 2 or row.owner.casefold() == owner.casefold())]
            if matches and (len(matches) != 1 or not matches[0].segment
                            or target.type & 0x20 and not matches[0].function):
                raise ValueError("ordinary data target has conflicting independent map placement")
            if target.storage == 3 and target.type == 0:
                if not matches and any(row.symbol == target.name for row in
                                       (*mapping.publics, *(mapping.statics or ()))):
                    raise ValueError("private ordinary target conflicts with another map identity")
                if fixup_target is not None and not matches:
                    bound, binding = fixup_target(envelope, relocation, target)
                    previous = target_envelopes.get(target.section)
                    if previous is not None and previous["address"] != bound["address"]:
                        raise ValueError("private data fixups disagree on section base")
                    target_envelopes[target.section] = bound
                    reference["private_binding"] = binding
                elif target.section not in target_envelopes:
                    target_envelopes[target.section] = observe(owner, target.section)
                bound = target_envelopes[target.section]
                if not 0 <= target.value + addend < bound["size"]:
                    raise ValueError("private ordinary reference exceeds its independently bounded section")
                address = bound["address"] + target.value
                if matches and (matches[0].function or matches[0].address != address):
                    raise ValueError("private ordinary target disagrees with its independent section envelope")
                reference.update(target_envelope=bound, target_selection=Result.PASS)
            elif len(matches) == 1:
                address = matches[0].address
                reference["map_target"] = {"symbol": matches[0].symbol, "owner": matches[0].owner,
                                           "address": address, "function": matches[0].function,
                                           "segment": matches[0].segment}
            else:
                raise ValueError("ordinary data target lacks independent placement")
            identity = ({"external": target.name} if target.storage == 2 else
                        {"local_owner": selected["owner"], "symbol": target.name,
                         "object_section": target.section, "object_offset": target.value})
        targets[relocation.target] = address
        reference.update(identity=identity, address=address)
        references.append(reference)
    span = Span(envelope["address"], envelope["address"] + envelope["size"])
    proof = compare_bound_code(span, image.read_initial(span), candidate, relocations, targets,
                               original_absolute_sites=image.absolute_sites(span), alignment=1)
    canonical = {"raw_sha256": sha256(candidate).hexdigest(), "size": len(candidate),
                 "flags": section.flags, "alignment": envelope["alignment"],
                 "references": [{key: row[key] for key in ("offset", "kind", "addend", "identity")}
                                for row in references]}
    return {"check": "linked_ordinary_data_contents", "result": proof["result"],
            "input_consistency": proof["result"], "envelope": envelope,
            "owner": selected["owner"], "input": dict(selected["identity"]),
            "object_sha256": selected["sha256"], "object_section": number,
            "raw_sha256": canonical["raw_sha256"], "content_fingerprint": digest(canonical),
            "contents": proof, "references": references, "target_contents": Result.UNKNOWN,
            "allocation_isolation": Result.UNKNOWN, "remaining_linked_effects": Result.UNKNOWN,
            "persisted": False, "ready_for_collection": False}
