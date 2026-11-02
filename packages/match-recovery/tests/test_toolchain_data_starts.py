"""Synthetic external BSS/COMMON points, with complete physical archive controls."""

from dataclasses import replace
import hashlib
import json
import struct

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, parse_components, render_component
from match_recovery.components import (CodeEntryAnchor, ToolchainBssDataStart,
                                       ToolchainCommonDataStart, ToolchainCodeSectionBinding,
                                       ToolchainManifest, ToolchainBinding)
from match_recovery.model import Component, Delivery, Origin, Ownership, Span, canonical
from test_toolchain_components import archive, pe_data_fixture


def obj(sections=(), symbols=(), relocations=()):
    """Regular physical COFF, including BSS declared size without file bytes."""
    cursor = 20 + 40 * len(sections)
    headers, bodies = [], []
    for number, (name, data, size, flags) in enumerate(sections, 1):
        rels = [row for row in relocations if row[0] == number]
        encoded = b"".join(struct.pack("<IIH", off, sym, kind) for _, off, sym, kind in rels)
        headers.append(name.encode().ljust(8, b"\0") + struct.pack(
            "<IIIIIIHHI", 0, 0, size, cursor if data else 0,
            cursor + len(data) if rels else 0, 0, len(rels), 0, flags))
        bodies.append(data + encoded)
        cursor += len(data) + len(encoded)
    names = bytearray(struct.pack("<I", 4))
    records = []
    for name, value, section, type_, storage, aux in symbols:
        encoded = name.encode()
        if len(encoded) > 8:
            encoded = struct.pack("<II", 0, len(names))
            names.extend(name.encode() + b"\0")
        records.append(encoded.ljust(8, b"\0") + struct.pack(
            "<IhHBB", value, section, type_, storage, len(aux) // 18) + aux)
    struct.pack_into("<I", names, 0, len(names))
    header = struct.pack("<HHIIIHH", 0x14C, len(sections), 0, cursor,
                         sum(1 + len(row[-1]) // 18 for row in symbols), 0, 0)
    return header + b"".join(headers + bodies + records) + names


def case(kind="bss", *, same_member=False, addend=8, extra=(), provider=None,
         reference_type=0, relocation_kind=6):
    image = bytearray(pe_data_fixture(bytes(48)))
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    struct.pack_into("<II", image, 0x178 + 40 + 16, 0, 0)  # .data virtual zero tail
    image[512:518] = b"\xa1" + (0x402000 + addend).to_bytes(4, "little") + b"\xc3"
    image = bytes(image)
    pin = hashlib.sha256(image).hexdigest()
    owner = "code.obj" if same_member else "data.obj"
    symbols = [("_entry", 0, 1, 0x20, 2, b""), ("_alias", 0, 1, 0x20, 2, b"")]
    sections = [(".text", b"\xa1" + addend.to_bytes(4, "little") + b"\xc3", 6, 0x60500020)]
    if same_member:
        sections.append((".bss", b"", 32, 0xC0300080))
        symbols.append(("_state", 0 if kind == "bss" else 4,
                        2 if kind == "bss" else 0, reference_type, 2, b""))
    else:
        symbols.append(("_state", 0, 0, reference_type, 2, b""))
    code = obj(sections, symbols, [(1, 1, 2, relocation_kind)])
    members = [("code.obj/", code)]
    if not same_member:
        provider = provider if provider is not None else obj(
            [(".bss", b"", 32, 0xC0300080)],
            [("_state", 0 if kind == "bss" else 4, 1 if kind == "bss" else 0, 0, 2, b"")])
        members.append(("data.obj/", provider))
    members.extend(extra)
    raw = archive(members)
    origin = Origin(pin, "pe:va", 0x402000)
    start = (ToolchainBssDataStart(owner, "_state", origin, 2 if same_member else 1,
                                 ".bss", 0, 32, "independent writer and loader evidence")
             if kind == "bss" else ToolchainCommonDataStart(
                 owner, "_state", origin, 4, "independent counter read/write evidence"))
    section = ToolchainCodeSectionBinding("code.obj", 1, ".text", Origin(pin, "pe:va", 0x401000),
        Span(0x401000, 0x401006), "independent complete contribution boundary",
        (CodeEntryAnchor("_entry", 0, "independent entry"),
         CodeEntryAnchor("_alias", 0, "independent coentry")))
    manifest = ToolchainManifest(Component("runtime-starts", Ownership.RUNTIME, Delivery.TOOLCHAIN,
        ("synthetic dependency",), "test"), hashlib.sha256(raw).hexdigest(),
        "synthetic provenance", code_sections=(section,), data_starts=(start,))
    return manifest, image, raw


@pytest.mark.parametrize("kind", ["bss", "common"])
@pytest.mark.parametrize("same_member", [False, True])
def test_physical_start_resolves_only_internal_references_and_keeps_all_object_obligations(kind, same_member):
    manifest, image, raw = case(kind, same_member=same_member)
    assert ToolchainManifest.from_dict(json.loads(canonical(manifest.to_dict()))) == manifest
    assert parse_component(render_component(manifest)) == manifest
    report = verify_toolchain(manifest, image, raw)
    point, = report["data_starts"]
    assert point["start_identity"] == "pass" and point["scoped_reference_target"] is True
    assert point["address"] == "0x00402000"
    assert all(point[field] == "unknown" for field in
               ("result", "original_extent", "original_content", "allocation", "selection", "lifetime"))
    assert point["callable"] is point["link_target"] is False
    section, = report["code_sections"]
    assert section["result"] == section["references"] == "pass"
    assert all(entry["result"] == "pass" for entry in section["entries"])
    assert section["relocations"][0]["result"] == "pass"  # preserves the +8 COFF addend
    assert report["result"] == "unknown" and report["functions"] == []
    assert report["link_verification"] == "unknown" and not report["ready_for_collection"]
    assert report["data_start_inventory"]["member_count"] == (1 if same_member else 2)


@pytest.mark.parametrize("kind", ["bss", "common"])
@pytest.mark.parametrize("damage", ["offset", "size", "member", "symbol", "original_start"])
def test_declared_identity_damage_never_borrows_the_operand(kind, damage):
    manifest, image, raw = case(kind)
    point, = manifest.data_starts
    if damage == "offset":
        changed = replace(point, offset=1) if kind == "bss" else replace(point, common_size=3)
    elif damage == "size":
        changed = replace(point, section_size=31) if kind == "bss" else replace(point, common_size=8)
    elif damage == "member":
        changed = replace(point, unit="absent.obj")
    elif damage == "symbol":
        changed = replace(point, symbol="_absent")
    else:
        changed = replace(point, origin=replace(point.origin, address=0x402004))
    report = verify_toolchain(replace(manifest, data_starts=(changed,)), image, raw)
    if damage == "original_start":
        assert report["data_starts"][0]["start_identity"] == "pass"
        assert report["code_sections"][0]["result"] == "fail"
    else:
        assert report["data_starts"][0]["start_identity"] == "unknown"
        assert report["code_sections"][0]["result"] == "unknown"


@pytest.mark.parametrize("kind", ["bss", "common"])
@pytest.mark.parametrize("damage", ["strong", "common", "weak", "fallback", "import", "unsupported", "alias"])
def test_unselected_archive_competitors_are_not_silently_ignored(kind, damage):
    if damage == "strong":
        other = obj([(".data", b"xxxx", 4, 0xC0300040)], [("_state", 0, 1, 0, 2, b"")])
    elif damage == "common":
        other = obj(symbols=[("_state", 4, 0, 0, 2, b"")])
    elif damage in ("weak", "fallback"):
        name, fallback = ("_state", "_other") if damage == "weak" else ("_other", "_state")
        aux = struct.pack("<II", 2, 3) + bytes(10)
        other = obj(symbols=[(name, 0, 0, 0, 105, aux), (fallback, 0, 0, 0, 2, b"")])
        other = bytes(2) + other[2:]  # valid machine-zero metadata must still participate
    elif damage == "import":
        names = b"_state\0RUNTIME.dll\0"
        other = struct.pack("<HHHHIIHH", 0, 0xFFFF, 0, 0x14C, 0, len(names), 0, 5) + names
    elif damage == "unsupported":
        other = b"unparsed records are not absence evidence"
    else:
        text = b" /alternatename:_other=_state"
        other = obj([(".drectve", text, len(text), 0x00100A00)])
    manifest, image, raw = case(kind, extra=[("unused.obj/", other)])
    report = verify_toolchain(manifest, image, raw)
    assert report["data_starts"][0]["start_identity"] == "unknown"
    assert report["code_sections"][0]["result"] == "unknown"


@pytest.mark.parametrize("damage", ["initialized", "comdat", "tls", "executable", "discardable", "offset", "function", "static", "absolute", "common_zero"])
def test_wrong_physical_provider_kind_cannot_qualify_as_bss(damage):
    flags, name, data, size, offset, section, type_, storage = 0xC0300080, ".bss", b"", 32, 0, 1, 0, 2
    if damage == "initialized": flags, data = 0xC0300040, bytes(32)
    elif damage == "comdat": flags |= 0x1000
    elif damage == "tls": name = ".tls"
    elif damage == "executable": flags |= 0x20000020
    elif damage == "discardable": flags |= 0x02000000
    elif damage == "offset": offset = 32
    elif damage == "function": type_ = 0x20
    elif damage == "static": storage = 3
    elif damage == "absolute": section = -1
    elif damage == "common_zero": section = 0
    provider = obj([(name, data, size, flags)], [("_state", offset, section, type_, storage, b"")])
    manifest, image, raw = case(provider=provider)
    report = verify_toolchain(manifest, image, raw)
    assert report["data_starts"][0]["start_identity"] == "unknown"
    assert report["code_sections"][0]["result"] == "unknown"


@pytest.mark.parametrize("damage", ["function_reference", "relative_reference", "image_code", "image_readonly", "image_raw", "archive_digest", "duplicate_member"])
def test_reference_storage_and_archive_guards(damage):
    kwargs = {"reference_type": 0x20} if damage == "function_reference" else {}
    if damage == "relative_reference": kwargs["relocation_kind"] = 20
    if damage == "duplicate_member": kwargs["extra"] = [("data.obj/", obj())]
    manifest, image, raw = case(**kwargs)
    if damage.startswith("image_"):
        image = bytearray(image)
        if damage == "image_code": struct.pack_into("<I", image, 0x178 + 40 + 36, 0xE00000A0)
        if damage == "image_readonly": struct.pack_into("<I", image, 0x178 + 40 + 36, 0x40000080)
        if damage == "image_raw": struct.pack_into("<II", image, 0x178 + 40 + 16, 512, 1024)
        image = bytes(image)
        pin = hashlib.sha256(image).hexdigest()
        section = manifest.code_sections[0]
        manifest = replace(manifest, data_starts=(replace(manifest.data_starts[0], origin=replace(manifest.data_starts[0].origin, image_sha256=pin)),),
                           code_sections=(replace(section, origin=replace(section.origin, image_sha256=pin)),))
    if damage == "archive_digest": raw += b"wrong distribution"
    report = verify_toolchain(manifest, image, raw)
    assert report["code_sections"][0]["result"] != "pass"
    assert all(not entry["scoped_reference_target"] for entry in report["code_sections"][0]["entries"])


def test_start_does_not_supply_section_bases_or_function_row_targets():
    manifest, image, raw = case()
    from match_recovery.coff_archive import CoffArchive
    # One ordinary function definition gives a measurable function witness;
    # the alias-bearing whole-contribution witness above has two public entries.
    code = obj([(".text", b"\xa1\x08\0\0\0\xc3", 6, 0x60500020)],
               [("_entry", 0, 1, 0x20, 2, b""), ("_state", 0, 0, 0, 2, b"")],
               [(1, 1, 1, 6)])
    raw = archive([("code.obj/", code), ("data.obj/", CoffArchive(raw).member("data.obj"))])
    function = ToolchainBinding(manifest.code_sections[0].origin, (Span(0x401000, 0x401006),),
                                "_entry", "code.obj", "independent function")
    report = verify_toolchain(replace(manifest, code_sections=(), bindings=(function,),
                                     archive_sha256=hashlib.sha256(raw).hexdigest()), image, raw)
    assert report["data_starts"][0]["start_identity"] == "pass"
    assert report["functions"][0]["references"] == "unknown"


@pytest.mark.parametrize("value", [None, False, 0, "", {}])
def test_start_array_schema_is_strict(value):
    manifest, _, _ = case()
    data = json.loads(canonical(manifest.to_dict()))
    data["data_starts"] = value
    with pytest.raises(ValueError, match="data_starts must be an array"):
        ToolchainManifest.from_dict(data)


def test_start_conflicts_and_fabricated_spans_are_rejected():
    manifest, _, _ = case()
    point, = manifest.data_starts
    with pytest.raises(ValueError, match="symbols conflict"):
        replace(manifest, data_starts=(point, point))
    with pytest.raises(ValueError, match="identities must be unique"):
        replace(manifest, data_starts=(point, replace(point, symbol="_other")))
    with pytest.raises(ValueError, match="overlaps"):
        replace(manifest, data_starts=(replace(point, origin=replace(point.origin, address=0x401001)),))
    data = json.loads(canonical(manifest.to_dict()))
    data["data_starts"][0]["span"] = {"start": 0x402000, "end": 0x402004}
    with pytest.raises(ValueError): ToolchainManifest.from_dict(data)
    other = replace(manifest, component=replace(manifest.component, component_id="other"),
                    code_sections=())
    with pytest.raises(ValueError, match="owned by multiple"):
        parse_components({"a.recovery.h": render_component(manifest), "b.recovery.h": render_component(other)})


def test_empty_data_starts_do_not_change_legacy_serialized_fields():
    manifest, _, _ = case()
    old = replace(manifest, data_starts=())
    data = old.to_dict()
    assert "data_starts" not in data
    assert list(data) == ["schema_version", "kind", "component", "archive_sha256",
                          "license_evidence", "bindings", "absolute_symbols", "data_sections", "code_sections"]
    assert ToolchainManifest.from_dict(json.loads(canonical(data))) == old
    assert parse_component(render_component(old)) == old


@pytest.mark.parametrize("damage", [None, "start", "missing"])
def test_only_fully_qualified_code_entries_publish_and_data_points_never_export(tmp_path, damage):
    from match_recovery.component_targets import component_targets
    from test_component_targets import _project
    manifest, image, raw = case()
    manifest = replace(manifest, component=replace(manifest.component, component_id="runtime"))
    if damage == "start":
        point, = manifest.data_starts
        manifest = replace(manifest, data_starts=(replace(point, origin=replace(point.origin, address=0x402004)),))
    elif damage == "missing":
        manifest = replace(manifest, data_starts=())
    path = tmp_path / "runtime.lib"
    path.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, path, image)
    targets, reports = component_targets(tmp_path, snapshot, image)
    assert "_state" not in targets
    assert targets == ({"_entry": 0x401000, "_alias": 0x401000} if damage is None else {})
    assert reports[0]["result"] != "pass"  # outstanding data or code obligations remain


def test_neutral_machine_zero_weak_member_is_positively_inventoried():
    aux = struct.pack("<II", 2, 3) + bytes(10)
    neutral = obj(symbols=[("_aliasx", 0, 0, 0, 105, aux), ("_undefx", 0, 0, 0, 2, b"")])
    neutral = bytes(2) + neutral[2:]
    manifest, image, raw = case(extra=[("neutral.obj/", neutral)])
    report = verify_toolchain(manifest, image, raw)
    assert report["data_start_inventory"]["result"] == "pass"
    assert report["data_start_inventory"]["member_count"] == 3
    assert report["data_starts"][0]["start_identity"] == "pass"


@pytest.mark.parametrize("damage", ["ordinal", "name", "raw_zero_bytes", "relocations"])
def test_bss_exact_section_identity_and_uninitialized_records(damage):
    provider = None
    if damage in ("raw_zero_bytes", "relocations"):
        provider = obj([(".bss", bytes(32) if damage == "raw_zero_bytes" else b"", 32, 0xC0300080)],
                       [("_state", 0, 1, 0, 2, b"")],
                       [(1, 0, 0, 6)] if damage == "relocations" else [])
    manifest, image, raw = case(provider=provider)
    if damage in ("ordinal", "name"):
        point, = manifest.data_starts
        point = replace(point, section=2) if damage == "ordinal" else replace(point, section_name=".data")
        manifest = replace(manifest, data_starts=(point,))
    report = verify_toolchain(manifest, image, raw)
    assert report["data_starts"][0]["start_identity"] == "unknown"
    assert report["code_sections"][0]["result"] == "unknown"


def test_common_binding_cannot_reinterpret_an_allocated_bss_definition():
    provider = obj([(".bss", b"", 32, 0xC0300080)], [("_state", 0, 1, 0, 2, b"")])
    manifest, image, raw = case("common", provider=provider)
    report = verify_toolchain(manifest, image, raw)
    assert report["data_starts"][0]["start_identity"] == "unknown"
    assert report["code_sections"][0]["result"] == "unknown"


def test_explicit_source_import_cannot_compete_with_a_data_start():
    from match_recovery.import_source import parse_imports
    manifest, image, raw = case()
    imports = parse_imports("// RECOVERY_IMPORT: v1 _state RUNTIME.dll name:State\n", "src/imports.h")
    with pytest.raises(ValueError, match="both data start and import"):
        verify_toolchain(manifest, image, raw, import_bindings=imports)


def test_component_target_wrapper_also_refuses_source_import_conflicts(tmp_path):
    import base64
    from match_recovery.component_targets import component_targets
    from test_component_targets import _project
    manifest, image, raw = case()
    manifest = replace(manifest, component=replace(manifest.component, component_id="runtime"))
    path = tmp_path / "runtime.lib"
    path.write_bytes(raw)
    snapshot = _project(tmp_path, manifest, path, image)
    declaration = "// RECOVERY_IMPORT: v1 _state RUNTIME.dll name:State\n"
    snapshot["files"]["src/imports.h"] = {"content_base64": base64.b64encode(declaration.encode()).decode()}
    with pytest.raises(ValueError, match="both data start and import"):
        component_targets(tmp_path, snapshot, image)
