"""Real import thunk contributions, not full application acceptance."""

from match_recovery.application_context import application_build
from match_recovery.linked_context import linked_build
from match_recovery.linked_contributions import ordinary_contributions
from match_recovery.linked_import_thunks import verify_import_thunk
from match_recovery.linker_generated_storage import assign_linker_contributions
from match_recovery.source_replica import replica_snapshot
from test_native_acceptance_compiler import prepared_project


def test_native_import_thunk_cannot_qualify_unrelated_linker_metadata(tmp_path, monkeypatch):
    root, _, _, _, _ = prepared_project(tmp_path, monkeypatch, proofs=False)
    entry = root / "src/entry.cpp"
    source = entry.read_text()
    assert source.count("__declspec(dllimport)") == 1
    # Test-owned source requests a direct import call, so LINK must emit a
    # thunk. This does not change product source, compiler flags or baselines.
    entry.write_text(source.replace("__declspec(dllimport) ", ""))
    files = {name: raw for name, raw in replica_snapshot(root, ("src",)).items() if raw is not None}
    with application_build(root, ("src",), files, versioned_files={}, retain=True) as context:
        assert context.report["result"] == "pass"
        with linked_build(context) as native:
            joined = ordinary_contributions(context, native.inputs)
            report = assign_linker_contributions(joined["contributions"], joined["modules"]["modules"],
                                                 native.inputs.image)
            # The real linker gives this thunk to its import-library module,
            # NOT to the synthetic debug-directory contributor. Verify that
            # positive separately through its full physical-producer route.
            label, = [row for row in native.inputs.mapping.publics if row.symbol == "_ExitProcess@4"]
            thunk = verify_import_thunk(native.inputs.objects, native.inputs.mapping,
                                        native.inputs.image, label.owner, label.symbol)
            assert thunk["result"] == thunk["proof"]["references"] == "pass"
            assert thunk["import"]["name"] == "ExitProcess"
            overlaps = [row for row in joined["contributions"]
                        if row["address"] <= thunk["address"] < row["address"] + row["size"]]
            assert len(overlaps) == 1, overlaps
            module = joined["modules"]["modules"][overlaps[0]["module"]]
            assert module["name"] != "* Linker *" and module["object_name"]
            assert report["records"], report
            assert report["assigned"] == 0 and report["result"] == "unknown", report
            assert all(row["assignment"] is None for row in report["records"])
            assert not report["ready_for_collection"]
            allocations = native.ordinary_allocations()
            assert allocations["generated_storage"] == report
            assert all(row["index"] in allocations["unmatched_contributions"] for row in report["records"])
            assert allocations["remaining_linked_effects"] == "unknown"
            assert not allocations["ready_for_collection"]
