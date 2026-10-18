"""UNBOUND records remain visible identity/claim obligations, not bodies."""

import json

from match_recovery.ownership_source import ownership_files, project_view, structural_impact
from match_recovery.partition import Candidate, partition
from match_recovery.source_inventory import inventory, source_snapshot
from match_recovery.source_quality import quality_report
from match_recovery.symbol_check import check_symbols
from match_recovery.model import Origin, Span


PIN = "a" * 64
MARKER = '''// ORIGINAL: 0x00401000 ?f@Widget@@QAEXXZ 0x00401000-0x00401008 UNBOUND BYTE_EXACT
// LEVER: owner-evidence pinned callback owner is unresolved
// TRIED: source body was an opaque adapter
// DEFERRED: resolve the original class member and callers
'''


def project(tmp_path, *, compiled_decoy=False):
    for name in ("src", "recovery", "build"):
        (tmp_path / name).mkdir()
    (tmp_path / "src/PIN").write_text("sha256 " + PIN + "\n")
    (tmp_path / "src/unbound.h").write_text(MARKER)
    source = {"image_pin": "src/PIN", "address_space": "pe:va", "roots": ["src"],
              "seed_roots": [], "include_paths": ["src"], "symbol_format": "msvc"}
    configuration = {"schema_version": 1, "source": source,
                     "mappings": {"compile_commands": "build/compile_commands.json",
                                  "forwarder_macros": []},
                     "measurement": {"backend": "pe-coff-i386", "compiler_profile": "test",
                                     "flag_sets": [["trial"]], "layout_flags": [],
                                     "unit_extensions": [".cc"], "inputs": [], "input_roots": []}}
    (tmp_path / "recovery/project.json").write_text(json.dumps(configuration))
    if compiled_decoy:
        (tmp_path / "src/decoy.cc").write_text("void f() {}\n")
        (tmp_path / "build/compile_commands.json").write_text(json.dumps([
            {"file": "src/decoy.cc", "directory": str(tmp_path),
             "arguments": ["test-compiler", "src/decoy.cc"]}]))
    else:
        (tmp_path / "build/compile_commands.json").write_text("[]")
    return tmp_path


def test_inventory_preserves_identity_and_claim_but_exposes_no_body_or_symbol(tmp_path):
    root = project(tmp_path)
    snapshot = source_snapshot(root)
    row, = [item for item in snapshot["records"] if item["path"] == "src/unbound.h"]
    assert row["catalogued_symbol"] == "?f@Widget@@QAEXXZ"
    assert row["emitted_symbol_hint"] == ""
    assert row["mapping_disposition"] == "unbound_pending"
    assert row["legacy_mode"] == row["legacy_state"] == "unbound"
    assert row["source_body_region_present"] is False
    assert row["body_path"] == row["context_path"] == ""
    assert row["function_fragment"] is None
    assert row["legacy_byte_exact"] is True
    assert row["levers"] == [["owner-evidence", "pinned callback owner is unresolved"]]
    assert row["tried"] == ["source body was an opaque adapter"]
    assert row["deferred"] == ["resolve the original class member and callers"]
    summary = inventory(snapshot)
    assert summary["unbound_pending"] == 1
    assert summary["mapping_dispositions"] == {"unbound_pending": 1}
    assert summary["historical_byte_obligations"] == 1


def test_partition_keeps_unbound_navigation_hint_but_not_source_home(tmp_path):
    view, snapshot = project_view(project(tmp_path))
    group, = view["groups"]
    assert group["selector"] == "class:Widget"  # catalogue hint only
    assert group["source_homes"] == []
    assert group["historical_tu"] == "unproven"
    assert group["next_action"] == "resolve_unbound"
    assert group["pending_unbound_count"] == 1
    assert ownership_files(snapshot, group) == ["src/unbound.h"]
    impact = structural_impact(snapshot, group)
    assert impact["root_files"] == ["src/unbound.h"]
    entity, = group["entities"]
    assert entity["mapping_disposition"] == "unbound_pending"
    candidate, = entity["candidates"]
    assert candidate["unbound"] is True and candidate["symbol"] == ""
    assert candidate["owner_hint"] is None and candidate["owner_hint_basis"] == "catalogued_symbol"
    assert candidate["catalogued_owner_hint"] == "Widget"
    assert candidate["context_path"] == "" and candidate["function_fragment"] is None


def test_partition_fails_closed_on_unbound_and_real_body_collision():
    origin = Origin(PIN, "pe:va", 0x401000)
    unbound = Candidate(origin, (Span(0x401000, 0x401008),),
                        "src/unbound.h", 1, False, "",
                        owner_hint_basis="catalogued_symbol", catalogued_owner_hint="Widget",
                        unbound=True)
    body = Candidate(origin, (Span(0x401000, 0x401008),),
                     "src/widget.cc", 1, False, "?f@Widget@@QAEXXZ",
                     owner_hint="Widget", owner_hint_basis="source_symbol")
    group, = partition((unbound, body))["groups"]
    entity, = group["entities"]
    assert group["selector"] == "conflict"
    assert "unbound identity collides with a bound source candidate" in entity["conflicts"]
    assert entity["mapping_disposition"] == "conflict"


def test_quality_reports_pending_unbound_not_empty_or_clean_body(tmp_path):
    report = quality_report(project(tmp_path))
    row, = report["functions"]
    assert row["status"] == "unbound"
    assert row["mapping_disposition"] == "unbound_pending"
    assert row["body_resolution"] == "UNBOUND mapping intentionally has no C++ body or emitted symbol"
    assert row["scan_observation"] == "unresolved"
    assert row["findings"] == []
    assert row["historical_byte_claim"] is True
    assert report["counts"]["unbound"] == report["pending_unbound"] == 1
    assert report["historical_byte_claims_by_status"]["unbound"] == {"claimed": 1, "unclaimed": 0}


def test_symbol_check_reports_unknown_without_compiling_or_borrowing_other_tu(tmp_path):
    root = project(tmp_path, compiled_decoy=True)
    calls = []

    def compiler(*args):
        calls.append(args)
        raise AssertionError("UNBOUND must not invoke a compiler")

    report = check_symbols(root, compiler=compiler)
    row, = report["functions"]
    assert report["result"] == "unknown"
    assert report["compile_attempts"] == 0 and calls == []
    assert row["result"] == "unknown" and row["symbol"] == ""
    assert row["mapping_disposition"] == "unbound_pending"
    assert row["variants"] == [{"result": "unknown",
                                "reason": "UNBOUND mapping intentionally has no C++ body or emitted symbol"}]
