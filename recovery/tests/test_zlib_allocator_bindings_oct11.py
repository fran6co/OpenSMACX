"""Two upstream callback identities; not application recovery or data proof."""
from dataclasses import fields, replace
import csv
import json
from pathlib import Path
import subprocess

import pytest

from match_recovery.component_source import BEGIN, parse_component, render_component
from match_recovery.source import read_file, read_text, write

ROOT = Path(__file__).resolve().parents[2]
BASE = "a85e8b9e467e6d4861df2c513d03ba70879d3460"
COMPANION = "src/vendor/zlib.recovery.h"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
EXPECTED = {
    0x640A70: (0x640A7C, "_zcfree", "zutil.c"),
    0x6440E0: (0x6440F8, "_falloc", "inftrees.c"),
}


def restore_allocator_source(source):
    """Reverse only the checked-in two-binding source edit, not arbitrary work."""
    patch = Path(__file__).with_name("zlib-allocator-bindings-oct11.patch").read_text()
    for hunk in reversed(patch.split("\n@@ ")[1:]):
        lines = hunk.splitlines(keepends=True)[1:]
        before = "".join(line[1:] for line in lines if line.startswith((" ", "-")))
        after = "".join(line[1:] for line in lines if line.startswith((" ", "+")))
        assert after and source.count(after) == 1
        source = source.replace(after, before, 1)
    return source


def without_allocator_bindings(manifest):
    additions = manifest.bindings[-2:]
    assert [(b.origin.address, b.spans[0].end, b.symbol, b.unit)
            for b in additions] == [(a, *details) for a, details in EXPECTED.items()]
    return replace(manifest, bindings=manifest.bindings[:-2])


def baseline(relative):
    return subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT)


def test_all_prior_history_claims_and_component_policy_preserved():
    path = ROOT / COMPANION
    old, current = baseline(COMPANION).decode(), path.read_text()
    previous, manifest = parse_component(old), parse_component(current)
    assert restore_allocator_source(current) == old
    assert without_allocator_bindings(manifest) == previous
    assert current.startswith(old[:old.index(BEGIN)])
    before = {r.address: r for r in read_text(old, path)}
    after = {r.address: r for r in read_file(path)}
    assert set(after) == set(before) | set(EXPECTED)
    for address, record in before.items():
        for field in fields(record):
            if field.name not in {"line", "region"}:
                assert getattr(record, field.name) == getattr(after[address], field.name)
    assert parse_component(render_component(manifest)) == manifest
    assert write(current, read_file(path)) == current
    for binding in manifest.bindings[-2:]:
        row = after[binding.origin.address]
        assert binding.origin.image_sha256 == PIN
        assert binding.origin.address_space == "pe:va"
        assert row.name == binding.symbol
        assert row.image_spans == tuple((s.start, s.end) for s in binding.spans)
        assert not row.byte_exact and not row.semantic
    assert "fixed_mem" in manifest.bindings[-1].evidence
    assert "remain unknown" in manifest.bindings[-1].evidence
    assert not any(b.symbol == "_fixed_mem" for b in manifest.bindings)


def test_no_upstream_or_build_policy_changes():
    manifest = parse_component((ROOT / COMPANION).read_text())
    for file in manifest.files:
        relative = manifest.source_root + "/" + file.path
        assert (ROOT / relative).read_bytes() == baseline(relative)
    for relative in ("CMakeLists.txt", "recovery/project.json", "recovery/build.json"):
        assert (ROOT / relative).read_bytes() == baseline(relative)


@pytest.mark.parametrize("address", EXPECTED)
def test_complete_pinned_native_extent(address):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned image unavailable")
    image = PeImage(path.read_bytes(), PIN)
    end, _, _ = EXPECTED[address]
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(address, end)), address))
    assert sum(i.size for i in instructions) == end - address
    assert instructions[-1].mnemonic == "ret"
    assert instructions[-1].address + instructions[-1].size == end
    if address == 0x640A70:
        assert any(i.mnemonic == "call" and i.op_str == "0x644ef2" for i in instructions)
    else:
        assert any(i.mnemonic == "lea" and "0x9bf410" in i.op_str for i in instructions)


def test_task_is_valid_and_exactly_pinned():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/zlib-allocator-bindings-oct11.json").read_text()))
    assert {s.address for s in task.subjects} == set(EXPECTED)
    assert all(s.image_sha256 == PIN and s.address_space == "pe:va" for s in task.subjects)


def test_independent_ida_extents_agree():
    with (ROOT / "docs/recovery/ida9-functions.csv").open(newline="") as stream:
        rows = {int(row["address"], 16): row for row in csv.DictReader(stream)}
    for address, (end, _, _) in EXPECTED.items():
        row = rows[address]
        assert int(row["end_address"], 16) == end
        assert int(row["size"]) == end - address
        assert row["body_ranges"].upper() == f"0x{address:08X}-0x{end:08X}".upper()
