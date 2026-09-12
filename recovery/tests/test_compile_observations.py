"""Fresh compile observations bind files, not compiler semantics or acceptance."""

import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.compile_file_evidence import verify_compile_files
from recovery.environment import wine, wine_compile
from recovery.environment.wine_repro import ReproducibleCompile


def observed_text(source, header):
    return (f'#line 1 "{source}"\nint value() {{ return 7; }}\n'
            f'#line 1 "{header}"\n').encode()


@pytest.fixture
def compilation(tmp_path, monkeypatch):
    for name in tuple(os.environ):
        if name.upper() in {'CL', '_CL_'}:
            monkeypatch.delenv(name)
    (tmp_path / 'src').mkdir()
    (tmp_path / 'build').mkdir()
    (tmp_path / 'BIN').mkdir()
    source, header = tmp_path / 'src/unit.cpp', tmp_path / 'src/value.h'
    source.write_text('#include "value.h"\nint value() { return VALUE; }\n')
    header.write_text('#define VALUE 7\n')
    tool = tmp_path / 'BIN/CL.EXE'
    tool.write_bytes(b'synthetic compiler identity; never executed')
    output = tmp_path / 'build/unit.obj'
    arguments = ['--repro-root=' + str(tmp_path), '/nologo', '/c', '/O2',
                 '/Fo' + str(output), str(source)]
    policy = ReproducibleCompile.read('cl', arguments, tmp_path / 'build')
    effective = policy.verify(lambda words, cwd: observed_text(source, header))
    return tmp_path, source, header, tool, output, arguments, policy, effective


def encoded(value):
    return json.dumps(value).encode()


def test_capture_binds_actual_verified_closure_and_effective_arguments(compilation):
    root, source, header, tool, output, _, policy, effective = compilation
    before = wine_compile.capture_before(policy, effective, tool)
    assert before['arguments'] == effective and effective[-1] == '/Brepro'
    assert before['cwd'] == str(root) and before['source'] == str(source)
    assert {row['path'] for row in before['inputs']} == {str(source), str(header), str(tool)}
    assert before['output'] == {'path': str(output), 'existed_before': False}
    output.write_bytes(b'synthetic object content')
    after = wine_compile.capture_after(before, 0)
    assert set(after) == {'schema_version', 'source', 'tool', 'cwd', 'arguments',
                          'inputs', 'output', 'exit_code'}
    assert after['schema_version'] == 1 and after['exit_code'] == 0
    assert after['output']['after'] == {'sha256': hashlib.sha256(output.read_bytes()).hexdigest(),
                                       'size': output.stat().st_size}
    assert all(row['before'] == row['after'] for row in after['inputs'])
    report = verify_compile_files(encoded(after), object_path=output)
    assert report['result'] == 'pass'
    assert report['input_coverage'] == report['linked_selection'] == 'unknown'


@pytest.mark.parametrize('damage', ['source', 'header', 'tool', 'missing_header'])
def test_input_change_between_observations_cannot_pass(compilation, damage):
    _, source, header, tool, output, _, policy, effective = compilation
    before = wine_compile.capture_before(policy, effective, tool)
    target = {'source': source, 'header': header, 'tool': tool, 'missing_header': header}[damage]
    if damage == 'missing_header':
        target.unlink()
    else:
        target.write_bytes(target.read_bytes() + b'changed')
    output.write_bytes(b'synthetic object')
    report = verify_compile_files(encoded(wine_compile.capture_after(before, 0)))
    assert report['result'] != 'pass'
    assert any(row['path'] == str(target) and row['result'] != 'pass' for row in report['inputs'])


@pytest.mark.parametrize('code,contents,existed', [(1, b'object', False), (-6, b'object', False),
                                                 (0, None, False), (0, b'', False), (0, b'object', True)])
def test_failed_empty_missing_or_preexisting_output_cannot_pass(compilation, code, contents, existed):
    _, _, _, tool, output, _, policy, effective = compilation
    if existed:
        output.write_bytes(b'old object')
    before = wine_compile.capture_before(policy, effective, tool)
    if contents is not None:
        output.write_bytes(contents)
    after = wine_compile.capture_after(before, code)
    assert after['exit_code'] == code and after['output']['existed_before'] == existed
    assert verify_compile_files(encoded(after))['result'] != 'pass'


@pytest.mark.parametrize('damage', ['unverified', 'no_policy', 'changed_header', 'overlap',
                                   'duplicate_output', 'relative_output', 'source_alias', 'output_alias'])
def test_capture_refuses_unverified_or_ambiguous_inputs(compilation, damage):
    root, source, header, tool, output, arguments, policy, effective = compilation
    if damage == 'unverified':
        policy = ReproducibleCompile.read('cl', arguments, root / 'build')
    elif damage == 'no_policy':
        policy = None
    elif damage == 'changed_header':
        header.write_text('#define VALUE 8\n')
    elif damage == 'overlap':
        effective = [word if not word.startswith('/Fo') else '/Fo' + str(source) for word in effective]
    elif damage == 'duplicate_output':
        effective = [*effective, '/Fo' + str(output)]
    elif damage == 'relative_output':
        effective = [word if not word.startswith('/Fo') else '/Founit.obj' for word in effective]
    elif damage == 'source_alias':
        alias = root / 'src/alias.cpp'
        alias.symlink_to(source)
        policy.source = alias
    else:
        output.symlink_to(source)
    with pytest.raises(ValueError):
        wine_compile.capture_before(policy, effective, tool)


@pytest.mark.parametrize('code,change_header', [(0, False), (7, False), (0, True)])
def test_wrapper_opt_in_captures_actual_compile_and_does_not_hide_failure(
        compilation, monkeypatch, capsys, code, change_header):
    root, source, header, _, output, arguments, _, effective = compilation
    monkeypatch.setattr(wine, 'VC6_ROOT', root)
    monkeypatch.chdir(root / 'build')
    destination = root / 'build/unit.observations'
    calls = []

    def run(command, **options):
        calls.append(command)
        assert not any(word.startswith('--compile-diagnostics=') for word in command)
        if '/E' in command:
            assert options['binary'] and options['explicit_compiler']
            return subprocess.CompletedProcess(command, 0, observed_text(source, header), b'')
        assert options == {'cwd': root, 'explicit_compiler': True}
        output.write_bytes(b'synthetic object')
        if change_header:
            header.write_text('#define VALUE 8\n')
        return subprocess.CompletedProcess(command, code, 'unit.cpp\n', '')

    monkeypatch.setattr(wine, 'run_tool', run)
    assert wine.main(['vc6-cl', *arguments, '--compile-diagnostics=' + str(destination)]) == (
        3 if change_header else code)
    assert len(calls) == 4 and sum('/E' not in call for call in calls) == 1
    if change_header:
        assert 'inputs changed' in capsys.readouterr().err
        assert not destination.exists()
    else:
        evidence = json.loads(destination.read_bytes())
        assert evidence['arguments'] == list(map(wine.translate, effective)) and evidence['exit_code'] == code
        report = verify_compile_files(destination.read_bytes(), object_path=output)
        assert (report['result'] == 'pass') == (code == 0)


@pytest.mark.parametrize('alias', [False, True])
def test_capture_observes_physical_files_through_stable_header_spellings(compilation, alias):
    root, source, header, tool, _, arguments, _, _ = compilation
    through = root / 'src/through'
    if alias:
        through.symlink_to(header.parent, target_is_directory=True)
        logical = through / header.name
    else:
        through.mkdir()
        logical = through / '..' / header.name
    policy = ReproducibleCompile.read('cl', arguments, root / 'build')
    effective = policy.verify(lambda words, cwd: observed_text(source, logical))
    before = wine_compile.capture_before(policy, effective, tool)
    assert str(header) in {row['path'] for row in before['inputs']}
    assert str(logical) not in {row['path'] for row in before['inputs']}


def test_wrapper_refuses_retargeted_logical_header_before_publishing_observations(
        compilation, monkeypatch, capsys):
    root, source, header, _, output, arguments, _, _ = compilation
    through = root / 'src/through'
    through.mkdir()
    logical = through / '..' / header.name
    other = root / 'different'
    (other / 'nested').mkdir(parents=True)
    (other / header.name).write_text('#define VALUE 99\n')
    source.write_text('#include "through/../value.h"\nint value() { return VALUE; }\n')
    original = header.read_bytes()
    monkeypatch.setattr(wine, 'VC6_ROOT', root)
    monkeypatch.chdir(root / 'build')
    destination = root / 'build/unit.observations'
    calls = []

    def run(command, **options):
        calls.append(command)
        if '/E' in command:
            return subprocess.CompletedProcess(command, 0, observed_text(source, logical), b'')
        # The receipt's canonical input stays intact, but the compiler-visible
        # logical path now denotes different bytes. The final reproducibility
        # check—not canonical-file observations alone—must refuse success.
        through.rmdir()
        through.symlink_to(other / 'nested', target_is_directory=True)
        output.write_bytes(b'synthetic object from a changed logical include')
        return subprocess.CompletedProcess(command, 0, '', '')

    monkeypatch.setattr(wine, 'run_tool', run)
    assert wine.main(['vc6-cl', *arguments, '--compile-diagnostics=' + str(destination)]) == 3
    assert len(calls) == 4 and output.exists()
    assert header.read_bytes() == original and logical.read_bytes() != original
    assert 'inputs changed' in capsys.readouterr().err
    assert not destination.exists()


@pytest.mark.parametrize('damage', ['duplicate', 'empty', 'link', 'lib', 'probe', 'no_policy',
                                   'existing_metadata', 'source_metadata', 'output_metadata', 'alias_metadata'])
def test_wrapper_refuses_invalid_observation_requests_without_object_compile(
        compilation, monkeypatch, damage):
    root, source, header, tool, output, arguments, _, _ = compilation
    monkeypatch.setattr(wine, 'VC6_ROOT', root)
    monkeypatch.chdir(root / 'build')
    destination = root / 'build/unit.observations'
    flags, launcher = ['--compile-diagnostics=' + str(destination)], 'vc6-cl'
    preserved = None
    if damage == 'duplicate':
        flags *= 2
    elif damage == 'empty':
        flags = ['--compile-diagnostics=']
    elif damage in {'link', 'lib'}:
        launcher = 'vc6-' + damage
        (tool.parent / wine.TOOLS[damage]).write_bytes(b'synthetic tool')
    elif damage == 'probe':
        arguments = [arguments[0], '/E', str(source)]
    elif damage == 'no_policy':
        arguments = arguments[1:]
    elif damage == 'existing_metadata':
        destination.write_bytes(b'do not overwrite')
        preserved = destination.read_bytes()
    elif damage == 'source_metadata':
        destination = source
        preserved = source.read_bytes()
    elif damage == 'output_metadata':
        destination = output
    else:
        destination.symlink_to(source)
        preserved = source.read_bytes()
    if damage not in {'duplicate', 'empty'}:
        flags = ['--compile-diagnostics=' + str(destination)]
    calls = []

    def run(command, **options):
        calls.append(command)
        assert '/E' in command, 'invalid observation request reached object compilation'
        return subprocess.CompletedProcess(command, 0, observed_text(source, header), b'')

    monkeypatch.setattr(wine, 'run_tool', run)
    assert wine.main([launcher, *arguments, *flags]) == 3
    assert not output.exists()
    assert len(calls) in {0, 3}
    if preserved is not None:
        assert destination.read_bytes() == preserved
    else:
        assert not destination.exists()


@pytest.mark.parametrize('alias', [False, True])
def test_wrapper_preserves_metadata_created_during_compilation(compilation, monkeypatch, alias):
    root, source, header, _, output, arguments, _, _ = compilation
    monkeypatch.setattr(wine, 'VC6_ROOT', root)
    monkeypatch.chdir(root / 'build')
    destination = root / 'build/unit.observations'
    original = source.read_bytes()

    def run(command, **options):
        if '/E' in command:
            return subprocess.CompletedProcess(command, 0, observed_text(source, header), b'')
        output.write_bytes(b'synthetic object')
        if alias:
            destination.symlink_to(source)
        else:
            destination.write_bytes(b'other writer owns this file')
        return subprocess.CompletedProcess(command, 0, '', '')

    monkeypatch.setattr(wine, 'run_tool', run)
    assert wine.main(['vc6-cl', *arguments, '--compile-diagnostics=' + str(destination)]) == 3
    assert output.read_bytes() == b'synthetic object'
    assert source.read_bytes() == original
    assert destination.read_bytes() == (original if alias else b'other writer owns this file')
    assert destination.is_symlink() == alias


def test_real_vc6_compile_observations_bind_header_and_output(tmp_path, monkeypatch):
    if not (wine.VC6_ROOT / 'BIN/CL.EXE').is_file():
        pytest.skip('VC6 is not installed')
    for name in tuple(os.environ):
        if name.upper() in {'CL', '_CL_'}:
            monkeypatch.delenv(name)
    source, header = tmp_path / 'unit.cpp', tmp_path / 'value.h'
    source.write_text('#include "value.h"\nint value() { return VALUE; }\n')
    header.write_text('#define VALUE 7\n')
    output, receipt = tmp_path / 'unit.obj', tmp_path / 'compile.observations'
    command = [sys.executable, str(Path(wine.__file__)), '--tool=cl',
               '--repro-root=' + str(tmp_path), '--compile-diagnostics=' + str(receipt),
               '/nologo', '/c', '/O2', '/Gy', '/Fo' + str(output), str(source)]
    result = subprocess.run(command, cwd=tmp_path, capture_output=True, timeout=60)
    assert result.returncode == 0, (result.stdout + result.stderr).decode(errors='replace')
    data = receipt.read_bytes()
    assert verify_compile_files(data, object_path=output)['result'] == 'pass'
    assert str(header) in {row['path'] for row in json.loads(data)['inputs']}
    original = header.read_bytes()
    header.write_bytes(original + b'// changed after compilation\n')
    assert verify_compile_files(data)['result'] == 'fail'
    header.write_bytes(original)
    assert verify_compile_files(data)['result'] == 'pass'
    output.write_bytes(output.read_bytes() + b'changed after compilation')
    assert verify_compile_files(data)['result'] == 'fail'
