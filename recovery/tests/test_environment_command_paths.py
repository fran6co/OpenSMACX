"""Project grammar describes operands without normalizing or judging commands."""

from copy import deepcopy
import json
import os
from pathlib import Path, PurePosixPath

import pytest

from recovery.environment.command_paths import describe_command


def test_link_metadata_declares_escaped_locator_without_guessing_operand_paths():
    from match_recovery.command_paths import PathEncoding, validate_command_paths
    from pathlib import Path

    description = describe_command(['/pdb:sample.pdb'], '/project/build', stage='link-wrapper')
    validate_command_paths(['/pdb:sample.pdb'], Path('/project/build'), description, stage='link-wrapper')
    encoded = {PathEncoding(Path(row['anchor']), row['prefix'], row['separator']).encode(
        Path('/project/build/sample.pdb')) for row in description['codecs']}
    assert 'Z:\\\\project\\\\build\\\\sample.pdb' in encoded
    malformed = describe_command(['/pdb:Z:\\\\project\\\\build\\\\sample.pdb'],
                                 '/project/build', stage='link-wrapper')
    assert malformed['unsupported']
from match_recovery.command_paths import compare_commands, validate_command_paths


def validate_syntax(arguments, cwd, stage):
    before = deepcopy(arguments)
    description = describe_command(arguments, cwd, stage=stage)
    assert arguments == before
    assert set(description) == {'schema_version', 'stage', 'codecs', 'operands', 'unsupported'}
    assert description['schema_version'] == 1 and description['stage'] == stage
    spans = {}
    for item in description['operands']:
        assert set(item) == {'argument', 'start', 'end', 'path', 'kind', 'codec'}
        codec = description['codecs'][item['codec']]
        relative = PurePosixPath(item['path']).relative_to(codec['anchor'])
        encoded = codec['prefix'] + codec['separator'].join(relative.parts)
        assert arguments[item['argument']][item['start']:item['end']] == encoded
        assert 0 <= item['start'] < item['end'] <= len(arguments[item['argument']])
        seen = spans.setdefault(item['argument'], set())
        positions = set(range(item['start'], item['end']))
        assert not seen & positions
        seen.update(positions)
    if not description['unsupported']:
        validate_command_paths(arguments, cwd, description, stage=stage)
    return description


def test_actual_application_compile_operand_shapes_are_lossless_without_filesystem_guesses():
    arguments = ['/nologo', '/TP', '/IZ:\\project\\cmake\\vc6-include', '/IZ:\\project\\src',
                 '/IZ:\\project\\src\\vendor\\zlib-1.0.2', '/IZ:\\sdk\\INCLUDE',
                 '/DWIN32', '/D_WINDOWS', '/Zm200', '/GX', '/O2', '/Ob2', '/DNDEBUG', '-MT',
                 '/FIZ:\\project\\src\\vc6_compat.h', '/W4',
                 '/FoZ:\\project\\build\\CMakeFiles\\OpenSMACX.dir\\src\\time.cpp.obj',
                 '/FdZ:\\project\\build\\CMakeFiles\\OpenSMACX.dir', '-c', 'src/time.cpp', '/Brepro']
    result = validate_syntax(arguments, '/project', 'compiler-effective')
    assert result['unsupported'] == []
    assert [(row['argument'], row['kind'], row['path']) for row in result['operands']] == [
        (2, 'input_directory', '/project/cmake/vc6-include'),
        (3, 'input_directory', '/project/src'),
        (4, 'input_directory', '/project/src/vendor/zlib-1.0.2'),
        (5, 'input_directory', '/sdk/INCLUDE'),
        (14, 'input_file', '/project/src/vc6_compat.h'),
        (16, 'output_file', '/project/build/CMakeFiles/OpenSMACX.dir/src/time.cpp.obj'),
        (17, 'output_directory', '/project/build/CMakeFiles/OpenSMACX.dir'),
        (19, 'source', '/project/src/time.cpp')]


@pytest.mark.parametrize('arguments', [
    ['/FdZ:\\project\\build\\CMakeFiles\\jackal.dir\\jackal.pdb'],
    ['/Fd', 'build/CMakeFiles/jackal.dir/jackal.pdb'],
    ['-Fdbuild/CMakeFiles/jackal.dir/jackal.PDB'],
])
def test_explicit_compile_pdb_filename_is_not_a_directory(arguments):
    result = validate_syntax(arguments, '/project', 'compiler-effective')
    assert result['unsupported'] == []
    assert len(result['operands']) == 1
    assert result['operands'][0]['kind'] == 'output_file'
    assert result['operands'][0]['path'].lower() == '/project/build/cmakefiles/jackal.dir/jackal.pdb'


def test_unknown_compile_pdb_extension_refuses_instead_of_guessing():
    result = describe_command(['/Fdbuild/jackal.unknown'], '/project', stage='compiler-effective')
    assert result['unsupported']
    assert result['operands'] == []


def test_host_stage_link_paths_preserve_prefix_case_and_library_multiplicity():
    arguments = ['/nologo', 'CMakeFiles/app.dir/src/time.cpp.obj', '/out:app.exe',
                 '/implib:app.lib', '/pdb:app.pdb', '/version:0.0', '/machine:X86',
                 '/INCREMENTAL:NO', '/subsystem:windows', '/MAP:/project/build/app.map',
                 '/MAPINFO:LINES', '/FIXED:NO', '/VERBOSE', 'zlib.lib', 'user32.lib',
                 'gdi32.lib', 'user32.lib', 'gdi32.lib']
    result = validate_syntax(arguments, '/project/build', 'link-wrapper')
    assert result['unsupported'] == []
    assert [(row['argument'], row['start'], row['kind']) for row in result['operands']] == [
        (1, 0, 'input_file'), (2, 5, 'output_file'), (3, 8, 'output_file'),
        (4, 5, 'output_file'), (9, 5, 'output_file')]
    assert result['operands'][3]['path'] == '/project/build/app.pdb'  # No file existence claim.
    assert all(row['argument'] < 13 for row in result['operands'])


def test_separated_path_options_and_macro_literals_remain_exact():
    arguments = ['/I', '/project/include', '-FI', 'src/forced.h', '/Fo', '/project/build/unit.obj',
                 '/DROOT=/project/old', '/D', 'OTHER=Z:\\project\\old',
                 '/U', 'macro/name', '-c', 'src/unit.cpp']
    result = validate_syntax(arguments, '/project', 'compiler-effective')
    assert result['unsupported'] == []
    assert [row['argument'] for row in result['operands']] == [1, 3, 5, 12]


def test_explicit_library_paths_and_search_directories_are_not_bare_requests():
    arguments = ['/LIBPATH:Z:\\sdk\\LIB', '/DEF:/project/app.def', '/project/build/runtime.lib',
                 'generated/app.exp', 'assets/app.res', 'runtime.lib', '/DEFAULTLIB:runtime.lib',
                 '/NODEFAULTLIB:other.lib']
    result = validate_syntax(arguments, '/project', 'link-wrapper')
    assert result['unsupported'] == []
    assert [row['argument'] for row in result['operands']] == [0, 1, 2, 3, 4]
    assert result['operands'][0]['kind'] == 'input_directory'


@pytest.mark.parametrize('spelling,path', [('/I/', '/'), ('/IZ:\\', '/'),
                                        ('/I/project with spaces/include', '/project with spaces/include'),
                                        ('/Irelative with spaces/include', '/project/relative with spaces/include')])
def test_exact_codecs_support_root_and_spaces(spelling, path):
    result = validate_syntax([spelling], '/project', 'compiler-effective')
    assert result['unsupported'] == [] and result['operands'][0]['path'] == path


@pytest.mark.parametrize('arguments,stage', [
    (['@build/compile.rsp'], 'compiler-effective'),
    (['/IC:\\sdk\\INCLUDE'], 'compiler-effective'),
    (['/Iz:\\sdk\\INCLUDE'], 'compiler-effective'),
    (['/IZ:/sdk/INCLUDE'], 'compiler-effective'),
    (['/IZ:\\sdk/INCLUDE'], 'compiler-effective'),
    (['src\\unit.cpp'], 'compiler-effective'),
    (['/I../include'], 'compiler-effective'),
    (['/I/project//include'], 'compiler-effective'),
    (['/I"/project/include"'], 'compiler-effective'),
    (['/Fpbuild/precompiled.pch'], 'compiler-effective'),
    (['/Yustdafx.h'], 'compiler-effective'),
    (['/OTHERPATH:/project/input'], 'compiler-effective'),
    (['/I'], 'compiler-effective'),
    (['/D'], 'compiler-effective'),
    (['C:\\source\\unit.cpp'], 'compiler-effective'),
    (['/ORDER:@build/order.txt'], 'link-wrapper'),
    (['/out:'], 'link-wrapper'),
    (['C:runtime.lib'], 'link-wrapper'),
    (['unknown-bare-input'], 'link-wrapper'),
])
def test_unsupported_encodings_and_path_grammars_are_explicit(arguments, stage):
    result = validate_syntax(arguments, '/project', stage)
    assert result['unsupported']


def test_unknown_nonpath_switches_are_not_rewritten_or_suppressed():
    arguments = ['/O1', '/O2', '/mystery:value', '/version:1.2', '-future', '/DNAME=path/inside/string']
    result = validate_syntax(arguments, '/project', 'compiler-effective')
    assert result['unsupported'] == [] and result['operands'] == []


@pytest.mark.parametrize('damage', [None, 'flag', 'macro_literal', 'include_order'])
def test_native_comparison_changes_only_explicitly_paired_path_operands(damage):
    old = ['/IZ:\\old\\include', '/IZ:\\sdk\\INCLUDE', '/O2', '/DNAME=unchanged',
           '/FoZ:\\old\\build\\unit.obj', '/FdZ:\\old\\build', 'src/unit.cpp']
    new = [word.replace('Z:\\old\\', 'Z:\\new\\') for word in old]
    if damage == 'flag':
        new[2] = '/O1'
    elif damage == 'macro_literal':
        old[3], new[3] = '/DNAME=/old/input', '/DNAME=/new/input'
    elif damage == 'include_order':
        new[0], new[1] = new[1], new[0]
    def parsed(argv, cwd):
        description = validate_syntax(argv, cwd, 'compiler-effective')
        return validate_command_paths(argv, cwd, description, stage='compiler-effective')
    report = compare_commands(parsed(old, '/old'), parsed(new, '/new'), path_pairs=[('/old', '/new')])
    assert report['result'] == ('pass' if damage is None else 'fail')


@pytest.mark.parametrize('damage', [None, 'prefix_case', 'libraries', 'duplicate_library'])
def test_native_link_comparison_retains_literal_prefixes_and_order(damage):
    old = ['CMakeFiles/unit.obj', '/out:app.exe', 'a.lib', 'b.lib', 'a.lib']
    new = list(old)
    if damage == 'prefix_case':
        new[1] = '/OUT:app.exe'
    elif damage == 'libraries':
        new[2], new[3] = new[3], new[2]
    elif damage == 'duplicate_library':
        new.pop()
    def parsed(argv, cwd):
        description = validate_syntax(argv, cwd, 'link-wrapper')
        return validate_command_paths(argv, cwd, description, stage='link-wrapper')
    report = compare_commands(parsed(old, '/old/build'), parsed(new, '/new/build'),
                              path_pairs=[('/old', '/new')])
    assert report['result'] == ('pass' if damage is None else 'fail')


@pytest.mark.parametrize('cwd', ['relative', '/project/../other', '/project//other', '//project',
                               '/project\\other', '/project/', ''])
def test_noncanonical_working_directory_refuses(cwd):
    with pytest.raises(ValueError):
        describe_command(['/c'], cwd, stage='compiler-effective')


@pytest.mark.parametrize('arguments', [[], 'compiler', [''], ['x\0y'], [7]])
def test_invalid_argument_shape_refuses(arguments):
    with pytest.raises(ValueError):
        describe_command(arguments, '/project', stage='compiler-effective')


def test_stage_must_identify_observed_command_boundary():
    with pytest.raises(ValueError):
        describe_command(['/c'], '/project', stage='guessed-stage')


def test_existing_retained_application_command_receipts_when_requested():
    location = os.environ.get('OPENSMACX_COMMAND_PATH_BUILD')
    if not location:
        pytest.skip('set OPENSMACX_COMMAND_PATH_BUILD to inspect an existing retained build; never builds one')
    directory = Path(location)
    assert directory.is_absolute() and directory.resolve() == directory and directory.is_dir()
    receipts = sorted(directory.glob('CMakeFiles/OpenSMACX.dir/**/*.compile.json'))
    assert receipts
    for path in receipts:
        receipt = json.loads(path.read_bytes())
        result = validate_syntax(receipt['arguments'], receipt['cwd'], 'compiler-effective')
        assert result['unsupported'] == [], (path, result['unsupported'])
        assert [row['path'] for row in result['operands'] if row['kind'] == 'source'] == [receipt['source']]
        assert receipt['output']['path'] in [row['path'] for row in result['operands'] if row['kind'] == 'output_file']
    link = json.loads((directory / 'OpenSMACX.link.inputs.json').read_bytes())
    result = validate_syntax(link['arguments'], link['cwd'], 'link-wrapper')
    assert result['unsupported'] == []
