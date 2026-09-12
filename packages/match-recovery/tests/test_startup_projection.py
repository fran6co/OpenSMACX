"""Exact synthetic COFF projection controls, not authorization/link proof."""

from copy import deepcopy
from dataclasses import replace
import struct

import pytest

from match_recovery.coff_replica import _object_evidence
from match_recovery.pe_coff import Coff, Section, Symbol
from match_recovery.startup_projection import project_startup


def object_with(roots=("old", "new"), *, slots=None, calls=None, metadata=b"/defaultlib:keep", labels=True,
                as_bytes=False):
    slots = roots if slots is None else slots
    calls = calls or {}
    specs = [(".text", b"\xe8" + bytes(4) + b"\xc3" if name in calls else b"\xc3",
              ((1, calls[name], 20),) if name in calls else (), 0x60501020) for name in roots]
    specs += [(".CRT$XCU", bytes(4 * len(slots)), tuple((n * 4, name, 6) for n, name in enumerate(slots)), 0xC0300040),
              (".drectve", metadata, (), 0x00100A00)]
    table = len(roots) + 1
    symbols = [(name, 0, n, 0x20, 3, b"") for n, name in enumerate(roots, 1)]
    symbols += [(".CRT$XCU", 0, table, 0, 3, struct.pack("<IHHIHB3x", len(slots) * 4, len(slots), 0, 0, 0, 0))]
    if labels:
        symbols += [("cell_" + name, n * 4, table, 0, 3, b"") for n, name in enumerate(slots)]
    for name in sorted(set(calls.values()) - set(roots)):
        symbols.append((name, 0, 0, 0x20, 2, b""))
    indices, strings, records = {}, bytearray(), bytearray()
    for name, value, number, kind, storage, auxiliary in symbols:
        indices[name] = len(records) // 18
        encoded = name.encode()
        if len(encoded) > 8:
            name_bytes = struct.pack("<II", 0, len(strings) + 4)
            strings.extend(encoded + b"\0")
        else:
            name_bytes = encoded.ljust(8, b"\0")
        records.extend(name_bytes + struct.pack("<IhHBB", value, number, kind, storage, len(auxiliary) // 18) + auxiliary)
    start = 20 + 40 * len(specs)
    headers, data = [], bytearray()
    for name, contents, relocations, flags in specs:
        offset = start + len(data)
        data.extend(contents)
        relocation_at = start + len(data) if relocations else 0
        for at, target, kind in relocations:
            data.extend(struct.pack("<IIH", at, indices[target], kind))
        headers.append(name.encode().ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(contents), offset,
                                                                  relocation_at, 0, len(relocations), 0, flags))
    raw = (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(data), len(records) // 18, 0, 0)
           + b"".join(headers) + data + records + struct.pack("<I", len(strings) + 4) + strings)
    return raw if as_bytes else Coff(raw)


def slot(obj, name, occurrence=0):
    number = next(n for n, section in enumerate(obj.sections, 1) if section.name == ".CRT$XCU")
    index = obj.function(name).index
    offsets = [offset for offset, target, kind in obj.sections[number - 1].relocations if target == index]
    return number, offsets[occurrence], index


def project(obj, *names):
    return project_startup(obj, tuple(slot(obj, name) for name in names))


@pytest.mark.parametrize("roots", [("new", "old"), ("old", "new")])
def test_partial_add_remove_preserves_remaining_order_and_metadata(roots):
    baseline = object_with(("old",))
    candidate = object_with(roots)
    before = deepcopy(candidate.__dict__)
    value, coverage = project(candidate, "new")
    assert value == _object_evidence(baseline)
    assert candidate.__dict__ == before
    assert coverage["remaining_slots"] == 1 and not coverage["table_removed"]
    assert coverage["removed_root_sections"] == [candidate.function("new").section]
    assert project(candidate, "new")[0] == value  # Same actual object, no mutable consumption.


def test_fully_covered_contribution_disappears_without_dropping_directives():
    obj = object_with(("new",))
    value, coverage = project(obj, "new")
    assert coverage["table_removed"]
    assert [row[1] for row in value["sections"]] == [".drectve"]
    assert value["sections"][0][4] == ("raw", b"/defaultlib:keep".hex())
    assert value["symbols"] == []


def test_no_slots_is_exact_ordinary_residual():
    obj = object_with()
    value, coverage = project_startup(obj, ())
    assert value == _object_evidence(obj) and not coverage["slots"]


def test_unowned_order_and_bytes_remain_visible():
    left = project(object_with(("old", "new", "other")), "new")[0]
    right = project(object_with(("other", "new", "old")), "new")[0]
    assert left != right
    assert project(object_with(metadata=b"changed"), "new")[0] != project(object_with(), "new")[0]


def test_covered_root_retains_undefined_callback_requirement():
    obj = object_with(calls={"new": "callback"})
    value, coverage = project(obj, "new")
    assert any(row[0][0] == "callback" and row[0][2] == "0" for row in value["symbols"])
    assert coverage["preserved_reference_symbols"]
    assert value != _object_evidence(object_with(("old",)))


@pytest.mark.parametrize("kind", ["helper", "guard", "types", "data"])
def test_other_sections_and_references_are_not_consumed(kind):
    obj = object_with(calls={"new": "old"})
    extra = Section(b"keep" if kind != "guard" else b"", (), 0xC0300080 if kind == "guard" else 0xC0300040,
                    {"helper": ".more", "guard": ".bss", "types": ".debug$T", "data": ".data"}[kind])
    obj.sections += (extra,)
    obj.section_sizes += (8 if kind == "guard" else 4,)
    value, _ = project(obj, "new")
    assert any(row[1] == extra.name and row[3] == obj.section_sizes[-1] for row in value["sections"])
    assert any(row[0][0] == "old" for row in value["symbols"])


@pytest.mark.parametrize("shared", ["table", "code", "data", "weak"])
def test_shared_generated_root_is_retained(shared):
    obj = object_with(("old", "new"), slots=("old", "new", "new") if shared == "table" else None,
                      labels=shared != "table", calls={"old": "new"} if shared == "code" else None)
    if shared == "data":
        obj.sections += (Section(bytes(4), ((0, obj.function("new").index, 6),), 0xC0300040, ".data"),)
        obj.section_sizes += (4,)
    if shared == "weak":
        index = max(obj.symbols) + 1
        obj.symbols[index] = Symbol(index, "alias", 0, 0, 0, 105)
        obj.auxiliary[index] = struct.pack("<II10x", obj.function("new").index, 3)
    value, coverage = project(obj, "new")
    assert coverage["retained_root_sections"] == [obj.function("new").section]
    assert any(row[0][0] == "new" for row in value["symbols"])


def test_associative_child_is_not_automatically_covered():
    obj = object_with()
    number = len(obj.sections) + 1
    obj.sections += (Section(b"metadata", (), 0xC0301040, ".xdata"),)
    obj.section_sizes += (8,)
    obj.associations[number] = obj.function("new").section
    value, coverage = project(obj, "new")
    assert any(row[1] == ".xdata" for row in value["sections"])
    assert value["associations"] and number not in coverage["removed_root_sections"]


@pytest.mark.parametrize("bad", [None, {}, [1], [(True, 0, 0)], [(2, -4, 0)], [(2, 1, 0)],
                                 [(999, 0, 0)], [(3, 4, 999)], [(3, 4, 0)], [(3, 400, 1)]])
def test_invalid_or_forged_slot_refuses(bad):
    with pytest.raises(ValueError):
        project_startup(object_with(), bad)


def test_duplicate_and_oversized_slots_refuse():
    obj = object_with()
    selected = slot(obj, "new")
    for slots in ((selected, selected), (selected,) * 4097):
        with pytest.raises(ValueError):
            project_startup(obj, slots)


@pytest.mark.parametrize("damage", ["addend", "kind", "missing", "duplicate", "alignment", "comdat", "external-root",
                                     "external-cell", "cell-aux", "section-aux", "cell-reference", "second-table"])
def test_unsupported_table_or_symbols_refuse(damage):
    obj = object_with()
    chosen = slot(obj, "new")
    number, offset, root = chosen
    section = obj.sections[number - 1]
    changed = section
    if damage == "addend":
        changed = replace(section, data=b"\x01" + section.data[1:])
    elif damage == "kind":
        changed = replace(section, relocations=((0, 0, 20), section.relocations[1]))
    elif damage == "missing":
        changed = replace(section, relocations=section.relocations[:1])
    elif damage == "duplicate":
        changed = replace(section, relocations=section.relocations + section.relocations[:1])
    elif damage == "alignment":
        changed = replace(section, flags=section.flags ^ 0x100000)
    elif damage == "comdat":
        changed = replace(section, flags=section.flags | 0x1000)
    elif damage == "external-root":
        obj.symbols[root] = replace(obj.symbols[root], storage=2)
    elif damage in ("external-cell", "cell-aux", "cell-reference"):
        cell = next(symbol for symbol in obj.symbols.values() if symbol.name == "cell_new")
        if damage == "external-cell":
            obj.symbols[cell.index] = replace(cell, storage=2)
        elif damage == "cell-aux":
            obj.auxiliary[cell.index] = bytes(18)
        else:
            obj.sections += (Section(bytes(4), ((0, cell.index, 6),), 0xC0300040, ".data"),)
            obj.section_sizes += (4,)
    elif damage == "section-aux":
        definition = next(symbol for symbol in obj.symbols.values() if symbol.name == ".CRT$XCU")
        obj.auxiliary[definition.index] = obj.auxiliary[definition.index][:12] + b"\1" + bytes(5)
    elif damage == "second-table":
        obj.sections += (section,)
        obj.section_sizes += (len(section.data),)
    obj.sections = tuple(changed if n == number else item for n, item in enumerate(obj.sections, 1))
    with pytest.raises(ValueError):
        project_startup(obj, (chosen,))


def test_mapped_function_scope_cannot_overlap_generated_coverage():
    obj = object_with()
    for excluded in ({slot(obj, "new")[0]}, {obj.function("new").section}):
        with pytest.raises(ValueError, match="storage|unscoped"):
            project_startup(obj, (slot(obj, "new"),), excluded=frozenset(excluded))


def test_debug_line_reference_does_not_create_a_runtime_owner():
    obj = object_with()
    obj.sections += (Section(bytes(4), ((0, obj.function("new").index, 6),), 0x42300040, ".debug$S"),)
    obj.section_sizes += (4,)
    assert project(obj, "new")[1]["removed_root_sections"] == [obj.function("new").section]


def test_partial_table_section_auxiliary_metadata_is_retained():
    old, new = object_with(("old",)), object_with()
    for obj in (old, new):
        definition = next(symbol for symbol in obj.symbols.values() if symbol.name == ".CRT$XCU")
        raw = obj.auxiliary[definition.index]
        obj.auxiliary[definition.index] = raw[:6] + struct.pack("<HI", 2, 123) + raw[12:]
    assert project(new, "new")[0] == _object_evidence(old)
    definition = next(symbol for symbol in new.symbols.values() if symbol.name == ".CRT$XCU")
    raw = new.auxiliary[definition.index]
    new.auxiliary[definition.index] = raw[:6] + struct.pack("<H", 3) + raw[8:]
    assert project(new, "new")[0] != _object_evidence(old)


def test_root_with_opaque_function_metadata_is_retained():
    obj = object_with()
    index = obj.function("new").index
    obj.auxiliary[index] = bytes(18)
    value, coverage = project(obj, "new")
    assert coverage["retained_root_sections"] == [obj.function("new").section]
    assert any(row[0][0] == "new" for row in value["symbols"])
