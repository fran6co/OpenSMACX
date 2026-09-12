"""Native fresh-build collection, with real file mutation/refusal controls."""

import hashlib
import json
from pathlib import Path

import pytest

from match_recovery import build_compile_evidence as collector
from test_compile_link_evidence import build as compile_link_fixture


@pytest.fixture
def observed(tmp_path, compile_link_fixture):
    # The fixture compiler emits into this root; enclosing project is its
    # canonical parent, matching a private native build subdirectory.
    compiles, link, sources = compile_link_fixture
    directory = tmp_path
    config = {'database': 'commands.json', 'source_extensions': ['.cc'], 'directory': 'objects',
              'suffix': '.compile.json', 'link_files': 'link.inputs.json', 'link_log': 'link.log'}
    from test_link_log import LOG
    (directory / 'objects').mkdir()
    (directory / 'objects/unit.compile.json').write_text(json.dumps(compiles[0]))
    (directory / 'commands.json').write_text(json.dumps([
        {'directory': str(directory), 'file': str(sources[0]), 'arguments': ['compiler', str(sources[0])]}]))
    (directory / 'link.inputs.json').write_text(json.dumps(link))
    (directory / 'link.log').write_text(LOG)
    (directory / 'application').write_bytes(b'synthetic linked program output')
    (directory / 'application.map').write_bytes(b'synthetic linker map output')
    outputs = []
    for name, kind in [('commands.json', 'compilation-database'), ('link.inputs.json', 'link-input-files'),
                       ('link.log', 'msvc-link-log'), ('application', 'file'), ('application.map', 'file')]:
        raw = (directory / name).read_bytes()
        outputs.append({'path': name, 'format': kind, 'sha256': hashlib.sha256(raw).hexdigest(),
                        'size': len(raw), 'result': 'pass'})
    return directory.parent, directory, config, outputs


def verify(observed):
    root, directory, config, outputs = observed
    return collector.verify_build_compilations(root, directory, config, outputs=outputs)


def repin(observed, name):
    raw = (observed[1] / name).read_bytes()
    row = next(item for item in observed[3] if item['path'] == name)
    row.update(sha256=hashlib.sha256(raw).hexdigest(), size=len(raw))


def test_native_collector_connects_declared_application_outputs(observed):
    report = verify(observed)
    assert report['result'] == 'pass'
    assert len(report['compilations']) == 1
    assert report['linked_selection'] == report['build_graph_verification'] == 'unknown'


@pytest.mark.parametrize('name', ['commands.json', 'link.inputs.json', 'link.log', 'application', 'application.map'])
def test_cannot_mix_artifacts_changed_after_build_format_checks(observed, name):
    path = observed[1] / name
    path.write_bytes(path.read_bytes() + b'\n')
    with pytest.raises(ValueError, match='artifact changed'):
        verify(observed)


@pytest.mark.parametrize('damage', ['symlink', 'hardlink', 'directory_alias', 'missing_tree'])
def test_observation_collection_refuses_unsafe_or_missing_paths(observed, damage):
    import os
    directory = observed[1]
    path = directory / 'objects/unit.compile.json'
    if damage == 'directory_alias':
        (directory / 'objects').rename(directory / 'elsewhere')
        (directory / 'objects').symlink_to(directory / 'elsewhere', target_is_directory=True)
    elif damage == 'missing_tree':
        observed[2]['directory'] = 'absent'
    else:
        path.rename(directory / 'saved.json')
        if damage == 'symlink':
            path.symlink_to(directory / 'saved.json')
        else:
            os.link(directory / 'saved.json', path)
    with pytest.raises(ValueError):
        verify(observed)


@pytest.mark.parametrize('damage', ['object', 'generated_header', 'metadata', 'added_receipt', 'executable', 'map'])
def test_mutation_during_verification_cannot_publish_stale_correspondence(observed, monkeypatch, damage):
    original = collector.verify_compile_link_inputs
    directory = observed[1]
    def mutate(*args, **kwargs):
        report = original(*args, **kwargs)
        assert report['result'] == 'pass'
        if damage == 'added_receipt':
            (directory / 'objects/added.compile.json').write_text('{}')
        else:
            path = directory / {'object': 'unit.obj', 'generated_header': 'header.h',
                                'metadata': 'objects/unit.compile.json',
                                'executable': 'application', 'map': 'application.map'}[damage]
            path.write_bytes(path.read_bytes() + b'changed')
        return report
    monkeypatch.setattr(collector, 'verify_compile_link_inputs', mutate)
    with pytest.raises(ValueError, match='changed during'):
        verify(observed)


@pytest.mark.parametrize('damage', ['outside_source', 'aliased_source', 'duplicate_source', 'outside_directory'])
def test_build_recipes_cannot_hide_ambiguous_or_external_source_context(observed, damage):
    directory = observed[1]
    path = directory / 'commands.json'
    rows = json.loads(path.read_bytes())
    if damage == 'duplicate_source':
        rows.append(dict(rows[0]))
    elif damage == 'outside_source':
        rows[0]['file'] = '/etc/hosts'
    elif damage == 'aliased_source':
        alias = directory / 'alias.cc'
        alias.symlink_to(directory / 'unit.cc')
        rows[0]['file'] = str(alias)
    else:
        rows[0]['directory'] = '/tmp'
        # tmp_path's project parent is nested within /tmp, so cwd is external.
    path.write_text(json.dumps(rows))
    repin(observed, 'commands.json')
    with pytest.raises(ValueError):
        verify(observed)


def test_missing_receipt_remains_unknown_not_empty_success(observed):
    (observed[1] / 'objects/unit.compile.json').unlink()
    assert verify(observed)['result'] == 'unknown'


def test_receipt_cannot_substitute_an_output_from_another_build(observed):
    path = observed[1] / 'objects/unit.compile.json'
    value = json.loads(path.read_bytes())
    value['output']['path'] = str(observed[0] / 'old.obj')
    path.write_text(json.dumps(value))
    with pytest.raises(ValueError, match='current fresh build'):
        verify(observed)


def test_command_binding_is_opt_in_and_receives_exact_verified_inputs(observed, monkeypatch):
    from match_recovery import build_command_evidence
    calls = []
    def bind(root, directory, compilations, link, stages):
        assert root == observed[0] and directory == observed[1]
        assert compilations[0]['source'] == str(directory / 'unit.cc')
        assert compilations[0]['output']['path'] in link['explicit_inputs']
        assert stages == {'compile': 'effective', 'link': 'wrapper'}
        calls.append(True)
        return {'result': 'pass', 'test_double': True}
    monkeypatch.setattr(build_command_evidence, 'bind_build_commands', bind)
    assert 'command_bindings' not in verify(observed) and calls == []
    observed[2]['command_stages'] = {'compile': 'effective', 'link': 'wrapper'}
    report = verify(observed)
    assert calls == [True] and report['command_bindings']['result'] == 'pass'


def test_failed_file_evidence_never_calls_command_describer(observed, monkeypatch):
    from match_recovery import build_command_evidence
    observed[2]['command_stages'] = {'compile': 'effective', 'link': 'wrapper'}
    monkeypatch.setattr(build_command_evidence, 'bind_build_commands',
                        lambda *a, **k: pytest.fail('cannot bind commands against missing compile evidence'))
    (observed[1] / 'objects/unit.compile.json').unlink()
    report = verify(observed)
    assert report['result'] == 'unknown' and report['command_bindings']['verification'] == 'not_measured'


@pytest.mark.parametrize('damage', ['source', 'executable', 'receipt', 'membership'])
def test_command_description_is_inside_the_file_freshness_epoch(observed, monkeypatch, damage):
    from match_recovery import build_command_evidence
    observed[2]['command_stages'] = {'compile': 'effective', 'link': 'wrapper'}
    def mutate(*args):
        path = observed[1] / {'source': 'unit.cc', 'executable': 'application',
                              'receipt': 'objects/unit.compile.json', 'membership': 'objects/new.compile.json'}[damage]
        path.write_bytes(b'changed during command description')
        return {'result': 'pass'}
    monkeypatch.setattr(build_command_evidence, 'bind_build_commands', mutate)
    with pytest.raises(ValueError, match='changed during'):
        verify(observed)


@pytest.mark.parametrize('stages', [None, {}, {'compile': 'one'}, {'compile': '', 'link': 'one'},
                                  {'compile': 'one', 'link': 'two', 'trust': True},
                                  {'compile': 'one', 'link': 7}, {'compile': 'one\0', 'link': 'two'}])
def test_command_observation_stages_are_explicit_policy(observed, stages):
    observed[2]['command_stages'] = stages
    with pytest.raises(ValueError, match='command_stages'):
        collector.validate_configuration(observed[2], [(item['path'], item['format']) for item in observed[3]])
