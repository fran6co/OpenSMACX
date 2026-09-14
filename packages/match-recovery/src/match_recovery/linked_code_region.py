"""Bound selected code regions without guessing ordinary function boundaries.

An ordinary section may contain assembler entry labels and jump vectors. Its
external entries jointly place the entire section, not inferred functions.
Leading bytes before the first entry remain covered. Other code/data sections
and runtime behavior remain unproved.
"""

from .linked_function import _verify_region, verify_function
from .model import Result, Span, digest
from .pe_coff import unaddressed_end_label


def _ordinary_symbols(obj, subject):
    section = obj.sections[subject.section - 1]
    if (subject.storage != 2 or not 0 <= subject.value < len(section.data) or not subject.type & 0x20
            or not section.flags & 0x20 or section.flags & 0x1000
            or not section.data or len(section.data) != obj.section_sizes[subject.section - 1]
            or obj.code_sections(subject) != (subject.section,)):
        raise ValueError("ordinary linked code requires an exact section and bounded external entry")
    members = [item for item in obj.symbols.values() if item.section == subject.section]
    rows = []
    for item in members:
        if (item.storage not in (2, 3, 6)
                or not (0 <= item.value < len(section.data) or unaddressed_end_label(obj, item))
                or item.type not in (0, 0x20)
                or item.storage == 2 and (item.type != 0x20 or obj.auxiliary.get(item.index))):
            raise ValueError("ordinary linked section has unsupported or unbounded internal symbols")
        if obj.auxiliary.get(item.index) and not (item.name == section.name and item.storage == 3
                                                  and item.value == 0 and item.type == 0):
            raise ValueError("ordinary section has unsupported auxiliary symbol metadata")
        # Preserve distinct names, kinds, offsets and multiplicity. Static
        # function-typed labels do not imply independently bounded functions.
        rows.append({"name": item.name, "offset": item.value, "type": item.type, "storage": item.storage})
    return sorted(rows, key=lambda row: (row["offset"], row["name"], row["type"], row["storage"]))


def _ordinary_placement(obj, subject, owner, mapping, image, objects, *, _index=None):
    from .linked_map_index import index_for
    lookup = index_for(mapping, _index)
    entries = [item for item in obj.symbols.values() if item.section == subject.section and item.storage == 2]
    if len(entries) > 1 or subject.value:
        from .linked_fixups import check_selected
        check_selected(obj, owner, objects)
    bases, names = set(), set()
    for entry in entries:
        if entry.name in names:
            raise ValueError("ordinary section has ambiguous external entry names")
        names.add(entry.name)
        matches = lookup.publics.get(entry.name, ())
        if (len(matches) != 1 or not matches[0].function or not matches[0].segment
                or matches[0].owner.casefold() != owner.casefold()):
            raise ValueError("ordinary entry lacks a unique same-owner code placement")
        bases.add(matches[0].address - entry.value)
    if len(bases) != 1:
        raise ValueError("ordinary entries disagree on their shared section base")
    base, = bases
    for item in obj.symbols.values():
        if item.section != subject.section or item.storage not in (3, 6) or obj.auxiliary.get(item.index):
            continue
        matches = lookup.statics.get((owner.casefold(), item.name), ())
        if matches and (len(matches) != 1 or not matches[0].segment
                        or matches[0].address != base + item.value):
            raise ValueError("ordinary internal symbol has contradictory map placement")
    span = Span(base, base + len(obj.sections[subject.section - 1].data))
    containing = [flags for (region, _, _), flags in zip(image.sections, image.section_flags)
                  if region.start <= span.start and span.end <= region.end]
    if len(containing) != 1 or not containing[0] & 0x20 or not containing[0] & 0x20000000:
        raise ValueError("ordinary section is not wholly placed in executable code")
    image.read(span)
    return base


def select_code_region(obj, name):
    matches = [item for item in obj.symbols.values() if item.name == name
               and item.section > 0 and item.type & 0x20 and item.storage in (2, 3)]
    if len(matches) != 1:
        raise ValueError("linked code requires one defined named entry")
    subject, = matches
    if obj.sections[subject.section - 1].flags & 0x1000:
        return obj.function(name)
    _ordinary_symbols(obj, subject)
    return subject


def verify_code_region(obj, symbol, owner, mapping, image, *, objects=None, fixups=None,
                       folded_frames=None, _index=None, _constants=None, codeview=None):
    subject = select_code_region(obj, symbol.name)
    if subject != symbol:
        raise ValueError("linked code entry differs from its selected object symbol")
    if obj.sections[subject.section - 1].flags & 0x1000:
        if subject.section in obj.associations.values() and fixups is not None:
            from .linked_bundle import verify_bundle
            return verify_bundle(obj, subject, owner, mapping, image, objects=objects, fixups=fixups,
                                 folded_frames=folded_frames, codeview=codeview,
                                 _index=_index, _constants=_constants)
        return verify_function(obj, subject, owner, mapping, image, objects=objects, fixups=fixups,
                               _index=_index, _constants=_constants)
    symbols = _ordinary_symbols(obj, subject)
    from .linked_map_index import index_for
    lookup = index_for(mapping, _index)
    base = _ordinary_placement(obj, subject, owner, mapping, image, objects, _index=lookup)
    proof = _verify_region(obj, subject, owner, mapping, image, objects=objects, fixups=fixups,
                           _index=lookup, _constants=_constants)
    if proof["address"] != base:
        raise ValueError("ordinary code proof differs from its shared section placement")
    return {**proof, "extent_kind": "selected_code_section", "section_symbols": symbols,
            "entry_address": base + subject.value, "entry_offset": subject.value,
            "body_fingerprint": digest({"section": proof["body_fingerprint"], "symbols": symbols,
                                        "entry": subject.name, "entry_offset": subject.value}),
            "other_sections": Result.UNKNOWN}
