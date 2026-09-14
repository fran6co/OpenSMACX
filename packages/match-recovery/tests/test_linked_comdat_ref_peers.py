"""REF certificates exclude exact physical no-fold competitors, not references."""

import json
import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_comdat_selection import comdat_selection, _entry
from match_recovery.model import digest
from test_linked_comdat_selection import fixture as ordinary_fixture
from test_linked_comdat_icf import object_keys, CODE
from test_linked_comdat_unique_placements import fixture as placed_fixture
from test_linked_initializers import update_artifact
from test_linked_modules import set_link
from test_link_file_evidence import stamp


def fixture(root, *, damage=None, child=False):
    """unit0 K@401000; dependency K then absent P. Child variant: P/child/K."""
    rows = [(b"_P", 2 if damage == "any_peer" else 1, CODE), (b"_K", 2, b"\x31\xc0\xc3")]
    if not child: rows.reverse()
    if damage == "hidden_peer": rows.append((b"_hidden", 1, CODE))
    raw = bytearray(object_keys(rows, companion=child))
    if child:
        at, = struct.unpack_from("<I", raw, 20 + 40 + 20)
        raw[at:at + 16] = CODE  # Same-byte associative code is a separate census candidate.
        if damage == "wrong_parent":
            symbols, = struct.unpack_from("<I", raw, 8)
            struct.pack_into("<H", raw, symbols + 4 * 18 + 12, 3)
        if damage == "child_alias":
            symbols, = struct.unpack_from("<I", raw, 8)
            raw[symbols + 5 * 18 + 16] = 2
    context = ordinary_fixture(root, winner=object_keys([(b"_K", 2, CODE)]), loser=bytes(raw))
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"].extend(["/OPT:ICF", "/VERBOSE"])
    if damage != "missing_ref_option": evidence["arguments"].append("/OPT:REF")
    if damage == "forced": evidence["arguments"].append("/INCLUDE:_P")
    if damage == "noref": evidence["arguments"].append("/OPT:NOREF")
    event = "Discarded _P from dependency.obj\n"
    if damage == "missing_event": event = ""
    if damage == "duplicate": event *= 2
    if damage == "ownerless": event = "Discarded _P\n"
    if damage == "section_only": event = "Discarded .text from dependency.obj\n"
    if damage == "qualified": event = "Discarded _P from wrong/dependency.obj\n"
    if damage == "wrong_owner": event = "Discarded _P from unit0.obj\n"
    if damage == "malformed": event = "Discarded invalid line\n"
    if damage == "winner_ref": event += "Discarded _K from unit0.obj\n"
    if damage == "icf":
        event += "Selected symbol:\n_K from unit0.obj\nReplaced symbol(s):\n_P from dependency.obj\nICF total savings: 16 bytes\n"
    log = (context.directory / "link.log").read_text().replace("End Pass1\n", "End Pass1\n" + event, 1)
    if damage == "qualified_final": log = log.replace("\ndependency.obj\n", "\nwrong/dependency.obj\n")
    if damage == "ambiguous_input":
        path = context.directory / "other/dependency.obj"
        path.parent.mkdir()
        path.write_bytes(context.objects[1].read_bytes())
        evidence["arguments"].append(str(path)); evidence["explicit_inputs"].append(str(path))
        evidence["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
    set_link(context, evidence, log)
    mapping = (context.directory / "app.map").read_bytes().replace(b"_f", b"_K")
    if damage == "mapped": mapping += b" 0001:00000040 _P 00401040 f dependency.obj\n"
    if damage == "child_mapped": mapping += b" 0001:00000040 $tail 00401040 dependency.obj\n"
    update_artifact(context, "app.map", mapping)
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x210] = CODE
    if damage == "winner_bytes": image[0x200] ^= 1
    update_artifact(context, "app.exe", bytes(image))
    return context


@pytest.mark.parametrize("ref_first", [False, True])
@pytest.mark.parametrize("child", [False, True])
def test_positive_ref_is_order_independent_and_preserves_exact_forest(tmp_path, ref_first, child):
    context = fixture(tmp_path / "project", child=child)
    p, k = (1, 3) if child else (2, 1)
    with linked_build(context) as native, comdat_selection(native) as selected:
        if ref_first: selected.discarded("dependency.obj", 2 if child else p)
        result = selected.loser("dependency.obj", k)
        ref = selected.discarded("dependency.obj", p)
        certificates = selected.certificates()
        any_cert, = [row for row in certificates if row["check"] == "native_comdat_any_selection"]
        ref_cert, = [row for row in certificates if row["check"] == "native_comdat_ref_selection"]
        assert result["certificate_id"] == any_cert["certificate_id"]
        assert any_cert["no_other_fold"]["ref_discard_certificates"] == [ref["certificate_id"]]
        assert selected.unique_placements() == []
        assert ref_cert["original"]["input"] == {"path": str(context.objects[1])}
        assert ref_cert["original"]["root"]["section"] == p
        assert ref_cert["incoming_references"] == ref_cert["outside_scope_verification"] == "unknown"
        assert any_cert["source_authority"] == "not_transferred"
        assert [r["object_section"] for r in ref_cert["original"]["sections"]] == ([1, 2] if child else [2])
        if child:
            assert selected.discarded("dependency.obj", 2)["certificate_id"] == ref["certificate_id"]
            assert any_cert["no_other_fold"]["other_physical_code_sections"] == 2


@pytest.mark.parametrize("damage", ["missing_event", "duplicate", "ownerless", "section_only", "qualified",
    "wrong_owner", "malformed", "qualified_final", "ambiguous_input", "forced", "noref", "missing_ref_option",
    "mapped", "winner_ref", "icf", "hidden_peer", "any_peer"])
def test_missing_unsupported_or_contradictory_ref_never_clears_competitor(tmp_path, damage):
    with pytest.raises(ValueError):
        with linked_build(fixture(tmp_path / "project", damage=damage)) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 1)


@pytest.mark.parametrize("damage", ["wrong_parent", "child_alias", "child_mapped"])
def test_child_authority_is_only_the_exact_original_ref_forest(tmp_path, damage):
    with pytest.raises(ValueError):
        with linked_build(fixture(tmp_path / "project", child=True, damage=damage)) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 3)


def test_absent_ref_keeps_prior_independent_placement_route(tmp_path):
    with linked_build(placed_fixture(tmp_path / "project")) as native, comdat_selection(native) as selected:
        selected.loser("dependency.obj", 1)
        cert, = selected.certificates()
        assert cert["no_other_fold"]["ref_discard_certificates"] == []
        assert len(selected.unique_placements()) == 1


def test_failed_winner_cannot_publish_any_using_valid_peer_ref(tmp_path):
    with linked_build(fixture(tmp_path / "project", damage="winner_bytes")) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="ANY winner code"):
            selected.loser("dependency.obj", 1)
        assert [row["check"] for row in selected.certificates()] == ["native_comdat_ref_selection"]


@pytest.mark.parametrize("target", ["physical", "parsed", "named_events", "certificates"])
def test_completed_batch_checks_peer_proof_physical_and_derived_state(tmp_path, target):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 1)
            state = _entry(selected)
            if target == "physical": context.objects[1].write_bytes(object_keys([(b"_P", 1, CODE)]))
            elif target == "named_events": state["discard_named_events"].clear()
            elif target == "certificates": state["ref_certificates"].clear()
            else:
                key = next(key for key, obj in state["objects"].items() if any(s.name == "_P" for s in obj.symbols.values()))
                state["objects"][key].symbols.clear()


def test_exact_physical_identity_and_hash_cannot_borrow_another_peer(tmp_path):
    from match_recovery import linked_comdat_selection as module
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        record = next(row for rows in _entry(selected)["records"].values() for row in rows
                      if row["path"] == str(context.objects[1]))
        for identity, sha, number in [(digest({"path": str(context.objects[0])}), record["sha256"], 2),
                                       (digest({"path": str(context.objects[1])}), "0" * 64, 2),
                                       (digest({"path": str(context.objects[1])}), record["sha256"], 99)]:
            with pytest.raises(ValueError, match="physical|outside"):
                module._discarded_candidate(_entry(selected), identity, sha, number)
        assert selected.certificates() == []


def test_reports_are_defensive_and_capability_expires(tmp_path):
    with linked_build(fixture(tmp_path / "project")) as native, comdat_selection(native) as selected:
        selected.loser("dependency.obj", 1)
        reports = selected.certificates()
        reports[0]["no_other_fold"]["ref_discard_certificates"].clear()
        assert selected.certificates()[0]["no_other_fold"]["ref_discard_certificates"]
    with pytest.raises(ValueError, match="expired"):
        selected.certificates()


def test_named_event_index_is_rechecked_even_when_a_later_winner_proof_refuses(tmp_path):
    with pytest.raises(ValueError, match="event observations changed"):
        with linked_build(fixture(tmp_path / "project", damage="winner_bytes")) as native, comdat_selection(native) as selected:
            selected.discarded("dependency.obj", 2)
            _entry(selected)["discard_named_events"].clear()
            selected.loser("dependency.obj", 1)
