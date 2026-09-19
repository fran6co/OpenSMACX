"""Real compiler COMDAT selection, not an application acceptance receipt."""

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
                # Removed roots carry the discard disposition; kept roots earn
                # their independent placement while staying code roots.
                assert section["retention"] == "pass"
                assert section["disposition"] == ("discarded_unreferenced_comdat" if eliminate
                                                  else "independently_placed_nodup_root")
            assert domain["result"] == domain["retained_section_selection"] == "unknown"
            assert not domain["ready_for_collection"]
            allocations = native.ordinary_allocations()
            for subject in subjects:
                allocation, = [item for item in allocations["sections"]
                               if item["input"] == actual["identity"]
                               and item["object_section"] == subject.section]
                assert allocation["size"] > 0
                assert (allocation.get("disposition") == "discarded_unreferenced_comdat") is eliminate
                assert any(node["owner"] == "owner.obj" and node["symbol"] == subject.name
                           for node in allocations["code"]["nodes"]) is not eliminate
                if eliminate:
                    assert allocation["nonoccupying"] == allocation["retention"] == "pass"
                    assert "envelope" not in allocation
                    assert not any(gap.get("input") == actual["identity"]
                                   and gap.get("object_section") == subject.section
                                   for gap in allocations["gaps"])
            assert not allocations["ready_for_collection"]


@pytest.mark.parametrize("debug", [False, True])
def test_native_any_copies_with_icf_keep_original_winners_separate_from_storage(tmp_path, monkeypatch, debug):
    from match_recovery.linked_comdat_selection import comdat_selection
    from match_recovery.linked_section_domain import observe_sections
    from match_recovery.msvc_icf import parse_icf_log

    root, _, _, _, _ = prepared_project(tmp_path, monkeypatch, proofs=False)
    header = root / "src/owner.h"
    header.write_text(header.read_text()
        + "\ninline int fold_left(int value) { return value + 739; }\n"
        + "inline int fold_right(int value) { return value + 739; }\n")
    for unit in ("owner", "entry"):
        source = root / "src" / (unit + ".cpp")
        source.write_text(source.read_text()
            + f"\nint (*{unit}_fold_references[])(int) = {{fold_left, fold_right}};\n")
    environment = root / "recovery/environment.py"
    text = environment.read_text()
    assert text.count('"/opt:noicf"') == 1
    text = text.replace('"/opt:noicf"', '"/opt:icf"')
    if not debug:
        # A separate test-owned build-policy variant, not a waiver of debug
        # evidence from a product build. The debug case must remain refused.
        assert text.count(', "/Z7"') == 1
        text = text.replace(', "/Z7"', "")
    environment.write_text(text)
    files = {name: raw for name, raw in replica_snapshot(root, ("src",)).items() if raw is not None}
    names = ("?fold_left@@YAHH@Z", "?fold_right@@YAHH@Z")
    with application_build(root, ("src",), files, versioned_files={}) as context:
        assert context.report["result"] == "pass"
        with linked_build(context) as native:
            log = parse_icf_log(native.inputs.artifacts["msvc-link-log"])
            relevant = [group for group in log["groups"] if set(names) <= {
                row["symbol"] for row in (group["selected"], *group["replaced"])}]
            assert len(relevant) == 1, log
            queries = []
            for owner in ("owner.obj", "entry.obj"):
                obj = Coff(native.inputs.objects.resolve(owner)["raw"])
                for name in names:
                    subject = obj.function(name)
                    assert section_policy(obj, subject.section) == 2
                    queries.append((owner, subject))
            discarded = []
            with comdat_selection(native) as selection:
                for owner, subject in queries:
                    if debug:
                        with pytest.raises(ValueError, match="CodeView|debug\\$S"):
                            selection.loser(owner, subject.section)
                    else:
                        result = selection.loser(owner, subject.section)
                        if result is not None:
                            assert result["disposition"] == "discarded_any_duplicate"
                            discarded.append((owner, subject))
                if not debug:
                    assert len(discarded) == 2
                    assert len(selection.folded_groups()) == 1
            domain = observe_sections(context, native.inputs.objects, native=native)
            for owner, subject in discarded:
                assert {"owner": owner, "symbol": subject.name} not in domain["code_roots"]
            if debug:
                for owner, subject in queries:
                    assert {"owner": owner, "symbol": subject.name} in domain["code_roots"]
            else:
                # The surviving winners earn their selected disposition while
                # staying code roots.
                winners = [pair for pair in queries if pair not in discarded]
                assert len(winners) == 2
                for owner, subject in winners:
                    assert {"owner": owner, "symbol": subject.name} in domain["code_roots"]
                    row, = [item for item in domain["inputs"] if item["owner"] == owner]
                    section, = [item for item in row["sections"]
                                if item["object_section"] == subject.section]
                    assert section["retention"] == "pass"
                    assert section["disposition"] == "selected_any_winner"
                    assert section["root_identity_kind"] == "external"
            assert domain["result"] == "unknown" and not domain["ready_for_collection"]


def test_identical_nodup_peer_has_its_own_native_placement(tmp_path, monkeypatch):
    from match_recovery.linked_comdat_nofold import sections_distinct
    from match_recovery.linked_comdat_selection import comdat_selection
    from match_recovery.linked_section_domain import observe_sections

    root, _, _, _, _ = prepared_project(tmp_path, monkeypatch, proofs=False)
    header = root / "src/owner.h"
    header.write_text(header.read_text()
        + "\ninline int placed_copy(int value) { return value + 739; }\n")
    for unit in ("owner", "entry"):
        source = root / "src" / (unit + ".cpp")
        source.write_text(source.read_text()
            + f"\nint (*{unit}_placed_reference)(int) = placed_copy;\n")
    owner_source = root / "src/owner.cpp"
    owner_source.write_text(owner_source.read_text()
        + '\nextern "C" int placed_peer(int value) { return value + 739; }\n')
    # Keep this fixture's explicit NOICF/NOREF policy: the proof must establish
    # distinct physical placements, not infer them from absent fold/discard logs.
    files = {name: raw for name, raw in replica_snapshot(root, ("src",)).items() if raw is not None}
    name = "?placed_copy@@YAHH@Z"
    with application_build(root, ("src",), files, versioned_files={}) as context:
        assert context.report["result"] == "pass"
        with linked_build(context) as native:
            owner_obj = Coff(native.inputs.objects.resolve("owner.obj")["raw"])
            peer = owner_obj.function("_placed_peer")
            assert section_policy(owner_obj, peer.section) == 1
            peer_map, = [row for row in native.inputs.mapping.publics if row.symbol == peer.name]
            copy_map, = [row for row in native.inputs.mapping.publics if row.symbol == name]
            assert peer_map.owner == "owner.obj" and peer_map.address != copy_map.address
            discarded = []
            with comdat_selection(native) as selection:
                for owner in ("owner.obj", "entry.obj"):
                    obj = Coff(native.inputs.objects.resolve(owner)["raw"])
                    subject = obj.function(name)
                    assert section_policy(obj, subject.section) == 2
                    assert not sections_distinct(obj.sections[subject.section - 1],
                        obj.section_sizes[subject.section - 1], owner_obj.sections[peer.section - 1],
                        owner_obj.section_sizes[peer.section - 1])
                    result = selection.loser(owner, subject.section)
                    if result is not None:
                        discarded.append(owner)
                assert len(discarded) == 1
            domain = observe_sections(context, native.inputs.objects, native=native)
            assert {"owner": "owner.obj", "symbol": peer.name} in domain["code_roots"]
            assert {"owner": copy_map.owner, "symbol": name} in domain["code_roots"]
            assert {"owner": discarded[0], "symbol": name} not in domain["code_roots"]
            assert domain["result"] == "unknown" and not domain["ready_for_collection"]


def test_identical_nodup_peer_requires_positive_native_ref_removal(tmp_path, monkeypatch):
    from match_recovery.linked_comdat_nofold import sections_distinct
    from match_recovery.linked_comdat_selection import comdat_selection
    from match_recovery.linked_section_domain import observe_sections

    root, _, _, _, _ = prepared_project(tmp_path, monkeypatch, proofs=False)
    header = root / "src/owner.h"
    header.write_text(header.read_text()
        + "\ninline int retained_copy(int value) { return value + 739; }\n"
        + "extern int (* volatile owner_reference)(int);\n"
        + "extern int (* volatile entry_reference)(int);\n")
    for unit in ("owner", "entry"):
        source = root / "src" / (unit + ".cpp")
        source.write_text(source.read_text()
            + f"\nint (* volatile {unit}_reference)(int) = retained_copy;\n")
    source = root / "src/entry.cpp"
    text = source.read_text()
    assert text.count("ExitProcess(owner.add(5))") == 1
    source.write_text(text.replace("ExitProcess(owner.add(5))",
        "ExitProcess(owner_reference(owner.add(5)) + entry_reference(7))"))
    source = root / "src/owner.cpp"
    source.write_text(source.read_text()
        + '\nextern "C" int unused_identical(int value) { return value + 739; }\n')
    environment = root / "recovery/environment.py"
    text = environment.read_text()
    assert text.count('"/opt:noref"') == 1
    environment.write_text(text.replace('"/opt:noref"', '"/opt:ref"'))
    files = {name: raw for name, raw in replica_snapshot(root, ("src",)).items() if raw is not None}
    name = "?retained_copy@@YAHH@Z"
    with application_build(root, ("src",), files, versioned_files={}) as context:
        assert context.report["result"] == "pass"
        with linked_build(context) as native:
            owner_obj = Coff(native.inputs.objects.resolve("owner.obj")["raw"])
            peer = owner_obj.function("_unused_identical")
            assert section_policy(owner_obj, peer.section) == 1
            assert not any(row.symbol == peer.name for row in native.inputs.mapping.publics)
            copy_map, = [row for row in native.inputs.mapping.publics if row.symbol == name]
            discarded = []
            with comdat_selection(native) as selection:
                # Query the ANY family first: positive REF proof cannot depend
                # on a prior census/query having already visited the peer.
                for owner in ("owner.obj", "entry.obj"):
                    obj = Coff(native.inputs.objects.resolve(owner)["raw"])
                    subject = obj.function(name)
                    assert section_policy(obj, subject.section) == 2
                    assert not sections_distinct(obj.sections[subject.section - 1],
                        obj.section_sizes[subject.section - 1], owner_obj.sections[peer.section - 1],
                        owner_obj.section_sizes[peer.section - 1])
                    result = selection.loser(owner, subject.section)
                    if result is not None:
                        discarded.append(owner)
                assert len(discarded) == 1
                removed = selection.discarded("owner.obj", peer.section)
                assert removed is not None and removed["disposition"] == "discarded_unreferenced_comdat"
                certificates = selection.certificates()
                any_copy, = [row for row in certificates if row["check"] == "native_comdat_any_selection"]
                assert removed["certificate_id"] in any_copy["no_other_fold"]["ref_discard_certificates"]
                assert any(row["symbol"] == peer.name and row["owner"]["label"] == "owner.obj"
                           for row in selection.discard_observations()["rows"])
            domain = observe_sections(context, native.inputs.objects, native=native)
            assert {"owner": copy_map.owner, "symbol": name} in domain["code_roots"]
            assert {"owner": discarded[0], "symbol": name} not in domain["code_roots"]
            assert {"owner": "owner.obj", "symbol": peer.name} not in domain["code_roots"]
            physical, = [row for row in domain["inputs"] if row["owner"] == "owner.obj"]
            section, = [row for row in physical["sections"] if row["object_section"] == peer.section]
            assert section["disposition"] == "discarded_unreferenced_comdat"
            assert section["symbols"] and section["retention"] == "pass"
            assert domain["result"] == "unknown" and not domain["ready_for_collection"]
