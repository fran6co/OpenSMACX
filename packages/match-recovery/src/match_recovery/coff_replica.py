"""Conservative compiler-local identities for replica and preservation controls.

These compare compiler outputs with one another, never against the original
executable. A control can refuse harmless representation differences; it must
not turn a candidate's successful compilation into a recovery claim.
"""

import re
import struct

from .model import digest
from .pe_coff import weak_external


_ASSOCIATIVE_LOCAL = "<coff-associative-local>"
_FUNCTION_LOCAL = "<coff-function-local>"


def _owned_local_names(obj):
    """Identify narrow numeric local labels by their compiler-owned location.

    Applicable to both replicas and baseline preservation, never original-image
    target resolution. The complete key still includes function/associative owner, section,
    offset, type and storage; neither bytes nor relocation edges are omitted.
    """
    if any(symbol.name in (_ASSOCIATIVE_LOCAL, _FUNCTION_LOCAL) for symbol in obj.symbols.values()):
        # Otherwise a real symbol in one object could impersonate a normalized
        # local in the other, even with no within-object collision.
        raise ValueError("reserved compiler-local identity appears in COFF symbols")
    names = {}
    # A numeric code label in the root COMDAT needs a real named function owner,
    # not merely a .text section or adjacency to another symbol. Validate only
    # roots containing eligible labels; ordinary objects need no owner searches.
    roots = {s.section for s in obj.symbols.values() if s.section > 0 and s.section not in obj.associations
             and s.storage == 6 and s.type == 0 and re.fullmatch(r"\$L[0-9]+", s.name)
             and not obj.auxiliary.get(s.index)}
    owners = {}
    for symbol in obj.symbols.values():
        if symbol.section in roots and symbol.storage in (2, 3) and symbol.type & 0x20:
            owners.setdefault(symbol.section, []).append(symbol)
    owned_roots = set()
    for section, functions in owners.items():
        if len(functions) != 1 or functions[0].storage != 2 or functions[0].type != 0x20:
            continue
        try:
            obj.function(functions[0].name)
        except ValueError:
            continue  # Unproven ownership retains exact label spellings.
        owned_roots.add(section)
    for symbol in obj.symbols.values():
        if (symbol.section in obj.associations and symbol.type == 0
                and ((symbol.storage == 6 and re.fullmatch(r"\$L[0-9]+", symbol.name))
                     or (symbol.storage == 3 and re.fullmatch(r"\$T[0-9]+", symbol.name)))
                and not obj.auxiliary.get(symbol.index)):
            names[symbol.index] = _ASSOCIATIVE_LOCAL
        elif (symbol.section in owned_roots and symbol.storage == 6 and symbol.type == 0
              and re.fullmatch(r"\$L[0-9]+", symbol.name) and not obj.auxiliary.get(symbol.index)):
            names[symbol.index] = _FUNCTION_LOCAL
    groups = {}
    for index, normalized in names.items():
        symbol = obj.symbols[index]
        key = (normalized, symbol.section, symbol.value, symbol.type, symbol.storage)
        groups.setdefault(key, []).append(symbol)
    for symbols in groups.values():
        if len(symbols) > 1:
            if len({symbol.name for symbol in symbols}) != len(symbols):
                raise ValueError("ambiguous compiler-local replica identity")
            # Distinct labels may legitimately share an instruction boundary.
            # Normalizing them would merge relocation identities: retain every
            # exact spelling instead, without pairing or collapsing aliases.
            for symbol in symbols:
                names.pop(symbol.index)
    return names


def _replica_names(obj):
    """Additional unnamed-namespace identities for unchanged-source controls.

    VC6 encodes a simple unnamed-namespace definition as
    ?name@?%<primary COFF FILE path><decimal nonce>@@<signature>. Recognize only
    that observed form and a defined function COMDAT or ordinary data symbol.
    Never infer this external alias for original-image or baseline preservation.
    """
    names = _owned_local_names(obj)
    # /Z7 adds FILE records for inline headers and repeated returns to the TU.
    # Only the first FILE is the primary source; never guess among headers or
    # choose a filename merely because it makes two symbol spellings agree.
    files = [obj.auxiliary.get(s.index, b"").split(b"\0", 1)[0]
             for _, s in sorted(obj.symbols.items()) if s.storage == 103]
    if files and files[0]:
        try:
            path = files[0].decode("ascii")
        except UnicodeDecodeError:
            path = ""  # Unknown filename encoding supplies no normalization.
        if path:
            pattern = re.compile(r"^\?([^@?]+)@\?%" + re.escape(path) + r"([0-9]+)@@([^%]+)$")
            recognized = [(s, pattern.fullmatch(s.name)) for s in obj.symbols.values()]
            recognized = [(s, match) for s, match in recognized if match is not None]
            # One TU namespace, not a guessed many-to-one merge of namespaces.
            if len({match[2] for _, match in recognized}) > 1:
                raise ValueError("replica has conflicting unnamed-namespace identities")
            for symbol, match in recognized:
                if symbol.storage != 2 or symbol.section <= 0 or symbol.type not in (0, 0x20):
                    raise ValueError("replica unnamed namespace requires a defined function or data symbol")
                if symbol.type == 0x20:
                    obj.function(symbol.name)  # Validate exclusive code COMDAT ownership.
                elif not obj.sections[symbol.section - 1].flags & 0xC0:
                    raise ValueError("replica unnamed namespace data lacks a data section")
                basename = path.replace("\\", "/").rsplit("/", 1)[-1]
                names[symbol.index] = "?" + match[1] + "@<replica-tu:" + basename + ">@@" + match[3]
    return names


def _validate_names(obj, names):
    # Refuse aliases introduced by normalization; distinct co-located numeric
    # labels already retain their exact spellings in _owned_local_names.
    # A relocation can never borrow the identity of another original symbol.
    seen = {}
    for symbol in obj.symbols.values():
        key = (names.get(symbol.index, symbol.name), symbol.section, symbol.value, symbol.type, symbol.storage)
        if key in seen and (symbol.index in names or seen[key] in names):
            raise ValueError("ambiguous compiler-local replica identity")
        seen[key] = symbol.index


def _section_keys(obj, overrides=None, names=None):
    """Explicit names and associative ownership, never byte-similarity pairing."""
    keys = {}
    for number, section in enumerate(obj.sections, 1):
        symbols = sorted(((names or {}).get(symbol.index, symbol.name), symbol.value, symbol.type, symbol.storage)
                       for symbol in obj.symbols.values() if symbol.section == number and symbol.storage != 103
                       and not (symbol.name == section.name and symbol.storage == 3 and symbol.type == 0))
        # Associative ownership is stronger than a child's compiler-local name.
        # Keep every child symbol/offset below; do not use labels as its owner.
        if symbols and number not in obj.associations:
            keys[number] = "named:" + digest(symbols)
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


def _object_evidence(obj, *, excluded=frozenset(), section_keys=None, entry_names=None, replica=False):
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
    names = _replica_names(obj) if replica else _owned_local_names(obj)
    _validate_names(obj, names)
    keys = _section_keys(obj, section_keys, names)
    debug = {index for index, section in enumerate(obj.sections, 1) if section.name == ".debug$S"}
    omitted = debug | set(excluded)
    entry_names = entry_names or {}
    referenced = {target for number, section in enumerate(obj.sections, 1) if number not in omitted
                  for _, target, _ in section.relocations}
    excluded_references = {target for number, section in enumerate(obj.sections, 1) if number in excluded
                           for _, target, _ in section.relocations}
    # Weak fallbacks are linker edges even when no retained instruction has a
    # relocation to the fallback itself. Keep their target records visible.
    referenced.update(weak_external(obj, symbol)[0].index for symbol in obj.symbols.values()
                      if symbol.storage == 105 and symbol.section not in omitted)

    type_contents = None
    if any(section.name == ".debug$T" for section in obj.sections):
        from .codeview_identity import type_identity
        try:
            # A relocated type stream would need its byte-offset relocations
            # mapped through the graph too. Do not silently detach those edges.
            if any(section.relocations for section in obj.sections if section.name == ".debug$T"):
                raise ValueError("relocated CodeView type stream")
            type_contents = type_identity(obj)
        except ValueError:
            pass  # Exact raw-stream equality remains required, never omission.

    def contents(section):
        if section.name == ".debug$T" and type_contents is not None:
            return ("codeview5-types", type_contents)
        return ("raw", section.data.hex())

    def symbol_key(symbol):
        return (entry_names.get(symbol.index, names.get(symbol.index, symbol.name)), symbol.value,
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
            # Only EXACT_MATCH selection uses content equality to select a
            # duplicate. Other supported policies use presence, size or owner;
            # non-COMDAT .drectve checksums can even vary between identical VC6
            # compiles. Compare all actual bytes/references and retain policy,
            # size and owner rather than trusting these unused checksum fields.
            if selection in (0, 1, 2, 3, 5, 6):
                checksum = None
            return ("section", length, relocs, lines, checksum, keys[parent] if selection == 5 else parent, selection)
        # Unknown auxiliary forms remain conservative byte comparisons, not
        # inferred aliases or discarded linker metadata.
        return ("raw", raw.hex())

    sections = sorted((keys[index], section.name, section.flags, obj.section_sizes[index - 1], contents(section),
                 [(offset, symbol_key(obj.symbols[target]), kind) for offset, target, kind in section.relocations])
                for index, section in enumerate(obj.sections, 1) if index not in omitted)
    symbols = sorted((symbol_key(symbol), auxiliary(index, symbol))
                     for index, symbol in obj.symbols.items() if symbol.storage != 103
                     and (symbol.section not in omitted or
                          (symbol.section in excluded and symbol.storage == 2 and index not in entry_names))
                     # Only an actual excluded edge establishes scoped-only use.
                     # Unreferenced externals can be implicit linker requirements;
                     # keep them even when unrelated scoped functions are removed.
                     # Common storage, weak aliases and opaque metadata stay strict.
                     and not (index in excluded_references and symbol.section == 0
                              and symbol.storage == 2 and symbol.value == 0 and symbol.type in (0, 0x20)
                              and not obj.auxiliary.get(index) and index not in referenced))
    return {"sections": sections, "symbols": symbols,
            "associations": sorted((keys[child], keys[parent]) for child, parent in obj.associations.items()
                                   if child not in excluded)}


def object_signature(obj):
    return digest(_object_evidence(obj, replica=True))


def function_signature(obj, name):
    """Selected code and stable external references, not compiler-local IDs.

    Local section/name churn cannot prove that an edit reached this function.
    A control changing only a local relocation's destination is conservatively
    refused; use a control with an observable change to the selected code.
    """
    symbol = obj.function(name)
    names = _replica_names(obj)
    _validate_names(obj, names)
    return digest([{"bytes": obj.sections[number - 1].data.hex(),
                    "relocations": [(offset, ("external", names.get(target, obj.symbols[target].name))
                                     if obj.symbols[target].storage == 2 else ("local",), kind)
                                    for offset, target, kind in obj.sections[number - 1].relocations]}
                   for number in obj.code_sections(symbol)])
