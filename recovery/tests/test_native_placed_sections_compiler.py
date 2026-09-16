"""Real compiler placement dispositions, not an application acceptance receipt."""

from match_recovery.application_context import application_build
from match_recovery.coff_comdat import comdat_selection as section_policy
from match_recovery.linked_context import linked_build
from match_recovery.pe_coff import Coff
from match_recovery.source_replica import replica_snapshot
from test_native_acceptance_compiler import prepared_project


def test_placed_nodup_roots_earn_retention_and_stay_code_roots(tmp_path, monkeypatch):
    from match_recovery.linked_comdat_selection import comdat_selection
    from match_recovery.linked_section_domain import observe_sections

    root, _, _, _, _ = prepared_project(tmp_path, monkeypatch, proofs=False)
    body = root / "src/owner.cpp"
    body.write_text(body.read_text() + '\nextern "C" int unused_probe(int value) { return value + 139; }\n'
                    + 'extern "C" int unused_other(int value) { return value - 71; }\n')
    files = {name: raw for name, raw in replica_snapshot(root, ("src",)).items() if raw is not None}
    with application_build(root, ("src",), files, versioned_files={}) as context:
        assert context.report["result"] == "pass"
        with linked_build(context) as native:
            actual = native.inputs.objects.resolve("owner.obj")
            obj = Coff(actual["raw"])
            subjects = [obj.function(name) for name in ("_unused_probe", "_unused_other")]
            placed_ids = {}
            with comdat_selection(native) as selection:
                for subject in subjects:
                    assert section_policy(obj, subject.section) == 1
                    # The default NOREF policy keeps both roots placed: no
                    # removal certificate exists, so placement is the query.
                    assert selection.discarded("owner.obj", subject.section) is None
                    placement = selection.placed("owner.obj", subject.section)
                    assert placement["disposition"] == "independently_placed_nodup_root"
                    assert placement["root_identity_kind"] == "external"
                    assert placement["queried_section"] == subject.section
                    assert subject.section in placement["sections"]
                    placed_ids[subject.name] = placement["placement_id"]
                recorded = {row["placement_id"]: row["address"] for row in selection.unique_placements()}
                assert set(placed_ids.values()) <= set(recorded)
                for subject in subjects:
                    label, = [row for row in native.inputs.mapping.publics if row.symbol == subject.name]
                    assert recorded[placed_ids[subject.name]] == label.address
            domain = observe_sections(context, native.inputs.objects, native=native)
            row, = [item for item in domain["inputs"] if item["owner"] == "owner.obj"]
            for subject in subjects:
                # A placed root stays a code root: annotation, never exclusion.
                assert {"owner": "owner.obj", "symbol": subject.name} in domain["code_roots"]
                section, = [item for item in row["sections"]
                            if item["object_section"] == subject.section]
                assert section["retention"] == "pass"
                assert section["disposition"] == "independently_placed_nodup_root"
                assert section["root_identity_kind"] == "external"
                assert section["selection_placement"] == placed_ids[subject.name]
            assert domain["result"] == domain["retained_section_selection"] == "unknown"
            assert not domain["ready_for_collection"]
