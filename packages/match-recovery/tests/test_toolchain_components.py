"""Synthetic precompiled dependencies: no proprietary library fixtures."""

from dataclasses import replace
import hashlib
import json

import pytest

from match_recovery.cli import main
from match_recovery.coff_archive import CoffArchive
from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, parse_components, render_component
from match_recovery.components import (DataSymbolAnchor, ToolchainBinding,
                                       ToolchainDataSectionBinding, ToolchainManifest,
                                       VendorBinding)
from match_recovery.model import Component, Delivery, Origin, Ownership, Span, canonical
from test_binary import coff_fixture, pe_fixture


def member(name, body):
    header = (name.ljust(16) + "0".ljust(12) + "0".ljust(6) + "0".ljust(6)
              + "0".ljust(8) + str(len(body)).ljust(10) + "`\n").encode("ascii")
    assert len(header) == 60
    return header + body + (b"\n" if len(body) % 2 else b"")


def archive(records):
    return b"!<arch>\n" + b"".join(member(name, body) for name, body in records)


def coff_data_fixture(data, *, data_flags=0xC0400040, data_relocation=False):
    import struct
    code = b"\xa1\0\0\0\0\xc3" + b"\xcc" * 10
    text_reloc = struct.pack("<IIH", 1, 1, 6)
    data_reloc = struct.pack("<IIH", 0, 1, 6) if data_relocation else b""
    data_at, data_reloc_at = 126, 126 + len(data)
    symbols_at = data_reloc_at + len(data_reloc)
    double_name, float_name = b"_DoubleFormat\0", b"_FloatFormat\0"
    strings = struct.pack("<I", 4 + len(double_name) + len(float_name)) + double_name + float_name
    symbols = (b"_f".ljust(8, b"\0") + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0)
               + bytes(4) + struct.pack("<I", 4) + struct.pack("<IhHBB", 0, 2, 0, 3, 0)
               + bytes(4) + struct.pack("<I", 4 + len(double_name))
               + struct.pack("<IhHBB", 24, 2, 0, 3, 0))
    header = struct.pack("<HHIIIHH", 0x14C, 2, 0, symbols_at, 3, 0, 0)
    text_section = b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, 16, 100, 116, 0, 1, 0, 0x60500020)
    data_section = b".data\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, 48, data_at,
                   data_reloc_at if data_relocation else 0, 0, int(data_relocation), 0, data_flags)
    return (header + text_section + data_section + code + text_reloc + data + data_reloc
            + symbols + strings)


def pe_data_fixture(data, *, data_relocation=False):
    import struct
    raw = bytearray(pe_fixture())
    raw.extend(bytes(512))
    struct.pack_into("<H", raw, 0x86, 2)
    at = 0x178 + 40
    raw[at:at + 8] = b".data\0\0\0"
    struct.pack_into("<IIII", raw, at + 8, 512, 0x2000, 512, 1024)
    struct.pack_into("<I", raw, at + 36, 0xC0000040)
    raw[512:518] = b"\xa1" + (0x402000).to_bytes(4, "little") + b"\xc3"
    raw[1024:1072] = data
    blocks = [(0x1000, 0x3001)] + ([(0x2000, 0x3000)] if data_relocation else [])
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1100, 12 * len(blocks))
    for index, (page, entry) in enumerate(blocks):
        struct.pack_into("<IIHH", raw, 768 + index * 12, page, 12, entry, 0)
    return bytes(raw)


def data_component(*, section_bytes=None, data_flags=0xC0400040,
                   data_relocation=False, double_offset=0, image_bytes=None):
    original, section = bytes(range(48)), section_bytes or bytes(range(48))
    image = pe_data_fixture(original) if image_bytes is None else image_bytes
    unit = "release/runtime.obj"
    raw = archive([("//", (unit + "\0").encode()), ("/0", coff_data_fixture(
        section, data_flags=data_flags, data_relocation=data_relocation))])
    pin = hashlib.sha256(image).hexdigest()
    function = VendorBinding(Origin(pin, "pe:va", 0x401000), (Span(0x401000, 0x401006),),
                             "_f", unit, "synthetic original function boundary")
    data = ToolchainDataSectionBinding(unit, 2, ".data", Origin(pin, "pe:va", 0x402000),
             Span(0x402000, 0x402030), (DataSymbolAnchor("_DoubleFormat", double_offset, 3),
             DataSymbolAnchor("_FloatFormat", 24, 3)), "synthetic full data-section placement")
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
             ("Synthetic compiler distribution",), "test"), hashlib.sha256(raw).hexdigest(),
             "Synthetic test fixture", (function,), data_sections=(data,))
    return manifest, image, raw


def test_opaque_archive_member_identity_works_and_vendor_paths_stay_guarded():
    image = pe_fixture(b"\xe8\xfb\xff\xff\xff\xc3")
    unit = "../build/intel/mt_obj/intrncvt.obj"
    raw = archive([("//", (unit + "\0").encode("ascii")), ("/0", coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 0, 20)))])
    pin = hashlib.sha256(image).hexdigest()
    binding = ToolchainBinding(Origin(pin, "pe:va", 0x401000), (Span(0x401000, 0x401006),),
                               "_f", unit, "exact archive member identity")
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
             ("Synthetic runtime",), "test"), hashlib.sha256(raw).hexdigest(), "test license", (binding,))
    assert ToolchainManifest.from_dict(json.loads(canonical(manifest.to_dict()))) == manifest
    legacy = json.loads(canonical(manifest.to_dict()))
    legacy.pop("data_sections")
    assert ToolchainManifest.from_dict(legacy) == manifest
    assert parse_component(render_component(manifest)) == manifest
    assert verify_toolchain(manifest, image, raw)["functions"][0]["result"] == "pass"
    with pytest.raises(ValueError, match="normalized relative path"):
        VendorBinding(binding.origin, binding.spans, "_f", unit, "not a vendor path")


@pytest.mark.parametrize("unit", ["", "../bad\nmember.obj", "obj/\x7fbad.obj", "café.obj"])
def test_opaque_archive_member_rejects_empty_control_and_nonascii(unit):
    with pytest.raises(ValueError):
        ToolchainBinding(Origin("a" * 64, "pe:va", 0x401000),
                         (Span(0x401000, 0x401001),), "_f", unit, "test")


def test_archive_member_identity_rejects_reader_noncanonical_names():
    with pytest.raises(ValueError, match="forward slashes"):
        ToolchainBinding(Origin("a" * 64, "pe:va", 0x401000),
                         (Span(0x401000, 0x401001),), "_f", "a\\b.obj", "test")


def test_full_section_data_binding_resolves_relocation_after_exact_checks():
    manifest, image, raw = data_component()
    assert ToolchainManifest.from_dict(json.loads(canonical(manifest.to_dict()))) == manifest
    assert parse_component(render_component(manifest)) == manifest
    result = verify_toolchain(manifest, image, raw)
    assert result["data_sections"][0]["result"] == "pass"
    assert result["functions"][0]["result"] == "pass"
    assert result["data_sections"][0]["span"] == {"start": 0x402000, "end": 0x402030}
    assert set(result["data_sections"][0]["symbol_offsets"]) == {"_DoubleFormat", "_FloatFormat"}
    assert not {"size", "type"} & set(result["data_sections"][0])


@pytest.mark.parametrize("damage", ["bytes", "anchor", "flags", "coff_reloc", "pe_reloc"])
def test_data_section_damage_never_supplies_reference_target(damage):
    kwargs = {}
    if damage == "bytes":
        kwargs["section_bytes"] = b"X" + bytes(range(1, 48))
    elif damage == "anchor":
        kwargs["double_offset"] = 1
    elif damage == "flags":
        kwargs["data_flags"] = 0xE0400040
    elif damage == "coff_reloc":
        kwargs["data_relocation"] = True
    elif damage == "pe_reloc":
        kwargs["image_bytes"] = pe_data_fixture(bytes(range(48)), data_relocation=True)
    manifest, image, raw = data_component(**kwargs)
    result = verify_toolchain(manifest, image, raw)
    assert result["data_sections"][0]["result"] != "pass"
    assert result["functions"][0]["result"] == "unknown"


@pytest.mark.parametrize("flag", [0x00000200, 0x00000800])
def test_data_sections_not_selected_or_allocated_by_linker_do_not_supply_bases(flag):
    manifest, image, raw = data_component(data_flags=0xC0400040 | flag)
    result = verify_toolchain(manifest, image, raw)
    assert result["data_sections"][0]["result"] != "pass"
    assert result["functions"][0]["result"] == "unknown"


@pytest.mark.parametrize("damage", ["section_ordinal", "section_name", "section_size", "missing_anchor"])
def test_data_section_metadata_damage_never_supplies_reference_target(damage):
    manifest, image, raw = data_component()
    data = manifest.data_sections[0]
    if damage == "section_ordinal":
        changed = replace(data, section=3)
    elif damage == "section_name":
        changed = replace(data, section_name=".rdata")
    elif damage == "section_size":
        changed = replace(data, span=Span(data.span.start, data.span.end - 1))
    else:
        changed = replace(data, symbols=data.symbols[:1])
    damaged = replace(manifest, data_sections=(changed,))
    result = verify_toolchain(damaged, image, raw)
    assert result["data_sections"][0]["result"] != "pass"
    assert result["functions"][0]["result"] == "unknown"


def test_duplicate_data_symbol_anchor_is_rejected_at_manifest_boundary():
    manifest, _, _ = data_component()
    data = manifest.data_sections[0]
    with pytest.raises(ValueError, match="unique symbol anchors"):
        replace(data, symbols=data.symbols + (data.symbols[0],))


def test_explicit_data_section_base_conflicts_with_function_section_placement():
    from match_recovery.bound_objects import FunctionBinding, check_objects
    from match_recovery.pe_coff import Coff, PeImage

    manifest, image, raw = data_component()
    function = manifest.bindings[0]
    unit = function.unit
    obj = Coff(CoffArchive(raw).member(unit))
    pinned = PeImage(image, hashlib.sha256(image).hexdigest())
    with pytest.raises(ValueError, match="conflicting original placement"):
        check_objects((FunctionBinding(function.origin, function.spans, function.symbol, unit),),
                      {unit: obj}, pinned, member_functions=True,
                      section_bases={(unit, 1): 0x401001})

@pytest.fixture
def dependency():
    image = pe_fixture(b"\xe8\xfb\xff\xff\xff\xc3")
    obj = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 0, 20))
    raw = archive([("//", b"release\\runtime.obj\0"), ("/0", obj)])
    pin = hashlib.sha256(image).hexdigest()
    binding = VendorBinding(Origin(pin, "pe:va", 0x401000), (Span(0x401000, 0x401006),),
                            "_f", "release/runtime.obj", "synthetic original function boundary")
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
                                           ("Synthetic compiler distribution",), "test"),
                                 hashlib.sha256(raw).hexdigest(), "Synthetic test fixture", (binding,))
    return manifest, image, raw


def test_toolchain_roundtrip_and_bounded_verification_need_no_compiler(dependency):
    manifest, image, raw = dependency
    assert ToolchainManifest.from_dict(json.loads(canonical(manifest.to_dict()))) == manifest
    text = render_component(manifest)
    assert parse_component(text) == manifest
    assert "0x00401000-0x00401006" in text and "release/runtime.obj" in text
    assert "source_root" not in text
    report = verify_toolchain(manifest, image, raw)
    assert report["result"] == report["archive"]["result"] == "pass"
    assert report["counts"] == {"pass": 1}
    assert report["compile_attempts"] == report["application_recoveries"] == 0
    assert report["member_reads"] == 1 and not report["persisted"] and not report["ready_for_collection"]
    assert report["link_verification"] == report["runtime_verification"] == "unknown"


@pytest.mark.parametrize("damage", ["image", "digest", "member", "symbol", "body", "boundary", "duplicate", "unsupported"])
def test_toolchain_damage_cannot_pass(dependency, damage):
    manifest, image, raw = dependency
    binding, = manifest.bindings
    if damage == "image":
        with pytest.raises(ValueError, match="digest mismatch"):
            verify_toolchain(manifest, image + b"x", raw)
        return
    if damage == "digest":
        raw += b"wrong distribution"
    elif damage == "member":
        manifest = replace(manifest, bindings=(replace(binding, unit="missing.obj"),))
    elif damage == "symbol":
        manifest = replace(manifest, bindings=(replace(binding, symbol="_absent"),))
    elif damage == "boundary":
        manifest = replace(manifest, bindings=(replace(binding, spans=(Span(0x401000, 0x401005),)),))
    else:
        obj = CoffArchive(raw).member(binding.unit)
        if damage == "body":
            obj = coff_fixture(b"\x90" * 6)
        elif damage == "unsupported":
            obj = b"not a supported COFF object"
        raw = archive([("//", b"release/runtime.obj\0"), ("/0", obj)] +
                      ([("/0", obj)] if damage == "duplicate" else []))
        manifest = replace(manifest, archive_sha256=hashlib.sha256(raw).hexdigest())
    result = verify_toolchain(manifest, image, raw)
    assert result["result"] == ("fail" if damage in ("digest", "body", "boundary") else "unknown")
    assert not result["ready_for_collection"]
    if damage == "digest":
        assert result["member_reads"] == 0 and result["functions"] == []


def test_unbound_static_dependency_is_not_resolved_from_original_operand(dependency):
    manifest, image, raw = dependency
    obj = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20))
    raw = archive([("//", b"release/runtime.obj\0"), ("/0", obj)])
    manifest = replace(manifest, archive_sha256=hashlib.sha256(raw).hexdigest())
    report = verify_toolchain(manifest, image, raw)
    assert report["result"] == "unknown"
    assert report["unresolved_symbols"] == ["_g"]


@pytest.mark.parametrize("damage", ["owner", "delivery", "license", "duplicate", "symbols", "overlap", "fields", "version"])
def test_toolchain_manifest_refuses_ambiguous_or_unjustified_bindings(dependency, damage):
    manifest, _, _ = dependency
    with pytest.raises(ValueError):
        if damage == "owner":
            replace(manifest, component=replace(manifest.component, ownership=Ownership.APPLICATION))
        elif damage == "delivery":
            replace(manifest, component=replace(manifest.component, delivery=Delivery.RECOVERED_SOURCE))
        elif damage == "license":
            replace(manifest, license_evidence="")
        elif damage == "duplicate":
            replace(manifest, bindings=manifest.bindings * 2)
        elif damage == "symbols":
            second = replace(manifest.bindings[0], origin=replace(manifest.bindings[0].origin, address=0x401001),
                             spans=(Span(0x401001, 0x401004),))
            replace(manifest, bindings=manifest.bindings + (second,))
        elif damage == "overlap":
            replace(manifest, bindings=(replace(manifest.bindings[0], spans=(Span(0x401000, 0x401006),
                                                                           Span(0x401004, 0x401008))),))
        else:
            data = json.loads(canonical(manifest.to_dict()))
            data.update({"trust_all_symbols": True} if damage == "fields" else {"schema_version": True})
            ToolchainManifest.from_dict(data)


def test_archive_exact_selection_supports_short_long_and_windows_names():
    raw = archive([("/", b"ignored index"), ("/", b"second index"),
                   ("//", b"long/member.obj\0windows\\member.obj\0"),
                   ("short.obj/", b"a"), ("/0", b"bb"), ("/16", b"ccc")])
    parsed = CoffArchive(raw)
    assert parsed.member("short.obj") == b"a"
    assert parsed.member("long/member.obj") == b"bb"
    assert parsed.member("windows/member.obj") == b"ccc"
    with pytest.raises(ValueError, match="found 0"):
        parsed.member("member.obj")


def test_archive_path_aliases_are_ambiguous_not_extracted(tmp_path):
    raw = archive([("//", b"a\\b.obj\0a/b.obj\0../escape.obj\0"),
                   ("/0", b"a"), ("/8", b"b"), ("/16", b"c")])
    parsed = CoffArchive(raw)
    with pytest.raises(ValueError, match="found 2"):
        parsed.member("a/b.obj")
    assert parsed.member("../escape.obj") == b"c"
    assert not list(tmp_path.iterdir())  # Names are not filesystem operations.


@pytest.mark.parametrize("raw", [
    b"!<thin>\n", b"!<arch>\nx", archive([("f/", b"a")])[:-1],
    archive([("f/", b"ab")])[:-1], archive([("/0", b"ab")]),
    archive([("//", b"name\0"), ("/1", b"ab")]),
    archive([("//", b"name/\n"), ("/0", b"ab")]),
    archive([("//", b"name\0"), ("//", b"other\0")]),
    archive([("#1/5", b"name\0data")]), archive([("/64", b"ab")]),
    archive([("/", b"index")]) + b"trailing", archive([("bad", b"ab")]),
])
def test_archive_malformed_and_unsupported_layouts_refuse(raw):
    with pytest.raises(ValueError):
        CoffArchive(raw)


@pytest.fixture
def toolchain_project(tmp_path, dependency):
    manifest, image, raw = dependency
    for directory in ("code", "recovery", "hints"):
        (tmp_path / directory).mkdir()
    (tmp_path / "code/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest() + "\n")
    (tmp_path / "code/runtime.recovery.h").write_text(render_component(manifest))
    (tmp_path / "original.exe").write_bytes(image)
    (tmp_path / "runtime.lib").write_bytes(raw)
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "pe:va", "roots": ["code", "hints"],
                   "seed_roots": ["hints"]}}))
    return tmp_path


def command(root):
    return ["components", "check-binary", "runtime", "--root", str(root),
            "--image", "original.exe", "--archive", "runtime.lib"]


def test_native_cli_and_ownership_keep_runtime_out_of_application_work(toolchain_project, capsys):
    from match_recovery.application_binary import select_bindings
    from match_recovery.conventional_layout import read_offsets
    from match_recovery.ownership_source import project_view
    from match_recovery.planning import _context
    from match_recovery.source_config import SourceConfiguration
    from match_recovery.source_form import check_source_form
    from test_claim_integrity import git
    root = toolchain_project
    assert main(command(root)) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == "pass" and report["compile_attempts"] == 0
    assert main(["components", "list", "--root", str(root)]) == 0
    assert json.loads(capsys.readouterr().out)[0]["workflow"] == "verify_dependency"
    view, snapshot = project_view(root, allow_empty=True)
    candidate = view["groups"][0]["entities"][0]["candidates"][0]
    assert candidate["component"] == "runtime" and candidate["path"] == "code/runtime.recovery.h"
    # Even the successful component command above is not a current application
    # dependency admission; its report must not silently become a call target.
    assert "_f" not in select_bindings(view, frozenset())[2]
    entity = view["groups"][0]["entities"][0]["entity_id"]
    bindings, refused, targets = select_bindings(view, frozenset({entity}))
    assert not bindings and not targets and "dependency binding" in refused[0]["reason"]
    assert read_offsets(snapshot, SourceConfiguration.read(root))[2] == []
    git(root, "init", "-b", "main")
    context, _, _ = _context(root, "fixture", {"author": "test", "max_entities": 10, "max_editable_files": 10})
    group, = context["groups"]
    assert not group["eligible"] and "verify_dependency" in group["reasons"]
    (root / "code/application.cpp").write_text("void f() { __asm { ret } }\n")
    assert check_source_form(root)["result"] == "fail"  # No C++ source exemptions.


@pytest.mark.parametrize("target", ["runtime.lib", "original.exe", "code/runtime.recovery.h", "recovery/project.json"])
def test_toolchain_input_mutation_cannot_publish_a_pass(toolchain_project, capsys, monkeypatch, target):
    from match_recovery import component_binary
    root = toolchain_project
    original = component_binary.verify_toolchain
    def mutate(*args, **kwargs):
        report = original(*args, **kwargs)
        assert report["result"] == "pass"
        path = root / target
        path.write_bytes(path.read_bytes() + b"\n")
        return report
    monkeypatch.setattr(component_binary, "verify_toolchain", mutate)
    assert main(command(root)) == 3
    assert "changed" in capsys.readouterr().err


def test_toolchain_missing_archive_and_source_request_do_not_pass(toolchain_project, capsys):
    root = toolchain_project
    assert main(command(root)[:-2]) == 3
    assert "requires --archive" in capsys.readouterr().err
    assert main(["components", "check-source", "runtime", "--root", str(root)]) == 3
    assert "no upstream source" in capsys.readouterr().err


def test_toolchain_annotations_remain_preservation_obligations(toolchain_project):
    from match_recovery.preservation import _outside_annotations, _symbol_sources
    from match_recovery.source_config import SourceConfiguration
    root = toolchain_project
    text = (root / "code/runtime.recovery.h").read_text()
    manifest = parse_component(text)
    config = SourceConfiguration.read(root)
    records = _outside_annotations({"code/runtime.recovery.h": text.encode()}, config,
                                   manifest.bindings[0].origin.image_sha256, set())
    row, = records.values()
    assert row["kind"] == "component"
    assert _symbol_sources({}, records, set())["_f"][0]["source"] == "code/runtime.recovery.h"
    with pytest.raises(ValueError, match="duplicate component"):
        parse_components({"one.recovery.h": text, "two.recovery.h": text})


def test_plain_archive_member_codes_verify_through_their_binding():
    """A pinned member's plain code section carries identity in its binding."""
    import struct
    image = pe_fixture(b"\xc3")
    plain = bytearray(coff_fixture(b"\xc3"))
    # Strip the COMDAT flag from the single .text section header (section
    # table starts at 20; flags are the trailing dword of the 40-byte row).
    struct.pack_into("<I", plain, 56, struct.unpack_from("<I", plain, 56)[0] & ~0x1000)
    raw = archive([("//", b"release\\runtime.obj\0"), ("/0", bytes(plain))])
    pin = hashlib.sha256(image).hexdigest()
    binding = VendorBinding(Origin(pin, "pe:va", 0x401000), (Span(0x401000, 0x401001),),
                            "_f", "release/runtime.obj", "plain member code section")
    manifest = ToolchainManifest(Component("runtime", Ownership.RUNTIME, Delivery.TOOLCHAIN,
                                           ("Synthetic compiler distribution",), "test"),
                                 hashlib.sha256(raw).hexdigest(), "Synthetic test fixture", (binding,))
    report = verify_toolchain(manifest, image, raw)
    assert report["result"] == "pass" and report["counts"] == {"pass": 1}

    # The product path still refuses the identical shape: a plain section
    # never proves an independent function boundary in a product object.
    from match_recovery.bound_objects import FunctionBinding, check_objects
    from match_recovery.pe_coff import Coff, PeImage
    product_binding = FunctionBinding(Origin(pin, "pe:va", 0x401000),
                                      (Span(0x401000, 0x401001),), "_f", "release/runtime.obj")
    row, = check_objects((product_binding,), {"release/runtime.obj": Coff(bytes(plain))},
                         PeImage(image, pin))
    assert row["result"] == "unknown" and "COMDAT" in row["reason"]


@pytest.mark.parametrize("invalid", [None, False, 0, "", {}])
def test_explicit_data_sections_must_be_an_array(invalid):
    manifest, _, _ = data_component()
    value = json.loads(json.dumps(manifest.to_dict()))
    value["data_sections"] = invalid
    with pytest.raises(ValueError, match="data_sections must be an array"):
        ToolchainManifest.from_dict(value)
