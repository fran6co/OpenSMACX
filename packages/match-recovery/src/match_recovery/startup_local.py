"""Read-only, slot-local startup-root observations; never authorization.

This does not remove bytes, rename symbols, or prove final-link preservation.
Recognized FPO children are reported, not declared discarded. Bounded order may
compare their complete root-relative structure, but their final-link treatment
and address-identity effects remain independent obligations. A caller must not
silently omit them or use this observation as full linked-effect acceptance.
"""

import struct

from .pe_coff import Symbol, weak_external


def _symbols(obj, number, *, table=False):
    section = obj.sections[number - 1]
    rows, definitions = [], []
    for symbol in obj.symbols.values():
        if symbol.section != number:
            continue
        raw = obj.auxiliary.get(symbol.index, b"")
        definition = (symbol.storage == 3 and symbol.type == 0
                      and symbol.name == section.name and symbol.value == 0)
        if definition:
            if len(raw) != 18:
                raise ValueError("startup section has opaque auxiliary metadata")
            length, relocations, lines, checksum, parent, selection = struct.unpack_from("<IHHIHB", raw)
            if (length != len(section.data) or relocations != len(section.relocations)
                    or lines or any(raw[15:]) or selection not in range(7)
                    or bool(selection) != bool(section.flags & 0x1000)
                    or (selection == 5 and obj.associations.get(number) != parent)
                    or (selection != 5 and parent)):
                raise ValueError("startup section has inconsistent auxiliary metadata")
            definitions.append(symbol)
        elif (raw or symbol.storage not in ((3,) if table else (3, 6))
              or symbol.type not in ((0,) if table else (0, 0x20))
              or not 0 <= symbol.value < len(section.data)
              or (table and symbol.value % 4)):
            raise ValueError("startup section has exposed or opaque symbols")
        rows.append(symbol)
    if len(definitions) > 1:
        raise ValueError("startup section has ambiguous section metadata")
    return rows, definitions


def _fpo_child(obj, number, root):
    """Recognize only the independently bounded, zero-frame FPO_DATA subset.

    FPO_DATA: https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-fpo_data
    Flags alone, a debug spelling, or an arbitrary discardable section are not
    evidence of this format. Nonzero frame descriptions remain unsupported.
    """
    section = obj.sections[number - 1]
    size = len(obj.sections[root.section - 1].data)
    symbols, definitions = _symbols(obj, number)
    if (section.name != ".debug$F" or section.flags != 0x42101048
            or obj.section_sizes[number - 1] != 16
            or section.data != struct.pack("<IIII", 0, size, 0, 0)
            or section.relocations != ((0, root.index, 7),)
            or len(symbols) != 1 or len(definitions) != 1
            or obj.auxiliary[definitions[0].index][14] != 5
            or obj.associations.get(number) != root.section
            or number in obj.associations.values()):
        raise ValueError("startup root has an unsupported associated child")
    return {"section": number, "format": "fpo_data", "symbol": definitions[0].index,
            "code_size": size, "data": section.data.hex(),
            "relocation": [0, root.index, 7]}


def slot_local_root(obj, *, table: int, offset: int, symbol: Symbol) -> dict:
    """Observe an exact private startup slot or refuse an unsupported case.

    The only permitted incoming code address is this one complete ABS32 cell,
    plus a strictly recognized associated FPO record reported to the caller.
    Even a root's own address, another cell, a weak alias or an address of any
    table cell refuses locality. All outgoing references and metadata remain
    untouched. This observation has no capability or acceptance authority.
    """
    if (type(table) is not int or type(offset) is not int or not isinstance(symbol, Symbol)
            or obj.symbols.get(symbol.index) != symbol):
        raise ValueError("invalid startup slot identity")
    tables = [number for number, section in enumerate(obj.sections, 1) if section.name == ".CRT$XCU"]
    if tables != [table]:
        raise ValueError("startup locality requires one ordinary initializer table")
    section = obj.sections[table - 1]
    if (table in obj.associations or table in obj.associations.values()
            or len(section.data) != obj.section_sizes[table - 1] or not section.data
            or len(section.data) % 4 or section.alignment != 4 or any(section.data)
            or section.flags & (0x20 | 0x80 | 0x800 | 0x1000 | 0x02000000 | 0x20000000)
            or not section.flags & 0x40):
        raise ValueError("unsupported startup table storage")
    slots = {at: (target, kind) for at, target, kind in section.relocations}
    if (len(slots) != len(section.relocations) or set(slots) != set(range(0, len(section.data), 4))
            or any(kind != 6 or target not in obj.symbols for target, kind in slots.values())):
        raise ValueError("startup table requires complete zero-addend ABS32 slots")
    if slots.get(offset) != (symbol.index, 6):
        raise ValueError("startup selection differs from actual slot")
    if (symbol.storage != 3 or symbol.type != 0x20 or symbol.section <= 0
            or obj.function(symbol.name) != symbol or symbol.section in obj.associations):
        raise ValueError("startup root requires an exclusive static code COMDAT")
    root = obj.sections[symbol.section - 1]
    if (not root.data or len(root.data) != obj.section_sizes[symbol.section - 1]
            or root.flags & (0x40 | 0x80 | 0x800 | 0x02000000) or not root.flags & 0x20000000):
        raise ValueError("startup root lacks a complete runtime code extent")
    root_symbols, _ = _symbols(obj, symbol.section)
    table_symbols, _ = _symbols(obj, table, table=True)
    root_indices = {item.index for item in root_symbols}
    table_indices = {item.index for item in table_symbols}
    children = [_fpo_child(obj, number, symbol) for number, parent in obj.associations.items()
                if parent == symbol.section]
    if len(children) > 1:
        raise ValueError("startup root has ambiguous FPO metadata")
    debug_indices = {row["symbol"] for row in children}
    debug_edges = {(row["section"], 0, symbol.index, 7) for row in children}
    outgoing, operands = [], set()
    for at, target, kind in root.relocations:
        if (kind not in (6, 20) or target not in obj.symbols or at < 0 or at + 4 > len(root.data)
                or operands.intersection(range(at, at + 4))):
            raise ValueError("startup root has unsupported relocation operands")
        operands.update(range(at, at + 4))
        outgoing.append({"offset": at, "target": target, "kind": kind,
                         "addend": int.from_bytes(root.data[at:at + 4], "little")})
    # No debug-name or discardable-flag shortcut: all other incoming operands
    # count, even if they originate in .debug$S or another .debug$F contribution.
    for number, item in enumerate(obj.sections, 1):
        for at, target, kind in item.relocations:
            if target in table_indices or target in debug_indices:
                raise ValueError("startup table or debug metadata address escapes")
            if target in root_indices and (number, at, target, kind) not in debug_edges:
                if (number, at, target, kind) != (table, offset, symbol.index, 6):
                    raise ValueError("startup root has an unselected incoming or self-address reference")
    protected = root_indices | table_indices | debug_indices
    for item in obj.symbols.values():
        if item.storage == 105:
            target, _ = weak_external(obj, item)
            if target.index in protected:
                raise ValueError("startup address escapes through a weak alias")
        # Known function-definition and begin-function auxiliary references
        # cannot smuggle an additional entry identity past relocation scanning.
        raw = obj.auxiliary.get(item.index, b"")
        if item.section > 0 and item.type == 0x20 and item.storage in (2, 3) and raw:
            if len(raw) != 18:
                raise ValueError("unsupported function auxiliary metadata")
            tag, _, _, following, _ = struct.unpack("<IIIIH", raw)
            if any(index and index in protected for index in (tag, following)):
                raise ValueError("startup address has an auxiliary incoming reference")
        if item.storage == 101 and item.name == ".bf" and raw:
            if len(raw) != 18:
                raise ValueError("unsupported begin-function auxiliary metadata")
            following, = struct.unpack_from("<I", raw, 12)
            if following and following in protected:
                raise ValueError("startup address has an auxiliary incoming reference")
    return {"check": "startup_slot_local_root", "result": "pass", "table": table,
            "offset": offset, "symbol": symbol.name, "symbol_index": symbol.index,
            "root_section": symbol.section, "root_size": len(root.data),
            "root_symbols": [{"index": item.index, "name": item.name, "offset": item.value,
                              "type": item.type, "storage": item.storage} for item in root_symbols],
            "outgoing_references": outgoing, "recognized_debug_children": children,
            "requires_linked_debug_proof": bool(children),
            "limitations": ["COFF locality only; not source ownership, projection or final-link acceptance"]}
