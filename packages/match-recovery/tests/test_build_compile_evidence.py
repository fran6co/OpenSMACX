"""Native fresh-build collection, with real file mutation/refusal controls."""

import hashlib
import json
from pathlib import Path

import pytest

from match_recovery import build_compile_evidence as collector
from test_compile_link_evidence import _archive, build as compile_link_fixture, stamp


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


def configure_archive(observed, *, outside_observation_tree=False, header_input=False):
    root, directory, config, outputs = observed
    receipt_path = directory / "objects/unit.compile.json"
    compilation = json.loads(receipt_path.read_bytes())
    if header_input:
        header = directory / "generated.h"
        header.write_bytes(b"generated header")
        header_stamp = stamp(header.read_bytes())
        compilation["inputs"].append({"path": str(header), "before": header_stamp, "after": header_stamp})
    member = "CMakeFiles/jackal.dir/src/jackal.cpp.obj"
    object_relative = (member if outside_observation_tree else
                       Path(compilation["output"]["path"]).relative_to(directory).as_posix())
    old_output = Path(compilation["output"]["path"])
    if outside_observation_tree:
        output = directory / object_relative
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_bytes(old_output.read_bytes())
        compilation["output"]["path"] = str(output)
    receipt_path.unlink()
    exact_receipt = directory / (object_relative + config["suffix"])
    exact_receipt.parent.mkdir(parents=True, exist_ok=True)
    exact_receipt.write_text(json.dumps(compilation))
    member_label = member.rsplit("/", 1)[-1]
    archive_relative = "jackal.lib"
    archive_path = directory / archive_relative
    archive_path.write_bytes(_archive(member, Path(compilation["output"]["path"]).read_bytes()))
    log = ("Start Pass1\nSearching Libraries\n  Searching " + str(archive_path) + ":\n"
           "End Pass1\nStart Pass2\n  jackal.lib(" + member_label + ")\nEnd Pass2\n")
    (directory / "link.log").write_text(log)
    link_path = directory / "link.inputs.json"
    link = json.loads(link_path.read_bytes())
    link["explicit_inputs"] = [path for path in link["explicit_inputs"] if path != compilation["output"]["path"]]
    link["arguments"] = [argument for argument in link["arguments"] if argument != compilation["output"]["path"]]
    raw_log = log.encode()
    link["link_log_sha256"] = hashlib.sha256(raw_log).hexdigest()
    link["library_searches"] = [{"spelling": str(archive_path), "path": str(archive_path)}]
    archive_stamp = stamp(archive_path.read_bytes())
    link["files"].append({"path": str(archive_path), "before": archive_stamp, "after": archive_stamp})
    link_path.write_text(json.dumps(link))
    config["archive_members"] = [{"archive": archive_relative, "object": object_relative, "member": member}]
    repin(observed, "link.inputs.json")
    repin(observed, "link.log")
    return archive_path, exact_receipt


def test_native_collector_connects_declared_application_outputs(observed):
    report = verify(observed)
    assert report['result'] == 'pass'
    assert len(report['compilations']) == 1
    assert report['linked_selection'] == report['build_graph_verification'] == 'unknown'


@pytest.mark.parametrize('change', ['arguments', 'formatting', 'receipt_path'])
def test_verified_epoch_fingerprint_detects_self_consistent_receipt_replacement(observed, change):
    before = verify(observed)
    path = observed[1] / 'objects/unit.compile.json'
    if change == 'receipt_path':
        path.rename(path.with_name('renamed.compile.json'))
    elif change == 'formatting':
        path.write_bytes(path.read_bytes() + b'\n')
    else:
        value = json.loads(path.read_bytes())
        value['arguments'].append('-DCHANGED')
        path.write_text(json.dumps(value))
    after = verify(observed)
    assert before['result'] == after['result'] == 'pass'
    assert before['evidence_fingerprint'] != after['evidence_fingerprint']
    # Receipt location/membership was previously absent from per-compile proof.
    # A live enclosing build must also retain that exact capture epoch.
    if change == 'receipt_path':
        before.pop('evidence_fingerprint')
        after.pop('evidence_fingerprint')
        assert before == after


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


@pytest.mark.parametrize('damage', ['object', 'generated_header', 'metadata', 'added_receipt', 'executable', 'map', 'archive', 'archive_receipt'])
def test_mutation_during_verification_cannot_publish_stale_correspondence(observed, monkeypatch, damage):
    original = collector.verify_compile_link_inputs
    directory = observed[1]
    if damage in ('archive', 'archive_receipt'):
        configure_archive(observed)
    def mutate(*args, **kwargs):
        report = original(*args, **kwargs)
        assert report['result'] == 'pass'
        if damage == 'added_receipt':
            (directory / 'objects/added.compile.json').write_text('{}')
        elif damage in ('archive', 'archive_receipt'):
            path = directory / ('jackal.lib' if damage == 'archive' else 'unit.obj.compile.json')
            path.write_bytes(path.read_bytes() + b'changed')
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


def test_archive_collector_binds_exact_member_to_its_output_receipt(observed):
    _, receipt = configure_archive(observed)
    report = verify(observed)
    assert report['result'] == report['archive_correspondence'] == 'pass', report
    assert report['archive_member_receipts'][0]['receipt'] == str(receipt)
    assert report['archive_member_receipts'][0]['result'] == 'pass'
    assert report['archive_members'][0]['result'] == 'pass'
    assert report['linked_selection'] == report['input_coverage'] == 'unknown'


def test_archive_receipt_outside_scanned_observation_tree_is_collected_and_sealed(observed):
    _, receipt = configure_archive(observed, outside_observation_tree=True)
    report = verify(observed)
    assert report['result'] == report['archive_correspondence'] == 'pass'
    assert report['archive_member_receipts'][0]['receipt'] == str(receipt)
    assert report['archive_member_receipts'][0]['result'] == 'pass'
    assert len(report['compilations']) == 1


@pytest.mark.parametrize('dependency', ['source', 'header', 'object'])
def test_extra_receipt_dependencies_are_guarded_during_verification(observed, monkeypatch, dependency):
    configure_archive(observed, header_input=True)
    original = collector.verify_compile_link_inputs
    directory = observed[1]
    path = directory / {'source': 'unit.cc', 'header': 'generated.h', 'object': 'unit.obj'}[dependency]
    def mutate(*args, **kwargs):
        report = original(*args, **kwargs)
        assert report['result'] == 'pass'
        path.write_bytes(path.read_bytes() + b'changed during verification')
        return report
    monkeypatch.setattr(collector, 'verify_compile_link_inputs', mutate)
    with pytest.raises(ValueError, match='changed during'):
        verify(observed)


@pytest.mark.parametrize('dependency', ['source', 'header', 'object'])
def test_extra_receipt_dependencies_remain_guarded_after_live_capture(observed, dependency):
    configure_archive(observed, header_input=True)
    root, directory, config, outputs = observed
    with collector.live_build_compilations(root, directory, config, outputs=outputs) as (report, current):
        assert report['result'] == 'pass'
        path = directory / {'source': 'unit.cc', 'header': 'generated.h', 'object': 'unit.obj'}[dependency]
        path.write_bytes(path.read_bytes() + b'changed after capture')
        with pytest.raises(ValueError, match='observed file contents'):
            current()


def test_missing_extra_receipt_appearing_during_verification_changes_membership(observed, monkeypatch):
    _, receipt = configure_archive(observed)
    contents = receipt.read_bytes()
    receipt.unlink()
    original = collector.verify_compile_link_inputs
    def appear(*args, **kwargs):
        report = original(*args, **kwargs)
        assert report['result'] == 'unknown'
        receipt.write_bytes(contents)
        return report
    monkeypatch.setattr(collector, 'verify_compile_link_inputs', appear)
    with pytest.raises(ValueError, match='membership changed during verification'):
        verify(observed)


def test_missing_exact_archive_output_receipt_stays_unknown(observed):
    _, receipt = configure_archive(observed)
    receipt.unlink()
    report = verify(observed)
    assert report['result'] == 'unknown'
    assert report['archive_member_receipts'][0]['result'] == 'unknown'
    assert report['archive_members'][0]['result'] == 'unknown'


def test_archive_receipt_cannot_bind_a_substituted_output_path(observed):
    _, receipt = configure_archive(observed)
    changed = json.loads(receipt.read_bytes())
    substitute = observed[1] / 'substitute.obj'
    substitute.write_bytes(Path(changed['output']['path']).read_bytes())
    changed['output']['path'] = str(substitute)
    receipt.write_text(json.dumps(changed))
    report = verify(observed)
    assert report['result'] != 'pass'
    assert report['archive_member_receipts'][0]['result'] == 'fail'


@pytest.mark.parametrize('damage', ['duplicate_binding', 'member_backslash', 'object_escape'])
def test_archive_configuration_refuses_ambiguous_or_escaping_bindings(observed, damage):
    config = observed[2]
    config['archive_members'] = [{"archive": "jackal.lib", "object": "unit.obj", "member": "unit.obj"}]
    if damage == 'duplicate_binding':
        config['archive_members'].append(dict(config['archive_members'][0]))
    elif damage == 'member_backslash':
        config['archive_members'][0]['member'] = 'dir\\unit.obj'
    else:
        config['archive_members'][0]['object'] = '../unit.obj'
    with pytest.raises(ValueError, match='archive|path|duplicate'):
        verify(observed)


def test_archive_configuration_allows_same_member_in_distinct_archives(observed):
    config = observed[2]
    config['archive_members'] = [
        {"archive": "one.lib", "object": "one.obj", "member": "shared/shared.obj"},
        {"archive": "two.lib", "object": "two.obj", "member": "shared/shared.obj"}]
    normalized = collector.validate_configuration(
        config, [(item['path'], item['format']) for item in observed[3]])
    assert len(normalized['archive_members']) == 2
