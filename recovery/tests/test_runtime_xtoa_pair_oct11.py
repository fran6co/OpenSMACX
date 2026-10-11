"""Pinned CRT conversion-family correspondence, not application recovery."""
from dataclasses import replace
import csv
import hashlib
from pathlib import Path
import subprocess

import pytest

from match_recovery.component_source import parse_component, render_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.model import Span
from test_crt_eight_entry_bindings_oct10 import inputs, verify, row, repin, raw_offset

ROOT = Path(__file__).resolve().parents[2]
SOURCE = "src/vendor/vc6-runtime.recovery.h"
BASE = "7668c4874c1f9e555ec049c1fc31fed89f0f8545"
UNIT = "build/intel/mt_obj/xtoa.obj"
EXPECTED = ((0x64FC88, 0x64FCB5, "__itoa", 1, 2),
            (0x64FCB5, 0x64FD11, "_xtoa", 2, 3))


def without_xtoa_pair(manifest):
    addresses = {a for a,*_ in EXPECTED}
    added = tuple(b for b in manifest.bindings if b.origin.address in addresses)
    assert tuple((b.origin.address, b.spans[0].end, b.symbol) for b in added) == tuple(
        item[:3] for item in EXPECTED)
    assert all(b.unit == UNIT and b.spans == (Span(a, e),)
               for b, (a, e, *_) in zip(added, EXPECTED))
    return replace(manifest, bindings=tuple(b for b in manifest.bindings if b.origin.address not in addresses))


def test_exact_append_only_source_metadata_and_writer_roundtrip():
    before = subprocess.check_output(["git", "show", BASE + ":" + SOURCE], cwd=ROOT, text=True)
    text = (ROOT / SOURCE).read_text()
    from test_runtime_atox_errno_oct11 import without_atox_errno
    installed = parse_component(text)
    historical = without_atox_errno(installed)
    added = installed.bindings[-7:]
    for b in added:
        line = next(l for l in text.splitlines() if '"origin": "0x%08X"' % b.origin.address in l)
        text = text.replace(line+'\n','',1)
    last = next(l for l in reversed(text.splitlines()) if '"origin": "0x' in l)
    assert last.endswith(',')
    text = text.replace(last+'\n',last[:-1]+'\n',1)
    manifest = parse_component(text)
    assert manifest == historical
    assert without_xtoa_pair(manifest) == parse_component(before)
    assert parse_component(render_component(manifest)) == manifest
    patch = Path(__file__).with_name("runtime-xtoa-pair-oct11.patch").read_text()
    for hunk in patch.split("\n@@ ")[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        old = "".join(line[1:] for line in lines if line.startswith((" ", "-")))
        new = "".join(line[1:] for line in lines if line.startswith((" ", "+")))
        assert old and before.count(old) == 1
        before = before.replace(old, new, 1)
    assert before == text


def test_native_pair_closes_references_and_preserves_all_prior_results(inputs):
    manifest, _, archive, _ = inputs
    prior = verify(inputs, without_xtoa_pair(manifest))
    current = verify(inputs)
    assert [r for r in current["functions"] if int(r['address'],16) not in {a for a,*_ in EXPECTED}] == prior["functions"]
    assert current["data_sections"] == prior["data_sections"]
    assert current["absolute_symbols"] == prior["absolute_symbols"]
    member = CoffArchive(archive).member(UNIT)
    assert hashlib.sha256(member).hexdigest() == "515195b822124c3c7a05b35c07342f926a8e43a5dcf5f3e89e5babf8d86738ca"
    obj = Coff(member)
    for a, e, name, section, storage in EXPECTED:
        result = row(current, a)
        assert all(result[k] == "pass" for k in ("result", "boundary", "bytes", "references")), result
        symbol, = [s for s in obj.symbols.values() if s.name == name]
        assert (symbol.section, symbol.value, symbol.storage) == (section, 0, storage)
        assert len(obj.sections[section - 1].data) == e - a
        assert result["alignment_fill"] == 0
    assert row(current, 0x64FC88)["relocations"] == [
        {"offset": 33, "target": "8", "result": "pass", "symbol": "_xtoa"}]
    assert row(current, 0x64FCB5)["relocations"] == []
    assert current["result"] == prior["result"] == "unknown"
    assert current["application_recoveries"] == 0 and not current["ready_for_collection"]


def test_independent_original_boundaries_and_connecting_call(inputs):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    manifest, data, _, _ = inputs
    path = ROOT / ".opensmacx/game/ida94-functions.csv"
    if not path.is_file():
        pytest.skip("independent IDA catalogue unavailable")
    with path.open() as handle:
        rows = list(csv.reader(handle))
    with (ROOT / "docs/recovery/ghidra-functions.csv").open() as handle:
        ghidra = list(csv.DictReader(handle))
    image = PeImage(data, manifest.bindings[0].origin.image_sha256)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for a, e, name, *_ in EXPECTED:
        assert any(r[:3] == [f"0x{a:08X}", f"0x{e:08X}", name] for r in rows)
        assert any(r["program_sha256"] == manifest.bindings[0].origin.image_sha256
                   and r["ghidra_version"] == "12.1.2"
                   and r["address"] == f"0x{a:08X}"
                   and r["end_address"] == f"0x{e:08X}"
                   and r["body_ranges"] == f"0x{a:08X}-0x{e:08X}"
                   and int(r["size"]) == e - a for r in ghidra)
        instructions = list(decoder.disasm(image.read(Span(a, e)), a))
        assert sum(i.size for i in instructions) == e - a
        assert instructions[-1].mnemonic == "ret" and instructions[-1].address + instructions[-1].size == e
    instructions = list(decoder.disasm(image.read(Span(0x64FC88, 0x64FCB5)), 0x64FC88))
    assert [(i.address, i.op_str) for i in instructions if i.mnemonic == "call"] == [(0x64FCA8, "0x64fcb5")]


@pytest.mark.parametrize("damage", ("none", "missing_callee", "extent", "symbol", "member", "archive_pin", "body"))
def test_incomplete_or_wrong_pair_never_passes(inputs, damage):
    manifest, image, archive, _ = inputs
    selected = replace(manifest, bindings=tuple(b for b in manifest.bindings if b.origin.address in {a for a,*_ in EXPECTED}), absolute_symbols=(), data_sections=())
    first, second = selected.bindings
    if damage == "missing_callee":
        selected = replace(selected, bindings=(first,))
    elif damage == "extent":
        selected = replace(selected, bindings=(replace(first, spans=(Span(first.origin.address, first.spans[0].end - 1),)), second))
    elif damage == "symbol":
        selected = replace(selected, bindings=(replace(first, symbol="_missing_xtoa"), second))
    elif damage == "member":
        selected = replace(selected, bindings=(replace(first, unit="build/intel/mt_obj/strrchr.obj"), second))
    elif damage == "archive_pin":
        selected = replace(selected, archive_sha256="0" * 64)
    elif damage == "body":
        changed = bytearray(image)
        changed[raw_offset(image, first.origin.address)] ^= 1
        image = bytes(changed)
        selected = repin(selected, image)
    report = verify(inputs, selected, image, archive)
    assert (report["result"] == "pass") == (damage == "none"), report


def test_native_task_exact_subject_scope():
    import json
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/runtime-xtoa-pair-oct11.json").read_text()))
    assert task.base_revision == BASE
    assert {s.address for s in task.subjects} == {a for a, *_ in EXPECTED}
    assert all(s.address_space == "pe:va" and s.image_sha256 ==
               "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf" for s in task.subjects)
