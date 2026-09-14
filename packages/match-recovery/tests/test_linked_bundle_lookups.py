"""Physical bundle lookup reuse: acceleration cannot become evidence."""
from dataclasses import replace
from hashlib import sha256
import struct

import pytest

from match_recovery.linked_bundle import verify_bundle
from match_recovery.linked_code_region import verify_code_region
from match_recovery.linked_code_graph import observe_code_graph
from match_recovery.linked_constant_batch import constant_lookups
from match_recovery.linked_map_index import MapIndex
from match_recovery.linked_objects import SelectedObjects
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.pdb_fixups import bound_fixups
from match_recovery.msvc_link_map import parse_link_map
from test_multispan import object_fixture
from test_pdb_fixups import parts
from test_msf import small_msf
from test_link_input_coverage import inputs
from test_link_file_evidence import stamp
from test_linked_objects import inventory
from test_linked_constant_graph import constant_object


def fixture(root, *, constant=False, value=7, operand=None, map_constant=False):
    root.mkdir()
    evidence, log = inputs(root)
    child = (b'\x68' if constant else b'\xe8') + bytes(4) + b'\xc3'
    raw = bytearray(object_fixture(tail=child))
    symbols, count = struct.unpack_from('<II', raw, 8)
    # Add the actual child relocation before its symbol table; the root's
    # independent REL32-to-child relocation is preserved unchanged.
    relocation = struct.pack('<IIH', 1, count if constant else 2, 6 if constant else 20)
    raw[symbols:symbols] = relocation
    symbols += len(relocation)
    struct.pack_into('<I', raw, 8, symbols)
    struct.pack_into('<I', raw, 60 + 24, symbols - len(relocation))
    struct.pack_into('<H', raw, 60 + 32, 1)
    struct.pack_into('<H', raw, symbols + 4 * 18 + 4, 1)
    if constant:
        raw[symbols + count * 18:symbols + count * 18] = (
            b'_offset\0' + struct.pack('<IhHBB', 0, 0, 0, 2, 0))
        struct.pack_into('<I', raw, 12, count + 1)
        path = root / 'constant.obj'
        path.write_bytes(constant_object(value))
        evidence['files'].append({'path': str(path), 'before': stamp(path.read_bytes()),
                                  'after': stamp(path.read_bytes())})
        evidence['arguments'].append(str(path))
        evidence['explicit_inputs'].append(str(path))
        log = log.replace('End Pass2', 'constant.obj\nEnd Pass2')
    (root / 'app.obj').write_bytes(raw)
    row = next(item for item in evidence['files'] if item['path'] == str(root / 'app.obj'))
    row['before'] = row['after'] = stamp(raw)
    evidence['link_log_sha256'] = sha256(log.encode()).hexdigest()
    objects = SelectedObjects(*inventory(evidence, log))
    fixup_rows = ((20, 0, 0x1001, 0x1020),) + (() if constant else ((20, 0, 0x1021, 0x1000),))
    image_raw, streams = parts(fixup_rows)
    image_raw = bytearray(image_raw)
    struct.pack_into('<I', image_raw, 0x98 + 16, 0x1000)
    struct.pack_into('<I', image_raw, 0x178 + 36, 0x60000020)
    streams[5] = bytes(image_raw[0x178:0x1A0])
    image_raw[0x200:0x210] = (b'\xe8' + struct.pack('<i', 0x1B) + b'\xc3').ljust(16, b'\xcc')
    effective = value if constant else -0x25
    effective = effective if operand is None else operand
    image_raw[0x220:0x230] = (child[:1] + struct.pack('<I', effective & 0xFFFFFFFF) + b'\xc3').ljust(16, b'\xcc')
    image = PeImage(bytes(image_raw), sha256(image_raw).hexdigest())
    fixups = bound_fixups(image, small_msf(streams))
    mapping = ('synthetic\n Preferred load address is 00400000\n'
               ' Address         Publics by Value              Rva+Base     Lib:Object\n'
               ' 0001:00000000 _f 00401000 f app.obj\n'
               + (' 0001:00000080 _offset 00401080 constant.obj\n' if map_constant else '')
               + ' entry point at 0001:00000000\n')
    return Coff(bytes(raw)), objects, image, fixups, parse_link_map(mapping.encode())


def verify(values, *, index=None, constants=None, direct=False):
    obj, objects, image, fixups, mapping = values
    method = verify_bundle if direct else verify_code_region
    return method(obj, obj.function('_f'), 'app.obj', mapping, image, objects=objects,
                  fixups=fixups, _index=index, _constants=constants)


@pytest.mark.parametrize('constant', [False, True])
@pytest.mark.parametrize('direct', [False, True])
def test_native_bundle_with_supplied_lookups_is_identical(tmp_path, constant, direct):
    values = fixture(tmp_path / 'project', constant=constant)
    expected = verify(values, direct=direct)
    assert expected['result'] == 'pass'
    with constant_lookups(values[1]) as lookup:
        actual = verify(values, index=MapIndex(values[4]), constants=lookup, direct=direct)
    assert actual == expected
    assert actual['extent_kind'] == 'associated_coff_regions'
    assert actual['target_contents'] == 'unknown'


@pytest.mark.parametrize('direct', [False, True])
def test_wrong_physical_or_rewritten_map_view_refuses_bundle(tmp_path, direct):
    values = fixture(tmp_path / 'project')
    foreign = replace(values[4])
    with pytest.raises(ValueError, match='another physical or owner-rewritten view'):
        verify(values, index=MapIndex(foreign), direct=direct)


def test_graph_bundle_uses_shared_optional_absence_and_one_index(tmp_path, monkeypatch):
    values = fixture(tmp_path / 'project')
    obj, objects, image, fixups, mapping = values
    resolutions, constructions = [], []
    original_resolve = SelectedObjects.resolve_constants
    original_init = MapIndex.__init__
    def resolve(self, names, **options):
        resolutions.append((list(names), options))
        return original_resolve(self, names, **options)
    def construct(self, view):
        constructions.append(view)
        original_init(self, view)
    monkeypatch.setattr(SelectedObjects, 'resolve_constants', resolve)
    monkeypatch.setattr(MapIndex, '__init__', construct)
    graph = observe_code_graph(objects, mapping, image, [{'owner': 'app.obj', 'symbol': '_f'}], fixups=fixups)
    assert graph['result'] == 'pass', graph
    assert resolutions == []  # _f is absent from the complete constant interest set.
    assert constructions == [mapping]


def test_present_constant_keeps_ordinary_resolver_and_exact_proof(tmp_path, monkeypatch):
    values = fixture(tmp_path / 'project', constant=True)
    calls = []
    original = SelectedObjects.resolve_constants
    def resolve(self, names, **options):
        calls.append((list(names), options))
        return original(self, names, **options)
    monkeypatch.setattr(SelectedObjects, 'resolve_constants', resolve)
    with constant_lookups(values[1]) as lookup:
        proof = verify(values, constants=lookup, index=MapIndex(values[4]))
    assert calls == [(['_offset'], {'optional': True})]
    reference, = proof['regions'][1]['references']
    assert reference['identity'] == {'absolute_constant': {'symbol': '_offset', 'value': 7}}
    assert reference['constant_binding']['input'] == {'path': str(tmp_path / 'project' / 'constant.obj')}


def test_supplied_lookup_never_hides_corrupt_constant_operand(tmp_path):
    values = fixture(tmp_path / 'project', constant=True, operand=8)
    with constant_lookups(values[1]) as lookup:
        result = verify(values, constants=lookup)
    assert result['result'] == result['regions'][1]['code']['result'] == 'fail'


def test_supplied_lookup_preserves_absolute_constant_map_conflict(tmp_path):
    values = fixture(tmp_path / 'project', constant=True, map_constant=True)
    with constant_lookups(values[1]) as lookup:
        with pytest.raises(ValueError, match='constant conflicts with a linked map identity'):
            verify(values, constants=lookup, index=MapIndex(values[4]))


@pytest.mark.parametrize('constant', [False, True])
@pytest.mark.parametrize('exception', [False, True])
def test_bundle_batch_final_guard_and_expiry_survive_late_physical_damage(tmp_path, constant, exception):
    values = fixture(tmp_path / 'project', constant=constant)
    with pytest.raises(ValueError, match='population changed'):
        with constant_lookups(values[1]) as lookup:
            assert verify(values, constants=lookup)['result'] == 'pass'
            path = tmp_path / 'project' / ('constant.obj' if constant else 'runtime.lib')
            path.write_bytes(path.read_bytes() + b'late damage')
            if exception:
                raise ValueError('consumer refused after observation')
    with pytest.raises(ValueError, match='expired'):
        lookup(['_offset'])


def test_present_constant_mutation_before_lookup_refuses(tmp_path):
    values = fixture(tmp_path / 'project', constant=True)
    with pytest.raises(ValueError):
        with constant_lookups(values[1]) as lookup:
            path = tmp_path / 'project' / 'constant.obj'
            path.write_bytes(constant_object(8))
            verify(values, constants=lookup)


def test_private_outward_fixup_binding_is_identical_with_shared_view(tmp_path):
    from test_linked_fixups import fixture as private_fixture
    values = private_fixture(tmp_path)
    expected = verify(values)
    assert expected['result'] == 'pass'
    with constant_lookups(values[1]) as lookup:
        actual = verify(values, index=MapIndex(values[4]), constants=lookup)
    assert actual == expected
    assert any('private_binding' in ref for region in actual['regions'] for ref in region['references'])


@pytest.mark.parametrize('damage', ['other_owner', 'public_namespace', 'contradiction', 'duplicate'])
def test_private_name_conflicts_remain_refusals_with_shared_view(tmp_path, damage):
    from test_linked_fixups import fixture as private_fixture
    values = private_fixture(tmp_path, map_row=' 0002:00000100 private 00402100 other.obj')
    obj, objects, image, fixups, mapping = values
    private, = mapping.statics
    if damage == 'public_namespace':
        mapping = replace(mapping, publics=(*mapping.publics, private), statics=())
    elif damage == 'contradiction':
        mapping = replace(mapping, statics=(replace(private, owner='app.obj', address=0x402104),))
    elif damage == 'duplicate':
        same = replace(private, owner='app.obj')
        mapping = replace(mapping, statics=(same, same))
    values = obj, objects, image, fixups, mapping
    with constant_lookups(objects) as lookup:
        with pytest.raises(ValueError, match='conflicts with existing linked map|consistent independent map identity'):
            verify(values, index=MapIndex(mapping), constants=lookup)


@pytest.mark.parametrize('damage', [None, 'missing', 'duplicate', 'address'])
def test_associated_data_outward_callback_uses_duplicate_preserving_index(tmp_path, damage):
    from test_linked_bundle_controls import physical_family
    objects, mapping, image, fixups = physical_family(tmp_path, callback=True)
    obj = Coff(objects.resolve('app.obj')['raw'])
    callback, = [row for row in mapping.publics if row.symbol == '_cb']
    if damage == 'missing':
        mapping = replace(mapping, publics=tuple(row for row in mapping.publics if row.symbol != '_cb'))
    elif damage == 'duplicate':
        mapping = replace(mapping, publics=(*mapping.publics, callback))
    elif damage == 'address':
        mapping = replace(mapping, publics=tuple(replace(row, address=row.address + 4)
                          if row.symbol == '_cb' else row for row in mapping.publics))
    values = obj, objects, image, fixups, mapping
    with constant_lookups(objects) as lookup:
        if damage:
            with pytest.raises(ValueError, match='consistent independent map identity'):
                verify(values, index=MapIndex(mapping), constants=lookup)
        else:
            assert verify(values, index=MapIndex(mapping), constants=lookup) == verify(values)
