"""Bounded CRT ownership, not application recovery or original link selection."""
from dataclasses import replace
import hashlib
from pathlib import Path
import subprocess

import pytest

from match_recovery.coff_archive import CoffArchive
from match_recovery.component_source import parse_component, render_component
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_text

from test_crt_eight_entry_bindings_oct10 import inputs as current_inputs, verify, row, repin, raw_offset

ROOT = Path(__file__).resolve().parents[2]
SOURCE = "src/vendor/vc6-runtime.recovery.h"
BASE = "f144385fa"
ADDITION = "602b8d5b4"
UNIT = "build/intel/mt_obj/crt0dat.obj"
MEMBER_SHA = "48be3defdca3560f39e7d593e4dd110cb2977874227eae911a707868f722b961"
EXPECTED = (
    (0x644EC6, 0x644ECF, "__lockexit", 14, 2, "__lock"),
    (0x644ECF, 0x644ED8, "__unlockexit", 16, 2, "__unlock"),
    (0x644ED8, 0x644EF2, "__initterm", 18, 3, None),
)


@pytest.fixture(scope="module")
def inputs(current_inputs):
    # Verify this historical addition against fresh pinned binaries, not a
    # replayed report. Later helper closure is tested in its own current scope.
    inherited, image, archive, imports = current_inputs
    # The eight-entry fixture deliberately supplies its own older historical
    # manifest. Installed-prefix continuity must read the actual companion.
    installed = parse_component((ROOT / SOURCE).read_text())
    assert installed.archive_sha256 == inherited.archive_sha256
    assert installed.bindings[0].origin.image_sha256 == inherited.bindings[0].origin.image_sha256
    historical = parse_component(subprocess.check_output(
        ["git", "show", ADDITION + ":" + SOURCE], cwd=ROOT, text=True))
    assert replace(installed, bindings=installed.bindings[:len(historical.bindings)]) == historical
    return historical, image, archive, imports


def without_new(manifest):
    addresses = {a for a, *_ in EXPECTED}
    added = tuple(b for b in manifest.bindings if b.origin.address in addresses)
    assert tuple((b.origin.address, b.spans[0].end, b.symbol) for b in added) == tuple(
        item[:3] for item in EXPECTED)
    assert all(b.unit == UNIT and b.spans == (Span(a, e),)
               for b, (a, e, *_) in zip(added, EXPECTED))
    return replace(manifest, bindings=tuple(
        b for b in manifest.bindings if b.origin.address not in addresses))


def test_only_three_bindings_added_without_changing_prior_source_or_claims():
    old = subprocess.check_output(["git", "show", BASE + ":" + SOURCE], cwd=ROOT, text=True)
    text = subprocess.check_output(["git", "show", ADDITION + ":" + SOURCE], cwd=ROOT, text=True)
    manifest = parse_component(text)
    assert without_new(manifest) == parse_component(old)
    assert parse_component(render_component(manifest)) == manifest
    assert [replace(r, line=1, region="") for r in read_text(text, ROOT / SOURCE)] == [
        replace(r, line=1, region="") for r in read_text(old, ROOT / SOURCE)]
    added = [line for line in text.splitlines()
             if any('"origin": "0x%08X"' % a in line for a, *_ in EXPECTED)]
    assert len(added) == 3
    for line in added:
        assert "ORIGINAL:" not in line and "BYTE_EXACT" not in line
        text = text.replace(line + "\n", "", 1)
    old_last = next(line for line in reversed(old.splitlines())
                    if '"origin":' in line and '"unit":' in line)
    assert text.count(old_last + ",\n") == 1
    assert text.replace(old_last + ",\n", old_last + "\n", 1) == old
    assert not any(b.symbol in ("_exit", "__doexit", "__cinit") for b in manifest.bindings)


def test_full_physical_members_and_existing_reference_closure(inputs):
    manifest, _, archive, _ = inputs
    before, after = verify(inputs, without_new(manifest)), verify(inputs)
    assert after["functions"][:-3] == before["functions"]
    body = CoffArchive(archive).member(UNIT)
    assert hashlib.sha256(body).hexdigest() == MEMBER_SHA
    obj = Coff(body)
    for address, end, symbol, section, storage, callee in EXPECTED:
        result = row(after, address)
        assert all(result[k] == "pass" for k in ("result", "boundary", "bytes", "references")), result
        assert result["section"] == section and result["alignment_fill"] == 0
        native_symbol, = [s for s in obj.symbols.values() if s.name == symbol]
        assert (native_symbol.section, native_symbol.value, native_symbol.storage) == (section, 0, storage)
        physical = obj.sections[section - 1]
        assert len(physical.data) == end - address and physical.flags == 0x60101020
        assert {r["symbol"] for r in result["relocations"]} == ({callee} if callee else set())
        assert all(r["result"] == "pass" for r in result["relocations"])
    assert after["result"] == "unknown"
    assert after["application_recoveries"] == 0 and not after["ready_for_collection"]


def test_independent_original_calls_returns_and_init_loop(inputs):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    manifest, data, _, _ = inputs
    image = PeImage(data, manifest.bindings[0].origin.image_sha256)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoded = {}
    for address, end, *_ in EXPECTED:
        instructions = list(decoder.disasm(image.read(Span(address, end)), address))
        assert sum(i.size for i in instructions) == end - address
        assert instructions[-1].mnemonic == "ret" and instructions[-1].address + instructions[-1].size == end
        decoded.update({i.address: (i.mnemonic, i.op_str) for i in instructions})
    assert decoded[0x644EC6] == decoded[0x644ECF] == ("push", "0xd")
    assert decoded[0x644EC8] == ("call", "0x647d04")
    assert decoded[0x644ED1] == ("call", "0x647d65")
    assert decoded[0x644EE1] == ("jae", "0x644ef0")
    assert decoded[0x644EE7] == ("je", "0x644eeb")
    assert decoded[0x644EE9] == ("call", "eax")
    assert decoded[0x644EEB] == ("add", "esi, 4")
    assert decoded[0x644EEE] == ("jmp", "0x644edd")
    for site, target in {0x644E22: 0x644EC6, 0x64531B: 0x644EC6,
                         0x644EAD: 0x644ECF, 0x64538F: 0x644ECF,
                         0x644DE7: 0x644ED8, 0x644DF6: 0x644ED8,
                         0x644E90: 0x644ED8, 0x644EA1: 0x644ED8}.items():
        instruction, = decoder.disasm(image.read(Span(site, site + 5)), site)
        assert (instruction.mnemonic, instruction.op_str) == ("call", hex(target))


@pytest.mark.parametrize("address,end,symbol,section,storage,callee", EXPECTED)
@pytest.mark.parametrize("damage", ("extent", "member", "body"))
def test_damaged_attributions_refuse(inputs, address, end, symbol, section, storage, callee, damage):
    manifest, image, _, _ = inputs
    if damage == "body":
        changed = bytearray(image)
        changed[raw_offset(image, address)] ^= 1
        image = bytes(changed)
        manifest = repin(manifest, image)
    else:
        manifest = replace(manifest, bindings=tuple(
            (replace(b, spans=(Span(address, end - 1),)) if damage == "extent"
             else replace(b, unit="build/intel/mt_obj/mlock.obj"))
            if b.origin.address == address else b for b in manifest.bindings))
    assert row(verify(inputs, manifest, image), address)["result"] != "pass"


@pytest.mark.parametrize("site,address", ((0x644EC8, 0x644EC6), (0x644ED1, 0x644ECF)))
def test_wrong_lock_target_refuses_with_fresh_image_pin(inputs, site, address):
    manifest, image, _, _ = inputs
    changed = bytearray(image)
    changed[raw_offset(image, site + 1)] ^= 1
    image = bytes(changed)
    result = row(verify(inputs, repin(manifest, image), image), address)
    assert result["references"] != "pass" and result["result"] != "pass"
