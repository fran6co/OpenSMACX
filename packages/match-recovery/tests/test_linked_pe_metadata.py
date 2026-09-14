"""Physical native metadata controls; no compiler or passing proof mocks."""

from hashlib import sha256
from pathlib import Path
import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.msf import SmallMsf
from match_recovery.pe_coff import PeImage
from test_linked_initializers import update_artifact
from test_linked_modules import fixture as module_fixture
from test_linked_modules import set_link
from test_msf import small_msf
from test_build_command_evidence import descriptor, STAGES
import json

from match_recovery import linked_pe_metadata as metadata

BODY = [{"entity_id": "root", "address": 0x4011E0, "size": 16}]


def fixture(root, *, stamp=1, signature=0x12345678, age=1, imports=True, damage=None):
    context = module_fixture(root)
    raw = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<II", raw, 0x98 + 32, 0x1000, 0x200)
    struct.pack_into("<I", raw, 0x98 + 56, 0x3000)
    struct.pack_into("<I", raw, 0x88, stamp)
    # Real debug metadata has non-executable backing; .text metadata is a
    # refusal control below, not an implicitly inert positive fixture.
    raw[0x4C0:0x4DC] = raw[0x300:0x31C]
    raw[0x300:0x31C] = bytes(28)
    struct.pack_into("<II", raw, 0x98 + 96 + 6 * 8, 0x20C0, 28)
    struct.pack_into("<I", raw, 0x4C4, stamp)
    struct.pack_into("<II", raw, 0x608, signature, age)
    if imports:
        struct.pack_into("<II", raw, 0x98 + 96 + 1 * 8, 0x2100, 40)
        struct.pack_into("<II", raw, 0x98 + 96 + 12 * 8, 0x2160, 8)
        raw[0x500:0x600] = bytes(256)
        struct.pack_into("<5I", raw, 0x500, 0x2140, 0, 0, 0x2180, 0x2160)
        struct.pack_into("<II", raw, 0x540, 0x2190, 0)
        struct.pack_into("<II", raw, 0x560, 0x2190, 0)
        raw[0x580:0x589] = b"DEMO.dll\0"
        raw[0x590:0x597] = b"\x01\x00Name\0"
    msf = SmallMsf((context.directory / "app.pdb").read_bytes())
    streams = [msf.stream(index) for index in range(msf.stream_count)]
    info = bytearray(streams[1])
    struct.pack_into("<II", info, 4, signature, age)
    streams[1] = bytes(info)
    dbi = bytearray(streams[3])
    struct.pack_into("<I", dbi, 8, age)
    streams[3] = bytes(dbi)
    if damage:
        damage(raw, streams)
    update_artifact(context, "app.exe", bytes(raw))
    update_artifact(context, "app.pdb", small_msf(streams))
    return context


def observe(context, bodies=BODY):
    with linked_build(context) as native:
        return metadata.qualify_metadata(native, bodies)


def test_native_projection_changes_only_three_identification_fields(tmp_path):
    a = observe(fixture(tmp_path / "old"))
    b = observe(fixture(tmp_path / "new", stamp=123, signature=99, age=3))
    assert a["projected_bytes"] == b["projected_bytes"]
    assert a["image_sha256"] != b["image_sha256"]
    assert a["fields"] == b["fields"] == [
        {"offset": 0x88, "size": 4, "kind": "coff_timestamp"},
        {"offset": 0x4C4, "size": 4, "kind": "debug_timestamp"},
        {"offset": 0x608, "size": 8, "kind": "nb10_signature_age"}]
    assert a["qualification"] == "pass"
    assert a["result"] == a["outside_scope_verification"] == "unknown"
    assert not a["ready_for_collection"]
    assert {row["kind"] for row in a["protected_ranges"]} >= {
        "import_module_name", "import_hint_name", "import_lookup", "import_iat", "nb10_payload"}


@pytest.mark.parametrize("offset", [0x200, 0x400, 0x580, 0x590, 0x592, 0x616])
def test_unscoped_bytes_import_hint_names_and_nb10_locator_are_literal(tmp_path, offset):
    a = observe(fixture(tmp_path / "old"))
    b = observe(fixture(tmp_path / "new", damage=lambda raw, _: raw.__setitem__(offset, raw[offset] ^ 1)))
    assert a["projected_bytes"] != b["projected_bytes"]


@pytest.mark.parametrize("directory", [2, 3, 4, 7, 8, 9, 10, 11, 13, 14, 15])
def test_unknown_directory_never_gains_a_timestamp_or_body_exemption(tmp_path, directory):
    context = fixture(tmp_path / "project", damage=lambda raw, _: struct.pack_into(
        "<II", raw, 0x98 + 96 + directory * 8, 0x2100, 4))
    with pytest.raises(ValueError, match="unsupported nonzero"):
        observe(context)


@pytest.mark.parametrize("offset", [0x504, 0x508])
def test_import_timestamp_and_forwarder_chain_are_not_identification_fields(tmp_path, offset):
    context = fixture(tmp_path / "project", damage=lambda raw, _: struct.pack_into("<I", raw, offset, 1))
    with pytest.raises(ValueError, match="bound or chained"):
        observe(context)


def test_import_metadata_can_live_outside_directory_but_not_inside_body(tmp_path):
    def damage(raw, _):
        struct.pack_into("<I", raw, 0x50C, 0x11E0)
        raw[0x3E0:0x3E9] = b"DEMO.dll\0"
    context = fixture(tmp_path / "project", damage=damage)
    with pytest.raises(ValueError, match="scoped body overlaps.*import_module_name"):
        observe(context)


def test_projected_nb10_identity_cannot_double_as_an_import_lookup(tmp_path):
    # The independent parser must refuse unsupported aliases, rather than
    # permit a field projection to hide observable loader-table differences.
    context = fixture(tmp_path / "project", damage=lambda raw, _: struct.pack_into("<I", raw, 0x500, 0x1104))
    with pytest.raises(ValueError):
        observe(context)


def test_wrong_pdb_signature_refuses_before_metadata_projection(tmp_path):
    def damage(raw, streams):
        info = bytearray(streams[1])
        struct.pack_into("<I", info, 4, 17)
        streams[1] = bytes(info)
    with pytest.raises(ValueError, match="signature/age"):
        observe(fixture(tmp_path / "project", damage=damage))


def test_scope_cannot_hide_debug_directory(tmp_path):
    def damage(raw, _):
        raw[0x300:0x31C] = raw[0x4C0:0x4DC]
        struct.pack_into("<II", raw, 0x98 + 96 + 6 * 8, 0x1100, 28)
    with pytest.raises(ValueError, match="overlaps.*metadata"):
        observe(fixture(tmp_path / "project", damage=damage),
                [{"entity_id": "wrong", "address": 0x401100, "size": 28}])


def test_expired_native_context_cannot_replay_metadata(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as native:
        metadata.qualify_metadata(native, BODY)
    with pytest.raises(ValueError, match="expired"):
        metadata.qualify_metadata(native, BODY)


def test_final_guard_detects_mutation_during_projection(tmp_path, monkeypatch):
    context = fixture(tmp_path / "project")
    original = metadata.bound_fixups
    def mutate(image, pdb):
        value = original(image, pdb)
        context.alive = False
        return value
    with pytest.raises(ValueError, match="expired"):
        with linked_build(context) as native:
            monkeypatch.setattr(metadata, "bound_fixups", mutate)
            metadata.qualify_metadata(native, BODY)


def test_report_mutation_does_not_mutate_native_or_later_observation(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as native:
        report = metadata.qualify_metadata(native, BODY)
        original = native.inputs.image.data
        report["fields"].clear()
        report["protected_ranges"].clear()
        report["projected_bytes"] = b"other"
        assert len(metadata.qualify_metadata(native, BODY)["fields"]) == 3
        assert native.inputs.image.data == original


def locator_fixture(root, *, spelling=None, declared="app.pdb", duplicate=False, omit=False, codec=True):
    context = fixture(root)
    path = context.directory / declared
    evidence = json.loads((context.directory / "link.json").read_bytes())
    arguments = list(evidence["arguments"])
    original_count = len(arguments)
    if not omit:
        arguments += ["/pdb:" + str(path)] * (2 if duplicate else 1)
    evidence["arguments"] = arguments
    set_link(context, evidence, (context.directory / "link.log").read_text())
    description = descriptor(arguments, STAGES["link"],
        [(i, 0, "input_file") for i in range(original_count)] +
        [(i, 5, "output_file") for i in range(original_count, len(arguments))])
    if codec:
        description["codecs"].append({"anchor": "/", "prefix": "Q:\\", "separator": "\\"})
    context.report["compile_link"]["command_bindings"]["link"]["description"] = description
    spelling = spelling or "Q:\\" + "\\".join(path.parts[1:])
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[0x610:] = spelling.encode() + b"\0"
    struct.pack_into("<I", raw, 0x4D0, len(raw) - 0x600)
    update_artifact(context, "app.exe", bytes(raw))
    return context


def test_logical_locator_uses_fresh_output_and_explicit_codec_not_wine_assumptions(tmp_path):
    projections = []
    for suffix in ("old", "new"):
        context = locator_fixture(tmp_path / suffix)
        with linked_build(context) as native:
            value = metadata.qualify_metadata(native, BODY, project_locator=True)
            assert value["locator_identity"] == {"format": "msvc-pdb", "path": "app.pdb"}
            assert value["fields"][-1]["logical_output"] == value["locator_identity"]
            assert value["projected_bytes"][-1:] == b"\0"
            assert native.inputs.image.data != value["projected_bytes"]
            projections.append(value)
    assert projections[0]["projected_bytes"] == projections[1]["projected_bytes"]


@pytest.mark.parametrize("options,reason", [
    ({"spelling": "Q:\\unrelated\\app.pdb"}, "does not encode"),
    ({"codec": False}, "does not encode"),
    ({"duplicate": True}, "one actual declared"),
    ({"omit": True}, "one actual declared"),
    ({"declared": "other.pdb"}, "one actual declared"),
])
def test_unexplained_locator_or_side_output_is_not_projected(tmp_path, options, reason):
    with linked_build(locator_fixture(tmp_path / "project", **options)) as native:
        with pytest.raises(ValueError, match=reason):
            metadata.qualify_metadata(native, BODY, project_locator=True)


def test_locator_projection_remains_explicit_and_length_changes_stay_visible(tmp_path):
    values = []
    for name in ("a", "longer"):
        with linked_build(locator_fixture(tmp_path / name)) as native:
            literal = metadata.qualify_metadata(native, BODY)
            projected = metadata.qualify_metadata(native, BODY, project_locator=True)
            assert literal["locator_identity"] is None and len(literal["fields"]) == 3
            assert literal["projected_bytes"] != projected["projected_bytes"]
            values.append(projected)
    assert len(values[0]["projected_bytes"]) != len(values[1]["projected_bytes"])
    assert values[0]["fields"][-1]["size"] != values[1]["fields"][-1]["size"]


def test_projected_header_timestamp_cannot_be_a_loader_fixup(tmp_path):
    def damage(raw, _):
        # Existing relocation block remains physically valid but now names
        # the loaded header's timestamp, not an ordinary data pointer.
        struct.pack_into("<I", raw, 0x380, 0)
        struct.pack_into("<H", raw, 0x388, 0x3088)
    with pytest.raises(ValueError, match="header identification field has a loader fixup"):
        observe(fixture(tmp_path / "project", damage=damage))


@pytest.mark.parametrize("bodies", [[], BODY])
def test_executable_metadata_outside_observed_roots_is_not_an_identification_exemption(tmp_path, bodies):
    def damage(raw, _):
        raw[0x300:0x31C] = raw[0x4C0:0x4DC]
        struct.pack_into("<II", raw, 0x98 + 96 + 6 * 8, 0x1100, 28)
    with pytest.raises(ValueError, match="overlaps executable storage"):
        observe(fixture(tmp_path / "project", damage=damage), bodies)


def test_execute_flag_matters_even_when_section_is_declared_initialized_data(tmp_path):
    def damage(raw, streams):
        struct.pack_into("<I", raw, 0x1A0 + 36, 0xE0000040)
        streams[5] = bytes(raw[0x178:0x1C8])
    with pytest.raises(ValueError, match="overlaps executable storage"):
        observe(fixture(tmp_path / "project", damage=damage))


@pytest.mark.parametrize("address,size", [(0x4020C4, 4), (0x4020C0, 28), (0x4020C3, 2), (0x4020C7, 2)])
def test_projected_field_cannot_overlap_observed_runtime_data(tmp_path, address, size):
    with linked_build(fixture(tmp_path / "project")) as native:
        with pytest.raises(ValueError, match="observed runtime region"):
            metadata.qualify_metadata(native, [], runtime_regions=[{"address": address, "size": size}])


def test_runtime_protection_accepts_code_and_data_without_omitting_either(tmp_path):
    regions = [{"address": 0x4011E0, "size": 16}, {"address": 0x402020, "size": 8}]
    with linked_build(fixture(tmp_path / "project")) as native:
        value = metadata.qualify_metadata(native, BODY, runtime_regions=regions)
        ordinary = metadata.qualify_metadata(native, BODY)
        assert value["projected_bytes"] == ordinary["projected_bytes"]
        assert value["fields"] == ordinary["fields"]
        assert value["projected_bytes"][0x420:0x428] == native.inputs.image.data[0x420:0x428]
        assert value["runtime_protection"] == [
            {"offset": 0x3E0, "size": 16, "kind": "runtime_region"},
            {"offset": 0x420, "size": 8, "kind": "runtime_region"}]


@pytest.mark.parametrize("regions", [None, {}, [{"address": True, "size": 4}],
    [{"address": 0x401000, "size": False}], [{"address": 0x401000, "size": 0}],
    [{"address": 0x401FFF, "size": 2}], [{"address": 0x400088, "size": 4}],
    [{"address": 0x402020, "size": 4, "permission": True}]])
def test_invalid_protection_ranges_refuse_instead_of_becoming_body_authority(tmp_path, regions):
    with linked_build(fixture(tmp_path / "project")) as native:
        with pytest.raises(ValueError, match="runtime protection"):
            metadata.qualify_metadata(native, BODY, runtime_regions=regions)


def test_runtime_protection_mutation_is_detected_by_final_guard(tmp_path, monkeypatch):
    regions = [{"address": 0x402020, "size": 8}]
    original = metadata.bound_fixups
    def mutate(image, pdb):
        value = original(image, pdb)
        regions.clear()
        return value
    with linked_build(fixture(tmp_path / "project")) as native:
        monkeypatch.setattr(metadata, "bound_fixups", mutate)
        with pytest.raises(ValueError, match="runtime protection changed"):
            metadata.qualify_metadata(native, BODY, runtime_regions=regions)
