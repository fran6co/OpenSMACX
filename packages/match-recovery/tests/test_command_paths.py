"""Lossless syntax comparison controls, never compiler acceptance tests."""

from copy import deepcopy
from dataclasses import replace
from pathlib import Path

import pytest

from match_recovery.command_paths import compare_commands, validate_command_paths


def command(root, *, separator='/', prefix='/', stage='compiler-effective'):
    root = Path(root)
    encode = lambda path: prefix + separator.join(path.parts[1:])
    arguments = ['/I' + encode(root / 'include'), '/Fo' + encode(root / 'build/unit.obj'),
                 '/Fd' + encode(root / 'build'), '-DKEEP=two words', '-MT', 'src/unit.cpp', '-MT']
    description = {'schema_version': 1, 'stage': stage,
                   'codecs': [{'anchor': '/', 'prefix': prefix, 'separator': separator},
                              {'anchor': str(root), 'prefix': '', 'separator': '/'}],
                   'operands': [], 'unsupported': []}
    for index, path, kind in [(0, root / 'include', 'input_directory'),
                              (1, root / 'build/unit.obj', 'output_file'),
                              (2, root / 'build', 'output_directory')]:
        description['operands'].append({'argument': index, 'start': 2 if index == 0 else 3,
                                        'end': len(arguments[index]), 'path': str(path), 'kind': kind, 'codec': 0})
    description['operands'].append({'argument': 5, 'start': 0, 'end': len(arguments[5]),
                                    'path': str(root / 'src/unit.cpp'), 'kind': 'source', 'codec': 1})
    return arguments, root, description


def parsed(value):
    arguments, cwd, description = value
    return validate_command_paths(arguments, cwd, description, stage=description['stage'])


@pytest.mark.parametrize('prefix,separator', [('/', '/'), ('Z:\\', '\\')])
def test_exact_explicit_path_relocation_preserves_every_other_character(prefix, separator):
    old = command('/before', prefix=prefix, separator=separator)
    new = command('/after', prefix=prefix, separator=separator)
    proof = compare_commands(parsed(old), parsed(new), path_pairs=[('/before', '/after')])
    assert proof['result'] == 'pass' and not proof['differences']
    assert proof['input_coverage'] == proof['build_graph_verification'] == 'unknown'
    assert not proof['persisted'] and not proof['ready_for_collection']
    assert compare_commands(parsed(old), parsed(new))['result'] == 'fail'


@pytest.mark.parametrize('damage', ['literal', 'prefix_case', 'argument_order', 'duplicate_option',
                                   'source_suffix', 'kind', 'codec', 'stage', 'cwd', 'unannotated'])
def test_no_option_token_or_undeclared_path_semantics_can_be_discarded(damage):
    old, new = command('/before'), command('/after')
    arguments, cwd, description = new
    if damage == 'literal': arguments[3] = '-DKEEP=different words'
    elif damage == 'prefix_case': arguments[0] = '/i' + arguments[0][2:]
    elif damage == 'argument_order': arguments[3], arguments[4] = arguments[4], arguments[3]
    elif damage == 'duplicate_option': arguments.pop()
    elif damage == 'source_suffix':
        arguments[5] = 'src/unit.cxx'
        description['operands'][-1].update(path='/after/src/unit.cxx', end=len(arguments[5]))
    elif damage == 'kind': description['operands'][2]['kind'] = 'output_file'
    elif damage == 'codec': new = command('/after', prefix='Z:\\', separator='\\')
    elif damage == 'stage': description['stage'] = 'link-wrapper'
    elif damage == 'cwd': new = (arguments, Path('/different'), description)
    else: description['operands'].pop(0)
    assert compare_commands(parsed(old), parsed(new), path_pairs=[('/before', '/after')])['result'] == 'fail'


def test_nested_build_anchor_preserves_generated_relative_output_identity():
    old, new = command('/before'), command('/after')
    for value, build in [(old, '/before/build/a'), (new, '/after/build/b')]:
        arguments, _, description = value
        for index, suffix in [(1, '/unit.obj'), (2, '')]:
            arguments[index] = arguments[index][:3] + build + suffix
            description['operands'][index].update(path=build + suffix, end=len(arguments[index]))
    assert compare_commands(parsed(old), parsed(new), path_pairs=[('/before', '/after')])['result'] == 'fail'
    assert compare_commands(parsed(old), parsed(new), path_pairs=[('/before', '/after'),
                            ('/before/build/a', '/after/build/b')])['result'] == 'pass'


@pytest.mark.parametrize('damage', ['schema', 'bool_version', 'stage', 'extra_field', 'missing_field',
                                   'unsupported', 'bool_index', 'negative_index', 'out_of_range',
                                   'empty_slice', 'wrong_slice', 'wrong_encoding', 'overlap', 'wrong_kind',
                                   'relative_path', 'dotdot', 'nul', 'encoding_anchor', 'encoding_prefix',
                                   'encoding_separator', 'nonarray', 'empty_arguments'])
def test_malformed_or_partial_descriptions_cannot_support_comparison(damage):
    arguments, cwd, description = command('/project')
    operand = description['operands'][0]
    if damage == 'schema': description['schema_version'] = 9
    elif damage == 'bool_version': description['schema_version'] = True
    elif damage == 'stage': description['stage'] = 'other-stage'
    elif damage == 'extra_field': description['trust'] = True
    elif damage == 'missing_field': del description['operands']
    elif damage == 'unsupported': description['unsupported'] = ['unexpanded response file']
    elif damage == 'bool_index': operand['argument'] = False
    elif damage == 'negative_index': operand['start'] = -1
    elif damage == 'out_of_range': operand['end'] = 999
    elif damage == 'empty_slice': operand['end'] = operand['start']
    elif damage == 'wrong_slice': operand['start'] += 1
    elif damage == 'wrong_encoding': operand['codec'] = 1
    elif damage == 'overlap': description['operands'].append(dict(operand))
    elif damage == 'wrong_kind': operand['kind'] = 'drop_argument'
    elif damage == 'relative_path': operand['path'] = 'include'
    elif damage == 'dotdot': operand['path'] = '/project/../include'
    elif damage == 'nul': arguments[3] += '\0'
    elif damage == 'encoding_anchor': description['codecs'][0]['anchor'] = '/unrelated'
    elif damage == 'encoding_prefix': description['codecs'][0]['prefix'] = 'not-a-root'
    elif damage == 'encoding_separator': description['codecs'][0]['separator'] = ':'
    elif damage == 'nonarray': description['operands'] = {}
    else: arguments.clear()
    with pytest.raises(ValueError):
        validate_command_paths(arguments, cwd, description, stage='compiler-effective')


@pytest.mark.parametrize('pairs', [[('/a', '/b'), ('/a', '/c')], [('/a', '/b'), ('/c', '/b')],
                                  [('relative', '/b')], [('/a', '/b/../c')], [('/a',)], ['not-a-pair']])
def test_ambiguous_or_unbounded_correspondence_spelling_refuses(pairs):
    value = parsed(command('/a'))
    with pytest.raises(ValueError):
        compare_commands(value, value, path_pairs=pairs)


def test_literal_text_cannot_collide_with_typed_path_identity():
    old = command('/before')
    new = command('/before')
    new[0][0] = repr(('path', 'input_directory', ('paired_path', 0, 'include')))
    new[2]['operands'].pop(0)
    assert compare_commands(parsed(old), parsed(new))['result'] == 'fail'


def test_pure_read_does_not_mutate_description_or_use_disk_state():
    value = command('/deliberately/nonexistent/project')
    copy = deepcopy(value)
    result = parsed(value)
    value[2]['operands'][0]['path'] = '/changed'
    assert compare_commands(result, parsed(copy))['result'] == 'pass'
    assert result.operands[0].path == Path('/deliberately/nonexistent/project/include')


@pytest.mark.parametrize('damage', ['literal_replaced', 'outside_argv', 'overlap', 'negative_offset',
                                   'wrong_path', 'wrong_encoding', 'wrong_kind', 'bad_operands'])
def test_public_value_construction_cannot_bypass_slice_validation(damage):
    original = parsed(command('/project'))
    first = original.operands[0]
    changed = original
    if damage == 'literal_replaced':
        arguments = list(original.arguments)
        arguments[0] = '/I' + 'X' * (len(arguments[0]) - 2)
        changed = replace(original, arguments=tuple(arguments))
    elif damage == 'overlap': changed = replace(original, operands=(first, *original.operands))
    elif damage == 'bad_operands': changed = replace(original, operands=('not an operand',))
    else:
        first = (replace(first, argument=900) if damage == 'outside_argv' else
                 replace(first, start=-1) if damage == 'negative_offset' else
                 replace(first, path=Path('/different')) if damage == 'wrong_path' else
                 replace(first, encoding=replace(first.encoding, prefix='Z:\\', separator='\\'))
                 if damage == 'wrong_encoding' else replace(first, kind='discard'))
        changed = replace(original, operands=(first, *original.operands[1:]))
    with pytest.raises(ValueError):
        compare_commands(original, changed)
    with pytest.raises(ValueError):
        compare_commands(changed, original)
