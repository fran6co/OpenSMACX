"""Typed associated CodeView procedures bound to a live native PDB module.

This proves one complete procedure-symbol child, not treatment of all debug
information, retained storage, hierarchy, or outside-scope preservation.
Executable relocation decoding remains deliberately separate from SECREL/SECTION.
"""

from dataclasses import asdict
import struct

from .codeview_procedures import decode_procedure, S_GPROC32_ST, S_LPROC32_ST, S_END
from .linked_fixups import check_image, check_selected
from .model import Result, Span
from .msf import SmallMsf
from .pdb_contributions import bound_contributions
from .pe_coff import weak_external


def source_procedure(obj, number, root):
    """Validate complete COFF ownership and its exact typed relocation pair."""
    if (type(number) is not int or not 1 <= number <= len(obj.sections)
            or obj.symbols.get(root.index) != root or obj.function(root.name) != root
            or root.value != 0 or root.section in obj.associations):
        raise ValueError("CodeView child requires an exact independent function root")
    section = obj.sections[number - 1]
    if (section.name != ".debug$S" or section.flags != 0x42101048
            or len(section.data) != obj.section_sizes[number - 1]
            or obj.associations.get(number) != root.section or number in obj.associations.values()
            or sum(parent == root.section and obj.sections[child - 1].name == ".debug$S"
                   for child, parent in obj.associations.items()) != 1
            or section.relocations != ((32, root.index, 11), (36, root.index, 10))):
        raise ValueError("unsupported associated CodeView ownership or relocation pair")
    symbols = [symbol for symbol in obj.symbols.values() if symbol.section == number]
    if len(symbols) != 1:
        raise ValueError("CodeView child has aliases or ambiguous section definition")
    definition, = symbols
    auxiliary = obj.auxiliary.get(definition.index, b"")
    if (definition.name != section.name or definition.storage != 3 or definition.type
            or definition.value or len(auxiliary) != 18):
        raise ValueError("CodeView child lacks an exact section definition")
    size, count, lines, _, parent, selection = struct.unpack_from("<IHHIHB", auxiliary)
    if (size != len(section.data) or count != 2 or lines or parent != root.section
            or selection != 5 or any(auxiliary[15:])):
        raise ValueError("CodeView child has contradictory section metadata")
    if any(target == definition.index for item in obj.sections for _, target, _ in item.relocations):
        raise ValueError("CodeView child has an incoming address reference")
    if any(symbol.storage == 105 and weak_external(obj, symbol)[0].index == definition.index
           for symbol in obj.symbols.values()):
        raise ValueError("CodeView child has a weak address alias")
    procedure = decode_procedure(section.data)
    if (procedure.parent or procedure.end or procedure.next or procedure.offset or procedure.section
            or procedure.kind != (S_GPROC32_ST if root.storage == 2 else S_LPROC32_ST)):
        raise ValueError("CodeView source procedure has unsupported links, binding addends or visibility")
    code = obj.sections[root.section - 1]
    if (not 0 < procedure.length <= len(code.data)
            or len(code.data) != procedure.length + (-procedure.length % code.alignment)
            or any(byte not in (0x90, 0xCC) for byte in code.data[procedure.length:])):
        raise ValueError("CodeView procedure extent leaves non-alignment function bytes")
    return procedure


def module_procedures(data):
    """Frame a complete CV5 module; decode every supported procedure in full.

    Other module records retain their framing, not a semantic permission. A
    candidate procedure cannot terminate inside one of those records or absorb
    another procedure. No byte search is used to locate records or definitions.
    """
    if type(data) is not bytes or not 4 <= len(data) <= 16 * 1024 * 1024 or data[:4] != b"\x02\0\0\0":
        raise ValueError("unsupported bounded CodeView module symbol stream")
    frames, at = {}, 4
    while at < len(data):
        if at + 4 > len(data) or len(frames) >= 100000:
            raise ValueError("truncated or oversized CodeView module records")
        size, kind = struct.unpack_from("<HH", data, at)
        end = at + 2 + size
        if size < 2 or end > len(data) or end % 4:
            raise ValueError("invalid CodeView module record extent")
        frames[at] = end, kind
        at = end
    result, consumed = [], set()
    for start, (end, kind) in frames.items():
        if kind not in (S_GPROC32_ST, S_LPROC32_ST):
            continue
        if end - start < 39 or start in consumed:
            raise ValueError("truncated or nested CodeView module procedure")
        parent, final, following = struct.unpack_from("<III", data, start + 4)
        if (parent or following or final <= start or final not in frames
                or frames[final][1] != S_END):
            raise ValueError("unsupported CodeView module procedure scope links")
        terminal = frames[final][0]
        procedure = decode_procedure(data[start:terminal], alignment=4, base_offset=start)
        if procedure.end_record_offset != final:
            raise ValueError("CodeView procedure end link differs from its complete records")
        offsets = {row.offset for row in procedure.records}
        if consumed & offsets:
            raise ValueError("overlapping CodeView module procedure scopes")
        consumed.update(offsets)
        result.append(procedure)
    return tuple(result)


def procedure_correspondence(source, target, source_types, target_types):
    """Compare complete decoded procedure contents; caller owns provenance."""
    from .codeview_type_graph import compare_type_roots

    fields = ("kind", "length", "debug_start", "debug_end", "flags", "name")
    if any(getattr(source, field) != getattr(target, field) for field in fields):
        raise ValueError("CodeView linked procedure metadata differs from its source")
    locals_ = lambda value: [{key: item for key, item in asdict(local).items() if key != "type_index"}
                            for local in value.locals]
    if locals_(source) != locals_(target):
        raise ValueError("CodeView linked local metadata differs from its source")
    types = compare_type_roots(source_types,
        (source.type_index, *(item.type_index for item in source.locals)), target_types,
        (target.type_index, *(item.type_index for item in target.locals)))
    return {"format": "codeview5_procedure", **{field: getattr(source, field) for field in fields},
            "locals": locals_(source), "types": types}


class LinkedProcedures:
    """Ephemeral verifier; module labels alone never select physical objects."""

    def __init__(self, linked):
        self.linked = linked

    def verify(self, obj, number, root, owner, image, address, fixups):
        from .codeview_type_graph import coff_type_records, tpi_type_records

        linked = self.linked
        try:
            linked.check_current()
            if image is not linked.inputs.image or fixups is not linked.inputs.fixups:
                raise ValueError("CodeView proof belongs to another native linked build")
            check_image(fixups, image)
            selected = check_selected(obj, owner, linked.inputs.objects)
            source = source_procedure(obj, number, root)
            if type(address) is not int or address < image.base:
                raise ValueError("invalid linked CodeView procedure address")
            mapping = linked.inputs.mapping
            pool = mapping.publics if root.storage == 2 else mapping.statics or ()
            entries = [item for item in pool if item.symbol == root.name and item.function and item.segment
                       and item.owner.casefold() == selected["owner"].casefold()]
            if len(entries) != 1 or entries[0].address != address:
                raise ValueError("CodeView procedure disagrees with its independent owner-qualified MAP entry")
            matches = [(index, region) for index, ((region, _, _), flags) in enumerate(
                zip(image.sections, image.section_flags), 1)
                if region.start <= address and address + source.length <= region.end
                and flags & 0x20 and flags & 0x20000000]
            if len(matches) != 1:
                raise ValueError("CodeView procedure lacks a unique executable image extent")
            segment, region = matches[0]
            image.read(Span(address, address + source.length))
            bindings = linked.module_bindings()
            modules = [row for row in bindings["modules"] if row.get("input") == selected["identity"]
                       and row.get("object_sha256") == selected["sha256"] and row["result"] == Result.PASS]
            if len(modules) != 1:
                raise ValueError("CodeView procedure lacks a unique physically bound PDB module")
            pdb = linked.inputs.artifacts["msvc-pdb"]
            inventory = bound_contributions(image, pdb)
            module = inventory.modules[modules[0]["index"]]
            if module.stream is None or module.c13_bytes:
                raise ValueError("CodeView module has no supported procedure stream")
            msf = SmallMsf(pdb)
            procedures = module_procedures(msf.stream(module.stream)[:module.symbol_bytes])
            placed = [item for item in procedures if item.section == segment
                      and item.offset == address - region.start]
            if len(placed) != 1:
                raise ValueError("CodeView linked procedure placement is missing or ambiguous")
            target, = placed
            return procedure_correspondence(source, target, coff_type_records(obj), tpi_type_records(msf.stream(2)))
        finally:
            linked.check_current()
