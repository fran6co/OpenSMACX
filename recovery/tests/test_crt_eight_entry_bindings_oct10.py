"""Bounded runtime correspondence; not selected-link or application coverage."""
from dataclasses import replace
import hashlib
import os
from pathlib import Path
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, render_component
from match_recovery.import_source import read_imports
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage

ROOT = Path(__file__).resolve().parents[2]
BASE = "b1dc558c176b97f7d957a50f43616bf4c29d16c2"
ADDITION = "2a85756a7"
COMPANION = "src/vendor/vc6-runtime.recovery.h"
EXPECTED = (
    (0x6458A0, 0x6458C7, "_strrchr", "build/intel/mt_obj/strrchr.obj"),
    (0x6458C8, 0x6458EF, "__ftol", "../build/intel/mt_obj/ftol.obj"),
    (0x6458F0, 0x645928, "_strncmp", "build/intel/mt_obj/strncmp.obj"),
    (0x645C70, 0x645D1C, "_memcmp", "build/intel/mt_obj/memcmp.obj"),
    (0x645E90, 0x645F8E, "_strncpy", "build/intel/mt_obj/strncpy.obj"),
    (0x6473F0, 0x647495, "_memchr", "build/intel/mt_obj/memchr.obj"),
    (0x6494A6, 0x6494D1, "__freebuf", "build/intel/mt_obj/_freebuf.obj"),
    (0x64C8B3, 0x64C8C5, "__frnd", "../build/intel/mt_obj/frnd.obj"),
)


def without_eight_bindings(manifest):
    """Remove exactly this reviewed addition for historical preservation controls."""
    addresses = {address for address, _, _, _ in EXPECTED}
    tail = tuple(binding for binding in manifest.bindings if binding.origin.address in addresses)
    assert tuple((b.origin.address, b.spans[0].end, b.symbol, b.unit)
                 for b in tail) == EXPECTED
    assert all(b.spans == (Span(a, e),)
               for b, (a, e, _, _) in zip(tail, EXPECTED))
    return replace(manifest, bindings=tuple(
        binding for binding in manifest.bindings if binding.origin.address not in addresses))


def test_only_eight_runtime_bindings_added_without_changing_prior_source():
    prior_text = subprocess.check_output(
        ["git", "show", BASE + ":" + COMPANION], cwd=ROOT, text=True)
    text = subprocess.check_output(
        ["git", "show", ADDITION + ":" + COMPANION], cwd=ROOT, text=True)
    current, prior = parse_component(text), parse_component(prior_text)
    assert without_eight_bindings(current) == prior
    installed = parse_component((ROOT / COMPANION).read_text())
    assert replace(installed, bindings=installed.bindings[:len(current.bindings)]) == current
    assert parse_component(render_component(current)) == current
    added = [line for line in text.splitlines()
             if any('"origin": "0x%08X"' % a in line for a, _, _, _ in EXPECTED)]
    assert len(added) == len(EXPECTED)
    restored = text
    for line in added:
        assert "ORIGINAL:" not in line and "BYTE_EXACT" not in line
        restored = restored.replace(line + "\n", "", 1)
    old_last = next(line for line in reversed(prior_text.splitlines())
                    if '"origin":' in line and '"unit":' in line)
    assert restored.count(old_last + ",\n") == 1
    restored = restored.replace(old_last + ",\n", old_last + "\n", 1)
    assert restored == prior_text


@pytest.fixture(scope="module")
def inputs():
    image_path = Path(os.environ.get("OPENSMACX_IMAGE",
                                     ROOT / ".opensmacx/game/terranx_original.exe"))
    if "OPENSMACX_IMAGE" in os.environ:
        assert image_path.is_file(), "explicit pinned image path is unavailable"
    archive_path = Path("/home/fran6co/opt/vc6/LIB/LIBCMT.LIB")
    if not image_path.is_file() or not archive_path.is_file():
        pytest.skip("external pinned image/runtime archive unavailable")
    # Preserve this addition's original before/after proof. Later independently
    # bound callers can legitimately depend on these eight runtime entries.
    # Current-manifest preservation is exercised by each subsequent batch.
    manifest = parse_component(subprocess.check_output(
        ["git", "show", ADDITION + ":" + COMPANION], cwd=ROOT, text=True))
    image, archive = image_path.read_bytes(), archive_path.read_bytes()
    assert hashlib.sha256(image).hexdigest() == manifest.bindings[0].origin.image_sha256
    assert hashlib.sha256(archive).hexdigest() == manifest.archive_sha256
    return manifest, image, archive, read_imports(ROOT)


def verify(inputs, manifest=None, image=None, archive=None):
    m, i, a, imports = inputs
    return verify_toolchain(m if manifest is None else manifest,
                            i if image is None else image,
                            a if archive is None else archive,
                            import_bindings=imports)


def row(report, address):
    return next(r for r in report["functions"] if int(r["address"], 16) == address)


def test_eight_pass_and_all_prior_runtime_results_remain_unchanged(inputs):
    manifest, _, _, _ = inputs
    before = verify(inputs, without_eight_bindings(manifest))
    after = verify(inputs)
    addresses = {address for address, _, _, _ in EXPECTED}
    assert [row for row in after["functions"] if int(row["address"], 16) not in addresses] == before["functions"]
    for address, _, _, _ in EXPECTED:
        result = row(after, address)
        assert all(result[k] == "pass" for k in ("result", "boundary", "bytes", "references"))
    assert after["result"] == "unknown"
    assert after["application_recoveries"] == 0
    assert not after["ready_for_collection"]


def repin(manifest, image):
    digest = hashlib.sha256(image).hexdigest()
    def origin(o):
        return replace(o, image_sha256=digest)
    return replace(manifest,
                   bindings=tuple(replace(b, origin=origin(b.origin)) for b in manifest.bindings),
                   data_sections=tuple(replace(b, origin=origin(b.origin)) for b in manifest.data_sections))


def raw_offset(image, address):
    pe = PeImage(image, hashlib.sha256(image).hexdigest())
    _, offset, _ = next((s, o, n) for s, o, n in pe.sections
                        if s.start <= address < s.start + n)
    section = next(s for s, _, n in pe.sections if s.start <= address < s.start + n)
    return offset + address - section.start


@pytest.mark.parametrize("address,end,symbol,unit", EXPECTED)
def test_original_byte_damage_refuses_even_with_fresh_pin(inputs, address, end, symbol, unit):
    manifest, image, _, _ = inputs
    changed = bytearray(image)
    changed[raw_offset(image, address)] ^= 1
    changed = bytes(changed)
    result = row(verify(inputs, repin(manifest, changed), changed), address)
    assert result["result"] != "pass" and result["bytes"] != "pass"


@pytest.mark.parametrize("address,end,symbol,unit", EXPECTED)
def test_truncated_extent_cannot_pass(inputs, address, end, symbol, unit):
    manifest, _, _, _ = inputs
    changed = replace(manifest, bindings=tuple(
        replace(b, spans=(Span(address, end - 1),)) if b.origin.address == address else b
        for b in manifest.bindings))
    result = row(verify(inputs, changed), address)
    assert result["result"] != "pass" and result["boundary"] != "pass"


def test_freebuf_wrong_call_target_refuses_under_fresh_pin(inputs):
    manifest, image, _, _ = inputs
    changed = bytearray(image)
    changed[raw_offset(image, 0x6494A6 + 20)] ^= 1
    changed = bytes(changed)
    result = row(verify(inputs, repin(manifest, changed), changed), 0x6494A6)
    assert result["references"] != "pass" and result["result"] != "pass"
