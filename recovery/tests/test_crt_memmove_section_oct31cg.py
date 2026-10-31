"""Pinned memmove contribution/size-free entry; not original link acceptance."""
import csv
from dataclasses import replace
import hashlib
import struct
import subprocess
import pytest
from match_recovery.coff_archive import CoffArchive
from match_recovery.component_source import parse_component, render_component
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, CoffRecords, PeImage
from test_crt_threadstart_contribution_oct19 import (
    ROOT, IMAGE_SHA256, inputs, repository_root, run_check, repin, raw_offset,
)
from test_crt_memcpy_section_oct19 import require_prior_cohort

BASELINE = 'e102ce2d138fb100a8785d3659c3e4af6972a053'
MEMBER = 'build/intel/mt_obj/memmove.obj'
MEMBER_SHA256 = '2d3c61f52e9d6896ad2d7b9d4da4cba7289e91ffd1498045d0c286fe19bc3fdc'
START, END = 0x6474E0, 0x647815

def contribution(manifest):
    rows = [s for s in manifest.code_sections if (s.unit, s.section) == (MEMBER, 1)]
    assert len(rows) == 1
    return rows[0]

def changed_section(manifest, section):
    old = contribution(manifest)
    return replace(manifest, code_sections=tuple(section if s == old else s for s in manifest.code_sections))

def selected_row(manifest, image, archive):
    rows = run_check(manifest, image, archive)['code_sections']
    result = [r for r in rows if r['unit'] == MEMBER]
    assert len(result) == 1
    return result[0]

def test_actual_additive_manifest_and_unique_archive_entry():
    manifest, _, image, archive = inputs()
    prior = parse_component(subprocess.check_output(
        ['git', 'show', f'{BASELINE}:src/vendor/vc6-runtime.recovery.h'],
        cwd=ROOT, text=True))
    require_prior_cohort(manifest, prior)
    section = contribution(manifest)
    assert section.span == Span(START, END) and section.section_name == '.text'
    assert section.comdat_selection is None
    assert [(e.name, e.offset) for e in section.entries] == [('_memmove', 0)]
    assert not any(b.symbol == '_memmove' for b in manifest.bindings)
    assert parse_component(render_component(manifest)) == manifest
    parsed = CoffArchive(archive); member = parsed.member(MEMBER)
    assert hashlib.sha256(member).hexdigest() == MEMBER_SHA256
    obj = Coff(member); code = obj.sections[0]
    assert len(code.data) == END - START == 821
    assert not code.flags & 0x1000 and code.flags & 0x20000020 == 0x20000020
    providers = []
    for name, bodies in parsed.members.items():
        for body in bodies:
            providers += [(name, s.value, s.section, s.type) for s in CoffRecords(body).symbols.values()
                          if s.name == '_memmove' and s.storage == 2 and s.section > 0]
    assert providers == [(MEMBER, 0, 1, 0x20)]
    assert len(code.relocations) == 46
    assert all(kind == 6 and obj.symbols[target].section == 1 and obj.symbols[target].storage == 3
               for _, target, kind in code.relocations)
    original = PeImage(image, IMAGE_SHA256)
    assert original.absolute_sites(Span(START, END)) == frozenset(off for off, _, _ in code.relocations)

def test_independent_disassembler_boundaries_and_names_remain_distinct():
    with (repository_root() / '.opensmacx/game/ida94-functions.csv').open(newline='') as f:
        ida = [r for r in csv.DictReader(f) if int(r['address'], 16) == START]
    assert len(ida) == 1 and ida[0]['name'] == '_memcpy_0' and int(ida[0]['end'], 16) == END
    with (ROOT / 'docs/recovery/ghidra-functions.csv').open(newline='') as f:
        gh = [r for r in csv.DictReader(f) if r['program_sha256'] == IMAGE_SHA256
              and r['ghidra_version'] == '12.1.2' and int(r['address'], 16) == START]
    assert len(gh) == 1 and gh[0]['name'] == 'FUN_006474e0'
    spans = [Span(*(int(x,16) for x in p.split('-'))) for p in gh[0]['body_ranges'].split(';')]
    assert len(spans) == 21 and sum(s.end - s.start for s in spans) == int(gh[0]['size']) == 664
    assert spans[0].start == START and spans[-1].end == END
    assert int(gh[0]['end_address'], 16) == spans[0].end == 0x64753B

def test_native_complete_section_and_scoped_entry_only():
    manifest, _, image, archive = inputs()
    row = selected_row(manifest, image, archive)
    assert all(row[k] == 'pass' for k in ('result','boundary','bytes','nonrelocation_bytes','references','entry_inventory'))
    assert len(row['relocations']) == len(row['highlow_offsets']) == 46
    assert all(r['result'] == 'pass' for r in row['relocations'])
    assert row['callable'] is False and row['link_target'] is False
    entry, = row['entries']
    assert entry['symbol'] == '_memmove' and entry['offset'] == 0 and entry['address'] == START
    assert entry['scoped_reference_target'] is True and entry['function_extent'] == 'unknown'
    assert entry['link_target'] is False

@pytest.mark.parametrize('damage', ['short', 'entry_offset', 'local_target'])
def test_native_damage_preserves_binary_and_entry_dimensions(damage):
    manifest, _, image, archive = inputs()
    section = contribution(manifest)
    if damage == 'short':
        manifest = changed_section(manifest, replace(section, span=Span(START, END-1)))
    elif damage == 'entry_offset':
        manifest = changed_section(manifest, replace(section, entries=(replace(section.entries[0], offset=1),)))
    else:
        changed = bytearray(image)
        at = raw_offset(image, START+56)
        value, = struct.unpack_from('<I', changed, at)
        struct.pack_into('<I', changed, at, value+4)
        image = bytes(changed)
        manifest = repin(manifest, image)
    row = selected_row(manifest, image, archive)
    assert row['entries'][0]['scoped_reference_target'] is False
    if damage == 'entry_offset':
        # Whole contribution bytes can pass while the declared entry fails.
        assert row['entry_inventory'] == 'fail'
    elif damage == 'local_target':
        assert row['references'] == 'fail'
    else:
        assert row['boundary'] != 'pass'
