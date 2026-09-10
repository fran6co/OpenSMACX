"""Numeric code labels retain function ownership and every observable edge."""

import copy
from dataclasses import replace
import struct

import pytest

from match_recovery.coff_preservation import compare_outside_functions
from match_recovery.coff_replica import object_signature, function_signature
from match_recovery.pe_coff import Coff, Symbol
from test_multispan import object_fixture


def pair():
    old = Coff(object_fixture(extra=(b".data", bytes(8), (), 0, 2, 0xC0501040)))
    old.symbols[100] = Symbol(100, "$L123", 1, 1, 0, 6)
    old.symbols[101] = Symbol(101, "$L124", 5, 1, 0, 6)
    # Self, associative child, and independent incoming edges all use the same
    # actual root-local identity. No edge is dropped by name normalization.
    old.sections = (replace(old.sections[0], relocations=((1, 100, 20),)),
                    replace(old.sections[1], relocations=((0, 100, 6),)),
                    replace(old.sections[2], relocations=((0, 100, 6),)))
    new = copy.deepcopy(old)
    new.symbols[100] = replace(new.symbols[100], name="$L987")
    new.symbols[101] = replace(new.symbols[101], name="$L988")
    return old, new


def test_root_numeric_names_use_exact_owned_locations_in_all_incoming_edges():
    old, new = pair()
    assert object_signature(old) == object_signature(new)
    assert function_signature(old, "_f") == function_signature(new, "_f")
    report = compare_outside_functions(old, new, {}, {})
    assert report["result"] == "pass"
    assert report["changed_sections"] == report["symbol_changes"] == []


@pytest.mark.parametrize("damage", ["offset", "owner", "target", "site", "kind", "addend",
                                     "bytes", "other_symbol", "extent", "flags", "association", "comdat_policy"])
def test_normalized_root_labels_never_hide_other_evidence(damage):
    old, new = pair()
    if damage == "offset":
        new.symbols[100] = replace(new.symbols[100], value=2)
    elif damage == "owner":
        new.symbols[2] = replace(new.symbols[2], name="_other_function")
    elif damage == "other_symbol":
        for obj in (old, new):
            obj.symbols[102] = Symbol(102, "meaningful_label", 3, 1, 0, 6)
        new.symbols[102] = replace(new.symbols[102], name="different_label")
    elif damage in ("target", "site", "kind"):
        relocation = {"target": (0, 101, 6), "site": (1, 100, 6), "kind": (0, 100, 20)}[damage]
        new.sections = (*new.sections[:2], replace(new.sections[2], relocations=(relocation,)))
    elif damage in ("bytes", "addend"):
        index = 0 if damage == "bytes" else 2
        sections = list(new.sections)
        sections[index] = replace(sections[index], data=b"\x01" + sections[index].data[1:])
        new.sections = tuple(sections)
    elif damage == "extent":
        new.section_sizes = (new.section_sizes[0] + 4, *new.section_sizes[1:])
    elif damage == "flags":
        new.sections = (replace(new.sections[0], flags=new.sections[0].flags ^ 0x80000000), *new.sections[1:])
    elif damage == "comdat_policy":
        raw = bytearray(new.auxiliary[0])
        struct.pack_into("B", raw, 14, 1)
        new.auxiliary[0] = bytes(raw)
    else:
        new.associations[2] = 3
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"
    assert object_signature(old) != object_signature(new)


def test_root_numeric_static_names_do_not_gain_function_label_identity():
    old, new = pair()
    for obj in (old, new):
        obj.symbols[100] = replace(obj.symbols[100], name=obj.symbols[100].name.replace("$L", "$T"), storage=3)
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"
    assert object_signature(old) != object_signature(new)


def test_duplicate_owner_name_in_another_comdat_does_not_establish_root_identity():
    old, new = pair()
    for obj in (old, new):
        obj.sections = (*obj.sections[:2], replace(obj.sections[2], flags=0x60501020))
        obj.symbols[8] = replace(obj.symbols[8], name="_f", type=0x20, storage=2)
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"
    assert object_signature(old) != object_signature(new)


@pytest.mark.parametrize("case", ["static_label", "external_label", "typed_label", "auxiliary",
                                   "non_numeric", "static_owner", "missing_owner", "multiple_owners",
                                   "nonzero_owner", "not_comdat", "not_code", "independent_data"])
def test_unproven_root_label_ownership_keeps_numeric_spellings_strict(case):
    old, new = pair()
    for obj in (old, new):
        if case in ("static_label", "external_label", "typed_label", "auxiliary", "non_numeric", "independent_data"):
            if case == "static_label":
                obj.symbols[100] = replace(obj.symbols[100], storage=3)
            elif case == "external_label":
                obj.symbols[100] = replace(obj.symbols[100], storage=2)
            elif case == "typed_label":
                obj.symbols[100] = replace(obj.symbols[100], type=1)
            elif case == "auxiliary":
                obj.auxiliary[100] = bytes(18)
            elif case == "non_numeric":
                obj.symbols[100] = replace(obj.symbols[100], name=obj.symbols[100].name + "_meaningful")
            else:
                obj.symbols[100] = replace(obj.symbols[100], section=3)
        elif case == "static_owner":
            obj.symbols[2] = replace(obj.symbols[2], storage=3)
        elif case == "missing_owner":
            del obj.symbols[2]
        elif case == "multiple_owners":
            obj.symbols[102] = Symbol(102, "_alias", 0, 1, 0x20, 2)
        elif case == "nonzero_owner":
            obj.symbols[2] = replace(obj.symbols[2], value=1)
        else:
            flag = 0x1000 if case == "not_comdat" else 0x20
            obj.sections = (replace(obj.sections[0], flags=obj.sections[0].flags & ~flag), *obj.sections[1:])
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"
    assert object_signature(old) != object_signature(new)


@pytest.mark.parametrize("operation", ["preservation", "object", "function"])
@pytest.mark.parametrize("damage", ["collision", "impersonation"])
def test_root_local_aliases_and_reserved_marker_impersonation_refuse(operation, damage):
    old, new = pair()
    if damage == "collision":
        new.symbols[101] = replace(new.symbols[101], value=new.symbols[100].value)
    else:
        new.symbols[100] = replace(new.symbols[100], name="<coff-function-local>")
    with pytest.raises(ValueError, match="ambiguous compiler-local|reserved compiler-local"):
        if operation == "preservation":
            compare_outside_functions(old, new, {}, {})
        elif operation == "object":
            object_signature(new)
        else:
            function_signature(new, "_f")
