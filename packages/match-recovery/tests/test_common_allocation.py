"""Synthetic selected storage and refusal controls; no original image bytes."""

import hashlib
import json
import struct

import pytest

from match_recovery.common_allocation import verify_common_allocations
from test_binary import coff_fixture
from test_coff_inventory import common_object, imported, weak_object
from test_link_input_coverage import inputs
from test_link_file_evidence import stamp
from test_link_map import MAP, executable
from test_toolchain_components import archive


def fixture(root):
    evidence, log = inputs(root)
    image = bytearray(executable())
    # Keep the code section/entry, add a separate writable data section. The
    # common occupies four bytes out of a much larger zero-initialized region.
    struct.pack_into("<H", image, 0x84 + 2, 2)
    image[0x1A0:0x1C8] = struct.pack("<8sIIIIIIHHI", b".data", 64, 0x2000, 0, 0, 0, 0, 0, 0, 0xC0000080)
    mapping = MAP.replace("0001:00000020 _g 00401020 f application.obj", "0002:00000000 _g 00402000 <common>")
    return evidence, log, bytes(image), mapping


def verify(evidence, log, image, mapping, names=("_g",)):
    return verify_common_allocations(json.dumps(evidence).encode(), log.encode(), image, mapping.encode(), names)


def replace_library(root, evidence, members):
    path = root / "runtime.lib"
    path.write_bytes(archive([("runtime.obj/", common_object()), ("DEMO.dll/", imported()), *members]))
    evidence["files"][0]["before"] = evidence["files"][0]["after"] = stamp(path.read_bytes())


def test_uniform_common_request_proves_bounded_candidate_storage_only(tmp_path):
    report = verify(*fixture(tmp_path))
    assert report["result"] == "pass", report
    row, = report["allocations"]
    assert (row["symbol"], row["address"], row["size"], row["initialization"]) == ("_g", 0x402000, 4, "pass")
    assert report["original_verification"] == report["runtime_verification"] == "unknown"
    assert report["input_coverage"]["allocation_verification"] == "unknown"
    assert not report["persisted"] and not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["strong", "absolute", "weak", "weak_target", "import", "import_slot",
                                    "size", "unsupported", "auxiliary", "function_type"])
def test_even_unselected_conflicts_block_a_common_allocation(tmp_path, damage):
    evidence, log, image, mapping = fixture(tmp_path)
    extra = bytearray(common_object())
    at = struct.unpack_from("<I", extra, 8)[0] + 18
    names = ("_g",)
    if damage in {"strong", "absolute"}:
        struct.pack_into("<h", extra, at + 12, 1 if damage == "strong" else -1)
    elif damage in {"weak", "weak_target"}:
        extra = bytearray(weak_object())
        if damage == "weak_target":
            # Keep the queried name as a fallback, not the weak symbol itself.
            extra = extra.replace(b"_g\0", b"_z\0").replace(b"_f\0", b"_g\0")
    elif damage in {"import", "import_slot"}:
        extra = bytearray(imported("_g", naming=1))
        if damage == "import_slot":
            # Query the IAT name and supply a common declaration for that name.
            names = ("__imp__g",)
            common = bytearray(common_object())
            common_at = struct.unpack_from("<I", common, 8)[0] + 18
            common[common_at:common_at + 8] = b"__imp__g"
            path = tmp_path / "slot.obj"
            path.write_bytes(common)
            evidence["explicit_inputs"].append(str(path))
            evidence["files"].append({"path": str(path), "before": stamp(common), "after": stamp(common)})
            mapping = mapping.replace(" _g ", " __imp__g ")
    elif damage == "size":
        extra = bytearray(common_object(8))
    elif damage == "unsupported":
        extra = bytearray(b"unsupported")
    elif damage == "auxiliary":
        symbol_offset, count = struct.unpack_from("<II", extra, 8)
        extra[at + 17] = 1
        extra[symbol_offset + count * 18:symbol_offset + count * 18] = bytes(18)
        struct.pack_into("<I", extra, 12, count + 1)
    else:
        struct.pack_into("<H", extra, at + 14, 0x20)
    replace_library(tmp_path, evidence, [("unused.obj/", bytes(extra))])
    report = verify(evidence, log, image, mapping, names)
    assert report["result"] == "unknown", report


@pytest.mark.parametrize("option", ["/FORCE", "/ALTERNATENAME:_g=_else", "/DEF:exports.def",
                                     "/SECTION:.data,RW", "/unknown:new", "@args.rsp", "/incremental:yes"])
def test_unsupported_command_policy_is_never_ignored(tmp_path, option):
    evidence, log, image, mapping = fixture(tmp_path)
    evidence["arguments"].append(option)
    report = verify(evidence, log, image, mapping)
    assert report["result"] == "unknown" and report["policy_gaps"]


@pytest.mark.parametrize("text,expected", [(' /DEFAULTLIB:"runtime" /MERGE:.CRT=.data ', "pass"),
    (' /EXPORT:_f ', "pass"), (' /EXPORT:_g,DATA ', "pass"), (' /EXPORT:_alias=_g ', "unknown"),
    (' /EXPORT:_alias=_g,DATA ', "unknown"), (' /EXPORT:_g,DATA,DATA ', "unknown"),
    (' /ALTERNATENAME:_g=_f ', "unknown"), (' /DEFAULTLIB:"unclosed ', "unknown"),
    (' /DEFAULTLIB:runtime\0/ALTERNATENAME:_g=_f ', "unknown")])
def test_directive_policy_includes_unselected_records(tmp_path, text, expected):
    evidence, log, image, mapping = fixture(tmp_path)
    extra = bytearray(coff_fixture(text.encode()))
    extra[20:28] = b".drectve"
    size = struct.unpack_from("<I", extra, 20 + 16)[0]
    extra[60 + len(text):60 + size] = b" " * (size - len(text))
    replace_library(tmp_path, evidence, [("directives.obj/", bytes(extra))])
    assert verify(evidence, log, image, mapping)["result"] == expected


@pytest.mark.parametrize("damage", ["owner", "function", "missing", "overlap", "readonly", "executable", "huge", "unselected"])
def test_map_and_storage_are_required_not_inferred_from_declarations(tmp_path, damage):
    evidence, log, image, mapping = fixture(tmp_path)
    if damage == "owner":
        mapping = mapping.replace("<common>", "other.obj")
    elif damage == "function":
        mapping = mapping.replace("<common>", "f <common>")
    elif damage == "missing":
        mapping = mapping.replace("_g", "_else")
    elif damage == "overlap":
        mapping = mapping.replace(" entry point", " 0002:00000001 _inside 00402001 another.obj\n entry point")
    elif damage in {"readonly", "executable"}:
        image = bytearray(image)
        struct.pack_into("<I", image, 0x1A0 + 36, 0x40000080 if damage == "readonly" else 0xE0000080)
        image = bytes(image)
    elif damage == "huge":
        path = tmp_path / "runtime.lib"
        path.write_bytes(archive([("runtime.obj/", common_object(0xFFFFFFFF)), ("DEMO.dll/", imported())]))
        evidence["files"][0]["before"] = evidence["files"][0]["after"] = stamp(path.read_bytes())
    else:
        log = log.replace("RUNTIME.lib(runtime.obj)\n", "")
        evidence["link_log_sha256"] = hashlib.sha256(log.encode()).hexdigest()
    assert verify(evidence, log, image, mapping)["result"] == "unknown"


def test_nonzero_initial_contents_are_a_counterexample(tmp_path):
    evidence, log, image, mapping = fixture(tmp_path)
    raw = bytearray(image)
    offset = len(raw)
    raw.extend(b"\x01" + bytes(63))
    struct.pack_into("<II", raw, 0x1A0 + 16, 64, offset)
    assert verify(evidence, log, bytes(raw), mapping)["result"] == "fail"


def test_changed_physical_input_cannot_replay_earlier_absence_evidence(tmp_path):
    values = fixture(tmp_path)
    assert verify(*values)["result"] == "pass"
    (tmp_path / "runtime.lib").write_bytes(b"changed")
    assert verify(*values)["result"] == "unknown"


def test_equal_requests_do_not_need_to_guess_an_unselected_member(tmp_path):
    evidence, log, image, mapping = fixture(tmp_path)
    replace_library(tmp_path, evidence, [("unused.obj/", common_object())])
    report = verify(evidence, log, image, mapping)
    assert report["result"] == "pass" and len(report["allocations"][0]["requests"]) == 2


def test_common_only_explicit_object_can_participate_without_a_final_label(tmp_path):
    evidence, log, image, mapping = fixture(tmp_path)
    path = tmp_path / "common-only.obj"
    path.write_bytes(common_object())
    evidence["explicit_inputs"].append(str(path))
    evidence["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
    log = log.replace("RUNTIME.lib(runtime.obj)\n", "")
    evidence["link_log_sha256"] = hashlib.sha256(log.encode()).hexdigest()
    assert verify(evidence, log, image, mapping)["result"] == "pass"


def test_ambiguous_final_archive_member_cannot_establish_participating_request(tmp_path):
    evidence, log, image, mapping = fixture(tmp_path)
    replace_library(tmp_path, evidence, [("runtime.obj/", common_object())])
    assert verify(evidence, log, image, mapping)["result"] == "unknown"


@pytest.mark.parametrize("site", [0x402000, 0x401FFE, 0x402002])
def test_contained_or_straddling_loader_relocation_refuses_common_storage(tmp_path, site):
    evidence, log, image, mapping = fixture(tmp_path)
    raw = bytearray(image)
    # PE relocation table in the already-backed first section; HIGHLOW must
    # not initialize or straddle an otherwise zero-filled common allocation.
    struct.pack_into("<I", raw, 0x98 + 92, 16)
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1040, 12)
    struct.pack_into("<IIHH", raw, 512 + 0x40, (site - 0x400000) & ~0xFFF, 12, 0x3000 | (site & 0xFFF), 0)
    assert verify(evidence, log, bytes(raw), mapping)["result"] == "unknown"


@pytest.mark.parametrize("names", [(), ("_g", "_g"), ("",), ("bad name",)])
def test_empty_duplicate_or_invalid_queries_refuse(tmp_path, names):
    with pytest.raises(ValueError, match="unique nonempty"):
        verify(*fixture(tmp_path), names)
