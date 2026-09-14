"""Real temporary file controls, synthetic receipts, no compiler process."""

from collections import Counter
import json
import os

import pytest

from match_recovery import build_compile_evidence as collector
from test_build_compile_evidence import observed, compile_link_fixture, repin
from test_application_evidence_context import native_context, configured, project
from test_application_context import context
from test_build_check import command


def epoch(observed):
    root, directory, configuration, outputs = observed
    return collector.live_build_compilations(root, directory, configuration, outputs=outputs)


def test_refresh_rehashes_each_unique_file_without_reparsing_or_rebinding(observed, monkeypatch):
    with epoch(observed) as (report, current):
        assert report == collector.verify_build_compilations(*observed[:3], outputs=observed[3])
        calls = Counter()
        original = collector._observe
        def observe(path):
            calls[path] += 1
            return original(path)
        monkeypatch.setattr(collector, '_observe', observe)
        def forbidden(*args, **kwargs):
            pytest.fail('live refresh must not repeat semantic verification')
        for name in ('entries', 'parse_json_value', 'parse_compile_files', 'parse_link_files', 'verify_compile_link_inputs'):
            monkeypatch.setattr(collector, name, forbidden)
        current()
        assert len(calls) >= 10
        assert set(calls.values()) == {1}
        current()
        assert set(calls.values()) == {2}


@pytest.mark.parametrize('name', ['commands.json', 'link.inputs.json', 'link.log', 'application',
                                  'application.map', 'unit.obj', 'unit.cc', 'header.h',
                                  'objects/unit.compile.json'])
def test_refresh_detects_content_changes_even_with_restored_mtime(observed, name):
    with epoch(observed) as (_, current):
        path = observed[1] / name
        raw, info = path.read_bytes(), path.stat()
        path.write_bytes(bytes([raw[0] ^ 1]) + raw[1:])
        os.utime(path, ns=(info.st_atime_ns, info.st_mtime_ns))
        with pytest.raises(ValueError, match='evidence changed'):
            current()


@pytest.mark.parametrize('change', ['added', 'removed', 'renamed', 'symlink', 'hardlink', 'tree_alias'])
def test_every_refresh_rechecks_membership_and_physical_files(observed, change):
    with epoch(observed) as (_, current):
        directory = observed[1]
        path = directory / 'objects/unit.compile.json'
        if change == 'added':
            (path.parent / 'other.compile.json').write_bytes(path.read_bytes())
        elif change == 'removed':
            path.unlink()
        elif change == 'renamed':
            path.rename(path.with_name('other.compile.json'))
        elif change == 'tree_alias':
            path.parent.rename(directory / 'moved')
            (directory / 'objects').symlink_to(directory / 'moved')
        else:
            path.rename(directory / 'saved')
            if change == 'symlink':
                path.symlink_to(directory / 'saved')
            else:
                os.link(directory / 'saved', path)
        with pytest.raises((OSError, ValueError)):
            current()


def test_refresh_detects_earlier_file_mutated_after_its_hash(observed, monkeypatch):
    with epoch(observed) as (_, current):
        original = collector._observe
        first = None
        def observe(path):
            nonlocal first
            result = original(path)
            if first is None:
                first = path
            elif first is not False:
                info = first.stat()
                first.write_bytes(first.read_bytes() + b'changed after its hash')
                os.utime(first, ns=(info.st_atime_ns, info.st_mtime_ns))
                first = False
            return result
        monkeypatch.setattr(collector, '_observe', observe)
        with pytest.raises(ValueError, match='changed during freshness'):
            current()


def test_refresh_rescans_membership_after_file_hashing(observed, monkeypatch):
    with epoch(observed) as (_, current):
        original = collector._observe
        def observe(path):
            result = original(path)
            (observed[1] / 'objects/extra.compile.json').write_text('{}')
            return result
        monkeypatch.setattr(collector, '_observe', observe)
        with pytest.raises(ValueError, match='observation membership'):
            current()


def test_detached_report_and_caller_config_cannot_repin_evidence(observed):
    with epoch(observed) as (report, current):
        report.clear()
        observed[2]['suffix'] = '.other'
        observed[3].clear()
        current()
        path = observed[1] / 'objects/unit.compile.json'
        value = json.loads(path.read_bytes())
        value['arguments'].append('-DSELF_CONSISTENT_CHANGE')
        path.write_text(json.dumps(value))
        with pytest.raises(ValueError, match='evidence changed'):
            current()


def test_live_callable_expires_after_success_or_exception(observed):
    with epoch(observed) as (_, current):
        current()
    with pytest.raises(ValueError, match='expired'):
        current()
    with pytest.raises(RuntimeError):
        with epoch(observed) as (_, current):
            raise RuntimeError('leave context')
    with pytest.raises(ValueError, match='expired'):
        current()


def test_missing_evidence_never_seals_a_caller_pass(observed):
    (observed[1] / 'objects/unit.compile.json').unlink()
    with pytest.raises(ValueError, match='fresh complete native'):
        with epoch(observed):
            pytest.fail('missing evidence cannot seal')


def test_separate_live_contexts_each_run_native_semantic_verification(observed, monkeypatch):
    original = collector.verify_compile_link_inputs
    calls = []
    def verify(*args, **kwargs):
        calls.append(True)
        return original(*args, **kwargs)
    monkeypatch.setattr(collector, 'verify_compile_link_inputs', verify)
    with epoch(observed) as (_, first):
        with epoch(observed) as (_, second):
            first()
            second()
    assert calls == [True, True]


@pytest.fixture
def commanded(observed, compile_link_fixture, monkeypatch):
    from match_recovery import build_command_evidence
    from test_build_command_evidence import observed as command_fixture, client, STAGES
    values = command_fixture.__wrapped__(observed[1], compile_link_fixture)
    provider = client(values)
    monkeypatch.setattr(build_command_evidence, 'Compiler', lambda root: provider)
    observed[2]['command_stages'] = STAGES
    (observed[1] / 'objects/unit.compile.json').write_text(json.dumps(values[2][0]))
    (observed[1] / 'link.inputs.json').write_text(json.dumps(values[3]))
    repin(observed, 'link.inputs.json')
    return observed, provider


def test_command_provider_runs_once_and_only_native_report_is_sealed(commanded):
    observed, provider = commanded
    with epoch(observed) as (report, current):
        assert report['command_bindings']['result'] == 'pass'
        current()
        current()
        assert len(provider.calls) == 1


@pytest.mark.parametrize('kind,damage', [
    ('include', 'missing'), ('include', 'symlink'), ('debug', 'file'),
    ('application.exe', 'directory'), ('application.exe', 'symlink')])
def test_command_path_roles_are_rechecked_without_describing_again(commanded, kind, damage):
    observed, provider = commanded
    with epoch(observed) as (_, current):
        path = observed[1] / kind
        if path.exists():
            path.rmdir()
        if damage == 'symlink':
            path.symlink_to(observed[1] / 'unit.obj')
        elif damage == 'file':
            path.write_bytes(b'wrong directory type')
        elif damage == 'directory':
            path.mkdir()
        with pytest.raises((ValueError, OSError)):
            current()
        assert len(provider.calls) == 1


def test_compiler_working_directory_is_rechecked_even_when_not_an_input(observed):
    cwd = observed[1] / 'compiler-cwd'
    cwd.mkdir()
    path = observed[1] / 'objects/unit.compile.json'
    receipt = json.loads(path.read_bytes())
    receipt['cwd'] = str(cwd)
    path.write_text(json.dumps(receipt))
    with epoch(observed) as (_, current):
        cwd.rmdir()
        with pytest.raises((OSError, ValueError)):
            current()


def test_excluded_language_recipe_paths_still_require_physical_sources(observed):
    source = observed[1] / 'resource.rc'
    source.write_text('synthetic resource source')
    path = observed[1] / 'commands.json'
    database = json.loads(path.read_bytes())
    database.append({'directory': str(observed[1]), 'file': str(source), 'arguments': ['resource-compiler', str(source)]})
    path.write_text(json.dumps(database))
    repin(observed, 'commands.json')
    with epoch(observed) as (_, current):
        source.unlink()
        with pytest.raises(ValueError, match='recipe paths'):
            current()


def test_receipt_cannot_omit_its_compiler_executable(observed):
    path = observed[1] / 'objects/unit.compile.json'
    receipt = json.loads(path.read_bytes())
    receipt['inputs'] = [row for row in receipt['inputs'] if row['path'] != receipt['tool']]
    path.write_text(json.dumps(receipt))
    with pytest.raises(ValueError, match='tool lacks file observations'):
        with epoch(observed):
            pytest.fail('a caller PASS cannot replace an unobserved executable')


@pytest.mark.parametrize('damage', ['private_input', 'original_input', 'environment'])
def test_outer_compiler_provider_extra_inputs_and_environment_stay_guarded(native_context, monkeypatch, damage):
    root = native_context
    helper_name = 'build/provider-extra.py'
    (root / 'build').mkdir(exist_ok=True)
    (root / helper_name).write_text('provider policy data, never executed')
    path = root / 'recovery/project.json'
    value = json.loads(path.read_bytes())
    value['compiler'] = {'command': command('pass'), 'inputs': [helper_name],
                         'environment': ['EPOCH_PROVIDER_OPTION']}
    path.write_text(json.dumps(value))
    with pytest.raises(ValueError, match='application inputs or Git state changed'):
        with context(root) as application:
            if damage == 'environment':
                monkeypatch.setenv('EPOCH_PROVIDER_OPTION', 'changed')
            else:
                target = root if damage == 'original_input' else application.root
                (target / helper_name).write_text('changed provider-only input')
            application.check_current()


def test_outer_toolchain_executable_absent_from_receipts_is_still_guarded(native_context, tmp_path_factory):
    root = native_context
    external = tmp_path_factory.mktemp('epoch-toolchain') / 'compiler'
    external.write_bytes(b'synthetic toolchain identity; never executed')
    description = {'schema_version': 1, 'files': {'compiler': str(external)}, 'directories': {}}
    script = root / 'recovery/discover-extra.py'
    script.write_text('import json, sys\njson.load(sys.stdin)\nprint(json.dumps(' + repr(description) + '))\n')
    path = root / 'recovery/project.json'
    value = json.loads(path.read_bytes())
    value['toolchain'] = {
        'command': command("import runpy; runpy.run_path('recovery/discover-extra.py',run_name='__main__')"),
        'inputs': ['recovery/discover-extra.py'], 'environment': []}
    path.write_text(json.dumps(value))
    with pytest.raises(ValueError, match='application inputs or Git state changed'):
        with context(root) as application:
            assert str(external) not in json.dumps(application.report['compile_link'])
            external.write_bytes(b'changed external compiler identity')
            application.check_current()


def test_application_report_mutation_cannot_repin_observed_outputs(native_context):
    with pytest.raises(ValueError, match='evidence changed'):
        with context(native_context) as application:
            application._report['compile_link'].clear()
            application._report['outputs'].clear()
            application.check_current()
            path = application.directory / 'application'
            path.write_bytes(path.read_bytes() + b'changed after detached report mutation')
            application.check_current()
