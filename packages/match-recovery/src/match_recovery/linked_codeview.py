"""Typed associated CodeView procedures bound to a live native PDB module.

This proves one complete procedure-symbol child, not treatment of all debug
information, retained storage, hierarchy, or outside-scope preservation.
Executable relocation decoding remains deliberately separate from SECREL/SECTION.
"""

from collections import Counter
from dataclasses import asdict
import re
import struct

from .codeview_procedures import (decode_procedure, S_GPROC32_ST, S_LPROC32_ST, S_END,
                                  S_LDATA32_ST, S_LABEL32_ST)
from .linked_fixups import check_image, check_selected
from .model import Result, Span
from .msf import SmallMsf
from .pdb_contributions import bound_contributions
from .pe_coff import weak_external


_USER_LABEL = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_LABEL_SERIAL = re.compile(r"[1-9][0-9]*")


def label_names_agree(codeview, coff):
    """Match one S_LABEL32 name to its COFF label symbol name.

    VC6 compiler-generated labels (``$L52070``) carry the same name in both
    places. A user label (C++ ``goto`` target or ``__asm`` label) keeps its
    source identifier in CodeView while the COFF symbol is decorated as
    ``$<identifier>$<serial>``, the serial sharing the compiler's decimal label
    counter. Any other spelling is refused.
    """
    if codeview == coff:
        return True
    if not _USER_LABEL.fullmatch(codeview) or not coff.startswith(f"${codeview}$"):
        return False
    return _LABEL_SERIAL.fullmatch(coff[len(codeview) + 2:]) is not None


def _eh_tail_labels(data, obj, eh_tail):
    """Frame VC6's leading S_LABEL32 records for a function's `.text$x` tail.

    VC6 writes one label record per EH-tail label before the procedure itself.
    Each must be a complete, unpadded, zero-coordinate record; its binding is
    checked by the caller against exact relocation pairs.
    """
    labels, at = [], 0
    while at + 4 <= len(data):
        size, kind = struct.unpack_from("<HH", data, at)
        if kind != S_LABEL32_ST:
            break
        if size < 10 or at + 2 + size > len(data):
            raise ValueError("truncated CodeView EH-tail label record")
        offset, section, flags, length = struct.unpack_from("<IHBB", data, at + 4)
        if offset or section or flags or size != 10 + length or not length:
            raise ValueError("unsupported CodeView EH-tail label record")
        labels.append((at, data[at + 12:at + 12 + length].decode("ascii")))
        at += 2 + size
    if not labels or len(labels) > len(obj.sections[eh_tail - 1].data):
        raise ValueError("EH-tail CodeView block lacks its leading tail labels")
    return labels, at


def source_procedure(obj, number, root, *, local_addresses=False, eh_tail=None):
    """Validate complete COFF ownership and exact typed relocation pairs.

    Optional local addresses admit only unnamed untyped data and ordinary
    named labels, each bound to one unique local symbol in this root. This
    proves object-local coordinates, not original types or PDB correspondence.
    `eh_tail` (with local addresses) names the root's caller-proved `.text$x`
    section; its leading compiler labels must bind uniquely into that tail.
    """
    if eh_tail is not None and (not local_addresses or type(eh_tail) is not int
                                or obj.associations.get(eh_tail) != root.section
                                or obj.sections[eh_tail - 1].name != ".text$x"):
        raise ValueError("CodeView EH-tail labels require one associative .text$x tail")
    if (type(local_addresses) is not bool
            or type(number) is not int or not 1 <= number <= len(obj.sections)
            or obj.symbols.get(root.index) != root or obj.function(root.name) != root
            or root.value != 0 or root.section in obj.associations):
        raise ValueError("CodeView child requires an exact independent function root")
    section = obj.sections[number - 1]
    if (section.name != ".debug$S" or section.flags != 0x42101048
            or len(section.data) != obj.section_sizes[number - 1]
            or obj.associations.get(number) != root.section or number in obj.associations.values()
            or sum(parent == root.section and obj.sections[child - 1].name == ".debug$S"
                   for child, parent in obj.associations.items()) != 1
            or (not local_addresses and section.relocations != ((32, root.index, 11), (36, root.index, 10)))
            or (eh_tail is not None and eh_tail in obj.associations.values())):
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
    if (size != len(section.data) or count != len(section.relocations) or lines or parent != root.section
            or selection != 5 or any(auxiliary[15:])):
        raise ValueError("CodeView child has contradictory section metadata")
    if any(target == definition.index for item in obj.sections for _, target, _ in item.relocations):
        raise ValueError("CodeView child has an incoming address reference")
    if any(symbol.storage == 105 and weak_external(obj, symbol)[0].index == definition.index
           for symbol in obj.symbols.values()):
        raise ValueError("CodeView child has a weak address alias")
    tail_labels, start = _eh_tail_labels(section.data, obj, eh_tail) if eh_tail is not None else ((), 0)
    procedure = decode_procedure(section.data[start:], base_offset=start, local_addresses=local_addresses)
    if (procedure.parent or procedure.end or procedure.next or procedure.offset or procedure.section
            or procedure.kind != (S_GPROC32_ST if root.storage == 2 else S_LPROC32_ST)):
        raise ValueError("CodeView source procedure has unsupported links, binding addends or visibility")
    code = obj.sections[root.section - 1]
    if (not 0 < procedure.length <= len(code.data)
            or len(code.data) != procedure.length + (-procedure.length % code.alignment)
            or any(byte not in (0x90, 0xCC) for byte in code.data[procedure.length:])):
        raise ValueError("CodeView procedure extent leaves non-alignment function bytes")
    by_offset = {offset: (offset, index, kind) for offset, index, kind in section.relocations}
    if len(by_offset) != len(section.relocations):
        raise ValueError("CodeView child has duplicate relocation offsets")
    expected = []
    if eh_tail is not None:
        tail = obj.sections[eh_tail - 1]
        tail_counts = Counter(symbol.value for symbol in obj.symbols.values()
                              if symbol.section == eh_tail and symbol.storage == 6)
        tail_used = set()
        for record_offset, label in tail_labels:
            at = record_offset + 4
            pairs = [by_offset.get(at), by_offset.get(at + 4)]
            if (any(pair is None for pair in pairs) or pairs[0][2] != 11
                    or pairs[1] != (at + 4, pairs[0][1], 10)):
                raise ValueError("CodeView EH-tail label lacks an exact SECREL/SECTION pair")
            target = obj.symbols.get(pairs[0][1])
            if (target is None or target.section != eh_tail or target.storage != 6 or target.type
                    or not 0 <= target.value < len(tail.data) or target.index in tail_used
                    or tail_counts[target.value] != 1 or target.name != label):
                raise ValueError("CodeView EH-tail label lacks a unique owned tail label")
            tail_used.add(target.index)
            expected.extend(pairs)
    expected.extend([(start + 32, root.index, 11), (start + 36, root.index, 10)])
    label_counts = Counter(symbol.value for symbol in obj.symbols.values()
                           if symbol.section == root.section and symbol.storage == 6)
    used = set()
    for address in procedure.addresses:
        if address.offset or address.section:
            raise ValueError("CodeView local address has nonzero relocation addends")
        if address.kind == S_LDATA32_ST:
            if address.type_index != 0 or address.name:
                raise ValueError("only unnamed untyped procedure-local data is supported")
            at = address.record_offset + 8
        elif address.kind == S_LABEL32_ST:
            if address.flags or not address.name:
                raise ValueError("unsupported procedure-local label flags or name")
            at = address.record_offset + 4
        else:
            raise ValueError("unsupported procedure-local address record")
        pairs = [by_offset.get(at), by_offset.get(at + 4)]
        if (any(pair is None for pair in pairs) or pairs[0][2] != 11
                or pairs[1] != (at + 4, pairs[0][1], 10)):
            raise ValueError("CodeView local address lacks an exact SECREL/SECTION pair")
        target = obj.symbols.get(pairs[0][1])
        if (target is None or target.section != root.section or target.storage != 6
                or target.type or not 0 <= target.value < procedure.length
                or target.index in used
                or label_counts[target.value] != 1
                or (address.kind == S_LABEL32_ST and not label_names_agree(address.name, target.name))):
            raise ValueError("CodeView local address lacks a unique owned root label")
        used.add(target.index)
        expected.extend(pairs)
    if tuple(expected) != section.relocations:
        raise ValueError("CodeView child has unexplained or reordered relocations")
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
    if source.addresses or target.addresses:
        raise ValueError("linked procedure-local address correspondence is not supported")
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
