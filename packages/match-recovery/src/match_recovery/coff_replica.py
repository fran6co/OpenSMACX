"""Conservative controls for compiling an unchanged TU through a source replica.

These compare compiler outputs with one another, never against the original
executable. A control can refuse harmless representation differences; it must
not turn a candidate's successful compilation into a recovery claim.
"""

import struct

from .model import digest
from .pe_coff import weak_external


def _section_keys(obj, overrides=None):
    """Explicit names and associative ownership, never byte-similarity pairing."""
    keys = {}
    for number, section in enumerate(obj.sections, 1):
        names = sorted((symbol.name, symbol.value, symbol.type, symbol.storage)
                       for symbol in obj.symbols.values() if symbol.section == number and symbol.storage != 103
                       and not (symbol.name == section.name and symbol.storage == 3 and symbol.type == 0))
        if names:
            keys[number] = "named:" + digest(names)
    keys.update(overrides or {})

    def key(number):
        if number not in keys:
            section = obj.sections[number - 1]
            keys[number] = ("associated:" + key(obj.associations[number]) + ":" + section.name
                            if number in obj.associations else "section:" + section.name)
        return keys[number]

    for number in range(1, len(obj.sections) + 1):
        key(number)
    if len(set(keys.values())) != len(keys):
        raise ValueError("replica control requires unambiguous section names or associative owners")
    return keys


def _object_evidence(obj, *, excluded=frozenset(), section_keys=None, entry_names=None):
    """All sections/relocations/symbols except file and CodeView line records.

    COFF timestamps are already omitted by the parser. Keep type records, BSS
    sizes, link directives, COMDAT auxiliary metadata and relocation identities.
    Section/symbol indices are compiler-local. Normalize them only through
    explicit names and associative ownership. Ambiguous identities are refused.
    Function-debug auxiliary links use those same identities; line-table file
    offsets retain presence, not location (line-table contents are not compared).

    Auxiliary layouts follow Microsoft's PE/COFF specification:
    https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#auxiliary-symbol-records
    """
    if any(section.name == ".debug$P" for section in obj.sections):
        raise ValueError("replica control requires standalone objects, not external precompiled types")
    keys = _section_keys(obj, section_keys)
    debug = {index for index, section in enumerate(obj.sections, 1) if section.name == ".debug$S"}
    omitted = debug | set(excluded)
    entry_names = entry_names or {}
    referenced = {target for number, section in enumerate(obj.sections, 1) if number not in omitted
                  for _, target, _ in section.relocations}

    def symbol_key(symbol):
        return (entry_names.get(symbol.index, symbol.name), symbol.value,
                keys[symbol.section] if symbol.section > 0 else str(symbol.section), symbol.type, symbol.storage)

    def auxiliary(index, symbol):
        raw = obj.auxiliary.get(index, b"")
        if not raw:
            return ("none",)
        def debug_reference(target_index, *, begin=False):
            if target_index == 0:
                return ("end",)
            target = obj.symbols.get(target_index)
            valid = (target is not None and target.section > 0 and
                     ((target.name == ".bf" and target.storage == 101 and target.type == 0) if begin else
                      (target.type == 0x20 and target.storage in (2, 3))))
            if not valid:
                raise ValueError("invalid COFF function-debug auxiliary reference")
            return symbol_key(target)

        if symbol.section > 0 and symbol.type == 0x20 and symbol.storage in (2, 3):
            # Format 1. VC6 also emits it for STATIC function definitions.
            # Longer FILE auxiliary strings renumber these symbol-table links
            # and move the line table, without changing executable contents.
            if len(raw) != 18:
                raise ValueError("unsupported COFF function-definition auxiliary extent")
            tag, size, lines, following, unused = struct.unpack("<IIIIH", raw)
            begin = debug_reference(tag, begin=True)
            if tag and obj.symbols[tag].section != symbol.section:
                raise ValueError("COFF function-debug tag belongs to another section")
            return ("function", begin, size, bool(lines), debug_reference(following), unused)
        if symbol.section > 0 and symbol.type == 0 and symbol.storage == 101 and symbol.name == ".bf":
            # Format 2. Retain line/unused fields; only normalize the next .bf
            # index. .ef has no such link and stays a conservative raw record.
            if len(raw) != 18:
                raise ValueError("unsupported COFF begin-function auxiliary extent")
            following, = struct.unpack_from("<I", raw, 12)
            return ("begin-function", raw[:12].hex(), debug_reference(following, begin=True), raw[16:].hex())
        if symbol.storage == 105:
            target, mode = weak_external(obj, symbol)
            return ("weak", symbol_key(target), mode)
        if symbol.section > 0 and symbol.storage == 3 and symbol.type == 0 and symbol.name == obj.sections[symbol.section - 1].name:
            # Coff already validates this section-definition auxiliary record.
            length, relocs, lines, checksum, parent, selection = struct.unpack_from("<IHHIHB", raw)
            # ANY and ASSOCIATIVE selection do not select by checksum. VC6 can
            # change these cached checksums when numbering/reordering symbols.
            # Compare actual section bytes/references and retain selection/owner;
            # EXACT_MATCH checksums and other unknown cases stay conservative.
            if selection in (2, 5):
                checksum = None
            return ("section", length, relocs, lines, checksum, keys[parent] if selection == 5 else parent, selection)
        # Unknown auxiliary forms remain conservative byte comparisons, not
        # inferred aliases or discarded linker metadata.
        return ("raw", raw.hex())

    sections = sorted((keys[index], section.name, section.flags, obj.section_sizes[index - 1], section.data.hex(),
                 [(offset, symbol_key(obj.symbols[target]), kind) for offset, target, kind in section.relocations])
                for index, section in enumerate(obj.sections, 1) if index not in omitted)
    symbols = sorted((symbol_key(symbol), auxiliary(index, symbol))
                     for index, symbol in obj.symbols.items() if symbol.storage != 103
                     and (symbol.section not in omitted or
                          (symbol.section in excluded and symbol.storage == 2 and index not in entry_names))
                     # A new ordinary undefined callee used only by scoped code
                     # is not an outside definition. Common storage, weak aliases
                     # and auxiliary metadata can affect linkage and stay visible.
                     and not (excluded and symbol.section == 0 and symbol.storage == 2 and symbol.value == 0
                              and not obj.auxiliary.get(index) and index not in referenced))
    return {"sections": sections, "symbols": symbols,
            "associations": sorted((keys[child], keys[parent]) for child, parent in obj.associations.items()
                                   if child not in excluded)}


def object_signature(obj):
    return digest(_object_evidence(obj))


def function_signature(obj, name):
    """Selected code and stable external references, not compiler-local IDs.

    Local section/name churn cannot prove that an edit reached this function.
    A control changing only a local relocation's destination is conservatively
    refused; use a control with an observable change to the selected code.
    """
    symbol = obj.function(name)
    return digest([{"bytes": obj.sections[number - 1].data.hex(),
                    "relocations": [(offset, ("external", obj.symbols[target].name)
                                     if obj.symbols[target].storage == 2 else ("local",), kind)
                                    for offset, target, kind in obj.sections[number - 1].relocations]}
                   for number in obj.code_sections(symbol)])
