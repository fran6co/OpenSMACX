"""Independent physical census completeness and live-epoch refusal controls."""

from copy import deepcopy
import json
import struct

import pytest

from match_recovery import linked_context, linked_storage_census
from match_recovery.linked_objects import SelectedObjects
from test_coff_inventory import imported
from test_link_file_evidence import stamp
from test_linked_constants import constant, symbol_object
from test_linked_objects import inventory
from test_linked_storage import OWNER, data_object, fixture
from test_linked_storage_controls import mutate, native_context, relocated


def configured(root, *, final="private.obj", extra=(), raw=None, explicit=()):
    value = fixture(root, raw=raw, extra=extra)
    evidence = deepcopy(value.evidence)
    for name, contents in explicit:
        path = root / name
        path.write_bytes(contents)
        evidence["explicit_inputs"].append(str(path))
        evidence["arguments"].append(str(path))
        evidence["files"].append({"path": str(path), "before": stamp(contents), "after": stamp(contents)})
    labels = [final] if isinstance(final, str) else final
    log = (f"Start Pass1\n Searching {root / 'runtime.lib'}:\nEnd Pass1\nStart Pass2\napp.obj\n"
           + "".join(f"RUNTIME.lib({name})\n" for name in labels) + "End Pass2\n")
    evidence["link_log_sha256"] = stamp(log.encode())["sha256"]
    value.evidence, value.coverage, value.records = inventory(evidence, log)
    value.objects = SelectedObjects(value.evidence, value.coverage, value.records)
    return value


def rows(report, member):
    return [row for row in report["inputs"] if row["input"].get("member") == member]


def pending(report):
    assert report["result"] == report["retained_data_coverage"] == report["allocation_isolation"] == "unknown"
    assert not report["ready_for_collection"] and not report["persisted"]


def test_unselected_archive_data_is_counted_but_never_given_selected_placement(tmp_path):
    value = configured(tmp_path, extra=(("unused.obj/", data_object()),))
    report = linked_storage_census.observe_storage(value)
    assert report["enumeration"] == "pass"
    selected, = rows(report, "private.obj")
    unused, = rows(report, "unused.obj")
    assert selected["participation"] == "final_pass"
    assert unused["participation"] == "archive_candidate_only"
    assert selected["sections"][0]["placement"] == "pass"
    assert unused["sections"][0]["size"] == 8
    assert unused["sections"][0]["placement"] == "unknown"
    assert unused["owners"] == []
    pending(report)


def test_duplicate_final_labels_do_not_grant_unique_physical_selection(tmp_path):
    report = linked_storage_census.observe_storage(configured(tmp_path, final=("private.obj", "private.obj")))
    row, = rows(report, "private.obj")
    assert row["sections"][0]["placement"] == "unknown"
    assert report["gaps"]
    pending(report)


@pytest.mark.parametrize("raw", [data_object(), constant(32, name="_g", section=0), imported(kind=1)])
def test_explicit_objects_without_final_labels_cannot_disappear(tmp_path, raw):
    report = linked_storage_census.observe_storage(configured(tmp_path, explicit=(("unlisted.obj", raw),)))
    row, = [row for row in report["inputs"] if row["input"]["path"] == str(tmp_path / "unlisted.obj")]
    assert row["participation"] == "explicit_unobserved" and row["owners"] == []
    assert report["gaps"]
    if raw == data_object():
        assert row["sections"][0]["size"] == 8 and row["sections"][0]["placement"] == "unknown"
    elif raw == imported(kind=1):
        assert row["format"] == "short_import" and row["sections"] == []
        assert row["imports"][0]["type"] == "data"
    else:
        assert row["sections"] == []
        assert row["common_requests"][0]["value"] == 32
    pending(report)


@pytest.mark.parametrize("mixed", [False, True])
def test_ambiguous_import_members_keep_every_physical_candidate_and_storage_obligation(tmp_path, mixed):
    extra = [("DEMO.dll/", imported()), ("DEMO.dll/", data_object() if mixed else imported("_Other@8"))]
    value = configured(tmp_path, final=("private.obj", "DEMO.dll", "DEMO.dll"), extra=extra)
    report = linked_storage_census.observe_storage(value)
    candidates = rows(report, "DEMO.dll")
    assert len(candidates) == 2 and {row["input"]["occurrence"] for row in candidates} == {0, 1}
    assert all(row["participation"] == "final_pass" for row in candidates)
    assert report["gaps"]
    assert sum(len(row["imports"]) for row in candidates) == (1 if mixed else 2)
    if mixed:
        data, = [row for row in candidates if row["format"] == "coff"]
        assert data["sections"][0]["size"] == 8 and data["sections"][0]["placement"] == "unknown"
    pending(report)


@pytest.mark.parametrize("name,flags", [(b".debug$S", 0xC0300040), (b".debug$T", 0x42300040),
                                       (b".data", 0xC2300040), (b".data", 0xC0301040)])
def test_debug_spelling_discardable_or_comdat_flags_never_erase_storage(tmp_path, name, flags):
    value = configured(tmp_path, raw=data_object(name=name, flags=flags))
    report = linked_storage_census.observe_storage(value)
    row, = rows(report, "private.obj")
    section, = row["sections"]
    assert section["size"] == 8 and section["flags"] == flags
    assert section["section_name"] == name.decode()
    assert section["placement"] == "unknown"
    assert report["gaps"]
    pending(report)


@pytest.mark.parametrize("runtime_flags", [False, True])
def test_directives_remain_visible_linker_effects_even_when_the_name_is_special(tmp_path, runtime_flags):
    raw = bytearray(symbol_object([], directives=b" /FORCE "))
    if runtime_flags:
        struct.pack_into("<I", raw, 56, 0xC0300040)
    report = linked_storage_census.observe_storage(configured(tmp_path, raw=bytes(raw)))
    row, = rows(report, "private.obj")
    assert row["directives"][0]["text"] == " /FORCE "
    assert row["sections"][0]["section_name"] == ".drectve"
    assert row["sections"][0]["placement"] == "unknown"
    assert report["gaps"]
    pending(report)


def test_zero_size_section_with_a_write_is_retained_as_malformed_not_empty(tmp_path):
    raw = bytearray(relocated(((0, 6),)))
    struct.pack_into("<I", raw, 36, 0)
    report = linked_storage_census.observe_storage(configured(tmp_path, raw=bytes(raw)))
    row, = rows(report, "private.obj")
    assert row["sections"][0]["size"] == 0
    assert row["sections"][0]["placement"] == "unknown"
    assert report["gaps"]
    pending(report)


def test_forged_candidate_removal_cannot_shrink_native_enumeration(tmp_path):
    value = configured(tmp_path, extra=(("unused.obj/", data_object()),))
    value.objects._constant_records = [row for row in value.objects._constant_records if row.get("member") != "unused.obj"]
    with pytest.raises(ValueError): linked_storage_census.observe_storage(value)


@pytest.mark.parametrize("limit", [True, False, 0, -1, 1000001, "100", None])
def test_section_budget_requires_an_exact_bounded_integer(tmp_path, limit):
    value = configured(tmp_path)
    with pytest.raises(ValueError, match="bounded section limit"):
        linked_storage_census.observe_storage(value, max_sections=limit)


def test_exhausted_budget_never_returns_a_partial_successful_census(tmp_path):
    context, _ = native_context(tmp_path / "project")
    with linked_context.linked_build(context) as live:
        with pytest.raises(ValueError, match="section resource bound"):
            live.storage_census(max_sections=1)
        report = live.storage_census()
        assert report["enumeration"] == "pass" and report["section_count"] > 1
        pending(report)


def test_native_census_defensive_output_and_expiry(tmp_path):
    context, _ = native_context(tmp_path / "project")
    with linked_context.linked_build(context) as live:
        report = live.storage_census()
        assert report["enumeration"] == "pass"
        saved = deepcopy(report)
        report["inputs"].clear()
        report["gaps"].clear()
        report["section_envelopes"].clear()
        report["result"] = "forged"
        assert live.storage_census() == saved
        pending(saved)
    assert context.alive
    with pytest.raises(ValueError, match="expired"): live.storage_census()


@pytest.mark.parametrize("damage", ["archive", "object", "tool", "image", "map", "log", "receipt", "expired",
                                    "parsed_image", "parsed_map", "evidence", "records", "cache"])
@pytest.mark.parametrize("at_close", [False, True])
def test_native_census_mutations_refuse_during_use_and_close(tmp_path, damage, at_close):
    context, _ = native_context(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            assert live.storage_census()["enumeration"] == "pass"
            mutate(context, live, damage)
            if not at_close:
                with pytest.raises(ValueError): live.storage_census()


@pytest.mark.parametrize("damage", ["archive", "image", "records", "cache", "expired"])
def test_native_final_guard_refuses_mutation_inside_census(tmp_path, monkeypatch, damage):
    context, _ = native_context(tmp_path / "project")
    original = linked_storage_census.observe_storage
    with pytest.raises(ValueError):
        with linked_context.linked_build(context) as live:
            def altered(*args, **kwargs):
                report = original(*args, **kwargs)
                assert report["enumeration"] == "pass"
                mutate(context, live, damage)
                return report
            monkeypatch.setattr(linked_storage_census, "observe_storage", altered)
            with pytest.raises(ValueError): live.storage_census()


def test_census_exception_still_expires_and_runs_final_guard(tmp_path, monkeypatch):
    context, _ = native_context(tmp_path / "project")
    with pytest.raises(ValueError, match="expired"):
        with linked_context.linked_build(context) as live:
            def refused(*args, **kwargs):
                context.alive = False
                raise ValueError("invalid census")
            monkeypatch.setattr(linked_storage_census, "observe_storage", refused)
            with pytest.raises(ValueError, match="expired"): live.storage_census()
