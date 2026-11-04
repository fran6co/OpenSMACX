"""Source-owned Txt lifetime identities; not linked selection or game proof."""
from dataclasses import replace
import hashlib
import json
import os
from pathlib import Path

import pytest

from match_recovery.source import Mode, read_text
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.model import Span

ROOT = Path(__file__).resolve().parents[2]
BASE = '0a4561ea47200ae85a3d3319d6c09db7a45a2297'
PIN = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
BINDINGS = {
    0x5FD400: ((0x5FD400, 0x5FD456), False),
    0x5FD460: ((0x5FD460, 0x5FD4B5), False),
    0x5FD4C0: ((0x5FD4C0, 0x5FD4CB), True),
    0x5FD4D0: ((0x5FD4D0, 0x5FD4DB), True),
}


def test_inline_destructor_retains_original_identity_and_body_binding():
    records = read_text((ROOT/'src/stringlist.cpp').read_text(), ROOT/'src/stringlist.cpp')
    record, = (row for row in records if row.address == 0x608C00)
    assert record.image_spans == ((0x608C00, 0x608C05),)
    assert (record.symbol or record.name) == '??1Text@@QAE@XZ'
    assert record.byte_exact and not record.extract_error
    assert record.body and (ROOT/record.body).is_file()
    assert record.context and (ROOT/record.context).is_file()


def test_original_text_crt_entries_bind_to_the_real_global_initializers():
    source = (ROOT / 'src/text.cpp').read_text()
    header = (ROOT / 'src/text.h').read_text()
    rows = {row.address: row for row in read_text(source, ROOT / 'src/text.cpp')}
    for address, (span, byte_exact) in BINDINGS.items():
        row = rows[address]
        assert row.mode is Mode.FILE
        assert row.image_spans == (span,)
        assert row.byte_exact is byte_exact
        assert not row.extract_error

    object_at = source.index('Text Txt(512);')
    get_at = source.index('LPSTR TextBufferGetPtr = Txt.get_buffer_get();')
    item_at = source.index('LPSTR TextBufferItemPtr = Txt.get_buffer_item();')
    assert object_at < get_at < item_at
    assert source.count('LPSTR TextBufferGetPtr = Txt.get_buffer_get();') == 1
    assert source.count('LPSTR TextBufferItemPtr = Txt.get_buffer_item();') == 1
    # The generated CRT callbacks own these original identities. The globals
    # now initialize through the public Text accessors, without duplicate
    # callable setter bodies or their private friend shims.
    assert 'void __cdecl text_set_get_ptr() {' not in source
    assert 'void __cdecl text_set_item_ptr() {' not in source
    assert 'text_set_get_ptr_source' not in source + header
    assert 'text_set_item_ptr_source' not in source + header


def test_original_crt_root_and_paired_cleanup():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    path = Path(os.environ.get(
        'OPENSMACX_IMAGE', str(ROOT/'.opensmacx/game/terranx_original.exe')))
    if not path.is_file():
        pytest.skip('pinned image unavailable')
    image = PeImage(path.read_bytes(), PIN)
    assert int.from_bytes(image.read(Span(0x682680, 0x682684)), 'little') == 0x5FD400
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    for address, end, calls in [(0x5FD400, 0x5FD456, ['0x5d4510', '0x5d4510', '0x645398']),
                                (0x5FD460, 0x5FD4B5, ['0x645598', '0x644ef2', '0x644ef2'])]:
        instructions = list(decoder.disasm(image.read(Span(address, end)), address))
        assert sum(i.size for i in instructions) == end-address
        assert instructions[-1].mnemonic == 'ret'
        assert [i.op_str for i in instructions if i.mnemonic == 'call'] == calls
        if address == 0x5FD400:
            assert any(i.mnemonic == 'push' and i.op_str == '0x5fd460' for i in instructions)
            assert sum(i.mnemonic == 'push' and i.op_str == '0x200' for i in instructions) == 2


def lifetime_graph(obj):
    """Observe both inline and split forms; never equate constructor-only code."""
    def refs(symbol):
        obj.function(symbol.name)  # exclusive physical function, not a label
        return [obj.symbols[i] for _, i, _ in obj.sections[symbol.section-1].relocations]

    def walk(symbol, active=()):
        assert symbol.name not in active, 'cyclic generated lifetime graph'
        result = [(symbol, refs(symbol))]
        for child in result[0][1]:
            if child.section > 0 and child.type & 0x20 and child.storage == 3 and child.name.startswith('_$E'):
                result.extend(walk(child, (*active, symbol.name)))
        return result

    roots = [obj.symbols[i] for sec in obj.sections if sec.name == '.CRT$XCU'
             for _, i, kind in sec.relocations if kind == 6]
    matches = []
    for root in roots:
        nodes = walk(root)
        constructors = [s for s, targets in nodes
                        if [t.name for t in targets].count('?mem_get@@YAPAXH@Z') == 2
                        and any(t.name == '?Txt@@3VText@@A' for t in targets)]
        registrations = [(s, targets) for s, targets in nodes
                         if any(t.name == '_atexit' for t in targets)]
        if len(constructors) != 1 or len(registrations) != 1:
            continue
        _, targets = registrations[0]
        destructors = []
        for t in targets:
            if t.section > 0 and t.type & 0x20 and t.storage == 3:
                names = [r.name for r in refs(t)]
                if '?Txt@@3VText@@A' in names and (
                        '?shutdown@Text@@QAEXXZ' in names or
                        names.count('_free') == 2 and '_fclose' in names):
                    destructors.append(t)
        if len(destructors) == 1:
            matches.append((root, destructors[0], constructors[0]))
    assert len(matches) == 1, 'missing or ambiguous Txt lifetime root'
    root, destructor, constructor = matches[0]
    assert roots.count(root) == 1
    return root, destructor, constructor


@pytest.fixture
def fresh_object():
    value = os.environ.get('OPENSMACX_TEXT_LIFETIME_OBJECT')
    if not value:
        pytest.skip('explicit fresh compiler-backed Text object required')
    path = Path(value)
    receipt = json.loads(Path(str(path)+'.compile.json').read_text())
    assert Path(receipt['source']).resolve() == ROOT/'src/text.cpp'
    inputs = {row['path']: row for row in receipt['inputs']}
    for source in ('src/text.cpp', 'src/text.h'):
        row = inputs[str(ROOT/source)]
        assert row['before'] == row['after']
        assert row['after']['sha256'] == hashlib.sha256((ROOT/source).read_bytes()).hexdigest()
    return Coff(path.read_bytes()), path.read_bytes()


@pytest.mark.parametrize('damage', ['none', 'missing_crt', 'missing_allocation', 'missing_atexit', 'missing_shutdown'])
def test_fresh_physical_graph_and_damage_refusals(fresh_object, damage):
    obj, _raw = fresh_object
    lifetime_graph(obj)
    if damage == 'missing_crt':
        obj.sections = tuple(replace(s, relocations=()) if s.name == '.CRT$XCU' else s for s in obj.sections)
    elif damage != 'none':
        removed = {'missing_allocation': {'?mem_get@@YAPAXH@Z'},
                   'missing_atexit': {'_atexit'},
                   'missing_shutdown': {'_free', '_fclose', '?shutdown@Text@@QAEXXZ'}}[damage]
        obj.sections = tuple(replace(s, relocations=tuple(r for r in s.relocations
                                    if obj.symbols[r[1]].name not in removed)) for s in obj.sections)
    if damage != 'none':
        with pytest.raises(AssertionError, match='missing or ambiguous'):
            lifetime_graph(obj)
        return


def test_native_task_preserves_existing_subject_scope():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT/'recovery/tasks/text-generated-lifetime-oct11.json').read_text()))
    assert task.base_revision == BASE and {s.address for s in task.subjects} == {0x5FD400, 0x5FD460}
    assert {p for p in task.editable_files if p.startswith('src/')} == {'src/text.cpp', 'src/text.h'}


@pytest.fixture
def fresh_destructor_object():
    value = os.environ.get('OPENSMACX_TEXT_DESTRUCTOR_OBJECT')
    if not value:
        pytest.skip('explicit fresh natural-destructor object required')
    record, = (r for r in read_text((ROOT/'src/stringlist.cpp').read_text(), ROOT/'src/stringlist.cpp')
               if r.address == 0x608C00)
    path = Path(value)
    receipt = json.loads(Path(str(path)+'.compile.json').read_text())
    assert Path(receipt['source']).resolve() == ROOT/record.context
    inputs = {row['path']: row for row in receipt['inputs']}
    for source in (record.context, record.body):
        row = inputs[str(ROOT/source)]
        assert row['before'] == row['after']
        assert row['after']['sha256'] == hashlib.sha256((ROOT/source).read_bytes()).hexdigest()
    return record, Coff(path.read_bytes())


def test_natural_destructor_matches_owned_shutdown_target(fresh_destructor_object):
    from match_recovery.bound_objects import FunctionBinding, check_objects
    from match_recovery.model import Origin
    record, obj = fresh_destructor_object
    image_path = Path(os.environ.get('OPENSMACX_IMAGE', ROOT/'.opensmacx/game/terranx_original.exe'))
    raw_image = image_path.read_bytes()
    assert hashlib.sha256(raw_image).hexdigest() == PIN
    image = PeImage(raw_image, PIN)
    symbol = record.symbol or record.name
    binding = FunctionBinding(Origin(PIN, 'pe:va', record.address),
                              tuple(Span(a,b) for a,b in record.image_spans), symbol, record.context)
    targets = {'?shutdown@Text@@QAEXXZ': 0x5FD970}
    row, = check_objects([binding], {record.context: obj}, image, external_targets=targets)
    assert row['boundary'] == row['bytes'] == row['references'] == 'pass', row
    damaged, = check_objects([binding], {record.context: obj}, image,
                             external_targets={'?shutdown@Text@@QAEXXZ': 0x5FD971})
    assert damaged['references'] == 'fail', damaged
    directives = b''.join(s.data for s in obj.sections if s.name == '.drectve')
    assert symbol.encode() not in directives
