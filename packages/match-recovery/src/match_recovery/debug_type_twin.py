"""Standalone CodeView types for a body object compiled without debug records.

A measured function body keeps the object compiled with its exact measured
flags. Bounded data dependencies in that same object still need standalone
CodeView types (`/Z7`) for their extents. A second, debug-typed compile of the
same unit may supply them only after this module proves that it is the same
object apart from its `.debug$S`/`.debug$T` records: every other section,
relocation identity, symbol, COMDAT selection/ownership and weak edge must be
equal under an explicit section renumbering. The renumbering may reorder
COMDAT sections, paired by the external names they define (`/Z7` can emit
compiler thunks in another order); the comparison then verifies the pairing. Line-number fields that debug
records add (section-definition line counts, function-definition links and
`.bf`/`.lf`/`.ef` symbols) and the label symbols `/Z7` adds for named C++
`goto` labels (storage class 6, `$label$N`, defined in a code section, with
no auxiliary record and no relocation from any non-debug section) are the only
tolerated differences. Compiler-numbered `$L<n>` labels (storage class 6, in a
code section, no auxiliary record) that no non-debug section relocates are
not linkable content in either object; `/Z7` adds such scope labels for its
CodeView records, so both sides drop them before comparing. Anything else refuses; the plain object then remains
without type evidence.

Layout readers resolve a measured extent in the twin and translate only its
section number back into the plain object's numbering.
"""

from __future__ import annotations

from collections import Counter
from dataclasses import dataclass
import re
import struct

from .pe_coff import weak_external

_DEBUG = (".debug$S", ".debug$T")
_COMDAT = 0x1000  # IMAGE_SCN_LNK_COMDAT
_DEBUG_FUNCTION_STORAGE = 101  # .bf/.lf/.ef line-number function records
_FILE_STORAGE = 103
# Compiler-numbered local labels, EH tables and literals. Debug records shift
# their counters; a local's identity is its defining section and offset.
_NUMBERED_LOCAL = re.compile(r"\$(?:L|T|SG|S)\d+")
_LABEL_STORAGE = 6
_NUMBERED_LABEL = re.compile(r"\$L\d+")
# A named source label as `/Z7` records it for CodeView: `$<name>$<counter>`.
_SOURCE_LABEL = re.compile(r"\$[A-Za-z_][A-Za-z0-9_]*\$\d+")


@dataclass(frozen=True)
class DebugTypeTwin:
    twin: object
    section_map: tuple[tuple[int, int], ...]  # (twin section, plain section)

    def plain_section(self, twin_section: int) -> int:
        for twin, plain in self.section_map:
            if twin == twin_section:
                return plain
        raise ValueError("debug-typed twin layout names an unmapped section")

    def remap(self, layout: dict) -> dict:
        return {**layout, "section": self.plain_section(layout["section"])}


def has_debug_records(obj) -> bool:
    return any(section.name.startswith(".debug$") and section.name != ".debug$F"
               for section in obj.sections)


def attached_twin(obj):
    """Return the proved twin only for a plain object without its own records."""
    witness = getattr(obj, "debug_type_twin", None)
    if witness is None or has_debug_records(obj):
        return None
    if type(witness) is not DebugTypeTwin:
        raise ValueError("unsupported debug-typed twin evidence")
    return witness


def _section_aux(obj, index, symbol, mapping):
    length, relocs, _lines, checksum, parent, selection = struct.unpack_from(
        "<IHHIHB", obj.auxiliary[index])
    if selection == 5:
        if parent not in mapping:
            raise ValueError("debug-typed twin associates code with a debug section")
        parent = mapping[parent]
    # Only EXACT_MATCH selection depends on the checksum; contents are
    # compared byte for byte below either way.
    return ("section", length, relocs, checksum if selection == 4 else None, parent, selection,
            obj.auxiliary[index][15:].hex())


def _symbols(obj, mapping, *, twin):
    def key(symbol):
        if symbol.section > 0:
            if symbol.section not in mapping:
                raise ValueError("debug-typed twin relocates or defines through a debug section")
            section = ("section", mapping[symbol.section])
        else:
            section = ("special", symbol.section)
        name = symbol.name
        if symbol.storage in (3, 6) and symbol.section > 0 and _NUMBERED_LOCAL.fullmatch(name):
            name = "<compiler-numbered local>"
        return (name, symbol.value, section, symbol.type, symbol.storage)

    referenced = {target for number in mapping for _offset, target, _kind
                  in obj.sections[number - 1].relocations}
    rows = Counter()
    for index, symbol in obj.symbols.items():
        if symbol.storage == _FILE_STORAGE:
            continue  # Source file names, not linkable content.
        if symbol.section > 0 and symbol.section not in mapping:
            if not twin:
                raise ValueError("plain object defines a symbol in an unmapped section")
            continue  # Debug section definitions exist only in the twin.
        if symbol.storage == _DEBUG_FUNCTION_STORAGE:
            if not twin:
                raise ValueError("plain body object unexpectedly carries line-number records")
            if symbol.name not in (".bf", ".lf", ".ef"):
                raise ValueError("unsupported debug-typed twin function record")
            continue
        raw = obj.auxiliary.get(index, b"")
        if (symbol.storage == _LABEL_STORAGE and symbol.section > 0 and not raw
                and index not in referenced and _NUMBERED_LABEL.fullmatch(symbol.name)
                and obj.sections[symbol.section - 1].flags & 0x20):
            continue  # An unreferenced numbered label; never a link target.
        if (twin and symbol.storage == _LABEL_STORAGE and symbol.section > 0 and symbol.type == 0
                and not raw and index not in referenced and _SOURCE_LABEL.fullmatch(symbol.name)
                and obj.sections[symbol.section - 1].flags & 0x20):
            # A goto label's CodeView name; the plain object never needs it.
            continue
        if not raw:
            aux = ("none",)
        elif symbol.storage == 105:
            target, mode = weak_external(obj, symbol)
            aux = ("weak", key(target), mode)
        elif (symbol.section > 0 and symbol.storage == 3 and symbol.type == 0
              and symbol.name == obj.sections[symbol.section - 1].name):
            aux = _section_aux(obj, index, symbol, mapping)
        elif twin and symbol.section > 0 and symbol.type == 0x20 and symbol.storage in (2, 3):
            # Function-definition auxiliary records link only line-number
            # debugging; the plain object must carry no such record.
            if len(raw) != 18:
                raise ValueError("unsupported function-definition auxiliary extent")
            aux = ("none",)
        else:
            aux = ("raw", raw.hex())
        rows[(key(symbol), aux)] += 1
    return rows, key


def _external_anchors(obj, numbers):
    """External names each section defines; a COMDAT's leader is among them."""
    anchors = {number: set() for number in numbers}
    for symbol in obj.symbols.values():
        if symbol.section in anchors and symbol.storage == 2:
            anchors[symbol.section].add(symbol.name)
    return {number: frozenset(names) for number, names in anchors.items()}


def _unique_children(obj, numbers, paired):
    """Unpaired associative children keyed by (parent, name), when unique."""
    keys = {number: (obj.associations[number], obj.sections[number - 1].name)
            for number in numbers if number not in paired and number in obj.associations}
    counts = Counter(keys.values())
    return {key: number for number, key in keys.items() if counts[key] == 1}


def _section_mapping(plain, twin, kept) -> dict[int, int]:
    """Pair twin sections with plain ones; every pairing is verified afterwards.

    `/Z7` may emit COMDAT sections in a different order (measured: VC6
    adjustor/vtordisp thunks such as `??_EMultiWin@@WEEE@AEPAXI@Z`, whose
    order also varies with the checkout path). A COMDAT section defining a
    set of external names that is unique on both sides pairs by those names;
    an associative child pairs with the same-named child of its paired
    parent when that is unique; everything else keeps its relative order.
    The mapping only proposes a renumbering: the caller still compares every
    section's bytes, relocations, symbols and COMDAT ownership through it, so
    a wrong pairing refuses rather than proving anything.
    """
    plain_numbers = list(range(1, len(plain.sections) + 1))
    plain_anchors, twin_anchors = _external_anchors(plain, plain_numbers), _external_anchors(twin, kept)

    def unique(anchors, sections):
        counts = Counter(anchors.values())
        return {names: number for number, names in anchors.items()
                if names and counts[names] == 1 and sections[number - 1].flags & _COMDAT}

    plain_unique, twin_unique = unique(plain_anchors, plain.sections), unique(twin_anchors, twin.sections)
    mapping = {number: plain_unique[names] for names, number in twin_unique.items() if names in plain_unique}
    while True:
        plain_children = _unique_children(plain, plain_numbers, set(mapping.values()))
        added = {number: plain_children[(mapping[parent], name)]
                 for (parent, name), number in _unique_children(twin, kept, set(mapping)).items()
                 if parent in mapping and (mapping[parent], name) in plain_children}
        if not added:
            break
        mapping.update(added)
    rest_twin = [number for number in kept if number not in mapping]
    rest_plain = [number for number in plain_numbers if number not in set(mapping.values())]
    if len(rest_twin) != len(rest_plain):
        raise ValueError("debug-typed twin changes the non-debug section count")
    mapping.update(zip(rest_twin, rest_plain))
    if sorted(mapping.values()) != plain_numbers:
        raise ValueError("debug-typed twin section pairing is not one to one")
    return mapping


def prove_debug_type_twin(plain, twin) -> DebugTypeTwin:
    """Prove that `twin` differs from `plain` only by standalone debug records."""
    if has_debug_records(plain):
        raise ValueError("plain object already carries CodeView records")
    if any(section.name == ".debug$P" for section in twin.sections):
        raise ValueError("debug-typed twin uses precompiled types")
    debug = {number for number, section in enumerate(twin.sections, 1) if section.name in _DEBUG}
    if [twin.sections[n - 1].name for n in sorted(debug)].count(".debug$T") != 1:
        raise ValueError("debug-typed twin requires one standalone CodeView type section")
    if any(section.name.startswith(".debug$") and section.name not in (*_DEBUG, ".debug$F")
           for section in twin.sections):
        raise ValueError("debug-typed twin carries an unsupported debug section")
    kept = [number for number in range(1, len(twin.sections) + 1) if number not in debug]
    if len(kept) != len(plain.sections):
        raise ValueError("debug-typed twin changes the non-debug section count")
    mapping = _section_mapping(plain, twin, kept)
    identity = {number: number for number in range(1, len(plain.sections) + 1)}
    plain_symbols, plain_key = _symbols(plain, identity, twin=False)
    twin_symbols, twin_key = _symbols(twin, mapping, twin=True)
    if plain_symbols != twin_symbols:
        raise ValueError("debug-typed twin changes the COFF symbol table")
    for number in kept:
        expected, actual = plain.sections[mapping[number] - 1], twin.sections[number - 1]
        if (actual.name, actual.flags, actual.data, twin.section_sizes[number - 1]) != (
                expected.name, expected.flags, expected.data, plain.section_sizes[mapping[number] - 1]):
            raise ValueError(f"debug-typed twin changes section {actual.name}")
        if ([(offset, plain_key(plain.symbols[target]), kind) for offset, target, kind in expected.relocations]
                != [(offset, twin_key(twin.symbols[target]), kind) for offset, target, kind in actual.relocations]):
            raise ValueError(f"debug-typed twin changes relocations of {actual.name}")
    twin_associations = {}
    for child, parent in twin.associations.items():
        if child in debug:
            if parent in debug:
                raise ValueError("debug-typed twin associates debug records with debug records")
            continue
        if parent not in mapping:
            raise ValueError("debug-typed twin associates code with a debug section")
        twin_associations[mapping[child]] = mapping[parent]
    if twin_associations != dict(plain.associations):
        raise ValueError("debug-typed twin changes COMDAT ownership")
    for number in debug:
        if twin.sections[number - 1].name == ".debug$T" and (
                number in twin.associations or twin.sections[number - 1].relocations):
            raise ValueError("debug-typed twin type stream is not standalone")
    return DebugTypeTwin(twin, tuple(sorted(mapping.items())))
