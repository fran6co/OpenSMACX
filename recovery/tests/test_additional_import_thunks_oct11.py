"""Four original SDK thunk bindings; not DLL/current-link or app coverage."""
from dataclasses import replace
from pathlib import Path
import subprocess

import pytest
from match_recovery.component_source import parse_component, render_component
from match_recovery.import_source import read_imports
from match_recovery.pe_coff import PeImage
from match_recovery.model import Span

ROOT = Path(__file__).resolve().parents[2]
BASE = "6e945592aba8e8497c06cde7f01deeb198f12f33"
EXPECTED = (
    ("vfw32", "VFW32.LIB", 0x644DC0, "ICClose", 27, "__imp__ICClose@4", "MSVFW32.dll", 0x669260, "3cd888a703a821bcd34d63371dd18ac2c84fd39d53f5be1fffafeab3ddc43d62"),
    ("vfw32", "VFW32.LIB", 0x644DCC, "ICOpen", 12, "__imp__ICOpen@12", "MSVFW32.dll", 0x669258, "814637da76e32c3ccd328bff53d512a17fcee7ad8543e227c73d1ab93fb71ca8"),
    ("comdlg32", "COMDLG32.LIB", 0x6501FC, "GetOpenFileNameA", 13, "__imp__GetOpenFileNameA@4", "comdlg32.dll", 0x669398, "df6cd43adcf5e6cc841d753475dd32fc48268665ebc72b406c5a337ee73b7ee7"),
    ("ddraw", "DDRAW.LIB", 0x650202, "DirectDrawEnumerateA", 11, "__imp__DirectDrawEnumerateA@8", "DDRAW.dll", 0x66901C, "39d5638d2f9250479e8f0f38b54552b973449537694dbf4d3b620fd9b8646259"),
)


def without_additional_imports(manifest):
    selected = [r for r in EXPECTED if any(b.origin.address == r[2] for b in manifest.bindings)]
    for _, _, address, symbol, occurrence, imp, module, _, _ in selected:
        binding, = [b for b in manifest.bindings if b.origin.address == address]
        assert (binding.symbol, binding.occurrence, binding.import_symbol, binding.unit, binding.spans) == (
            symbol, occurrence, imp, module, (Span(address, address+6),))
    addresses = {r[2] for r in selected}
    return replace(manifest, bindings=tuple(b for b in manifest.bindings if b.origin.address not in addresses))


@pytest.mark.parametrize("key", ("vfw32", "comdlg32", "ddraw"))
def test_exact_additions_preserve_prior_component_bindings(key):
    name = "src/vendor/" + key + ".recovery.h"
    before = subprocess.check_output(["git", "show", BASE + ":" + name], cwd=ROOT, text=True)
    after = (ROOT/name).read_text()
    manifest = parse_component(after)
    assert manifest == replace(parse_component(before), bindings=manifest.bindings)
    assert without_additional_imports(manifest) == parse_component(before)
    prefix = after[:after.index("// Original-image bindings")]
    assert after == prefix + render_component(manifest)
    assert "ORIGINAL:" not in after and "BYTE_EXACT" not in after


@pytest.mark.parametrize("path,lines", (
    ("src/video.cpp", "// RECOVERY_IMPORT: v1 __imp__ICClose@4 MSVFW32.dll name:ICClose\n// RECOVERY_IMPORT: v1 __imp__ICOpen@12 MSVFW32.dll name:ICOpen\n"),
    ("src/filebox.cpp", "// RECOVERY_IMPORT: v1 __imp__GetOpenFileNameA@4 comdlg32.dll name:GetOpenFileNameA\n"),
))
def test_actual_source_only_gains_import_identity_comments(path, lines):
    before = subprocess.check_output(["git", "show", BASE + ":" + path], cwd=ROOT, text=True)
    after = (ROOT/path).read_text()
    assert after.count(lines) == 1
    assert after.replace(lines, "", 1) == before


@pytest.mark.parametrize("row", EXPECTED, ids=[r[3] for r in EXPECTED])
@pytest.mark.parametrize("damage", ("none", "missing_import", "wrong_record", "opcode"))
def test_physical_thunk_and_independent_sdk_record(row, damage):
    key, archive_name, address, symbol, occurrence, imp, module, slot, digest = row
    path = ROOT/".opensmacx/game/terranx_original.exe"
    archive = Path("/home/fran6co/opt/vc6/LIB")/archive_name
    if not path.is_file() or not archive.is_file():
        pytest.skip("pinned proprietary inputs unavailable")
    manifest = parse_component((ROOT/("src/vendor/"+key+".recovery.h")).read_text())
    image, library = path.read_bytes(), archive.read_bytes()
    declarations = read_imports(ROOT)
    binding, = [b for b in manifest.bindings if b.origin.address == address]
    if damage == "missing_import":
        declarations = [d for d in declarations if d.symbol != imp]
    elif damage == "wrong_record":
        manifest = replace(manifest, bindings=tuple(replace(b, occurrence=occurrence+1) if b==binding else b for b in manifest.bindings))
    elif damage == "opcode":
        pe = PeImage(image, binding.origin.image_sha256)
        section, offset, size = next((s,o,n) for s,o,n in pe.sections if s.start<=address<s.start+n)
        changed = bytearray(image); changed[offset+address-section.start] ^= 1
        image = bytes(changed)
        import hashlib
        manifest = replace(manifest, bindings=tuple(replace(b, origin=replace(b.origin, image_sha256=hashlib.sha256(image).hexdigest())) for b in manifest.bindings))
    report = manifest.verify(image, library, declarations)
    if damage != "none":
        assert report["result"] != "pass"
        return
    assert report["result"] == "pass" and report["application_recoveries"] == 0
    result, = [r for r in report["functions"] if r["address"] == f"0x{address:08X}"]
    assert result["bytes"] == result["boundary"] == result["references"] == "pass"
    assert result["physical_record"] == {"member": module, "occurrence": occurrence, "sha256": digest}
    assert result["import"] == {"module": module, "name": symbol, "ordinal": None, "address": f"0x{slot:08X}"}
    assert result["archive_selection"] == report["link_verification"] == report["dll_implementation"] == report["runtime_verification"] == "unknown"
