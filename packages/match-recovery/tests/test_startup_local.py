"""Private startup observations refuse address escape; no coverage is granted."""

from copy import deepcopy
from dataclasses import replace
import struct

import pytest

from match_recovery.pe_coff import Section, Symbol
from match_recovery.startup_local import slot_local_root
from test_startup_projection import object_with, slot


def observe(obj, name="new"):
    table, offset, index = slot(obj, name)
    return slot_local_root(obj, table=table, offset=offset, symbol=obj.symbols[index])


def add_symbol(obj, name, section, *, value=0, storage=3, kind=0, auxiliary=b""):
    index = max(obj.symbols) + 1 + (len(obj.auxiliary.get(max(obj.symbols), b"")) // 18)
    symbol = Symbol(index, name, value, section, kind, storage)
    obj.symbols[index] = symbol
    if auxiliary:
        obj.auxiliary[index] = auxiliary
    return symbol


def change_section(obj, number, **changes):
    obj.sections = tuple(replace(section, **changes) if index == number else section
                         for index, section in enumerate(obj.sections, 1))


def incoming(obj, target, *, name=".data", flags=0xC0300040):
    obj.sections += (Section(bytes(4), ((0, target, 6),), flags, name),)
    obj.section_sizes += (4,)


def fpo(obj):
    root = obj.function("new")
    number = len(obj.sections) + 1
    obj.sections += (Section(struct.pack("<IIII", 0, len(obj.sections[root.section - 1].data), 0, 0),
                             ((0, root.index, 7),), 0x42101048, ".debug$F"),)
    obj.section_sizes += (16,)
    obj.associations[number] = root.section
    add_symbol(obj, ".debug$F", number,
               auxiliary=struct.pack("<IHHIHB3x", 16, 1, 0, 0, root.section, 5))
    return number


def test_private_slot_observes_without_mutation_or_name_heuristics():
    obj = object_with(("other", "arbitrary_private_spelling"), calls={"arbitrary_private_spelling": "callback"})
    before = deepcopy(obj.__dict__)
    result = observe(obj, "arbitrary_private_spelling")
    assert result["result"] == "pass"
    assert result["offset"] == 4 and result["root_size"] == 6
    assert result["outgoing_references"] == [{"offset": 1, "target": next(
        item.index for item in obj.symbols.values() if item.name == "callback"), "kind": 20, "addend": 0}]
    assert not result["recognized_debug_children"] and not result["requires_linked_debug_proof"]
    assert obj.__dict__ == before


def test_local_aliases_are_reported_with_multiplicity_not_discarded():
    obj = object_with()
    add_symbol(obj, "$local", obj.function("new").section, storage=6)
    add_symbol(obj, "$other", obj.function("new").section)
    assert [row["name"] for row in observe(obj)["root_symbols"]] == ["new", "$local", "$other"]


@pytest.mark.parametrize("target", ["root", "alias", "section", "cell", "other_cell"])
@pytest.mark.parametrize("origin", ["data", "debug_name", "discardable"])
def test_unselected_incoming_references_refuse_even_debug_names(target, origin):
    obj = object_with()
    root = obj.function("new")
    targets = {"root": root.index,
               "alias": add_symbol(obj, "alias", root.section).index}
    targets.update({"section": next(s.index for s in obj.symbols.values() if s.name == ".CRT$XCU"),
                    "cell": next(s.index for s in obj.symbols.values() if s.name == "cell_new"),
                    "other_cell": next(s.index for s in obj.symbols.values() if s.name == "cell_old")})
    incoming(obj, targets[target], name=".debug$S" if origin == "debug_name" else ".data",
             flags=0x42100040 if origin == "discardable" else 0xC0300040)
    with pytest.raises(ValueError, match="escapes|incoming"):
        observe(obj)


@pytest.mark.parametrize("target", ["new", "cell_new", "cell_old", ".CRT$XCU", "root_alias"])
def test_weak_alias_edges_are_incoming_references(target):
    obj = object_with()
    add_symbol(obj, "root_alias", obj.function("new").section)
    index = next(s.index for s in obj.symbols.values() if s.name == target)
    add_symbol(obj, "weak", 0, storage=105, auxiliary=struct.pack("<II10x", index, 3))
    with pytest.raises(ValueError, match="weak alias"):
        observe(obj)


def test_repeated_slot_refuses():
    with pytest.raises(ValueError, match="incoming"):
        observe(object_with(slots=("old", "new", "new"), labels=False))


@pytest.mark.parametrize("kind", [6, 20])
def test_root_self_address_refuses_even_relative_call(kind):
    obj = object_with(calls={"new": "new"})
    root = obj.function("new")
    change_section(obj, root.section, relocations=((1, root.index, kind),))
    with pytest.raises(ValueError, match="self-address"):
        observe(obj)


@pytest.mark.parametrize("damage", ["kind", "bounds", "overlap", "missing_symbol", "short_raw", "discardable"])
def test_root_extent_and_outgoing_operands_must_remain_bounded(damage):
    obj = object_with(calls={"new": "callback"})
    root = obj.function("new")
    section = obj.sections[root.section - 1]
    at, target, kind = section.relocations[0]
    if damage == "kind":
        change_section(obj, root.section, relocations=((at, target, 7),))
    elif damage == "bounds":
        change_section(obj, root.section, relocations=((4, target, kind),))
    elif damage == "overlap":
        change_section(obj, root.section, relocations=((at, target, kind), (at + 1, target, kind)))
    elif damage == "missing_symbol":
        change_section(obj, root.section, relocations=((at, 999999, kind),))
    elif damage == "short_raw":
        change_section(obj, root.section, data=section.data[:-1])
    else:
        change_section(obj, root.section, flags=section.flags | 0x02000000)
    with pytest.raises(ValueError):
        observe(obj)


def test_auxiliary_function_reference_to_selected_nonzero_index_refuses():
    obj = object_with()
    old, new = obj.function("old"), obj.function("new")
    obj.auxiliary[old.index] = struct.pack("<IIIIH", 0, 1, 0, new.index, 0)
    with pytest.raises(ValueError, match="auxiliary incoming"):
        observe(obj)


@pytest.mark.parametrize("target", ["new", "cell_new", "cell_old"])
@pytest.mark.parametrize("damage", ["external", "auxiliary", "out_of_bounds"])
def test_exposed_or_opaque_symbols_refuse(target, damage):
    obj = object_with()
    symbol = next(s for s in obj.symbols.values() if s.name == target)
    if damage == "external":
        obj.symbols[symbol.index] = replace(symbol, storage=2)
    elif damage == "auxiliary":
        obj.auxiliary[symbol.index] = bytes(18)
    else:
        obj.symbols[symbol.index] = replace(symbol, value=999)
    with pytest.raises(ValueError):
        observe(obj)


@pytest.mark.parametrize("damage", ["missing", "duplicate", "kind", "addend", "alignment", "second_table",
                                     "short_raw", "comdat", "removed"])
def test_incomplete_or_ambiguous_table_refuses(damage):
    obj = object_with()
    table, offset, index = slot(obj, "new")
    section = obj.sections[table - 1]
    if damage in ("missing", "duplicate", "kind"):
        rows = section.relocations[:-1] if damage == "missing" else section.relocations + section.relocations[:1]
        if damage == "kind":
            rows = ((0, section.relocations[0][1], 20), *section.relocations[1:])
        change_section(obj, table, relocations=rows)
    elif damage == "addend":
        change_section(obj, table, data=b"\x01" + section.data[1:])
    elif damage == "short_raw":
        change_section(obj, table, data=section.data[:4])
    elif damage == "second_table":
        obj.sections += (section,)
        obj.section_sizes += (len(section.data),)
    else:
        change_section(obj, table, flags=section.flags ^ {"alignment": 0x100000, "comdat": 0x1000,
                                                        "removed": 0x800}[damage])
    with pytest.raises(ValueError):
        slot_local_root(obj, table=table, offset=offset, symbol=obj.symbols[index])


def test_strict_fpo_observation_retains_explicit_final_link_obligation():
    obj = object_with(calls={"new": "callback"})
    child = fpo(obj)
    before = deepcopy(obj.__dict__)
    result = observe(obj)
    assert result["requires_linked_debug_proof"] is True
    assert result["recognized_debug_children"][0]["section"] == child
    assert result["recognized_debug_children"][0]["data"] == obj.sections[child - 1].data.hex()
    assert result["outgoing_references"] and obj.__dict__ == before


@pytest.mark.parametrize("damage", ["name", "flags", "addend", "size", "frame", "relocation", "extra_relocation",
                                     "extra_symbol", "external_symbol", "incoming", "weak", "descendant",
                                     "auxiliary", "raw_extent", "runtime_child"])
def test_fpo_metadata_is_not_a_blanket_debug_exception(damage):
    obj = object_with()
    child = fpo(obj)
    section = obj.sections[child - 1]
    debug_symbol = next(s for s in obj.symbols.values() if s.section == child)
    if damage == "name":
        change_section(obj, child, name=".debug$S")
    elif damage == "flags":
        change_section(obj, child, flags=section.flags | 0x20000000)
    elif damage in ("addend", "size", "frame"):
        at = {"addend": 0, "size": 4, "frame": 8}[damage]
        data = bytearray(section.data)
        data[at] ^= 1
        change_section(obj, child, data=bytes(data))
    elif damage == "relocation":
        change_section(obj, child, relocations=((0, obj.function("old").index, 7),))
    elif damage == "extra_relocation":
        change_section(obj, child, relocations=section.relocations + ((4, obj.function("old").index, 7),))
    elif damage in ("extra_symbol", "external_symbol"):
        add_symbol(obj, "exposed", child, storage=2 if damage == "external_symbol" else 3)
    elif damage == "incoming":
        incoming(obj, debug_symbol.index)
    elif damage == "weak":
        add_symbol(obj, "weak", 0, storage=105, auxiliary=struct.pack("<II10x", debug_symbol.index, 3))
    elif damage == "descendant":
        number = len(obj.sections) + 1
        obj.sections += (Section(b"data", (), 0xC0301040, ".xdata"),)
        obj.section_sizes += (4,)
        obj.associations[number] = child
    elif damage == "runtime_child":
        change_section(obj, child, name=".xdata", flags=0xC0301040)
    elif damage == "auxiliary":
        obj.auxiliary[debug_symbol.index] += bytes(18)
    else:
        obj.section_sizes = obj.section_sizes[:-1] + (32,)
    with pytest.raises(ValueError):
        observe(obj)


@pytest.mark.parametrize("damage", ["wrong_root", "wrong_slot", "boolean_table", "forged_symbol"])
def test_forged_selection_refuses(damage):
    obj = object_with()
    table, offset, index = slot(obj, "new")
    symbol = obj.symbols[index]
    if damage == "wrong_root":
        symbol = obj.function("old")
    elif damage == "wrong_slot":
        offset = 0
    elif damage == "boolean_table":
        table = True
    else:
        symbol = replace(symbol, name="invented")
    with pytest.raises(ValueError):
        slot_local_root(obj, table=table, offset=offset, symbol=symbol)
