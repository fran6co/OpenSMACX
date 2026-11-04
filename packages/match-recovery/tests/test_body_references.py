"""Source body links cannot escape the configured map or borrow a namesake."""

import json

import pytest

from match_recovery.mapping_check import check_mappings
from test_mapping_check import project


def link(root, name):
    path = root / "engine/current/widget.cc"
    lines = path.read_text().splitlines(keepends=True)
    lines.insert(1, f"// body {name}\n")
    path.write_text("".join(lines))


def test_body_reference_uses_exact_configured_file(project):
    link(project, "engine/value.hpp")
    (project / "engine/value.hpp").write_text("inline int f() { return 0; }\n")
    report = check_mappings(project)
    assert report["result"] == "pass"
    body, = report["body_references"]
    assert body['path'] == 'engine/current/widget.cc' and body['body_path'] == 'engine/value.hpp'
    assert body['definition_line'] == body['definition_end_line'] == 1
    assert 'compiler identity/context unproven' in body['evidence']


@pytest.mark.parametrize("name", ["engine/missing.hpp", "../missing.hpp", "/tmp/missing.hpp"])
def test_missing_or_escaping_body_reference_fails(project, name):
    link(project, name)
    report = check_mappings(project)
    assert report["result"] == "fail" and report["findings"][0]["rule"] == "body_reference"


def test_body_reference_without_definition_remains_unknown(project):
    link(project, "engine/value.hpp")
    (project / "engine/value.hpp").write_text("int unrelated();\n")
    report = check_mappings(project)
    assert report["result"] == "unknown"
    assert '0 definitions found' in report['errors'][0]['reason']


@pytest.mark.parametrize('text', ['int f();\n', '// f is defined elsewhere\n'])
def test_declarations_and_comments_are_not_definition_evidence(project, text):
    link(project, 'engine/value.hpp')
    (project / 'engine/value.hpp').write_text(text)
    report = check_mappings(project)
    assert report['result'] == 'unknown'
    assert report['body_references'] == []


def test_uncovered_body_reference_stays_unknown(project):
    link(project, "uncovered/outside.hpp")
    (project / "uncovered").mkdir()
    (project / "uncovered/outside.hpp").write_text("int f();\n")
    report = check_mappings(project)
    assert report["result"] == "unknown" and report["errors"][0]["body_path"] == "uncovered/outside.hpp"


def test_body_witness_uses_emitted_msvc_identifier(project):
    path = project / "recovery/project.json"
    value = json.loads(path.read_text())
    value["source"]["symbol_format"] = "msvc"
    path.write_text(json.dumps(value))
    owner = project / "engine/current/widget.cc"
    owner.write_text(owner.read_text().replace("\nint f", "\n// symbol ?other@Value@@QAEHXZ\nint f"))
    link(project, "engine/value.hpp")
    (project / "engine/value.hpp").write_text("struct Value { int other(); };\nint Value::other() { return 0; }\n")
    report = check_mappings(project)
    assert report["result"] == "pass"
    assert report["body_references"][0]["definition_line"] == 2
