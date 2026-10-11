"""Reference-disambiguated CRT bindings, not final-link or runtime proof."""
from dataclasses import replace
import csv
import hashlib
import json
import os
from pathlib import Path
import re
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.import_source import read_imports
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from test_crt_eight_entry_bindings_oct10 import repin, raw_offset

ROOT = Path(__file__).resolve().parents[2]
BASE = "209b1258c"
ADDITION = "f5dc7f52a"
COMPANION = "src/vendor/vc6-runtime.recovery.h"
EXPECTED = (
    (0x00646829, 0x00646855, "__mkdir", "build/intel/mt_obj/mkdir.obj", 1, "e1607841a197a3a188f4bc29cd8e1e90393965e41dc19ce0563f4cd6c5c4d009"),
    (0x00646950, 0x0064697A, "_remove", "build/intel/mt_obj/unlink.obj", 1, "08dd58c6b1e13ec871519a6f534034c84894d4aee46d513a403f5488c514d675"),
    (0x006469F7, 0x00646A25, "_rename", "build/intel/mt_obj/rename.obj", 1, "199f7e4ae63d310cff34d6514aa8ae9d9c3a254c968c41df255130d18218926f"),
    (0x0064A178, 0x0064A1DD, "__read", "build/intel/mt_obj/read.obj", 1, "e8ff059f905a1d2f1fb1598354e5ffa7dbdf349bcaf29cfc2c00c8f930f28be3"),
    (0x0064BF67, 0x0064BFCC, "__lseek", "build/intel/mt_obj/lseek.obj", 1, "d0e995a8077188a06229360a13286e2618ae971c0a0e657447c415469271e153"),
    (0x0064CB98, 0x0064CBB4, "?_ValidateRead@@YAHPBXI@Z", "build/intel/mt_obj/validate.obj", 1, "a12a6559529c363ab497ad3f717b9d4fa79252541a181e2d46dc716197b5a0a7"),
    (0x0064CBB4, 0x0064CBD0, "?_ValidateWrite@@YAHPAXI@Z", "build/intel/mt_obj/validate.obj", 3, "a12a6559529c363ab497ad3f717b9d4fa79252541a181e2d46dc716197b5a0a7"),
    (0x00650220, 0x00650231, "__tell", "build/intel/mt_obj/tell.obj", 1, "d90a50fea2ba348270772d44f9f362561d8f22a56f7a67c93cee5ebeba5a481d"),
)
RIVALS = (
    (0x00646829, "build/intel/mt_obj/wmkdir.obj", "__wmkdir"),
    (0x00646950, "build/intel/mt_obj/rmdir.obj", "__rmdir"),
    (0x00646950, "build/intel/mt_obj/wrmdir.obj", "__wrmdir"),
    (0x00646950, "build/intel/mt_obj/wunlink.obj", "__wremove"),
    (0x006469F7, "build/intel/mt_obj/wrename.obj", "__wrename"),
    (0x0064A178, "build/intel/mt_obj/lseek.obj", "__lseek"),
    (0x0064A178, "build/intel/mt_obj/write.obj", "__write"),
    (0x0064BF67, "build/intel/mt_obj/read.obj", "__read"),
    (0x0064BF67, "build/intel/mt_obj/write.obj", "__write"),
    (0x0064CB98, "build/intel/mt_obj/validate.obj", "?_ValidateWrite@@YAHPAXI@Z"),
    (0x0064CBB4, "build/intel/mt_obj/validate.obj", "?_ValidateRead@@YAHPBXI@Z"),
    (0x00650220, "build/intel/mt_obj/ismbbyte.obj", "__ismbbkalnum"),
)
ADDRESSES = {x[0] for x in EXPECTED}
NEW_IMPORTS = (
    ("CreateDirectoryA", 8), ("DeleteFileA", 4), ("MoveFileA", 8),
    ("IsBadReadPtr", 8), ("IsBadWritePtr", 8),
)


def prior():
    return subprocess.check_output(["git", "show", BASE + ":" + COMPANION],
                                   cwd=ROOT, text=True)


def test_preserves_source_and_previous_bindings():
    old = prior()
    text = subprocess.check_output(
        ["git", "show", ADDITION + ":" + COMPANION], cwd=ROOT, text=True)
    before, after = parse_component(old), parse_component(text)
    installed = parse_component((ROOT / COMPANION).read_text())
    assert replace(installed, bindings=installed.bindings[:232]) == after
    assert len(before.bindings) == 224 and len(after.bindings) == 232
    assert replace(after, bindings=after.bindings[:224]) == before
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit)
            for b in after.bindings[224:]] == [x[:4] for x in EXPECTED]
    additions = [line for line in text.splitlines() if line.startswith("//     {")
                 and '"unit"' in line
                 and int(json.loads(line[3:].rstrip(","))["origin"], 16) in ADDRESSES]
    assert len(additions) == 8
    for line in additions:
        text = text.replace(line + "\n", "", 1)
    for name, argc in NEW_IMPORTS:
        line = f"// RECOVERY_IMPORT: v1 __imp__{name}@{argc} KERNEL32.dll name:{name}"
        assert text.count(line + "\n") == 1
        text = text.replace(line + "\n", "", 1)
    last = next(line for line in reversed(old.splitlines()) if '"unit"' in line and '"origin"' in line)
    assert text.replace(last + ",\n", last + "\n", 1) == old


@pytest.fixture(scope="module")
def inputs():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    archive_path = Path("/home/fran6co/opt/vc6/LIB/LIBCMT.LIB")
    if "OPENSMACX_IMAGE" in os.environ:
        assert path.is_file(), "explicit pinned image path unavailable"
    if not path.is_file() or not archive_path.is_file():
        pytest.skip("external pinned image/runtime archive unavailable")
    manifest = parse_component(subprocess.check_output(
        ["git", "show", ADDITION + ":" + COMPANION], cwd=ROOT, text=True))
    image, archive = path.read_bytes(), archive_path.read_bytes()
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


def test_physical_members_and_catalogue_extents(inputs):
    _, _, archive, _ = inputs
    members = CoffArchive(archive)
    with (ROOT / "docs/recovery/ghidra-functions.csv").open() as stream:
        catalogue = {int(r["address"], 16): r for r in csv.DictReader(stream)}
    for address, end, symbol, unit, section, digest in EXPECTED:
        assert int(catalogue[address]["end_address"], 16) == end
        data = members.member(unit)
        assert hashlib.sha256(data).hexdigest() == digest
        obj = Coff(data)
        function = obj.function(symbol, comdat=False)
        assert function.section == section and function.value == 0
        assert len(obj.subject(function)[0]) == end - address


def test_selected_results_and_monotonic_old_references(inputs):
    before = verify(inputs, parse_component(prior()))
    after = verify(inputs)
    for address, *_ in EXPECTED:
        r = row(after, address)
        assert r["boundary"] == "pass" and not r["differing_offsets"]
        assert r["result"] == ("unknown" if address in (0x64A178, 0x64BF67) else "pass")
        assert all(x["result"] in ("pass", "unknown") for x in r["relocations"])
    for old in before["functions"]:
        current = row(after, int(old["address"], 16))
        if old == current:
            continue
        assert current["result"] != "fail"
        assert current["boundary"] == old["boundary"]
        assert current["differing_offsets"] == old["differing_offsets"]
        if old["result"] == "pass":
            assert current["result"] == "pass"
        assert len(old["relocations"]) == len(current["relocations"])
        for a, b in zip(old["relocations"], current["relocations"]):
            assert a["symbol"] == b["symbol"]
            if a != b:
                assert a["result"] == "unknown" and b["result"] == "pass"
                assert a["symbol"] in {x[2] for x in EXPECTED}
    assert after["result"] == "unknown"
    assert after["application_recoveries"] == 0 and not after["ready_for_collection"]


@pytest.mark.parametrize("address,unit,symbol", RIVALS)
def test_each_competing_member_has_independent_reference_contradiction(inputs, address, unit, symbol):
    manifest, image, archive, _ = inputs
    pe = PeImage(image, hashlib.sha256(image).hexdigest())
    slots = {entry.address: entry for entry in pe.imports()}
    bindings = {b.symbol: b.origin.address for b in manifest.bindings}
    obj = Coff(CoffArchive(archive).member(unit))
    fn = obj.function(symbol, comdat=False)
    sec = obj.sections[fn.section - 1]
    contradictions = []
    for offset, symbol_index, kind in sec.relocations:
        target = obj.symbols[symbol_index].name
        original = int.from_bytes(pe.read(Span(address + offset, address + offset + 4)),
                                  "little", signed=kind == 20)
        addend = int.from_bytes(sec.data[offset:offset + 4], "little", signed=kind == 20)
        if kind == 6 and target.startswith("__imp__"):
            match = re.fullmatch(r"__imp__(.+)@[0-9]+", target)
            if match and original - addend in slots:
                actual = slots[original - addend]
                contradictions.append(actual.module.lower() != "kernel32.dll"
                                      or actual.name != match.group(1))
        elif kind == 20 and target in bindings:
            contradictions.append(address + offset + 4 + original != bindings[target] + addend)
    assert any(contradictions), (hex(address), unit, symbol)


@pytest.mark.parametrize("address,end,symbol,unit,section,digest", EXPECTED)
def test_opcode_damage_refused(inputs, address, end, symbol, unit, section, digest):
    manifest, image, _, _ = inputs
    changed = bytearray(image)
    changed[raw_offset(image, address)] ^= 1
    changed = bytes(changed)
    assert row(verify(inputs, repin(manifest, changed), changed), address)["result"] == "fail"


@pytest.mark.parametrize("address,end,symbol,unit,section,digest", EXPECTED)
def test_distinguishing_reference_damage_refused(inputs, address, end, symbol, unit, section, digest):
    manifest, image, archive, _ = inputs
    obj = Coff(CoffArchive(archive).member(unit))
    fn = obj.function(symbol, comdat=False)
    sec = obj.sections[fn.section - 1]
    targets = {0x64A178: "__read_lk", 0x64BF67: "__lseek_lk", 0x650220: "__lseek"}
    sites = [offset for offset, index, kind in sec.relocations
             if (obj.symbols[index].name == targets[address] if address in targets
                 else obj.symbols[index].name.startswith("__imp__")
                 and obj.symbols[index].name != "__imp__GetLastError@0")]
    assert len(sites) == 1
    changed = bytearray(image)
    changed[raw_offset(image, address + sites[0])] ^= 1
    changed = bytes(changed)
    r = row(verify(inputs, repin(manifest, changed), changed), address)
    assert r["result"] == "fail" and r["references"] == "fail"


@pytest.mark.parametrize("address,end,symbol,unit,section,digest", EXPECTED)
def test_shortened_extent_refused(inputs, address, end, symbol, unit, section, digest):
    manifest, _, _, _ = inputs
    changed = replace(manifest, bindings=tuple(
        replace(b, spans=(Span(address, end - 1),)) if b.origin.address == address else b
        for b in manifest.bindings))
    r = row(verify(inputs, changed), address)
    assert r["boundary"] != "pass" and r["result"] != "pass"

