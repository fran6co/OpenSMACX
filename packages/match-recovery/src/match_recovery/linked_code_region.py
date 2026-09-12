"""Bound selected code regions without guessing ordinary function boundaries.

An ordinary section may contain assembler entry labels and jump vectors. Its
single external zero-offset anchor places the entire section, not one inferred
function. Other code/data sections and runtime behavior remain unproved.
"""

from .linked_function import _verify_region, verify_function
from .model import Result


def _ordinary_symbols(obj, subject):
    section = obj.sections[subject.section - 1]
    if (subject.storage != 2 or subject.value != 0 or not subject.type & 0x20
            or not section.flags & 0x20 or section.flags & 0x1000
            or not section.data or len(section.data) != obj.section_sizes[subject.section - 1]
            or obj.code_sections(subject) != (subject.section,)):
        raise ValueError("ordinary linked code requires an exact section and external zero-offset anchor")
    members = [item for item in obj.symbols.values() if item.section == subject.section]
    if [item.index for item in members if item.storage == 2] != [subject.index]:
        raise ValueError("ordinary linked section has additional external entries or aliases")
    rows = []
    for item in members:
        if (item.storage not in (2, 3, 6) or not 0 <= item.value < len(section.data)
                or item.type not in (0, 0x20)):
            raise ValueError("ordinary linked section has unsupported or unbounded internal symbols")
        # Preserve distinct names, kinds, offsets and multiplicity. Static
        # function-typed labels do not imply independently bounded functions.
        rows.append({"name": item.name, "offset": item.value, "type": item.type, "storage": item.storage})
    return sorted(rows, key=lambda row: (row["offset"], row["name"], row["type"], row["storage"]))


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


def verify_code_region(obj, symbol, owner, mapping, image, *, objects=None):
    subject = select_code_region(obj, symbol.name)
    if subject != symbol:
        raise ValueError("linked code entry differs from its selected object symbol")
    if obj.sections[subject.section - 1].flags & 0x1000:
        return verify_function(obj, subject, owner, mapping, image, objects=objects)
    symbols = _ordinary_symbols(obj, subject)
    proof = _verify_region(obj, subject, owner, mapping, image, objects=objects)
    return {**proof, "extent_kind": "selected_code_section", "section_symbols": symbols,
            "other_sections": Result.UNKNOWN}
