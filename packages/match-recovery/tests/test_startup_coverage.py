"""Native trial/capability composition with controlled linked observations."""

from copy import copy, deepcopy
from dataclasses import replace
import json
import pickle
import struct

import pytest

from match_recovery import preservation_objects as native
from match_recovery.coff_replica import object_signature
from match_recovery.pe_coff import Coff
from match_recovery.startup_coverage import StartupCoverage, consume_startup
from test_linked_initializers import initializer_object
from test_preservation_application_objects import graph, trial, FLAGS, INPUT, UNIT
from test_build_graph_commands import stamp


def renumber(raw):
    """Different actual section/symbol indices, equal native object replica."""
    data = bytearray(raw)
    count, = struct.unpack_from("<H", data, 2)
    at, symbols = struct.unpack_from("<II", data, 8)
    assert count == 2
    data[20:60], data[60:100] = data[60:100], data[20:60]
    for number in range(count):
        reloc, = struct.unpack_from("<I", data, 20 + number * 40 + 24)
        amount, = struct.unpack_from("<H", data, 20 + number * 40 + 32)
        for index in range(amount):
            target, = struct.unpack_from("<I", data, reloc + 10 * index + 4)
            struct.pack_into("<I", data, reloc + 10 * index + 4, target + 2)
    for index in range(symbols):
        section, = struct.unpack_from("<h", data, at + 18 * index + 12)
        if section > 0:
            struct.pack_into("<h", data, at + 18 * index + 12, 3 - section)
    struct.pack_into("<I", data, 12, symbols + 2)
    file = b".file\0\0\0" + struct.pack("<IhHBB", 0, -2, 0, 103, 1) + b"replica.cpp\0".ljust(18, b"\0")
    return bytes(data[:at]) + file + bytes(data[at:])


@pytest.fixture
def inputs(graph, monkeypatch):
    changes = [{"entity_id": "entity-" + "a" * 64, "unit": UNIT, "version": "current",
                "symbol": "_$E1", "offset": 0, "evidence": ["source-owned startup interpretation"]}]
    authorization = graph.authorization
    authorization["permission"] = {"migration_id": "startup-owner", "author": "coordinator",
                                   "evidence": ["bounded source-owned startup change"], "allowed": [],
                                   "generated_changes": deepcopy(changes)}
    graph = replace(graph, _authorization=authorization)
    raw = initializer_object(b"\xc3")
    row = graph.current.proof["compilations"][0]
    output = graph.current.directory / row["output"]
    output.write_bytes(raw)
    row["compilation"]["output"]["after"] = stamp(raw)
    observation = {"result": "pass", "code_dependencies": {"result": "pass"}, "entries": [
        {"input": {"path": str(output)}, "object_sha256": stamp(raw)["sha256"],
         "section": 2, "offset": 0, "target": 0x401000,
         "function": {"owner": output.name, "symbol": "_$E1"}}]}
    calls = []
    def observe(context):
        calls.append(context)
        context.check_current()
        return deepcopy(observation)
    monkeypatch.setattr(native, "observe_build_initializers", observe)
    return graph, changes, raw, observation, calls


def cover(run, graph, changes, raw, obj=None, **kwargs):
    observed = trial(graph, raw=raw)
    obj = Coff(raw) if obj is None else obj
    arguments = dict(unit=UNIT, version="current", flags=FLAGS, input_fingerprint=INPUT, changes=changes)
    arguments.update(kwargs)
    return run.cover_startup(observed, obj, **arguments), obj


@pytest.mark.parametrize("renumbered", [False, True])
def test_real_trial_is_bound_to_its_own_structural_indices(inputs, renumbered):
    graph, changes, raw, observation, calls = inputs
    actual = renumber(raw) if renumbered else raw
    assert object_signature(Coff(actual)) == object_signature(Coff(raw))
    with native.application_objects(graph) as run:
        capability, obj = cover(run, graph, changes, actual)
        slots = consume_startup(capability, obj)
        assert slots == ((1 if renumbered else 2, 0, 2 if renumbered else 0),)
        assert type(capability) is StartupCoverage
        assert calls == [graph.current]
    with pytest.raises(ValueError, match="expired"):
        consume_startup(capability, obj)


@pytest.mark.parametrize("value", [{"result": "pass", "slots": [[2, 0, 0]]}, None, (), "pass"])
def test_serialized_or_forged_reports_cannot_be_consumed(value):
    with pytest.raises(ValueError, match="authentic"):
        consume_startup(value, Coff(initializer_object(b"\xc3")))


def test_constructor_and_forged_instance_do_not_grant_coverage():
    with pytest.raises(TypeError, match="native"):
        StartupCoverage()
    with pytest.raises(ValueError, match="authentic"):
        consume_startup(object.__new__(StartupCoverage), Coff(initializer_object(b"\xc3")))


def test_capability_cannot_be_serialized_or_copied(inputs):
    graph, changes, raw, _, _ = inputs
    with native.application_objects(graph) as run:
        capability, obj = cover(run, graph, changes, raw)
        for operation in (pickle.dumps, json.dumps, copy, deepcopy):
            with pytest.raises(TypeError):
                operation(capability)
        assert consume_startup(capability, obj)


@pytest.mark.parametrize("damage", ["equal-other", "bytes", "symbol", "extra-attribute"])
def test_capability_requires_the_exact_unchanged_mutable_instance(inputs, damage):
    graph, changes, raw, _, _ = inputs
    with native.application_objects(graph) as run:
        capability, obj = cover(run, graph, changes, raw)
        if damage == "equal-other":
            obj = Coff(raw)
        elif damage == "bytes":
            obj.sections = (replace(obj.sections[0], data=b"\x90" + obj.sections[0].data[1:]), *obj.sections[1:])
        elif damage == "symbol":
            obj.symbols[0] = replace(obj.symbols[0], name="other")
        else:
            obj.unobserved = True
        with pytest.raises(ValueError, match="another or changed"):
            consume_startup(capability, obj)


@pytest.mark.parametrize("field,value", [("unit", "src/b.cpp"), ("version", "baseline"),
                                       ("symbol", "other"), ("offset", 4), ("entity_id", "other"),
                                       ("evidence", ["different rationale"])])
def test_permission_membership_is_exact_not_a_unit_or_symbol_allowlist(inputs, field, value):
    graph, changes, raw, _, calls = inputs
    changes = [{**changes[0], field: value}]
    with native.application_objects(graph) as run:
        with pytest.raises(ValueError, match="permission"):
            cover(run, graph, changes, raw)
    assert not calls


@pytest.mark.parametrize("damage", ["empty", "duplicate", "oversized", "extra-field", "missing-field", "bool-offset"])
def test_malformed_changes_refuse(inputs, damage):
    graph, changes, raw, _, calls = inputs
    if damage == "empty": changes = []
    elif damage == "duplicate": changes *= 2
    elif damage == "oversized": changes *= 4097
    elif damage == "extra-field": changes = [{**changes[0], "result": "pass"}]
    elif damage == "missing-field": changes = [{key: value for key, value in changes[0].items() if key != "evidence"}]
    else: changes = [{**changes[0], "offset": False}]
    with native.application_objects(graph) as run:
        with pytest.raises(ValueError):
            cover(run, graph, changes, raw)
    assert not calls


def test_other_flag_trial_cannot_inherit_application_correspondence(inputs):
    graph, changes, raw, _, calls = inputs
    different = initializer_object(b"\x90\xc3")
    with native.application_objects(graph) as run:
        with pytest.raises(ValueError, match="whole application-object replica"):
            cover(run, graph, changes, different)
        observed = trial(graph, raw=raw)
        with pytest.raises(ValueError, match="identity or authorization"):
            run.cover_startup(observed, Coff(raw), unit=UNIT, version="current", flags=(*FLAGS, "/Oy-"),
                              input_fingerprint=INPUT, changes=changes)
    assert not calls


@pytest.mark.parametrize("damage", ["table-fail", "table-unknown", "code-fail", "code-unknown", "code-missing", "path", "archive",
                                     "duplicate", "offset", "symbol", "object-sha", "section"])
def test_missing_inconsistent_or_unselected_link_evidence_refuses(inputs, damage):
    graph, changes, raw, observation, calls = inputs
    entry = observation["entries"][0]
    if damage == "table-fail": observation["result"] = "fail"
    elif damage == "table-unknown": observation["result"] = "unknown"
    elif damage == "code-fail": observation["code_dependencies"]["result"] = "fail"
    elif damage == "code-unknown": observation["code_dependencies"]["result"] = "unknown"
    elif damage == "code-missing": del observation["code_dependencies"]
    elif damage == "path": entry["input"]["path"] += ".other"
    elif damage == "archive": entry["input"]["member"] = "unit.obj"
    elif damage == "duplicate": observation["entries"].append(deepcopy(entry))
    elif damage == "offset": entry["offset"] = 4
    elif damage == "symbol": entry["function"]["symbol"] = "other"
    elif damage == "object-sha": entry["object_sha256"] = "0" * 64
    else: entry["section"] = 1
    with native.application_objects(graph) as run:
        with pytest.raises(ValueError):
            cover(run, graph, changes, raw)
    assert calls == [graph.current]


def test_link_observation_cannot_change_captured_object_receipt(inputs, monkeypatch):
    graph, changes, raw, observation, _ = inputs
    output = graph.current.directory / graph.current.proof["compilations"][0]["output"]
    def damage(context):
        output.write_bytes(initializer_object(b"\x90\xc3"))
        return deepcopy(observation)
    monkeypatch.setattr(native, "observe_build_initializers", damage)
    with native.application_objects(graph) as run:
        with pytest.raises(ValueError, match="fresh compiler receipt"):
            cover(run, graph, changes, raw)


def test_live_authorization_change_invalidates_existing_coverage(inputs):
    graph, changes, raw, _, _ = inputs
    with native.application_objects(graph) as run:
        capability, obj = cover(run, graph, changes, raw)
        graph._authorization["permission"]["generated_changes"] = []
        with pytest.raises(ValueError, match="authorization changed"):
            consume_startup(capability, obj)


def test_live_graph_failure_refuses_consumption_and_final_exit(inputs):
    graph, changes, raw, _, _ = inputs
    with pytest.raises(ValueError, match="context changed"):
        with native.application_objects(graph) as run:
            capability, obj = cover(run, graph, changes, raw)
            graph.current.fail_at = graph.current.checks + 1
            with pytest.raises(ValueError, match="context changed"):
                consume_startup(capability, obj)


def test_native_observation_is_cached_only_inside_live_application_epoch(inputs):
    graph, changes, raw, _, calls = inputs
    for epoch in range(2):
        with native.application_objects(graph) as run:
            for _ in range(3):
                capability, obj = cover(run, graph, changes, raw)
                assert consume_startup(capability, obj)
        assert calls == [graph.current] * (epoch + 1)


def test_permission_membership_does_not_equate_json_boolean_and_number(inputs):
    graph, changes, raw, _, calls = inputs
    graph._authorization["permission"]["generated_changes"][0]["evidence"] = [True]
    changes[0]["evidence"] = [1]
    with native.application_objects(graph) as run:
        with pytest.raises(ValueError, match="permission"):
            cover(run, graph, changes, raw)
    assert not calls


@pytest.mark.parametrize("damage", [None, "unowned-root", "directives", "dependency", "data"])
def test_live_producer_to_native_residual_only_covers_authorized_startup(inputs, damage):
    from match_recovery.coff_preservation import compare_outside_functions
    from test_startup_projection import object_with

    graph, changes, _, observation, calls = inputs
    old_raw = object_with(("old",), as_bytes=True)
    raw = object_with(metadata=b"changed" if damage == "directives" else b"/defaultlib:keep",
                      calls={"new": "dependency"} if damage == "dependency" else None, as_bytes=True)
    if damage == "unowned-root":
        altered = bytearray(raw)
        at, = struct.unpack_from("<I", altered, 20 + 20)
        altered[at] = 0x90
        raw = bytes(altered)
    if damage == "data":
        def data_section(contents, change=False):
            altered = bytearray(contents)
            count, = struct.unpack_from("<H", altered, 2)
            header = 20 + 40 * (count - 1)
            altered[header:header + 8] = b".data\0\0\0"
            struct.pack_into("<I", altered, header + 36, 0xC0300040)
            if change:
                at, = struct.unpack_from("<I", altered, header + 20)
                altered[at] ^= 1
            return bytes(altered)
        old_raw, raw = data_section(old_raw), data_section(raw, change=True)
    old = Coff(old_raw)
    changes[0].update(symbol="new", offset=4)
    graph._authorization["permission"]["generated_changes"] = deepcopy(changes)
    row = graph.current.proof["compilations"][0]
    output = graph.current.directory / row["output"]
    output.write_bytes(raw)
    row["compilation"]["output"]["after"] = stamp(raw)
    observation["entries"] = [{"input": {"path": str(output)}, "object_sha256": stamp(raw)["sha256"],
                               "section": 3, "offset": 4, "target": 0x401000,
                               "function": {"owner": output.name, "symbol": "new"}}]
    with native.application_objects(graph) as run:
        capability, obj = cover(run, graph, changes, raw)
        before = compare_outside_functions(old, obj, {}, {})
        assert before["result"] == "fail"
        report = compare_outside_functions(old, obj, {}, {}, current_startup=capability)
        assert report["result"] == ("pass" if damage is None else "fail"), report
    with pytest.raises(ValueError, match="expired"):
        compare_outside_functions(old, obj, {}, {}, current_startup=capability)


@pytest.mark.parametrize("side", ["baseline", "current"])
def test_explicit_outside_source_binding_cannot_be_hidden_by_startup_permission(inputs, side):
    from match_recovery.coff_preservation import compare_outside_functions

    graph, changes, raw, _, _ = inputs
    with native.application_objects(graph) as run:
        capability, obj = cover(run, graph, changes, raw)
        other = Coff(raw)
        options = {side + "_startup": capability, side + "_outside": frozenset({"_$E1"})}
        baseline, current = (obj, other) if side == "baseline" else (other, obj)
        with pytest.raises(ValueError, match="protected outside source binding"):
            compare_outside_functions(baseline, current, {}, {}, **options)


def test_new_source_binding_cannot_claim_preexisting_generated_root(inputs):
    from match_recovery.coff_preservation import compare_outside_functions

    graph, changes, raw, _, _ = inputs
    with native.application_objects(graph) as run:
        capability, obj = cover(run, graph, changes, raw)
        with pytest.raises(ValueError, match="protected outside source binding"):
            compare_outside_functions(Coff(raw), obj, {}, {"_$E1": "entity-" + "a" * 64},
                                      current_startup=capability)
