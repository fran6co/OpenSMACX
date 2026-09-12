"""Compiler-local name controls use synthetic objects, never game fixtures."""

import copy
from dataclasses import replace

import pytest

from match_recovery.coff_preservation import compare_outside_functions
from match_recovery.coff_replica import function_signature, object_signature
from match_recovery.pe_coff import Coff, Symbol
from test_multispan import object_fixture


def fixture(path=r"Z:\project\code\unit.cpp", nonce="123"):
    obj = Coff(object_fixture(extra=(b".xdata$x", bytes(16), (), 1, 5, 0x40401040)))
    obj.symbols[2] = replace(obj.symbols[2], name="?helper@?%" + path + nonce + "@@YAHH@Z")
    obj.symbols[5] = replace(obj.symbols[5], name="$L" + nonce)
    obj.symbols[8] = replace(obj.symbols[8], name="$T" + nonce, storage=3)
    obj.symbols[100] = Symbol(100, ".file", 0, -2, 0, 103)
    raw = path.encode() + b"\0"
    obj.auxiliary[100] = raw + bytes(-len(raw) % 18)
    return obj


def pair():
    return fixture(), fixture(r"Z:\private\replica\code\unit.cpp", "9876")


def test_replica_path_nonce_and_associative_local_names_are_not_object_behavior():
    old, new = pair()
    assert object_signature(old) == object_signature(new)
    # This rule is deliberately not an alias in baseline/source preservation.
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"


def test_debug_header_file_records_do_not_replace_primary_tu_identity():
    old, new = pair()
    for obj in (old, new):
        obj.symbols[200] = Symbol(200, ".file", 0, -2, 0, 103)
        obj.auxiliary[200] = b"header.hpp\0"
        obj.symbols[201] = Symbol(201, ".file", 0, -2, 0, 103)
        obj.auxiliary[201] = obj.auxiliary[100]
    assert object_signature(old) == object_signature(new)


@pytest.mark.parametrize("damage", ["code", "data", "offset", "target", "type", "storage", "name", "signature", "basename", "file-record", "missing-file"])
def test_replica_normalization_never_waives_actual_contents_or_identity(damage):
    old, new = pair()
    if damage in ("code", "data"):
        index = 0 if damage == "code" else 2
        sections = list(new.sections)
        sections[index] = replace(sections[index], data=b"\x90" + sections[index].data[1:])
        new.sections = tuple(sections)
    elif damage == "offset":
        new.symbols[5] = replace(new.symbols[5], value=1)
    elif damage == "target":
        new.sections = (replace(new.sections[0], relocations=((1, 8, 20),)), *new.sections[1:])
    elif damage in ("type", "storage"):
        new.symbols[5] = replace(new.symbols[5], **{damage: 2})
    elif damage in ("name", "signature"):
        name = new.symbols[2].name.replace("helper", "other") if damage == "name" else new.symbols[2].name[:-1] + "X"
        new.symbols[2] = replace(new.symbols[2], name=name)
    elif damage == "basename":
        new = fixture(r"Z:\private\code\other.cpp", "9876")
    elif damage == "file-record":
        new.auxiliary[100] = b"not-the-symbol-path.cpp\0"
    else:
        del new.symbols[100]
    assert object_signature(old) != object_signature(new)


def test_undefined_unnamed_namespace_is_not_assumed_translation_unit_local():
    obj = fixture()
    obj.symbols[2] = replace(obj.symbols[2], section=0)
    with pytest.raises(ValueError, match="defined function or data"):
        object_signature(obj)


def test_private_data_keeps_storage_value_and_reference_identity():
    old, new = pair()
    for obj in (old, new):
        obj.symbols[2] = replace(obj.symbols[2], type=0, section=3,
                                name=obj.symbols[2].name.replace("@@YAHH@Z", "@@3HA"))
    assert object_signature(old) == object_signature(new)
    new.symbols[2] = replace(new.symbols[2], value=4)
    assert object_signature(old) != object_signature(new)


def test_two_namespace_nonces_cannot_be_collapsed_into_one_identity():
    obj = fixture()
    obj.symbols[101] = replace(obj.symbols[2], index=101, name=obj.symbols[2].name.replace("123", "456"))
    with pytest.raises(ValueError, match="conflicting unnamed-namespace"):
        object_signature(obj)


def test_distinct_alias_labels_at_one_offset_retain_exact_replica_identity():
    obj = fixture()
    obj.symbols[101] = replace(obj.symbols[5], index=101, name="$L456")
    same = copy.deepcopy(obj)
    assert object_signature(obj) == object_signature(same)
    assert function_signature(obj, obj.symbols[2].name) == function_signature(same, same.symbols[2].name)
    same.symbols[101] = replace(same.symbols[101], name="$L789")
    assert object_signature(obj) != object_signature(same)
    assert function_signature(obj, obj.symbols[2].name) == function_signature(same, same.symbols[2].name)


def test_static_numeric_name_in_independent_data_is_not_inferred_owned():
    old = fixture()
    del old.associations[3]
    new = copy.deepcopy(old)
    new.symbols[8] = replace(new.symbols[8], name="$T999")
    assert object_signature(old) != object_signature(new)


def local_pair():
    old = fixture()
    # A named function owns the same exception sections in both versions;
    # this is not permission to alias an unnamed-namespace definition.
    old.symbols[2] = replace(old.symbols[2], name="_owner")
    old.sections = (replace(old.sections[0], relocations=((1, 8, 6),)),
                    replace(old.sections[1], relocations=((0, 5, 6),)), old.sections[2])
    new = copy.deepcopy(old)
    new.symbols[5] = replace(new.symbols[5], name="$L9876")
    new.symbols[8] = replace(new.symbols[8], name="$T9876")
    return old, new


def test_preservation_uses_associative_owner_and_offset_for_numeric_local_labels():
    old, new = local_pair()
    report = compare_outside_functions(old, new, {}, {})
    assert report["result"] == "pass"
    assert report["changed_sections"] == report["symbol_changes"] == []


@pytest.mark.parametrize("damage", ["code", "data", "offset", "target", "type", "storage",
                                     "auxiliary", "owner", "external", "non_numeric", "independent",
                                     "relocation_site", "relocation_kind", "addend", "parent"])
def test_preserved_local_labels_do_not_waive_any_target_or_content_dimension(damage):
    old, new = local_pair()
    if damage in ("code", "data"):
        index = 0 if damage == "code" else 2
        sections = list(new.sections)
        sections[index] = replace(sections[index], data=b"\x90" + sections[index].data[1:])
        new.sections = tuple(sections)
    elif damage == "offset":
        new.symbols[8] = replace(new.symbols[8], value=4)
    elif damage == "target":
        new.sections = (replace(new.sections[0], relocations=((1, 5, 6),)), *new.sections[1:])
    elif damage in ("relocation_site", "relocation_kind"):
        relocation = (2, 8, 6) if damage == "relocation_site" else (1, 8, 20)
        new.sections = (replace(new.sections[0], relocations=(relocation,)), *new.sections[1:])
    elif damage == "addend":
        new.sections = (replace(new.sections[0], data=new.sections[0].data[:1] + b"\x04" +
                                new.sections[0].data[2:]), *new.sections[1:])
    elif damage == "parent":
        new.associations[3] = 2
    elif damage in ("type", "storage"):
        new.symbols[8] = replace(new.symbols[8], **{damage: 2})
    elif damage == "auxiliary":
        new.auxiliary[8] = bytes(18)
    elif damage == "owner":
        new.symbols[2] = replace(new.symbols[2], name="_different_owner")
    elif damage == "external":
        for obj in (old, new):
            obj.symbols[8] = replace(obj.symbols[8], storage=2)
    elif damage == "non_numeric":
        new.symbols[8] = replace(new.symbols[8], name="$Tsemantic_name")
    else:
        for obj in (old, new):
            del obj.associations[3]
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"


def test_preservation_accepts_distinct_alias_local_labels_only_with_exact_names():
    old, _ = local_pair()
    old.symbols[101] = replace(old.symbols[5], index=101, name="$L456")
    same = copy.deepcopy(old)
    assert compare_outside_functions(old, same, {}, {})["result"] == "pass"
    same.symbols[101] = replace(same.symbols[101], name="$L789")
    assert compare_outside_functions(old, same, {}, {})["result"] == "fail"


def test_preservation_never_normalizes_opaque_local_auxiliary_records():
    old, new = local_pair()
    for obj in (old, new):
        obj.auxiliary[8] = bytes(18)
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"


@pytest.mark.parametrize("operation", ["preservation", "object", "function"])
def test_raw_symbol_cannot_impersonate_a_normalized_local_in_another_object(operation):
    old, new = local_pair()
    new.symbols[5] = replace(new.symbols[5], name="<coff-associative-local>")
    # There need not be another recognized label or a same-object collision.
    new.symbols[8] = replace(new.symbols[8], name="opaque_static")
    with pytest.raises(ValueError, match="reserved compiler-local"):
        if operation == "object":
            object_signature(new)
        elif operation == "function":
            function_signature(new, "_owner")
        else:
            compare_outside_functions(old, new, {}, {})


def test_private_helper_name_churn_cannot_satisfy_selected_function_damage_control():
    old, new = pair()
    for obj in (old, new):
        # Point a cleanup relocation at its own defined anonymous helper.
        obj.sections = (obj.sections[0], replace(obj.sections[1], relocations=((0, 2, 20),)), obj.sections[2])
    assert function_signature(old, old.symbols[2].name) == function_signature(new, new.symbols[2].name)
    new.symbols[2] = replace(new.symbols[2], name=new.symbols[2].name.replace("helper", "other"))
    assert function_signature(old, old.symbols[2].name) != function_signature(new, new.symbols[2].name)


def colocated_labels(kind):
    obj, _ = local_pair()
    section, storage, prefix = {"function": (1, 6, "$L"), "associated_code": (2, 6, "$L"),
                                "associated_data": (3, 3, "$T")}[kind]
    obj.symbols[101] = Symbol(101, prefix + "27488", 1, section, 0, storage)
    obj.symbols[102] = Symbol(102, prefix + "27489", 1, section, 0, storage)
    obj.sections = (replace(obj.sections[0], relocations=((1, 101, 6),)), *obj.sections[1:])
    return obj


@pytest.mark.parametrize("kind", ["function", "associated_code", "associated_data"])
def test_colocated_numeric_labels_with_exact_names_and_edges_are_stable(kind):
    old = colocated_labels(kind)
    new = copy.deepcopy(old)
    assert object_signature(old) == object_signature(new)
    assert compare_outside_functions(old, new, {}, {})["result"] == "pass"
    assert function_signature(old, "_owner") == function_signature(new, "_owner")


@pytest.mark.parametrize("kind", ["function", "associated_code", "associated_data"])
@pytest.mark.parametrize("change", ["rename", "rename_both", "add", "remove", "move", "move_both", "relocation_target"])
def test_colocated_labels_never_merge_names_multiplicity_offsets_or_edges(kind, change):
    old = colocated_labels(kind)
    new = copy.deepcopy(old)
    if change in ("rename", "rename_both"):
        new.symbols[102] = replace(new.symbols[102], name=new.symbols[102].name + "0")
        if change == "rename_both":
            new.symbols[101] = replace(new.symbols[101], name=new.symbols[101].name + "0")
    elif change == "add":
        new.symbols[103] = replace(new.symbols[102], index=103, name=new.symbols[102].name + "0")
    elif change == "remove":
        del new.symbols[102]
    elif change in ("move", "move_both"):
        new.symbols[102] = replace(new.symbols[102], value=2)
        if change == "move_both":
            new.symbols[101] = replace(new.symbols[101], value=2)
    else:
        new.sections = (replace(new.sections[0], relocations=((1, 102, 6),)), *new.sections[1:])
    assert object_signature(old) != object_signature(new)
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"
    # Stronger whole-object identity must not let a local-label-only change
    # masquerade as the changed instruction body required by probe controls.
    assert function_signature(old, "_owner") == function_signature(new, "_owner")


@pytest.mark.parametrize("kind", ["function", "associated_code", "associated_data"])
def test_colocated_label_symbol_indices_can_change_only_with_matching_edges(kind):
    old = colocated_labels(kind)
    new = copy.deepcopy(old)
    indices = {101: 211, 102: 210}
    new.symbols = {indices.get(index, index): replace(symbol, index=indices.get(index, index))
                   for index, symbol in reversed(list(new.symbols.items()))}
    new.sections = tuple(replace(section, relocations=tuple((offset, indices.get(target, target), relocation)
                         for offset, target, relocation in section.relocations)) for section in new.sections)
    assert object_signature(old) == object_signature(new)
    assert compare_outside_functions(old, new, {}, {})["result"] == "pass"
    assert function_signature(old, "_owner") == function_signature(new, "_owner")
    new.sections = (replace(new.sections[0], relocations=((1, 210, 6),)), *new.sections[1:])
    assert object_signature(old) != object_signature(new)
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"


@pytest.mark.parametrize("kind", ["function", "associated_code", "associated_data"])
@pytest.mark.parametrize("operation", ["object", "preservation", "function"])
def test_duplicate_raw_numeric_labels_remain_ambiguous(kind, operation):
    obj = colocated_labels(kind)
    obj.symbols[103] = replace(obj.symbols[101], index=103)
    with pytest.raises(ValueError, match="ambiguous compiler-local"):
        if operation == "object":
            object_signature(obj)
        elif operation == "preservation":
            compare_outside_functions(obj, obj, {}, {})
        else:
            function_signature(obj, "_owner")
