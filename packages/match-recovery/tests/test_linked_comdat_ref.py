"""Positive native REF events bound to actual physical original code roots."""

import json
import pickle
from pathlib import Path
import struct

import pytest

from match_recovery.linked_comdat_selection import ComdatSelection, comdat_selection
from match_recovery.linked_context import linked_build
from match_recovery.linked_input_epoch import input_epoch
from test_linked_comdat_selection import fixture as any_fixture, object_bytes
from test_linked_initializers import update_artifact
from test_linked_modules import set_link, add_archive
from test_link_file_evidence import stamp
from test_coff_inventory import imported


OPTIONS = ("/OPT:REF", "/INCREMENTAL:NO", "/VERBOSE")


def fixture(root, *, child=True, events=None, target=None, options=OPTIONS):
    context = any_fixture(root, loser=object_bytes(b"\x90\xc3", policy=1, child=child, key=b"_unused")
                          if target is None else target)
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"] = [word for word in evidence["arguments"] if word.lower() != "/incremental:no"] + list(options)
    rows = "Discarded _unused from dependency.obj\n" if events is None else events
    log = (context.directory / "link.log").read_text().replace("End Pass1\n", "End Pass1\n" + rows, 1)
    set_link(context, evidence, log)
    return context


def observe(context):
    with linked_build(context) as native, comdat_selection(native) as selected:
        return selected.discarded("dependency.obj", 1), selected.certificates(), selected.discard_observations()


def test_positive_ref_binds_exact_original_root_and_complete_association_forest(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.discard_observations() is None
        root = selected.discarded("dependency.obj", 1)
        child = selected.discarded("dependency.obj", 2)
        assert root["sections"] == child["sections"] == [1, 2]
        assert root["certificate_id"] == child["certificate_id"]
        assert root["disposition"] == "discarded_unreferenced_comdat"
        certificate, = selected.certificates()
        assert certificate["check"] == "native_comdat_ref_selection" and certificate["result"] == "pass"
        assert certificate["original"]["input"] == {"path": str(context.objects[1])}
        assert certificate["original"]["object_sha256"] == stamp(context.objects[1].read_bytes())["sha256"]
        assert certificate["original"]["root"]["name"] == "_unused"
        assert certificate["original"]["sections"][1]["parent"] == 1
        assert certificate["event"]["owner"]["label"] == "dependency.obj"
        assert certificate["outside_scope_verification"] == certificate["incoming_references"] == "unknown"
        assert certificate["source_authority"] == "not_transferred" and not certificate["ready_for_collection"]
        assert selected.discarded("unit0.obj", 1) is None  # Existing ANY root is not a REF NODUP target.
        assert selected.discard_observations()["row_count"] == 1


def test_missing_event_and_ownerless_diagnostics_never_infer_discard(tmp_path):
    for index, events in enumerate(("", "Discarded _other from dependency.obj\n", "Discarded _unused\n")):
        with linked_build(fixture(tmp_path / str(index), events=events)) as native, comdat_selection(native) as selected:
            assert selected.discarded("dependency.obj", 1) is None
            assert selected.certificates() == []
            assert selected.discard_observations()["selection_verification"] == "unknown"


@pytest.mark.parametrize("options", [(), ("/OPT:REF", "/VERBOSE"), ("/OPT:REF", "/INCREMENTAL:NO"),
    ("/VERBOSE", "/INCREMENTAL:NO"), OPTIONS + ("/OPT:NOREF",), OPTIONS + ("/INCLUDE:_unused",),
    OPTIONS + ("/FORCE:MULTIPLE",), OPTIONS + ("/ALTERNATENAME:_weak=_unused",),
    ("/OPT:REF", "/INCREMENTAL:NO", "/VERBOSE:LIB")])
def test_positive_event_without_supported_actual_native_options_refuses(tmp_path, options):
    with pytest.raises(ValueError):
        observe(fixture(tmp_path / "project", options=options))


def test_repeated_events_are_preserved_but_not_used_as_unique_root_authority(tmp_path):
    context = fixture(tmp_path / "project", events="Discarded _unused from dependency.obj\n" * 2)
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="repeated"):
            selected.discarded("dependency.obj", 1)
        assert selected.discard_observations()["row_count"] == 2
        assert selected.certificates() == []


@pytest.mark.parametrize("owner", ["elsewhere/dependency.obj", "elsewhere\\dependency.obj", "Z:\\elsewhere\\dependency.obj", "./dependency.obj"])
def test_present_owner_qualification_never_falls_back_to_basename(tmp_path, owner):
    with pytest.raises(ValueError, match="qualified"):
        observe(fixture(tmp_path / "project", events=f"Discarded _unused from {owner}\n"))


def test_qualified_final_pass_label_is_not_authenticated_by_same_basename_event(tmp_path):
    context = fixture(tmp_path / "project")
    evidence = json.loads((context.directory / "link.json").read_bytes())
    log = (context.directory / "link.log").read_text().replace("\ndependency.obj\n", "\nelsewhere/dependency.obj\n")
    set_link(context, evidence, log)
    with pytest.raises(ValueError, match="qualified"):
        observe(context)


def test_ambiguous_physical_owner_does_not_borrow_equal_object_hash(tmp_path):
    context = fixture(tmp_path / "project")
    path = context.directory / "other/dependency.obj"
    path.parent.mkdir()
    path.write_bytes(context.objects[1].read_bytes())
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"].append(str(path)); evidence["explicit_inputs"].append(str(path))
    evidence["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
    set_link(context, evidence, (context.directory / "link.log").read_text())
    with pytest.raises(ValueError):
        observe(context)


@pytest.mark.parametrize("kind", ["strong", "weak", "common", "import", "alias", "associated_external"])
def test_conflicting_native_declarations_cannot_be_silently_discarded(tmp_path, kind):
    target = object_bytes(policy=1, child=True, key=b"_unused", weak=kind=="weak", alias=kind=="alias")
    if kind == "associated_external":
        raw = bytearray(target); at, = struct.unpack_from("<I", raw, 8); raw[at + 5 * 18 + 16] = 2; target = bytes(raw)
    context = fixture(tmp_path / "project", target=target)
    if kind in ("strong", "common", "import"):
        raw = (imported("_unused") if kind == "import" else object_bytes(key=b"_unused", policy=1,
               declaration=(4, 0, 2) if kind == "common" else None))
        add_archive(context, [("extra.obj/", raw)], [])
    with pytest.raises(ValueError):
        observe(context)


@pytest.mark.parametrize("symbol", ["_unused", "$tail"])
def test_positive_placement_of_original_root_or_child_contradicts_ref_event(tmp_path, symbol):
    context = fixture(tmp_path / "project")
    text = (context.directory / "app.map").read_text() + f" 0001:00000040 {symbol} 00401040 dependency.obj\n"
    update_artifact(context, "app.map", text.encode())
    with pytest.raises(ValueError, match="MAP"):
        observe(context)


@pytest.mark.parametrize("pdb", [False, True])
@pytest.mark.parametrize("label", ["_unused", ".text", ".text$x"])
def test_icf_event_for_original_ref_root_is_a_contradiction(tmp_path, pdb, label):
    context = fixture(tmp_path / "project")
    evidence = json.loads((context.directory / "link.json").read_bytes())
    log = (context.directory / "link.log").read_text().replace("Start Pass2", (
        f"Selected symbol:\n _f from unit0.obj\nReplaced symbol(s):\n {label} from dependency.obj\n"
        "ICF total savings: 16 bytes\nStart Pass2"))
    set_link(context, evidence, log)
    if not pdb:
        context.report["outputs"] = [row for row in context.report["outputs"] if row["format"] != "msvc-pdb"]
        context.pin_outputs()
    with pytest.raises(ValueError, match="ICF"):
        observe(context)


def test_malformed_log_caught_by_query_still_refuses_final_batch(tmp_path):
    context = fixture(tmp_path / "project", events="Discarded:_unused\n")
    with pytest.raises(ValueError, match="malformed"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            with pytest.raises(ValueError):
                selected.discarded("dependency.obj", 1)
            assert selected.certificates() == []


def test_defensive_reports_and_expired_opaque_handle_do_not_replay_ref_authority(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as native, comdat_selection(native) as selected:
        selected.discarded("dependency.obj", 1)
        selected.discard_observations()["rows"].clear()
        selected.certificates()[0]["original"]["sections"].clear()
        assert selected.discarded("dependency.obj", 2)["sections"] == [1, 2]
        with pytest.raises(TypeError): pickle.dumps(selected)
    with pytest.raises(ValueError, match="expired"): selected.discarded("dependency.obj", 1)
    with pytest.raises(TypeError): ComdatSelection()


@pytest.mark.parametrize("name", ["app.exe", "app.map", "link.log", "link.json", "dependency.obj"])
def test_physical_mutation_cannot_escape_native_ref_batch(tmp_path, name):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError, match="changed"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.discarded("dependency.obj", 1)
            path = context.directory / name; path.write_bytes(path.read_bytes() + b"changed")


@pytest.mark.parametrize("field", ["discard_observation", "discard_events", "ref_certificates", "ref_refusals"])
def test_derived_ref_observation_and_capability_caches_remain_final_guarded(tmp_path, field):
    from match_recovery.linked_comdat_selection import _entry
    with pytest.raises(ValueError, match="changed"):
        with linked_build(fixture(tmp_path / "project")) as native, comdat_selection(native) as selected:
            selected.discarded("dependency.obj", 1)
            state = _entry(selected)
            if field == "ref_refusals": state[field]["forged"] = "not a native refusal"
            else: state[field].clear()


def archive_fixture(root, *, members=None, event="Discarded _unused from RUNTIME.lib(part.obj)\n"):
    # The original dependency has an unrelated key; only the explicitly loaded
    # physical archive candidate supplies the requested NODUP root.
    context = fixture(root, events=event, target=object_bytes(key=b"_other", policy=1))
    body = object_bytes(key=b"_unused", policy=1, child=True)
    path, _ = add_archive(context, [("part.obj/", body)] if members is None else members, ["part.obj"])
    return context, path


def test_archive_ref_retains_exact_physical_member_occurrence(tmp_path):
    context, path = archive_fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.discarded("RUNTIME:part.obj", 2)["sections"] == [1, 2]
        certificate, = selected.certificates()
        assert certificate["original"]["input"] == {"path": str(path), "member": "part.obj", "occurrence": 0}


@pytest.mark.parametrize("event", ["Discarded _unused from wrong/RUNTIME.lib(part.obj)\n",
                                  "Discarded _unused from RUNTIME.lib(wrong/part.obj)\n"])
def test_qualified_archive_or_member_never_borrows_bare_selected_owner(tmp_path, event):
    context, _ = archive_fixture(tmp_path / "project", event=event)
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="qualified"):
            selected.discarded("RUNTIME:part.obj", 1)


@pytest.mark.parametrize("names", [("part.obj/", "part.obj/"), ("a/part.obj/", "b/part.obj/")])
def test_repeated_equal_hash_archive_candidates_cannot_be_chosen_by_discard_label(tmp_path, names):
    body = object_bytes(key=b"_unused", policy=1)
    context, _ = archive_fixture(tmp_path / "project", members=[(name, body) for name in names])
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError):
            selected.discarded("RUNTIME:part.obj", 1)


def test_epoch_from_other_native_build_cannot_bind_ref_events(tmp_path):
    with linked_build(fixture(tmp_path / "first")) as first, linked_build(fixture(tmp_path / "second")) as second:
        with input_epoch(first.context, first.inputs.objects) as epoch:
            with pytest.raises(ValueError, match="another context"):
                with comdat_selection(second, epoch=epoch):
                    pass


def test_retained_incoming_reference_is_still_a_graph_obligation_after_discard_query(tmp_path):
    context = fixture(tmp_path / "project")
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
        if row["path"] == str(context.objects[0]): row["before"] = row["after"] = stamp(caller)
    set_link(context, evidence, (context.directory / "link.log").read_text())
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x210] = b"\xe8" + struct.pack("<i", 0x3B) + b"\xc3" + b"\xcc" * 10
    update_artifact(context, "app.exe", bytes(image))
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.discarded("dependency.obj", 1)["sections"] == [1, 2]
        graph = native.code([{"owner": "unit0.obj", "symbol": "_f"}])
        assert graph["result"] != "pass" and graph["gaps"]
        assert graph["gaps"][0]["owner"] == "unit0.obj" and graph["gaps"][0]["symbol"] == "_f"
        assert "reference has no unique independent map placement" in graph["gaps"][0]["reason"]
        assert selected.certificates()[0]["incoming_references"] == "unknown"


def test_many_ref_queries_parse_and_bind_once_without_rehashing_native_epoch(tmp_path, monkeypatch):
    from match_recovery import linked_comdat_selection as selection
    real = selection._ref_certificate_uncached
    calls = []
    def observed(*args):
        calls.append(args[3].name)
        return real(*args)
    monkeypatch.setattr(selection, "_ref_certificate_uncached", observed)
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        first = selected.discarded("dependency.obj", 1)
        checks = context.checks
        for _ in range(50):
            assert selected.discarded("dependency.obj", 1) == first
            assert selected.discarded("dependency.obj", 2)["certificate_id"] == first["certificate_id"]
        assert calls == ["_unused"] and context.checks == checks


def test_code_independent_data_merge_does_not_claim_data_preservation(tmp_path):
    result, certificates, _ = observe(fixture(tmp_path / "project", options=OPTIONS + ("/MERGE:.CRT=.data",)))
    assert result["disposition"] == "discarded_unreferenced_comdat"
    assert certificates[0]["noncode_merges"][0]["data_preservation"] == "unknown"
    with pytest.raises(ValueError, match="code"):
        observe(fixture(tmp_path / "bad", options=OPTIONS + ("/MERGE:.text=.data",)))


def test_ref_positive_removal_does_not_require_a_winner_or_pdb_placement(tmp_path):
    context = fixture(tmp_path / "project")
    context.report["outputs"] = [row for row in context.report["outputs"] if row["format"] != "msvc-pdb"]
    context.pin_outputs()
    assert observe(context)[0]["disposition"] == "discarded_unreferenced_comdat"


def test_clearing_a_cached_malformed_transcript_cannot_erase_the_final_refusal(tmp_path):
    from match_recovery.linked_comdat_selection import _entry
    with pytest.raises(ValueError, match="changed"):
        with linked_build(fixture(tmp_path / "project", events="Discarded:_unused\n")) as native, comdat_selection(native) as selected:
            with pytest.raises(ValueError): selected.discarded("dependency.obj", 1)
            _entry(selected)["discard_failure"] = None


def test_discard_query_cannot_erase_adjacent_or_out_of_family_sections(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="outside"):
            selected.discarded("dependency.obj", 3)
        with pytest.raises(ValueError): selected.discarded("dependency.obj", True)
        assert selected.discarded("unit0.obj", 1) is None
        assert selected.certificates() == []


@pytest.mark.parametrize("retention", ["none", "include", "map"])
def test_weak_alias_to_private_child_cannot_escape_root_key_conflict_checks(tmp_path, retention):
    raw = bytearray(object_bytes(policy=1, child=True, weak=True, key=b"_unused"))
    symbols, = struct.unpack_from("<I", raw, 8)
    # The weak symbol at index 6 falls back to the private child label at 5,
    # not the external root at 2. Its auxiliary occupies original index 7.
    struct.pack_into("<I", raw, symbols + 7 * 18, 5)
    context = fixture(tmp_path / "project", target=bytes(raw),
                      options=OPTIONS + (("/INCLUDE:_weak",) if retention == "include" else ()))
    if retention == "map":
        text = (context.directory / "app.map").read_text()
        update_artifact(context, "app.map", (text + " 0001:00000040 _weak 00401040 dependency.obj\n").encode())
    with pytest.raises(ValueError, match="weak fallback"):
        observe(context)
