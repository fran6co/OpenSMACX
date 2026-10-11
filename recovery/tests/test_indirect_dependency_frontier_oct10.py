"""Original dependency-entry qualification, not linked selection or game proof."""
from dataclasses import replace
import hashlib
from pathlib import Path
import struct
import subprocess

import pytest

from match_recovery.component_source import parse_component, render_component
from match_recovery.import_source import read_imports
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin

ROOT = Path(__file__).resolve().parents[2]
BASE = "8f030dcb9"
NEW_COMPONENTS = {key + "-import-thunks" for key in ("vfw32", "dsound", "comdlg32", "dplayx")}
EXPECTED = {
    "vfw32": [(0x644DC6, "ICSendMessage", "__imp__ICSendMessage@16", 9, "MSVFW32.dll", "ICSendMessage", None, 0x66925C),
              (0x644DC0, "ICClose", "__imp__ICClose@4", 27, "MSVFW32.dll", "ICClose", None, 0x669260),
              (0x644DCC, "ICOpen", "__imp__ICOpen@12", 12, "MSVFW32.dll", "ICOpen", None, 0x669258)],
    "dsound": [(0x6501F0, "DirectSoundCreate", "__imp__DirectSoundCreate@12", 6, "DSOUND.dll", None, 1, 0x669034)],
    "comdlg32": [(0x6501F6, "GetSaveFileNameA", "__imp__GetSaveFileNameA@4", 15, "comdlg32.dll", "GetSaveFileNameA", None, 0x669394),
                 (0x6501FC, "GetOpenFileNameA", "__imp__GetOpenFileNameA@4", 13, "comdlg32.dll", "GetOpenFileNameA", None, 0x669398)],
    "dplayx": [
        (0x650208, "DirectPlayLobbyCreateA", "__imp__DirectPlayLobbyCreateA@20", 7, "DPLAYX.dll", None, 4, 0x669024),
        (0x65020E, "DirectPlayEnumerateA", "__imp__DirectPlayEnumerateA@8", 5, "DPLAYX.dll", None, 2, 0x669028),
        (0x650214, "DirectPlayCreate", "__imp__DirectPlayCreate@12", 3, "DPLAYX.dll", None, 1, 0x66902C),
    ],
}
ARCHIVES = {"vfw32": "VFW32.LIB", "dsound": "DSOUND.LIB", "comdlg32": "COMDLG32.LIB", "dplayx": "DPLAYX.LIB"}


def without_stored_binding(manifest):
    """Undo only this exact addition for older baseline comparisons."""
    binding = manifest.bindings[-1]
    assert (binding.origin.address, binding.symbol, binding.unit, binding.spans) == (
        0x63D600, "_deflate_stored", "deflate.c", (Span(0x63D600, 0x63D6EF),))
    return replace(manifest, bindings=manifest.bindings[:-1])


def restore_import_archive_configuration(data):
    old = b'      "ddraw-directdrawcreate-thunk": "/home/fran6co/opt/vc6/LIB/DDRAW.LIB"'
    addition = b''',
      "vfw32-import-thunks": "/home/fran6co/opt/vc6/LIB/VFW32.LIB",
      "dsound-import-thunks": "/home/fran6co/opt/vc6/LIB/DSOUND.LIB",
      "comdlg32-import-thunks": "/home/fran6co/opt/vc6/LIB/COMDLG32.LIB",
      "dplayx-import-thunks": "/home/fran6co/opt/vc6/LIB/DPLAYX.LIB"'''
    assert data.count(old + addition) == 1
    return data.replace(old + addition, old, 1)


def restore_prior_ddraw_archive_configuration(data):
    # Older zlib controls predate the independently reviewed DirectDraw step.
    # Undo exactly its archive-path addition; do not replace their baselines.
    old = b'      "winmm-timegettime-thunk": "/home/fran6co/opt/vc6/LIB/WINMM.LIB"'
    addition = b',\n      "ddraw-directdrawcreate-thunk": "/home/fran6co/opt/vc6/LIB/DDRAW.LIB"'
    assert data.count(old + addition) == 1
    return data.replace(old + addition, old, 1)


def test_only_declared_external_archive_paths_added_to_configuration():
    prior = subprocess.check_output(["git", "show", BASE + ":recovery/project.json"], cwd=ROOT)
    assert restore_import_archive_configuration((ROOT / "recovery/project.json").read_bytes()) == prior


def test_only_one_zlib_dependency_binding_added_without_changing_prior_obligations():
    relative = "src/vendor/zlib.recovery.h"
    prior_text = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT, text=True)
    text = (ROOT / relative).read_text()
    prior, current = parse_component(prior_text), parse_component(text)
    assert without_stored_binding(current) == prior
    assert parse_component(render_component(current)) == current
    added = [line for line in text.splitlines() if '"origin": "0x0063D600"' in line]
    assert len(added) == 1
    assert "ORIGINAL:" not in added[0] and "BYTE_EXACT" not in added[0]
    # Existing source prose, legacy markers and all other manifest bytes remain.
    restored = text.replace(added[0] + "\n", "", 1)
    old_last = prior_text.splitlines()[-4]
    assert old_last + "," in restored
    restored = restored.replace(old_last + ",", old_last, 1)
    assert restored == prior_text


@pytest.fixture(scope="module")
def original():
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned proprietary executable unavailable")
    data = path.read_bytes()
    return data, PeImage(data, image_pin(ROOT))


def inputs(key, original):
    text = (ROOT / ("src/vendor/" + key + ".recovery.h")).read_text()
    manifest = parse_component(text)
    assert text.endswith(render_component(manifest))
    assert "ORIGINAL:" not in text and "BYTE_EXACT" not in text
    path = Path("/home/fran6co/opt/vc6/LIB") / ARCHIVES[key]
    if not path.is_file():
        pytest.skip("external proprietary VC6 import archive unavailable")
    declarations = read_imports(ROOT)
    for binding, expected in zip(manifest.bindings, EXPECTED[key]):
        address, symbol, imp, occurrence, module, name, ordinal, slot = expected
        assert (binding.origin.address, binding.symbol, binding.import_symbol, binding.occurrence,
                binding.unit, binding.spans) == (address, symbol, imp, occurrence, module, (Span(address, address + 6),))
        declaration, = [d for d in declarations if d.symbol == imp]
        assert (declaration.module, declaration.name, declaration.ordinal) == (module, name, ordinal)
        assert struct.unpack("<I", original[1].read(Span(address + 2, address + 6)))[0] == slot
    assert len(manifest.bindings) == len(EXPECTED[key])
    return manifest, declarations, original[0], path.read_bytes()


@pytest.mark.parametrize("key", EXPECTED)
def test_all_six_source_bound_original_import_thunks_pass_with_explicit_limits(key, original):
    manifest, declarations, image, archive = inputs(key, original)
    report = manifest.verify(image, archive, declarations)
    assert report["result"] == "pass"
    assert report["application_recoveries"] == 0
    assert report["link_verification"] == "unknown"
    for row in report["functions"]:
        assert row["bytes"] == row["boundary"] == row["references"] == "pass"
        assert row["archive_selection"] == "unknown"


@pytest.mark.parametrize("key", EXPECTED)
@pytest.mark.parametrize("damage", ["missing_import", "wrong_export", "wrong_record", "archive", "opcode"])
def test_import_and_physical_dependency_damage_cannot_pass(key, damage, original):
    manifest, declarations, image, archive = inputs(key, original)
    binding = manifest.bindings[0]
    if damage == "missing_import":
        declarations = [d for d in declarations if d.symbol != binding.import_symbol]
    elif damage == "wrong_export":
        declarations = [replace(d, name=None, ordinal=65535) if d.symbol == binding.import_symbol else d for d in declarations]
    elif damage == "wrong_record":
        manifest = replace(manifest, bindings=(replace(binding, occurrence=9999),) + manifest.bindings[1:])
    elif damage == "archive":
        archive += b"\0"
    else:
        pe = original[1]
        region, offset, size = next((r, o, s) for r, o, s in pe.sections if r.start <= binding.origin.address < r.start + s)
        changed = bytearray(image)
        changed[offset + binding.origin.address - region.start] ^= 1
        image = bytes(changed)
        pin = hashlib.sha256(image).hexdigest()
        manifest = replace(manifest, bindings=tuple(replace(b, origin=Origin(pin, "pe:va", b.origin.address)) for b in manifest.bindings))
    assert manifest.verify(image, archive, declarations)["result"] != "pass"


def test_stored_compression_strategy_is_real_indirect_dependency_target(original):
    pe = original[1]
    assert struct.unpack("<I", pe.read(Span(0x698CF8, 0x698CFC)))[0] == 0x63D600
    assert 0x698CF8 in pe.relocations
    assert pe.read(Span(0x63D6EE, 0x63D6F0)) == b"\xc3\x90"
    upstream = (ROOT / "src/vendor/zlib-1.0.2/deflate.c").read_text()
    assert "/* 0 */ {0,    0,  0,    0, deflate_stored}" in upstream
