"""Ten dependency interpretations; continuity is not binary/runtime qualification."""

import csv
from dataclasses import fields, replace
from pathlib import Path
import subprocess

from match_recovery.component_source import parse_component, render_component
from test_zlib_inflate_group_bindings_oct10 import EXPECTED as FOURTEEN_MEMBERS, UNITS
from match_recovery.source.reader import read_file, read_text
from match_recovery.source.writer import write
from test_zlib_deflate_tree_group_bindings_oct10 import EXPECTED as SIXTEEN_MEMBERS


ROOT = Path(__file__).resolve().parents[2]
BASE = "3ca66d69914ff1e08a26457cfe4b6f8d63d300a1"
COMPANION = "src/vendor/zlib.recovery.h"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EXPECTED = {
    0x0063CF30: (0x0063D11C, "_deflateInit2_", "deflate.c"),
    0x0063D120: (0x0063D193, "_deflateReset", "deflate.c"),
    0x0063D1A0: (0x0063D413, "_deflate", "deflate.c"),
    0x0063D560: (0x0063D5F3, "_lm_init", "deflate.c"),
    0x00640BB0: (0x00640C22, "__tr_init", "trees.c"),
    0x00640C30: (0x00640E41, "_tr_static_init", "trees.c"),
    0x00640E50: (0x00640EB6, "_init_block", "trees.c"),
    0x00640EC0: (0x00640F33, "_gen_codes", "trees.c"),
    0x00640F40: (0x00640FF3, "__tr_stored_block", "trees.c"),
    0x00641000: (0x00641273, "__tr_align", "trees.c"),
}


def baseline(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT)


def test_prior_twelve_identities_claims_history_and_policy_unchanged():
    old = baseline(COMPANION).decode()
    current = (ROOT / COMPANION).read_text()
    previous, manifest = parse_component(old), parse_component(current)
    assert len(previous.bindings) == 12
    assert manifest.bindings[:12] == previous.bindings
    assert replace(manifest, bindings=previous.bindings, units=previous.units) == previous
    assert current.startswith(old[:old.index("// RECOVERY_COMPONENT_BEGIN")])
    before = {row.address: row for row in read_text(old, ROOT / COMPANION)}
    after = {row.address: row for row in read_file(ROOT / COMPANION)}
    assert set(after) == set(before) | set(EXPECTED) | set(SIXTEEN_MEMBERS) | set(FOURTEEN_MEMBERS)
    for address, row in before.items():
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(after[address], field.name)
        assert row.origin == after[address].origin


def test_ten_exact_identity_routes_and_native_writer_roundtrip():
    text = (ROOT / COMPANION).read_text()
    manifest = parse_component(text)
    assert len(manifest.bindings) == 52
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


def test_spans_agree_with_independently_pinned_catalog_not_candidate_lengths():
    with (ROOT / "docs/recovery/ghidra-functions.csv").open(newline="") as stream:
        catalog = {int(row["address"], 16): row for row in csv.DictReader(stream)
                   if row["program_sha256"] == PIN}
    for address, (end, _, _) in EXPECTED.items():
        assert int(catalog[address]["end_address"], 16) == end
        assert int(catalog[address]["size"]) == end - address


def test_no_upstream_or_application_build_change():
    manifest = parse_component((ROOT / COMPANION).read_text())
    for file in manifest.files:
        path = manifest.source_root + "/" + file.path
        assert (ROOT / path).read_bytes() == baseline(path)
    assert (ROOT / "CMakeLists.txt").read_bytes() == baseline("CMakeLists.txt")
