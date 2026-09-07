"""Outside preservation must not inherit relocation-masking blind spots."""

import copy
from dataclasses import replace
import struct

import pytest

from match_recovery.coff_preservation import compare_outside_functions
from match_recovery.model import Origin
from match_recovery.pe_coff import Coff, Symbol
from test_binary import coff_fixture
from test_multispan import object_fixture


ENTITY = Origin("a" * 64, "pe:va", 0x401000).entity_id
SCOPE = {"_f": ENTITY}


def test_scoped_body_may_regress_without_requiring_original_correctness():
    old = Coff(coff_fixture(b"\x90\xc3"))
    new = Coff(coff_fixture(b"\xeb\xfe"))
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"
    report = compare_outside_functions(old, new, SCOPE, SCOPE)
    assert report["result"] == "pass" and report["baseline_fingerprint"] == report["current_fingerprint"]


def test_new_callee_used_only_by_scoped_code_is_not_an_outside_definition():
    old = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))
    new = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)).replace(b"_g", b"_h"))
    assert compare_outside_functions(old, new, SCOPE, SCOPE)["result"] == "pass"
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"


def test_local_relocation_targets_are_not_collapsed_to_a_placeholder():
    old = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))
    old.symbols[1] = replace(old.symbols[1], name="$local", section=1, value=1, storage=3)
    new = copy.deepcopy(old)
    new.symbols[1] = replace(new.symbols[1], value=4)
    assert compare_outside_functions(old, new, {}, {})["result"] == "fail"


def test_owned_associative_code_can_change_but_independent_data_cannot():
    extra = (b".data", bytes(4), (), 0, 2, 0xC0501040)
    old = Coff(object_fixture(extra=extra))
    new = Coff(object_fixture(tail=b"\x90\xc3", extra=extra))
    assert compare_outside_functions(old, new, SCOPE, SCOPE)["result"] == "pass"
    new.sections = (*new.sections[:2], replace(new.sections[2], data=b"\x01" + new.sections[2].data[1:]))
    assert compare_outside_functions(old, new, SCOPE, SCOPE)["result"] == "fail"


@pytest.mark.parametrize("damage", ["common", "weak", "alias", "directive", "bss"])
def test_linkage_data_and_public_aliases_cannot_disappear_with_scoped_code(damage):
    old = Coff(coff_fixture())
    if damage == "common":
        old.symbols[1] = replace(old.symbols[1], value=16)
    elif damage == "weak":
        old.symbols[2] = Symbol(2, "_weak", 0, 0, 0, 105)
        old.auxiliary[2] = struct.pack("<II10x", 1, 2)
    elif damage == "alias":
        old.symbols[2] = Symbol(2, "_exported_alias", 0, 1, 0, 2)
    else:
        extra = (b".drectve" if damage == "directive" else b".bss", b"linker-policy", (), 0, 2, 0xC0501040)
        old = Coff(object_fixture(extra=extra))
    new = copy.deepcopy(old)
    if damage == "common": new.symbols[1] = replace(new.symbols[1], value=32)
    elif damage in ("weak", "alias"):
        del new.symbols[2]
        new.auxiliary.pop(2, None)
    elif damage == "bss": new.section_sizes = (*new.section_sizes[:2], new.section_sizes[2] + 4)
    else: new.sections = (*new.sections[:2], replace(new.sections[2], data=b"changed"))
    assert compare_outside_functions(old, new, SCOPE, SCOPE)["result"] == "fail"


def test_scope_does_not_authorize_another_mapped_associative_body():
    obj = Coff(object_fixture())
    with pytest.raises(ValueError, match="outside source binding"):
        compare_outside_functions(obj, obj, SCOPE, SCOPE, baseline_outside={"$tail2"})


def test_ambiguous_function_ownership_is_refused():
    obj = Coff(coff_fixture())
    obj.symbols[2] = Symbol(2, "_peer", 0, 1, 0x20, 2)
    with pytest.raises(ValueError, match="exclusively"):
        compare_outside_functions(obj, obj, SCOPE, SCOPE)


def test_scoped_entry_rename_uses_original_identity_not_old_spelling():
    old, new = Coff(coff_fixture()), Coff(coff_fixture().replace(b"_f", b"_h"))
    assert compare_outside_functions(old, new, SCOPE, {"_h": ENTITY})["result"] == "pass"


def test_no_object_bytes_are_exposed_in_report():
    old, new = Coff(coff_fixture(b"\x11\x22\x33\x44")), Coff(coff_fixture(b"\x55\x66\x77\x88"))
    report = compare_outside_functions(old, new, {}, {})
    assert report["changed_sections"] and "11223344" not in repr(report) and "55667788" not in repr(report)
