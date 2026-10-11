"""Correct distinct-function overlaps without deleting code or claims."""
from dataclasses import fields, replace
from pathlib import Path
import csv
import json
import subprocess

import pytest
from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes

ROOT = Path(__file__).resolve().parents[2]
BASE = '635b9085b7a426bb3b20f5fd3175af1237ae5174'
FIXED = '18f93db66f4f21dc74122f759676e95aee5b3126'
EXTENTS = {
    0x4CD4E0: ('src/veh.cpp', 0x4CD6F9, 0x4CD69D, 0x4CD6A0),
    0x42B940: ('src/datalink.cpp', 0x42BACC, 0x42BAA3, 0x42BAB0),
    0x5CED1D: ('src/movie_media_decoder.cpp', 0x5CEF8A, 0x5CEE80, 0x5CEE80),
}


@pytest.mark.parametrize('address', EXTENTS)
def test_writer_extent_and_exact_bounded_source(address):
    relative, old_end, new_end, successor = EXTENTS[address]
    path = ROOT / relative
    old = subprocess.check_output(['git', 'show', BASE + ':' + relative], cwd=ROOT)
    # The bounded historical writer operation must reproduce its checkpoint,
    # not freeze every unrelated future implementation in these owner files.
    fixed = subprocess.check_output(['git', 'show', FIXED + ':' + relative], cwd=ROOT)
    expected = old.decode()
    fixture = Path(__file__).with_name('function-span-overlaps-oct11.patch').read_text()
    section = next(s for s in fixture.split('diff --git ')[1:]
                   if s.startswith('a/' + relative + ' b/' + relative + '\n'))
    for hunk in section.split('\n@@ ')[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = ''.join(l[1:] for l in lines if l.startswith((' ', '-')))
        after = ''.join(l[1:] for l in lines if l.startswith((' ', '+')))
        assert before and expected.count(before) == 1
        expected = expected.replace(before, after, 1)
    assert expected == fixed.decode()
    before, after = read_text(old.decode(), path), read_text(expected, path)
    assert len(before) == len(after)
    for a, b in zip(before, after):
        wanted = a
        if a.address == address:
            assert not a.byte_exact and a.image_spans == ((address, old_end),)
            wanted = replace(a, image_spans=((address, new_end),))
            generated = write_bytes(old, [wanted]).decode().splitlines()[a.line - 1]
            assert expected.splitlines()[b.line - 1] == generated
        for field in fields(a):
            if field.name not in {'line', 'region', 'origin'}:
                assert getattr(wanted, field.name) == getattr(b, field.name)
        lessons = lambda r: [(l.kind, l.key, l.prose, tuple(t for _, t in l.lines)) for l in r.origin]
        assert lessons(a) == lessons(b)
    following = next(r for r in after if r.address == successor)
    assert following.image_spans[0][0] >= new_end
    current = read_text(path.read_text(), path)
    assert next(r for r in current if r.address == address).image_spans == ((address, new_end),)
    assert next(r for r in current if r.address == successor).image_spans[0][0] >= new_end


@pytest.mark.parametrize('address', EXTENTS)
def test_pinned_complete_body_return_alignment_and_successor(address):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    path = ROOT / '.opensmacx/game/terranx_original.exe'
    if not path.is_file():
        pytest.skip('pinned original unavailable')
    image = PeImage(path.read_bytes(), '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf')
    relative, old_end, end, successor = EXTENTS[address]
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    body = list(decoder.disasm(image.read(Span(address, end)), address))
    assert sum(i.size for i in body) == end - address
    assert body[-1].mnemonic == 'ret' and body[-1].address + body[-1].size == end
    for instruction in body:
        if instruction.mnemonic.startswith('j'):
            assert instruction.op_str.startswith('0x')
            assert address <= int(instruction.op_str, 16) < end
    if end < successor:
        assert image.read(Span(end, successor)) == b'\x90' * (successor - end)
    rows = read_text((ROOT / relative).read_text(), ROOT / relative)
    following = next(r for r in rows if r.address == successor)
    next_end = following.image_spans[0][1]
    next_body = list(decoder.disasm(image.read(Span(successor, next_end)), successor))
    assert next_body and next_body[0].address == successor
    assert sum(i.size for i in next_body) == next_end - successor
    assert next_body[-1].mnemonic == 'ret'
    # The historical span actually attributed successor instructions to this
    # body, rather than merely carrying a few alignment bytes.
    assert old_end > successor


def test_task_preserves_all_original_identities_without_granting_claims():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT / 'recovery/tasks/function-span-overlaps-oct11.json').read_text()))
    assert {s.address for s in task.subjects} == set(EXTENTS)
    assert task.base_revision == BASE
