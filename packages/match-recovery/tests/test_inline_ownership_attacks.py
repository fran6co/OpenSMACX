"""Adversarial ownership controls, using portable synthetic compiler objects."""

import copy
from dataclasses import replace
import json
import struct
from types import SimpleNamespace

import pytest

from match_recovery.inline_ownership import definition_signature, observe_copies
from match_recovery.model import Origin
from match_recovery.pe_coff import Coff, Symbol
from match_recovery.preservation import check_preservation
from match_recovery.source_config import SourceConfiguration
from test_binary import coff_fixture
from test_inline_preservation import InlineCompiler, inline_project, object_bytes
from test_multispan import object_fixture
from test_preservation import project


ENTITY = Origin("a" * 64, "pe:va", 0x401000).entity_id
OTHER = Origin("a" * 64, "pe:va", 0x401010).entity_id
HEADER = "code/model.h"
ROW = {"unit": "code/owner.cc", "symbol": "_f", "body": HEADER}
SETTINGS = SimpleNamespace(unit_extensions=(".cc",), source=SourceConfiguration(
    image_pin="original/PIN", address_space="pe:va", roots=("code",)))


def observe(canonical, candidate, *, bindings=None, files=None, editable=None, damaged=None):
    probe = Coff(coff_fixture(b"\xeb\xfe"))
    return observe_copies(
        candidate, "code/caller.cc", bindings or {ENTITY: [ROW]}, {ENTITY},
        {HEADER} if editable is None else editable,
        {HEADER: b"inline int _f() { return 1; }"} if files is None else files,
        SETTINGS, lambda unit: canonical,
        damaged or (lambda unit, header, contents: probe))


def assert_refused(canonical, candidate, **kwargs):
    allowed, reports = observe(canonical, candidate, **kwargs)
    assert allowed == {}
    assert reports and all(row["result"] == "unknown" for row in reports)


def test_same_name_owned_by_an_outside_binding_is_not_attributed():
    obj = Coff(coff_fixture())
    assert_refused(obj, obj, bindings={ENTITY: [ROW], OTHER: [{**ROW, "unit": "code/else.cc"}]})


def test_duplicate_canonical_annotations_are_not_scope():
    obj = Coff(coff_fixture())
    assert_refused(obj, obj, bindings={ENTITY: [ROW, {**ROW, "unit": "code/else.cc"}]})


@pytest.mark.parametrize("damage", ["unauthorized", "absent", "source-unit", "overload"])
def test_unproven_body_sources_cannot_grant_ownership(damage):
    obj = Coff(coff_fixture())
    kwargs = {}
    if damage == "unauthorized":
        kwargs["editable"] = set()
    elif damage == "absent":
        kwargs["files"] = {}
    elif damage == "source-unit":
        kwargs.update(bindings={ENTITY: [{**ROW, "body": "code/other.cc"}]},
                      editable={"code/other.cc"}, files={"code/other.cc": b"int _f() {}"})
    else:
        kwargs["files"] = {HEADER: b"int _f() {} int _f(int n) {}"}
    assert_refused(obj, obj, **kwargs)


def test_same_bytes_with_different_external_reference_are_not_equal():
    raw = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20))
    canonical, candidate = Coff(raw), Coff(raw.replace(b"_g", b"_h"))
    assert definition_signature(canonical, "_f") != definition_signature(candidate, "_f")
    assert_refused(canonical, candidate)


def test_same_local_name_with_different_target_offset_is_not_equal():
    canonical = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))
    canonical.symbols[1] = replace(canonical.symbols[1], name="$local", section=1, value=1, storage=3)
    candidate = copy.deepcopy(canonical)
    candidate.symbols[1] = replace(candidate.symbols[1], value=4)
    assert definition_signature(canonical, "_f") != definition_signature(candidate, "_f")
    assert_refused(canonical, candidate)


def test_associative_child_contents_are_part_of_definition():
    canonical, candidate = Coff(object_fixture()), Coff(object_fixture(tail=b"\x90\xc3"))
    assert definition_signature(canonical, "_f") != definition_signature(candidate, "_f")
    assert_refused(canonical, candidate)


def test_comdat_selection_is_part_of_definition():
    canonical = Coff(object_bytes([("_f", b"\xc3")]))
    candidate = copy.deepcopy(canonical)
    index = next(i for i, symbol in candidate.symbols.items()
                 if symbol.name == ".text" and symbol.storage == 3)
    data = bytearray(candidate.auxiliary[index])
    data[14] = 3
    candidate.auxiliary[index] = bytes(data)
    assert definition_signature(canonical, "_f") != definition_signature(candidate, "_f")
    assert_refused(canonical, candidate)


def test_public_alias_does_not_disappear_from_owned_definition():
    canonical = Coff(coff_fixture())
    candidate = copy.deepcopy(canonical)
    candidate.symbols[20] = Symbol(20, "_alias", 0, 1, 0, 2)
    assert definition_signature(canonical, "_f") != definition_signature(candidate, "_f")
    assert_refused(canonical, candidate)


def test_same_named_static_function_is_not_an_external_inline_copy():
    canonical = Coff(coff_fixture())
    candidate = copy.deepcopy(canonical)
    symbol = candidate.function("_f")
    candidate.symbols[symbol.index] = replace(symbol, storage=3)
    assert_refused(canonical, candidate)


def test_same_named_local_dependency_in_another_tu_cannot_supply_identity():
    canonical = Coff(object_fixture(extra=(b".data", b"AAAA", (), 0, 2, 0xC0501040)))
    canonical.sections = (replace(canonical.sections[0], relocations=((1, 8, 20),)), *canonical.sections[1:])
    canonical.symbols[8] = replace(canonical.symbols[8], name="_local", storage=3)
    candidate = copy.deepcopy(canonical)
    candidate.sections = (*candidate.sections[:2], replace(candidate.sections[2], data=b"BBBB" + candidate.sections[2].data[4:]))
    # Matching local names, offsets and section keys do not prove that two
    # independently compiled static objects have the same storage identity.
    assert_refused(canonical, candidate)


@pytest.mark.parametrize("name,section,value", [
    ("_absolute", -1, 0x12345678),
    ("?g@?%model.h42@@YAHXZ", 0, 0),
    ("?g@?A0x12345678@@YAHXZ", 0, 0),
    ("?s@?1??f@@YAHXZ@4HA", 0, 0),
])
def test_absolute_or_compiler_local_external_names_are_not_bound_dependencies(name, section, value):
    obj = Coff(coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))
    obj.symbols[1] = replace(obj.symbols[1], name=name, section=section, value=value, storage=2)
    assert_refused(obj, copy.deepcopy(obj))


@pytest.mark.parametrize("damage", ["different-bodies", "missing-symbol", "unchanged-canonical", "unchanged-copy"])
def test_damage_must_change_both_target_definitions_equivalently(damage):
    obj = Coff(coff_fixture())
    def damaged(unit, header, contents):
        if damage == "missing-symbol":
            return Coff(coff_fixture().replace(b"_f", b"_x"))
        if damage == "unchanged-canonical" and unit == "code/owner.cc":
            return obj
        if damage == "unchanged-copy" and unit == "code/caller.cc":
            return obj
        code = b"\xcc" if damage == "different-bodies" and unit == "code/caller.cc" else b"\xeb\xfe"
        return Coff(coff_fixture(code))
    assert_refused(obj, obj, damaged=damaged)


def test_new_header_binding_cannot_waive_a_previously_unbound_copy(project):
    owner = project / "code/owner.cc"
    owner.write_text(owner.read_text().replace('#include "model.h"', '// body code/model.h\n#include "model.h"'))
    (project / HEADER).write_text("inline int _f() { return 2; }\n")
    report = check_preservation(project, "owner", compiler=InlineCompiler(project))
    caller = next(row for row in report["units"] if row["unit"] == "code/caller.cc")
    assert report["result"] == "fail"
    assert caller["newly_bound_protected_functions"] == {"_f": ENTITY}
    assert caller["current_scoped_functions"] == {}


def test_body_probe_cache_does_not_cross_source_versions(inline_project):
    compiler = InlineCompiler(inline_project)
    report = check_preservation(inline_project, "inline-owner", compiler=compiler)
    assert report["result"] == "pass"
    probes = [(unit, flags) for unit, replacements, flags in compiler.calls
              if b"for (;;)" in replacements.get(HEADER, b"")]
    # Baseline/current probes have identical replacement bytes; each version
    # still needs its own canonical and dependent-TU compilations.
    assert probes.count(("code/owner.cc", ("optimize",))) == 2
    assert probes.count(("code/caller.cc", ("optimize",))) == 2


def test_passing_first_flag_cannot_hide_failed_second_flag_damage(inline_project):
    config = inline_project / "recovery/project.json"
    value = json.loads(config.read_text())
    value["measurement"]["flag_sets"].append(["other-flags"])
    config.write_text(json.dumps(value))
    class Compiler(InlineCompiler):
        def output(self, source, replacements, flags):
            raw = super().output(source, replacements, flags)
            if flags == ("other-flags",) and b"for (;;)" in replacements.get(HEADER, b""):
                number = 2
                functions = [("_f", b"\xb8" + struct.pack("<I", number) + b"\xc3")]
                if source.name == "caller.cc":
                    functions.append(("_h", b"\xb8\x01\0\0\0\xc3"))
                return object_bytes(functions)
            return raw
    report = check_preservation(inline_project, "inline-owner", compiler=Compiler(inline_project))
    caller = [row for row in report["units"] if row["unit"] == "code/caller.cc"]
    assert report["result"] == "unknown"
    assert caller[0]["result"] == "pass"
    assert caller[1]["result"] == "unknown"
    assert not caller[1]["current_scoped_functions"]
