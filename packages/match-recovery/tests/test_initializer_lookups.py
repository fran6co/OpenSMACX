"""Native direct initializer roots share guarded lookups, not proof reports."""
from contextlib import contextmanager

import pytest

from match_recovery import linked_initializers as initializers
from match_recovery.linked_context import linked_build, parse_inputs
from match_recovery.linked_map_index import MapIndex
from match_recovery.linked_objects import SelectedObjects
from test_linked_initializers import build
from test_linked_constant_graph import linked_constant, constant_object


def physical_inputs(context):
    return parse_inputs(initializers._artifacts(context))


def calling_roots(root, count=3):
    return build(root, (b'\xe8' + bytes(4) + b'\xc3',) * count,
                 references=('_dep',) * count)


@pytest.mark.parametrize('constant', [False, True])
def test_shared_and_standalone_native_evidence_is_identical(tmp_path, constant):
    context = linked_constant(tmp_path / 'project', value=7) if constant else calling_roots(tmp_path / 'project')
    with linked_build(context) as live:
        values = live.inputs
        # No supplied lookup keeps each ordinary resolver's complete physical
        # checks. This is native verification, not a fabricated PASS report.
        expected = initializers._observe_inputs_guarded(values, max_functions=4096, _constants=None)
        actual = initializers._observe_inputs(values)
        assert expected == actual
        assert actual['result'] == actual['code_dependencies']['result'] == 'pass'


def test_many_actual_roots_use_one_batch_and_one_root_map_index(tmp_path, monkeypatch):
    context = calling_roots(tmp_path / 'project')
    roots, graphs, batches, resolve_calls, indexes = [], [], [], [], []
    original_function = initializers._function
    original_graph = initializers._guarded_code_graph
    original_batch = initializers.constant_lookups
    original_resolve = SelectedObjects.resolve_constants
    original_index = MapIndex.__init__
    def function(*args, **kwargs):
        roots.append((kwargs['_index'], kwargs['_constants']))
        return original_function(*args, **kwargs)
    def graph(*args, **kwargs):
        graphs.append(kwargs['_constants'])
        return original_graph(*args, **kwargs)
    @contextmanager
    def batch(objects):
        batches.append(objects)
        with original_batch(objects) as lookup:
            yield lookup
    def resolve(self, names, **options):
        resolve_calls.append((list(names), options))
        return original_resolve(self, names, **options)
    def index(self, mapping):
        indexes.append(mapping)
        original_index(self, mapping)
    monkeypatch.setattr(initializers, '_function', function)
    monkeypatch.setattr(initializers, '_guarded_code_graph', graph)
    monkeypatch.setattr(initializers, 'constant_lookups', batch)
    monkeypatch.setattr(SelectedObjects, 'resolve_constants', resolve)
    monkeypatch.setattr(MapIndex, '__init__', index)
    result = initializers.observe_build_initializers(context)
    assert result['result'] == result['code_dependencies']['result'] == 'pass', result
    assert len(result['entries']) == len(roots) == 3
    assert len(batches) == len(graphs) == 1
    assert all(index is roots[0][0] and lookup is roots[0][1] for index, lookup in roots)
    assert graphs[0] is roots[0][1]
    assert len(indexes) == 2 and indexes[0] is indexes[1]  # roots + exact graph view
    assert resolve_calls == []  # _dep absent from complete native constant interest
    with pytest.raises(ValueError, match='expired'):
        roots[0][1](['_dep'])


def test_present_constants_still_revalidate_for_root_and_graph(tmp_path, monkeypatch):
    context = linked_constant(tmp_path / 'project', value=7)
    calls = []
    original = SelectedObjects.resolve_constants
    def resolve(self, names, **options):
        calls.append((list(names), options))
        return original(self, names, **options)
    monkeypatch.setattr(SelectedObjects, 'resolve_constants', resolve)
    result = initializers.observe_build_initializers(context)
    assert result['result'] == result['code_dependencies']['result'] == 'pass', result
    assert calls == [(['_offset'], {'optional': True})] * 2
    reference, = result['functions'][0]['references']
    assert reference['constant_binding']['value'] == 7


@pytest.mark.parametrize('damage', ['operand', 'relocated'])
def test_actual_constant_counterexamples_remain_measured_failures(tmp_path, damage):
    context = linked_constant(tmp_path / 'project', value=7,
                              **({'operand': 8} if damage == 'operand' else {'relocated': True}))
    result = initializers.observe_build_initializers(context)
    assert result['result'] == result['functions'][0]['result'] == 'fail'
    assert result['code_dependencies']['result'] == 'fail'


@pytest.mark.parametrize('when', ['root', 'graph'])
@pytest.mark.parametrize('exception', [False, True])
@pytest.mark.parametrize('damage', ['physical', 'interest', 'witness'])
def test_late_initializer_damage_cannot_escape_final_batch_guard(tmp_path, monkeypatch, when, exception, damage):
    context = calling_roots(tmp_path / 'project', 1)
    values = physical_inputs(context)
    method = '_function' if when == 'root' else '_guarded_code_graph'
    original = getattr(initializers, method)
    captured = []
    def changed(*args, **kwargs):
        result = original(*args, **kwargs)
        if not captured:
            captured.append(kwargs['_constants'])
            if damage == 'physical':
                path = context.directory / 'dependency.obj'
                path.write_bytes(path.read_bytes() + b'changed')
            elif damage == 'interest':
                values.objects._constant_interest.add('injected')
            else:
                values.objects._constant_physical.clear()
            if exception:
                raise ValueError('consumer refusal after native verification')
        return result
    monkeypatch.setattr(initializers, method, changed)
    with pytest.raises(ValueError, match='population changed|indices changed'):
        initializers._observe_inputs(values)
    assert len(captured) == 1
    with pytest.raises(ValueError, match='expired'):
        captured[0](['_dep'])


def test_present_constant_mutated_before_direct_root_lookup_refuses(tmp_path, monkeypatch):
    context = linked_constant(tmp_path / 'project', value=7)
    values = physical_inputs(context)
    original = initializers._function
    def changed(*args, **kwargs):
        (context.directory / 'constant.obj').write_bytes(constant_object(8))
        return original(*args, **kwargs)
    monkeypatch.setattr(initializers, '_function', changed)
    with pytest.raises(ValueError, match='population changed'):
        initializers._observe_inputs(values)


def test_public_observer_never_returns_partial_root_pass_after_late_input_damage(tmp_path, monkeypatch):
    context = calling_roots(tmp_path / 'project', 1)
    original = initializers._function
    def changed(*args, **kwargs):
        result = original(*args, **kwargs)
        path = context.directory / 'dependency.obj'
        path.write_bytes(path.read_bytes() + b'late physical change')
        raise ValueError('consumer refuses before dependency graph')
    monkeypatch.setattr(initializers, '_function', changed)
    report = initializers.observe_build_initializers(context)
    assert report['result'] == 'unknown'
    assert 'population changed' in report['reason']
    assert 'functions' not in report and 'entries' not in report


def test_live_context_final_expiry_still_refuses(tmp_path, monkeypatch):
    context = calling_roots(tmp_path / 'project', 1)
    original = initializers._function
    def expired(*args, **kwargs):
        result = original(*args, **kwargs)
        context.alive = False
        return result
    monkeypatch.setattr(initializers, '_function', expired)
    with pytest.raises(ValueError, match='expired'):
        initializers.observe_build_initializers(context)
