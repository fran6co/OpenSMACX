"""Native, advisory proposal of entry names within verified code contributions."""

from dataclasses import replace
import hashlib
import json
import struct

import pytest

from match_recovery.cli import main
from match_recovery.component_proposals import propose_toolchain_entries
from match_recovery.component_source import render_component
from match_recovery.components import (CodeEntryAnchor, ToolchainCodeSectionBinding,
                                       ToolchainBinding, ToolchainManifest)
from match_recovery.model import (Component, Delivery, Origin, Ownership, Span,
                                  canonical)
from test_imports import DECLARATION
from test_toolchain_components import archive as make_archive
from test_toolchain_public_entries import UNIT, case, member_object


def json_manifest(manifest):
    return json.loads(canonical(manifest.to_dict()))


def test_proposal_adds_complete_external_inventory_as_size_free_anchors():
    manifest, image, archive = case(anchors=())
    proposal = propose_toolchain_entries(manifest, image, archive)
    assert proposal["result"] == "advisory"
    assert proposal["baseline_verification"]["result"] == "pass"
    assert proposal["candidate_verification"]["result"] == "pass"
    assert proposal["changes"]["new_entries"] == 2
    encoded = proposal["candidate_manifest"]
    candidate = ToolchainManifest.from_dict(encoded)
    original = manifest.code_sections[0]
    updated = candidate.code_sections[0]
    assert replace(updated, entries=()) == original
    assert [(item.name, item.offset) for item in updated.entries] == [
        ("_strcpy", 0), ("_strcat", 1)]
    assert all("extent" not in item and "size" not in item
               for item in encoded["code_sections"][0]["entries"])
    assert proposal["candidate_verification"]["code_sections"][0]["result"] == "pass"


def test_proposal_preserves_and_validates_explicit_entries_idempotently():
    manifest, image, archive = case()
    first = propose_toolchain_entries(manifest, image, archive)
    assert first["changes"]["new_entries"] == 0
    assert first["proposals"][0]["decision"] == "preserved_existing_entries"
    assert first["candidate_manifest"] == json_manifest(manifest)
    candidate = ToolchainManifest.from_dict(first["candidate_manifest"])
    second = propose_toolchain_entries(candidate, image, archive)
    assert second["changes"]["new_entries"] == 0
    assert second["candidate_manifest"] == first["candidate_manifest"]


@pytest.mark.parametrize(("symbols", "needle"), [
    ((('_strcpy', 0, 0x20, 2), ('_data', 1, 0, 2)), "non-function"),
    ((('_same', 0, 0x20, 2), ('_same', 1, 0x20, 2)), "duplicate external function names"),
    ((('_strcpy', 0, 0x20, 2), ('_strcat', 2, 0x20, 2)), "outside its code contribution"),
])
def test_incomplete_or_ambiguous_external_inventory_is_skipped(symbols, needle):
    manifest, image, archive = case(symbols=symbols, anchors=())
    proposal = propose_toolchain_entries(manifest, image, archive)
    assert proposal["changes"]["new_entries"] == 0
    assert proposal["proposals"][0]["decision"] == "skipped"
    assert needle in proposal["proposals"][0]["reason"]
    assert proposal["candidate_manifest"] == json_manifest(manifest)


def test_inexact_whole_code_extent_never_gets_entries():
    manifest, image, archive = case(anchors=())
    section = manifest.code_sections[0]
    changed = replace(section, span=replace(section.span, end=section.span.end + 1))
    manifest = replace(manifest, code_sections=(changed,))
    proposal = propose_toolchain_entries(manifest, image, archive)
    assert proposal["changes"]["new_entries"] == 0
    assert proposal["proposals"][0]["decision"] == "skipped"
    assert "boundary" in proposal["proposals"][0]["reason"] or "extent" in proposal["proposals"][0]["reason"]


def test_nonrelocation_byte_mismatch_never_gets_entries():
    manifest, image, archive = case(anchors=())
    changed = bytearray(image)
    changed[512] = 0x90
    changed = bytes(changed)
    section = manifest.code_sections[0]
    updated = replace(section, origin=replace(section.origin,
        image_sha256=hashlib.sha256(changed).hexdigest()))
    manifest = replace(manifest, code_sections=(updated,))
    proposal = propose_toolchain_entries(manifest, changed, archive)
    assert proposal["proposals"][0]["decision"] == "skipped"
    assert proposal["proposals"][0]["nonrelocation_bytes"] == "fail"
    assert proposal["changes"]["new_entries"] == 0


def test_wrong_resolved_rel32_target_is_stopped_by_the_byte_gate():
    from test_binary import pe_fixture
    from test_toolchain_components import archive as make_archive
    from match_recovery.component_binary import verify_toolchain

    original = b"\xe8\x0b\0\0\0\xc3"
    image = bytearray(pe_fixture(original))
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    image = bytes(image)
    code = b"\xe8\0\0\0\0\xc3"
    relocation = struct.pack("<IIH", 1, 0, 20)
    symbols_at = 60 + len(code) + len(relocation)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbols_at, 1, 0, 0)
    section = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(code), 60,
               66, 0, 1, 0, 0x60500020))
    symbol = b"_entry\0\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
    raw = make_archive([("wrong-call.obj/", header + section + code + relocation
                         + symbol + struct.pack("<I", 4))])
    pin = hashlib.sha256(image).hexdigest()
    binding = ToolchainCodeSectionBinding("wrong-call.obj", 1, ".text",
        Origin(pin, "pe:va", 0x401000), Span(0x401000, 0x401006), "bounded code contribution")
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
        ("synthetic test runtime",), "test"), hashlib.sha256(raw).hexdigest(),
        "synthetic archive provenance", code_sections=(binding,))
    baseline = verify_toolchain(manifest, image, raw)["code_sections"][0]
    assert baseline["boundary"] == "pass" and baseline["references"] == "fail"
    proposal = propose_toolchain_entries(manifest, image, raw)
    assert proposal["changes"]["new_entries"] == 0
    assert proposal["proposals"][0]["decision"] == "skipped"
    assert proposal["proposals"][0]["reason"] == "nonrelocation code bytes are not exact"


def test_missing_original_highlow_site_blocks_known_address_entry_proposal():
    from test_binary import pe_fixture
    from test_toolchain_components import archive as make_archive
    from match_recovery.component_binary import verify_toolchain

    original = b"\xa1\0\x10\x40\0\xc3"  # The operand is the local function VA 0x401000.
    image = bytearray(pe_fixture(original))
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    image = bytes(image)
    code = b"\xa1\0\0\0\0\xc3"
    relocation = struct.pack("<IIH", 1, 0, 6)
    symbols_at = 60 + len(code) + len(relocation)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbols_at, 1, 0, 0)
    section = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(code), 60,
               66, 0, 1, 0, 0x60500020))
    symbol = b"_entry\0\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
    raw = make_archive([("wrong-call.obj/", header + section + code + relocation
                         + symbol + struct.pack("<I", 4))])
    pin = hashlib.sha256(image).hexdigest()
    binding = ToolchainCodeSectionBinding("wrong-call.obj", 1, ".text",
        Origin(pin, "pe:va", 0x401000), Span(0x401000, 0x401006), "bounded code contribution")
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
        ("synthetic test runtime",), "test"), hashlib.sha256(raw).hexdigest(),
        "synthetic archive provenance", code_sections=(binding,))
    baseline = verify_toolchain(manifest, image, raw)["code_sections"][0]
    assert baseline["boundary"] == baseline["nonrelocation_bytes"] == "pass", baseline
    assert baseline["references"] == baseline["result"] == "fail"
    assert baseline["bytes"] == "pass"
    proposal = propose_toolchain_entries(manifest, image, raw)
    assert proposal["changes"]["new_entries"] == 0
    assert proposal["proposals"][0]["decision"] == "skipped"
    assert "known whole-contribution" in proposal["proposals"][0]["reason"]


def test_aliases_are_retained_as_distinct_names_at_one_physical_entry():
    manifest, image, archive = case(symbols=(('_strcpy', 0, 0x20, 2),
                                              ('_strcat', 0, 0x20, 2)), anchors=())
    proposal = propose_toolchain_entries(manifest, image, archive)
    entries = proposal["candidate_manifest"]["code_sections"][0]["entries"]
    assert [(item["name"], item["offset"]) for item in entries] == [
        ("_strcat", 0), ("_strcpy", 0)]
    assert proposal["candidate_verification"]["result"] == "pass"
    verified = proposal["candidate_verification"]["code_sections"][0]["entries"]
    assert len(verified) == 2 and {row["address"] for row in verified} == {0x401000}


def test_cross_section_duplicate_external_names_skip_conflicting_proposal():
    from test_binary import pe_fixture
    import struct

    image = bytearray(pe_fixture(b"\xc3\xc3\xc3\xc3"))
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    image = bytes(image)
    raw = make_archive([
        ("first.obj/", member_object((('_same', 0, 0x20, 2),))),
        ("second.obj/", member_object((('_same', 0, 0x20, 2),))),
    ])
    pin = hashlib.sha256(image).hexdigest()
    first = ToolchainCodeSectionBinding("first.obj", 1, ".text", Origin(pin, "pe:va", 0x401000),
        Span(0x401000, 0x401002), "first complete contribution")
    second = ToolchainCodeSectionBinding("second.obj", 1, ".text", Origin(pin, "pe:va", 0x401002),
        Span(0x401002, 0x401004), "second complete contribution")
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
        ("synthetic test runtime",), "test"), hashlib.sha256(raw).hexdigest(),
        "synthetic archive provenance", code_sections=(first, second))
    proposal = propose_toolchain_entries(manifest, image, raw)
    assert [row["decision"] for row in proposal["proposals"]] == ["proposed", "skipped"]
    assert "unique" in proposal["proposals"][1]["reason"]
    assert "entries" not in proposal["candidate_manifest"]["code_sections"][1]


def test_unknown_external_references_remain_unknown_after_entry_proposal():
    original = b"\xe8\xfb\xff\xff\xff\xc3"
    code = b"\xe8\0\0\0\0\xc3"
    import struct
    from test_binary import pe_fixture
    image = bytearray(pe_fixture(original))
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    image = bytes(image)
    relocation = struct.pack("<IIH", 1, 1, 20)
    symbols = (b"_entry\0\0" + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
               + b"_missing" + struct.pack("<IhHBB", 0, 0, 0x20, 2, 0))
    symbols_at = 60 + len(code) + len(relocation)
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbols_at, 2, 0, 0)
    section = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(code), 60,
               66, 0, 1, 0, 0x60500020))
    member = header + section + code + relocation + symbols + struct.pack("<I", 4)
    from test_toolchain_components import archive as make_archive
    raw = make_archive([(UNIT + "/", member)])
    import hashlib
    from match_recovery.components import ToolchainCodeSectionBinding
    from match_recovery.model import Component, Delivery, Origin, Ownership, Span
    pin = hashlib.sha256(image).hexdigest()
    section_binding = ToolchainCodeSectionBinding(UNIT, 1, ".text", Origin(pin, "pe:va", 0x401000),
        Span(0x401000, 0x401006), "whole contribution")
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
        ("synthetic test runtime",), "test"), hashlib.sha256(raw).hexdigest(),
        "synthetic archive provenance", code_sections=(section_binding,))
    proposal = propose_toolchain_entries(manifest, image, raw)
    assert proposal["changes"]["new_entries"] == 1
    assert proposal["candidate_verification"]["result"] == "unknown"
    code_row = proposal["candidate_verification"]["code_sections"][0]
    assert code_row["references"] == code_row["result"] == "unknown"
    assert code_row["entries"][0]["result"] == "unknown"
    assert not code_row["entries"][0]["scoped_reference_target"]


def test_archive_pin_mismatch_skips_and_image_pin_mismatch_refuses():
    manifest, image, archive = case(anchors=())
    wrong_archive = archive + b"x"
    proposal = propose_toolchain_entries(manifest, image, wrong_archive)
    assert proposal["changes"]["new_entries"] == 0
    assert proposal["proposals"][0]["decision"] == "skipped"
    assert proposal["proposals"][0]["reason"] == "pinned archive did not verify"
    with pytest.raises(ValueError):
        propose_toolchain_entries(manifest, image + b"x", archive)


def test_native_inputs_and_import_generator_are_frozen_across_both_checks(monkeypatch):
    manifest, image, archive = case(anchors=())
    image_buffer, archive_buffer = bytearray(image), bytearray(archive)
    from match_recovery import component_proposals
    from match_recovery.import_source import parse_imports

    imports = tuple(parse_imports(DECLARATION, "code/system.h"))
    original = component_proposals.verify_toolchain
    calls = []

    def verify(candidate, image_data, archive_data, *, import_bindings=()):
        frozen_imports = tuple(import_bindings)
        calls.append((image_data, archive_data, frozen_imports))
        report = original(candidate, image_data, archive_data, import_bindings=frozen_imports)
        if len(calls) == 1:
            image_buffer[512] ^= 1
            archive_buffer[-1] ^= 1
        return report

    monkeypatch.setattr(component_proposals, "verify_toolchain", verify)
    proposal = propose_toolchain_entries(manifest, image_buffer, archive_buffer,
                                         import_bindings=iter(imports))
    assert len(calls) == 2
    assert calls[0] == calls[1] == (image, archive, imports)
    assert proposal["candidate_verification"]["result"] == "pass"


def _write_project(root, manifest, image, archive):
    for directory in ("code", "hints", "recovery"):
        (root / directory).mkdir()
    (root / "code/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest() + "\n")
    (root / "code/runtime.recovery.h").write_text(render_component(manifest))
    (root / "original.exe").write_bytes(image)
    (root / "runtime.lib").write_bytes(archive)
    (root / "recovery/project.json").write_text(json.dumps({"schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va",
                   "roots": ["code", "hints"], "seed_roots": ["hints"]}}))


@pytest.mark.parametrize("aliases", [False, True])
def test_public_cli_emits_advisory_manifest_without_writing_source(tmp_path, capsys, aliases):
    options = {"symbols": (('_strcpy', 0, 0x20, 2), ('_strcat', 0, 0x20, 2))} if aliases else {}
    manifest, image, archive = case(anchors=(), **options)
    _write_project(tmp_path, manifest, image, archive)
    before = (tmp_path / "code/runtime.recovery.h").read_bytes()
    code = main(["components", "propose-entries", "runtime", "--root", str(tmp_path),
                 "--image", "original.exe", "--archive", "runtime.lib"])
    report = json.loads(capsys.readouterr().out)
    assert code == 0 and report["result"] == "advisory"
    assert report["candidate_verification"]["result"] == "pass"
    assert (tmp_path / "code/runtime.recovery.h").read_bytes() == before


def test_public_cli_refuses_cross_component_entry_name_collision(tmp_path, capsys):
    manifest, image, archive = case(anchors=())
    _write_project(tmp_path, manifest, image, archive)
    origin = Origin(hashlib.sha256(image).hexdigest(), "pe:va", 0x401100)
    other_binding = ToolchainBinding(origin, (Span(0x401100, 0x401101),),
        "_strcpy", "other.obj", "independent other-component binding")
    other = ToolchainManifest(Component("other-runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
        ("second synthetic runtime",), "test"), hashlib.sha256(b"other archive").hexdigest(),
        "other synthetic archive provenance", bindings=(other_binding,))
    (tmp_path / "code/other.recovery.h").write_text(render_component(other))
    code = main(["components", "propose-entries", "runtime", "--root", str(tmp_path),
                 "--image", "original.exe", "--archive", "runtime.lib"])
    assert code == 3
    assert "candidate conflicts with current component ownership" in capsys.readouterr().err


@pytest.mark.parametrize("target", [
    "code/runtime.recovery.h", "code/system.h", "original.exe", "runtime.lib"])
def test_public_cli_rechecks_source_and_artifact_epochs_before_returning_proposal(
        tmp_path, monkeypatch, capsys, target):
    manifest, image, archive = case(anchors=())
    _write_project(tmp_path, manifest, image, archive)
    (tmp_path / "code/system.h").write_text(DECLARATION)
    from match_recovery import component_proposals
    original = component_proposals.propose_toolchain_entries
    def mutate(*args, **kwargs):
        result = original(*args, **kwargs)
        path = tmp_path / target
        path.write_bytes(path.read_bytes() + b"\n")
        return result
    monkeypatch.setattr(component_proposals, "propose_toolchain_entries", mutate)
    code = main(["components", "propose-entries", "runtime", "--root", str(tmp_path),
                 "--image", "original.exe", "--archive", "runtime.lib"])
    assert code == 3
    assert "changed" in capsys.readouterr().err
