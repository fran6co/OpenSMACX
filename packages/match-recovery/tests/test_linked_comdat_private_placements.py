"""Private no-fold witnesses keep exact original object-local identities."""

import json
from pathlib import Path
import struct

import pytest

from match_recovery.coff_comdat import comdat_key
from match_recovery.coff_private_function import private_comdat_root
from match_recovery.linked_comdat_selection import comdat_selection, _entry
from match_recovery.linked_context import linked_build
from match_recovery.pe_coff import Coff
from test_linked_comdat_unique_placements import fixture as external_fixture
from test_linked_comdat_icf import object_keys, CODE
from test_linked_initializers import update_artifact
from test_linked_modules import set_link
from test_link_file_evidence import stamp
from test_ordinary_contributions import set_contributions
from test_pdb_contributions import contribution


def private_object(rows, *, fpo=False, damage=None):
    raw = bytearray(object_keys(rows, fpo=fpo))
    obj = Coff(bytes(raw))
    table, count = struct.unpack_from("<II", raw, 8)
    root = obj.function("_P")
    raw[table + root.index * 18 + 16] = 3
    if damage in ("alias", "local_alias", "duplicate", "external_undefined", "weak"):
        if damage == "weak":
            extra = b"_weak\0\0\0" + struct.pack("<IhHBB", 0, 0, 0, 105, 1)
            extra += struct.pack("<II", root.index, 3) + b"\0" * 10
            added = 2
        else:
            extra = (b"_alias\0\0" if damage in ("alias", "local_alias") else b"_P\0\0\0\0\0\0")
            extra += struct.pack("<IhHBB", 0, 0 if damage == "external_undefined" else root.section,
                                 32 if damage == "duplicate" else 0,
                                 6 if damage == "local_alias" else 3 if damage == "duplicate" else 2, 0)
            added = 1
        raw[table + count * 18:table + count * 18] = extra
        struct.pack_into("<I", raw, 12, count + added)
    if damage == "nonzero": struct.pack_into("<I", raw, table + root.index * 18 + 8, 1)
    if damage == "type": struct.pack_into("<H", raw, table + root.index * 18 + 14, 0)
    if damage == "any": raw[table + (root.index - 1) * 18 + 14] = 2
    if damage == "aux":
        raw[table + root.index * 18 + 17] = 1
        raw[table + (root.index + 1) * 18:table + (root.index + 1) * 18] = b"\0" * 18
        struct.pack_into("<I", raw, 12, count + 1)
    return bytes(raw)


def fixture(root, *, damage=None, two=True, fpo=False):
    assert not (two and fpo)
    base_damage = damage if damage in ("overlap", "partial_overlap", "wrong_module", "peer_bytes", "missing_sc",
        "duplicate_sc", "missing_fpo", "nonexecuting", "cross_boundary") else None
    context = external_fixture(root, damage=base_damage, fpo=fpo)
    context.objects[1].write_bytes(private_object([(b"_K", 2, b"\x31\xc0\xc3"), (b"_P", 1, CODE)],
                                                fpo=fpo, damage=damage))
    if two:
        context.objects[0].write_bytes(private_object([(b"_K", 2, CODE), (b"_P", 1, CODE)]))
        context.report["compile_link"]["compilations"][0]["compilation"]["output"]["after"] = stamp(context.objects[0].read_bytes())
    evidence = json.loads((context.directory / "link.json").read_bytes())
    for row in evidence["files"]: row["before"] = row["after"] = stamp(Path(row["path"]).read_bytes())
    events = {"own_ref": "Discarded _P from dependency.obj\n", "ownerless": "Discarded _P\n",
        "own_section": "Discarded .text from dependency.obj\n", "other_ref": "Discarded _P from unit0.obj\n",
        "other_section": "Discarded .text from unit0.obj\n", "qualified": "Discarded _P from wrong/unit0.obj\n",
        "missing_other": "Discarded _P from missing.obj\n",
        "own_icf": "Selected symbol:\n_K from unit0.obj\nReplaced symbol(s):\n_P from dependency.obj\nICF total savings: 16 bytes\n"}
    log = (context.directory / "link.log").read_text()
    if damage in events: log = log.replace("Start Pass2", events[damage] + "Start Pass2")
    set_link(context, evidence, log)
    mapping = (context.directory / "app.map").read_text()
    rows = [line for line in mapping.splitlines() if " _P " in line]
    mapping = "\n".join(line for line in mapping.splitlines() if " _P " not in line) + "\n"
    if damage != "missing_map":
        row = rows[0]
        if damage == "wrong_owner": row = row.replace("dependency.obj", "unit0.obj")
        mapping += row + "\n"
        if damage == "duplicate_map": mapping += row + "\n"
    if damage == "public_map":
        mapping = mapping.replace(" entry point", rows[0] + "\n entry point")
    if two: mapping += " 0001:00000080 _P 00401080 f unit0.obj\n"
    update_artifact(context, "app.map", mapping.encode())
    if two:
        image = bytearray((context.directory / "app.exe").read_bytes())
        image[0x280:0x290] = CODE
        update_artifact(context, "app.exe", bytes(image))
        set_contributions(context, b"".join(contribution(section=1, offset=offset, size=16, flags=0x60501020, module=module)
            for offset, module in ((0, 0), (0x40, 1), (0x80, 0))))
    if damage == "winner_bytes":
        image = bytearray((context.directory / "app.exe").read_bytes()); image[0x200] ^= 1
        update_artifact(context, "app.exe", bytes(image))
    return context


def test_two_same_private_names_keep_two_exact_physical_placements(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        selected.loser("dependency.obj", 1)
        placements = selected.unique_placements()
        assert len(placements) == 2
        assert {p["address"] for p in placements} == {0x401040, 0x401080}
        assert {p["owner"] for p in placements} == {"unit0.obj", "dependency.obj"}
        assert all(p["root_identity_kind"] == "private" and p["root"]["name"] == "_P" for p in placements)
        assert all(p["proof"]["result"] == "pass" and p["outside_scope_verification"] == "unknown" for p in placements)
        cert, = selected.certificates()
        assert set(cert["no_other_fold"]["unique_placements"]) == {p["placement_id"] for p in placements}
        assert cert["no_other_fold"]["ref_discard_certificates"] == []
        for path in context.objects:
            obj = Coff(path.read_bytes())
            assert private_comdat_root(obj, 2) == obj.function("_P")
            with pytest.raises(ValueError, match="external"):
                comdat_key(obj, 2)


@pytest.mark.parametrize("damage", ["missing_map", "wrong_owner", "duplicate_map", "public_map", "alias", "local_alias", "weak",
    "duplicate", "external_undefined", "nonzero", "type", "any", "aux", "own_ref", "ownerless", "own_section",
    "qualified", "missing_other", "own_icf", "overlap", "partial_overlap", "wrong_module", "peer_bytes",
    "missing_sc", "duplicate_sc", "nonexecuting", "cross_boundary"])
def test_unsupported_private_identity_or_placement_refuses(tmp_path, damage):
    with pytest.raises(ValueError):
        with linked_build(fixture(tmp_path / "project", two=False, damage=damage)) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 1)


@pytest.mark.parametrize("damage", [None, "missing_fpo"])
def test_private_placement_keeps_full_native_fpo_obligation(tmp_path, damage):
    context = fixture(tmp_path / "project", two=False, fpo=True, damage=damage)
    with linked_build(context) as native, comdat_selection(native) as selected:
        if damage:
            with pytest.raises(ValueError, match="FPO|metadata"):
                selected.loser("dependency.obj", 1)
        else:
            selected.loser("dependency.obj", 1)
            peer, = selected.unique_placements()
            assert len(peer["sections"]) == 2 and peer["root"]["section"] == 3
            assert peer["proof"]["result"] == "pass"


def test_failed_winner_does_not_publish_private_peer_as_any_authority(tmp_path):
    with linked_build(fixture(tmp_path / "project", damage="winner_bytes")) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="ANY winner code"):
            selected.loser("dependency.obj", 1)
        assert len(selected.unique_placements()) == 2 and selected.certificates() == []


@pytest.mark.parametrize("target", ["physical", "parsed", "cache"])
def test_private_placement_is_guarded_after_proof(tmp_path, target):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 1)
            state = _entry(selected)
            if target == "physical": context.objects[1].write_bytes(object_keys([(b"_P", 1, CODE)]))
            elif target == "cache": state["unique_placements"].clear()
            else: next(iter(state["objects"].values())).symbols.clear()


def test_cached_private_placement_does_not_waive_another_query_overlap(tmp_path):
    from match_recovery import linked_comdat_selection as module
    context = fixture(tmp_path / "project", two=False)
    with linked_build(context) as native, comdat_selection(native) as selected:
        selected.loser("dependency.obj", 1)
        state = _entry(selected)
        cert, = selected.certificates()
        _, obj, _ = module._query(state, "unit0.obj", 1)
        with pytest.raises(ValueError, match="not separately placed"):
            module._no_other_fold(state, obj, obj.function("_K"), cert["candidates"], 0x401040)


def test_parsed_private_auxiliary_deletion_does_not_create_supported_form():
    obj = Coff(private_object([(b"_P", 1, CODE)], damage="aux"))
    obj.auxiliary.pop(obj.function("_P").index)
    with pytest.raises(ValueError, match="auxiliary"):
        private_comdat_root(obj, 1)


@pytest.mark.parametrize("damage", ["other_ref", "other_section"])
def test_other_owner_event_is_not_this_private_root_but_its_own_obligation_remains(tmp_path, damage):
    from match_recovery import linked_comdat_selection as module
    from match_recovery.model import digest
    context = fixture(tmp_path / "project", damage=damage)
    with linked_build(context) as native, comdat_selection(native) as selected:
        state = _entry(selected)
        original = native.inputs.objects.resolve("dependency.obj")
        placement = module._unique_placement(state, digest(original["identity"]), original["sha256"], 2)
        assert placement["owner"] == "dependency.obj" and placement["proof"]["result"] == "pass"
        # This proof does not erase the contradictory event belonging to the
        # other retained private root. Full ANY candidate traversal refuses it.
        with pytest.raises(ValueError, match="own REF|ANY winner"):
            selected.loser("dependency.obj", 1)
        assert selected.certificates() == []


def test_independent_external_same_spelling_does_not_become_a_private_alias(tmp_path):
    context = fixture(tmp_path / "project", two=False)
    raw = object_keys([(b"_K", 2, CODE), (b"_P", 1, b"\x31\xc0\xc3")])
    context.objects[0].write_bytes(raw)
    context.report["compile_link"]["compilations"][0]["compilation"]["output"]["after"] = stamp(raw)
    evidence = json.loads((context.directory / "link.json").read_bytes())
    for row in evidence["files"]: row["before"] = row["after"] = stamp(Path(row["path"]).read_bytes())
    set_link(context, evidence, (context.directory / "link.log").read_text())
    mapping = (context.directory / "app.map").read_text()
    mapping = mapping.replace(" entry point", " 0001:00000080 _P 00401080 f unit0.obj\n entry point")
    update_artifact(context, "app.map", mapping.encode())
    with linked_build(context) as native, comdat_selection(native) as selected:
        selected.loser("dependency.obj", 1)
        peer, = selected.unique_placements()
        assert peer["owner"] == "dependency.obj" and peer["root_identity_kind"] == "private"
        assert peer["address"] == 0x401040


def test_private_form_recognizer_never_authorizes_private_ref_removal(tmp_path):
    from match_recovery import linked_comdat_selection as module
    from match_recovery.model import digest
    context = fixture(tmp_path / "project", two=False, damage="own_ref")
    with linked_build(context) as native, comdat_selection(native) as selected:
        original = native.inputs.objects.resolve("dependency.obj")
        assert module._discarded_candidate(_entry(selected), digest(original["identity"]), original["sha256"], 2) is None
        assert selected.certificates() == []
        with pytest.raises(ValueError, match="external"):
            selected.discarded("dependency.obj", 2)


def test_external_key_dispatch_stays_separate_and_private_form_requires_nodup():
    obj = Coff(object_keys([(b"_P", 1, CODE)]))
    assert private_comdat_root(obj, 1) is None
    assert comdat_key(obj, 1) == obj.function("_P")
    for damage in ("any", "aux", "nonzero", "type", "external_undefined"):
        with pytest.raises(ValueError):
            private_comdat_root(Coff(private_object([(b"_P", 1, CODE)], damage=damage)), 1)


@pytest.mark.parametrize("event_form", ["named", "section", "unprinted_alias"])
def test_other_private_icf_cannot_rewrite_owner_and_impersonate_this_root(tmp_path, event_form):
    from match_recovery import linked_comdat_selection as module
    from match_recovery.model import digest
    context = fixture(tmp_path / "project")
    context.objects[1].write_bytes(private_object([(b"_K", 2, b"\x31\xc0\xc3"), (b"_P", 1, CODE), (b"_J", 1, CODE)]))
    if event_form == "unprinted_alias":
        raw = bytearray(object_keys([(b"_K", 2, CODE), (b"_Q", 1, CODE)]))
        table, count = struct.unpack_from("<II", raw, 8)
        raw[table + count * 18:table + count * 18] = b"_P\0\0\0\0\0\0" + struct.pack("<IhHBB", 0, 2, 0, 6, 0)
        struct.pack_into("<I", raw, 12, count + 1)
        context.objects[0].write_bytes(raw)
        context.report["compile_link"]["compilations"][0]["compilation"]["output"]["after"] = stamp(bytes(raw))
    evidence = json.loads((context.directory / "link.json").read_bytes())
    for row in evidence["files"]: row["before"] = row["after"] = stamp(Path(row["path"]).read_bytes())
    original = ".text" if event_form == "section" else "_Q" if event_form == "unprinted_alias" else "_P"
    group = f"Selected symbol:\n_J from dependency.obj\nReplaced symbol(s):\n{original} from unit0.obj\nICF total savings: 16 bytes\n"
    log = (context.directory / "link.log").read_text().replace("Start Pass2", group + "Start Pass2")
    set_link(context, evidence, log)
    mapping = (context.directory / "app.map").read_text().replace(" 0001:00000080 _P 00401080 f unit0.obj\n", "")
    mapping = mapping.replace(" entry point", " 0001:00000040 _J 00401040 f dependency.obj\n entry point")
    update_artifact(context, "app.map", mapping.encode())
    with linked_build(context) as native, comdat_selection(native) as selected:
        state = _entry(selected)
        original = native.inputs.objects.resolve("dependency.obj")
        obj = module._query(state, "dependency.obj", 2)[1]
        root = obj.function("_P")
        # Matching bytes, references and own module/SC independently pass for
        # B:P; they cannot prove the MAP row wasn't rewritten from folded A:P.
        proof = module.verify_code_region(obj, root, "dependency.obj", native.inputs.mapping, native.inputs.image,
            objects=native.inputs.objects, fixups=native.inputs.fixups, _index=state["map_index"],
            _constants=state["constants"], codeview=state["codeview"])
        assert proof["result"] == "pass"
        placement = module._placed_storage(state, original["identity"], original["sha256"], "dependency.obj", root,
                                            obj, 0x401040, native.inputs.mapping)
        assert placement["address"] == 0x401040
        with pytest.raises(ValueError, match="ICF provenance"):
            module._unique_placement(state, digest(original["identity"]), original["sha256"], 2)
        assert selected.unique_placements() == selected.certificates() == []
