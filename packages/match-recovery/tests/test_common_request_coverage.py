"""Native two-build common redundancy; no saved passing proof fixtures."""

from copy import copy, deepcopy
from dataclasses import replace
import json
import pickle
import struct

import pytest

from match_recovery.build_graph import ApplicationGraph
from match_recovery.coff_preservation import compare_outside_functions, compare_unit_presence
from match_recovery.common_preservation import _compare_linked
from match_recovery.common_request_coverage import DuplicateCommonRequestCoverage
from match_recovery.linked_context import linked_build
from match_recovery.linked_initializers import observe_build_initializers
from match_recovery.model import digest
from match_recovery.pe_coff import Coff
from match_recovery.preservation_objects import ObjectTrial, application_objects
from match_recovery.source_replica import snapshot_digest
from test_link_file_evidence import stamp
from test_linked_initializers import update_artifact
from test_startup_linked_consumption import context as startup_context, UNIT, FLAGS


INPUT = "b" * 64


def append_section(raw, *, data=b"\x7f", references=()):
    obj = Coff(raw)
    specifications = [(s.name.encode(), s.data, s.relocations, s.flags) for s in obj.sections]
    specifications.append((b".data", data, references, 0xC0300040))
    start = 20 + 40 * len(specifications)
    headers, payload = [], bytearray()
    for name, body, relocations, flags in specifications:
        at = start + len(payload)
        payload.extend(body)
        reloc_at = start + len(payload) if relocations else 0
        payload.extend(b"".join(struct.pack("<IIH", *row) for row in relocations))
        headers.append(struct.pack("<8sIIIIIIHHI", name, 0, 0, len(body), at, reloc_at, 0,
                                   len(relocations), 0, flags))
    symbols, count = struct.unpack_from("<II", raw, 8)
    header = struct.pack("<HHIIIHH", 0x14C, len(specifications), 0, start + len(payload), count, 0, 0)
    return header + b"".join(headers) + payload + raw[symbols:]


def common_context(root, *, request=False, witness=True, request_size=4, witness_size=4,
                   retained_reference=False, extra_data=False, damage=None):
    """Real startup and selected witness objects, before native receipt pins.

    dependency.cpp is an ordinary independently selected compiled TU. It is not
    a generated-root owner unless a test explicitly expands the graph domain.
    """
    def configure(context):
        changed = {}
        if request:
            path = context.objects[0]
            raw = bytearray(path.read_bytes())
            symbol_at, = struct.unpack_from("<I", raw, 8)
            struct.pack_into("<I", raw, symbol_at + 2 * 18 + 8, request_size)
            struct.pack_into("<H", raw, symbol_at + 2 * 18 + 14, 0)
            reloc_at, = struct.unpack_from("<I", raw, 20 + 24)
            struct.pack_into("<H", raw, reloc_at + 8, 6)
            if retained_reference:
                raw = append_section(bytes(raw), data=bytes(4), references=((0, 2, 6),))
            elif extra_data:
                raw = append_section(bytes(raw))
            path.write_bytes(raw)
            changed[str(path)] = stamp(raw)
        if witness:
            path = context.objects[1]
            raw = bytearray(path.read_bytes())
            symbol_at, = struct.unpack_from("<I", raw, 8)
            struct.pack_into("<I", raw, symbol_at + 18 + 8, witness_size)
            path.write_bytes(raw)
            changed[str(path)] = stamp(raw)
        evidence = json.loads((context.directory / "link.json").read_bytes())
        for row in evidence["files"]:
            if row["path"] in changed:
                row["before"] = row["after"] = changed[row["path"]]
        update_artifact(context, "link.json", json.dumps(evidence).encode())
        mapping = (context.directory / "app.map").read_text().replace(
            " entry point", " 0002:00000080 _g 00402080 <common>\n entry point")
        update_artifact(context, "app.map", mapping.encode())
        image = bytearray((context.directory / "app.exe").read_bytes())
        if request:
            struct.pack_into("<I", image, 513, 0x402080)
        root_sites = [0x3001, 0] if request else []
        data_sites = [0x3004, 0x30C0] if retained_reference else [0x3004, 0]
        relocations = ((struct.pack("<II2H", 0x1000, 12, *root_sites) if root_sites else b"")
                       + struct.pack("<II2H", 0x2000, 12, *data_sites))
        image[896:896 + len(relocations)] = relocations
        struct.pack_into("<II", image, 0x98 + 96 + 5 * 8, 0x1180, len(relocations))
        if retained_reference:
            struct.pack_into("<I", image, 1024 + 0xC0, 0x402080)
        elif extra_data:
            image[1024 + 0xC0] = 0x7F
        update_artifact(context, "app.exe", bytes(image))
        if damage is not None:
            damage(context)
    return startup_context(root, b"\xa1" + bytes(4) + b"\xc3" if request else b"\xc3",
                           reference="_g" if request else None, damage=configure)


def graph_at(root, *, old_options=None, current_options=None, permission_units=()):
    coordinator = root / "coordinator"
    coordinator.mkdir()
    old, old_sources = common_context(root / "baseline", **(old_options or {}))
    new, new_sources = common_context(root / "current", **({"request": True} | (current_options or {})))
    changes = [{"entity_id": "entity-" + "a" * 64, "unit": UNIT, "version": version,
                "symbol": "_$E1", "offset": 0, "evidence": ["synthetic generated-root change"]}
               for version in ("baseline", "current")]
    changes.extend({"entity_id": "entity-" + "d" * 64, "unit": unit, "version": version,
                    "symbol": "_$other", "offset": 0, "evidence": ["separate authorized generated domain"]}
                   for unit, version in permission_units)
    authorization = {"task": {"task_id": "owner"}, "decisions": [], "permission": {
        "migration_id": "startup-common", "author": "coordinator", "allowed": [],
        "evidence": ["bounded common request redundancy"], "generated_changes": changes}}
    def fresh():
        try:
            old.check_current()
        finally:
            new.check_current()
    graph = ApplicationGraph(coordinator, "owner", "1" * 40, ("src",), old, new,
        {"baseline": old_sources, "current": new_sources}, {"result": "unknown"}, authorization, fresh)
    return graph, changes


def capture(graph, version="current"):
    context = getattr(graph, version)
    raw = context.objects[0].read_bytes()
    trial = ObjectTrial(raw, graph.root, graph.task_name, graph.baseline_revision, digest(graph.authorization),
                        snapshot_digest(graph.source_versions[version]), UNIT, version, FLAGS, INPUT)
    return trial, Coff(raw)


def startup(run, graph, changes, trial, obj):
    return run.cover_startup(trial, obj, unit=UNIT, version=trial.version, flags=FLAGS,
        input_fingerprint=INPUT, changes=[row for row in changes if row["unit"] == UNIT and row["version"] == trial.version])


def common(run, trial, obj, cap, **overrides):
    arguments = dict(unit=UNIT, version=trial.version, flags=FLAGS, input_fingerprint=INPUT, startup=cap)
    arguments.update(overrides)
    return run.cover_common_requests(trial, obj, **arguments)


def test_fixture_has_real_linked_initializer_and_uniform_common_proofs(tmp_path):
    graph, _ = graph_at(tmp_path)
    for context in (graph.baseline, graph.current):
        observed = observe_build_initializers(context)
        assert observed["result"] == observed["code_dependencies"]["result"] == "pass", observed
    with linked_build(graph.baseline) as old, linked_build(graph.current) as new:
        proof = _compare_linked(old, new, ("_g",))
        assert proof["result"] == "pass"
        assert proof["baseline"]["allocations"][0]["size"] == proof["current"]["allocations"][0]["size"] == 4
        assert proof["allocation_isolation"] == "unknown"


def test_real_native_duplicate_request_projection_removes_only_the_common_symbol_row(tmp_path):
    graph, changes = graph_at(tmp_path)
    with application_objects(graph) as run:
        old_trial, old = capture(graph, "baseline")
        new_trial, new = capture(graph)
        old_startup = startup(run, graph, changes, old_trial, old)
        new_startup = startup(run, graph, changes, new_trial, new)
        assert common(run, old_trial, old, old_startup) is None
        before = compare_outside_functions(old, new, {}, {}, baseline_startup=old_startup, current_startup=new_startup)
        assert before["result"] == "fail"
        cap = common(run, new_trial, new, new_startup)
        assert cap is not None
        after = compare_outside_functions(old, new, {}, {}, baseline_startup=old_startup,
                                         current_startup=new_startup, current_common=cap)
        assert after["result"] == "pass"
        assert graph.report["result"] == "unknown"


@pytest.mark.parametrize("version", ["baseline", "current"])
def test_witness_in_any_version_of_generated_domain_is_not_retained(tmp_path, version):
    graph, changes = graph_at(tmp_path, permission_units=(("src/dependency.cpp", version),))
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        cap = startup(run, graph, changes, trial, obj)
        with pytest.raises(ValueError):
            common(run, trial, obj, cap)


@pytest.mark.parametrize("old_options,current_options", [({"witness": False}, {"witness": False}),
    ({"witness": False}, {}), ({}, {"witness": False}), ({}, {"witness_size": 8}),
    ({"witness_size": 8}, {})])
def test_missing_or_mismatched_surviving_witness_never_grants_projection(tmp_path, old_options, current_options):
    graph, changes = graph_at(tmp_path, old_options=old_options, current_options=current_options)
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        cap = startup(run, graph, changes, trial, obj)
        with pytest.raises(ValueError):
            common(run, trial, obj, cap)


@pytest.mark.parametrize("effect", ["retained_reference", "extra_data"])
def test_common_redundancy_never_erases_a_remaining_reference_or_unowned_data(tmp_path, effect):
    graph, changes = graph_at(tmp_path, current_options={effect: True})
    with application_objects(graph) as run:
        old_trial, old = capture(graph, "baseline")
        trial, obj = capture(graph)
        old_cap = startup(run, graph, changes, old_trial, old)
        startup_cap = startup(run, graph, changes, trial, obj)
        if effect == "retained_reference":
            with pytest.raises(ValueError):
                cap = common(run, trial, obj, startup_cap)
                compare_outside_functions(old, obj, {}, {}, baseline_startup=old_cap,
                    current_startup=startup_cap, current_common=cap)
        else:
            cap = common(run, trial, obj, startup_cap)
            assert compare_outside_functions(old, obj, {}, {}, baseline_startup=old_cap,
                current_startup=startup_cap, current_common=cap)["result"] == "fail"


def test_startup_capability_from_another_live_producer_epoch_is_refused(tmp_path):
    graph, changes = graph_at(tmp_path)
    trial, obj = capture(graph)
    with application_objects(graph) as first, application_objects(graph) as second:
        borrowed = startup(first, graph, changes, trial, obj)
        with pytest.raises(ValueError):
            common(second, trial, obj, borrowed)


@pytest.mark.parametrize("cap", [None, {}, {"result": "pass", "symbols": ["_g"]}])
def test_common_producer_cannot_use_a_report_as_startup_authority(tmp_path, cap):
    graph, _ = graph_at(tmp_path)
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        with pytest.raises(ValueError):
            common(run, trial, obj, cap)


def test_common_capability_is_not_constructible_copyable_or_serializable(tmp_path):
    graph, changes = graph_at(tmp_path)
    with pytest.raises(TypeError):
        DuplicateCommonRequestCoverage()
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        start = startup(run, graph, changes, trial, obj)
        cap = common(run, trial, obj, start)
        assert cap is not None
        for operation in (copy, deepcopy, pickle.dumps, json.dumps):
            with pytest.raises(TypeError):
                operation(cap)
        forged = object.__new__(DuplicateCommonRequestCoverage)
        with pytest.raises(ValueError, match="authentic"):
            compare_outside_functions(obj, obj, {}, {}, current_startup=start, current_common=forged)


@pytest.mark.parametrize("damage", ["missing-startup", "other-startup", "other-object", "parsed-mutation"])
def test_common_consumer_requires_exact_live_object_and_actual_startup_projection(tmp_path, damage):
    graph, changes = graph_at(tmp_path)
    with application_objects(graph) as run:
        old_trial, old = capture(graph, "baseline")
        trial, obj = capture(graph)
        old_start = startup(run, graph, changes, old_trial, old)
        start = startup(run, graph, changes, trial, obj)
        cap = common(run, trial, obj, start)
        if damage == "missing-startup":
            start = None
        elif damage == "other-startup":
            start = startup(run, graph, changes, trial, obj)
        elif damage == "other-object":
            obj = Coff(trial.raw)
        else:
            obj.symbols[2] = replace(obj.symbols[2], value=8)
        with pytest.raises(ValueError):
            compare_outside_functions(old, obj, {}, {}, baseline_startup=old_start,
                                     current_startup=start, current_common=cap)


def test_common_capability_expires_with_native_producer(tmp_path):
    graph, changes = graph_at(tmp_path)
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        start = startup(run, graph, changes, trial, obj)
        cap = common(run, trial, obj, start)
    with pytest.raises(ValueError, match="expired"):
        compare_outside_functions(obj, obj, {}, {}, current_startup=start, current_common=cap)
    with pytest.raises(ValueError, match="expired"):
        common(run, trial, obj, start)


@pytest.mark.parametrize("field,value", [("flags", ("/c", "/Od")), ("unit", "src/dependency.cpp"),
    ("input_fingerprint", "c" * 64), ("version", "baseline")])
def test_common_capability_cannot_retarget_native_trial(tmp_path, field, value):
    graph, changes = graph_at(tmp_path)
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        start = startup(run, graph, changes, trial, obj)
        with pytest.raises(ValueError):
            common(run, trial, obj, start, **{field: value})


@pytest.mark.parametrize("version", ["baseline", "current"])
@pytest.mark.parametrize("consume", [False, True])
def test_deferred_witness_mutation_is_refused_even_without_another_consumer(tmp_path, version, consume):
    graph, changes = graph_at(tmp_path)
    with pytest.raises(ValueError):
        with application_objects(graph) as run:
            trial, obj = capture(graph)
            start = startup(run, graph, changes, trial, obj)
            cap = common(run, trial, obj, start)
            assert cap is not None
            witness = getattr(graph, version).objects[1]
            raw = bytearray(witness.read_bytes())
            raw[4] ^= 1  # Physical input identity changes, even if code does not.
            witness.write_bytes(raw)
            if consume:
                with pytest.raises(ValueError):
                    compare_outside_functions(obj, obj, {}, {}, current_startup=start, current_common=cap)


def test_authorization_mutation_after_last_consume_refuses_final_epoch(tmp_path):
    graph, changes = graph_at(tmp_path)
    with pytest.raises(ValueError, match="authorization changed"):
        with application_objects(graph) as run:
            old_trial, old = capture(graph, "baseline")
            trial, obj = capture(graph)
            old_start = startup(run, graph, changes, old_trial, old)
            start = startup(run, graph, changes, trial, obj)
            cap = common(run, trial, obj, start)
            assert compare_outside_functions(old, obj, {}, {}, baseline_startup=old_start,
                current_startup=start, current_common=cap)["result"] == "pass"
            graph._authorization["permission"]["evidence"].append("changed decision")


def test_unchanged_common_declarations_need_no_projection_even_without_outside_witness(tmp_path):
    graph, changes = graph_at(tmp_path, old_options={"request": True, "witness": False},
                              current_options={"witness": False})
    with linked_build(graph.baseline) as old, linked_build(graph.current) as new:
        assert _compare_linked(old, new, ("_g",))["result"] == "pass"
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        start = startup(run, graph, changes, trial, obj)
        assert common(run, trial, obj, start) is None


def append_symbols(context, rows):
    """Change physical root symbols before the native receipts are sealed."""
    path = context.objects[0]
    raw = bytearray(path.read_bytes())
    start, count = struct.unpack_from("<II", raw, 8)
    raw[start + count * 18:start + count * 18] = b"".join(rows)
    struct.pack_into("<I", raw, 12, count + len(rows))
    path.write_bytes(raw)
    receipt = json.loads((context.directory / "link.json").read_bytes())
    row = next(row for row in receipt["files"] if row["path"] == str(path))
    row["before"] = row["after"] = stamp(raw)
    update_artifact(context, "link.json", json.dumps(receipt).encode())


@pytest.mark.parametrize("kind", ["same-name", "weak-fallback"])
def test_physical_alias_or_weak_fallback_cannot_be_hidden_as_redundant_common(tmp_path, kind):
    def damage(context):
        if kind == "same-name":
            rows = [struct.pack("<8sIhHBB", b"_g", 0, 0, 0, 2, 0)]
        else:
            rows = [struct.pack("<8sIhHBB", b"_alias", 0, 0, 0, 105, 1),
                    struct.pack("<II10s", 2, 3, bytes(10))]
        append_symbols(context, rows)
    graph, changes = graph_at(tmp_path, current_options={"damage": damage})
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        assert len([s for s in obj.symbols.values() if s.name == "_g"]) == (2 if kind == "same-name" else 1)
        with pytest.raises(ValueError):
            start = startup(run, graph, changes, trial, obj)
            common(run, trial, obj, start)


def test_common_capability_also_flows_through_unit_presence_without_waiving_other_effects(tmp_path):
    graph, changes = graph_at(tmp_path)
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        start = startup(run, graph, changes, trial, obj)
        cap = common(run, trial, obj, start)
        assert compare_unit_presence(obj, {}, present="current", startup=start)["result"] == "fail"
        assert compare_unit_presence(obj, {}, present="current", startup=start, common=cap)["result"] == "pass"


def test_startup_capability_cannot_cross_broader_authorization_epochs(tmp_path):
    graph, changes = graph_at(tmp_path)
    authorization = deepcopy(graph.authorization)
    authorization["permission"]["generated_changes"].append({
        "entity_id": "entity-" + "d" * 64, "unit": "src/dependency.cpp", "version": "current",
        "symbol": "_$other", "offset": 0, "evidence": ["broader generated domain"]})
    other = replace(graph, _authorization=authorization)
    trial, obj = capture(graph)
    other_trial = replace(trial, authorization=digest(other.authorization))
    with application_objects(graph) as first, application_objects(other) as second:
        start = startup(first, graph, changes, trial, obj)
        assert second(other_trial, obj, unit=UNIT, version="current", flags=FLAGS,
                      input_fingerprint=INPUT)["result"] == "pass"
        with pytest.raises(ValueError):
            common(second, other_trial, obj, start)


@pytest.mark.parametrize("field,value", [("input_fingerprint", "c" * 64), ("flags", ("/c", "/Od"))])
def test_same_epoch_startup_capability_cannot_be_borrowed_by_another_valid_trial(tmp_path, field, value):
    graph, changes = graph_at(tmp_path)
    with application_objects(graph) as run:
        trial, obj = capture(graph)
        start = startup(run, graph, changes, trial, obj)
        altered = replace(trial, **{field: value})
        arguments = dict(unit=UNIT, version="current", flags=FLAGS, input_fingerprint=INPUT)
        arguments[field] = value
        # Whole-output equality alone permits this second independently bound
        # trial. Its startup authority must nevertheless be its own.
        assert run(altered, obj, **arguments)["result"] == "pass"
        with pytest.raises(ValueError):
            run.cover_common_requests(altered, obj, startup=start, **arguments)
