"""Source correspondence controls; binary qualification remains independent."""

import csv
from dataclasses import fields, replace
from pathlib import Path
import subprocess

from match_recovery.component_source import parse_component, render_component
from match_recovery.source.reader import read_file, read_text
from match_recovery.source.writer import write

ROOT = Path(__file__).resolve().parents[2]
BASE = "a964c27bd332afcb97b9c84b965d30f2731667ac"
COMPANION = "src/vendor/zlib.recovery.h"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
UNITS = {"adler32.c", "deflate.c", "infblock.c", "infcodes.c", "inffast.c",
         "inflate.c", "inftrees.c", "infutil.c", "trees.c", "uncompr.c", "zutil.c"}
EXPECTED = {
    0x0063E110: (0x0063E4E8, "_inflate", "inflate.c"),
    0x00642C20: (0x0064390D, "_inflate_blocks", "infblock.c"),
    0x00644180: (0x006448E6, "_inflate_codes", "infcodes.c"),
}


def baseline(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT)


def test_prior_fifty_two_identities_history_claims_and_policy_preserved():
    old = baseline(COMPANION).decode()
    current = (ROOT / COMPANION).read_text()
    previous, manifest = parse_component(old), parse_component(current)
    assert len(previous.bindings) == 52
    assert manifest.bindings[:52] == previous.bindings
    assert replace(manifest, bindings=previous.bindings, units=previous.units) == previous
    assert current.startswith(old[:old.index("// RECOVERY_COMPONENT_BEGIN")])
    before = {row.address: row for row in read_text(old, ROOT / COMPANION)}
    after = {row.address: row for row in read_file(ROOT / COMPANION)}
    assert set(after) == set(before) | set(EXPECTED)
    for address, row in before.items():
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(after[address], field.name)
        assert row.origin == after[address].origin


def test_three_state_machines_exact_routes_unclaimed_and_native_roundtrips():
    text = (ROOT / COMPANION).read_text()
    manifest = parse_component(text)
    assert len(manifest.bindings) == 55
    assert set(manifest.units) == UNITS
    assert parse_component(render_component(manifest)) == manifest
    assert write(text, read_file(ROOT / COMPANION)) == text
    rows = {row.address: row for row in read_file(ROOT / COMPANION)}
    bindings = {binding.origin.address: binding for binding in manifest.bindings}
    for address, (end, symbol, unit) in EXPECTED.items():
        row, binding = rows[address], bindings[address]
        assert row.name == binding.symbol == symbol
        assert row.image_spans == ((address, end),)
        assert [(span.start, span.end) for span in binding.spans] == [(address, end)]
        assert binding.unit == unit
        assert binding.origin.image_sha256 == PIN
        assert not row.byte_exact and not row.semantic
        assert "not a current byte claim" in binding.evidence
        assert "any missing data/string/helper evidence must remain unknown" in binding.evidence
        assert "retained selection" in binding.evidence
        assert "compiler switch tables beyond this code span" in binding.evidence
        assert "Preserve the code-only span and native boundary failure" in binding.evidence
        assert "do not extend it" in binding.evidence
    assert 0x0063D600 not in bindings  # outside this pinned-Ghidra-witnessed set


def test_three_state_machines_extents_have_independent_original_catalog_evidence():
    with (ROOT / "docs/recovery/ghidra-functions.csv").open(newline="") as stream:
        catalog = {int(row["address"], 16): row for row in csv.DictReader(stream)
                   if row["program_sha256"] == PIN}
    for address, (end, _, _) in EXPECTED.items():
        assert int(catalog[address]["end_address"], 16) == end
        assert int(catalog[address]["size"]) == end - address


def test_upstream_archive_inputs_and_application_policy_unchanged():
    manifest = parse_component((ROOT / COMPANION).read_text())
    for file in manifest.files:
        path = manifest.source_root + "/" + file.path
        assert (ROOT / path).read_bytes() == baseline(path)
    for path in ("CMakeLists.txt", "recovery/project.json", "recovery/build.json"):
        assert (ROOT / path).read_bytes() == baseline(path)
