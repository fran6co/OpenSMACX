"""Fifteen bounded archive correspondences, not original link selection proof."""
from collections import Counter
from dataclasses import replace
from pathlib import Path
import csv
import subprocess

import pytest

from match_recovery.component_binary import verify_toolchain
from match_recovery.component_source import parse_component, render_component
from match_recovery.import_source import read_imports
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from recovery.environment.wine import VC6_ROOT

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/vendor/vc6-runtime.recovery.h"
BASE = "82ee8966f"
EXPECTED = (
    (0x646010, 0x64601D, "_srand", "build/intel/mt_obj/rand.obj", 1),
    (0x64601D, 0x64603F, "_rand", "build/intel/mt_obj/rand.obj", 3),
    (0x6474A0, 0x6474DE, "_strcspn", "build/intel/mt_obj/strcspn.obj", 1),
    (0x649979, 0x6499B7, "__fassign", "../build/intel/mt_obj/cvt.obj", 8),
    (0x649D60, 0x649D94, "__allmul", "build/intel/mt_obj/llmul.obj", 1),
    (0x64C839, 0x64C861, "__set_errno", "../build/intel/mt_obj/fpexcept.obj", 37),
    (0x64C886, 0x64C8B3, "__errcode", "../build/intel/mt_obj/fpexcept.obj", 41),
    (0x64C8C5, 0x64C8EE, "__set_exp", "../build/intel/mt_obj/util.obj", 2),
    (0x64C8EE, 0x64C902, "__get_exp", "../build/intel/mt_obj/util.obj", 3),
    (0x64C902, 0x64C95C, "__sptype", "../build/intel/mt_obj/util.obj", 8),
    (0x64D87E, 0x64D934, "___dtold", "../build/intel/mt_obj/cfout.obj", 2),
    (0x64DB00, 0x64DB3A, "_strpbrk", "build/intel/mt_obj/strpbrk.obj", 1),
    (0x64DCD0, 0x64DD38, "__aulldiv", "build/intel/mt_obj/ulldiv.obj", 1),
    (0x64DD40, 0x64DDB5, "__aullrem", "build/intel/mt_obj/ullrem.obj", 1),
    (0x64FF52, 0x650172, "___ld12mul", "../build/intel/mt_obj/tenpow.obj", 1),
)


def previous():
    return subprocess.check_output(["git", "show", BASE + ":src/vendor/vc6-runtime.recovery.h"], cwd=ROOT, text=True)


def without_strict_frontier(manifest):
    """Project this exact later addition out of older historical controls."""
    from recovery.tests.test_crt_fp_control_oct11 import without_fp_control
    manifest = without_fp_control(manifest)
    owned = {row[2] for row in EXPECTED}
    additions = tuple(b for b in manifest.bindings if b.symbol in owned)
    if not additions:
        return manifest
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit) for b in additions] == [row[:4] for row in EXPECTED]
    assert all(b.spans == (Span(start, end),) for b, (start, end, *_) in zip(additions, EXPECTED))
    return replace(manifest, bindings=tuple(b for b in manifest.bindings if b.symbol not in owned))


def source_without_strict_frontier(text):
    projected = render_component(without_strict_frontier(parse_component(text)))
    begin, end = "// RECOVERY_COMPONENT_BEGIN v1", "// RECOVERY_COMPONENT_END"
    return text[:text.index(begin)] + projected[projected.index(begin):].rstrip("\n") + text[text.index(end) + len(end):]


def test_only_append_fifteen_bounded_bindings():
    before = previous()
    after = SOURCE.read_text()
    old, current = parse_component(before), parse_component(after)
    owned = {b.symbol for b in old.bindings} | {row[2] for row in EXPECTED}
    current = replace(current, bindings=tuple(b for b in current.bindings if b.symbol in owned))
    assert current.bindings[:-15] == old.bindings
    assert replace(current, bindings=old.bindings) == old
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit) for b in current.bindings[-15:]] == [row[:4] for row in EXPECTED]
    assert all("not original archive selection" in b.evidence for b in current.bindings[-15:])
    for delimiter, index in (("// RECOVERY_COMPONENT_BEGIN v1", 0), ("// RECOVERY_COMPONENT_END", 1)):
        assert before.split(delimiter)[index] == after.split(delimiter)[index]


@pytest.fixture(scope="module")
def artifacts():
    image = ROOT / ".opensmacx/game/terranx_original.exe"
    archive = VC6_ROOT / "LIB/LIBCMT.LIB"
    if not image.is_file() or not archive.is_file():
        pytest.skip("pinned executable and external runtime archive required")
    return parse_component(SOURCE.read_text()), image.read_bytes(), archive.read_bytes()


def test_native_strict_rows_and_unchanged_previous_verdicts(artifacts):
    manifest, image, archive = artifacts
    old = parse_component(previous())
    owned = {b.symbol for b in old.bindings} | {row[2] for row in EXPECTED}
    manifest = replace(manifest, bindings=tuple(b for b in manifest.bindings if b.symbol in owned))
    imports = read_imports(ROOT)
    before = verify_toolchain(old, image, archive, import_bindings=imports)
    after = verify_toolchain(manifest, image, archive, import_bindings=imports)
    previous_rows = {r["symbol"]: r for r in before["functions"]}
    current_rows = {r["symbol"]: r for r in after["functions"]}
    assert all(current_rows[symbol] == row for symbol, row in previous_rows.items())
    assert Counter(r["result"] for r in before["functions"]) == {"pass": 45, "unknown": 26}
    assert Counter(r["result"] for r in after["functions"]) == {"pass": 60, "unknown": 26}
    for start, end, symbol, unit, section in EXPECTED:
        row = current_rows[symbol]
        assert row["result"] == row["boundary"] == row["bytes"] == row["references"] == "pass", row
        assert row["span"] == {"start": start, "end": end}
        assert row["unit"] == unit and row["section"] == section
        assert row["alignment_fill"] == 0
    assert after["result"] == "unknown" and after["application_recoveries"] == 0
    assert not after["ready_for_collection"]


def test_independent_catalogue_extents(artifacts):
    catalogue = ROOT / ".opensmacx/game/ida94-functions.csv"
    if not catalogue.is_file():
        pytest.skip("local independent IDA catalogue required")
    with catalogue.open() as stream:
        intervals = {int(row["address"], 16): int(row["end"], 16) for row in csv.DictReader(stream)}
    assert all(intervals[start] == end for start, end, *_ in EXPECTED)


@pytest.mark.parametrize("symbol", [row[2] for row in EXPECTED])
@pytest.mark.parametrize("damage", ("extent", "body"))
def test_correspondence_damage_refuses_pass(artifacts, symbol, damage):
    manifest, image, archive = artifacts
    owned = {b.symbol for b in parse_component(previous()).bindings} | {row[2] for row in EXPECTED}
    bindings = tuple(b for b in manifest.bindings if b.symbol in owned)
    target = next(b for b in bindings if b.symbol == symbol)
    if damage == "extent":
        target = replace(target, spans=(Span(target.origin.address, target.spans[0].end - 1),))
        bindings = tuple(target if b.symbol == symbol else b for b in bindings)
    else:
        import hashlib
        from match_recovery.model import Origin
        parsed = PeImage(image, target.origin.image_sha256)
        section, offset, size = next((s, o, n) for s, o, n in parsed.sections if s.start <= target.origin.address < s.start + n)
        changed = bytearray(image)
        changed[offset + target.origin.address - section.start] ^= 1
        image = bytes(changed)
        pin = hashlib.sha256(image).hexdigest()
        bindings = tuple(replace(b, origin=Origin(pin, "pe:va", b.origin.address)) for b in bindings)
        manifest = replace(manifest, bindings=bindings, data_sections=tuple(
            replace(d, origin=Origin(pin, "pe:va", d.origin.address))
            for d in manifest.data_sections))
    manifest = replace(manifest, bindings=bindings)
    report = verify_toolchain(manifest, image, archive, import_bindings=read_imports(ROOT))
    row = next(r for r in report["functions"] if r["symbol"] == symbol)
    assert row["result"] != "pass", row
