"""Project command-description protocol never invokes a compiler or judges recovery."""

from copy import deepcopy
import io
import json
import os
from pathlib import Path

import pytest

from match_recovery.command_paths import validate_command_paths
from recovery.environment import adapter
from recovery.environment.command_paths import describe_command


@pytest.fixture(autouse=True)
def no_compiler(monkeypatch):
    def unexpected(*args, **kwargs):
        pytest.fail('command descriptions must not invoke compilation or toolchain discovery')
    monkeypatch.setattr(adapter, 'compile_object', unexpected)
    monkeypatch.setattr(adapter, 'compile_overlay', unexpected)
    monkeypatch.setattr(adapter, 'toolchain_inputs', unexpected)


@pytest.fixture
def request_value(tmp_path, monkeypatch):
    monkeypatch.chdir(tmp_path)
    (tmp_path / 'build').mkdir()
    return {'schema_version': 1, 'operation': 'command_paths', 'profile': 'application',
            'commands': [
                {'arguments': ['/I' + str(tmp_path / 'include'), '/O2', '/Fd' + str(tmp_path / 'build'),
                               '/Fo' + str(tmp_path / 'build/unit.obj'), 'src/unit.cpp'],
                 'cwd': str(tmp_path), 'stage': 'compiler-effective'},
                {'arguments': ['unit.obj', '/out:app.exe', '/pdb:app.pdb', 'a.lib', 'a.lib'],
                 'cwd': str(tmp_path / 'build'), 'stage': 'link-wrapper'}]}


def invoke(value, monkeypatch, capsys):
    raw = value if isinstance(value, str) else json.dumps(value)
    monkeypatch.setattr(adapter.sys, 'stdin', io.StringIO(raw))
    code = adapter.main()
    captured = capsys.readouterr()
    assert captured.err == ''
    return code, json.loads(captured.out)


@pytest.mark.parametrize('profile', ['application', 'default'])
def test_batch_protocol_returns_only_exact_project_descriptions(request_value, monkeypatch, capsys, profile):
    request_value['profile'] = profile
    before = deepcopy(request_value)
    def wrong_protocol(*args, **kwargs):
        pytest.fail('command-path request went through the compilation request parser')
    monkeypatch.setattr(adapter.CompilationRequest, 'read', wrong_protocol)
    code, response = invoke(request_value, monkeypatch, capsys)
    assert code == 0 and set(response) == {'schema_version', 'descriptions'}
    assert response['schema_version'] == 1 and len(response['descriptions']) == 2
    assert request_value == before
    for command, description in zip(request_value['commands'], response['descriptions'], strict=True):
        assert description == describe_command(command['arguments'], command['cwd'], stage=command['stage'])
        validate_command_paths(command['arguments'], command['cwd'], description, stage=command['stage'])
        assert 'result' not in description and 'verdict' not in description


def test_unsupported_grammar_remains_explicit_for_native_refusal(request_value, monkeypatch, capsys):
    request_value['commands'][0]['arguments'] = ['@unexpanded.rsp']
    code, response = invoke(request_value, monkeypatch, capsys)
    assert code == 0
    assert response['descriptions'][0]['unsupported']
    command = request_value['commands'][0]
    with pytest.raises(ValueError, match='unsupported'):
        validate_command_paths(command['arguments'], command['cwd'], response['descriptions'][0],
                               stage=command['stage'])
    assert response['descriptions'][1]['unsupported'] == []


@pytest.mark.parametrize('damage', ['schema', 'schema_bool', 'extra', 'missing_profile', 'unknown_profile',
                                   'empty_profile', 'nontext_profile', 'empty_commands', 'nonarray_commands',
                                   'command_extra', 'command_missing', 'empty_arguments', 'nonarray_arguments',
                                   'invalid_argument', 'nul_argument', 'relative_cwd', 'outside_cwd',
                                   'dotdot_cwd', 'nontext_cwd', 'empty_stage', 'unknown_stage'])
def test_invalid_requests_refuse_without_any_compile(request_value, monkeypatch, capsys, tmp_path, damage):
    value, command = request_value, request_value['commands'][0]
    if damage == 'schema': value['schema_version'] = 2
    elif damage == 'schema_bool': value['schema_version'] = True
    elif damage == 'extra': value['unexpected'] = 1
    elif damage == 'missing_profile': del value['profile']
    elif damage == 'unknown_profile': value['profile'] = 'unapproved'
    elif damage == 'empty_profile': value['profile'] = ''
    elif damage == 'nontext_profile': value['profile'] = 2
    elif damage == 'empty_commands': value['commands'] = []
    elif damage == 'nonarray_commands': value['commands'] = {}
    elif damage == 'command_extra': command['normalized_verdict'] = 'pass'
    elif damage == 'command_missing': del command['stage']
    elif damage == 'empty_arguments': command['arguments'] = []
    elif damage == 'nonarray_arguments': command['arguments'] = '/c src/unit.cpp'
    elif damage == 'invalid_argument': command['arguments'] = [None]
    elif damage == 'nul_argument': command['arguments'] = ['x\0y']
    elif damage == 'relative_cwd': command['cwd'] = 'build'
    elif damage == 'outside_cwd': command['cwd'] = str(tmp_path.parent)
    elif damage == 'dotdot_cwd': command['cwd'] = str(tmp_path / 'build/..')
    elif damage == 'nontext_cwd': command['cwd'] = 3
    elif damage == 'empty_stage': command['stage'] = ''
    else: command['stage'] = 'unknown-command-stage'
    code, response = invoke(value, monkeypatch, capsys)
    assert code == 3 and set(response) == {'error'} and response['error']


@pytest.mark.parametrize('value', ['not json', '[]',
                                  '{"schema_version":1,"schema_version":1,"operation":"command_paths"}'])
def test_invalid_record_encoding_refuses(value, monkeypatch, capsys):
    code, response = invoke(value, monkeypatch, capsys)
    assert code == 3 and set(response) == {'error'}


def test_retained_build_batch_metadata_when_requested(monkeypatch, capsys):
    location = os.environ.get('OPENSMACX_COMMAND_PATH_BUILD')
    if not location:
        pytest.skip('set OPENSMACX_COMMAND_PATH_BUILD to read existing receipts; never builds one')
    directory = Path(location)
    assert directory.is_absolute() and directory.resolve() == directory and directory.is_dir()
    root = Path(__file__).resolve().parents[2]
    monkeypatch.chdir(root)
    commands = []
    for path in sorted(directory.glob('CMakeFiles/OpenSMACX.dir/**/*.compile.json')):
        row = json.loads(path.read_bytes())
        commands.append({'arguments': row['arguments'], 'cwd': row['cwd'], 'stage': 'compiler-effective'})
    assert commands
    link = json.loads((directory / 'OpenSMACX.link.inputs.json').read_bytes())
    commands.append({'arguments': link['arguments'], 'cwd': link['cwd'], 'stage': 'link-wrapper'})
    code, response = invoke({'schema_version': 1, 'operation': 'command_paths',
                             'profile': 'application', 'commands': commands}, monkeypatch, capsys)
    assert code == 0 and len(response['descriptions']) == len(commands)
    for command, description in zip(commands, response['descriptions'], strict=True):
        assert description['unsupported'] == []
        validate_command_paths(command['arguments'], command['cwd'], description, stage=command['stage'])
