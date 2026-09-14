"""Exact independent COFF COMDAT keys, never link-time selection authority.

Microsoft PE/COFF: the first symbol for a COMDAT section is its section
definition, and the second is its COMDAT key. A later same-named entry or
ordinary symbol cannot substitute. The native caller must bind parsed records
to fresh physical inputs and independently prove the retained winner.
https://learn.microsoft.com/en-us/windows/win32/debug/pe-format#comdat-sections-object-only
"""

import struct

from .pe_coff import Coff


def _definition(obj, number):
    if (type(obj) is not Coff or type(number) is not int
            or not 1 <= number <= len(obj.sections)):
        raise ValueError("COMDAT metadata requires a strict COFF object and exact section")
    section = obj.sections[number - 1]
    if not section.flags & 0x1000 or section.name.startswith("/"):
        raise ValueError("COMDAT metadata requires an explicitly named COMDAT section")
    symbols = sorted((symbol for symbol in obj.symbols.values() if symbol.section == number),
                     key=lambda symbol: symbol.index)
    if not symbols:
        raise ValueError("COMDAT section has no section-definition symbol")
    definition = symbols[0]
    raw = obj.auxiliary.get(definition.index, b"")
    if (definition.name != section.name or definition.value != 0
            or definition.type != 0 or definition.storage != 3 or len(raw) != 18):
        raise ValueError("first COMDAT section symbol is not its exact section definition")
    size, relocations, _, _, parent, selection = struct.unpack_from("<IHHIHB", raw)
    if (size != obj.section_sizes[number - 1] or relocations != len(section.relocations)
            or selection not in range(1, 7) or any(raw[15:])):
        raise ValueError("COMDAT section definition is inconsistent")
    if selection == 5:
        if obj.associations.get(number) != parent:
            raise ValueError("COMDAT association differs from its parsed parent")
    elif parent or number in obj.associations:
        raise ValueError("independent COMDAT has unexplained parent metadata")
    return symbols, selection


def comdat_selection(obj, section):
    """Read the exact section policy; no winner or discard is inferred."""
    return _definition(obj, section)[1]


def comdat_key(obj, section):
    """Return the exact external key of an independent code/data COMDAT.

    Associative children follow their parent; they do not get independent keys
    through a convenient symbol spelling. Unsupported local-key forms refuse.
    """
    symbols, selection = _definition(obj, section)
    if selection == 5 or len(symbols) < 2:
        raise ValueError("COMDAT key requires an independent section and its second symbol")
    key = symbols[1]
    if (key.storage != 2 or key.value != 0 or key.type not in (0, 0x20)
            or not key.name or key.name == obj.sections[section - 1].name
            or obj.auxiliary.get(key.index)):
        raise ValueError("second COMDAT section symbol is not a supported external key")
    return key
