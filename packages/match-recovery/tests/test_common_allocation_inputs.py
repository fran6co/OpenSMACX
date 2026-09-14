"""Parsed common-storage observations preserve the physical bytes API checks."""

from copy import deepcopy
import hashlib
import json
import struct

import pytest

from match_recovery import common_allocation as native
from match_recovery.link_file_evidence import parse_link_files
from match_recovery.link_input_coverage import _input_inventory
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pe_coff import PeImage
from test_coff_inventory import common_object, imported, weak_object
from test_common_allocation import fixture, replace_library, verify
from test_link_file_evidence import stamp
from test_toolchain_components import archive


def parsed(values):
    evidence, log, image, mapping = values
    data, log_data = json.dumps(evidence).encode(), log.encode()
    coverage, records = _input_inventory(data, log_data)
    return (parse_link_files(data), coverage, records,
            PeImage(image, hashlib.sha256(image).hexdigest()), parse_link_map(mapping.encode()))


def helper(values, names=("_g",)):
    return native._verify_common_inputs(*parsed(values), names)


def test_native_parsed_input_path_exactly_matches_the_bytes_wrapper(tmp_path):
    values = fixture(tmp_path)
    report = helper(values)
    wrapped = verify(*values)
    assert {key: value for key, value in wrapped.items() if key != "map_sha256"} == report
    assert wrapped["map_sha256"] == hashlib.sha256(values[3].encode()).hexdigest()
    assert report["result"] == "pass"
    assert report["original_verification"] == report["runtime_verification"] == "unknown"
    assert not report["ready_for_collection"] and not report["persisted"]


def test_parsed_observation_does_not_reparse_files_or_mutate_inputs(tmp_path, monkeypatch):
    values = parsed(fixture(tmp_path))
    before = deepcopy((values[:3], values[3].__dict__, values[4]))
    def unexpected(*args, **kwargs):
        pytest.fail("parsed helper attempted to repeat native input parsing")
    monkeypatch.setattr(native, "_input_inventory", unexpected)
    monkeypatch.setattr(native, "parse_link_files", unexpected)
    monkeypatch.setattr(native, "parse_link_map", unexpected)
    monkeypatch.setattr(native, "PeImage", unexpected)
    assert native._verify_common_inputs(*values, ("_g",))["result"] == "pass"
    assert (values[:3], values[3].__dict__, values[4]) == before


@pytest.mark.parametrize("damage", ["strong", "absolute", "weak", "weak_fallback", "conflicting_size",
                                    "unsupported_record", "ambiguous_member", "unselected", "policy",
                                    "owner", "function", "overlap", "readonly", "executable", "huge",
                                    "nonzero", "highlow", "straddling_highlow", "stale_input"])
def test_existing_physical_refusals_remain_identical_through_both_paths(tmp_path, damage):
    evidence, log, image, mapping = fixture(tmp_path)
    if damage in {"strong", "absolute", "weak", "weak_fallback", "conflicting_size", "unsupported_record", "ambiguous_member", "huge"}:
        extra = bytearray(common_object())
        at = struct.unpack_from("<I", extra, 8)[0] + 18
        member = "unused.obj/"
        if damage in {"strong", "absolute"}:
            struct.pack_into("<h", extra, at + 12, 1 if damage == "strong" else -1)
        elif damage in {"weak", "weak_fallback"}:
            extra = bytearray(weak_object())
            if damage == "weak_fallback":
                extra = extra.replace(b"_g\0", b"_z\0").replace(b"_f\0", b"_g\0")
        elif damage == "conflicting_size": extra = bytearray(common_object(8))
        elif damage == "huge":
            # A uniform enormous request still cannot extend the PE section.
            extra = bytearray(common_object(0xFFFFFFFF))
        elif damage == "unsupported_record": extra = bytearray(b"unsupported COFF")
        else: member = "runtime.obj/"
        replace_library(tmp_path, evidence, [(member, bytes(extra))])
        if damage == "huge":
            path = tmp_path / "runtime.lib"
            path.write_bytes(archive([("runtime.obj/", bytes(extra)), ("DEMO.dll/", imported())]))
            evidence["files"][0]["before"] = evidence["files"][0]["after"] = stamp(path.read_bytes())
    elif damage == "unselected":
        log = log.replace("RUNTIME.lib(runtime.obj)\n", "")
        evidence["link_log_sha256"] = hashlib.sha256(log.encode()).hexdigest()
    elif damage == "policy": evidence["arguments"].append("/FORCE")
    elif damage == "owner": mapping = mapping.replace("<common>", "another.obj")
    elif damage == "function": mapping = mapping.replace("<common>", "f <common>")
    elif damage == "overlap":
        mapping = mapping.replace(" entry point", " 0002:00000001 _alias 00402001 another.obj\n entry point")
    elif damage in {"readonly", "executable"}:
        raw = bytearray(image)
        struct.pack_into("<I", raw, 0x1A0 + 36, 0x40000080 if damage == "readonly" else 0xE0000080)
        image = bytes(raw)
    elif damage == "nonzero":
        raw = bytearray(image)
        offset = len(raw)
        raw.extend(b"\x01" + bytes(63))
        struct.pack_into("<II", raw, 0x1A0 + 16, 64, offset)
        image = bytes(raw)
    elif damage in {"highlow", "straddling_highlow"}:
        site = 0x402000 if damage == "highlow" else 0x401FFE
        raw = bytearray(image)
        struct.pack_into("<I", raw, 0x98 + 92, 16)
        struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1040, 12)
        struct.pack_into("<IIHH", raw, 512 + 0x40, (site - 0x400000) & ~0xFFF, 12, 0x3000 | (site & 0xFFF), 0)
        image = bytes(raw)
    elif damage == "stale_input": (tmp_path / "runtime.lib").write_bytes(b"changed")
    values = evidence, log, image, mapping
    wrapped = verify(*values)
    report = helper(values)
    assert {key: value for key, value in wrapped.items() if key != "map_sha256"} == report
    assert report["result"] == ("fail" if damage == "nonzero" else "unknown")


@pytest.mark.parametrize("damage", ["coverage", "missing_declarations", "selection"])
def test_observed_missing_prerequisites_do_not_become_storage_proof(tmp_path, damage):
    evidence, coverage, records, image, mapping = parsed(fixture(tmp_path))
    if damage == "coverage": coverage["result"] = "unknown"
    elif damage == "missing_declarations":
        records = [{**row, "symbols": [symbol for symbol in row["symbols"] if symbol["declaration"] != "common"]}
                   for row in records]
    else:
        for item in coverage["inputs"]: item["selection"] = "unknown"
    report = native._verify_common_inputs(evidence, coverage, records, image, mapping, ("_g",))
    assert report["result"] == "unknown"


@pytest.mark.parametrize("names", [(), ("_g", "_g"), ("",), ("bad name",), (True,)])
def test_both_interfaces_reject_invalid_symbol_queries(tmp_path, names):
    values = fixture(tmp_path)
    with pytest.raises(ValueError, match="unique nonempty"): verify(*values, names)
    with pytest.raises(ValueError, match="unique nonempty"): helper(values, names)
