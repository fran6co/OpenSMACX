"""Private COMDAT placement uses physical owners, not global symbol spelling."""

import pytest

from match_recovery.application_context import application_build
from match_recovery.coff_comdat import comdat_key, comdat_selection as section_policy
from match_recovery.linked_comdat_nofold import sections_distinct
from match_recovery.linked_comdat_selection import comdat_selection
from match_recovery.linked_context import linked_build
from match_recovery.linked_section_domain import observe_sections
from match_recovery.pe_coff import Coff
from match_recovery.source_replica import replica_snapshot
from test_native_acceptance_compiler import prepared_project


@pytest.mark.parametrize("debug", [False, True])
def test_same_named_private_peers_require_independent_native_placements(tmp_path, monkeypatch, debug):
    root, _, _, _, _ = prepared_project(tmp_path, monkeypatch, proofs=False)
    header = root / "src/owner.h"
    header.write_text(header.read_text()
        + "\ninline int private_copy(int value) { return value + 739; }\n")
    for unit in ("owner", "entry"):
        source = root / "src" / (unit + ".cpp")
        source.write_text(source.read_text()
            + "\nstatic int private_peer(int value) { return value + 739; }\n"
            + f"int (* volatile {unit}_private_references[])(int) = {{private_copy, private_peer}};\n")
    if not debug:
        environment = root / "recovery/environment.py"
        text = environment.read_text()
        assert text.count(', "/Z7"') == 1
        environment.write_text(text.replace(', "/Z7"', ""))
    # Explicit NOREF/NOICF stays in force. The native proof still needs each
    # peer's independent physical placement; option absence is not authority.
    files = {name: raw for name, raw in replica_snapshot(root, ("src",)).items() if raw is not None}
    name, private = "?private_copy@@YAHH@Z", "?private_peer@@YAHH@Z"
    with application_build(root, ("src",), files, versioned_files={}, retain=True) as context:
        assert context.report["result"] == "pass"
        with linked_build(context) as native:
            peers = [row for row in native.inputs.mapping.statics if row.symbol == private]
            assert len(peers) == 2
            assert {row.owner for row in peers} == {"owner.obj", "entry.obj"}
            assert len({row.address for row in peers}) == 2
            assert not any(row.symbol == private for row in native.inputs.mapping.publics)
            copy_map, = [row for row in native.inputs.mapping.publics if row.symbol == name]
            discarded = []
            with comdat_selection(native) as selection:
                for owner in ("owner.obj", "entry.obj"):
                    obj = Coff(native.inputs.objects.resolve(owner)["raw"])
                    peer, subject = obj.function(private), obj.function(name)
                    assert peer.storage == 3 and peer.type == 32 and peer.value == 0
                    assert section_policy(obj, peer.section) == 1
                    assert section_policy(obj, subject.section) == 2
                    # The general external-key contract is deliberately unchanged.
                    with pytest.raises(ValueError, match="external key"):
                        comdat_key(obj, peer.section)
                    assert not sections_distinct(obj.sections[subject.section - 1],
                        obj.section_sizes[subject.section - 1], obj.sections[peer.section - 1],
                        obj.section_sizes[peer.section - 1])
                    result = selection.loser(owner, subject.section)
                    if result is not None:
                        discarded.append(owner)
                assert len(discarded) == 1
                placements = selection.unique_placements()
                assert len(placements) == 2
                assert {row["root_identity_kind"] for row in placements} == {"private"}
                assert {(row["owner"], row["address"]) for row in placements} == {
                    (row.owner, row.address) for row in peers}
                assert len({row["placement_id"] for row in placements}) == 2
                assert all(row["proof"]["result"] == "pass" for row in placements)
                if debug:
                    from match_recovery.codeview_procedures import S_LPROC32_ST
                    for row in placements:
                        procedures = [item for item in row["proof"]["debug_regions"]
                                      if item.get("format") == "codeview5_procedure"]
                        assert len(procedures) == 1 and procedures[0]["kind"] == S_LPROC32_ST
                certificate, = selection.certificates()
                assert set(certificate["no_other_fold"]["unique_placements"]) == {
                    row["placement_id"] for row in placements}
            domain = observe_sections(context, native.inputs.objects, native=native)
            assert {"owner": copy_map.owner, "symbol": name} in domain["code_roots"]
            assert {"owner": discarded[0], "symbol": name} not in domain["code_roots"]
            placements_by_owner = {row["owner"]: row for row in placements}
            for owner in ("owner.obj", "entry.obj"):
                assert {"owner": owner, "symbol": private} in domain["code_roots"]
                original, = [row for row in domain["inputs"] if row["owner"] == owner]
                obj = Coff(native.inputs.objects.resolve(owner)["raw"])
                section, = [row for row in original["sections"]
                            if row["object_section"] == obj.function(private).section]
                # Each private peer earns its own placement annotation while
                # remaining an ordinary code root.
                assert section["retention"] == "pass"
                assert section["disposition"] == "independently_placed_nodup_root"
                assert section["root_identity_kind"] == "private"
                assert section["selection_placement"] == placements_by_owner[owner]["placement_id"]
            assert domain["result"] == domain["retained_section_selection"] == "unknown"
            assert not domain["ready_for_collection"]
