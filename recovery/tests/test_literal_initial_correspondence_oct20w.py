"""Source-bound initial literal identities for four existing TU-private strings."""
from dataclasses import replace
import hashlib
import json
import os
from pathlib import Path

import pytest

from match_recovery.literal_data import check_literals
from match_recovery.literal_source import read_literals
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
IMAGE_PATH = Path(os.environ.get(
    "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))

# These are reviewed original-object facts. The original image check below is
# independent of the source annotation and prevents this table becoming a
# self-comparison-only acceptance control.
EXPECTED = {
    ("src/famewin.cpp", "??_C@_02JKAF@rb?$AA@"): (0x684620, 0x684623, 0x44AE50, b"rb\0"),
    ("src/famewin.cpp", "??_C@_02NGAF@wb?$AA@"): (0x68460C, 0x68460F, 0x44AE50, b"wb\0"),
    ("src/basewin.cpp", "??_C@_06LABA@RENAME?$AA@"): (
        0x682C70, 0x682C77, 0x41A2B0, b"RENAME\0"),
    ("src/alphanet.cpp", "??_C@_0BK@LCOK@NETCONNECT_JOIN_OR_CREATE?$AA@"): (
        0x689240, 0x68925A, 0x4E29E0, b"NETCONNECT_JOIN_OR_CREATE\0"),
}


def image_and_bindings():
    if not IMAGE_PATH.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = IMAGE_PATH.read_bytes()
    assert hashlib.sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    wanted_units = {unit for unit, _ in EXPECTED}
    bindings = tuple(item for item in read_literals(ROOT, PIN) if item.unit in wanted_units)
    return image, bindings


def check_reviewed_facts(bindings, image):
    actual = {(item.unit, item.symbol): item for item in bindings}
    assert set(actual) == set(EXPECTED)
    for key, (start, end, evidence, contents) in EXPECTED.items():
        item = actual[key]
        assert item.origin.address == start
        assert item.span == Span(start, end)
        assert item.evidence.address == evidence
        assert image.read_initial(item.span) == contents


def test_four_source_bindings_match_reviewed_original_initial_bytes():
    image, bindings = image_and_bindings()
    check_reviewed_facts(bindings, image)


@pytest.mark.parametrize("field", ["start", "end", "symbol", "anchor", "missing"])
def test_source_fact_controls_reject_binding_damage(field):
    image, bindings = image_and_bindings()
    key = next(iter(EXPECTED))
    item = next(bound for bound in bindings if (bound.unit, bound.symbol) == key)
    if field == "start":
        damaged = replace(item, origin=replace(item.origin, address=item.origin.address + 1),
                          span=replace(item.span, start=item.span.start + 1))
    elif field == "end":
        damaged = replace(item, span=replace(item.span, end=item.span.end - 1))
    elif field == "symbol":
        damaged = replace(item, symbol=item.symbol + "_wrong")
    elif field == "anchor":
        damaged = replace(item, evidence=replace(item.evidence, address=item.evidence.address + 1))
    else:
        damaged = None
    changed = [bound for bound in bindings if bound is not item]
    if damaged is not None:
        changed.append(damaged)
    with pytest.raises(AssertionError):
        check_reviewed_facts(tuple(changed), image)


def test_real_coff_literal_storage_when_coordinator_manifest_is_supplied():
    manifest_name = os.environ.get("OPENSMACX_LITERAL_OBJECTS")
    if not manifest_name:
        pytest.skip("set OPENSMACX_LITERAL_OBJECTS to a coordinator-qualified TU/object manifest")
    image, bindings = image_and_bindings()
    manifest_path = Path(manifest_name)
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    units = {unit for unit, _ in EXPECTED}
    assert set(manifest) == units
    objects = {unit: Coff(Path(manifest[unit]).read_bytes()) for unit in units}
    targets, rows = check_literals(bindings, objects, image)
    assert len(rows) == len(EXPECTED)
    assert all(row["definition"] == "pass" and row["content"] == "pass" for row in rows), rows
    assert all(targets[item.unit, item.symbol] is not None for item in bindings)

