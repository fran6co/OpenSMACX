"""Real compiler REF retention/removal, not an application acceptance receipt."""

import pytest

from match_recovery.application_context import application_build
from match_recovery.coff_comdat import comdat_selection as section_policy
from match_recovery.linked_context import linked_build
from match_recovery.pe_coff import Coff
from match_recovery.source_replica import replica_snapshot
from test_native_acceptance_compiler import prepared_project


@pytest.mark.parametrize("eliminate", [False, True])
def test_native_ref_event_controls_original_root_seeding(tmp_path, monkeypatch, eliminate):
    from match_recovery.linked_comdat_selection import comdat_selection
    from match_recovery.linked_section_domain import observe_sections

    root, _, _, _, _ = prepared_project(tmp_path, monkeypatch, proofs=False)
    body = root / "src/owner.cpp"
    body.write_text(body.read_text() + '\nextern "C" int unused_probe(int value) { return value + 139; }\n'
                    + 'extern "C" int unused_other(int value) { return value - 71; }\n')
    if eliminate:
        # Only this disposable test project's compiler interaction changes.
        # Native command receipts must capture the actual REF invocation.
        environment = root / "recovery/environment.py"
        text = environment.read_text()
        assert text.count('"/opt:noref"') == 1
        environment.write_text(text.replace('"/opt:noref"', '"/opt:ref"'))
    files = {name: raw for name, raw in replica_snapshot(root, ("src",)).items() if raw is not None}
    with application_build(root, ("src",), files, versioned_files={}) as context:
        assert context.report["result"] == "pass"
        with linked_build(context) as native:
            actual = native.inputs.objects.resolve("owner.obj")
            obj = Coff(actual["raw"])
            subjects = [obj.function(name) for name in ("_unused_probe", "_unused_other")]
            with comdat_selection(native) as selection:
                for subject in subjects:
                    assert section_policy(obj, subject.section) == 1
                    result = selection.discarded("owner.obj", subject.section)
                    observations = selection.discard_observations()
                    if eliminate:
                        assert result is not None
                        assert result["disposition"] == "discarded_unreferenced_comdat"
                        assert subject.section in result["sections"]
                        assert any(row["symbol"] == subject.name for row in observations["rows"])
                        assert not any(row.symbol == subject.name for row in native.inputs.mapping.publics)
                    else:
                        assert result is None
                        assert any(row.symbol == subject.name for row in native.inputs.mapping.publics)
            domain = observe_sections(context, native.inputs.objects, native=native)
            row, = [item for item in domain["inputs"] if item["owner"] == "owner.obj"]
            for subject in subjects:
                root_row = {"owner": "owner.obj", "symbol": subject.name}
                assert (root_row in domain["code_roots"]) is not eliminate
                section, = [item for item in row["sections"] if item["object_section"] == subject.section]
                assert section["retention"] == ("pass" if eliminate else "unknown")
            assert domain["result"] == domain["retained_section_selection"] == "unknown"
            assert not domain["ready_for_collection"]
