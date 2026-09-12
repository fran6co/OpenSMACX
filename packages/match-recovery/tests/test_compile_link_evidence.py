"""Exact file correspondence controls; synthetic compiler/link observations."""

from copy import deepcopy
import json
from pathlib import Path

import pytest

from match_recovery.compile_link_evidence import verify_compile_link_inputs
from test_compile_file_evidence import evidence as compilation_evidence, stamp
from test_link_file_evidence import evidence as link_evidence
from test_link_log import LOG


@pytest.fixture
def build(tmp_path):
    compile = compilation_evidence(tmp_path)
    link = link_evidence(tmp_path)
    output = compile['output']['path']
    link['explicit_inputs'].append(output)
    link['arguments'].append(output)
    link['files'].append({'path': output, 'before': compile['output']['after'],
                          'after': compile['output']['after']})
    return [compile], link, [Path(compile['source'])]


def verify(build, log=LOG):
    compiles, link, sources = build
    return verify_compile_link_inputs((json.dumps(row).encode() for row in compiles),
                                     json.dumps(link).encode(), log.encode(), sources=sources)


def test_exact_expected_compilations_bind_to_actual_link_input_files(build):
    report = verify(build)
    assert report['result'] == report['source_membership'] == 'pass'
    assert report['missing_sources'] == report['unexpected_sources'] == []
    row, = report['compilations']
    assert row['result'] == row['compilation']['result'] == row['link_correspondence'] == 'pass'
    assert report['other_explicit_inputs'] == build[1]['explicit_inputs'][:1]
    assert report['input_coverage'] == report['linked_selection'] == report['build_graph_verification'] == 'unknown'
    assert not report['persisted'] and not report['ready_for_collection']


def test_absent_compilation_observations_stay_unknown(build):
    build[0].clear()
    report = verify(build)
    assert report['result'] == report['source_membership'] == 'unknown'
    assert report['missing_sources'] == [str(build[2][0])]
    assert report['other_explicit_inputs'] == build[1]['explicit_inputs']


def test_missing_expected_source_is_not_erased_by_supplied_subset(build, tmp_path):
    build[2].append(tmp_path / 'missing.cpp')
    report = verify(build)
    assert report['result'] == 'unknown'
    assert report['missing_sources'] == [str(tmp_path / 'missing.cpp')]


def test_unexpected_source_cannot_be_silently_dropped(build, tmp_path):
    build[2][:] = [tmp_path / 'different.cpp']
    report = verify(build)
    assert report['result'] == report['source_membership'] == 'fail'
    assert report['unexpected_sources'] == [build[0][0]['source']]


@pytest.mark.parametrize('damage', ['source', 'output', 'expected'])
def test_ambiguous_identities_refuse(build, tmp_path, damage):
    if damage == 'expected':
        build[2].append(build[2][0])
    else:
        duplicate = deepcopy(build[0][0])
        if damage == 'source':
            duplicate['output']['path'] = str(tmp_path / 'other.obj')
        else:
            original = duplicate['source']
            duplicate['source'] = str(tmp_path / 'different.cpp')
            next(row for row in duplicate['inputs'] if row['path'] == original)['path'] = duplicate['source']
        build[0].append(duplicate)
    with pytest.raises(ValueError, match='duplicate|ambiguous'):
        verify(build)


@pytest.mark.parametrize('damage', ['not_supplied', 'twice', 'same_name_same_bytes_elsewhere'])
def test_compiler_output_requires_one_exact_link_input_identity(build, tmp_path, damage):
    output = build[0][0]['output']['path']
    if damage == 'twice':
        build[1]['explicit_inputs'].append(output)
    else:
        build[1]['explicit_inputs'].remove(output)
        if damage != 'not_supplied':
            (tmp_path / 'other').mkdir()
            substitute = tmp_path / 'other' / Path(output).name
            substitute.write_bytes(Path(output).read_bytes())
            build[1]['explicit_inputs'].append(str(substitute))
            build[1]['files'].append({'path': str(substitute), 'before': stamp(substitute.read_bytes()),
                                      'after': stamp(substitute.read_bytes())})
    report = verify(build)
    assert report['result'] == report['compilations'][0]['link_correspondence'] == 'fail'


def test_different_compile_and_link_bytes_fail_even_when_current_object_matches_compile(build):
    output = build[0][0]['output']['path']
    item = next(row for row in build[1]['files'] if row['path'] == output)
    item['before'] = item['after'] = stamp(b'other linked contents')
    report = verify(build)
    assert report['compilations'][0]['compilation']['result'] == 'pass'
    assert report['result'] == report['compilations'][0]['link_correspondence'] == 'fail'


@pytest.mark.parametrize('damage', ['compile_failure', 'source_changed', 'tool_changed',
                                   'object_changed', 'library_changed', 'missing_observation'])
def test_aggregate_never_hides_failed_or_incomplete_file_evidence(build, damage):
    compile = build[0][0]
    if damage == 'compile_failure':
        compile['exit_code'] = 2
    elif damage == 'missing_observation':
        compile['output']['after'] = None
    else:
        path = (compile['source'] if damage == 'source_changed' else
                compile['tool'] if damage == 'tool_changed' else
                compile['output']['path'] if damage == 'object_changed' else
                build[1]['library_searches'][0]['path'])
        Path(path).write_bytes(b'changed observed input')
    assert verify(build)['result'] == ('unknown' if damage == 'missing_observation' else 'fail')


def test_link_diagnostics_must_belong_to_the_same_link(build):
    with pytest.raises(ValueError, match='different diagnostics'):
        verify(build, LOG + '\n')


@pytest.mark.parametrize('sources', [[], [Path('relative.cpp')], ['/absolute/../alias.cpp']])
def test_expected_source_identifiers_are_explicit_normalized_absolute_paths(build, sources):
    build[2][:] = sources
    with pytest.raises(ValueError):
        verify(build)


def test_inputs_and_output_rows_are_not_mutated_or_persisted(build):
    before = deepcopy(build)
    first, second = verify(build), verify(build)
    assert first == second and build == before
