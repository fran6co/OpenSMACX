"""Source correspondence controls; binary qualification remains independent."""

import csv
from dataclasses import fields, replace
from pathlib import Path
import subprocess

from match_recovery.component_source import parse_component, render_component
from test_indirect_dependency_frontier_oct10 import restore_import_archive_configuration, restore_prior_ddraw_archive_configuration
from test_zlib_main_state_machine_bindings_oct10 import EXPECTED as MAIN_STATE_MACHINES
from match_recovery.source.reader import read_file, read_text
from match_recovery.source.writer import write

ROOT = Path(__file__).resolve().parents[2]
BASE = "d8936a1c802f766ce8ab4105956e0e532366369f"
COMPANION = "src/vendor/zlib.recovery.h"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
UNITS = {"adler32.c", "deflate.c", "infblock.c", "infcodes.c", "inffast.c",
         "inflate.c", "inftrees.c", "infutil.c", "trees.c", "uncompr.c", "zutil.c"}
EXPECTED = {
    0x00642B10: (0x00642BA2, "_inflate_blocks_reset", "infblock.c"),
    0x00642BB0: (0x00642C1E, "_inflate_blocks_new", "infblock.c"),
    0x00643950: (0x00643981, "_inflate_blocks_free", "infblock.c"),
    0x00644140: (0x00644179, "_inflate_codes_new", "infcodes.c"),
    0x00644910: (0x00644924, "_inflate_codes_free", "infcodes.c"),
    0x00644A70: (0x00644DB2, "_inflate_fast", "inffast.c"),
    0x0063DF40: (0x0063DF84, "_inflateReset", "inflate.c"),
    0x0063DFE0: (0x0063E0EE, "_inflateInit2_", "inflate.c"),
    0x00643990: (0x006439E5, "_inflate_trees_bits", "inftrees.c"),
    0x006439F0: (0x00643EA1, "_huft_build", "inftrees.c"),
    0x00643EB0: (0x00643F98, "_inflate_trees_dynamic", "inftrees.c"),
    0x00643FA0: (0x006440DE, "_inflate_trees_fixed", "inftrees.c"),
    0x00644100: (0x0064413F, "_inflate_trees_free", "inftrees.c"),
    0x00640A50: (0x00640A63, "_zcalloc", "zutil.c"),
}


def baseline(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"], cwd=ROOT)


def test_prior_thirty_eight_identities_history_claims_and_policy_preserved():
    old = baseline(COMPANION).decode()
    current = (ROOT / COMPANION).read_text()
    previous, manifest = parse_component(old), parse_component(current)
    assert len(previous.bindings) == 38
    assert manifest.bindings[:38] == previous.bindings
    assert replace(manifest, bindings=previous.bindings, units=previous.units) == previous
    assert current.startswith(old[:old.index("// RECOVERY_COMPONENT_BEGIN")])
    before = {row.address: row for row in read_text(old, ROOT / COMPANION)}
    after = {row.address: row for row in read_file(ROOT / COMPANION)}
    assert set(after) == set(before) | set(EXPECTED) | set(MAIN_STATE_MACHINES)
    for address, row in before.items():
        for field in fields(row):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(row, field.name) == getattr(after[address], field.name)
        assert row.origin == after[address].origin


def test_fourteen_exact_routes_unclaimed_and_native_roundtrips():
    text = (ROOT / COMPANION).read_text()
    manifest = parse_component(text)
    assert len(manifest.bindings) == 56
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
    assert 0x0063D600 not in EXPECTED  # independently native-bounded in the later dependency-frontier step


def test_fourteen_extents_have_independent_original_catalog_evidence():
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
        current = (ROOT / path).read_bytes()
        if path == "recovery/project.json":
            current = restore_prior_ddraw_archive_configuration(restore_import_archive_configuration(current))
        assert current == baseline(path)
