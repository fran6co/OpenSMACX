"""Exact component-owned ORIGINAL routing controls using synthetic inputs."""

from dataclasses import replace
import base64
import hashlib
import json
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.component_original_routing import (component_claim_result,
                                                        component_function_observations,
                                                        exact_component_record_bindings)
from match_recovery.component_source import render_component
from match_recovery.components import (SourceFile, ToolchainManifest, VendorBinding,
                                       VendorManifest)
from match_recovery.mapping_check import check_mappings
from match_recovery.match_check import check_matches
from match_recovery.model import Component, Delivery, Origin, Ownership, Span
from match_recovery.ownership_source import project_view
from match_recovery.source import read_text, write
from match_recovery.source_inventory import source_snapshot
from match_recovery.source_measure import measure_source
from test_binary import coff_fixture
from test_claim_integrity import commit, git
from test_toolchain_components import archive
from test_binary import pe_fixture


@pytest.fixture
def component_route_project(tmp_path):
    for name in ("engine", "seeds", "build", "original", "recovery"):
        (tmp_path / name).mkdir()
    image = pe_fixture(b"\xc3\x33\xc0\xc3")
    (tmp_path / "original/input.exe").write_bytes(image)
    (tmp_path / "original/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest() + "\n")
    (tmp_path / "engine/family.cc").write_text("void f() {}\n")
    (tmp_path / "build/commands.json").write_text(json.dumps([
        {"directory": str(tmp_path), "file": "engine/family.cc", "arguments": ["compiler", "-c", "engine/family.cc"]}]))
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "original/PIN", "address_space": "pe:va",
                   "roots": ["engine", "seeds"], "extensions": [".cc"],
                   "seed_roots": ["seeds"], "include_paths": ["engine"]},
        "compiler": {"command": {"argv": [sys.executable, "-c", "raise SystemExit(99)"], "timeout": 5},
                     "inputs": [], "environment": []},
        "measurement": {"backend": "pe-coff-i386", "compiler_profile": "fixture",
                        "flag_sets": [[]], "layout_flags": [], "unit_extensions": [".cc"],
                        "inputs": [], "input_roots": []},
        "mappings": {"compile_commands": "build/commands.json", "forwarder_macros": []},
    }))
    subprocess.run(["git", "init", "-b", "main"], cwd=tmp_path, check=True, capture_output=True)
    subprocess.run(["git", "config", "user.name", "Component control"], cwd=tmp_path, check=True)
    subprocess.run(["git", "config", "user.email", "component@example.test"], cwd=tmp_path, check=True)
    git(tmp_path, "add", ".")
    git(tmp_path, "commit", "-m", "Component test baseline")
    return tmp_path


def _manifest(image, *, code=b"\xc3", component_id="runtime", unit="runtime.obj"):
    image_pin = hashlib.sha256(image).hexdigest()
    origin = Origin(image_pin, "pe:va", 0x401000)
    binding = VendorBinding(origin, (Span(0x401000, 0x401001),), "_f", unit,
                            "pinned compiler-library member and exact original span")
    raw = archive([(unit + "/", coff_fixture(code))])
    manifest = ToolchainManifest(
        Component(component_id, Ownership.RUNTIME, Delivery.TOOLCHAIN,
                  ("synthetic exact member ownership",), "fixture"),
        hashlib.sha256(raw).hexdigest(), "synthetic archive provenance", (binding,))
    return manifest, raw


def _companion_text(manifest, marker="// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001 BYTE_EXACT\n"):
    # Produce the marker through the installed reader/writer grammar, then
    # retain the existing component manifest in the same companion.
    records = read_text(marker, "component.recovery.h") if marker else []
    return (write(marker, records) if records else "") + render_component(manifest)


def _install_component(root, manifest, raw, *, marker=None):
    component = root / "engine/runtime.recovery.h"
    component.write_text(_companion_text(manifest, marker or
                                         "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001 BYTE_EXACT\n"))
    (root / "runtime.lib").write_bytes(raw)
    project = root / "recovery/project.json"
    value = json.loads(project.read_text())
    value["source"]["extensions"] = list(dict.fromkeys([*value["source"].get(
        "extensions", [".cpp", ".c", ".h", ".cc", ".cxx", ".hpp", ".hh"]), ".h"]))
    value["components"] = {"toolchain_archives": {manifest.component.component_id: "runtime.lib"}}
    project.write_text(json.dumps(value))
    return component


def test_exact_same_companion_mapping_is_the_only_unbuilt_host_route(component_route_project):
    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    manifest, raw = _manifest(image)
    _install_component(root, manifest, raw)
    (root / "seeds/old.cc").write_text(
        "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001 BYTE_EXACT\nvoid old_seed() {}\n")
    (root / "engine/family.cc").write_text("void source_without_original_marker() {}\n")
    view, snapshot = project_view(root)
    assert view["entity_count"] == 1
    group, = view["groups"]
    assert group["selector"] == "component:runtime"
    entity, = group["entities"]
    assert len(entity["candidates"]) == 2
    assert sum(row["component"] == "runtime" for row in entity["candidates"]) == 1
    assert sum(row["seed"] for row in entity["candidates"]) == 1
    report = check_mappings(root)
    assert report["result"] == "pass" and len(report["seed_alternatives"]) == 1
    owners = exact_component_record_bindings(snapshot)
    assert list(owners) == [("engine/runtime.recovery.h", entity["entity_id"])]


@pytest.mark.parametrize("damage", ["wrong-origin", "wrong-span", "wrong-companion", "missing-binding"])
def test_component_binding_does_not_exempt_mismatched_header_or_duplicate_candidate(component_route_project, damage):
    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    manifest, raw = _manifest(image)
    marker = "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001 BYTE_EXACT\n"
    if damage == "wrong-origin":
        binding = replace(manifest.bindings[0], origin=replace(manifest.bindings[0].origin, address=0x401001),
                          spans=(Span(0x401001, 0x401002),))
        manifest = replace(manifest, bindings=(binding,))
    elif damage == "wrong-span":
        binding = replace(manifest.bindings[0], spans=(Span(0x401000, 0x401002),))
        manifest = replace(manifest, bindings=(binding,))
    elif damage == "wrong-companion":
        manifest, raw = _manifest(image, component_id="runtime", unit="runtime.obj")
    elif damage == "missing-binding":
        other, _ = _manifest(image, component_id="other", unit="other.obj")
        binding = replace(other.bindings[0], origin=replace(other.bindings[0].origin, address=0x401001),
                          spans=(Span(0x401001, 0x401002),))
        manifest = replace(other, bindings=(binding,))
    _install_component(root, manifest, raw, marker="" if damage == "wrong-companion" else marker)
    if damage == "wrong-companion":
        (root / "engine/other.h").write_text(marker)
    (root / "engine/family.cc").write_text("void source_without_original_marker() {}\n")
    report = check_mappings(root)
    assert report["result"] == "fail"
    assert any(row["rule"] == "unbuilt_host" for row in report["findings"])


def test_conflicting_second_component_owner_is_not_collapsed(component_route_project):
    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    first, raw = _manifest(image)
    _install_component(root, first, raw)
    second = replace(first, component=replace(first.component, component_id="runtime-two"))
    (root / "engine/other.recovery.h").write_text(render_component(second))
    (root / "engine/family.cc").write_text("void source_without_original_marker() {}\n")
    with pytest.raises(ValueError, match="original binding owned by multiple components"):
        check_mappings(root)


def test_component_measurement_uses_real_existing_backend_and_keeps_provenance(component_route_project):
    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    manifest, raw = _manifest(image)
    _install_component(root, manifest, raw)
    (root / "engine/family.cc").write_text("void f() {}\n")
    snapshot = source_snapshot(root)
    owners = exact_component_record_bindings(snapshot)
    rows, checks = component_function_observations(root, root / "original/input.exe", owners)
    row, = rows.values()
    assert checks[0]["result"] == "pass"
    assert row["measurement_backend"] == "component_binary"
    assert component_claim_result(row) == ("pass", "BYTE_EXACT")
    result = measure_source(root, ("_f",), image=Path("original/input.exe"))
    measured, = result["measurements"]
    assert result["result"] == "pass" and result["translation_units"] == result["compile_attempts"] == 0
    assert measured["measurement_backend"] == "component_binary"
    assert measured["component"] == "runtime" and measured["provenance"] == "verified_component_mapping"
    # The normal product compiler adapter remains unused for this header claim.
    assert result["compile_attempts"] == 0

    check = check_matches(root, Path("original/input.exe"))
    assert check["result"] == "pass" and check["compile_attempts"] == 0
    assert check["measurements"][0]["measurement_backend"] == "component_binary"


def test_component_route_accepts_explicit_absolute_archive_and_refuses_relative_escape(
        component_route_project):
    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    manifest, raw = _manifest(image)
    _install_component(root, manifest, raw)
    config_path = root / "recovery/project.json"
    config = json.loads(config_path.read_text())
    config["components"]["toolchain_archives"]["runtime"] = str((root / "runtime.lib").resolve())
    config_path.write_text(json.dumps(config))
    snapshot = source_snapshot(root)
    owners = exact_component_record_bindings(snapshot)
    rows, checks = component_function_observations(root, root / "original/input.exe", owners)
    assert checks[0]["result"] == "pass"
    assert component_claim_result(rows[manifest.bindings[0].origin.entity_id]) == ("pass", "BYTE_EXACT")

    config["components"]["toolchain_archives"]["runtime"] = "../runtime.lib"
    config_path.write_text(json.dumps(config))
    snapshot = source_snapshot(root)
    owners = exact_component_record_bindings(snapshot)
    with pytest.raises(ValueError, match="normalized relative path"):
        component_function_observations(root, root / "original/input.exe", owners)


def test_component_claim_requires_each_explicit_proof_dimension():
    all_pass = {"bytes": "pass", "boundary": "pass", "references": "pass"}
    assert component_claim_result(all_pass) == ("pass", "BYTE_EXACT")
    for missing in ("bytes", "boundary", "references"):
        partial = dict(all_pass)
        del partial[missing]
        assert component_claim_result(partial) == ("unknown", "UNRESOLVED")
        partial[missing] = None
        assert component_claim_result(partial) == ("unknown", "UNRESOLVED")
        partial[missing] = "unknown"
        assert component_claim_result(partial) == ("unknown", "UNRESOLVED")
        partial[missing] = "fail"
        assert component_claim_result(partial) == ("fail", "MISMATCH")


def test_toolchain_archive_prerequisite_gates_rows_but_other_function_failure_does_not(
        component_route_project, monkeypatch):
    import match_recovery.component_commands as commands

    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    manifest, raw = _manifest(image)
    _install_component(root, manifest, raw)
    owners = {manifest.bindings[0].origin.entity_id:
              (manifest.component.component_id, manifest, manifest.bindings[0])}

    def synthetic_report(archive_result, functions):
        return 0, {"result": "fail" if archive_result == "fail" or any(
            item.get("result") == "fail" for item in functions) else "pass",
            "check": "toolchain_binary_references",
            "archive": {"result": archive_result}, "functions": functions}

    passing_row = {"entity_id": manifest.bindings[0].origin.entity_id,
                   "bytes": "pass", "boundary": "pass", "references": "pass",
                   "result": "pass"}
    monkeypatch.setattr(commands, "execute", lambda *_args, **_kwargs:
                        synthetic_report("fail", [passing_row]))
    rows, checks = component_function_observations(root, root / "original/input.exe", owners)
    observed = rows[manifest.bindings[0].origin.entity_id]
    assert checks[0]["result"] == "fail"
    assert observed.get("bytes") is None
    assert component_claim_result(observed) == ("fail", "MISMATCH")

    for missing_state in ("unknown", None):
        def missing_archive(*_args, **_kwargs):
            report = {"result": "unknown", "check": "toolchain_binary_references",
                      "functions": [passing_row]}
            if missing_state is not None:
                report["archive"] = {"result": missing_state}
            return 3, report
        monkeypatch.setattr(commands, "execute", missing_archive)
        rows, _ = component_function_observations(root, root / "original/input.exe", owners)
        assert component_claim_result(rows[manifest.bindings[0].origin.entity_id]) == (
            "unknown", "UNRESOLVED")

    unrelated = {"entity_id": "unrelated-original-function", "result": "fail",
                 "bytes": "fail", "boundary": "pass", "references": "pass"}
    monkeypatch.setattr(commands, "execute", lambda *_args, **_kwargs:
                        synthetic_report("pass", [passing_row, unrelated]))
    rows, checks = component_function_observations(root, root / "original/input.exe", owners)
    assert checks[0]["result"] == "fail"
    assert component_claim_result(rows[manifest.bindings[0].origin.entity_id]) == ("pass", "BYTE_EXACT")


def test_missing_archive_or_damaged_component_body_remains_unknown_or_fail(component_route_project):
    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    manifest, raw = _manifest(image)
    _install_component(root, manifest, raw)
    (root / "engine/family.cc").write_text("void f() {}\n")
    project = root / "recovery/project.json"
    value = json.loads(project.read_text())
    value.pop("components")
    project.write_text(json.dumps(value))
    assert measure_source(root, ("_f",), image=root / "original/input.exe")["result"] == "unknown"

    damaged, changed_archive = _manifest(image, code=b"\x90")
    _install_component(root, damaged, changed_archive)
    result = measure_source(root, ("_f",), image=root / "original/input.exe")
    assert result["result"] == "fail"
    assert result["measurements"][0]["measurement_backend"] == "component_binary"


def test_component_route_never_invents_a_missing_member_or_symbol(component_route_project):
    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    manifest, raw = _manifest(image)
    binding = replace(manifest.bindings[0], unit="absent.obj")
    manifest = replace(manifest, bindings=(binding,))
    _install_component(root, manifest, raw)
    snapshot = source_snapshot(root)
    owners = exact_component_record_bindings(snapshot)
    rows, checks = component_function_observations(root, root / "original/input.exe", owners)
    row, = rows.values()
    assert checks[0]["result"] == "unknown"
    assert component_claim_result(row) == ("unknown", "UNRESOLVED")


def test_recording_does_not_route_component_header_through_measurement(component_route_project):
    from match_recovery.recording import record

    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    manifest, raw = _manifest(image)
    component = _install_component(root, manifest, raw)
    before = component.read_bytes()
    (root / ".gitignore").write_text("build/recovery/publications/\n")
    project = root / "recovery/project.json"
    configuration = json.loads(project.read_text())
    configuration["source_history"] = {
        "baseline_revision": git(root, "rev-parse", "HEAD").strip(),
        "evidence": ["recording refusal control baseline"],
    }
    project.write_text(json.dumps(configuration))
    git(root, "add", ".gitignore")
    git(root, "add", "recovery/project.json")
    git(root, "commit", "-m", "Configure ignored recording staging")
    with pytest.raises(ValueError, match="selected mapping host is not a configured translation unit"):
        record(root, ("_f",), image=Path("original/input.exe"))
    assert component.read_bytes() == before
    assert not (root / "build/recovery/publications").exists()


def test_vendor_manifest_route_requires_a_physical_function_in_its_named_unit(component_route_project):
    root = component_route_project
    image = (root / "original/input.exe").read_bytes()
    pin = hashlib.sha256(image).hexdigest()
    origin = Origin(pin, "pe:va", 0x401000)
    binding = VendorBinding(origin, (Span(0x401000, 0x401001),), "_f", "f.c",
                            "source unit and original function boundary evidence")
    manifest = VendorManifest(Component("zlib-fixture", Ownership.THIRD_PARTY,
                                        Delivery.UPSTREAM_SOURCE, ("synthetic source identity",), "fixture"),
                              "vendor", "https://invalid.test/source.tar", "a" * 64, "LICENSE",
                              (SourceFile("f.c", "f.c"), SourceFile("LICENSE", "LICENSE")),
                              "c", ("f.c",), (), (binding,))
    (root / "vendor").mkdir()
    (root / "vendor/f.c").write_text("int f(void) { return 0; }\n")
    (root / "vendor/LICENSE").write_text("synthetic test license\n")
    script = root / "recovery/compiler.py"
    encoded = base64.b64encode(coff_fixture(b"\xc3")).decode()
    script.write_text("import json\nprint(json.dumps({'object_base64': " + repr(encoded) + "}))\n")
    project = root / "recovery/project.json"
    value = json.loads(project.read_text())
    value["compiler"] = {"command": {"argv": [sys.executable, "recovery/compiler.py"], "timeout": 5},
                         "inputs": ["recovery/compiler.py"], "environment": []}
    value["source"]["extensions"].append(".h")
    project.write_text(json.dumps(value))
    marker = "// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001 BYTE_EXACT\n"
    (root / "engine/zlib.recovery.h").write_text(_companion_text(manifest, marker=marker))
    snapshot = source_snapshot(root)
    owners = exact_component_record_bindings(snapshot)
    rows, checks = component_function_observations(root, root / "original/input.exe", owners)
    row, = rows.values()
    assert checks[0]["result"] == row["result"] == "pass"
    assert row["measurement_backend"] == "component_binary"
    assert component_claim_result(row) == ("pass", "BYTE_EXACT")

    wrong = replace(binding, symbol="_inline_or_folded_without_a_coff_definition")
    broken = replace(manifest, bindings=(wrong,))
    (root / "engine/zlib.recovery.h").write_text(_companion_text(broken, marker=marker))
    owners = exact_component_record_bindings(source_snapshot(root))
    rows, checks = component_function_observations(root, root / "original/input.exe", owners)
    row, = rows.values()
    assert checks[0]["result"] == "unknown"
    assert component_claim_result(row) == ("unknown", "UNRESOLVED")


def test_claim_identity_survives_seed_to_component_companion_move(tmp_path):
    from match_recovery.claim_integrity import check_claims

    for name in ("engine", "seeds", "original", "recovery"):
        (tmp_path / name).mkdir()
    subprocess.run(["git", "init", "-b", "main"], cwd=tmp_path, check=True, capture_output=True)
    subprocess.run(["git", "config", "user.name", "Component control"], cwd=tmp_path, check=True)
    subprocess.run(["git", "config", "user.email", "component@example.test"], cwd=tmp_path, check=True)
    (tmp_path / "original/PIN").write_text("sha256 " + "a" * 64 + "\n")
    marker = ("// RECOVERY_ROLE: SCAFFOLD compiler shim delegates to the runtime archive\n"
              "// ORIGINAL: 0x00000010 _first 0x00000010-0x00000018 BYTE_EXACT\n"
              "// LEVER: call-layout retain the original call sequence\n"
              "// TRIED: indirect owner was a discarded hypothesis\n"
              "// DEFERRED: final runtime selection needs separate evidence\n"
              "int first() { return 1; }\n")
    source = tmp_path / "seeds/first.cc"
    source.write_text(marker)
    config = {"schema_version": 1,
              "source": {"image_pin": "original/PIN", "address_space": "pe:va",
                         "roots": ["engine", "seeds"], "extensions": [".cc"],
                         "seed_roots": ["seeds"]}}
    (tmp_path / "recovery/project.json").write_text(json.dumps(config))
    baseline = commit(tmp_path, "Seed original claim")
    config["source"]["extensions"].append(".h")
    config["source_history"] = {"baseline_revision": baseline,
                                "evidence": ["Initial source-obligation baseline"]}
    (tmp_path / "recovery/project.json").write_text(json.dumps(config))
    commit(tmp_path, "Declare claim history")
    marker = marker.replace("int first() { return 1; }\n", "")
    pin = "a" * 64
    origin = Origin(pin, "pe:va", 0x10)
    binding = VendorBinding(origin, (Span(0x10, 0x18),), "_first", "runtime.obj",
                            "synthetic component mapping evidence")
    raw = archive([("runtime.obj/", coff_fixture(b"\xc3"))])
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
                                           ("synthetic",), "fixture"),
                                 hashlib.sha256(raw).hexdigest(), "archive evidence", (binding,))
    component = tmp_path / "engine/runtime.recovery.h"
    component.write_text(_companion_text(manifest, marker=marker))
    source.unlink()
    commit(tmp_path, "Move original annotated owner to component companion")
    report = check_claims(tmp_path)
    assert report["result"] == "pass"
    assert report["historical_requirements"] == 1 and report["claims"] == 1
    transferred = component.read_text()
    assert "RECOVERY_ROLE: SCAFFOLD compiler shim delegates to the runtime archive" in transferred
    assert "LEVER: call-layout retain the original call sequence" in transferred
    assert "TRIED: indirect owner was a discarded hypothesis" in transferred
    assert "DEFERRED: final runtime selection needs separate evidence" in transferred
    component.write_text(component.read_text().replace(" BYTE_EXACT", ""))
    assert check_claims(tmp_path)["result"] == "fail"
