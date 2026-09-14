"""Physical one-hop data FIXUP anchors; no circular or partial-byte proof."""

from dataclasses import replace
from hashlib import sha256
import json
import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_data_contents import verify_data_contents
from match_recovery.linked_storage import observe_data_section, _data_sections
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.pe_coff import PeImage
from test_link_file_evidence import stamp
from test_linked_data_contents import fixture as data_fixture, object_bytes, pointer, OWNER
from test_linked_initializers import Context
from test_linked_storage_controls import map_bytes
from test_msf import small_msf
from test_pdb_fixups import parts


def fixture(root, *, addend=0, kind=6, damage=None):
    root.mkdir()
    raw = object_bytes(pointer(addend & 0xFFFFFFFF), references=((0, 2, kind),))
    if damage == "cycle":
        raw = bytearray(raw)
        symbols, = struct.unpack_from("<I", raw, 8)
        raw[symbols:symbols] = struct.pack("<IIH", 0, 0, 6)
        struct.pack_into("<I", raw, 8, symbols + 10)
        struct.pack_into("<I", raw, 84, symbols)
        struct.pack_into("<H", raw, 92, 1)
        target_data, = struct.unpack_from("<I", raw, 80)
        raw[target_data:target_data + 4] = bytes(4)
        raw = bytes(raw)
    address = 0x401034 + addend - (0x401014 if kind == 20 else 0)
    contents = bytearray(pointer(address & 0xFFFFFFFF))
    if damage == "tail":
        contents[-1] ^= 1
    inputs = data_fixture(root, raw=raw, contents=bytes(contents), target=False,
                          absolute=() if kind == 20 or damage == "highlow" else
                          (0x10, 0x30) if damage == "cycle" else (0x10,))
    if damage in {"unseeded", "cycle"}:
        inputs.mapping = replace(inputs.mapping, statics=())
    if damage == "contradiction":
        # A second name in B has an independently mapped conflicting base.
        symbols = [("_anchor", 4, 1, 0, 3, b""), ("_inner", 8, 1, 0, 3, b""),
                   ("_target", 4, 2, 0, 3, b""), ("_public", 0, 0, 0, 2, b""),
                   ("_other", 0, 2, 0, 3, b"")]
        raw = object_bytes(pointer(0), references=((0, 2, 6),), symbols=symbols)
        # Rebuild a coherent physical inventory rather than mutate selected reports.
        (root / "other").mkdir()
        inputs = data_fixture(root / "other", raw=raw, contents=pointer(0x401034),
                              target=False, absolute=(0x10,))
        row = replace(inputs.mapping.statics[0], symbol="_other", address=0x401038, offset=0x38)
        inputs.mapping = replace(inputs.mapping, statics=(*inputs.mapping.statics, row))
    rows = () if damage == "missing" else ((7 if damage == "kind" else kind, 0,
                0x1010, 0x1038 if damage == "target" else 0x1034),)
    if damage == "cycle":
        rows += ((6, 0, 0x1030, 0x1014),)
    reference, streams = parts(rows)
    image = bytearray(inputs.image.data)
    if damage == "cycle":
        struct.pack_into("<I", image, 0x230, 0x401014)
    if damage == "target_contents":
        image[0x230] ^= 1
    payload = reference[0x400:]
    at = len(image)
    image.extend(payload)
    struct.pack_into("<I", image, 0x98 + 60, 0x200)
    struct.pack_into("<II", image, 0x98 + 96 + 6 * 8, 0x1100, 28)
    struct.pack_into("<IIHHIIII", image, 0x300, 0, 0, 0, 0, 2, len(payload), 0, at)
    streams[5] = bytes(image[0x178:0x1A0])
    inputs.image = PeImage(bytes(image), sha256(image).hexdigest())
    inputs.pdb = small_msf(streams)
    inputs.fixups = bound_fixups(inputs.image, inputs.pdb)
    return inputs


@pytest.mark.parametrize("kind,addend", [(6, 0), (6, -2), (6, 2), (20, -2), (20, 2)])
def test_native_fixup_places_unmapped_target_only_after_entire_data_source_passes(tmp_path, kind, addend):
    inputs = fixture(tmp_path / "inputs", kind=kind, addend=addend)
    envelope = observe_data_section(inputs, OWNER, 2)
    assert envelope["address"] == 0x401030 and envelope["size"] == 8
    anchor, = envelope["anchors"]
    assert anchor["placement"] == "verified_data_pdb_fixup"
    assert anchor["address"] == 0x401034 and anchor["object_offset"] == 4
    assert anchor["referring_site"] == 0x401010
    assert envelope["data_evidence"]["sources"][0]["result"] == "pass"
    assert verify_data_contents(inputs, OWNER, 1)["result"] == "pass"
    assert verify_data_contents(inputs, OWNER, 2)["result"] == "pass"
    assert envelope["allocation_isolation"] == envelope["contents"] == "unknown"


@pytest.mark.parametrize("damage", ["tail", "missing", "kind", "target", "highlow", "unseeded", "cycle"])
def test_partial_or_unseeded_sources_cannot_supply_target_placement(tmp_path, damage):
    inputs = fixture(tmp_path / "inputs", damage=damage)
    with pytest.raises(ValueError, match="independent placement"):
        observe_data_section(inputs, OWNER, 2)
    if damage not in {"unseeded", "cycle"}:
        evidence = observe_data_section(inputs, OWNER, 1)["data_evidence"]
        assert evidence["gaps"] and not any(row.get("anchor_count", 0) for row in evidence["sources"])


@pytest.mark.parametrize("addend", [-5, 4])
def test_fixup_does_not_waive_effective_target_bounds(tmp_path, addend):
    inputs = fixture(tmp_path / "inputs", addend=addend)
    with pytest.raises(ValueError, match="independent placement"):
        observe_data_section(inputs, OWNER, 2)


def test_contradictory_independent_target_base_never_produces_data_anchor(tmp_path):
    inputs = fixture(tmp_path / "inputs", damage="contradiction")
    report = observe_data_section(inputs, OWNER, 2)
    assert report["address"] == 0x401038
    assert all(row["placement"] != "verified_data_pdb_fixup" for row in report["anchors"])
    assert "disagree" in report["data_evidence"]["gaps"][0]["reason"]
    assert verify_data_contents(inputs, OWNER, 1)["result"] == "fail"


def test_source_placement_does_not_claim_target_contents(tmp_path):
    inputs = fixture(tmp_path / "inputs", damage="target_contents")
    assert observe_data_section(inputs, OWNER, 2)["placement"] == "pass"
    assert verify_data_contents(inputs, OWNER, 1)["result"] == "pass"
    assert verify_data_contents(inputs, OWNER, 2)["result"] == "fail"


def context(root):
    inputs = fixture(root / "inputs")
    directory = root / "artifacts"
    directory.mkdir()
    evidence = inputs.evidence
    library = root / "inputs/runtime.lib"
    log = (f"Start Pass1\n Searching {library}:\nEnd Pass1\nStart Pass2\napp.obj\n"
           "RUNTIME.lib(data.obj)\nEnd Pass2\n").encode()
    assert stamp(log)["sha256"] == evidence["link_log_sha256"]
    rows = []
    for name, kind, raw in [("app.exe", "pe32-i386", inputs.image.data),
                            ("app.pdb", "msvc-pdb", inputs.pdb),
                            ("app.map", "msvc-map", map_bytes(inputs.mapping)),
                            ("link.log", "msvc-link-log", log),
                            ("link.json", "link-input-files", json.dumps(evidence).encode())]:
        (directory / name).write_bytes(raw)
        rows.append({"path": name, "format": kind, "result": "pass", **stamp(raw)})
    return Context(root, directory, {"result": "pass", "compile_link": {"result": "pass"}, "outputs": rows})


def test_actual_live_context_path_expires_and_returns_defensive_observations(tmp_path):
    ctx = context(tmp_path)
    with linked_build(ctx) as live:
        report = live.data_section(OWNER, 2)
        assert report["address"] == 0x401030
        report["data_evidence"]["sources"][0]["address"] = 0
        assert live.data_section(OWNER, 2)["address"] == 0x401030
        assert live.data_contents(OWNER, 2)["result"] == "pass"
    with pytest.raises(ValueError):
        live.data_section(OWNER, 2)


@pytest.mark.parametrize("artifact", ["app.pdb", "app.exe", "link.json"])
def test_native_final_epoch_guards_reject_artifact_mutation(tmp_path, artifact):
    ctx = context(tmp_path)
    with pytest.raises(ValueError):
        with linked_build(ctx) as live:
            assert live.data_section(OWNER, 2)["placement"] == "pass"
            path = ctx.directory / artifact
            path.write_bytes(path.read_bytes() + b"damage")


def test_native_final_epoch_guard_rejects_changed_physical_archive(tmp_path):
    ctx = context(tmp_path)
    with pytest.raises(ValueError):
        with linked_build(ctx) as live:
            assert live.data_section(OWNER, 2)["placement"] == "pass"
            path = tmp_path / "inputs/runtime.lib"
            path.write_bytes(path.read_bytes() + b"damage")


def test_batch_return_does_not_mutate_pending_anchor_cache(tmp_path):
    inputs = fixture(tmp_path / "inputs")
    with _data_sections(inputs) as observe:
        report = observe(OWNER, 2)
        report["data_evidence"]["sources"][0]["address"] = 0
        report["anchors"][0]["address"] = 0
        assert observe(OWNER, 2)["address"] == 0x401030
