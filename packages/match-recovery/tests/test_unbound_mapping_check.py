"""UNBOUND notes are identities, never source body candidates."""

import json
from pathlib import Path

import pytest

from match_recovery import mapping_check as mappings
from match_recovery.component_original_routing import component_record_identity_candidates
from match_recovery.component_source import render_component
from match_recovery.components import SourceFile, VendorBinding, VendorManifest
from match_recovery.model import Component, Delivery, Origin, Ownership, Span
from match_recovery.source_inventory import source_snapshot


@pytest.fixture
def project(tmp_path):
    for name in ("engine/current", "engine/seeds", "build", "recovery", "original"):
        (tmp_path / name).mkdir(parents=True)
    (tmp_path / "original/PIN").write_text("sha256 " + "a" * 64 + "\n")
    marker = "// ORIGINAL: 0x00401000 f 0x00401000-0x00401008 BYTE_EXACT\nint f() { return 1; }\n"
    (tmp_path / "engine/current/widget.cc").write_text(marker)
    (tmp_path / "engine/seeds/widget.cc").write_text(
        marker.replace("0x00401000", "0x00402000").replace("0x00401008", "0x00402008"))
    (tmp_path / "engine/deferred.cc").write_text("int deferred() { return 0; }\n")
    (tmp_path / "build/compilation.json").write_text(json.dumps([
        {"file": name, "directory": "engine", "arguments": ["compiler-not-run", "-c", name]}
        for name in ("current/widget.cc", "deferred.cc")]))
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "original/PIN", "address_space": "custom:offset",
                   "roots": ["engine"], "extensions": [".cc", ".hpp"],
                   "seed_roots": ["engine/seeds"], "include_paths": ["engine"]},
        "mappings": {"compile_commands": "build/compilation.json", "forwarder_macros": []},
    }))
    return tmp_path


def _write_unbound(project, path="engine/current/pending.hpp", address="0x00403000",
                   span="0x00403000-0x00403008", tail=""):
    target = project / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(f"// ORIGINAL: {address} _pending {span} UNBOUND{tail}\n")
    return target


def _rule(report, name):
    return [row for row in report["findings"] if row["rule"] == name]


def test_configured_unbuilt_interface_unbound_is_retained_without_body_fallback(project):
    header = _write_unbound(project, tail=" BYTE_EXACT")
    snapshot = source_snapshot(project)
    row, = [record for record in snapshot["records"] if record["path"] == "engine/current/pending.hpp"]
    assert row["legacy_mode"] == row["legacy_state"] == "unbound"
    assert row["emitted_symbol_hint"] == ""
    assert not row["source_body_region_present"]
    assert not row["body_path"] and not row["context_path"]
    assert row["legacy_byte_exact"]  # still an obligation; this check does not waive it

    report = mappings.check_mappings(project)
    assert report["result"] == "pass", report["findings"]
    assert report["annotations"] == report["entities"] == 3
    assert report["build_units"] == 2
    assert not _rule(report, "unbuilt_host")
    assert not _rule(report, "unbound_source")
    assert header.exists()


def test_unbound_admission_uses_configured_interface_extensions_not_filename(project):
    config_path = project / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["source"]["extensions"].append(".api")
    config["source"]["interface_extensions"] = [".api"]
    config_path.write_text(json.dumps(config))
    _write_unbound(project, "engine/current/records.api")
    report = mappings.check_mappings(project)
    assert report["result"] == "pass", report["findings"]
    assert not _rule(report, "unbuilt_host")


def test_accidentally_compiled_unbound_interface_fails(project):
    header = _write_unbound(project)
    database = project / "build/compilation.json"
    rows = json.loads(database.read_text())
    rows.append({"file": "current/pending.hpp", "directory": "engine",
                 "arguments": ["compiler-not-run", "-c", "current/pending.hpp"]})
    database.write_text(json.dumps(rows))

    report = mappings.check_mappings(project)
    unbound, = _rule(report, "unbound_source")
    assert "build input" in unbound["reason"]
    assert unbound["path"] == "engine/current/pending.hpp"
    assert header.exists()


@pytest.mark.parametrize("mode_tail", ["", " FILE"])
def test_ordinary_unbuilt_body_or_file_in_interface_still_fails(project, mode_tail):
    header = project / "engine/current/ordinary.hpp"
    header.write_text("// ORIGINAL: 0x00403000 _ordinary 0x00403000-0x00403008" + mode_tail +
                      "\nint ordinary() { return 0; }\n")
    report = mappings.check_mappings(project)
    assert report["result"] == "fail"
    assert any(row["path"] == "engine/current/ordinary.hpp" for row in _rule(report, "unbuilt_host"))


def test_unbound_seed_is_not_an_unbound_interface_exception_or_buildable_seed(project):
    _write_unbound(project, "engine/seeds/pending.hpp", address="0x00404000",
                   span="0x00404000-0x00404008")
    database = project / "build/compilation.json"
    rows = json.loads(database.read_text())
    rows.append({"file": "seeds/pending.hpp", "directory": "engine",
                 "arguments": ["compiler-not-run", "-c", "seeds/pending.hpp"]})
    database.write_text(json.dumps(rows))
    report = mappings.check_mappings(project)
    assert report["result"] == "fail"
    assert any(row["path"] == "engine/seeds/pending.hpp" for row in _rule(report, "unbound_source"))
    assert any(row["rule"] == "seed_build_input" and row["path"] == "engine/seeds/pending.hpp"
               for row in report["findings"])


def test_unbound_cannot_compete_with_product_body_for_same_original(project):
    _write_unbound(project, address="0x00401000", span="0x00401000-0x00401008")
    report = mappings.check_mappings(project)
    ambiguous, = _rule(report, "ambiguous_source")
    assert "unbound identity" in ambiguous["reason"]
    assert {row["path"] for row in ambiguous["candidates"]} == {
        "engine/current/widget.cc", "engine/current/pending.hpp"}


def test_product_over_seed_adoption_cannot_satisfy_unbound_identity(project):
    _write_unbound(project, address="0x00402000", span="0x00402000-0x00402008")
    report = mappings.check_mappings(project)
    ambiguous, = _rule(report, "ambiguous_source")
    assert "unbound identity" in ambiguous["reason"]
    assert {row["path"] for row in ambiguous["candidates"]} == {
        "engine/current/pending.hpp", "engine/seeds/widget.cc"}
    assert not report["seed_alternatives"]


def test_component_manifest_collision_is_not_an_unbuilt_unbound_exemption(project):
    config_path = project / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["source"]["extensions"].append(".h")
    config["source"]["interface_extensions"] = [".hpp", ".h"]
    config_path.write_text(json.dumps(config))
    companion = _write_unbound(project, "engine/current/pending.recovery.h",
                               address="0x00403000", span="0x00403000-0x00403008")
    snapshot = source_snapshot(project)
    row, = [record for record in snapshot["records"]
            if record["path"] == "engine/current/pending.recovery.h"]
    origin = Origin(snapshot["image_sha256"], "custom:offset", 0x403000)
    binding = VendorBinding(origin, (Span(0x403000, 0x403008),), "_pending",
                            "vendor/LICENSE", "component collision control")
    manifest = VendorManifest(
        Component("runtime", Ownership.RUNTIME, Delivery.UPSTREAM_SOURCE,
                  ("component collision control",)),
        "vendor", "https://example.invalid/source.tar", "b" * 64, "vendor/LICENSE",
        (SourceFile("vendor/LICENSE", "vendor/LICENSE"),), "c++", ("vendor/LICENSE",), (),
        (binding,))
    companion.write_text(companion.read_text() + render_component(manifest))

    snapshot = source_snapshot(project)
    key = ("engine/current/pending.recovery.h", row["entity_id"])
    assert key in component_record_identity_candidates(snapshot)
    assert mappings.exact_component_record_bindings(snapshot) == {}
    report = mappings.check_mappings(project)
    conflict, = [finding for finding in _rule(report, "unbound_source")
                 if "component manifest interpretation" in finding["reason"]]
    assert conflict["path"] == key[0]
    assert report["result"] == "fail"


def test_invalid_unbound_extent_fails_closed(project):
    _write_unbound(project, span="0x00403008-0x00403000")
    with pytest.raises(ValueError, match="invalid original image span"):
        mappings.check_mappings(project)


@pytest.mark.parametrize("damage", ["entity", "origin", "body", "context", "symbol"])
def test_unbound_missing_identity_or_forbidden_fallback_fails_closed(project, monkeypatch, damage):
    _write_unbound(project)
    original_snapshot = mappings.source_snapshot

    def damaged_snapshot(root):
        snapshot = original_snapshot(root)
        row = next(record for record in snapshot["records"]
                   if record["path"] == "engine/current/pending.hpp")
        if damage == "entity":
            row["entity_id"] = ""
        elif damage == "origin":
            row["origin"].pop("address")
        elif damage == "body":
            row["body_path"] = "engine/current/widget.cc"
        elif damage == "context":
            row["context_path"] = "engine/current/widget.cc"
        else:
            row["emitted_symbol_hint"] = "_pending"
        return snapshot

    monkeypatch.setattr(mappings, "source_snapshot", damaged_snapshot)
    report = mappings.check_mappings(project)
    row, = _rule(report, "unbound_source")
    expected = ("missing a valid original identity" if damage in {"entity", "origin"}
                else "body, context, or emitted-symbol fallback")
    assert expected in row["reason"]


def test_unbound_marker_without_original_identity_is_rejected_by_reader():
    from match_recovery.source.reader import read_text

    with pytest.raises(ValueError, match="UNBOUND"):
        read_text("// ORIGINAL: 0x00403000 UNBOUND\n", Path("engine/current/pending.hpp"))
