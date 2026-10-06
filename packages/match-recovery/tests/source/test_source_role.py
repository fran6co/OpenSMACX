"""Promotion candidates for packages/match-recovery/tests/source/.

These import the package API normally (no direct parser imports or monkeypatch
of module lookup), so after integration they are ordinary installed-package
regressions rather than packet contract tests.
"""
from pathlib import Path

import pytest

from match_recovery import binding_source
from match_recovery.source import (SourceRole, read_file, read_text, remove,
                                   write, write_symbol)
from match_recovery.source.asm import AsmComparison
from match_recovery.source.recording import record_claims
from match_recovery.source_inventory import inventory
from match_recovery.source import Tier


ANNOTATED = '''/*
Purpose: source-role reader/writer test
// RECOVERY_ROLE: SCAFFOLD manual entry adapter; real owner is separate
// ORIGINAL: 0x00401000 ?adapter@@YAXXZ 0x00401000-0x00401006 BYTE_EXACT SEMANTIC
// RECOVERY_BINDING: v1 {"evidence":["pinned span and caller"],"reason":"Adapter is not the implementation owner."}
// TRIED: preserve this source history
*/
void __cdecl adapter() { return; }
'''


def test_reader_writer_symbol_and_binding_roundtrip(tmp_path):
    path = tmp_path / "adapter.cpp"
    path.write_text(ANNOTATED)
    record, = read_file(path)
    assert record.source_role is SourceRole.SCAFFOLD
    assert record.source_role_reason == "manual entry adapter; real owner is separate"
    assert record.address == 0x401000
    assert record.name == "?adapter@@YAXXZ"
    assert record.image_spans == ((0x401000, 0x401006),)
    assert record.byte_exact and record.semantic and record.region.strip()
    binding = binding_source.parse(ANNOTATED, [record])

    rewritten = write(ANNOTATED, [record])
    reread, = read_text(rewritten, path)
    assert reread.source_role == record.source_role
    assert reread.source_role_reason == record.source_role_reason
    assert reread.address == record.address and reread.name == record.name
    assert reread.image_spans == record.image_spans
    assert reread.byte_exact and reread.semantic
    assert binding_source.parse(rewritten, [reread]) == binding

    changed = write_symbol(ANNOTATED.encode(), record, "?adapter_emitted@@YAXXZ")
    symbol_record, = read_text(changed.decode(), path)
    assert symbol_record.symbol == "?adapter_emitted@@YAXXZ"
    assert symbol_record.source_role == record.source_role
    assert binding_source.parse(changed.decode(), [symbol_record]) == binding

    removed = remove(ANNOTATED, [record])
    assert "RECOVERY_ROLE:" not in removed
    assert "RECOVERY_BINDING:" not in removed
    assert "ORIGINAL:" not in removed


def test_record_claim_update_preserves_role_binding_and_history(tmp_path):
    path = tmp_path / "adapter.cpp"
    path.write_text(ANNOTATED.replace(" BYTE_EXACT SEMANTIC", " SEMANTIC"))
    selected, = read_file(path)
    result = record_claims(
        [selected],
        measure=lambda: [(selected, AsmComparison(verdict=Tier.BYTE_EXACT))],
        validate=lambda: None,
        staging_directory=tmp_path / "staging")
    assert result.exit_code == 0
    reread, = read_file(path)
    assert reread.byte_exact and not reread.semantic
    assert reread.source_role is SourceRole.SCAFFOLD
    assert reread.source_role_reason == selected.source_role_reason
    assert binding_source.parse(path.read_text(), [reread])
    assert "// TRIED: preserve this source history" in path.read_text()


@pytest.mark.parametrize("reason", [
    "", "leading\nsecond line", "line\u2028split", "paragraph\u2029split",
    "control\x01byte", "ORIGINAL: nested mapping", "RECOVERY_ROLE: nested role",
    "close */ enclosing block", "open /* nested block",
])
def test_invalid_role_rationale_fails_before_reader_or_writer_can_embed_it(reason):
    text = ANNOTATED.replace(
        "manual entry adapter; real owner is separate", reason)
    with pytest.raises(ValueError, match="RECOVERY_ROLE|source role"):
        read_text(text, "unsafe.cpp")


def test_multiline_raw_string_does_not_supply_role_or_original_annotations():
    raw = '''const char *payload = R"tag(
// RECOVERY_ROLE: SCAFFOLD spoof inside raw data
// ORIGINAL: 0x00409999 ?fake@@YAXXZ 0x00409999-0x0040999A
)tag";
'''
    actual = ANNOTATED.replace(
        "// RECOVERY_ROLE: SCAFFOLD manual entry adapter; real owner is separate\n",
        "")
    records = read_text(raw + actual, "raw.cpp")
    assert len(records) == 1
    assert records[0].address == 0x401000
    assert records[0].source_role is SourceRole.UNCLASSIFIED
    raw_with_separator = raw.replace("spoof inside raw data", "spoof\u2028 inside raw data")
    records = read_text(raw_with_separator + actual, "raw-separator.cpp")
    assert len(records) == 1 and records[0].source_role is SourceRole.UNCLASSIFIED

    body_raw = ANNOTATED.replace(
        "void __cdecl adapter() { return; }",
        'void __cdecl adapter() {\n'
        '  const char *payload = R"tag(\n'
        '  // RECOVERY_ROLE: SCAFFOLD spoof in body data\n'
        '  // ORIGINAL: 0x00409999 ?fake@@YAXXZ 0x00409999-0x0040999A\n'
        '  )tag";\n  return;\n}')
    body_record, = read_text(body_raw, "raw-body.cpp")
    assert body_record.address == 0x401000
    assert body_record.source_role is SourceRole.SCAFFOLD
    assert "0x00409999" in body_record.region


def test_body_region_presence_is_not_reported_as_implementation_coverage():
    record, = read_text(ANNOTATED, "inventory.cpp")
    assert record.region.strip()
    row = {
        "entity_id": "entity-fixture", "candidate_kind": "source",
        "source_role": record.source_role.value,
        "source_body_region_present": bool(record.region.strip()),
        "legacy_state": record.state.value, "kind_hint": "game",
        "invalid_spans": [], "spans": [[0x401000, 0x401006]],
        "legacy_byte_exact": record.byte_exact,
        "legacy_semantic": record.semantic,
    }
    report = inventory({
        "image_sha256": "0" * 64, "revision": "test", "image_verified": False,
        "files": {"inventory.cpp": {}}, "records": [row],
        "historical_obligations": [{"dimension": "bytes"}],
    })
    # This region includes comment metadata, so it is only a presence fact.
    # Neither it nor the source-derived state becomes recovery coverage.
    assert report["source_body_region_presence_by_role"] == {
        "scaffold": {"present": 1}}
    assert report["source_body_states_by_role"] == {
        "scaffold": {"implemented": 1}}
    assert report["historical_byte_obligations"] == 1
    assert report["historical_semantic_claims"] == 1
    assert "recovered" not in report and "recovery_coverage" not in report
