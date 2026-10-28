"""Exact associated COFF FPO -> image-bound PDB metadata, not runtime proof.

FPO_DATA: https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-fpo_data
The complete frame description is retained. Matching a sidecar record does not
prove loaded allocation, discard, unwinder behavior or outside-effect safety.
"""

import hashlib
import struct

from .model import Span
from .pdb_fixups import BoundFixups
from .pe_coff import Coff, Symbol, weak_external


def _root_reference(obj, target, root):
    """A direct definition or ordinary external declaration of this root.

    COFF may retain both an earlier undefined declaration and its later
    definition. The native caller independently binds the root's selected
    owner/address; this does not resolve weak aliases or choose a definition.
    PE/COFF: Section Number Values and Storage Class (EXTERNAL).
    https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
    """
    if target == root:
        return
    if root.storage != 2 or target.name != root.name:
        raise ValueError("FPO relocation does not reference its exact external root")
    for symbol in obj.symbols.values():
        if symbol.name != root.name or symbol == root:
            continue
        if (symbol.storage != 2 or symbol.section != 0 or symbol.value
                or symbol.type not in (0, 0x20) or obj.auxiliary.get(symbol.index)):
            raise ValueError("FPO root declaration has a conflicting definition or alias")


def validate_fpo_object(obj, number, root, *, source_codeview=False):
    """Validate one exact object-local FPO child and its root code extent.

    This proves only physical COFF metadata and ownership. In particular, the
    discardable section characteristic does not prove that a linker selected
    or removed the child. The complete FPO bytes remain in the returned
    witness for callers to retain and compare.
    ``source_codeview`` permits siblings only after the complete native
    source-procedure validator proves their CodeView ownership. The default
    linked path retains its existing single-child restriction.
    """
    if (type(source_codeview) is not bool or type(obj) is not Coff or type(number) is not int
            or not 1 <= number <= len(obj.sections)
            or not isinstance(root, Symbol) or obj.symbols.get(root.index) != root
            or obj.function(root.name) != root or root.section in obj.associations):
        raise ValueError("FPO object evidence lacks an exact independent function root")
    section = obj.sections[number - 1]
    direct_children = [child for child, parent in obj.associations.items()
                       if parent == root.section]
    if (section.name != ".debug$F" or section.flags != 0x42101048
            or len(section.data) != 16 or obj.section_sizes[number - 1] != 16
            or len(section.relocations) != 1
            or obj.associations.get(number) != root.section
            or number in obj.associations.values()
            or (not source_codeview and direct_children != [number])):
        raise ValueError("unsupported associated FPO child")
    if source_codeview:
        from .linked_codeview import source_procedure
        for child in direct_children:
            if child != number:
                source_procedure(obj, child, root, local_addresses=True)
    offset, target, kind = section.relocations[0]
    if offset != 0 or kind != 7 or target not in obj.symbols:
        raise ValueError("unsupported associated FPO root relocation")
    _root_reference(obj, obj.symbols[target], root)
    symbols = [symbol for symbol in obj.symbols.values() if symbol.section == number]
    if len(symbols) != 1:
        raise ValueError("FPO child has aliases or ambiguous section definition")
    definition = symbols[0]
    auxiliary = obj.auxiliary.get(definition.index, b"")
    if (definition.name != section.name or definition.storage != 3 or definition.type
            or definition.value or len(auxiliary) != 18):
        raise ValueError("FPO child lacks an exact section definition")
    size, relocations, lines, _, parent, selection = struct.unpack_from("<IHHIHB", auxiliary)
    if (size != 16 or relocations != 1 or lines or parent != root.section
            or selection != 5 or any(auxiliary[15:])):
        raise ValueError("FPO child has contradictory section metadata")
    for item in obj.sections:
        if any(target == definition.index for _, target, _ in item.relocations):
            raise ValueError("FPO child has an incoming address reference")
    for symbol in obj.symbols.values():
        if symbol.storage == 105 and weak_external(obj, symbol)[0].index == definition.index:
            raise ValueError("FPO child has a weak address alias")
    addend, code_size, _, _, frame = struct.unpack("<IIIHH", section.data)
    if (addend or not 0 < code_size <= len(obj.sections[root.section - 1].data)
            or frame & 0x2000 or (frame & 0xFF) > code_size):
        raise ValueError("unsupported FPO frame extent, reserved bits or root addend")
    root_section = obj.sections[root.section - 1]
    if (len(root_section.data) != code_size + (-code_size % root_section.alignment)
            or any(byte not in (0x90, 0xCC) for byte in root_section.data[code_size:])):
        raise ValueError("FPO code extent leaves non-alignment root bytes")
    return {
        "format": "fpo_data",
        "data": section.data[4:].hex(),
        "code_size": code_size,
        "section": number,
        "section_name": section.name,
        "section_flags": section.flags,
        "section_bytes": section.data.hex(),
        "section_auxiliary": auxiliary.hex(),
        "association_parent": root.section,
        "relocations": [{"offset": offset, "target_symbol": target,
                         "target_name": obj.symbols[target].name, "kind": kind,
                         "addend": addend}],
    }


def validate_fpo_source(obj, number, root, image, address, fixups):
    """Validate an emitted leaf frame and linked extent, not PDB correspondence.

    The caller establishes native object selection and the root's linked entry;
    no byte search, section placement or owner inference happens here. The
    image-bound sidecar must be valid, but this helper does not select or
    consume any FPO record. Its canonical result cannot prove that a matching
    record exists, is unique, or accounts for all folded members.
    """
    if (type(number) is not int or not 1 <= number <= len(obj.sections)
            or not isinstance(root, Symbol) or obj.symbols.get(root.index) != root
            or obj.function(root.name) != root or root.section in obj.associations
            or type(address) is not int or not isinstance(fixups, BoundFixups)
            or not isinstance(image.data, bytes)
            or fixups.image_sha256 != hashlib.sha256(image.data).hexdigest()):
        raise ValueError("FPO evidence lacks exact selected root and image binding")
    object_frame = validate_fpo_object(obj, number, root)
    code_size = object_frame["code_size"]
    if address < image.base or address + code_size > 0x100000000:
        raise ValueError("invalid linked FPO root extent")
    span = Span(address, address + code_size)
    image.read(span)
    owners = [flags for (region, _, _), flags in zip(image.sections, image.section_flags)
              if region.start <= span.start and span.end <= region.end]
    if len(owners) != 1 or not owners[0] & 0x20 or not owners[0] & 0x20000000:
        raise ValueError("FPO root is not inside linked executable code")
    return {key: object_frame[key] for key in ("format", "data", "code_size")}


def verify_fpo(obj, number, root, image, address, fixups):
    """Verify one source-valid leaf frame against exactly one bound PDB record."""
    frame = validate_fpo_source(obj, number, root, image, address, fixups)
    root_rva = address - image.base
    matches = [record for record in fixups.fpo_records if struct.unpack_from("<I", record)[0] == root_rva]
    if len(matches) != 1 or matches[0] != struct.pack("<I", root_rva) + bytes.fromhex(frame["data"]):
        raise ValueError("associated FPO has missing, ambiguous or different linked metadata")
    return frame
