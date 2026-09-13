"""Private outward fixups retain selected-object and independent-image identity."""

from hashlib import sha256
import struct

import pytest

from match_recovery.linked_bundle import verify_bundle
from match_recovery.linked_fixups import local_data_bindings
from match_recovery.linked_objects import SelectedObjects
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.pe_coff import Coff, PeImage
from test_link_file_evidence import stamp
from test_link_input_coverage import inputs
from test_linked_initializers import build
from test_linked_objects import inventory
from test_msf import small_msf
from test_pdb_fixups import parts


def private_object(*, code_target=False, duplicate=False):
    references = ((1, 5, 20), (6, 8, 6), (11, 9 if duplicate else 8, 6))
    specs = [(b".text", b"\xe8" + bytes(4) + b"\x68" + bytes(4) + b"\x68" + bytes(4) + b"\xc3",
              references, 0, 2, 0x60501020),
             (b".text$x", b"\xc3" + b"\xcc" * 15, (), 1, 5, 0x60501020),
             (b".data", bytes(16), (), 0, 0, 0x60500020 if code_target else 0xC0300040)]
    headers, payload, symbols = [], bytearray(), bytearray()
    start = 20 + len(specs) * 40
    for number, (name, data, relocations, parent, selection, flags) in enumerate(specs, 1):
        at = start + len(payload)
        payload.extend(data)
        reloc_at = start + len(payload) if relocations else 0
        for row in relocations:
            payload.extend(struct.pack("<IIH", *row))
        headers.append(name.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(data), at,
                       reloc_at, 0, len(relocations), 0, flags))
        symbols.extend(name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, 0, 3, 1))
        symbols.extend(struct.pack("<IHHIHB3x", len(data), len(relocations), 0, 0, parent, selection))
        name = (b"_f", b"$tail", b"private")[number - 1]
        symbols.extend(name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number,
                       32 if number == 1 else 0, 2 if number == 1 else 3, 0))
    if duplicate:
        symbols.extend(b"private\0" + struct.pack("<IhHBB", 0, 3, 0, 3, 0))
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(payload), len(symbols) // 18, 0, 0)
            + b"".join(headers) + payload + symbols + struct.pack("<I", 4))


def fixture(root, *, second_target=0x2100, code_target=False, duplicate=False, map_row=None):
    raw_object = private_object(code_target=code_target, duplicate=duplicate)
    evidence, log = inputs(root)
    path = root / "app.obj"
    path.write_bytes(raw_object)
    row = next(row for row in evidence["files"] if row["path"] == str(path))
    row["before"] = row["after"] = stamp(raw_object)
    objects = SelectedObjects(*inventory(evidence, log))
    context = build(root / "image")
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[512:528] = (b"\xe8\x1b\0\0\0\x68" + struct.pack("<I", 0x402100)
                    + b"\x68" + struct.pack("<I", 0x400000 + second_target) + b"\xc3")
    raw[544:560] = b"\xc3" + b"\xcc" * 15
    # Root absolute operands and the existing initializer table have loader records.
    relocations = (struct.pack("<IIHH", 0x1000, 12, 0x3006, 0x300B)
                   + struct.pack("<IIHH", 0x2000, 12, 0x3004, 0))
    raw[512 + 0x180:512 + 0x180 + len(relocations)] = relocations
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1180, len(relocations))
    reference, streams = parts(((20, 0, 0x1001, 0x1020), (6, 0, 0x1006, 0x2100),
                                (6, 0, 0x100B, second_target)))
    payload = reference[0x400:]
    at = len(raw)
    raw.extend(payload)
    struct.pack_into("<I", raw, 0x98 + 60, 0x200)
    struct.pack_into("<II", raw, 0x98 + 96 + 6 * 8, 0x1100, 28)
    struct.pack_into("<IIHHIIII", raw, 0x300, 0, 0, 0, 0, 2, len(payload), 0, at)
    streams[5] = bytes(raw[0x178:0x1C8])
    image = PeImage(bytes(raw), sha256(raw).hexdigest())
    fixups = bound_fixups(image, small_msf(streams))
    mapping = ("synthetic\n Preferred load address is 00400000\n"
               " Address         Publics by Value              Rva+Base     Lib:Object\n"
               " 0001:00000000 _f 00401000 f app.obj\n"
               " entry point at 0001:00000000\n Static symbols\n")
    if map_row:
        mapping += map_row + "\n"
    return Coff(raw_object), objects, image, fixups, parse_link_map(mapping.encode())


def observe(values):
    obj, objects, image, fixups, mapping = values
    return verify_bundle(obj, obj.function("_f"), "app.obj", mapping, image,
                         objects=objects, fixups=fixups)


def local(values, *, obj=None, image=None, names=("private",)):
    original, objects, original_image, fixups, mapping = values
    obj = original if obj is None else obj
    image = original_image if image is None else image
    return local_data_bindings(obj, obj.function("_f"), "app.obj", mapping, image,
                               names, objects, fixups)


def test_selected_private_data_binding_preserves_nonacceptance_boundary(tmp_path):
    values = fixture(tmp_path)
    binding = local(values)["private"]
    assert binding["address"] == 0x402100 and binding["placement"] == "pdb_fixup"
    assert binding["object_section"] == 3 and binding["object_index"] == 8
    assert binding["object_sha256"] == sha256(private_object()).hexdigest()
    assert binding["input"] == {"path": str(tmp_path / "app.obj")}
    assert "result" not in binding and "contents" not in binding
    proof = observe(values)
    assert proof["result"] == "pass"
    assert proof["target_contents"] == "unknown"
    refs = proof["regions"][0]["references"][1:]
    assert all(row["private_binding"] == binding for row in refs)


@pytest.mark.parametrize("mode", ["local", "bundle"])
@pytest.mark.parametrize("options,reason", [({"second_target": 0x2104}, "disagree"),
                                            ({"duplicate": True}, "disagree"),
                                            ({"code_target": True}, "unverified code")])
def test_contradictory_repeated_symbol_or_unsupported_code_refuses(tmp_path, mode, options, reason):
    values = fixture(tmp_path, **options)
    with pytest.raises(ValueError, match=reason):
        (local if mode == "local" else observe)(values)


@pytest.mark.parametrize("row", [" 0002:00000104 private 00402104 app.obj",
    " 0002:00000100 private 00402100 app.obj\n 0002:00000100 private 00402100 app.obj"])
def test_bundle_contradictory_or_duplicate_map_identity_refuses(tmp_path, row):
    with pytest.raises(ValueError, match="consistent independent map identity|duplicate MSVC link map"):
        observe(fixture(tmp_path, map_row=row))


def test_selected_referring_object_must_match_physical_input(tmp_path):
    values = fixture(tmp_path)
    with pytest.raises(ValueError, match="differs from its selected physical input"):
        local(values, obj=Coff(private_object(duplicate=True)))


def test_private_fixups_cannot_be_replayed_against_a_changed_image(tmp_path):
    values = fixture(tmp_path)
    raw = bytearray(values[2].data)
    raw[600] ^= 1
    with pytest.raises(ValueError, match="exact image"):
        local(values, image=PeImage(bytes(raw), sha256(raw).hexdigest()))


def test_private_binding_batch_cannot_partially_cover_missing_names(tmp_path):
    with pytest.raises(ValueError, match="every requested symbol"):
        local(fixture(tmp_path), names=("private", "missing"))


@pytest.mark.parametrize("mode", ["local", "bundle"])
def test_other_owner_map_evidence_is_not_hidden_by_private_fallback(tmp_path, mode):
    values = fixture(tmp_path, map_row=" 0002:00000100 private 00402100 other.obj")
    with pytest.raises(ValueError, match="conflicts with existing linked map"):
        (local if mode == "local" else observe)(values)
