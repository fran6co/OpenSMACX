"""Native source operations need configuration, not a project Python adapter."""

import json
from dataclasses import replace

import pytest

from match_recovery.cli import main
from match_recovery.ownership_source import partition_report
from match_recovery.source_config import SourceConfiguration
from match_recovery.source_inventory import source_snapshot
from match_recovery.component_source import parse_components, render_component
from match_recovery.components import SourceFile, VendorBinding, VendorManifest
from match_recovery.model import Component, Delivery, Origin, Ownership, Span


@pytest.fixture
def configured(tmp_path):
    for name in ("recovery", "code", "hints", "original"):
        (tmp_path / name).mkdir()
    (tmp_path / "original/identity").write_text("sha256 " + "a" * 64 + "\n")
    (tmp_path / "code/owner.cc").write_text(
        '#include "owner.hpp"\n// ORIGINAL: 0x00000010 _owner 0x00000010-0x00000018 BYTE_EXACT\n'
        'void owner() {}\n')
    (tmp_path / "code/owner.hpp").write_text("void owner();\n")
    (tmp_path / "hints/attempt.cc").write_text(
        '// ORIGINAL: 0x00000010 _owner 0x00000010-0x00000018\nvoid owner() {}\n')
    configuration = {"image_pin": "original/identity", "address_space": "custom:offset",
                     "roots": ["code", "hints"], "seed_roots": ["hints"], "include_paths": ["code"]}
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1, "source": configuration}))
    return tmp_path


def test_inventory_and_partition_need_no_adapter_code(configured, capsys):
    before = sorted(path.relative_to(configured) for path in configured.rglob("*"))
    assert main(["inventory", "--root", str(configured)]) == 0
    inventory = json.loads(capsys.readouterr().out)
    assert inventory["annotations"] == 2 and inventory["entities"] == 1
    assert inventory["candidates"] == {"legacy_seed": 1, "source": 1}
    assert main(["partition", "--root", str(configured), "--details"]) == 0
    report = json.loads(capsys.readouterr().out)
    group, = report["groups"]
    assert group["source_homes"] == ["code/owner.cc"]
    origin = group["entities"][0]["origin"]
    assert origin["address_space"] == "custom:offset" and origin["address"] == 16
    detailed = partition_report(configured, group=group["selector"], impact=True)
    assert detailed["groups"][0]["impact"]["root_files"] == ["code/owner.cc", "code/owner.hpp"]
    assert sorted(path.relative_to(configured) for path in configured.rglob("*")) == before


def test_source_selection_is_configured_not_inferred_from_directory_names(configured):
    directory = configured / "code/unrecovered"
    directory.mkdir()
    (directory / "new.cc").write_text('// ORIGINAL: 0x00000020 _new 0x00000020-0x00000028\nvoid f() {}\n')
    snapshot = source_snapshot(configured)
    found = next(row for row in snapshot["records"] if row["path"].endswith("new.cc"))
    assert found["candidate_kind"] == "source"


def test_inventory_body_presence_is_separate_from_historical_matching_and_acceptance(configured, capsys):
    (configured / "code/pending.cc").write_text(
        '// ORIGINAL: 0x00000020 _pending 0x00000020-0x00000028\n'
        'void pending() {\n// BODY GOES HERE.\n}\n')
    assert main(["inventory", "--root", str(configured)]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["source_body_states"] == {"source": {"implemented": 1, "placeholder": 1},
                                             "legacy_seed": {"implemented": 1}}
    assert report["historical_byte_obligations"] == 1
    assert report["body_state_basis"] == "source_text_only_not_recovery_acceptance"
    assert report["verification"] == "not_measured"


def test_configured_readers_do_not_guess_msvc_ownership(configured):
    path = configured / "code/owner.cc"
    path.write_text(path.read_text().replace("_owner", "??0Widget@@QAE@XZ"))
    report = partition_report(configured, details=True)
    assert all(group["selector"] != "class:Widget" for group in report["groups"])


@pytest.mark.parametrize("change", [
    {"image_pin": "../outside"}, {"roots": ["code", "code/nested"]}, {"roots": []},
    {"roots": "code"}, {"seed_roots": ["elsewhere"]}, {"extensions": ["*.cc"]},
    {"symbol_format": "invented"}, {"address_space": ""}, {"surprise": True},
])
def test_invalid_source_configuration_is_refused(change):
    with pytest.raises(ValueError):
        SourceConfiguration.from_dict({"image_pin": "original/pin", "address_space": "custom:offset", **change})


def test_missing_source_root_is_not_silently_dropped(configured):
    (configured / "hints/attempt.cc").unlink()
    (configured / "hints").rmdir()
    with pytest.raises(ValueError, match="source root is missing"):
        source_snapshot(configured)


def test_external_source_or_pin_symlinks_are_refused(configured, tmp_path_factory):
    outside = tmp_path_factory.mktemp("outside-input") / "secret"
    outside.write_text("not a source input")
    source = configured / "code/escaped.cc"
    source.symlink_to(outside)
    with pytest.raises(ValueError, match="outside the worktree"):
        source_snapshot(configured)
    source.unlink()
    pin = configured / "original/identity"
    pin.unlink()
    pin.symlink_to(outside)
    with pytest.raises(ValueError, match="outside the worktree"):
        source_snapshot(configured)


def test_configuration_change_during_read_cannot_return_mixed_evidence(configured, monkeypatch):
    from match_recovery import source_inventory
    parse = source_inventory._parse_file
    def changing(*args):
        result = parse(*args)
        path = configured / "recovery/project.json"
        configuration = json.loads(path.read_text())
        configuration["source"]["address_space"] = "changed:offset"
        path.write_text(json.dumps(configuration))
        return result
    monkeypatch.setattr(source_inventory, "_parse_file", changing)
    with pytest.raises(ValueError, match="inventory changed"):
        source_snapshot(configured)


def vendor_manifest(space="custom:offset"):
    component = Component("codec", Ownership.THIRD_PARTY, Delivery.UPSTREAM_SOURCE, ("synthetic",), "1")
    binding = VendorBinding(Origin("a" * 64, space, 16), (Span(16, 24),), "_owner", "codec.c", "synthetic")
    return VendorManifest(component, "code/vendor", "https://invalid.test/fixture", "b" * 64,
                          "codec.c", (SourceFile("codec.c", "release/codec.c"),), "c",
                          ("codec.c",), ("-O2",), (binding,))


def test_component_claims_use_snapshot_and_configured_address_space(configured):
    path = configured / "code/codec.recovery.h"
    path.write_text(render_component(vendor_manifest()))
    report = partition_report(configured)
    assert report["groups"][0]["selector"] == "component:codec"
    assert report["groups"][0]["next_action"] == "verify_dependency"
    path.write_text(render_component(vendor_manifest("wrong:offset")))
    with pytest.raises(ValueError, match="image/address space"):
        partition_report(configured)


def test_seed_component_text_cannot_override_product_ownership(configured):
    (configured / "hints/untrusted.recovery.h").write_text(render_component(vendor_manifest()))
    assert partition_report(configured)["groups"][0]["selector"] == "source:code/owner.cc"


def test_component_ownership_validation_spans_all_configured_roots():
    manifest = vendor_manifest()
    first = render_component(manifest)
    with pytest.raises(ValueError, match="duplicate component"):
        parse_components({"code/codec.recovery.h": first, "other/codec.recovery.h": first})
    other = render_component(replace(manifest, component=replace(manifest.component, component_id="other")))
    with pytest.raises(ValueError, match="multiple components"):
        parse_components({"code/codec.recovery.h": first, "other/codec.recovery.h": other})
