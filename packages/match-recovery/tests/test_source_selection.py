"""Snapshot selection is metadata-only; selected bodies remain native parses."""

from dataclasses import replace

import pytest

from match_recovery.source import read_file
from match_recovery.source_inventory import source_snapshot
from match_recovery.source_selection import select_records, snapshot_mappings, selected_source_records
from test_recording import project, FIRST


def test_snapshot_projection_selects_the_same_mappings_and_ambiguities(project):
    (project / "seeds/f.cc").write_text(FIRST)
    snapshot = source_snapshot(project)
    parsed = [row for name in snapshot["files"] for row in read_file(project / name)]
    indexed = snapshot_mappings(project, snapshot)
    for targets, host in [(["_f"], None), (["_f", "_f"], project / "engine/family.cc"),
                          (["missing"], None), (["0x00401000", "_h"], project / "engine/family.cc")]:
        try:
            expected = select_records(parsed, targets, host, "msvc")
        except ValueError as error:
            with pytest.raises(ValueError) as actual:
                select_records(indexed, targets, host, "msvc")
            assert str(actual.value) == str(error)
        else:
            chosen = select_records(indexed, targets, host, "msvc")
            assert selected_source_records(project, snapshot, chosen) == expected
    assert not hasattr(indexed[0], "region") and not hasattr(indexed[0], "byte_exact")


def test_selected_hosts_are_parsed_once_in_requested_order_from_snapshot_bytes(project, monkeypatch):
    import match_recovery.source_selection as module
    path = project / "engine/other.cc"
    path.write_text("// ORIGINAL: 0x00401010 _other 0x00401010-0x00401011\nvoid other() {}\n")
    snapshot = source_snapshot(project)
    selected = select_records(snapshot_mappings(project, snapshot), ["_h", "_other", "_f"], None, "msvc")
    original, paths = module.read_text, []

    def counted(text, path):
        paths.append(path)
        return original(text, path)

    monkeypatch.setattr(module, "read_text", counted)
    (project / "engine/family.cc").write_text("// changed AFTER captured source\n")
    rows = selected_source_records(project, snapshot, selected)
    assert [row.name for row in rows] == ["_h", "_other", "_f"]
    assert paths == [project / "engine/family.cc", project / "engine/other.cc"]
    assert "void f()" in rows[-1].region
    # This helper does not establish currentness; callers still check the snapshot.
    from match_recovery.source_inventory import assert_snapshot_current
    with pytest.raises(ValueError, match="source changed"):
        assert_snapshot_current(project, snapshot)


@pytest.mark.parametrize("field", ["address", "path", "line", "catalogue", "emitted", "spans"])
def test_selected_native_parse_must_match_each_indexed_identity_field(project, field):
    snapshot = source_snapshot(project)
    fact = snapshot["records"][0]
    if field == "address": fact["origin"]["address"] += 1
    elif field == "path": fact["path"] = "engine/types.hpp"
    elif field == "line": fact["line"] += 1
    elif field == "catalogue": fact["catalogued_symbol"] = "_forged"
    elif field == "emitted": fact["emitted_symbol_hint"] = "_forged"
    else: fact["spans"][0][1] += 1
    with pytest.raises(ValueError, match="differs from freshly parsed captured source"):
        selected_source_records(project, snapshot, snapshot_mappings(project, snapshot)[:1])


def test_selected_mapping_must_belong_to_the_provided_snapshot(project):
    snapshot = source_snapshot(project)
    mapping = snapshot_mappings(project, snapshot)[0]
    with pytest.raises(ValueError, match="differs from its snapshot index"):
        selected_source_records(project, snapshot, [replace(mapping, name="_forged")])


def test_index_requires_captured_relative_hosts(project):
    snapshot = source_snapshot(project)
    fact = snapshot["records"][0]
    fact["path"] = "engine/missing.cc"
    with pytest.raises(ValueError, match="no captured host"):
        snapshot_mappings(project, snapshot)
    fact["path"] = "../outside.cc"
    with pytest.raises(ValueError):
        snapshot_mappings(project, snapshot)


def test_selected_native_parse_keeps_reader_utf8_replacement_and_extraction_diagnostics(project):
    path = project / "engine/family.cc"
    path.write_bytes(path.read_bytes().replace(b"seed observation", b"seed\xffobservation"))
    snapshot = source_snapshot(project)
    selected = select_records(snapshot_mappings(project, snapshot), ["_f", "_h"], None, "msvc")
    assert selected_source_records(project, snapshot, selected) == read_file(path)
    assert "\ufffd" in selected_source_records(project, snapshot, selected)[0].region


def test_selected_native_parse_retains_failed_extraction(project):
    path = project / "engine/family.cc"
    path.write_text("// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\nnot a C++ function\n")
    snapshot = source_snapshot(project)
    rows = selected_source_records(project, snapshot, snapshot_mappings(project, snapshot))
    assert rows == read_file(path)
    assert rows[0].extract_error
