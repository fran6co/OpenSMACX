"""Selected runtime correspondence; not application coverage or full closure."""
from dataclasses import replace
import csv
import hashlib
import json
from pathlib import Path
import subprocess

import pytest
from match_recovery.component_source import parse_component, render_component
from match_recovery.coff_archive import CoffArchive
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.model import Span
from test_crt_eight_entry_bindings_oct10 import inputs, verify, row, repin, raw_offset

ROOT = Path(__file__).resolve().parents[2]
BASE = '86cbf7e2cdf108524130ba2200f62e7c29528c92'
SOURCE = 'src/vendor/vc6-runtime.recovery.h'
EXPECTED = (
    (0x645D1C, 0x645DA7, '_atol', 'atox', 1),
    (0x645DA7, 0x645DB2, '_atoi', 'atox', 3),
    (0x64AA36, 0x64AA3F, '__errno', 'dosmap', 4),
    (0x64AA3F, 0x64AA48, '___doserrno', 'dosmap', 6),
    (0x6491B0, 0x6491C3, '__initptd', 'tidtable', 6),
    (0x6491C3, 0x64922A, '__getptd', 'tidtable', 8),
    (0x64922A, 0x6492CA, '__freeptd', 'tidtable', 10),
)
QUALIFIED = {0x645DA7, 0x64AA36, 0x64AA3F}
MEMBERS = {'atox': '1ef88177c1cc4517e658fdfaec788a2f47bccd7f56a04f957cd075ef51f569d4',
           'dosmap': '687d2b49e9b78f360c6dd2cd137b89ec11dd446c022c3b8729d72296365f550a',
           'tidtable': 'f9f509000e8996dd877fc89a0f9e2f92f22da4c3bdbd229557a45707e5ca371f'}


def without_atox_errno(manifest):
    tail = manifest.bindings[-len(EXPECTED):]
    assert tuple((b.origin.address, b.spans[0].end, b.symbol, b.unit) for b in tail) == tuple(
        (a, e, s, 'build/intel/mt_obj/'+u+'.obj') for a,e,s,u,_ in EXPECTED)
    assert all(b.spans == (Span(a,e),) for b,(a,e,*_) in zip(tail,EXPECTED))
    return replace(manifest, bindings=manifest.bindings[:-len(EXPECTED)])


def test_exact_append_only_metadata_and_native_rendering():
    before = subprocess.check_output(['git','show',BASE+':'+SOURCE],cwd=ROOT,text=True)
    text = (ROOT/SOURCE).read_text()
    current, old = parse_component(text), parse_component(before)
    assert without_atox_errno(current) == old
    assert parse_component(render_component(current)) == current
    old_last = next(l for l in reversed(before.splitlines()) if '"origin": "0x' in l)
    new_lines = [l for l in render_component(current).splitlines()
                 if any('"origin": "0x%08X"' % a in l for a,*_ in EXPECTED)]
    assert len(new_lines) == len(EXPECTED) and before.count(old_last+'\n') == 1
    assert text == before.replace(old_last+'\n', old_last+',\n'+'\n'.join(new_lines)+'\n',1)


def test_three_closed_rows_and_four_explicit_unknowns_preserve_prior_results(inputs):
    manifest, _, archive, _ = inputs
    previous, current = verify(inputs,without_atox_errno(manifest)), verify(inputs)
    assert current['functions'][:-len(EXPECTED)] == previous['functions']
    for key in ('data_sections','absolute_symbols'):
        assert current[key] == previous[key]
    physical = CoffArchive(archive)
    for a,e,name,unit,section in EXPECTED:
        member = physical.member('build/intel/mt_obj/'+unit+'.obj')
        assert hashlib.sha256(member).hexdigest() == MEMBERS[unit]
        obj = Coff(member)
        symbol = obj.function(name,comdat=False)
        assert (symbol.section,symbol.value) == (section,0)
        assert len(obj.sections[section-1].data) == e-a
        result = row(current,a)
        if a in QUALIFIED:
            assert all(result[k] == 'pass' for k in ('result','boundary','bytes','references')), result
        else:
            assert result['result'] == 'unknown', result
    assert current['result'] == previous['result'] == 'unknown'
    assert current['application_recoveries'] == 0 and not current['ready_for_collection']


def test_independent_catalogues_and_complete_original_bounds(inputs):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    manifest, data, _, _ = inputs
    ida = ROOT/'.opensmacx/game/ida94-functions.csv'
    if not ida.is_file(): pytest.skip('independent IDA catalogue unavailable')
    with ida.open() as f: rows = list(csv.reader(f))
    with (ROOT/'docs/recovery/ghidra-functions.csv').open() as f: ghidra = list(csv.DictReader(f))
    pin = manifest.bindings[0].origin.image_sha256
    image = PeImage(data,pin)
    decoder = Cs(CS_ARCH_X86,CS_MODE_32)
    for a,e,name,_,_ in EXPECTED:
        assert any(r[:3] == [f'0x{a:08X}',f'0x{e:08X}',name] for r in rows)
        assert any(r['program_sha256'] == pin and r['address'] == f'0x{a:08X}'
                   and r['end_address'] == f'0x{e:08X}' and int(r['size']) == e-a for r in ghidra)
        ins = list(decoder.disasm(image.read(Span(a,e)),a))
        assert sum(i.size for i in ins) == e-a
        assert ins[-1].mnemonic == 'ret' and ins[-1].address+ins[-1].size == e


@pytest.mark.parametrize('address',sorted(QUALIFIED))
@pytest.mark.parametrize('damage',['none','callee','extent','symbol','member','archive_pin','body'])
def test_bad_or_incomplete_correspondence_never_passes(inputs,address,damage):
    manifest,image,archive,_ = inputs
    selected = replace(manifest,bindings=manifest.bindings[-len(EXPECTED):],data_sections=(),absolute_symbols=())
    subject = next(b for b in selected.bindings if b.origin.address == address)
    if damage == 'callee':
        target = 0x645D1C if address == 0x645DA7 else 0x6491C3
        selected = replace(selected,bindings=tuple(b for b in selected.bindings if b.origin.address != target))
    elif damage in {'extent','symbol','member'}:
        changed = (replace(subject,spans=(Span(address,subject.spans[0].end-1),)) if damage == 'extent' else
                   replace(subject,symbol='_missing_runtime_symbol') if damage == 'symbol' else
                   replace(subject,unit='build/intel/mt_obj/strrchr.obj'))
        selected = replace(selected,bindings=tuple(changed if b == subject else b for b in selected.bindings))
    elif damage == 'archive_pin': selected = replace(selected,archive_sha256='0'*64)
    elif damage == 'body':
        changed = bytearray(image); changed[raw_offset(image,address)] ^= 1
        image = bytes(changed); selected = repin(selected,image)
    report = verify(inputs,selected,image,archive)
    if damage == 'archive_pin':
        assert report['result'] == 'fail' and report['functions'] == [], report
        return
    assert (row(report,address)['result'] == 'pass') == (damage == 'none'), report


def test_task_limits_runtime_source_family_scope():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT/'recovery/tasks/runtime-atox-errno-oct11.json').read_text()))
    assert task.base_revision == BASE
    assert {s.address for s in task.subjects} == {a for a,*_ in EXPECTED}
    assert [p for p in task.editable_files if p.startswith('src/')] == [SOURCE]
