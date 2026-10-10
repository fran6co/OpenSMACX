"""Source-owned dependency continuity; fresh compiler checks remain separate."""

from dataclasses import fields, replace
from pathlib import Path
import subprocess

from match_recovery.component_source import parse_component, render_component
from match_recovery.source.reader import read_file, read_text
from match_recovery.source.writer import write

from test_zlib_ten_member_bindings_oct10 import EXPECTED as TEN_MEMBERS


ROOT = Path(__file__).resolve().parents[2]
BASE = "af2878e4788199a72fede56dec4f263a45786952"
COMPANION = "src/vendor/zlib.recovery.h"
EXPECTED = {
    0x00642960: (0x006429EB, "_bi_flush"),
    0x006429F0: (0x00642A6C, "_bi_windup"),
    0x00642A70: (0x00642B07, "_copy_block"),
}


def baseline(path):
    return subprocess.check_output(["git", "show", f"{BASE}:{path}"],
                                   cwd=ROOT)


def test_prior_dependency_history_and_configuration_are_retained():
    old = baseline(COMPANION).decode()
    current = (ROOT / COMPANION).read_text()
    previous = parse_component(old)
    manifest = parse_component(current)
    assert manifest.bindings[:len(previous.bindings)] == previous.bindings
    assert replace(manifest, bindings=previous.bindings) == previous
    # Historical prose/envelopes are unchanged; new envelopes are inserted
    # immediately before the component block, not into upstream code.
    assert current.startswith(old[:old.index("// RECOVERY_COMPONENT_BEGIN")])
    old_rows = {row.address: row for row in read_text(old, ROOT / COMPANION)}
    rows = {row.address: row for row in read_file(ROOT / COMPANION)}
    assert set(rows) == set(old_rows) | set(EXPECTED) | set(TEN_MEMBERS)
    for address, before in old_rows.items():
        after = rows[address]
        for field in fields(before):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(before, field.name) == getattr(after, field.name)
        assert before.origin == after.origin


def test_new_originals_join_exact_upstream_bindings_without_claims():
    source = (ROOT / COMPANION).read_text()
    manifest = parse_component(source)
    assert parse_component(render_component(manifest)) == manifest
    assert write(source, read_file(ROOT / COMPANION)) == source
    rows = {row.address: row for row in read_file(ROOT / COMPANION)}
    bindings = {b.origin.address: b for b in manifest.bindings}
    for address, (end, symbol) in EXPECTED.items():
        row, binding = rows[address], bindings[address]
        assert row.name == binding.symbol == symbol
        assert row.image_spans == ((address, end),)
        assert [(s.start, s.end) for s in binding.spans] == [(address, end)]
        assert binding.unit == "trees.c"
        assert not row.byte_exact and not row.semantic
        assert "not a current byte claim" in binding.evidence
        assert "retained selection" in binding.evidence
    assert "_bi_windup at0x006429F0" in bindings[0x00642A70].evidence


def test_selected_upstream_files_and_application_build_are_unchanged():
    manifest = parse_component((ROOT / COMPANION).read_text())
    for file in manifest.files:
        path = manifest.source_root + "/" + file.path
        assert (ROOT / path).read_bytes() == baseline(path)
    assert (ROOT / "CMakeLists.txt").read_bytes() == baseline("CMakeLists.txt")
