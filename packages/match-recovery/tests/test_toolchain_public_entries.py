"""Size-free public entry anchors on fully verified ordinary contributions."""

from dataclasses import replace
import base64
import hashlib
import json
import struct

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, parse_component_entries, render_component
from match_recovery.component_targets import component_targets
from match_recovery.application_binary import select_bindings
from match_recovery.components import (CodeEntryAnchor, ToolchainCodeSectionBinding,
                                       ToolchainBinding, ToolchainManifest)
from match_recovery.model import Component, Delivery, Origin, Ownership, Span, canonical
from match_recovery import ownership_source
from test_component_targets import _project
from test_toolchain_components import archive
from test_binary import pe_fixture


UNIT = "../strcat.obj"
CODE = b"\xc3\xc3"


def member_object(symbols=(('_strcpy', 0, 0x20, 2), ('_strcat', 1, 0x20, 2)),
                  code=CODE):
    symbols_at = 60 + len(code)
    records = b"".join(name.encode().ljust(8, b"\0")
                       + struct.pack("<IhHBB", offset, 1, kind, storage, 0)
                       for name, offset, kind, storage in symbols)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbols_at, len(symbols), 0, 0)
    section = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(code), 60,
               0, 0, 0, 0, 0x60500020))
    return header + section + code + records + struct.pack("<I", 4)


def case(*, symbols=(('_strcpy', 0, 0x20, 2), ('_strcat', 1, 0x20, 2)),
         anchors=None, code=CODE):
    image = bytearray(pe_fixture(code))
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    image = bytes(image)
    raw = archive([(UNIT + "/", member_object(symbols, code))])
    pin = hashlib.sha256(image).hexdigest()
    if anchors is None:
        anchors = (CodeEntryAnchor("_strcpy", 0, "COFF function symbol inventory"),
                   CodeEntryAnchor("_strcat", 1, "COFF function symbol inventory"))
    section = ToolchainCodeSectionBinding(UNIT, 1, ".text", Origin(pin, "pe:va", 0x401000),
                                          Span(0x401000, 0x401000 + len(code)),
                                          "whole ordinary contribution", tuple(anchors))
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
                                ("synthetic test runtime",), "test"),
                 hashlib.sha256(raw).hexdigest(), "synthetic archive provenance",
                 code_sections=(section,))
    return manifest, image, raw


def test_entry_schema_source_roundtrip_is_size_free_and_legacy_empty_is_stable():
    manifest, _, _ = case()
    encoded = json.loads(canonical(manifest.to_dict()))
    assert ToolchainManifest.from_dict(encoded) == manifest
    assert parse_component(render_component(manifest)) == manifest
    section = encoded["code_sections"][0]
    assert section["entries"][1] == {"name": "_strcat", "offset": 1,
                                    "evidence": "COFF function symbol inventory"}
    assert not {"size", "span", "extent"} & section["entries"][1].keys()

    legacy, _, _ = case(anchors=())
    legacy_dict = legacy.to_dict()
    assert "entries" not in legacy_dict["code_sections"][0]
    assert ToolchainManifest.from_dict(json.loads(canonical(legacy_dict))) == legacy


def test_verified_entries_are_separate_from_parent_and_function_counts():
    manifest, image, raw = case()
    report = verify_toolchain(manifest, image, raw)
    parent = report["code_sections"][0]
    assert report["result"] == "pass"
    assert report["functions"] == [] and report["counts"] == {}
    assert report["code_section_counts"] == {"pass": 1}
    assert report["code_entry_counts"] == {"pass": 2}
    assert parent["callable"] is parent["link_target"] is False
    assert [(row["symbol"], row["address"]) for row in parent["entries"]] == [
        ("_strcpy", 0x401000), ("_strcat", 0x401001)]
    for row in parent["entries"]:
        assert row["result"] == "pass"
        assert row["callable"] is row["scoped_reference_target"] is True
        assert row["link_target"] is False and row["function_extent"] == "unknown"


@pytest.mark.parametrize("alias", [False, True])
def test_unknown_parent_reference_never_promotes_preflight_entry_anchor(alias):
    original = b"\xe8\xfb\xff\xff\xff\xc3"
    code = b"\xe8\0\0\0\0\xc3"
    image = bytearray(pe_fixture(original))
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    image = bytes(image)
    relocation = struct.pack("<IIH", 1, 1, 20)
    symbols = (b"_entry\0\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
               + b"_missing" + struct.pack("<IhHBB", 0, 0, 0x20, 2, 0))
    if alias:
        symbols += b"_alias\0\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
    symbols_at = 60 + len(code) + len(relocation)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbols_at, 2 + int(alias), 0, 0)
    section_header = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(code), 60,
                      66, 0, 1, 0, 0x60500020))
    obj = header + section_header + code + relocation + symbols + struct.pack("<I", 4)
    raw = archive([(UNIT + "/", obj)])
    pin = hashlib.sha256(image).hexdigest()
    anchors = (CodeEntryAnchor("_entry", 0, "symbol"),)
    if alias:
        anchors += (CodeEntryAnchor("_alias", 0, "physical alias"),)
    binding = ToolchainCodeSectionBinding(UNIT, 1, ".text", Origin(pin, "pe:va", 0x401000),
        Span(0x401000, 0x401006), "whole contribution", anchors)
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
        ("synthetic test runtime",), "test"), hashlib.sha256(raw).hexdigest(),
        "synthetic archive provenance", code_sections=(binding,))
    row = verify_toolchain(manifest, image, raw)["code_sections"][0]
    assert row["entry_inventory"] == "pass"
    assert row["boundary"] == "pass" and row["nonrelocation_bytes"] == "pass"
    assert row["references"] == row["result"] == "unknown"
    assert all(entry["result"] == "unknown" and entry["address"] is None
               and not entry["scoped_reference_target"] for entry in row["entries"])


@pytest.mark.parametrize("symbols", [
    (('_strcpy', 0, 0x20, 2), ('_other', 1, 0x20, 2)),  # missing declared name
    (('_strcpy', 0, 0x20, 2), ('_strcat', 0, 0x20, 2)),  # wrong offset
    (('_strcpy', 0, 0x20, 2), ('_strcat', 1, 0, 2)),  # wrong COFF type
    (('_strcpy', 0, 0x20, 2), ('_strcat', 1, 0x30, 2)),  # derived/non-function type
    (('_strcpy', 0, 0x20, 2), ('_strcat', 1, 0x20, 3)),  # static, not external
    (('_strcpy', 0, 0x20, 2), ('_strcat', 1, 0x20, 2), ('_strcat', 1, 0x20, 2)),
    (('_strcpy', 0, 0x20, 2), ('_strcat', 1, 0x20, 2), ('_strcat', 0, 0x20, 2)),
])
def test_actual_malformed_coff_member_never_qualifies_anchors(symbols):
    manifest, image, raw = case(symbols=symbols)
    report = verify_toolchain(manifest, image, raw)
    row = report["code_sections"][0]
    assert row["result"] == "pass"  # parent bytes remain independently measurable
    assert row["entry_inventory"] == "fail"
    assert all(entry["result"] == "fail" and entry["address"] is None
               for entry in row["entries"])
    assert report["result"] == "fail" and report["code_entry_counts"] == {"fail": 2}


@pytest.mark.parametrize("anchor", [
    {"name": "_x", "offset": True, "evidence": "test"},
])
def test_anchor_rejects_boolean_and_out_of_range_offsets(anchor):
    with pytest.raises(ValueError):
        CodeEntryAnchor(**anchor)


def test_anchor_rejects_out_of_range_offset_at_section_boundary():
    with pytest.raises(ValueError, match="outside the contribution span"):
        case(anchors=(CodeEntryAnchor("_x", 2, "test"),))


def test_anchor_names_are_unique_within_section():
    with pytest.raises(ValueError, match="names must be unique"):
        case(anchors=(CodeEntryAnchor("_x", 0, "a"), CodeEntryAnchor("_x", 1, "b")))


def test_actual_same_address_aliases_qualify_only_as_scoped_entries(tmp_path):
    anchors = (CodeEntryAnchor("_strcpy", 0, "first public name"),
               CodeEntryAnchor("_strcat", 0, "same-address public alias"))
    manifest, image, raw = case(symbols=(('_strcpy', 0, 0x20, 2),
                                         ('_strcat', 0, 0x20, 2)), anchors=anchors)
    encoded = render_component(manifest)
    assert parse_component_entries({"src/runtime.recovery.h": encoded})[0][1] == manifest
    report = verify_toolchain(manifest, image, raw)
    parent = report["code_sections"][0]
    assert report["result"] == parent["entry_inventory"] == "pass"
    assert report["functions"] == []
    assert {row["address"] for row in parent["entries"]} == {0x401000}
    assert all(row["callable"] and row["scoped_reference_target"]
               and not row["link_target"] and row["function_extent"] == "unknown"
               for row in parent["entries"])
    archive_path = tmp_path / "runtime.lib"
    archive_path.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, archive_path, image)
    targets, _ = component_targets(tmp_path, snapshot, image)
    assert targets == {"_strcpy": 0x401000, "_strcat": 0x401000}
    (tmp_path / "src/IMAGE_PIN").write_text("sha256 " + manifest.code_sections[0].origin.image_sha256 + "\n")
    view, _ = ownership_source.project_view(tmp_path, allow_empty=True)
    selected = frozenset(entity["entity_id"] for group in view["groups"] for entity in group["entities"])
    bindings, refused, unverified = select_bindings(view, selected)
    assert not bindings and not unverified
    assert len(refused) == 1 and "dependency binding" in refused[0]["reason"]
    bindings, refused, resolved = select_bindings(view, selected, component_targets=targets)
    assert not bindings and len(refused) == 1
    assert resolved == targets


@pytest.mark.parametrize("actual,declared", [
    ((('_strcpy', 0, 0x20, 2), ('_strcat', 0, 0x20, 2)), (('_strcpy', 0),)),
    ((('_strcpy', 0, 0x20, 2),), (('_strcpy', 0), ('_strcat', 0))),
    ((('_strcpy', 0, 0x20, 2), ('_strcat', 1, 0x20, 2)), (('_strcpy', 0), ('_strcat', 0))),
])
def test_missing_invented_or_displaced_alias_never_exports_targets(tmp_path, actual, declared):
    manifest, image, raw = case(symbols=actual,
        anchors=tuple(CodeEntryAnchor(name, offset, "claimed alias") for name, offset in declared))
    archive_path = tmp_path / "runtime.lib"
    archive_path.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, archive_path, image)
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert targets == {}
    parent = reports[0]["code_sections"][0]
    assert parent["entry_inventory"] == "fail"
    assert all(row["address"] is None and not row["callable"] for row in parent["entries"])


def test_alias_address_cannot_be_owned_by_another_contribution():
    manifest, _, _ = case()
    second = replace(manifest.code_sections[0], unit="../other.obj",
        origin=Origin(manifest.code_sections[0].origin.image_sha256, "pe:va", 0x401001),
        span=Span(0x401001, 0x401003), entries=(CodeEntryAnchor("_other", 0, "overlapping entry"),))
    with pytest.raises(ValueError, match="spans overlap"):
        replace(manifest, code_sections=manifest.code_sections + (second,))


def test_component_source_rejects_cross_component_entry_name_and_origin_conflicts():
    manifest, _, _ = case()
    second = replace(manifest, component=Component("runtime-copy", Ownership.RUNTIME,
        Delivery.TOOLCHAIN, ("second synthetic component",), "test"))
    sources = {"src/a.recovery.h": render_component(manifest),
               "src/b.recovery.h": render_component(second)}
    with pytest.raises(ValueError, match="duplicate original code-entry identity"):
        parse_component_entries(sources)

    distinct_origin_section = replace(second.code_sections[0], origin=Origin(
        second.code_sections[0].origin.image_sha256, "pe:va", 0x401010),
        span=Span(0x401010, 0x401012))
    distinct_origin = replace(second, code_sections=(distinct_origin_section,))
    sources["src/b.recovery.h"] = render_component(distinct_origin)
    with pytest.raises(ValueError, match="multiple components"):
        parse_component_entries(sources)


def test_malformed_actual_member_is_not_exported_by_component_target_adapter(tmp_path):
    # The archive really contains a different second public function, rather
    # than a mocked verifier row or an inferred address from section bytes.
    manifest, image, raw = case(symbols=(('_strcpy', 0, 0x20, 2), ('_other', 1, 0x20, 2)))
    archive_path = tmp_path / "runtime.lib"
    archive_path.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, archive_path, image)
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert targets == {}
    assert reports[0]["code_sections"][0]["entry_inventory"] == "fail"


def test_actual_multientry_function_row_with_missing_boundary_is_not_exported(tmp_path):
    manifest, image, raw = case(anchors=())
    section = manifest.code_sections[0]
    binding = ToolchainBinding(Origin(section.origin.image_sha256, "pe:va", section.span.start),
                               (Span(section.span.start, section.span.start + 1),),
                               "_strcpy", UNIT, "historical bounded function mapping")
    manifest = replace(manifest, code_sections=(), bindings=(binding,))
    archive_path = tmp_path / "runtime.lib"
    archive_path.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, archive_path, image)
    targets, reports = component_targets(tmp_path, snapshot, image)
    row = reports[0]["functions"][0]
    assert row["result"] == "unknown" and row.get("boundary") != "pass"
    assert targets == {}


def test_component_target_name_conflicts_with_product_or_import_are_removed(tmp_path):
    manifest, image, raw = case()
    archive_path = tmp_path / "runtime.lib"
    archive_path.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, archive_path, image)
    product_origin = Origin(manifest.code_sections[0].origin.image_sha256, "pe:va", 0x402000)
    snapshot["records"] = [{"entity_id": product_origin.entity_id,
                            "origin": {"image_sha256": product_origin.image_sha256,
                                       "address_space": "pe:va", "address": product_origin.address},
                            "path": "src/product.cpp", "candidate_kind": "source",
                            "emitted_symbol_hint": "_strcat", "catalogued_symbol": "_strcat"}]
    targets, _ = component_targets(tmp_path, snapshot, image)
    assert targets == {"_strcpy": 0x401000}

    snapshot["records"] = []
    import_text = "// RECOVERY_IMPORT: v1 _strcat KERNEL32 name:strcat\n"
    snapshot["files"]["src/import.cpp"] = {
        "content_base64": base64.b64encode(import_text.encode()).decode()}
    targets, _ = component_targets(tmp_path, snapshot, image)
    assert targets == {"_strcpy": 0x401000}


@pytest.mark.parametrize("component_marker", [False, True])
def test_partial_product_address_agreement_never_exports_a_target(tmp_path, component_marker):
    manifest, image, raw = case()
    symbol, address = ("_strcpy", 0x401000) if component_marker else ("_strcat", 0x401001)
    if component_marker:
        manifest = replace(manifest, bindings=(ToolchainBinding(
            manifest.code_sections[0].origin, (Span(address, address + 1),),
            symbol, UNIT, "historical component interpretation"),))
    archive_path = tmp_path / "runtime.lib"
    archive_path.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, archive_path, image)
    rows = []
    for location, entry in (("src/vendor/test.recovery.h" if component_marker else
                             "src/agreeing.cpp", address), ("src/conflicting.cpp", 0x402000)):
        origin = Origin(manifest.code_sections[0].origin.image_sha256, "pe:va", entry)
        rows.append({"entity_id": origin.entity_id,
                     "origin": {"image_sha256": origin.image_sha256,
                                "address_space": origin.address_space, "address": entry},
                     "path": location, "candidate_kind": "source",
                     "emitted_symbol_hint": symbol, "catalogued_symbol": symbol,
                     "spans": [[entry, entry + 1]], "invalid_spans": [],
                     "function_fragment": None})
    snapshot["records"] = rows
    targets, _ = component_targets(tmp_path, snapshot, image)
    assert symbol not in targets
    assert targets, "unrelated verified targets must remain available"


def test_ownership_view_adds_only_spanless_entry_candidates_and_fails_source_conflicts(
        tmp_path, monkeypatch):
    manifest, image, _ = case()
    header = render_component(manifest)
    snapshot = {"image_sha256": manifest.code_sections[0].origin.image_sha256,
                "source_configuration": {"image_pin": "src/IMAGE_PIN", "address_space": "pe:va",
                                         "roots": ["src"], "seed_roots": ["src/unrecovered"],
                                         "symbol_format": "none"},
                "records": [],
                "files": {"src/vendor/test.recovery.h": {
                    "content_base64": base64.b64encode(header.encode()).decode()}}}
    monkeypatch.setattr(ownership_source, "source_snapshot",
                        lambda *args, **kwargs: snapshot)
    view, _ = ownership_source.project_view(tmp_path)
    rows = [entity for group in view["groups"] for entity in group["entities"]]
    strcat = next(row for row in rows if row["origin"]["address"] == 0x401001)
    candidate = strcat["candidates"][0]
    assert candidate["symbol"] == "_strcat" and candidate["component"] == "runtime"
    assert candidate["spans"] == ()

    product = {"entity_id": Origin(manifest.code_sections[0].origin.image_sha256,
                                    "pe:va", 0x401001).entity_id,
               "origin": {"image_sha256": manifest.code_sections[0].origin.image_sha256,
                          "address_space": "pe:va", "address": 0x401001},
               "path": "src/product.cpp", "line": 1, "candidate_kind": "source",
               "catalogued_symbol": "_strcat", "emitted_symbol_hint": "_strcat",
               "spans": [], "invalid_spans": [], "function_fragment": None}
    snapshot["records"] = [product]
    with pytest.raises(ValueError, match="product source mapping"):
        ownership_source.project_view(tmp_path)


def test_ownership_view_does_not_duplicate_same_identity_legacy_symbol(tmp_path, monkeypatch):
    manifest, _, _ = case()
    header = render_component(manifest)
    legacy = {"entity_id": Origin(manifest.code_sections[0].origin.image_sha256,
                                  "pe:va", 0x401001).entity_id,
              "origin": {"image_sha256": manifest.code_sections[0].origin.image_sha256,
                         "address_space": "pe:va", "address": 0x401001},
              "path": "src/unrecovered/old.cpp", "line": 1,
              "candidate_kind": "legacy_seed", "catalogued_symbol": "_strcat",
              "emitted_symbol_hint": "_strcat", "spans": [], "invalid_spans": [],
              "function_fragment": None}
    snapshot = {"image_sha256": manifest.code_sections[0].origin.image_sha256,
                "source_configuration": {"image_pin": "src/IMAGE_PIN", "address_space": "pe:va",
                                         "roots": ["src"], "seed_roots": ["src/unrecovered"],
                                         "symbol_format": "none"},
                "records": [legacy],
                "files": {"src/vendor/test.recovery.h": {
                    "content_base64": base64.b64encode(header.encode()).decode()}}}
    monkeypatch.setattr(ownership_source, "source_snapshot",
                        lambda *args, **kwargs: snapshot)
    view, _ = ownership_source.project_view(tmp_path)
    rows = [entity for group in view["groups"] for entity in group["entities"]]
    strcat = next(row for row in rows if row["origin"]["address"] == 0x401001)
    assert len(strcat["candidates"]) == 1


def test_entry_symbol_cannot_overlap_a_different_function_binding():
    manifest, _, _ = case()
    binding_origin = Origin(manifest.code_sections[0].origin.image_sha256, "pe:va", 0x401001)
    from match_recovery.components import ToolchainBinding
    function = ToolchainBinding(binding_origin, (Span(0x401001, 0x401002),),
                                "_strcat_alias", UNIT, "unanchored overlapping extent")
    with pytest.raises(ValueError, match="overlap"):
        replace(manifest, bindings=(function,))


def test_existing_anchored_function_binding_can_overlap_only_its_parent_section():
    manifest, _, _ = case()
    section = manifest.code_sections[0]
    from match_recovery.components import ToolchainBinding
    strcpy = ToolchainBinding(Origin(section.origin.image_sha256, "pe:va", section.span.start),
                              (Span(section.span.start, section.span.start + 1),),
                              "_strcpy", UNIT, "historical bounded function mapping")
    accepted = replace(manifest, bindings=(strcpy,))
    assert accepted.bindings == (strcpy,)
    assert parse_component(render_component(accepted)) == accepted
    unanchored = replace(strcpy, symbol="_unrelated")
    with pytest.raises(ValueError, match="both a function and a section"):
        replace(manifest, bindings=(unanchored,))
