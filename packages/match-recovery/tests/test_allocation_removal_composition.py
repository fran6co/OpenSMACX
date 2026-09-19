"""Positive native removal narrows allocations, never physical obligations."""

from contextlib import contextmanager
import json
import struct

import pytest

from match_recovery.linked_context import linked_build
from test_linked_comdat_selection import fixture as any_fixture, object_bytes
from test_linked_comdat_ref import fixture as ref_fixture, archive_fixture, OPTIONS
from test_linked_initializers import update_artifact
from test_linked_modules import set_link, add_archive
from test_link_file_evidence import stamp


def observe(context):
    with linked_build(context) as native:
        return native.ordinary_allocations()


def rows(report, context, index=1):
    return [row for row in report["sections"] if row["input"] == {"path": str(context.objects[index])}]


@pytest.mark.parametrize("kind", ["any", "ref"])
@pytest.mark.parametrize("child_kind", ["code", "debug", "data"])
def test_certified_family_keeps_all_original_rows_but_needs_no_final_allocation(tmp_path, kind, child_kind):
    raw = bytearray(object_bytes(b"\x90\xc3", child=True,
                                policy=2 if kind == "any" else 1,
                                key=b"_f" if kind == "any" else b"_unused"))
    if child_kind != "code":
        raw[60:68] = b".debug$F" if child_kind == "debug" else b".rdata$x"
        symbols, = struct.unpack_from("<I", raw, 8)
        raw[symbols + 3 * 18:symbols + 3 * 18 + 8] = raw[60:68]
        struct.pack_into("<I", raw, 60 + 36, 0x42501840 if child_kind == "debug" else 0x40501040)
    context = (any_fixture(tmp_path, loser=bytes(raw)) if kind == "any"
               else ref_fixture(tmp_path, target=bytes(raw)))
    report = observe(context)
    removed = rows(report, context)
    assert len(removed) == 2 and all(row["size"] == 16 for row in removed)
    assert all(row["nonoccupying"] == row["retention"] == "pass" for row in removed)
    assert {row["disposition"] for row in removed} == {
        "discarded_any_duplicate" if kind == "any" else "discarded_unreferenced_comdat"}
    assert len({row["selection_certificate"] for row in removed}) == 1
    assert all("envelope" not in row and "contribution" not in row for row in removed)
    assert not [gap for gap in report["gaps"] if gap.get("input") == removed[0]["input"]]
    assert [(node["owner"], node["symbol"]) for node in report["code"]["nodes"]] == [("unit0.obj", "_f")]
    assert report["comdat_removal_certificates"]
    assert report["result"] == report["allocation_domain"] == "pass"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("options", [
    {"events": ""}, {"events": "Discarded _unused\n"},
    {"events": "Discarded _unused from wrong.obj\n"},
    {"events": "Discarded _unused from dependency.obj\n" * 2},
    {"options": (*OPTIONS, "/INCLUDE:_unused")},
    {"options": (*OPTIONS, "/OPT:NOREF")},
])
def test_missing_ambiguous_or_forced_ref_evidence_preserves_original_roots(tmp_path, options):
    context = ref_fixture(tmp_path, **options)
    report = observe(context)
    assert all("nonoccupying" not in row for row in rows(report, context))
    assert any(node["owner"] == "dependency.obj" and node["symbol"] == "_unused"
               for node in report["code"]["nodes"])
    assert report["result"] != "pass"


@pytest.mark.parametrize("options", [
    {"corrupt": True}, {"sc": False}, {"wrong_module": True},
    {"loser": object_bytes(alias=True)}, {"loser": object_bytes(weak=True)},
])
def test_unqualified_any_winner_or_competitor_preserves_loser_obligations(tmp_path, options):
    context = any_fixture(tmp_path, **options)
    report = observe(context)
    assert all("nonoccupying" not in row for row in rows(report, context))
    assert any(node["owner"] == "dependency.obj" for node in report["code"]["nodes"])
    assert report["result"] != "pass"


def test_map_contradiction_cannot_be_removed(tmp_path):
    context = ref_fixture(tmp_path)
    mapping = (context.directory / "app.map").read_bytes().replace(
        b" Static symbols\n", b" Static symbols\n 0001:00000040 $tail 00401040 dependency.obj\n")
    update_artifact(context, "app.map", mapping)
    report = observe(context)
    assert all("nonoccupying" not in row for row in rows(report, context))
    assert any("contradictory native MAP" in row.get("removal_refusal", "")
               for row in rows(report, context))


def test_ref_removal_does_not_hide_retained_code_damage(tmp_path):
    context = ref_fixture(tmp_path)
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200] ^= 1
    update_artifact(context, "app.exe", bytes(image))
    report = observe(context)
    assert all(row["nonoccupying"] == "pass" for row in rows(report, context))
    assert report["code"]["result"] == report["result"] == "fail"


@pytest.mark.parametrize("ambiguous", [False, True])
def test_archive_removal_requires_exact_member_occurrence(tmp_path, ambiguous):
    body = object_bytes(key=b"_unused", policy=1, child=True)
    members = [("part.obj/", body)] * (2 if ambiguous else 1)
    context, archive = archive_fixture(tmp_path, members=members)
    report = observe(context)
    targets = [row for row in report["sections"] if row["input"]["path"] == str(archive)]
    assert len(targets) == len(members) * 2
    assert {row["input"]["occurrence"] for row in targets} == set(range(len(members)))
    assert all((row.get("disposition") == "discarded_unreferenced_comdat") is not ambiguous
               for row in targets)
    if ambiguous:
        assert all("nonoccupying" not in row for row in targets)
        assert report["result"] != "pass"


@pytest.mark.parametrize("kind", ["strong", "common", "import"])
def test_unselected_archive_declarations_still_prevent_ref_removal(tmp_path, kind):
    from test_coff_inventory import imported
    context = ref_fixture(tmp_path)
    competitor = (imported("_unused") if kind == "import" else object_bytes(
        key=b"_unused", policy=1, declaration=(4, 0, 2) if kind == "common" else None))
    add_archive(context, [("extra.obj/", competitor)], [])
    report = observe(context)
    assert all("nonoccupying" not in row for row in rows(report, context))
    assert report["result"] != "pass"


def test_removed_root_still_has_a_retained_incoming_reference_obligation(tmp_path):
    context = ref_fixture(tmp_path)
    caller = bytearray(object_bytes(b"\xe8\0\0\0\0\xc3"))
    at, count = struct.unpack_from("<II", caller, 8)
    caller[at:at] = struct.pack("<IIH", 1, 3, 20)
    struct.pack_into("<II", caller, 8, at + 10, count + 1)
    struct.pack_into("<I", caller, 20 + 24, at)
    struct.pack_into("<H", caller, 20 + 32, 1)
    struct.pack_into("<H", caller, at + 10 + 18 + 4, 1)
    end = at + 10 + count * 18
    caller[end:end] = b"_unused\0" + struct.pack("<IhHBB", 0, 0, 0x20, 2, 0)
    context.objects[0].write_bytes(caller)
    context.report["compile_link"]["compilations"][0]["compilation"]["output"]["after"] = stamp(caller)
    evidence = json.loads((context.directory / "link.json").read_bytes())
    for row in evidence["files"]:
        if row["path"] == str(context.objects[0]):
            row["before"] = row["after"] = stamp(caller)
    set_link(context, evidence, (context.directory / "link.log").read_text())
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x210] = b"\xe8" + struct.pack("<i", 0x3B) + b"\xc3" + b"\xcc" * 10
    update_artifact(context, "app.exe", bytes(image))
    report = observe(context)
    assert all(row["nonoccupying"] == "pass" for row in rows(report, context))
    assert not any(node["owner"] == "dependency.obj" for node in report["code"]["nodes"])
    assert any("reference has no unique independent map placement" in gap["reason"]
               for gap in report["code"]["gaps"])
    assert report["result"] != "pass"


def test_removed_copies_do_not_explain_reserved_flags_or_unmatched_storage(tmp_path):
    from test_ordinary_contributions import set_contributions
    from test_pdb_contributions import contribution
    context = any_fixture(tmp_path)
    set_contributions(context, contribution(section=1, offset=0, size=16, flags=0x60503020)
                      + contribution(section=1, offset=0x60, size=16, flags=0x60500020))
    report = observe(context)
    assert all(row["nonoccupying"] == "pass" for row in rows(report, context))
    assert rows(report, context, 0)[0]["characteristics_agreement"] == "unknown"
    assert report["unmatched_contributions"]
    assert report["result"] == "unknown"


def test_selection_final_only_refusal_escapes_before_code_consumption(tmp_path, monkeypatch):
    from match_recovery import linked_comdat_selection as selection, application_code_roots
    original = selection.comdat_selection

    @contextmanager
    def final_only(native, **kwargs):
        with original(native, **kwargs) as handle:
            yield handle
        raise ValueError("selection final-only refusal")

    def unexpected(*args, **kwargs):
        pytest.fail("code roots consumed before selection finalization")

    monkeypatch.setattr(selection, "comdat_selection", final_only)
    monkeypatch.setattr(application_code_roots, "combine_roots", unexpected)
    with pytest.raises(ValueError, match="selection final-only refusal"):
        observe(ref_fixture(tmp_path))


def test_changed_native_inputs_cannot_escape_after_a_successful_removal(tmp_path, monkeypatch):
    from match_recovery.linked_comdat_selection import ComdatSelection
    context = ref_fixture(tmp_path)
    original = ComdatSelection.discarded
    changed = False

    def corrupt(handle, owner, section):
        nonlocal changed
        result = original(handle, owner, section)
        if result is not None and not changed:
            changed = True
            path = context.objects[1]
            path.write_bytes(path.read_bytes() + b"changed after removal proof")
        return result

    monkeypatch.setattr(ComdatSelection, "discarded", corrupt)
    with pytest.raises(ValueError, match="changed"):
        observe(context)


def test_diagnostic_certificates_do_not_authorize_another_observation(tmp_path):
    context = ref_fixture(tmp_path)
    with linked_build(context) as native:
        report = native.ordinary_allocations()
        report["comdat_removal_certificates"].clear()
        for row in rows(report, context):
            row["size"] = 0
        fresh = native.ordinary_allocations()
        assert fresh["comdat_removal_certificates"]
        assert all(row["size"] == 16 for row in rows(fresh, context))
