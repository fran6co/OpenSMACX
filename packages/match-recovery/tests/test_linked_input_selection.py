"""Physical final families include dependencies outside initializer closure."""

from copy import deepcopy
import json
import struct

import pytest

from match_recovery import linked_input_epoch as epochs
from match_recovery.linked_context import linked_build
from match_recovery.linked_data_ownership import section_producers
from match_recovery.linked_input_selection import observe_input_selection, compare_selection
from test_binary import coff_fixture
from test_coff_inventory import imported
from test_linked_data_ownership import fixture as data_context
from test_linked_modules import fixture, add_archive
from test_generated_selection_residual import install_archive
from test_link_file_evidence import stamp
from test_linked_initializers import update_artifact


def observe(context):
    with linked_build(context) as live:
        return observe_input_selection(context, live.inputs.objects)


def test_compiled_changes_are_separate_from_dependency_family_equality(tmp_path):
    old = data_context(tmp_path / "old")
    raw = bytearray(coff_fixture())
    struct.pack_into("<I", raw, 4, 17)
    new = data_context(tmp_path / "new", raw=bytes(raw))
    before, after = observe(old), observe(new)
    comparison = compare_selection(before, after)
    assert comparison["dependency_families"] == "pass"
    assert [row["unit"] for row in comparison["compiled_changes"]] == ["src/unit0.cpp"]
    assert comparison["dependency_changes"] == []
    assert not comparison["family_sequence_changed"]
    assert comparison["result"] == comparison["retained_section_selection"] == "unknown"


@pytest.mark.parametrize("change", ["bytes", "added", "removed", "reordered"])
def test_unrelated_selected_dependency_member_delta_is_not_hidden(tmp_path, change):
    old, new = fixture(tmp_path / "old"), fixture(tmp_path / "new")
    ordinary = coff_fixture(b"\x90\xc3")
    other = coff_fixture(b"\x91\xc3")
    old_members = [("helper.obj/", ordinary), ("unrelated.obj/", ordinary)]
    new_members = [("helper.obj/", ordinary), ("unrelated.obj/", other if change == "bytes" else ordinary)]
    before_finals = ["helper.obj", "unrelated.obj"] if change != "added" else ["helper.obj"]
    after_finals = ["helper.obj", "unrelated.obj"] if change != "removed" else ["helper.obj"]
    if change == "reordered":
        after_finals.reverse()
    add_archive(old, old_members, before_finals)
    add_archive(new, new_members, after_finals)
    before, after = observe(old), observe(new)
    comparison = compare_selection(before, after)
    assert comparison["dependency_families"] == "fail"
    assert comparison["dependency_order_changed"]
    assert bool(comparison["dependency_changes"]) == (change != "reordered")
    assert comparison["compiled_changes"] == []
    assert comparison["result"] == "unknown"


def test_unselected_member_changes_remain_inventory_delta_not_claimed_selection_delta(tmp_path):
    old, new = fixture(tmp_path / "old"), fixture(tmp_path / "new")
    for context, code in ((old, b"\xc3"), (new, b"\x90\xc3")):
        add_archive(context, [("selected.obj/", coff_fixture()), ("unused.obj/", coff_fixture(code))], ["selected.obj"])
    comparison = compare_selection(observe(old), observe(new))
    assert comparison["dependency_families"] == "pass"
    assert len(comparison["dependency_candidate_changes"]) == 2
    assert comparison["dependency_changes"] == []


@pytest.mark.parametrize("same_bytes", [False, True])
def test_repeated_name_imports_retain_every_occurrence_without_member_selection(tmp_path, same_bytes):
    context = fixture(tmp_path)
    add_archive(context, [("DEMO.dll/", imported()),
                          ("DEMO.dll/", imported() if same_bytes else imported("_Other@8"))], ["DEMO.dll", "DEMO.dll"])
    report = observe(context)
    families = [row for row in report["families"] if row["owner"] == "RUNTIME:DEMO.dll"]
    assert len(families) == 2
    for row in families:
        assert row["selection"] == "unknown" and row["candidate_coverage"] == "pass"
        assert len(row["candidates"]) == 2
        assert {candidate["producer"]["occurrence"] for candidate in row["candidates"]} == {0, 1}
    assert report["selected_input_identity"] == report["retained_section_selection"] == "unknown"
    assert set(report["ambiguous_families"]) == {row["index"] for row in families}
    comparison = compare_selection(report, deepcopy(report))
    assert comparison["dependency_families"] == "pass"  # Equal candidate families, not unique members.
    assert comparison["retained_section_selection"] == "unknown"


def test_shared_epoch_parses_inventory_once_for_selection_and_section_consumers(tmp_path, monkeypatch):
    context = data_context(tmp_path)
    original = epochs._input_inventory
    calls = []
    def counted(*args):
        calls.append(True)
        return original(*args)
    monkeypatch.setattr(epochs, "_input_inventory", counted)
    with linked_build(context) as live:
        with epochs.input_epoch(context, live.inputs.objects) as epoch:
            before = observe_input_selection(context, live.inputs.objects, epoch=epoch)
            with section_producers(context, live.inputs.objects, epoch=epoch) as sections:
                identity = sections.identify("unit0.obj", 1)
                assert sections.resolve(identity) == {"owner": "unit0.obj", "section": 1}
            assert observe_input_selection(context, live.inputs.objects, epoch=epoch) == before
            assert len(calls) == 1
        with pytest.raises(ValueError, match="expired"):
            observe_input_selection(context, live.inputs.objects, epoch=epoch)
        with pytest.raises(ValueError, match="expired"):
            with section_producers(context, live.inputs.objects, epoch=epoch):
                pytest.fail("expired shared epoch was borrowed")


def test_shared_epoch_requires_exact_context_and_objects_not_equal_contents(tmp_path):
    old, new = data_context(tmp_path / "old"), data_context(tmp_path / "new")
    with linked_build(old) as a, linked_build(new) as b:
        with epochs.input_epoch(old, a.inputs.objects) as epoch:
            with pytest.raises(ValueError, match="another context"):
                observe_input_selection(new, b.inputs.objects, epoch=epoch)
            with pytest.raises(ValueError, match="another context"):
                with section_producers(old, b.inputs.objects, epoch=epoch):
                    pytest.fail("foreign objects borrowed epoch")
            with pytest.raises(TypeError):
                epochs.InputEpoch()
            fake = object.__new__(epochs.InputEpoch)
            with pytest.raises(ValueError, match="unrecognized"):
                observe_input_selection(old, a.inputs.objects, epoch=fake)


@pytest.mark.parametrize("damage", ["source", "output", "archive", "report"])
def test_selection_handles_cannot_refresh_mutated_native_inputs(tmp_path, damage):
    context = fixture(tmp_path)
    archive, _ = add_archive(context, [("unrelated.obj/", coff_fixture())], ["unrelated.obj"])
    with pytest.raises(ValueError):
        with linked_build(context) as live:
            with epochs.input_epoch(context, live.inputs.objects) as epoch:
                observe_input_selection(context, live.inputs.objects, epoch=epoch)
                if damage == "source":
                    (context.root / "src/unit.cpp").write_bytes(b"changed source")
                elif damage == "output":
                    context.objects[0].write_bytes(b"changed compiled output")
                elif damage == "archive":
                    archive.write_bytes(b"changed dependency bytes")
                else:
                    context.report["compile_link"]["compilations"][0]["compilation"]["exit_code"] = False
                with pytest.raises(ValueError):
                    observe_input_selection(context, live.inputs.objects, epoch=epoch)


def test_diagnostic_arrays_do_not_mutate_live_native_family_inventory(tmp_path):
    context = fixture(tmp_path)
    add_archive(context, [("unrelated.obj/", coff_fixture())], ["unrelated.obj"])
    with linked_build(context) as live:
        with epochs.input_epoch(context, live.inputs.objects) as epoch:
            first = observe_input_selection(context, live.inputs.objects, epoch=epoch)
            expected = deepcopy(first)
            first["inventory"].clear()
            first["families"][-1]["candidates"][0]["producer"]["member"] = "changed.obj"
            assert observe_input_selection(context, live.inputs.objects, epoch=epoch) == expected


def test_unique_short_import_has_physical_identity_not_retained_section_proof(tmp_path):
    old, new = fixture(tmp_path / "old"), fixture(tmp_path / "new")
    for context, name in ((old, "_First@4"), (new, "_Second@4")):
        add_archive(context, [("DEMO.dll/", imported(name))], ["DEMO.dll"])
    before, after = observe(old), observe(new)
    family, = [row for row in before["families"] if row["owner"] == "RUNTIME:DEMO.dll"]
    assert family["selection"] == "pass"
    assert family["candidates"][0]["format"] == "short_import"
    assert family["candidates"][0]["producer"]["occurrence"] == 0
    assert before["retained_section_selection"] == before["result"] == "unknown"
    assert compare_selection(before, before)["dependency_families"] == "pass"
    comparison = compare_selection(before, after)
    assert comparison["dependency_families"] == "fail"
    assert len(comparison["dependency_candidate_changes"]) == 2


@pytest.mark.parametrize("order", ["compiled_swap", "interleaving"])
def test_full_native_family_sequence_retains_compiled_positions(tmp_path, order):
    old = data_context(tmp_path / "old", damage=install_archive)
    def changed(context):
        install_archive(context)
        log = (context.directory / "link.log").read_text()
        before = "unit0.obj\ndependency.obj\nRUNTIME.lib(sibling.obj)\n"
        assert before in log
        after = ("dependency.obj\nunit0.obj\nRUNTIME.lib(sibling.obj)\n" if order == "compiled_swap"
                 else "unit0.obj\nRUNTIME.lib(sibling.obj)\ndependency.obj\n")
        log = log.replace(before, after)
        evidence = json.loads((context.directory / "link.json").read_bytes())
        evidence["link_log_sha256"] = stamp(log.encode())["sha256"]
        update_artifact(context, "link.log", log.encode())
        update_artifact(context, "link.json", json.dumps(evidence).encode())
    new = data_context(tmp_path / "new", damage=changed)
    comparison = compare_selection(observe(old), observe(new))
    assert comparison["dependency_families"] == "pass"
    assert comparison["dependency_changes"] == comparison["compiled_changes"] == []
    assert not comparison["dependency_order_changed"]
    assert comparison["family_sequence_changed"]
    assert comparison["family_sequence"]["baseline"] != comparison["family_sequence"]["current"]
    assert str(old.directory) not in json.dumps(comparison["family_sequence"])
    assert str(new.directory) not in json.dumps(comparison["family_sequence"])
    assert comparison["retained_section_selection"] == comparison["result"] == "unknown"


@pytest.mark.parametrize("damage", ["empty", "ambiguous_pass", "ambiguous_indices"])
def test_pure_selection_comparison_refuses_inconsistent_family_diagnostics(tmp_path, damage):
    context = fixture(tmp_path)
    add_archive(context, [("DEMO.dll/", imported()), ("DEMO.dll/", imported("_Other@8"))],
                ["DEMO.dll", "DEMO.dll"])
    report = observe(context)
    if damage == "empty":
        report["families"].clear()
    elif damage == "ambiguous_pass":
        report["families"][-1]["selection"] = "pass"
    else:
        report["ambiguous_families"].clear()
    with pytest.raises(ValueError):
        compare_selection(report, report)


@pytest.mark.parametrize("when", ["next_borrow", "final_only"])
@pytest.mark.parametrize("damage", ["compiled", "coverage", "records", "evidence", "root", "directory",
                                    "resolver_index", "resolver_cache", "resolver_file", "memo_raw", "memo_remove"])
def test_shared_derived_state_mutation_refuses_at_next_use_and_final_exit(tmp_path, when, damage):
    context = data_context(tmp_path)
    with linked_build(context) as live:
        with pytest.raises(ValueError):
            with epochs.input_epoch(context, live.inputs.objects) as epoch:
                observe_input_selection(context, live.inputs.objects, epoch=epoch)
                with epochs._borrow_epoch(epoch, context, live.inputs.objects) as state:
                    captured = state
                if damage == "compiled":
                    captured.compiled["src/forged.cpp"] = captured.compiled.pop("src/unit0.cpp")
                elif damage == "coverage":
                    captured.coverage["inputs"].clear()
                elif damage == "records":
                    captured.records[0]["sha256"] = "0" * 64
                elif damage == "evidence":
                    captured.evidence["arguments"].append("/FORCE")
                elif damage in ("root", "directory"):
                    object.__setattr__(captured, damage, tmp_path / "foreign")
                elif damage == "resolver_index":
                    captured.native._constant_records.clear()
                elif damage == "resolver_cache":
                    captured.native._resolved["unit0.obj"]["identity"]["path"] = str(tmp_path / "forged.obj")
                elif damage == "resolver_file":
                    name = next(iter(captured.native._files))
                    captured.native._files[name] += b"changed"
                elif damage == "memo_raw":
                    path = next(iter(captured.inputs.files))
                    raw, stamp = captured.inputs.files[path]
                    captured.inputs.files[path] = raw + b"changed", stamp
                else:
                    captured.inputs.files.clear()
                if when == "next_borrow":
                    with pytest.raises(ValueError):
                        observe_input_selection(context, live.inputs.objects, epoch=epoch)
