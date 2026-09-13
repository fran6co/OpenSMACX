"""Selected-object targets from independently image-bound linker fixups.

Fixup records name symbol-base RVAs, not relocated operand values. Caller-owned
live build guards remain mandatory; this module grants no source ownership.
"""

from hashlib import sha256

from .pe_coff import Coff


def check_image(fixups, image):
    from .pdb_fixups import BoundFixups
    if not isinstance(fixups, BoundFixups) or fixups.image_sha256 != sha256(image.data).hexdigest():
        raise ValueError("linker fixups are not bound to this exact image")


def check_selected(obj, owner, objects):
    if objects is None:
        raise ValueError("linker fixup targets require selected physical object evidence")
    selected = objects.resolve(owner)
    actual = Coff(selected["raw"])
    if any(getattr(actual, key) != getattr(obj, key) for key in
           ("symbols", "sections", "auxiliary", "associations", "section_sizes")):
        raise ValueError("fixup referring object differs from its selected physical input")
    return selected


def target_address(fixups, image, address, offset, kind):
    row = fixups.get(address + offset - image.base)
    if row is None or row.kind != kind or kind not in (6, 20):
        raise ValueError("missing or incompatible independently linked fixup")
    return image.base + row.target_rva


def private_data_binding(obj, target, selected, image, address):
    """Validate a private target after independent source-site placement."""
    if (target.storage != 3 or target.type != 0 or target.section <= 0
            or obj.auxiliary.get(target.index)):
        raise ValueError("fixup fallback requires a defined private data symbol")
    section = obj.sections[target.section - 1]
    if (section.flags & (0x20 | 0x20000000) or not section.flags & (0x40 | 0x80)
            or not 0 <= target.value < obj.section_sizes[target.section - 1]):
        raise ValueError("fixup fallback cannot hide unverified code or unbounded data")
    containing = [flags for (span, _, _), flags in zip(image.sections, image.section_flags)
                  if span.start <= address < span.end]
    if (len(containing) != 1 or containing[0] & (0x20 | 0x20000000)
            or not containing[0] & (0x40 | 0x80)):
        raise ValueError("private fixup target is not image data")
    return {"symbol": target.name, "address": address, "owner": selected["owner"],
            "input": selected["identity"], "object_sha256": selected["sha256"],
            "object_index": target.index, "object_section": target.section,
            "object_offset": target.value, "placement": "pdb_fixup"}


def local_data_bindings(obj, subject, owner, mapping, image, names, objects, fixups):
    """Missing named private data can use a fixup at its placed referring site."""
    check_image(fixups, image)
    selected = check_selected(obj, owner, objects)
    if any(row.symbol in names for row in (*mapping.publics, *(mapping.statics or ()))):
        raise ValueError("private fallback conflicts with existing linked map evidence")
    placements = mapping.publics if subject.storage == 2 else mapping.statics or ()
    roots = [row for row in placements if row.symbol == subject.name and row.function and row.segment
             and row.owner.casefold() == owner.casefold()]
    if len(roots) != 1:
        raise ValueError("fixup referring code has no unique owner-qualified placement")
    result = {}
    for offset, index, kind in obj.sections[subject.section - 1].relocations:
        if kind == 0:
            continue
        target = obj.symbols[index]
        if target.name not in names:
            continue
        address = target_address(fixups, image, roots[0].address, offset, kind)
        binding = private_data_binding(obj, target, selected, image, address)
        if target.name in result and result[target.name] != binding:
            raise ValueError("private fixup targets disagree on symbol placement")
        result[target.name] = binding
    if set(result) != set(names):
        raise ValueError("private fixup evidence does not cover every requested symbol")
    return result
