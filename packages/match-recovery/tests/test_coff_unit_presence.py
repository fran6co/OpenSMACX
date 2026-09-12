"""Present-object residuals need no fabricated absent COFF object.

These portable comparisons establish neither source/recipe absence nor replica
equivalence or acceptance. The caller must independently verify those facts.
"""

import copy
from dataclasses import replace
import struct

import pytest

from match_recovery.coff_preservation import compare_unit_presence
from match_recovery.model import Origin
from match_recovery.pe_coff import Coff, Symbol
from test_binary import coff_fixture
from test_function_witness import object_bytes
from test_multispan import object_fixture


ENTITY = Origin("a" * 64, "pe:va", 0x401000).entity_id
SCOPE = {"_f": ENTITY}


def scoped_only():
    obj = Coff(coff_fixture(b"\xc3"))
    # The general fixture contains an unused external _g. Removing it here is
    # explicit test input construction, not a waiver in production comparison.
    del obj.symbols[1]
    return obj


@pytest.mark.parametrize("present", ["baseline", "current"])
def test_scoped_only_present_object_has_no_residual_but_is_not_acceptance(present):
    obj = scoped_only()
    original = copy.deepcopy(obj)
    report = compare_unit_presence(obj, SCOPE, present=present)
    assert report["result"] == "pass"
    assert report["baseline_fingerprint"] == report["current_fingerprint"]
    assert report["changed_sections"] == report["symbol_changes"] == []
    assert report["residual_sections"] == {"baseline": 0, "current": 0}
    assert report["unit_presence"] == {"baseline": present == "baseline", "current": present == "current"}
    assert report[present + "_scoped_functions"] == {ENTITY: {"symbol": "_f", "owned_sections": 1}}
    other = "current" if present == "baseline" else "baseline"
    assert report[other + "_scoped_functions"] == {}
    assert report["newly_bound_protected_functions"] == {}
    assert "ready_for_collection" not in report and "acceptance" not in report
    assert obj.symbols == original.symbols and obj.sections == original.sections


@pytest.mark.parametrize("present", ["baseline", "current"])
def test_unmapped_function_is_real_residual_code(present):
    report = compare_unit_presence(scoped_only(), {}, present=present)
    assert report["result"] == "fail"
    section, = report["changed_sections"]
    assert section["change"] == ("removed" if present == "baseline" else "added")
    assert [symbol["symbol"] for symbol in section[present + "_symbols"]] == ["_f"]
    assert report["baseline_fingerprint"] != report["current_fingerprint"]


@pytest.mark.parametrize("present", ["baseline", "current"])
def test_standalone_unmapped_helper_cannot_borrow_scoped_function_ownership(present):
    obj = Coff(object_bytes([("_f", b"\xc3", ()), ("_helper", b"\x90\xc3", ())]))
    report = compare_unit_presence(obj, SCOPE, present=present)
    assert report["result"] == "fail" and report["residual_sections"][present] == 1
    assert any(row["symbol"] == "_helper" for row in report["symbol_changes"])
    assert not any(row["symbol"] == "scoped-entry:" + ENTITY for row in report["symbol_changes"])


@pytest.mark.parametrize("present", ["baseline", "current"])
@pytest.mark.parametrize("kind", ["data", "bss", "directive", "empty_section", "types"])
def test_unowned_data_types_and_linker_metadata_are_not_compiler_scaffolding(present, kind):
    name, contents, flags = {
        "data": (b".data", b"sensitive-data", 0xC0501040),
        "bss": (b".bss", bytes(16), 0xC0501080),
        "directive": (b".drectve", b"/DEFAULTLIB:required", 0x40501040),
        "empty_section": (b".data", b"", 0xC0501040),
        "types": (b".debug$T", struct.pack("<I", 2), 0x40501040),
    }[kind]
    obj = Coff(object_fixture(extra=(name, contents, (), 0, 2, flags)))
    if kind == "bss":
        obj.sections = (*obj.sections[:2], replace(obj.sections[2], data=b""))
    report = compare_unit_presence(obj, SCOPE, present=present)
    assert report["result"] == "fail"
    section, = report["changed_sections"]
    assert section["name"] == name.decode()
    assert report["residual_sections"][present] == 1
    assert report["baseline_fingerprint"] != report["current_fingerprint"]
    if kind == "data":
        assert contents.hex() not in repr(report)


@pytest.mark.parametrize("present", ["baseline", "current"])
@pytest.mark.parametrize("kind", ["unused_external", "common", "weak", "public_alias"])
def test_linker_visible_symbols_cannot_disappear_with_scoped_code(present, kind):
    obj = scoped_only()
    if kind == "public_alias":
        obj.symbols[1] = Symbol(1, "_public_alias", 0, 1, 0, 2)
        expected = {"_public_alias"}
    else:
        obj.symbols[1] = Symbol(1, "_requirement", 32 if kind == "common" else 0, 0, 0, 2)
        expected = {"_requirement"}
        if kind == "weak":
            obj.symbols[2] = Symbol(2, "_weak", 0, 0, 0, 105)
            obj.auxiliary[2] = struct.pack("<II10x", 1, 2)
            expected.add("_weak")
    report = compare_unit_presence(obj, SCOPE, present=present)
    assert report["result"] == "fail" and report["symbols_changed"]
    assert {row["symbol"] for row in report["symbol_changes"]} == expected
    assert {row["change"] for row in report["symbol_changes"]} == {"removed" if present == "baseline" else "added"}


@pytest.mark.parametrize("present", ["baseline", "current"])
def test_owned_associative_sections_can_be_subtracted_but_outside_binding_refuses(present):
    obj = Coff(object_fixture())
    report = compare_unit_presence(obj, SCOPE, present=present)
    assert report["result"] == "pass"
    assert report[present + "_scoped_functions"][ENTITY]["owned_sections"] == 2
    with pytest.raises(ValueError, match="outside source binding"):
        compare_unit_presence(obj, SCOPE, present=present, outside=frozenset({"$tail2"}))


@pytest.mark.parametrize("present", ["baseline", "current"])
def test_protected_previously_unbound_function_cannot_be_hidden_by_new_annotation(present):
    report = compare_unit_presence(scoped_only(), SCOPE, present=present, protected=frozenset({"_f"}))
    assert report["result"] == "fail"
    assert report["newly_bound_protected_functions"] == SCOPE
    assert report[present + "_scoped_functions"] == {}
    assert report["residual_sections"][present] == 1
    assert any(row["symbol"] == "_f" for row in report["symbol_changes"])


@pytest.mark.parametrize("present", ["baseline", "current"])
def test_duplicate_unreferenced_symbol_multiplicity_is_retained(present):
    obj = scoped_only()
    obj.symbols[1] = Symbol(1, "_requirement", 0, 0, 0, 2)
    obj.symbols[2] = replace(obj.symbols[1], index=2)
    report = compare_unit_presence(obj, SCOPE, present=present)
    assert report["result"] == "fail"
    row, = report["symbol_changes"]
    assert row["symbol"] == "_requirement" and row["count"] == 2
    del obj.symbols[2]
    single = compare_unit_presence(obj, SCOPE, present=present)
    assert single[present + "_fingerprint"] != report[present + "_fingerprint"]
    assert single["symbol_changes"][0]["count"] == 1


@pytest.mark.parametrize("present", ["baseline", "current"])
def test_residual_relocation_targets_and_multiplicity_are_exact(present):
    obj = Coff(object_fixture(extra=(b".data", bytes(8), (), 0, 2, 0xC0501040)))
    obj.symbols[100] = Symbol(100, "_first", 0, 0, 0, 2)
    obj.symbols[101] = Symbol(101, "_second", 0, 0, 0, 2)
    obj.sections = (*obj.sections[:2], replace(obj.sections[2], relocations=((0, 100, 6), (0, 100, 6), (4, 101, 6))))
    report = compare_unit_presence(obj, SCOPE, present=present)
    assert report["result"] == "fail"
    direction = "references_removed" if present == "baseline" else "references_added"
    references = report["changed_sections"][0][direction]
    assert [(row["offset"], row["kind"], row["target"]["symbol"], row["count"]) for row in references] == [
        (0, 6, "_first", 2), (4, 6, "_second", 1)]
    obj.sections = (*obj.sections[:2], replace(obj.sections[2], relocations=((0, 101, 6), (0, 101, 6), (4, 101, 6))))
    changed = compare_unit_presence(obj, SCOPE, present=present)
    assert changed[present + "_fingerprint"] != report[present + "_fingerprint"]
    assert changed["changed_sections"][0][direction][0]["target"]["symbol"] == "_second"


@pytest.mark.parametrize("present", [None, "", "both", "absent", True, 1, [], {}])
def test_invalid_present_side_refuses_before_comparison(present):
    with pytest.raises(ValueError, match="baseline or current"):
        compare_unit_presence(scoped_only(), SCOPE, present=present)
