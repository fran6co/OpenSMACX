"""Retained native file evidence cannot acquire a new self-consistent epoch."""

import json
from pathlib import Path

import pytest

from match_recovery import application_context as module
from match_recovery.build_compile_evidence import verify_build_compilations
from test_application_context import configured, context
from test_build_check import project, write_config
from test_build_compile_evidence import observed as observations_fixture
from test_compile_link_evidence import build as files_fixture
from test_compile_file_evidence import stamp


@pytest.fixture
def native_context(configured, monkeypatch):
    configuration = {'database': 'commands.json', 'source_extensions': ['.cc'], 'directory': 'objects',
                     'suffix': '.compile.json', 'link_files': 'link.inputs.json', 'link_log': 'link.log'}
    outputs = [{'path': name, 'format': kind} for name, kind in (
        ('commands.json', 'compilation-database'), ('link.inputs.json', 'link-input-files'),
        ('link.log', 'msvc-link-log'), ('application', 'file'), ('application.map', 'file'))]
    write_config(configured, compile_link=configuration, outputs=outputs)

    def build(root, settings, *, retain):
        assert retain is True
        directory = root / settings.artifacts / 'synthetic-build'
        directory.mkdir(parents=True)
        _, _, policy, artifacts = observations_fixture.__wrapped__(directory, files_fixture.__wrapped__(directory))
        assert policy == settings.compile_link
        # Real files/native collector; only the compiler/build process is a
        # fixture. No synthetic result is used for correspondence or freshness.
        proof = verify_build_compilations(root, directory, policy, outputs=artifacts)
        assert proof['result'] == 'pass'
        return {'result': 'pass', 'directory': directory.relative_to(root).as_posix(),
                'outputs': artifacts, 'compile_link': proof}

    monkeypatch.setattr(module, 'generate_build', build)
    return configured


def test_native_evidence_remains_available_and_fresh_through_context(native_context):
    with context(native_context) as application:
        assert application.report['compile_link']['result'] == 'pass'
        application.check_current()


@pytest.mark.parametrize('damage', ['receipt_arguments', 'receipt_location', 'coherent_header'])
def test_self_consistent_new_evidence_cannot_refresh_live_context(native_context, damage):
    with pytest.raises(ValueError, match='evidence changed'):
        with context(native_context) as application:
            receipt = application.directory / 'objects/unit.compile.json'
            if damage == 'receipt_location':
                receipt.rename(receipt.with_name('other.compile.json'))
            else:
                value = json.loads(receipt.read_bytes())
                if damage == 'receipt_arguments':
                    value['arguments'].append('-DCHANGED')
                else:
                    header = application.directory / 'header.h'
                    header.write_bytes(header.read_bytes() + b'\nchanged header\n')
                    row = next(item for item in value['inputs'] if Path(item['path']) == header)
                    row['before'] = row['after'] = stamp(header.read_bytes())
                receipt.write_text(json.dumps(value))
            settings = module.Build.read(application.root)
            refreshed = verify_build_compilations(application.root, application.directory,
                                                  settings.compile_link, outputs=application.report['outputs'])
            assert refreshed['result'] == 'pass'  # A fresh PASS is not the original epoch.
            assert refreshed['evidence_fingerprint'] != application.report['compile_link']['evidence_fingerprint']
            application.check_current()
