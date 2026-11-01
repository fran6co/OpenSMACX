"""Explicit emission contexts survive the complete scoped measurement path."""

import json
import hashlib

import pytest

from match_recovery.application_binary import check_compiled, select_bindings, verify_records, verify_task
from match_recovery.bound_objects import FunctionBinding
from match_recovery.function_contexts import ContextFunctionTarget
from match_recovery.model import Origin, Result, Span
from match_recovery.pe_coff import Coff, PeImage
from test_application_commands import project
from test_binary import coff_fixture, pe_fixture
from test_function_contexts import coff_object
from test_tu_local_function_bindings import _call_object, _candidate
from match_recovery.partition import partition
from match_recovery.source import read_file
from match_recovery.selected_evidence import select_evidence
from test_function_fragments import associated_parent_coff


def configure_context(root):
    owner = root / "code/unit.cc"
    owner.write_text(owner.read_text().replace(
        "\nvoid", "\n// compile_context code/context.cc\nvoid"))
    (root / "code/context.cc").write_text('#include "unit.hpp"\n')
    (root / "build").mkdir()
    database = root / "build/commands.json"
    database.write_text(json.dumps([{
        "directory": str(root), "file": "code/context.cc",
        "arguments": ["compiler", "code/context.cc"]}]))
    path = root / "recovery/project.json"
    config = json.loads(path.read_text())
    config["mappings"] = {"compile_commands": "build/commands.json", "forwarder_macros": []}
    config["measurement"]["flag_sets"] = [["-trial"], ["-alternate"]]
    config["measurement"]["unit_flag_sets"] = {"code/context.cc": [1]}
    path.write_text(json.dumps(config))
    return database


def test_context_measurement_uses_declared_unit_and_its_profile(project):
    configure_context(project)
    calls = []

    def compile_unit(path, command, flags):
        calls.append((path.relative_to(project).as_posix(), tuple(flags)))
        return coff_fixture()

    report = verify_task(project, "owner", project / "original.exe",
                         compiler=compile_unit, command_for=lambda _: ())
    assert report["result"] == Result.PASS
    assert calls and set(calls) == {("code/context.cc", ("-alternate",))}
    assert report["translation_units"] == ["code/context.cc"]
    row = report["functions"][0]
    assert row["source_unit"] == "code/unit.cc"
    assert row["unit"] == row["compilation_context"] == "code/context.cc"
    checks = row["variants"][0]["context_target_checks"]
    assert checks[0]["result"] == Result.PASS
    assert checks[0]["linked_selection"] is False


@pytest.mark.parametrize("damage", ["missing_recipe", "missing_unit", "duplicate_recipe"])
def test_invalid_context_never_falls_back_to_marker_owner(project, damage):
    database = configure_context(project)
    if damage == "missing_recipe":
        database.write_text("[]")
    elif damage == "missing_unit":
        (project / "code/context.cc").unlink()
        config_path = project / "recovery/project.json"
        config = json.loads(config_path.read_text())
        del config["measurement"]["unit_flag_sets"]
        config_path.write_text(json.dumps(config))
    else:
        database.write_text(json.dumps(json.loads(database.read_text()) * 2))
    calls = []
    report = verify_task(project, "owner", project / "original.exe",
                         compiler=lambda *args: calls.append(args) or coff_fixture(),
                         command_for=lambda _: ())
    assert report["result"] == Result.UNKNOWN
    assert calls == []


def test_context_recipe_mutation_invalidates_measurement_epoch(project):
    database = configure_context(project)

    def compile_unit(path, command, flags):
        database.write_text("[]")
        return coff_fixture()

    with pytest.raises(ValueError, match="changed during measurement"):
        verify_task(project, "owner", project / "original.exe",
                    compiler=compile_unit, command_for=lambda _: ())


def test_context_placeholder_remains_unfinished_in_its_marker_owner(project):
    configure_context(project)
    owner = project / "code/unit.cc"
    owner.write_text(owner.read_text().replace("void f() {}", "void f() {\n// BODY GOES HERE.\n}"))
    report = verify_task(project, "owner", project / "original.exe",
                         compiler=lambda *args: coff_fixture(), command_for=lambda _: ())
    assert report["result"] != Result.PASS
    assert "placeholder" in str(report).lower()


@pytest.mark.parametrize("damage", [None, "missing", "static", "unsupported_relocation", "associative_child"])
def test_selected_context_cannot_reexport_an_unqualified_target_to_its_caller(damage):
    image_data = bytearray(pe_fixture())
    image_data[0x200:0x206] = b"\xe8\x1b\0\0\0\xc3"
    image_data[0x220] = 0xc3
    pin = hashlib.sha256(image_data).hexdigest()
    caller = FunctionBinding(Origin(pin, "pe:va", 0x401000),
                             (Span(0x401000, 0x401006),), "_caller", "code/caller.cc")
    origin = Origin(pin, "pe:va", 0x401020)
    context = FunctionBinding(origin, (Span(0x401020, 0x401021),), "_f",
                              "code/context.cc", source_unit="code/owner.cc")
    objects = {"code/caller.cc": _call_object("_caller", define_local=False),
               # An unrelated valid copy must never replace the declared one.
               "code/other.cc": Coff(coff_fixture())}
    if damage != "missing":
        objects[context.unit] = (
            Coff(associated_parent_coff()) if damage == "associative_child" else
            coff_object(storage=3) if damage == "static" else
            coff_object(references=(("_other", 2, 7),))
            if damage == "unsupported_relocation" else Coff(coff_fixture()))
    rows = check_compiled([caller, context], [], [], objects, PeImage(bytes(image_data), pin),
                          {"_f": ContextFunctionTarget(origin, "_f", context.unit, context.source_unit)})[0]
    by_symbol = {row["symbol"]: row for row in rows}
    assert by_symbol["_caller"]["result"] == (Result.PASS if damage is None else Result.UNKNOWN)
    if damage:
        assert by_symbol["_f"]["result"] != Result.PASS


def test_context_fragment_parent_never_publishes_a_target_even_when_unselected():
    pin = "0" * 64
    parent = _candidate(pin, 0x401000, "code/owner.cc", context_path="code/context.cc")
    child = _candidate(pin, 0x401001, "code/owner.cc", symbol="_tail", function_fragment={
        "parent_address": 0x401000, "parent_symbol": "_f",
        "reason": "synthetic adjacent fragment", "evidence": "control"})
    view = partition([parent, child])
    bindings, refused, targets = select_bindings(
        view, frozenset(), context_units=frozenset({"code/context.cc"}))
    assert "_f" not in targets
    assert "_tail" not in targets
    assert bindings == []


def test_explicit_record_check_uses_context_without_moving_the_marker(project):
    configure_context(project)
    owner = project / "code/unit.cc"
    before = owner.read_bytes()
    report = verify_records(project, read_file(owner), project / "original.exe",
                            flag_sets=(("-trial",),), compiler=lambda *args: coff_fixture(),
                            command_for=lambda _: ())
    assert report["result"] == Result.PASS
    assert report["functions"][0]["source_unit"] == "code/unit.cc"
    assert owner.read_bytes() == before


def test_selected_evidence_schedules_only_referenced_declared_contexts():
    origin = Origin("0" * 64, "pe:va", 0x401000)
    caller = FunctionBinding(origin, (Span(0x401000, 0x401006),), "_caller", "code/caller.cc")
    contexts = {
        name: ContextFunctionTarget(Origin(origin.image_sha256, "pe:va", address),
                                    name, unit, "code/owners.cc")
        for name, address, unit in [("_f", 0x401020, "code/context.cc"),
                                    ("_unused", 0x401040, "code/unrelated.cc")]}
    selection = select_evidence([caller], {origin.entity_id},
                                {caller.unit: _call_object("_caller", define_local=False)},
                                (), (), (), (), (), context_targets=contexts)
    assert selection.units == {"code/caller.cc", "code/context.cc"}
