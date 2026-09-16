"""Freshly verified toolchain components may publish scoped relocation targets."""

import base64
import hashlib
import json

import pytest

from match_recovery.application_binary import select_bindings
from match_recovery.component_source import render_component
from match_recovery.component_targets import component_targets
from match_recovery.records import canonical
from test_toolchain_components import dependency  # noqa: F401  shared pytest fixture


def entity(address, symbol, component=None, seed=False):
    candidate = {"symbol": symbol, "seed": seed, "component": component, "spans": [], "path": "src/a.cpp"}
    return {"entity_id": f"entity-{address}", "origin": {"address": address, "image_sha256": "0" * 64,
                                                         "address_space": "pe:va", "entity_id": f"entity-{address}"},
            "conflicts": False, "candidates": [candidate]}


def test_reserved_names_resolve_only_through_fresh_component_targets():
    view = {"groups": [{"entities": [entity(0x401000, "_product"), entity(0x646178, "_fread", component="vc6-runtime")]}]}
    # Without fresh verification the annotation resolves nothing.
    _, _, externals = select_bindings(view, frozenset())
    assert externals == {"_product": 0x401000}
    # With it, the reserved dependency name resolves at its verified origin.
    _, _, externals = select_bindings(view, frozenset(), component_targets={"_fread": 0x646178})
    assert externals == {"_product": 0x401000, "_fread": 0x646178}


def test_product_conflict_refuses_component_resolution():
    view = {"groups": [{"entities": [
        entity(0x401000, "_fread"),
        entity(0x646178, "_fread", component="vc6-runtime")]}]}
    _, _, externals = select_bindings(view, frozenset(), component_targets={"_fread": 0x646178})
    assert "_fread" not in externals


def test_disagreeing_product_binding_stays_unresolved():
    view = {"groups": [{"entities": [entity(0x646178, "_fread")]}]}
    _, _, externals = select_bindings(view, frozenset(), component_targets={"_fread": 0x645598})
    assert "_fread" not in externals


def _project(tmp_path, manifest, archive_path, image):
    (tmp_path / "recovery").mkdir()
    header = render_component(manifest)
    (tmp_path / "src").mkdir()
    (tmp_path / "src/vendor").mkdir()
    (tmp_path / "src/vendor/test.recovery.h").write_text(header)
    config = {"image_pin": "src/IMAGE_PIN", "address_space": "pe:va",
              "seed_roots": ["src/unrecovered"], "symbol_format": "msvc"}
    (tmp_path / "recovery/project.json").write_text(
        json.dumps({"schema_version": 1, "components": {"toolchain_archives": {"runtime": str(archive_path)}}}))
    snapshot = {"image_sha256": hashlib.sha256(image).hexdigest(), "source_configuration": config,
                "files": {"src/vendor/test.recovery.h":
                          {"content_base64": base64.b64encode(header.encode()).decode()}}}
    return snapshot


def test_component_targets_publish_only_freshly_passing_origins(tmp_path, dependency):
    manifest, image, raw = dependency
    archive = tmp_path / "runtime.lib"
    archive.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, archive, image)
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert targets == {"_f": 0x401000}
    assert len(reports) == 1 and reports[0]["result"] == "pass"


def test_failed_verification_publishes_nothing(tmp_path, dependency):
    from dataclasses import replace
    manifest, image, raw = dependency
    manifest = replace(manifest, archive_sha256="0" * 64)
    archive = tmp_path / "runtime.lib"
    archive.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, archive, image)
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert targets == {}
    assert reports and reports[0]["result"] != "pass"


def test_missing_configuration_publishes_nothing(tmp_path, dependency):
    manifest, image, raw = dependency
    snapshot = _project(tmp_path, manifest, tmp_path / "unused.lib", image)
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1}))
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert targets == {} and reports == []


def test_invalid_configuration_refuses(tmp_path, dependency):
    manifest, image, raw = dependency
    snapshot = _project(tmp_path, manifest, tmp_path / "unused.lib", image)
    (tmp_path / "recovery/project.json").write_text(
        json.dumps({"schema_version": 1, "components": {"toolchain_archives": {"x": ""}}}))
    with pytest.raises(ValueError, match="archive paths"):
        component_targets(tmp_path, snapshot, image)


def test_failed_extent_rows_never_publish(tmp_path, monkeypatch, dependency):
    from match_recovery import component_targets as module
    manifest, image, raw = dependency
    archive = tmp_path / "runtime.lib"
    archive.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, archive, image)
    from match_recovery.model import Result
    def stub(unused_manifest, unused_image, unused_raw):
        return {"result": Result.FAIL, "archive": {"result": Result.PASS},
                "functions": [
                    {"symbol": "_ok", "result": Result.PASS, "address": "0x00401000"},
                    {"symbol": "_deep", "result": Result.UNKNOWN, "address": "0x00401006"},
                    {"symbol": "_wrong", "result": Result.FAIL, "address": "0x00401010"}]}
    monkeypatch.setattr(module, "verify_toolchain", stub)
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert targets == {"_ok": 0x401000, "_deep": 0x401006}
    assert reports[0]["functions"][2]["symbol"] not in targets
