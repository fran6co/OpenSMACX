"""Complete original body extents without source/claim substitution."""
from dataclasses import fields, replace
import json
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes
from test_verified_tail_claims_oct11 import (
    RECORDED, apply_verified_tail_claims, expected_recorded_tail,
)

ROOT = Path(__file__).resolve().parents[2]
BASE = "32729ca84"
FIXTURE = Path(__file__).with_name("function-span-tails-oct11.patch")
TAILS = {
    0x617DB0: ("src/caviar.cpp", 0x617F12, 0x617F1F),
    0x618E20: ("src/caviar.cpp", 0x618E2F, 0x618E30),
    0x4B9E90: ("src/statuswin.cpp", 0x4B9E97, 0x4B9E9D),
}


def apply_span_tail_migration(source, relative):
    section = next(part for part in FIXTURE.read_text().split("diff --git ")[1:]
                   if part.startswith("a/" + relative + " b/" + relative + "\n"))
    for hunk in section.split("\n@@ ")[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = "".join(line[1:] for line in lines if line.startswith((" ", "-")))
        after = "".join(line[1:] for line in lines if line.startswith((" ", "+")))
        assert before and source.count(before) == 1
        source = source.replace(before, after, 1)
    return source


def expected_extended_record(record):
    if record.address not in TAILS:
        return record
    relative, old_end, new_end = TAILS[record.address]
    assert record.image_spans in (((record.address, old_end),), ((record.address, new_end),))
    return replace(record, image_spans=((record.address, new_end),))


@pytest.mark.parametrize("relative", ["src/caviar.cpp", "src/statuswin.cpp"])
def test_api_extent_edits_preserve_all_other_source_and_claims(relative):
    path = ROOT / relative
    old = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT)
    current = path.read_bytes()
    migrated = apply_span_tail_migration(old.decode(), relative)
    assert apply_verified_tail_claims(migrated, relative) == current.decode()
    before = {r.address: r for r in read_text(old.decode(), path)}
    after = {r.address: r for r in read_text(current.decode(), path)}
    assert before.keys() == after.keys()
    for address, record in before.items():
        expected = expected_recorded_tail(expected_extended_record(record))
        for field in fields(record):
            if field.name not in {"line", "region", "origin"}:
                assert getattr(expected, field.name) == getattr(after[address], field.name)
        # Inserting durable binding evidence shifts lesson line numbers, not
        # their authored content. Check every lesson without position metadata.
        def lessons(row):
            return [(lesson.kind, lesson.key, lesson.prose,
                     tuple(text for _, text in lesson.lines))
                    for lesson in row.origin]
        assert lessons(record) == lessons(after[address])
        if address in TAILS:
            assert not record.byte_exact
            assert after[address].byte_exact == (address in RECORDED)
            # Generate the marker through the real writer, not a hand-spelled
            # marker expectation. Durable rationale/size are separate edits.
            preview = write_bytes(old, [expected]).decode()
            wanted_line = preview.splitlines()[record.line - 1]
            assert current.decode().splitlines()[after[address].line - 1] == wanted_line


@pytest.mark.parametrize("address", TAILS)
def test_pinned_native_complete_tail_and_adjacent_padding(address):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage
    path = ROOT / ".opensmacx/game/terranx_original.exe"
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    image = PeImage(path.read_bytes(), "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf")
    relative, old_end, end = TAILS[address]
    instructions = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(address, end)), address))
    assert instructions[-1].mnemonic == "ret"
    assert instructions[-1].address + instructions[-1].size == end
    assert sum(i.size for i in instructions) == end - address
    by_address = {i.address: (i.mnemonic, i.op_str) for i in instructions}
    if address == 0x617DB0:
        assert by_address[0x617DBD] == ("je", "0x617f12")
        assert by_address[0x617DC9] == ("je", "0x617f12")
        assert by_address[0x617F15] == ("mov", "eax, 0x10")
        assert by_address[0x617F1C] == ("ret", "0x10")
        assert image.read(Span(end, 0x617F20)) == b"\x90"
    elif address == 0x618E20:
        assert by_address[0x618E26] == ("je", "0x618e2f")
        assert by_address[0x618E29] == ("call", "0x644ef2")
    else:
        assert by_address[0x4B9E97] == ("mov", "dword ptr [0x939ed8], eax")
        assert image.read(Span(end, 0x4B9EA0)) == b"\x90" * 3
    rows = read_text((ROOT / relative).read_text(), ROOT / relative)
    for row in rows:
        if row.address != address:
            assert all(stop <= address or start >= end for start, stop in row.image_spans)


def test_only_twenty_omitted_bytes_added():
    assert sum(end - old for _, old, end in TAILS.values()) == 20


def test_authorization_is_native_task_with_exact_subject_scope():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/function-span-tails-oct11.json").read_text()))
    assert task.task_id == "function-span-tails-oct11"
    assert {subject.address for subject in task.subjects} == set(TAILS)
    assert all(subject.address_space == "pe:va" and subject.image_sha256 ==
               "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
               for subject in task.subjects)
