"""Original dispatch-storage facts and exact comment-only source transport.

These controls do not waive unsupported physical COMDAT/gap/reference cases.
The full application build and strict comparison remain separate requirements.
"""
from collections import defaultdict
from dataclasses import replace
from functools import cache
from hashlib import sha256
import os
from pathlib import Path
import struct
import subprocess

import pytest

from match_recovery.associated_source import parse_associated, read_associated, validate_associated
from match_recovery.model import Span
from match_recovery.source import read_file
from match_recovery.tasks import TaskStore


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
BASE = "31b3b481c9fd83784323ac60e8bb659a80c5295c"
PUBLISHED = "3b55542d34a3ef9a18b9cc1289e97cba5459ae34"
TASK = "switch-owner-storage-oct18"


def population(rows):
    task = TaskStore(ROOT).read(TASK)
    assert task.base_revision == BASE
    owners = {subject.address for subject in task.subjects}
    assert len(owners) == 17
    validate_associated(rows)
    added = tuple(row for row in rows if row.owner.address in owners)
    assert len(added) == 36
    assert {row.owner.address for row in added} == owners
    # Earlier utility storage remains with its real source owner. The numeric
    # owner move preserves both stoi records; HSV stays in General. These
    # mappings are not part of this annotation-publication cohort.
    utilities = {
        (row.owner.address, row.span, row.kind, row.evidence.address, row.unit)
        for row in rows
        if row.unit in {"src/general.cpp", "src/number_parsing.cpp"}
    }
    assert utilities == {
        (0x628950, Span(0x628A00, 0x628A10), "switch_targets", 0x628976,
         "src/number_parsing.cpp"),
        (0x628950, Span(0x628A10, 0x628A47), "switch_selector", 0x628970,
         "src/number_parsing.cpp"),
        (0x628B70, Span(0x628D94, 0x628DAC), "switch_targets", 0x628C65,
         "src/general.cpp"),
    }
    return added


@cache
def bindings():
    return population(read_associated(ROOT, PIN))


def committed(unit, revision=BASE):
    return subprocess.run(["git", "show", f"{revision}:{unit}"], cwd=ROOT,
                          check=True, capture_output=True).stdout


def inverse(raw, unit):
    records = parse_associated(raw.decode(), unit, PIN, address_space="pe:va")
    owners = {row.owner.address for row in bindings()}
    lines = {row.line for row in records if row.owner.address in owners}
    return b"".join(line for number, line in enumerate(raw.splitlines(keepends=True), 1)
                    if number not in lines)


def test_complete_source_population_and_published_checkpoint_exact_inverse():
    rows = bindings()
    for unit in sorted({row.unit for row in rows}):
        # Transport is a fact about this immutable publication, not a ban on
        # later genuine recovery in those ten live translation units.
        assert inverse(committed(unit, PUBLISHED), unit) == committed(unit)


@pytest.mark.parametrize("damage", ["missing", "duplicate", "additional"])
def test_population_refuses_lost_duplicated_or_extra_bindings(damage):
    rows = read_associated(ROOT, PIN)
    added = bindings()[0]
    if damage == "missing":
        changed = tuple(row for row in rows if row != added)
    elif damage == "duplicate":
        changed = (*rows, added)
    else:
        extra = replace(added, span=Span(0x10000000, 0x10000004))
        changed = (*rows, extra)
    with pytest.raises((AssertionError, ValueError)):
        population(changed)


@pytest.mark.parametrize("damage", ["body", "claim", "unrelated-comment"])
def test_inverse_rejects_changes_other_than_the_authorized_annotations(damage):
    unit = "src/prefwin.cpp"
    original = committed(unit, PUBLISHED)
    if damage == "body":
        damaged = original.replace(b"switch (", b"switch (1 + ", 1)
    elif damage == "claim":
        damaged = original.replace(b"BYTE_EXACT", b"OLD_CLAIM", 1)
    else:
        damaged = original + b"\n// unrelated edit\n"
    assert damaged != original
    assert inverse(damaged, unit) != committed(unit)


@cache
def original():
    from match_recovery.pe_coff import PeImage
    configured = os.environ.get("OPENSMACX_IMAGE")
    paths = ([Path(configured)] if configured else
             [parent / ".opensmacx/game/terranx_original.exe" for parent in (ROOT, *ROOT.parents)])
    path = next((path for path in paths if path.is_file()), None)
    if path is None:
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    return PeImage(raw, PIN)


@cache
def dispatches():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.associated_binary import _anchor_field, _memory_operand, _prove_dispatch_extent

    grouped = defaultdict(list)
    for row in bindings():
        grouped[row.owner.address].append(row)
    image = original()
    engine = Cs(CS_ARCH_X86, CS_MODE_32)
    engine.detail = True
    result = []
    for owner, rows in sorted(grouped.items()):
        record, = [record for record in read_file(ROOT / rows[0].unit) if record.address == owner]
        body, = [Span(start, end) for start, end in record.image_spans]
        instructions = list(engine.disasm(image.read(body), body.start))
        assert instructions[0].address == owner
        assert sum(ins.size for ins in instructions) == body.end - body.start
        by_address = {ins.address: ins for ins in instructions}
        for table in [row for row in rows if row.kind == "switch_targets"]:
            selectors = [row for row in rows if row.kind == "switch_selector" and row.span.start == table.span.end]
            assert len(selectors) <= 1
            selector = selectors[0] if selectors else None
            jump = by_address[table.evidence.address]
            operand = _memory_operand(jump, table.kind)
            _anchor_field(image, jump, operand, table.span.start)
            load = by_address[selector.evidence.address] if selector else None
            load_operand = _memory_operand(load, selector.kind) if selector else None
            if selector:
                _anchor_field(image, load, load_operand, selector.span.start)
                assert not image.absolute_sites(selector.span)
            if owner == 0x635870:
                # Five indirect jumps make the current single-dispatch guard
                # proof refuse. Inspect the independent local guard facts but
                # do not claim a full dominating-guard/envelope proof for DDInit.
                compare_address, branch_address, maximum = {
                    0x635964: (0x635953, 0x635956, 0x4B),
                    0x6359D5: (0x6359C4, 0x6359C7, 0x66),
                    0x635A9C: (0x635A89, 0x635A8E, 0xB4),
                    0x635C2A: (0x635C17, 0x635C1C, 0x96),
                    0x635DA5: (0x635D94, 0x635D97, 0x6C),
                }[jump.address]
                assert by_address[compare_address].mnemonic == "cmp"
                assert by_address[compare_address].op_str == f"eax, 0x{maximum:x}"
                assert by_address[branch_address].mnemonic == "ja"
                assert selector and selector.span.end - selector.span.start == maximum + 1
                with pytest.raises(ValueError, match="unambiguous unsigned dominating guard"):
                    _prove_dispatch_extent(instructions, body, image, table, jump, operand,
                                           selector, load, load_operand)
                proof = None
            else:
                proof = _prove_dispatch_extent(instructions, body, image, table, jump, operand,
                                               selector, load, load_operand)
            count = (table.span.end - table.span.start) // 4
            if proof:
                assert proof["target_count"] == count
            assert image.absolute_sites(table.span) == frozenset(range(0, 4 * count, 4))
            targets = struct.unpack(f"<{count}I", image.read(table.span))
            assert set(targets) <= by_address.keys()
            if selector:
                values = image.read(selector.span)
                assert max(values) + 1 == count
                if proof:
                    assert proof["selector_count"] == len(values)
            result.append((instructions, body, image, table, jump, operand, selector, load, load_operand))
    assert len(result) == 21  # Five independent DDInit dispatches share one owner.
    return tuple(result)


def supported_dispatches():
    return tuple(row for row in dispatches() if row[1].start != 0x635870)


def test_original_guards_anchor_full_domains_and_every_target_cell():
    assert dispatches()


@pytest.mark.parametrize("index", range(16))
def test_truncated_table_cannot_satisfy_original_guard_domain(index):
    from match_recovery.associated_binary import _prove_dispatch_extent
    values = list(supported_dispatches()[index])
    table = values[3]
    values[3] = replace(table, span=Span(table.span.start, table.span.end - 4))
    with pytest.raises(ValueError, match="guard-derived|target table|selector.*domain"):
        _prove_dispatch_extent(*values)


def test_shortened_selector_maps_cannot_hide_in_domain_inputs():
    from match_recovery.associated_binary import _prove_dispatch_extent
    count = 0
    for current in supported_dispatches():
        values = list(current)
        selector = values[6]
        if selector is None:
            continue
        values[6] = replace(selector, span=Span(selector.span.start, selector.span.end - 1))
        with pytest.raises(ValueError, match="selector map length"):
            _prove_dispatch_extent(*values)
        count += 1
    assert count == 10
