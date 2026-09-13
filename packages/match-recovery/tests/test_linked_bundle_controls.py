"""Independent physical controls for linked associated code/data closure."""

from hashlib import sha256
import struct

import pytest

from match_recovery.linked_code_graph import observe_code_graph
from match_recovery.linked_objects import SelectedObjects
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.pe_coff import Coff, PeImage
from test_link_file_evidence import stamp
from test_link_input_coverage import inputs
from test_linked_objects import inventory
from test_msf import small_msf
from test_pdb_fixups import parts


def family_object(*, callback=False, overlap=False, extra=None):
    """Root reaches code and data independently; data reaches code or callback."""
    specs = [
        (b".text", b"\xe8" + bytes(4) + b"\x68" + bytes(4) + b"\xc3",
         ((1, 5, 20), (6, 8, 6)), 0, 2, 0x60501020, b"_f", 32, 2),
        (b".text$x", b"\xb8" + bytes(4) + b"\xc3", ((1, 8, 6),),
         1, 5, 0x60501020, b"$child", 0, 6),
        (b".xdata$x", bytes(16), ((0, 11 if callback else 5, 6),),
         1, 5, 0x40401040, b"$data", 0, 3),
        (b".text", b"\xb8" + bytes(4) + b"\xc3" if overlap else b"\xc3",
         ((1, 8, 6),) if overlap else (), 0, 0, 0x60500020, b"_cb", 32, 2),
    ]
    if extra:
        specs.append((b".extra", bytes(16) if extra == "data" else b"\xc3", (),
                      1, 5, 0x40401040 if extra == "data" else 0x60501020, b"$unused", 0, 6))
    start = 20 + len(specs) * 40
    payload, records, headers = bytearray(), bytearray(), []
    for number, (name, body, edges, parent, selection, flags, label, kind, storage) in enumerate(specs, 1):
        raw = body + b"\xcc" * (-len(body) % 16)
        at = start + len(payload)
        payload.extend(raw)
        reloc_at = start + len(payload) if edges else 0
        payload.extend(b"".join(struct.pack("<IIH", *row) for row in edges))
        headers.append(name.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(raw), at,
                                                         reloc_at, 0, len(edges), 0, flags))
        records.extend(name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, 0, 3, 1))
        records.extend(struct.pack("<IHHIHB3x", len(raw), len(edges), 0, 0, parent, selection))
        records.extend(label.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, kind, storage, 0))
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(payload), len(records) // 18, 0, 0)
            + b"".join(headers) + payload + records + struct.pack("<I", 4))


def physical_family(root, *, callback=False, overlap=False, extra=None, convergence=0x2000,
                    code_flags=0x60000020, data_flags=0x40000040, missing=None,
                    damage=None, missing_highlow=None):
    """Build independent selected COFF, map, PE bytes, NB10 and PDB fixups."""
    object_raw = family_object(callback=callback, overlap=overlap, extra=extra)
    obj = Coff(object_raw)
    evidence, log = inputs(root)
    path = root / "app.obj"
    path.write_bytes(object_raw)
    row = next(row for row in evidence["files"] if row["path"] == str(path))
    row["before"] = row["after"] = stamp(object_raw)
    objects = SelectedObjects(*inventory(evidence, log))
    callback_rva = 0x1020 if overlap else 0x1080
    records = [(20, 0, 0x1001, 0x1020), (6, 0, 0x1006, 0x2000),
               (6, 0, 0x1021, convergence), (6, 0, 0x2000, callback_rva if callback else 0x1020)]
    original, streams = parts(tuple(record for record in records if record[2] != missing))
    payload = original[1024:]
    raw = bytearray(original[:1024] + bytes(512) + payload)
    struct.pack_into("<H", raw, 0x86, 2)
    struct.pack_into("<I", raw, 0x98 + 16, 0x1000)
    struct.pack_into("<I", raw, 0x178 + 36, code_flags)
    raw[0x1A0:0x1C8] = b".data\0\0\0" + struct.pack("<IIIIIIHHI", 512, 0x2000, 512, 1024, 0, 0, 0, 0, data_flags)
    struct.pack_into("<I", raw, 0x300 + 24, 1536)
    streams[5] = bytes(raw[0x178:0x1C8])
    for number, at in ((1, 512), (2, 544), (3, 1024), (4, 512 + callback_rva - 0x1000)):
        section = obj.sections[number - 1]
        raw[at:at + len(section.data)] = section.data
    for kind, _, site, target in records:
        at = 512 + site - 0x1000 if site < 0x2000 else 1024 + site - 0x2000
        struct.pack_into("<I", raw, at, (target - site - 4 if kind == 20 else 0x400000 + target) & 0xFFFFFFFF)
    highlow = [(site // 4096 * 4096, 0x3000 + site % 4096)
               for kind, _, site, _ in records if kind == 6 and site != missing_highlow]
    relocation_bytes = bytearray()
    for page in sorted({page for page, _ in highlow}):
        entries = [entry for block, entry in highlow if block == page]
        entries += [0] * (len(entries) % 2)
        relocation_bytes.extend(struct.pack("<II", page, 8 + len(entries) * 2))
        relocation_bytes.extend(struct.pack("<" + "H" * len(entries), *entries))
    raw[896:896 + len(relocation_bytes)] = relocation_bytes
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1180, len(relocation_bytes))
    if damage is not None:
        raw[damage] ^= 1
    image = PeImage(bytes(raw), sha256(raw).hexdigest())
    fixups = bound_fixups(image, small_msf(streams))
    mapping = parse_link_map((
        "synthetic\n Preferred load address is 00400000\n"
        " Address         Publics by Value              Rva+Base     Lib:Object\n"
        " 0001:00000000 _f 00401000 f app.obj\n"
        f" 0001:{callback_rva - 0x1000:08x} _cb {0x400000 + callback_rva:08x} f app.obj\n"
        " entry point at 0001:00000000\n").encode())
    return objects, mapping, image, fixups


def observe(values, roots=("_f",)):
    objects, mapping, image, fixups = values
    return observe_code_graph(objects, mapping, image,
                              [{"owner": "app.obj", "symbol": name} for name in roots], fixups=fixups)


def test_associated_code_and_data_paths_converge_without_operand_inference(tmp_path):
    report = observe(physical_family(tmp_path))
    assert report["result"] == "pass", report["gaps"]
    regions = report["nodes"][0]["proof"]["regions"]
    assert [row["address"] for row in regions] == [0x401000, 0x401020, 0x402000]
    assert all(row["code"]["result"] == "pass" for row in regions)
    assert report["remaining_linked_effects"] == "unknown" and not report["ready_for_collection"]


@pytest.mark.parametrize("options", [{"code_flags": 0x40000020}, {"code_flags": 0x40000040},
                                      {"data_flags": 0x60000040}, {"data_flags": 0x60000020}])
def test_pe_permissions_and_code_data_kind_cannot_disagree_with_owned_regions(tmp_path, options):
    report = observe(physical_family(tmp_path, **options))
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None
    assert any("section kind" in gap["reason"] for gap in report["gaps"])


def test_independent_paths_cannot_place_one_child_at_two_different_bases(tmp_path):
    report = observe(physical_family(tmp_path, convergence=0x2004))
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None
    assert any("paths disagree" in gap["reason"] for gap in report["gaps"])


@pytest.mark.parametrize("roots", [("_f", "_cb"), ("_cb", "_f")])
def test_ordinary_and_associated_ownership_overlap_refuses_in_both_root_orders(tmp_path, roots):
    report = observe(physical_family(tmp_path, overlap=True), roots)
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None
    assert any("overlap" in gap["reason"] for gap in report["gaps"])
    assert all(row["proof"]["result"] == "pass" for row in report["nodes"])


@pytest.mark.parametrize("root", ["_f", "_cb"])
def test_overlap_fixture_each_owner_has_independently_verifiable_code(tmp_path, root):
    report = observe(physical_family(tmp_path, overlap=True), (root,))
    assert report["result"] == "pass", report["gaps"]
    assert len(report["nodes"]) == 1


@pytest.mark.parametrize("extra", ["code", "data"])
def test_unreachable_associated_children_are_not_dropped_from_the_family(tmp_path, extra):
    report = observe(physical_family(tmp_path, extra=extra))
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None
    assert any("lack independent" in gap["reason"] for gap in report["gaps"])


@pytest.mark.parametrize("site", [0x1001, 0x1006, 0x1021, 0x2000])
def test_each_internal_fixup_is_required_even_with_an_alternative_placement_path(tmp_path, site):
    report = observe(physical_family(tmp_path, missing=site))
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None


@pytest.mark.parametrize("site", [0x1006, 0x1021, 0x2000])
def test_each_image_address_still_requires_its_loader_fixup(tmp_path, site):
    report = observe(physical_family(tmp_path, missing_highlow=site))
    assert report["result"] == "fail" and report["graph_fingerprint"] is None


@pytest.mark.parametrize("offset", [512 + 10, 544 + 5, 1024 + 12])
def test_every_root_child_and_associated_data_byte_is_compared(tmp_path, offset):
    report = observe(physical_family(tmp_path, damage=offset))
    assert report["result"] == "fail" and report["graph_fingerprint"] is None


def test_callback_reached_through_associated_data_is_a_real_outward_graph_dependency(tmp_path):
    report = observe(physical_family(tmp_path, callback=True))
    assert report["result"] == "pass", report["gaps"]
    assert [row["symbol"] for row in report["nodes"]] == ["_f", "_cb"]
    data_edges = report["nodes"][0]["edges"][2]["edges"]
    assert data_edges[0]["target"] == {"node": 1, "external": "_cb"}
    assert report["remaining_linked_effects"] == "unknown" and not report["ready_for_collection"]


def test_changed_callback_cannot_hide_behind_verified_associated_data(tmp_path):
    report = observe(physical_family(tmp_path, callback=True, damage=512 + 0x80))
    assert report["result"] == "fail" and report["graph_fingerprint"] is None
    callback = next(row for row in report["nodes"] if row["symbol"] == "_cb")
    assert callback["result"] == "fail"
