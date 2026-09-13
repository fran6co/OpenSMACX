"""Exact associated COFF FPO -> image-bound PDB metadata, not runtime proof.

FPO_DATA: https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-fpo_data
The complete frame description is retained. Matching a sidecar record does not
prove loaded allocation, discard, unwinder behavior or outside-effect safety.
"""

import hashlib
import struct

from .model import Span
from .pdb_fixups import BoundFixups
from .pe_coff import Symbol, weak_external


def verify_fpo(obj, number, root, image, address, fixups):
    """Verify one selected leaf metadata child against one bound PDB record.

    The caller establishes native object selection and the root's linked entry;
    no byte search, section placement or owner inference happens here.
    """
    if (type(number) is not int or not 1 <= number <= len(obj.sections)
            or not isinstance(root, Symbol) or obj.symbols.get(root.index) != root
            or obj.function(root.name) != root or root.section in obj.associations
            or type(address) is not int or not isinstance(fixups, BoundFixups)
            or not isinstance(image.data, bytes)
            or fixups.image_sha256 != hashlib.sha256(image.data).hexdigest()):
        raise ValueError("FPO evidence lacks exact selected root and image binding")
    section = obj.sections[number - 1]
    if (section.name != ".debug$F" or section.flags != 0x42101048
            or len(section.data) != 16 or obj.section_sizes[number - 1] != 16
            or section.relocations != ((0, root.index, 7),)
            or obj.associations.get(number) != root.section
            or number in obj.associations.values()
            or sum(parent == root.section and obj.sections[child - 1].name == ".debug$F"
                   for child, parent in obj.associations.items()) != 1):
        raise ValueError("unsupported associated FPO child")
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
    if address < image.base or address + code_size > 0x100000000:
        raise ValueError("invalid linked FPO root extent")
    span = Span(address, address + code_size)
    image.read(span)
    owners = [flags for (region, _, _), flags in zip(image.sections, image.section_flags)
              if region.start <= span.start and span.end <= region.end]
    if len(owners) != 1 or not owners[0] & 0x20 or not owners[0] & 0x20000000:
        raise ValueError("FPO root is not inside linked executable code")
    root_rva = address - image.base
    matches = [record for record in fixups.fpo_records if struct.unpack_from("<I", record)[0] == root_rva]
    if len(matches) != 1 or matches[0] != struct.pack("<I", root_rva) + section.data[4:]:
        raise ValueError("associated FPO has missing, ambiguous or different linked metadata")
    return {"format": "fpo_data", "data": section.data[4:].hex(), "code_size": code_size}
