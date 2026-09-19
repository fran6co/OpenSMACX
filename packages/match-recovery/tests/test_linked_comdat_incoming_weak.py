"""Physical weak incoming uses are not definitions or alias-resolution proof."""

import json
import struct

import pytest

from match_recovery.linked_comdat_selection import comdat_selection, _entry
from match_recovery.linked_context import linked_build
from test_linked_comdat_selection import fixture, object_bytes
from test_linked_initializers import update_artifact
from test_linked_modules import add_archive, set_link
from test_link_file_evidence import stamp


def incoming(raw, *, mode=2, target="undefined", weak_name=b"_e", weak_type=32,
             duplicate=None, malformed=None, reference=False, key=b"_f"):
    """Append the original auxiliary-index edge, not a parsed-symbol mock."""
    raw = bytearray(raw)
    at, count = struct.unpack_from("<II", raw, 8)
    target_index, weak_index = count, count + 1
    name = b"_other" if weak_name == key else key
    value, section, kind, storage, aux = {
        "undefined": (0, 0, 32, 2, 0), "common": (4, 0, 32, 2, 0),
        "absolute": (0, -1, 32, 2, 0), "local": (0, 1, 32, 3, 0),
        "data": (0, 0, 0, 2, 0), "auxiliary": (0, 0, 32, 2, 1),
        "weak": (0, 0, 32, 105, 1), "defined": (0, 0, 32, 2, 0),
    }[target]
    records = bytearray(name.ljust(8, b"\0") + struct.pack("<IhHBB", value, section, kind, storage, aux))
    if aux:
        records.extend(struct.pack("<II10x", 2, 2) if target == "weak" else bytes(18))
        weak_index += 1
    records.extend(weak_name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, 0, weak_type, 105, 1))
    tag = 2 if target == "defined" else target_index
    if malformed == "index":
        tag = 65535
    records.extend(struct.pack("<II10x", tag, mode))
    if malformed == "unused":
        records[-1] = 1
    if duplicate is not None:
        records.extend(duplicate.ljust(8, b"\0") + struct.pack("<IhHBB", 0, 0, 32, 2, 0))
    raw[at + count * 18:at + count * 18] = records
    struct.pack_into("<I", raw, 12, count + len(records) // 18)
    if reference:
        raw[at:at] = struct.pack("<IIH", 1, weak_index, 20)
        struct.pack_into("<I", raw, 8, at + 10)
        struct.pack_into("<I", raw, 20 + 24, at)
        struct.pack_into("<H", raw, 20 + 32, 1)
        struct.pack_into("<H", raw, at + 10 + 18 + 4, 1)
    return bytes(raw)


@pytest.mark.parametrize("mode", [1, 2, 3])
@pytest.mark.parametrize("owner", ["winner", "loser", "unselected"])
def test_incoming_use_keeps_complete_distinct_any_family_without_resolving_alias(tmp_path, mode, owner):
    raw = incoming(object_bytes(b"\x90\xc3" if owner != "winner" else b"\xc3"), mode=mode)
    context = fixture(tmp_path, **({owner: raw} if owner != "unselected" else {}))
    if owner == "unselected":
        add_archive(context, [("unused.obj/", raw)], [])
    with linked_build(context) as native, comdat_selection(native) as selected:
        removal = selected.loser("dependency.obj", 1)
        certificate, = selected.certificates()
        assert removal["disposition"] == "discarded_any_duplicate"
        assert len(certificate["candidates"]) == (3 if owner == "unselected" else 2)
        assert len({row["object_sha256"] for row in certificate["candidates"]}) >= 2
        use, = certificate["incoming_weak_references"]
        assert use["weak_symbol"]["name"] == "_e"
        assert use["fallback_symbol"]["name"] == "_f"
        assert use["fallback_symbol"]["section"] == 0
        assert use["search_mode"] == mode and use["resolution"] == "unknown"
        assert certificate["source_authority"] == "not_transferred"
        assert certificate["outside_scope_verification"] == "unknown"


@pytest.mark.parametrize("options", [
    {"target": "defined"}, {"target": "local"}, {"target": "common"},
    {"target": "absolute"}, {"target": "weak"}, {"target": "data"},
    {"target": "auxiliary"}, {"weak_name": b"_f"}, {"weak_type": 0},
    {"duplicate": b"_e"}, {"duplicate": b"_f"},
    {"mode": 0}, {"mode": 4}, {"malformed": "index"}, {"malformed": "unused"},
])
def test_unsupported_targets_ambiguous_names_and_malformed_edges_remain_refused(tmp_path, options):
    context = fixture(tmp_path, loser=incoming(object_bytes(b"\x90\xc3"), **options))
    with pytest.raises(ValueError):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("unit0.obj", 1)


def test_incoming_use_does_not_hide_a_different_conflicting_same_key_record(tmp_path):
    context = fixture(tmp_path, loser=incoming(object_bytes(b"\x90\xc3")))
    add_archive(context, [("conflict.obj/", object_bytes(declaration=(4, 0, 2)))], [])
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="conflicting declaration"):
            selected.loser("dependency.obj", 1)


def test_reference_only_archive_member_does_not_become_a_definition_candidate(tmp_path):
    context = fixture(tmp_path)
    raw = incoming(object_bytes(b"\x90\x90\xc3", key=b"_other"))
    archive, _ = add_archive(context, [("reference.obj/", raw)], [])
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.loser("dependency.obj", 1)["disposition"] == "discarded_any_duplicate"
        certificate, = selected.certificates()
        assert len(certificate["candidates"]) == 2
        assert certificate["incoming_weak_references"][0]["input"]["path"] == str(archive)
        assert all(row["input"]["path"] != str(archive) for row in certificate["candidates"])


def test_allocation_consumer_uses_removal_without_weak_resolution_permission(tmp_path):
    context = fixture(tmp_path, loser=incoming(object_bytes(b"\x90\xc3")))
    with linked_build(context) as native:
        report = native.ordinary_allocations()
    removed, = [row for row in report["sections"] if row["input"] == {"path": str(context.objects[1])}]
    assert removed["size"] == 16 and removed["disposition"] == "discarded_any_duplicate"
    assert "envelope" not in removed
    certificate, = report["comdat_removal_certificates"]
    assert certificate["incoming_weak_references"][0]["resolution"] == "unknown"
    assert not any(node["owner"] == "dependency.obj" for node in report["code"]["nodes"])
    assert not report["ready_for_collection"]


def test_actual_weak_call_still_requires_independent_alias_resolution(tmp_path):
    context = fixture(tmp_path, winner=incoming(object_bytes(b"\xe8\0\0\0\0\xc3"), reference=True))
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x210] = b"\xe8\xfb\xff\xff\xff\xc3" + b"\xcc" * 10
    update_artifact(context, "app.exe", bytes(image))
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="unsupported linked function relocation identity"):
            selected.loser("dependency.obj", 1)
        assert selected.certificates() == []
        graph = native.code([{"owner": "unit0.obj", "symbol": "_f"}])
        assert graph["result"] == "unknown"
        assert "unsupported linked function relocation identity" in graph["nodes"][0]["reason"]


def test_incoming_reports_are_defensive_and_handle_expires(tmp_path):
    context = fixture(tmp_path, loser=incoming(object_bytes(b"\x90\xc3")))
    with linked_build(context) as native, comdat_selection(native) as selected:
        selected.loser("dependency.obj", 1)
        selected.certificates()[0]["incoming_weak_references"][0]["fallback_symbol"]["section"] = 1
        assert selected.certificates()[0]["incoming_weak_references"][0]["fallback_symbol"]["section"] == 0
    with pytest.raises(ValueError, match="expired"):
        selected.loser("dependency.obj", 1)


@pytest.mark.parametrize("damage", ["physical", "parsed", "certificate"])
def test_incoming_use_stays_inside_physical_and_derived_guards(tmp_path, damage):
    context = fixture(tmp_path, loser=incoming(object_bytes(b"\x90\xc3")))
    with pytest.raises(ValueError, match="changed"):
        with linked_build(context) as native, comdat_selection(native) as selected:
            selected.loser("dependency.obj", 1)
            if damage == "physical":
                path = context.objects[1]
                path.write_bytes(path.read_bytes() + b"changed")
            elif damage == "parsed":
                obj = next(obj for obj in _entry(selected)["objects"].values()
                           if any(symbol.storage == 105 for symbol in obj.symbols.values()))
                obj.auxiliary.clear()
            else:
                _entry(selected)["certificates"]["_f"]["incoming_weak_references"].clear()


def replace_object(context, index, raw):
    path = context.objects[index]
    path.write_bytes(raw)
    if index == 0:
        context.report["compile_link"]["compilations"][0]["compilation"]["output"]["after"] = stamp(raw)
    evidence = json.loads((context.directory / "link.json").read_bytes())
    for row in evidence["files"]:
        if row["path"] == str(path):
            row["before"] = row["after"] = stamp(raw)
    set_link(context, evidence, (context.directory / "link.log").read_text())


@pytest.mark.parametrize("key,index", [(b"_K", 0), (b"_J", 1)])
@pytest.mark.parametrize("mode", [1, 2, 3])
@pytest.mark.parametrize("fpo", [False, True])
def test_folded_any_and_peer_namespaces_keep_incoming_uses_and_every_member(tmp_path, key, index, mode, fpo):
    from test_linked_comdat_icf import fixture as folded_fixture
    context = folded_fixture(tmp_path, fpo=fpo)
    replace_object(context, index, incoming(context.objects[index].read_bytes(), key=key, mode=mode))
    with linked_build(context) as native, comdat_selection(native) as selected:
        removal = selected.loser("dependency.obj", 3 if fpo else 2)
        assert removal["disposition"] == "discarded_any_duplicate"
        certificate, = selected.certificates()
        group, = selected.folded_groups()
        assert certificate["winner"]["owner"] == "unit0.obj"
        assert certificate["final_storage"]["owner"] == "dependency.obj"
        assert {(row["owner"], row["root"]["name"]) for row in group["members"]} == {
            ("unit0.obj", "_K"), ("dependency.obj", "_J")}
        assert all(row["proof"]["result"] == "pass" for row in group["members"])
        if fpo:
            assert all(len(row["proof"]["debug_regions"]) == 1 for row in group["members"])
        use, = group["incoming_weak_references"]
        assert use["fallback_symbol"]["name"] == key.decode()
        assert use["search_mode"] == mode and use["resolution"] == "unknown"
        assert group["source_authority"] == "not_transferred"
        assert group["outside_scope_verification"] == "unknown"


@pytest.mark.parametrize("damage", ["common", "direct_weak", "defined", "missing_fpo"])
def test_folded_peer_conflicts_and_missing_frames_are_not_hidden_by_incoming_use(tmp_path, damage):
    from test_linked_comdat_icf import fixture as folded_fixture
    context = folded_fixture(tmp_path, fpo=damage == "missing_fpo",
                             damage="missing_fpo" if damage == "missing_fpo" else None)
    options = {"weak_name": b"_J"} if damage == "direct_weak" else {
        "target": "defined"} if damage == "defined" else {}
    replace_object(context, 1, incoming(context.objects[1].read_bytes(), key=b"_J", **options))
    if damage == "common":
        add_archive(context, [("conflict.obj/", object_bytes(key=b"_J", declaration=(4, 0, 2)))], [])
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError):
            selected.loser("dependency.obj", 3 if damage == "missing_fpo" else 2)
        assert selected.certificates() == [] and selected.folded_groups() == []


def test_folded_members_actual_weak_calls_still_refuse_alias_resolution(tmp_path):
    from test_linked_comdat_icf import fixture as folded_fixture
    context = folded_fixture(tmp_path)
    for index in (0, 1):
        raw = bytearray(context.objects[index].read_bytes())
        body, = struct.unpack_from("<I", raw, 20 + 20)
        raw[body:body + 16] = b"\xe8\0\0\0\0\xc3" + b"\xcc" * 10
        replace_object(context, index, incoming(bytes(raw), key=b"_K", reference=True))
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x210] = b"\xe8\xfb\xff\xff\xff\xc3" + b"\xcc" * 10
    update_artifact(context, "app.exe", bytes(image))
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError, match="unsupported linked function relocation identity"):
            selected.loser("dependency.obj", 2)
        assert selected.certificates() == [] and selected.folded_groups() == []
