"""Whole ordinary COFF code contributions; never function-entry recovery."""

from dataclasses import replace
import hashlib
import json
import struct

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, render_component
from match_recovery.components import (AbsoluteSymbolBinding, ToolchainBinding,
                                       ToolchainCodeSectionBinding, ToolchainManifest)
from match_recovery.model import Component, Delivery, Origin, Ownership, Span, canonical
from test_binary import coff_fixture
from test_toolchain_components import archive, pe_fixture


UNIT = "runtime.obj"


def coff_code(code, *, relocation=None, symbol_name="_local", symbol_section=1,
              symbol_storage=3, symbol_value=0, section_flags=0x60500020):
    relocations = b"" if relocation is None else struct.pack("<IIH", relocation[0], 0,
                                                              relocation[1])
    symbol = (symbol_name.encode("ascii").ljust(8, b"\0")
              + struct.pack("<IhHBB", symbol_value, symbol_section, 0,
                            symbol_storage, 0))
    symbol_at = 60 + len(code) + len(relocations)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbol_at, 1, 0, 0)
    section = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(code), 60,
               60 + len(code) if relocation else 0, 0, int(relocation is not None), 0,
               section_flags))
    return header + section + code + relocations + symbol + struct.pack("<I", 4)


def case(*, code=b"\xc3", candidate=None, relocation=None, symbol_name="_local",
         symbol_section=1, symbol_storage=3, symbol_value=0, original_site=None,
         section_flags=0x60500020):
    candidate = code if candidate is None else candidate
    image = bytearray(pe_fixture(code, absolute_site=original_site))
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    image = bytes(image)
    obj = coff_code(candidate, relocation=relocation, symbol_name=symbol_name,
                    symbol_section=symbol_section, symbol_storage=symbol_storage,
                    symbol_value=symbol_value, section_flags=section_flags)
    raw = archive([(UNIT + "/", obj)])
    pin = hashlib.sha256(image).hexdigest()
    section = ToolchainCodeSectionBinding(UNIT, 1, ".text", Origin(pin, "pe:va", 0x401000),
                                          Span(0x401000, 0x401000 + len(code)),
                                          "independent whole-section boundary")
    manifest = ToolchainManifest(Component("runtime-code", Ownership.RUNTIME, Delivery.TOOLCHAIN,
                                ("synthetic test component",), "test"),
                 hashlib.sha256(raw).hexdigest(), "synthetic archive", code_sections=(section,))
    return manifest, image, raw


def test_code_only_manifest_roundtrips_and_pass_does_not_create_function_coverage():
    manifest, image, raw = case()
    assert ToolchainManifest.from_dict(json.loads(canonical(manifest.to_dict()))) == manifest
    rendered = render_component(manifest)
    assert parse_component(rendered) == manifest
    assert '"bindings": [' in rendered and '"code_sections": [' in rendered
    report = verify_toolchain(manifest, image, raw)
    assert report["result"] == "pass"
    assert report["functions"] == [] and report["counts"] == {}
    assert report["code_section_counts"] == {"pass": 1}
    assert report["code_sections"][0]["callable"] is False
    assert report["code_sections"][0]["link_target"] is False
    assert report["application_recoveries"] == 0
    assert report["link_verification"] == report["runtime_verification"] == "unknown"
    assert report["ready_for_collection"] is False


def test_internal_same_member_relocation_uses_only_the_explicit_section_placement():
    original = b"\xe8\xfb\xff\xff\xff\xc3"
    candidate = b"\xe8\0\0\0\0\xc3"
    manifest, image, raw = case(code=original, candidate=candidate,
                               relocation=(1, 20), symbol_name="_local")
    report = verify_toolchain(manifest, image, raw)
    row = report["code_sections"][0]
    assert row["result"] == row["references"] == "pass"
    assert row["relocations"][0]["symbol"] == "_local"
    assert row["relocations"][0]["result"] == "pass"
    assert report["functions"] == []


def test_absolute_relocation_requires_exact_original_highlow_shape():
    original = b"\xa1\x00\x10\x40\x00\xc3"
    candidate = b"\xa1\0\0\0\0\xc3"
    manifest, image, raw = case(code=original, candidate=candidate,
                               relocation=(1, 6), original_site=1)
    row = verify_toolchain(manifest, image, raw)["code_sections"][0]
    assert row["result"] == row["references"] == "pass"
    assert row["highlow_offsets"] == [1]


@pytest.mark.parametrize("original_site", [None, 2])
def test_missing_or_misplaced_highlow_site_fails_code_reference_check(original_site):
    original = b"\xa1\x00\x10\x40\x00\xc3"
    candidate = b"\xa1\0\0\0\0\xc3"
    manifest, image, raw = case(code=original, candidate=candidate,
                               relocation=(1, 6), original_site=original_site)
    row = verify_toolchain(manifest, image, raw)["code_sections"][0]
    assert row["boundary"] == "pass"
    assert row["nonrelocation_bytes"] == "pass"
    assert row["references"] == row["result"] == "fail"


def test_absolute_constant_requires_coff_section_minus_one_and_exact_value():
    code = b"\xa1\0\0\0\0\xc3"
    manifest, image, raw = case(code=code, relocation=(1, 6), symbol_name="_abs",
                               symbol_section=-1, symbol_storage=2)
    manifest = replace(manifest, absolute_symbols=(AbsoluteSymbolBinding(
        "_abs", 0, UNIT, "independent linker-constant identity"),))
    report = verify_toolchain(manifest, image, raw)
    assert report["absolute_symbols"][0]["result"] == "pass"
    assert report["code_sections"][0]["result"] == "pass"


@pytest.mark.parametrize("section,value", [(-2, 0), (-1, 1)])
def test_debug_or_conflicting_negative_section_symbol_never_resolves_constant(section, value):
    code = b"\xa1\0\0\0\0\xc3"
    manifest, image, raw = case(code=code, relocation=(1, 6), symbol_name="_abs",
                               symbol_section=section, symbol_storage=2, symbol_value=value)
    manifest = replace(manifest, absolute_symbols=(AbsoluteSymbolBinding(
        "_abs", 0, UNIT, "independent linker-constant identity"),))
    report = verify_toolchain(manifest, image, raw)
    assert report["absolute_symbols"][0]["result"] == "unknown"
    assert report["code_sections"][0]["references"] == "unknown"
    assert report["code_sections"][0]["result"] == "unknown"


@pytest.mark.parametrize("section_flags", [0x60501020, 0x40500020])
def test_comdat_or_nonexecutable_code_sections_are_not_qualified(section_flags):
    manifest, image, raw = case(section_flags=section_flags)
    row = verify_toolchain(manifest, image, raw)["code_sections"][0]
    assert row["result"] == row["boundary"] == "unknown"
    assert "ordinary executable code contribution" in row["reason"]


def test_unresolved_code_reference_remains_unknown_not_a_operand_derived_target():
    original = b"\xe8\xfb\xff\xff\xff\xc3"
    candidate = b"\xe8\0\0\0\0\xc3"
    manifest, image, raw = case(code=original, candidate=candidate, relocation=(1, 20),
                               symbol_name="_missing", symbol_section=0, symbol_storage=2)
    row = verify_toolchain(manifest, image, raw)["code_sections"][0]
    assert row["boundary"] == row["nonrelocation_bytes"] == "pass"
    assert row["references"] == row["result"] == "unknown"


def test_out_of_section_symbol_offsets_do_not_become_targets():
    original = b"\xe8\xfb\xff\xff\xff\xc3"
    candidate = b"\xe8\0\0\0\0\xc3"
    manifest, image, raw = case(code=original, candidate=candidate, relocation=(1, 20),
                               symbol_name="_local", symbol_value=100)
    row = verify_toolchain(manifest, image, raw)["code_sections"][0]
    assert row["boundary"] == row["nonrelocation_bytes"] == "pass"
    assert row["references"] == row["result"] == "unknown"


def test_unknown_code_contribution_does_not_supply_function_reference_targets():
    function_unit, code_unit = "function.obj", "code.obj"
    function_bytes = b"\xe8\x0b\0\0\0\xc3" + b"\xcc" * 10
    code_bytes = b"\xc3"
    image = bytearray(pe_fixture(function_bytes + code_bytes))
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    image = bytes(image)
    function_obj = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20))
    code_obj = coff_code(code_bytes, symbol_name="_g", symbol_storage=3)
    raw = archive([(function_unit + "/", function_obj), (code_unit + "/", code_obj)])
    pin = hashlib.sha256(image).hexdigest()
    function = ToolchainBinding(Origin(pin, "pe:va", 0x401000),
                                (Span(0x401000, 0x401010),), "_f", function_unit,
                                "independent function boundary")
    contribution = ToolchainCodeSectionBinding(code_unit, 1, ".text",
        Origin(pin, "pe:va", 0x401010), Span(0x401010, 0x401011),
        "independent whole-section boundary")
    manifest = ToolchainManifest(Component("runtime-code", Ownership.RUNTIME, Delivery.TOOLCHAIN,
        ("synthetic test component",), "test"), hashlib.sha256(raw).hexdigest(),
        "synthetic archive", (function,), code_sections=(contribution,))
    report = verify_toolchain(manifest, image, raw)
    assert report["code_sections"][0]["result"] == "pass"
    assert report["functions"][0]["references"] == report["functions"][0]["result"] == "unknown"


@pytest.mark.parametrize("damage", ["image_byte", "extent", "member", "section", "digest"])
def test_code_section_damage_fails_closed(damage):
    manifest, image, raw = case()
    section = manifest.code_sections[0]
    if damage == "image_byte":
        changed = bytearray(image)
        changed[512] ^= 1
        pin = hashlib.sha256(changed).hexdigest()
        manifest = replace(manifest, code_sections=(replace(section,
            origin=Origin(pin, "pe:va", section.origin.address)),))
        image = bytes(changed)
    elif damage == "extent":
        manifest = replace(manifest, code_sections=(replace(section,
            span=Span(section.span.start, section.span.end + 1)),))
    elif damage == "member":
        manifest = replace(manifest, code_sections=(replace(section, unit="missing.obj"),))
    elif damage == "section":
        manifest = replace(manifest, code_sections=(replace(section, section_name=".rdata"),))
    else:
        raw += b"changed archive"
    report = verify_toolchain(manifest, image, raw)
    assert report["result"] != "pass"
    assert report["code_sections"][0]["result"] != "pass"


def test_duplicate_or_overlapping_code_placements_are_rejected():
    manifest, _, _ = case()
    section = manifest.code_sections[0]
    with pytest.raises(ValueError, match="duplicate toolchain section placement"):
        replace(manifest, code_sections=(section, section))


def test_code_section_metadata_cannot_alias_a_function_identity():
    manifest, _, _ = case()
    section = manifest.code_sections[0]
    function = ToolchainBinding(section.origin, (section.span,), "_f", UNIT, "test")
    with pytest.raises(ValueError, match="one original identity"):
        replace(manifest, bindings=(function,))

