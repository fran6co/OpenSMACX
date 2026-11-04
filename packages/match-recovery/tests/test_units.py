"""Controls for explicit contribution ranges and coherent native trial scoring."""

import json
import base64
from types import SimpleNamespace

import pytest

from match_recovery.model import Origin
from match_recovery.graph import DependencyGraph
from match_recovery.cli import parser
from match_recovery import units as units_module
from match_recovery.units import (_identity_counts, _native_current_translation_units,
                                  _counterpart_representation, _single_trial, _summary_identity_counts,
                                  _unit_members, _unit_trial_status,
                                  read_units_config)


PIN = "a" * 64


class BytesImage:
    def __init__(self, data):
        self.data = data

    def read(self, span):
        return self.data[span.start - 0x1000:span.end - 0x1000]


def unit(start=0x1000, end=0x1008):
    return {"name": "fixture", "start": start, "end_exclusive": end,
            "reason": "synthetic interval", "evidence": ["synthetic source evidence"]}


def entity(address, *, spans=(), source="src/a.cpp", unbound=False, component=None, seed=False):
    origin = Origin(PIN, "pe:va", address)
    candidate = {"origin": {"image_sha256": PIN, "address_space": "pe:va", "address": address},
                 "spans": [[a, b] for a, b in spans], "path": source, "line": 1,
                 "seed": seed, "symbol": "_f" if not unbound else "",
                 "component": component, "context_path": "", "unbound": unbound}
    return {"entity_id": origin.entity_id, "origin": candidate["origin"],
            "mapping_disposition": "unbound_pending" if unbound else "source_mapped",
            "candidates": [candidate]}


def ledger(*entities, rows=()):
    view = {"groups": [{"entities": list(entities)}]}
    snapshot = {"image_sha256": PIN, "source_configuration": {"address_space": "pe:va"},
                "records": list(rows)}
    return view, snapshot


def source_row(address, spans, *, mode="body", path="src/a.cpp", disposition="source_mapping",
               candidate_kind="source", invalid=()):
    origin = Origin(PIN, "pe:va", address)
    return {"entity_id": origin.entity_id, "origin": {"image_sha256": PIN,
            "address_space": "pe:va", "address": address}, "path": path, "line": 1,
            "candidate_kind": candidate_kind, "legacy_mode": mode, "spans": [list(s) for s in spans],
            "invalid_spans": [list(s) for s in invalid], "mapping_disposition": disposition}


def test_config_uses_only_explicit_nonoverlapping_intervals(tmp_path):
    path = tmp_path / "units.json"
    path.write_text(json.dumps({"schema_version": 1, "units": [
        {"name": "a", "start": "0x1000", "end_exclusive": "0x1010",
         "recovered_tu": "src/a.cpp", "reason": "evidence", "evidence": ["source marker"]}]}))
    parsed = read_units_config(path)
    assert parsed[0]["start"] == 0x1000
    assert "members" not in parsed[0] and "identity_ids" not in parsed[0]
    parsed[0]["name"] = "mutated"
    overlap = {"schema_version": 1, "units": [
        {"name": "a", "start": 0x1000, "end_exclusive": 0x1010,
         "recovered_tu": "src/a.cpp", "reason": "x", "evidence": ["x"]},
        {"name": "b", "start": 0x100f, "end_exclusive": 0x1020,
         "recovered_tu": "src/b.cpp", "reason": "y", "evidence": ["y"]}]}
    path.write_text(json.dumps(overlap))
    with pytest.raises(ValueError, match="overlap"):
        read_units_config(path)


def _representation_fixture(tmp_path):
    (tmp_path / "src").mkdir()
    (tmp_path / "src/a.cpp").write_text("// empty counterpart\n")
    source = SimpleNamespace(
        path=lambda root, relative: root / relative,
        roots=("src",), interface_extensions={".h"},
        is_seed=lambda relative: False,
    )
    snapshot = {"files": {"src/a.cpp": {} }}
    return source, snapshot, DependencyGraph(())


def test_placeholder_is_source_represented_without_becoming_byte_exact(tmp_path):
    source, snapshot, closure = _representation_fixture(tmp_path)
    unit_row = {"start": 0x1000, "end_exclusive": 0x1010, "recovered_tu": "src/a.cpp"}
    representation = _counterpart_representation(
        tmp_path, unit_row, [], {"groups": []}, snapshot, source,
        {".cpp"}, {"src/a.cpp"}, closure)
    assert representation["represented_cpp"]
    assert representation["counterpart_placeholder"]
    assert not representation["representation_issues"]


def test_split_home_and_external_body_block_source_representation(tmp_path):
    source, snapshot, closure = _representation_fixture(tmp_path)
    member = {"entity_id": "inside", "address": "0x00001000", "source_candidates": [
        {"path": "src/a.cpp", "context_path": "", "body_path": "src/b.cpp",
         "seed": False, "unbound": False, "component": None} ]}
    outside = {"entity_id": "outside", "origin": {"address": 0x2000}, "candidates": [
        {"path": "src/a.cpp", "context_path": "", "seed": False,
         "unbound": False, "component": None}]}
    (tmp_path / "src/b.cpp").write_text("void body() {}\n")
    snapshot["files"]["src/b.cpp"] = {}
    unit_row = {"start": 0x1000, "end_exclusive": 0x1010, "recovered_tu": "src/a.cpp"}
    representation = _counterpart_representation(
        tmp_path, unit_row, [member], {"groups": [{"entities": [outside]}]},
        snapshot, source, {".cpp"}, {"src/a.cpp", "src/b.cpp"}, closure)
    kinds = {issue["kind"] for issue in representation["representation_issues"]}
    assert not representation["represented_cpp"]
    assert {"implementation_in_other_translation_unit",
            "external_original_identity_in_recovered_tu"} <= kinds


def test_unbound_and_seed_candidates_do_not_establish_representation(tmp_path):
    source, snapshot, closure = _representation_fixture(tmp_path)
    unit_row = {"start": 0x1000, "end_exclusive": 0x1010, "recovered_tu": "src/a.cpp"}
    for seed, unbound in ((False, True), (True, False)):
        member = {"entity_id": "inside", "address": "0x00001000", "source_candidates": [
            {"path": "src/a.cpp", "context_path": "", "body_path": "",
             "seed": seed, "unbound": unbound, "component": None}]}
        result = _counterpart_representation(tmp_path, unit_row, [member], {"groups": []},
            snapshot, source, {".cpp"}, {"src/a.cpp"}, closure)
        assert not result["represented_cpp"]


def test_unconfigured_or_missing_counterpart_cannot_represent_candidate(tmp_path):
    source, snapshot, closure = _representation_fixture(tmp_path)
    unit_row = {"start": 0x1000, "end_exclusive": 0x1010, "recovered_tu": "src/missing.cpp"}
    result = _counterpart_representation(tmp_path, unit_row, [], {"groups": []},
        snapshot, source, {".cpp"}, {"src/a.cpp"}, closure)
    assert not result["represented_cpp"]
    assert "recovered_tu_not_built" in {issue["kind"] for issue in result["representation_issues"]}


def test_summary_identity_counts_are_unique_and_limited_to_declared_units():
    member = {"entity_id": "identity-a"}
    counts = _summary_identity_counts([
        {"members": [member], "matched_code_unit": True, "evidence_closed_unit": False},
        {"members": [member, {"entity_id": "identity-b"}],
         "matched_code_unit": False, "evidence_closed_unit": True},
    ])
    assert counts["candidate_identity_count"] == 2
    assert counts["matched_code_unit_identity_count"] == 1
    assert counts["evidence_closed_unit_identity_count"] == 2
    assert "not a global" in counts["identity_count_scope"]


def test_cli_exposes_configured_units_metric():
    args = parser().parse_args(["units", "--image", "game.exe",
                                "--config", "recovery/units.json", "--summary", "--mapping-only"])
    assert args.command == "units" and args.image.as_posix() == "game.exe"
    assert args.config.as_posix() == "recovery/units.json" and args.summary
    assert args.mapping_only


def test_mapping_only_reports_representation_without_calling_native_verifier(tmp_path, monkeypatch):
    root = tmp_path
    (root / "recovery").mkdir()
    (root / "src").mkdir()
    (root / "recovery/project.json").write_text(json.dumps({
        "mappings": {"compile_commands": "build/commands.json"}}))
    (root / "build").mkdir()
    (root / "build/commands.json").write_text("[]")
    image_path = root / "game.exe"
    image_path.write_bytes(b"pinned image fixture")
    config_path = root / "recovery/units.json"
    config_path.write_text(json.dumps({"schema_version": 1, "units": [{
        "name": "candidate", "start": "0x1000", "end_exclusive": "0x1008",
        "recovered_tu": "src/a.cpp", "reason": "fixture", "evidence": ["fixture"]}]}))
    pin = "c" * 64
    settings = SimpleNamespace(flag_sets=(), unit_extensions={".cpp"},
                               snapshot=lambda _root, _extra: {"epoch": 1})
    source = SimpleNamespace(path=lambda _root, relative: root / relative,
                             include_paths=(), roots=("src",), interface_extensions={".h"},
                             is_seed=lambda _path: False)
    record = SimpleNamespace(address=0x1000)
    member = {"entity_id": "fixture-id", "address": "0x00001000",
              "current_source_homes": [], "seed_only": False,
              "unbound": False, "component": None}
    snapshot = {"image_sha256": pin, "revision": "fixture", "source_configuration": {
        "address_space": "pe:va"}, "files": {"src/a.cpp": {
            "content_base64": base64.b64encode(b"void placeholder();\n").decode()}},
        "records": []}
    monkeypatch.setattr(units_module.Measurement, "read", lambda _root: settings)
    monkeypatch.setattr(units_module, "image_pin", lambda _root: pin)
    monkeypatch.setattr(units_module, "project_view", lambda *_args: ({"groups": []}, snapshot))
    monkeypatch.setattr(units_module, "PeImage", lambda *_args: SimpleNamespace(
        read=lambda span: b"\x90" * (span.end - span.start)))
    monkeypatch.setattr(units_module, "_source_records", lambda *_args: {})
    monkeypatch.setattr(units_module.SourceConfiguration, "from_dict", lambda _value: source)
    monkeypatch.setattr(units_module, "include_graph", lambda *_args: (SimpleNamespace(edges=()), ()))
    monkeypatch.setattr(units_module, "assert_snapshot_current", lambda *_args: None)
    monkeypatch.setattr(units_module, "_unit_members", lambda *_args: (
        [member], [record], [], {"closed": True}))
    monkeypatch.setattr(units_module, "_counterpart_representation", lambda *_args: {
        "recovered_tu": "src/a.cpp", "represented_cpp": True,
        "counterpart_placeholder": False, "representation_issues": []})
    monkeypatch.setattr(units_module, "_owned_obligations", lambda *_args: [])
    import match_recovery.mapping_check as mapping_check
    monkeypatch.setattr(mapping_check, "build_units", lambda *_args: ({"src/a.cpp"}, ()))
    monkeypatch.setattr(units_module, "verify_records", lambda *_args, **_kwargs: pytest.fail(
        "mapping-only mode must not invoke native verification"))

    report = units_module.units_report(root, image_path, config_path, mapping_only=True)
    assert report["metric_mode"] == "mapping_only"
    assert report["counts"]["identified_original_tu_candidates"] == 1
    assert report["counts"]["represented_cpp_units"] == 1
    assert report["counts"]["fully_byte_exact_units"] is None
    assert report["units"][0]["fully_byte_exact_unit"] is None
    assert report["units"][0]["matched_code_unit"] is None
    assert report["units"][0]["native_measurement"] == "not_run"


def test_per_function_best_union_does_not_make_a_whole_unit_match():
    members = [{"entity_id": "a"}, {"entity_id": "b"}]
    report = {"functions": [{"entity_id": "a", "result": "pass"},
                             {"entity_id": "b", "result": "pass"}],
              "selected_evidence": {"trials": [
                  {"result": "fail", "function_results": ["pass", "fail"]},
                  {"result": "fail", "function_results": ["fail", "pass"]}]}}
    complete, code, evidence = _single_trial(report, members)
    assert complete and code == [] and evidence == []


def test_missing_identity_in_native_report_cannot_match():
    complete, code, evidence = _single_trial(
        {"functions": [{"entity_id": "a", "result": "pass"}],
         "selected_evidence": {"trials": [{"result": "pass", "function_results": ["pass"]}]}},
        [{"entity_id": "a"}, {"entity_id": "missing"}])
    assert not complete and code == [] and evidence == []


def test_code_match_and_evidence_closure_use_the_same_native_trial():
    members = [{"entity_id": "a"}, {"entity_id": "b"}]
    report = {"functions": [{"entity_id": "a", "result": "pass"},
                             {"entity_id": "b", "result": "pass"}],
              "selected_evidence": {"trials": [
                  {"result": "fail", "function_results": ["pass", "pass"],
                   "evidence_results": ["fail"]},
                  {"result": "pass", "function_results": ["pass", "pass"],
                   "evidence_results": ["pass"]}]}}
    complete, code, evidence = _single_trial(report, members)
    assert complete and code == [0, 1] and evidence == [1]


def test_unexplained_original_gap_and_overlap_remain_visible():
    first, second = entity(0x1000, spans=((0x1000, 0x1004),)), entity(0x1004, spans=((0x1004, 0x1008),))
    rows = [source_row(0x1000, ((0x1000, 0x1004),)), source_row(0x1004, ((0x1004, 0x1008),))]
    view, snapshot = ledger(first, second, rows=rows)
    members, _, issues, coverage = _unit_members(view, snapshot, {}, unit(), BytesImage(b"\x90\x90\x90\x90\x90\x90\x90\x90"))
    assert len(members) == 2 and coverage["closed"]
    # A non-padding byte in a mapped-function gap cannot be called padding.
    first, second = entity(0x1000, spans=((0x1000, 0x1002),)), entity(0x1006, spans=((0x1006, 0x1008),))
    rows = [source_row(0x1000, ((0x1000, 0x1002),)), source_row(0x1006, ((0x1006, 0x1008),))]
    view, snapshot = ledger(first, second, rows=rows)
    data = b"\xc3\xc3\x00\x90\x90\xc3\xc3\x90"
    _, _, issues, coverage = _unit_members(view, snapshot, {}, unit(), BytesImage(data))
    assert not coverage["closed"] and any(item["kind"] == "unexplained_gap" for item in issues)
    first, second = entity(0x1000, spans=((0x1000, 0x1005),)), entity(0x1004, spans=((0x1004, 0x1008),))
    rows = [source_row(0x1000, ((0x1000, 0x1005),)), source_row(0x1004, ((0x1004, 0x1008),))]
    view, snapshot = ledger(first, second, rows=rows)
    _, _, issues, coverage = _unit_members(view, snapshot, {}, unit(), BytesImage(b"\x90" * 8))
    assert not coverage["closed"] and any(item["kind"] == "overlapping_function_spans" for item in issues)


def test_unbound_member_is_counted_but_blocks_verified_code_coverage():
    ordinary = entity(0x1000, spans=((0x1000, 0x1002),))
    pending = entity(0x1002, unbound=True)
    rows = [source_row(0x1000, ((0x1000, 0x1002),)),
            source_row(0x1002, (), mode="unbound", disposition="unbound_pending")]
    view, snapshot = ledger(ordinary, pending, rows=rows)
    members, _, issues, coverage = _unit_members(
        view, snapshot, {}, unit(), BytesImage(b"\xc3\xc3\x90\x90\x90\x90\x90\x90"))
    assert len(members) == 2
    assert not coverage["closed"]
    assert any(item["kind"] == "unbound_identity" for item in issues)
    assert any(item["kind"] == "missing_primary_span" for item in issues)
    counts = _identity_counts(members)
    assert counts["known_function_count"] == 2
    assert counts["known_product_function_count"] == 1
    assert counts["unbound_function_count"] == 1


def test_passing_seed_only_identity_stays_in_denominator_but_not_product_match():
    seed = entity(0x1000, spans=((0x1000, 0x1008),), source="seeds/old.cpp", seed=True)
    row = source_row(0x1000, ((0x1000, 0x1008),), path="seeds/old.cpp",
                     candidate_kind="legacy_seed")
    view, snapshot = ledger(seed, rows=[row])
    seed_record = type("Record", (), {"address": 0x1000})()
    members, records, issues, coverage = _unit_members(
        view, snapshot, {seed["entity_id"]: [("seeds/old.cpp", seed_record)]},
        unit(), BytesImage(b"\xc3" * 8))
    assert members[0]["seed_only"] and records == [seed_record]
    assert _identity_counts(members) == {"known_function_count": 1,
                                         "known_product_function_count": 0,
                                         "seed_only_function_count": 1,
                                         "unbound_function_count": 0,
                                         "dependency_component_count": 0}
    passing = {"functions": [{"entity_id": seed["entity_id"], "result": "pass", "unit": "seeds/old.cpp"}],
               "selected_evidence": {"trials": [{"result": "pass", "function_results": ["pass"]}]}}
    assert _unit_trial_status(passing, members, coverage["closed"]) == (False, False, [], [])


def test_current_tu_owner_comes_from_native_function_receipt_not_source_home():
    member = {"entity_id": "entity-a", "current_source_homes": ["src/annotation-home.cpp"]}
    report = {"functions": [{"entity_id": "entity-a", "result": "fail",
                              "unit": "src/actual-build-owner.cpp"}]}
    owners, single, unknown = _native_current_translation_units(report, [member])
    assert owners == ["src/actual-build-owner.cpp"] and single is True and unknown == []
    owners, single, unknown = _native_current_translation_units({"functions": []}, [member])
    assert owners == [] and single is None and unknown[0]["entity_id"] == "entity-a"
    owners, single, unknown = _native_current_translation_units(
        {"functions": [{"entity_id": "entity-a", "unit": "src/a.cpp"},
                       {"entity_id": "entity-a", "unit": "src/b.cpp"}]}, [member])
    assert owners == ["src/a.cpp", "src/b.cpp"] and single is None and unknown


def test_detached_secondary_span_is_retained_without_cutting_primary_core():
    member = entity(0x1000, spans=((0x1000, 0x1004), (0x2000, 0x2004)))
    row = source_row(0x1000, ((0x1000, 0x1004), (0x2000, 0x2004)))
    view, snapshot = ledger(member, rows=[row])
    members, _, issues, coverage = _unit_members(
        view, snapshot, {}, unit(), BytesImage(b"\xc3\xc3\xc3\xc3\x90\x90\x90\x90"))
    assert coverage["closed"] and not issues
    assert members[0]["secondary_spans_outside_core"] == [["0x00002000", "0x00002004"]]


def test_candidate_membership_stays_address_based_when_source_home_moves():
    row = source_row(0x1000, ((0x1000, 0x1008),))
    row["spans"] = [[0x1000, 0x1008]]
    before = entity(0x1000, spans=((0x1000, 0x1008),), source="src/old.cpp")
    after = entity(0x1000, spans=((0x1000, 0x1008),), source="src/split.cpp")
    view_a, snapshot_a = ledger(before, rows=[row])
    view_b, snapshot_b = ledger(after, rows=[{**row, "path": "src/split.cpp"}])
    unit_config = unit(end=0x1008)
    members_a, _, _, _ = _unit_members(view_a, snapshot_a, {}, unit_config, BytesImage(b"\xc3" * 8))
    members_b, _, _, _ = _unit_members(view_b, snapshot_b, {}, unit_config, BytesImage(b"\xc3" * 8))
    assert [row["entity_id"] for row in members_a] == [row["entity_id"] for row in members_b]
    assert len(members_a) == len(members_b) == 1
    assert members_a[0]["current_source_homes"] == ["src/old.cpp"]
    assert members_b[0]["current_source_homes"] == ["src/split.cpp"]

