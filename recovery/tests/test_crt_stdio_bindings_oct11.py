"""Bounded stdio dependency attribution, not application or runtime acceptance."""
from dataclasses import replace
import hashlib
import json
import os
from pathlib import Path
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.import_source import read_imports
from match_recovery.model import Span
from match_recovery.pe_coff import Coff
from match_recovery.source import read_text
from test_crt_eight_entry_bindings_oct10 import repin, raw_offset

ROOT = Path(__file__).resolve().parents[2]
COMPANION = "src/vendor/vc6-runtime.recovery.h"
BASE = "fdd976176ef1b5cb07bdfd6b16dc771377e4af19"
ADDITION = "239910dc8"
EXPECTED = (
    (0x64603F, 0x64606E, "_fwrite", "build/intel/mt_obj/fwrite.obj", 1),
    (0x64606E, 0x646178, "__fwrite_lk", "build/intel/mt_obj/fwrite.obj", 2),
    (0x647C37, 0x647C66, "__lock_file", "build/intel/mt_obj/_file.obj", 9),
    (0x647C89, 0x647CB8, "__unlock_file", "build/intel/mt_obj/_file.obj", 13),
    (0x6494D1, 0x6494FF, "__fflush_lk", "build/intel/mt_obj/fflush.obj", 3),
    (0x6494FF, 0x64955B, "__flush", "build/intel/mt_obj/fflush.obj", 5),
    (0x64955B, 0x649564, "__flushall", "build/intel/mt_obj/fflush.obj", 7),
    (0x649564, 0x649608, "_flsall", "build/intel/mt_obj/fflush.obj", 9),
    (0x649D94, 0x649EAC, "__flsbuf", "build/intel/mt_obj/_flsbuf.obj", 1),
    (0x649EAC, 0x649F11, "__write", "build/intel/mt_obj/write.obj", 1),
    (0x649F11, 0x64A09C, "__write_lk", "build/intel/mt_obj/write.obj", 3),
)
ADDRESSES = {item[0] for item in EXPECTED}
IMPORTS = (
    "// RECOVERY_IMPORT: v1 __imp__WriteFile@20 KERNEL32.dll name:WriteFile",
    "// RECOVERY_IMPORT: v1 __imp__GetLastError@0 KERNEL32.dll name:GetLastError",
)


def test_existing_bindings_claims_and_source_preserved():
    old = subprocess.check_output(["git", "show", BASE + ":" + COMPANION],
                                  cwd=ROOT, text=True)
    text = subprocess.check_output(["git", "show", ADDITION + ":" + COMPANION],
                                   cwd=ROOT, text=True)
    manifest = parse_component(text)
    installed = parse_component((ROOT / COMPANION).read_text())
    assert replace(installed, bindings=installed.bindings[:len(manifest.bindings)]) == manifest
    additions = tuple(b for b in manifest.bindings if b.origin.address in ADDRESSES)
    assert tuple((b.origin.address, b.spans[0].end, b.symbol, b.unit)
                 for b in additions) == tuple(item[:4] for item in EXPECTED)
    assert replace(manifest, bindings=tuple(b for b in manifest.bindings
                   if b.origin.address not in ADDRESSES)) == parse_component(old)
    assert [replace(r, line=1, region="") for r in read_text(text, ROOT / COMPANION)] == [
        replace(r, line=1, region="") for r in read_text(old, ROOT / COMPANION)]
    added_lines = [line for line in text.splitlines()
                   if line.startswith("//     {") and '"unit"' in line
                   and int(json.loads(line[3:].rstrip(","))["origin"], 16) in ADDRESSES]
    assert len(added_lines) == 11
    for line in added_lines + list(IMPORTS):
        assert text.count(line + "\n") == 1
        text = text.replace(line + "\n", "", 1)
    last = next(line for line in reversed(old.splitlines())
                if '"origin":' in line and '"unit":' in line)
    assert text.replace(last + ",\n", last + "\n", 1) == old


@pytest.fixture(scope="module")
def inputs():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE",
                                     ROOT / ".opensmacx/game/terranx_original.exe"))
    archive_path = Path("/home/fran6co/opt/vc6/LIB/LIBCMT.LIB")
    if not image_path.is_file() or not archive_path.is_file():
        pytest.skip("external pinned image/runtime archive unavailable")
    manifest = parse_component(subprocess.check_output(
        ["git", "show", ADDITION + ":" + COMPANION], cwd=ROOT, text=True))
    image, archive = image_path.read_bytes(), archive_path.read_bytes()
    assert hashlib.sha256(image).hexdigest() == manifest.bindings[0].origin.image_sha256
    assert hashlib.sha256(archive).hexdigest() == manifest.archive_sha256
    return manifest, image, archive, read_imports(ROOT)


def verify(inputs, manifest=None, image=None):
    m, i, archive, imports = inputs
    return verify_toolchain(m if manifest is None else manifest,
                            i if image is None else image, archive,
                            import_bindings=imports)


def row(report, address):
    return next(r for r in report["functions"] if int(r["address"], 16) == address)


def test_independent_complete_physical_members(inputs):
    _, _, archive, _ = inputs
    members = CoffArchive(archive)
    hashes = {
        "fwrite": "d2f82c08be3cdd06ed629201808be63520a540c6e6fc69160b5d983236d2cc03",
        "_file": "cde568d9abdcb6cbb7be79d229deb579e504d9ce01b780076fa9ecba85d37bcf",
        "fflush": "c835e4f1d3c0fda66035da10c1b0d0ea1bf670783696aff1025aa31f62131a46",
        "_flsbuf": "0e9b2e26313b127cd2cd0f8dd8ff397ea29449c3a6b4e0830f95eca0c81d6790",
        "write": "ac8b6a970dd35191ec36f65865b2cb23609e13cdeceeebac05697a723f4ae22b",
    }
    for address, end, symbol, unit, section in EXPECTED:
        data = members.member(unit)
        assert hashlib.sha256(data).hexdigest() == hashes[Path(unit).stem]
        obj = Coff(data)
        function = obj.function(symbol, comdat=False)
        assert function.section == section and function.value == 0
        assert len(obj.subject(function)[0]) == end - address


def test_complete_boundaries_and_reference_limitations(inputs):
    report = verify(inputs)
    for address, end, symbol, unit, section in EXPECTED:
        result = row(report, address)
        assert result["boundary"] == "pass"
        assert not result["differing_offsets"]
        assert result["result"] in ("pass", "unknown")
        assert all(r["result"] in ("pass", "unknown") for r in result["relocations"])
    for address in (0x64603F, 0x6494FF, 0x64955B):
        result = row(report, address)
        assert all(result[k] == "pass" for k in ("result", "boundary", "bytes", "references"))
    write = row(report, 0x649F11)
    for symbol in ("__imp__WriteFile@20", "__imp__GetLastError@0"):
        assert any(r["symbol"] == symbol and r["result"] == "pass"
                   for r in write["relocations"])
    # Do not fabricate storage to turn unresolved CRT references into success.
    assert any(r["result"] == "unknown" and "iob" in r["symbol"]
               for r in row(report, 0x647C37)["relocations"])
    assert report["result"] == "unknown"
    assert report["application_recoveries"] == 0
    assert not report["ready_for_collection"]


def test_old_results_change_only_for_independently_bound_stream_locks(inputs):
    manifest, _, _, _ = inputs
    prior = parse_component(subprocess.check_output(
        ["git", "show", BASE + ":" + COMPANION], cwd=ROOT, text=True))
    before, after = verify(inputs, prior), verify(inputs, manifest)
    assert len(before["functions"]) == 94
    assert len(after["functions"]) == 105
    assert [r["address"] for r in after["functions"][:94]] == [
        r["address"] for r in before["functions"]]
    improved = set()
    for old, current in zip(before["functions"], after["functions"]):
        if old == current:
            continue
        assert old.keys() == current.keys()
        for key in old:
            if key not in ("relocations", "diagnostic"):
                assert old[key] == current[key]
        a, b = old["diagnostic"], current["diagnostic"]
        assert {k: v for k, v in a.items() if k != "emitted_references"} == {
            k: v for k, v in b.items() if k != "emitted_references"}
        assert len(a["emitted_references"]) == len(b["emitted_references"])
        for x, y in zip(a["emitted_references"], b["emitted_references"]):
            if x != y:
                expected = {"__lock_file": "0x00647C37", "__unlock_file": "0x00647C89"}
                assert x["binding_address"] is None
                assert y["binding_address"] == expected[x["symbol"]]
                assert {k: v for k, v in x.items() if k != "binding_address"} == {
                    k: v for k, v in y.items() if k != "binding_address"}
        assert len(old["relocations"]) == len(current["relocations"])
        for a, b in zip(old["relocations"], current["relocations"]):
            if a != b:
                assert a["symbol"] == b["symbol"]
                assert a["symbol"] in ("__lock_file", "__unlock_file")
                assert a["result"] == "unknown" and b["result"] == "pass"
                improved.add(int(current["address"], 16))
    assert improved == {0x645598, 0x646178, 0x647330, 0x647920}


@pytest.mark.parametrize("address,end,symbol,unit,section", EXPECTED)
def test_original_opcode_damage_is_detected_with_fresh_pin(inputs, address, end, symbol, unit, section):
    manifest, image, _, _ = inputs
    changed = bytearray(image)
    changed[raw_offset(image, address)] ^= 1
    changed = bytes(changed)
    result = row(verify(inputs, repin(manifest, changed), changed), address)
    assert result["result"] == "fail"


@pytest.mark.parametrize("address,end,symbol,unit,section", EXPECTED)
def test_shortened_extent_refuses(inputs, address, end, symbol, unit, section):
    manifest, _, _, _ = inputs
    changed = replace(manifest, bindings=tuple(
        replace(b, spans=(Span(address, end - 1),)) if b.origin.address == address else b
        for b in manifest.bindings))
    result = row(verify(inputs, changed), address)
    assert result["boundary"] != "pass" and result["result"] != "pass"


def test_changed_known_wrapper_call_refuses(inputs):
    manifest, image, _, _ = inputs
    changed = bytearray(image)
    changed[raw_offset(image, 0x646047)] ^= 1
    changed = bytes(changed)
    result = row(verify(inputs, repin(manifest, changed), changed), 0x64603F)
    assert result["references"] != "pass" and result["result"] != "pass"


def test_wrong_member_refuses(inputs):
    manifest, _, _, _ = inputs
    changed = replace(manifest, bindings=tuple(
        replace(b, unit="build/intel/mt_obj/fflush.obj")
        if b.origin.address == 0x64603F else b for b in manifest.bindings))
    assert row(verify(inputs, changed), 0x64603F)["result"] != "pass"

