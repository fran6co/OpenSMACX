"""Bound one observed linker debug allocation without omitting any bytes.

The supported shape is one NB10 directory entry, optionally followed by 28
retained zero bytes. The suffix is not a second entry, unused space or an exemption.
The enclosing native epoch owns freshness; saved results confer no authority.
"""

from dataclasses import asdict
from hashlib import sha256
import struct

from .model import Result, Span
from .pdb_contributions import bound_contributions
from .pdb_fixups import bound_fixups
from .pe_debug import _mapped, linked_coff_symbols
from .pe_output_residual import _geometry


def verify_debug_storage(image, pdb, row):
    """Qualify exact PE/PDB-backed generated storage, not input retention."""
    if type(row.get("size")) is not int or row["size"] not in (28, 56) or row.get("flags") != 0x40000040:
        raise ValueError("unsupported readonly debug-directory allocation shape")
    if type(pdb) is not bytes:
        raise ValueError("debug storage requires this image's physical PDB")
    _geometry(image)
    inventory = bound_contributions(image, pdb)
    index = row.get("index")
    if type(index) is not int or not 0 <= index < len(inventory.contributions):
        raise ValueError("debug contribution index is absent from the physical PDB")
    contribution = inventory.contributions[index]
    if any(type(row.get(key)) is not int or row[key] != value
           for key, value in asdict(contribution).items()):
        raise ValueError("debug contribution differs from its physical PDB record")
    module = inventory.modules[contribution.module]
    if module.name != "* Linker *" or module.object_name:
        raise ValueError("debug contribution lacks native linker-module identity")
    region = image.sections[contribution.section - 1][0]
    address = region.start + contribution.offset
    if type(row.get("address")) is not int or row["address"] != address:
        raise ValueError("debug contribution address differs from physical coordinates")
    span = Span(address, address + contribution.size)
    if (contribution.size not in (28, 56) or contribution.flags != 0x40000040
            or image.section_flags[contribution.section - 1] != 0x40000040):
        raise ValueError("unsupported readonly debug-directory allocation shape")
    # Re-read the complete native population, not just the caller's filtered
    # contribution list: an omitted competitor must still prevent assignment.
    overlaps = [item for item in inventory.contributions if item.size > 0
        and item.section == contribution.section
        and item.offset < contribution.offset + contribution.size
        and contribution.offset < item.offset + item.size]
    if len(overlaps) != 1:
        raise ValueError("debug allocation overlaps another physical contribution")
    rva, size = image.directories[6] if len(image.directories) > 6 else (0, 0)
    if rva != address - image.base or size != 28:
        raise ValueError("debug allocation is not the exact single-entry directory start")
    linked_coff_symbols(image)  # Full directory and payload physical framing.
    contents, file_offset = _mapped(image, rva, contribution.size)
    flags, _, major, minor, kind, length, payload_rva, payload_offset = struct.unpack(
        "<IIHHIIII", contents[:28])
    if flags or major or minor or kind != 2 or payload_rva:
        raise ValueError("debug allocation requires one overlay NB10 directory entry")
    suffix_size = contribution.size - 28
    if contents[28:] != bytes(suffix_size):
        raise ValueError("debug allocation suffix is not retained zero bytes")
    # This reuses the native NB10/DBI/FIXUP/section-header binding, including
    # the complete terminated payload, rather than accepting a byte signature.
    fixups = bound_fixups(image, pdb)
    if any(site < rva + contribution.size and rva < site + 4 for site in fixups):
        raise ValueError("PDB fixup write intersects the debug allocation")
    # No other directly declared loader directory may reuse these bytes.
    for number, (other_rva, other_size) in enumerate(image.directories):
        if number in (4, 6) or not other_size:  # Certificates use file offsets.
            continue
        if other_rva < rva + contribution.size and rva < other_rva + other_size:
            raise ValueError("debug allocation overlaps another PE directory")
    if len(image.directories) > 4:
        offset, count = image.directories[4]
        if count and offset < file_offset + contribution.size and file_offset < offset + count:
            raise ValueError("debug allocation overlaps certificate file storage")
    return {"result": Result.PASS, "kind": ("nb10_directory_with_retained_zero_suffix"
            if suffix_size else "nb10_debug_directory"),
            "image_sha256": inventory.image_sha256, "pdb_sha256": inventory.pdb_sha256,
            "address": span.start, "size": contribution.size, "directory_size": size,
            "retained_zero_suffix_size": suffix_size, "sha256": sha256(contents).hexdigest(),
            "payload_sha256": sha256(image.data[payload_offset:payload_offset + length]).hexdigest(),
            "byte_projection": False, "ready_for_collection": False}
