"""Complete original ANY selection composed with physical native ICF groups."""

import json
from pathlib import Path
import struct

import pytest

from match_recovery.linked_context import linked_build
from match_recovery.linked_comdat_selection import comdat_selection, _entry
from match_recovery.msf import SmallMsf
from test_linked_comdat_selection import fixture as ordinary_fixture
from test_linked_initializers import update_artifact
from test_linked_modules import set_link
from test_link_file_evidence import stamp
from test_msf import small_msf
from test_ordinary_contributions import set_contributions
from test_pdb_contributions import contribution


CODE = b"\x90" * 15 + b"\xc3"


def object_keys(rows, *, fpo=False, alias=False, companion=False):
    specs, symbols = [], []
    for order, (name, policy, body) in enumerate(rows):
        number = len(specs) + 1
        root_index = len(symbols) + 2
        body += b"\xcc" * (-len(body) % 16)
        relocations = ((1, root_index + 3, 20),) if companion and order == 0 else ()
        specs.append((b".text", body, relocations, 0x60501020))
        symbols.extend([b".text\0\0\0" + struct.pack("<IhHBB", 0, number, 0, 3, 1),
            struct.pack("<IHHIHB3x", len(body), len(relocations), 0, 0, 0, policy),
            name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, 32, 2, 0)])
        if companion and order == 0:
            specs.append((b".text$x", b"\xc3" + b"\xcc" * 15, (), 0x60501020))
            symbols.extend([b".text$x\0" + struct.pack("<IhHBB", 0, number + 1, 0, 3, 1),
                struct.pack("<IHHIHB3x", 16, 0, 0, 0, number, 5),
                b"$tail\0\0\0" + struct.pack("<IhHBB", 0, number + 1, 0, 6, 0)])
        if fpo:
            payload = struct.pack("<IIIHH", 0, 16, 3, 2, 0x0201)
            specs.append((b".debug$F", payload, ((0, root_index, 7),), 0x42101048))
            symbols.extend([b".debug$F" + struct.pack("<IhHBB", 0, number + 1, 0, 3, 1),
                struct.pack("<IHHIHB3x", 16, 1, 0, 0, number, 5)])
    if alias:
        symbols.append(b"_alias\0\0" + struct.pack("<IhHBB", 0, 1, 0, 2, 0))
    headers, payload = [], bytearray()
    beginning = 20 + len(specs) * 40
    for name, body, relocations, flags in specs:
        offset = beginning + len(payload)
        payload.extend(body)
        relocation_at = beginning + len(payload) if relocations else 0
        payload.extend(b"".join(struct.pack("<IIH", *row) for row in relocations))
        headers.append(name.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(body), offset,
            relocation_at, 0, len(relocations), 0, flags))
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, beginning + len(payload), len(symbols), 0, 0)
            + b"".join(headers) + payload + b"".join(symbols) + struct.pack("<I", 4))


def fixture(root, *, fpo=False, damage=None):
    """A=unit0 owns selected K; B=dependency owns J and discarded K."""
    companion = damage == "shared_companion"
    code = b"\xe8\0\0\0\0\xc3" if companion else CODE
    a_code = code + b"\xcc" * 16 if damage == "unequal_extent" else code
    a = object_keys([(b"_K", 2, a_code if damage != "original_bytes" else b"\x31\xc0\xc3")],
                   fpo=fpo, companion=companion)
    if damage == "codeview": a = a.replace(b".debug$F", b".debug$S")
    rows = [(b"_J", 2 if damage == "both_any" else 1, code if damage != "survivor_bytes" else b"\x31\xc0\xc3"),
            (b"_K", 2, b"\x31\xc0\xc3")]
    if damage in ("chain", "duplicate_key_group", "section_alias_group"):
        rows.append((b"_Q", 1, CODE))
    b = object_keys(rows, fpo=fpo, alias=damage == "survivor_alias", companion=companion)
    context = ordinary_fixture(root, winner=a, loser=b)
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"].extend(["/OPT:ICF", "/VERBOSE"])
    if damage == "policy": evidence["arguments"].append("/OPT:NOICF")
    log = (context.directory / "link.log").read_text()
    group = "Selected symbol:\n_J from dependency.obj\nReplaced symbol(s):\n_K from unit0.obj\n"
    if damage == "missing_event": group = ""
    if damage == "duplicate": group += "_K from unit0.obj\n"
    if damage == "qualified": group = group.replace("from unit0.obj", "from wrong/unit0.obj")
    if damage == "wrong_event_owner": group = group.replace("_K from unit0.obj", "_K from dependency.obj")
    if damage == "chain":
        group += "Selected symbol:\n_Q from dependency.obj\nReplaced symbol(s):\n_J from dependency.obj\n"
    if damage in ("duplicate_key_group", "section_alias_group"):
        other = "_K" if damage == "duplicate_key_group" else ".text"
        group += f"Selected symbol:\n_Q from dependency.obj\nReplaced symbol(s):\n{other} from unit0.obj\n"
    if damage == "ref_conflict": group = "Discarded _J from dependency.obj\n" + group
    log = log.replace("Start Pass2", group + f"ICF total savings: {16 if group else 0} bytes\nStart Pass2")
    if damage == "qualified_final": log = log.replace("\ndependency.obj\n", "\nwrong/dependency.obj\n")
    set_link(context, evidence, log)
    mapping = ("folded\n Preferred load address is 00400000\n"
        " Address Publics by Value Rva+Base Lib:Object\n"
        " 0001:00000000 _J 00401000 f dependency.obj\n"
        " 0001:00000000 _K 00401000 f dependency.obj\n"
        + (" 0001:00000000 _Q 00401000 f dependency.obj\n"
           if damage in ("chain", "duplicate_key_group", "section_alias_group") else "")
        + " entry point at 0001:00000000\n Static symbols\n")
    update_artifact(context, "app.map", mapping.encode())
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x210] = CODE
    if damage == "unequal_extent": image[0x210:0x220] = b"\xcc" * 16
    if companion:
        image[0x200:0x210] = b"\xe8\x1b\0\0\0\xc3" + b"\xcc" * 10
        image[0x220:0x230] = b"\xc3" + b"\xcc" * 15
    update_artifact(context, "app.exe", bytes(image))
    row = contribution(section=1, offset=0, size=16, flags=0x60501020,
                       module=0 if damage == "wrong_module" else 1)
    set_contributions(context, row * (2 if damage == "duplicate_sc" else 1))
    if companion:
        msf = SmallMsf((context.directory / "app.pdb").read_bytes())
        streams = [msf.stream(index) for index in range(msf.stream_count)]
        streams[4] = struct.pack("<HHII", 20, 0, 0x1001, 0x1020)
        update_artifact(context, "app.pdb", small_msf(streams))
    if fpo:
        msf = SmallMsf((context.directory / "app.pdb").read_bytes())
        streams = [msf.stream(index) for index in range(msf.stream_count)]
        dbi = bytearray(streams[3])
        counts = struct.unpack_from("<8I", dbi, 24)
        debug_at = 64 + sum(counts[:5]) + counts[7]
        struct.pack_into("<H", dbi, debug_at, len(streams))
        streams[3] = bytes(dbi)
        count = 1 if damage in ("missing_fpo", "codeview") else 3 if damage == "extra_fpo" else 2
        streams.append(struct.pack("<IIIHH", 0x1000, 16, 3, 2, 0x0201) * count)
        update_artifact(context, "app.pdb", small_msf(streams))
    return context


@pytest.mark.parametrize("fpo", [False, True])
def test_original_any_winner_is_not_the_map_owners_discarded_copy(tmp_path, fpo):
    context = fixture(tmp_path / "project", fpo=fpo)
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.loser("unit0.obj", 1) is None
        result = selected.loser("dependency.obj", 3 if fpo else 2)
        cert, = selected.certificates()
        group, = selected.folded_groups()
        assert cert["winner"]["owner"] == "unit0.obj"
        assert cert["winner"]["input"] == {"path": str(context.objects[0])}
        assert cert["final_storage"]["owner"] == "dependency.obj"
        assert cert["final_storage"]["root"]["name"] == "_J"
        assert cert["fold_group_id"] == group["group_id"]
        assert group["result"] == "pass" and len(group["members"]) == 2
        assert result["sections"] == ([3, 4] if fpo else [2])
        assert selected.loser("dependency.obj", 1) is None
        assert cert["outside_scope_verification"] == "unknown"
        assert group["source_authority"] == "not_transferred"


@pytest.mark.parametrize("damage", ["original_bytes", "survivor_bytes", "survivor_alias", "policy",
    "missing_event", "duplicate", "qualified", "qualified_final", "wrong_event_owner",
    "chain", "duplicate_key_group", "section_alias_group", "ref_conflict", "wrong_module", "duplicate_sc"])
def test_folded_selection_damage_never_selects_convenient_copy(tmp_path, damage):
    context = fixture(tmp_path / "project", damage=damage)
    with pytest.raises(ValueError):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 2)


@pytest.mark.parametrize("damage", ["missing_fpo", "extra_fpo"])
def test_all_original_frames_remain_required(tmp_path, damage):
    context = fixture(tmp_path / "project", fpo=True, damage=damage)
    with pytest.raises(ValueError, match="FPO"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 3)


def test_group_reports_are_defensive_and_handles_expire(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        selected.loser("dependency.obj", 2)
        rows = selected.folded_groups()
        rows[0]["members"].clear()
        assert len(selected.folded_groups()[0]["members"]) == 2
    with pytest.raises(ValueError, match="expired"):
        selected.folded_groups()


@pytest.mark.parametrize("changed", ["fold_groups", "fold_index"])
def test_derived_group_cache_cannot_change_after_native_proof(tmp_path, changed):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError, match="indices changed"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 2)
            _entry(selected)[changed].clear()


def test_original_physical_member_mutation_invalidates_completed_batch(tmp_path):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 2)
            context.objects[0].write_bytes(object_keys([(b"_K", 2, b"\x31\xc0\xc3")]))


def test_folded_codeview_is_not_waived_by_successful_member_or_frame_checks(tmp_path):
    context = fixture(tmp_path / "project", fpo=True, damage="codeview")
    with pytest.raises(ValueError, match="CodeView requires native non-folded"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 3)


def test_complete_group_is_verified_once_for_multiple_key_families(monkeypatch, tmp_path):
    from match_recovery import linked_comdat_selection as module
    original = module.verify_code_region
    calls = []
    def counted(*args, **kwargs):
        calls.append(args[1].name)
        return original(*args, **kwargs)
    monkeypatch.setattr(module, "verify_code_region", counted)
    context = fixture(tmp_path / "project", damage="both_any")
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.loser("dependency.obj", 2) is not None
        assert selected.loser("dependency.obj", 1) is None
        assert selected.loser("unit0.obj", 1) is None
        assert len(selected.certificates()) == 2
        assert len(selected.folded_groups()) == 1
    assert calls == ["_J", "_K"]


@pytest.mark.parametrize("damage", ["unequal_extent", "shared_companion"])
def test_individually_valid_members_cannot_claim_incompatible_shared_regions(monkeypatch, tmp_path, damage):
    from match_recovery import linked_comdat_selection as module
    original = module.verify_code_region
    proofs = []
    def observed(*args, **kwargs):
        proof = original(*args, **kwargs)
        proofs.append(proof)
        return proof
    monkeypatch.setattr(module, "verify_code_region", observed)
    context = fixture(tmp_path / "project", damage=damage)
    with pytest.raises(ValueError, match="unequal or shared companion runtime extents"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 3 if damage == "shared_companion" else 2)
    assert len(proofs) == 2 and all(row["result"] == "pass" for row in proofs)
    if damage == "unequal_extent":
        assert [row["size"] for row in proofs] == [16, 32]
    else:
        assert [[part["address"] for part in row["regions"]] for row in proofs] == [
            [0x401000, 0x401020], [0x401000, 0x401020]]


def test_overlap_work_exhaustion_never_returns_a_partial_group(monkeypatch, tmp_path):
    from match_recovery import linked_comdat_selection as module
    monkeypatch.setattr(module, "MAX_FOLD_OVERLAPS", 0)
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="overlap work"):
            selected.loser("dependency.obj", 2)
        assert selected.folded_groups() == []
        assert selected.certificates() == []
