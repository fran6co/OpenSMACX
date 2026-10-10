"""Source correspondence controls; binary qualification remains independent."""

import csv
from dataclasses import fields, replace
from pathlib import Path
import subprocess

from match_recovery.component_source import parse_component, render_component
from test_zlib_main_state_machine_bindings_oct10 import EXPECTED as MAIN_STATE_MACHINES
from test_zlib_inflate_group_bindings_oct10 import EXPECTED as FOURTEEN_MEMBERS, UNITS
from match_recovery.source.reader import read_file, read_text
from match_recovery.source.writer import write

ROOT = Path(__file__).resolve().parents[2]
BASE = "775f2ff67263c4880a180d88f11a505673593c32"
COMPANION = "src/vendor/zlib.recovery.h"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EXPECTED = {
    0x63D6F0: (0x63D812, "_fill_window", "deflate.c"),
    0x63D820: (0x63D891, "_read_buf", "deflate.c"),
    0x63D8A0: (0x63DACF, "_deflate_fast", "deflate.c"),
    0x63DAD0: (0x63DC3F, "_longest_match", "deflate.c"),
    0x63DC40: (0x63DF33, "_deflate_slow", "deflate.c"),
    0x641280: (0x6414A3, "__tr_flush_block", "trees.c"),
    0x6414B0: (0x6416E3, "_build_tree", "trees.c"),
    0x6416F0: (0x6417C1, "_pqdownheap", "trees.c"),
    0x6417D0: (0x6419F1, "_gen_bitlen", "trees.c"),
    0x641A00: (0x641A6F, "_build_bl_tree", "trees.c"),
    0x641A70: (0x641B57, "_scan_tree", "trees.c"),
    0x641B60: (0x641DC5, "_send_all_trees", "trees.c"),
    0x641DD0: (0x642345, "_send_tree", "trees.c"),
    0x642350: (0x642477, "__tr_tally", "trees.c"),
    0x642480: (0x6428BB, "_compress_block", "trees.c"),
    0x6428C0: (0x642934, "_set_data_type", "trees.c"),
}


def baseline(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT)


def test_prior_twenty_two_identities_history_claims_and_policy_preserved():
    old = baseline(COMPANION).decode()
    current = (ROOT / COMPANION).read_text()
    previous, manifest = parse_component(old), parse_component(current)
    assert len(previous.bindings) == 22
    assert manifest.bindings[:22] == previous.bindings
    assert replace(manifest, bindings=previous.bindings, units=previous.units) == previous
    assert current.startswith(old[:old.index("// RECOVERY_COMPONENT_BEGIN")])
    before = {row.address: row for row in read_text(old, ROOT / COMPANION)}
    after = {row.address: row for row in read_file(ROOT / COMPANION)}
    assert set(after) == set(before) | set(EXPECTED) | set(FOURTEEN_MEMBERS) | set(MAIN_STATE_MACHINES)
    for address, row in before.items():
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(after[address], field.name)
        assert row.origin == after[address].origin


def test_sixteen_exact_routes_unclaimed_and_native_roundtrips():
    text = (ROOT / COMPANION).read_text()
    manifest = parse_component(text)
    assert len(manifest.bindings) == 55
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
    assert 0x0063D600 not in bindings  # outside this pinned-Ghidra-witnessed set


def test_sixteen_extents_have_independent_original_catalog_evidence():
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
