"""Physical native ANY selection, not copied passing observer reports."""

import json
import pickle
from pathlib import Path
import struct

import pytest

from match_recovery.linked_comdat_selection import AnySelection, any_selection
from match_recovery.linked_context import linked_build
from match_recovery.linked_input_epoch import input_epoch
from test_linked_modules import fixture as module_fixture, set_link, set_modules, add_archive
from test_linked_initializers import update_artifact
from test_link_file_evidence import stamp
from test_ordinary_contributions import set_contributions
from test_pdb_contributions import contribution, module
from test_coff_inventory import imported


def object_bytes(code=b"\xc3", *, policy=2, child=False, alias=False, key=b"_f", weak=False, declaration=None):
    specs = [(b".text", code, 0, policy)]
    if child:
        specs.append((b".text$x", b"\x90\xc3", 1, 5))
    headers, payload, symbols = [], bytearray(), bytearray()
    header_size = 20 + 40 * len(specs)
    for number, (name, body, parent, selection) in enumerate(specs, 1):
        body += b"\xcc" * (-len(body) % 16)
        flags = 0x60501020 if selection else 0x60500020
        headers.append(name.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(body), header_size + len(payload),
                                                        0, 0, 0, 0, flags))
        payload.extend(body)
        symbols.extend(name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, 0, 3, 1))
        symbols.extend(struct.pack("<IHHIHB3x", len(body), 0, 0, 0, parent, selection))
        label = key if number == 1 else b"$tail"
        symbols.extend(label.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, 0x20 if number == 1 else 0,
                                                         2 if number == 1 else 6, 0))
    if alias:
        symbols.extend(b"_alias\0\0" + struct.pack("<IhHBB", 0, 1, 0, 2, 0))
    if declaration:
        value, section, storage = declaration
        # Replace the independent external definition with a conflicting shape.
        struct.pack_into("<IhHBB", symbols, 2 * 18 + 8, value, section, 0x20, storage, 0)
    if weak:
        symbols.extend(b"_weak\0\0\0" + struct.pack("<IhHBB", 0, 0, 0, 105, 1))
        symbols.extend(struct.pack("<II10x", 2, 3))
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, header_size + len(payload), len(symbols) // 18, 0, 0)
            + b"".join(headers) + payload + symbols + struct.pack("<I", 4))


def fixture(root, *, loser=None, winner=None, corrupt=False, sc=True, duplicate_sc=False,
            wrong_module=False, duplicate_map=False, option=None, missing_winner=False):
    context = module_fixture(root)
    outputs = context.objects
    outputs[0].write_bytes(object_bytes() if winner is None else winner)
    outputs[1].write_bytes(object_bytes(b"\x90\xc3") if loser is None else loser)
    context.report["compile_link"]["compilations"][0]["compilation"]["output"]["after"] = stamp(outputs[0].read_bytes())
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"].append("/INCREMENTAL:NO")
    for row in evidence["files"]:
        row["before"] = row["after"] = stamp(Path(row["path"]).read_bytes())
    if option:
        evidence["arguments"].append(option)
    log = (context.directory / "link.log").read_text()
    if missing_winner:
        log = log.replace(outputs[0].name + "\nEnd", "End")
        log = log.replace("\n" + outputs[0].name + "\n", "\n")
    set_link(context, evidence, log)
    text = ("selection\n Preferred load address is 00400000\n"
            " Address Publics by Value Rva+Base Lib:Object\n"
            f" 0001:00000000 _f 00401000 f {outputs[0].name}\n"
            + (f" 0001:00000020 _f 00401020 f {outputs[1].name}\n" if duplicate_map else "")
            + " entry point at 0001:00000000\n Static symbols\n")
    update_artifact(context, "app.map", text.encode())
    image = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<I", image, 0x98 + 16, 0x1000)
    image[0x200:0x210] = b"\xc3" + b"\xcc" * 15
    if corrupt:
        image[0x200] = 0x90
    update_artifact(context, "app.exe", bytes(image))
    set_modules(context, b"".join(module(str(path).encode(), str(path).encode()) for path in outputs))
    row = contribution(section=1, offset=0, size=16, flags=0x60501020, module=int(wrong_module))
    set_contributions(context, row * (2 if duplicate_sc else 1) if sc else b"")
    return context


def observe(context):
    with linked_build(context) as native:
        with any_selection(native) as selected:
            return selected.loser("dependency.obj", 1), selected.certificates()


def test_different_any_loser_body_has_exact_native_winner_and_complete_family(tmp_path):
    context = fixture(tmp_path / "project")
    result, certificates = observe(context)
    certificate, = certificates
    assert result["disposition"] == "discarded_any_duplicate"
    assert result["certificate_id"] == certificate["certificate_id"]
    assert result["sections"] == [1]
    assert certificate["result"] == "pass" and len(certificate["candidates"]) == 2
    assert len({row["object_sha256"] for row in certificate["candidates"]}) == 2
    assert certificate["winner"]["input"] == {"path": str(context.objects[0])}
    assert certificate["winner"]["owner"] == "unit0.obj"
    assert certificate["winner"]["address"] == 0x401000
    assert certificate["outside_scope_verification"] == "unknown" and not certificate["ready_for_collection"]
    assert certificate["source_authority"] == "not_transferred"


def test_complete_original_association_forest_follows_only_its_any_loser(tmp_path):
    context = fixture(tmp_path / "project", loser=object_bytes(b"\x90\xc3", child=True))
    with linked_build(context) as native, any_selection(native) as selected:
        root = selected.loser("dependency.obj", 1)
        child = selected.loser("dependency.obj", 2)
        assert root["sections"] == child["sections"] == [1, 2]
        assert root["certificate_id"] == child["certificate_id"]
        assert len(selected.certificates()) == 1
        assert selected.loser("unit0.obj", 1) is None
        with pytest.raises(ValueError, match="outside"):
            selected.loser("dependency.obj", 3)


@pytest.mark.parametrize("options", [dict(corrupt=True), dict(sc=False), dict(duplicate_sc=True),
    dict(wrong_module=True), dict(duplicate_map=True), dict(missing_winner=True),
    dict(loser=object_bytes(policy=1)), dict(loser=object_bytes(policy=0)),
    dict(loser=object_bytes(alias=True)), dict(winner=object_bytes(alias=True)),
    dict(loser=object_bytes(weak=True)), dict(loser=object_bytes(declaration=(4, 0, 2))),
    dict(loser=object_bytes(declaration=(0, -1, 2)))])
def test_missing_or_contradictory_native_winner_and_family_cannot_certify(tmp_path, options):
    context = fixture(tmp_path / "project", **options)
    with pytest.raises(ValueError):
        with linked_build(context) as native, any_selection(native) as selected:
            # Query the actual ANY winner: unsupported other definitions cannot
            # be silently omitted just because they are not themselves ANY.
            selected.loser("unit0.obj", 1)


@pytest.mark.parametrize("option", ["/FORCE:MULTIPLE", "/ALTERNATENAME:_f=_x", "/MERGE:.text=.data"])
def test_selection_changing_policy_is_not_hidden_by_matching_winner_bytes(tmp_path, option):
    with pytest.raises(ValueError):
        observe(fixture(tmp_path / "project", option=option))


def test_unrelated_strict_coff_refusal_does_not_poison_relevant_key(tmp_path):
    context = fixture(tmp_path / "project")
    unrelated = bytearray(object_bytes(b"\x90\xc3", key=b"_other"))
    at, = struct.unpack_from("<I", unrelated, 8)
    struct.pack_into("<I", unrelated, at + 18, 99)  # Invalid section definition, valid complete inventory.
    add_archive(context, [("unused.obj/", bytes(unrelated))], [])
    assert observe(context)[0]["disposition"] == "discarded_any_duplicate"


def test_conflicting_unselected_import_candidate_remains_in_complete_family(tmp_path):
    context = fixture(tmp_path / "project")
    add_archive(context, [("DEMO.dll/", imported("_f"))], [])
    with pytest.raises(ValueError, match="import"):
        observe(context)


def test_unselected_any_candidates_are_kept_without_inventing_selected_owners(tmp_path):
    context = fixture(tmp_path / "project")
    path, _ = add_archive(context, [("unused.obj/", object_bytes(b"\xcc\xc3"))], [])
    _, certificates = observe(context)
    assert len(certificates[0]["candidates"]) == 3
    candidate = next(row for row in certificates[0]["candidates"] if row["input"]["path"] == str(path))
    assert candidate["input"]["member"] == "unused.obj" and candidate["input"]["occurrence"] == 0
    assert "owner" not in candidate


def test_observation_is_defensive_and_native_handle_cannot_be_replayed(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, any_selection(native) as selected:
        original = selected.loser("dependency.obj", 1)
        selected.certificates()[0]["candidates"].clear()
        original["sections"].clear()
        assert selected.loser("dependency.obj", 1)["sections"] == [1]
        assert len(selected.certificates()[0]["candidates"]) == 2
        with pytest.raises(TypeError):
            pickle.dumps(selected)
    with pytest.raises(ValueError, match="expired"):
        selected.loser("dependency.obj", 1)
    with pytest.raises(TypeError):
        AnySelection()


def test_epoch_from_another_live_build_is_not_compatible(tmp_path):
    with linked_build(fixture(tmp_path / "first")) as first, linked_build(fixture(tmp_path / "second")) as second:
        with input_epoch(first.context, first.inputs.objects) as epoch:
            with pytest.raises(ValueError, match="another context"):
                with any_selection(second, epoch=epoch):
                    pass


@pytest.mark.parametrize("name", ["app.exe", "app.pdb", "app.map", "link.log", "link.json", "dependency.obj"])
def test_mutation_cannot_escape_batch_final_physical_guards(tmp_path, name):
    context = fixture(tmp_path / "project")
    with pytest.raises(ValueError, match="changed"):
        with linked_build(context) as native, any_selection(native) as selected:
            selected.loser("dependency.obj", 1)
            path = context.directory / name
            path.write_bytes(path.read_bytes() + b"damage")


def test_report_and_object_cache_mutation_cannot_escape_native_final_guard(tmp_path):
    from match_recovery import linked_comdat_selection as selection
    for mode in ("coff", "certificate", "index", "metadata", "population", "section"):
        context = fixture(tmp_path / mode)
        with pytest.raises(ValueError, match="changed"):
            with linked_build(context) as native, any_selection(native) as selected:
                selected.loser("dependency.obj", 1)
                state = selection._entry(selected)
                if mode == "coff":
                    next(iter(state["objects"].values())).symbols.clear()
                elif mode == "certificate":
                    state["certificates"]["_f"]["candidates"].clear()
                elif mode == "index":
                    state["map_index"].publics.clear()
                elif mode == "metadata":
                    object.__setattr__(state["metadata"][0], "modules", ())
                elif mode == "population":
                    state["code_candidates"] = ()
                else:
                    object.__setattr__(state["code_candidates"][0][3], "data", b"damage")


def test_constructor_and_nonany_query_need_no_pdb_or_selection_policy_waiver(tmp_path):
    context = fixture(tmp_path / "project", winner=object_bytes(policy=1), loser=object_bytes(policy=1),
                      option="/FORCE:MULTIPLE")
    context.report["outputs"] = [row for row in context.report["outputs"] if row["format"] != "msvc-pdb"]
    context.pin_outputs()
    with linked_build(context) as native, any_selection(native) as selected:
        assert selected.loser("dependency.obj", 1) is None
        assert selected.certificates() == []


def test_any_query_without_own_pdb_refuses_when_it_needs_a_winner(tmp_path):
    context = fixture(tmp_path / "project")
    context.report["outputs"] = [row for row in context.report["outputs"] if row["format"] != "msvc-pdb"]
    context.pin_outputs()
    with linked_build(context) as native, any_selection(native) as selected:
        with pytest.raises(ValueError, match="PDB"):
            selected.loser("dependency.obj", 1)


def test_map_owner_does_not_substitute_for_original_icf_member_provenance(tmp_path):
    context = fixture(tmp_path / "project")
    evidence = json.loads((context.directory / "link.json").read_bytes())
    log = (context.directory / "link.log").read_text().replace("Start Pass2", (
        "Selected symbol:\n _f from unit0.obj\nReplaced symbol(s):\n _f from dependency.obj\n"
        "ICF total savings: 16 bytes\nStart Pass2"))
    set_link(context, evidence, log)
    with linked_build(context) as native, any_selection(native) as selected:
        with pytest.raises(ValueError, match="ICF"):
            selected.loser("dependency.obj", 1)


def test_independently_placed_loser_descendant_label_blocks_discard(tmp_path):
    context = fixture(tmp_path / "project", loser=object_bytes(child=True))
    text = (context.directory / "app.map").read_text()
    update_artifact(context, "app.map", (text + " 0001:00000040 $tail 00401040 dependency.obj\n").encode())
    with linked_build(context) as native, any_selection(native) as selected:
        with pytest.raises(ValueError, match="contradictory independently placed"):
            selected.loser("dependency.obj", 2)


def test_associated_external_entry_cannot_disappear_with_parent(tmp_path):
    loser = bytearray(object_bytes(child=True))
    at, = struct.unpack_from("<I", loser, 8)
    loser[at + 5 * 18 + 16] = 2
    with pytest.raises(ValueError, match="additional external"):
        observe(fixture(tmp_path / "project", loser=bytes(loser)))


def test_relevant_unselected_strict_coff_failure_blocks_only_its_key(tmp_path):
    context = fixture(tmp_path / "project")
    malformed = bytearray(object_bytes())
    at, = struct.unpack_from("<I", malformed, 8)
    struct.pack_into("<I", malformed, at + 18, 99)
    add_archive(context, [("unused.obj/", bytes(malformed))], [])
    with pytest.raises(ValueError):
        observe(context)


def reference_context(root, *, wrong_target=False):
    raw = bytearray(object_bytes(b"\xe8\0\0\0\0\xc3"))
    at, count = struct.unpack_from("<II", raw, 8)
    raw[at:at] = struct.pack("<IIH", 1, 3, 20)
    struct.pack_into("<II", raw, 8, at + 10, count + 1)
    struct.pack_into("<I", raw, 20 + 24, at)
    struct.pack_into("<H", raw, 20 + 32, 1)
    struct.pack_into("<H", raw, at + 10 + 18 + 4, 1)
    end = at + 10 + count * 18
    raw[end:end] = b"_dest\0\0\0" + struct.pack("<IhHBB", 0, 0, 0x20, 2, 0)
    context = fixture(root, winner=bytes(raw))
    add_archive(context, [("dest.obj/", object_bytes(key=b"_dest"))], ["dest.obj"])
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x210] = b"\xe8" + struct.pack("<i", 0x3B if not wrong_target else 0x3C) + b"\xc3" + b"\xcc" * 10
    image[0x240:0x250] = b"\xc3" + b"\xcc" * 15
    update_artifact(context, "app.exe", bytes(image))
    text = (context.directory / "app.map").read_text().replace(" entry point", (
        " 0001:00000040 _dest 00401040 f RUNTIME:dest.obj\n entry point"))
    update_artifact(context, "app.map", text.encode())
    return context


def test_winner_references_must_match_independently_selected_linked_target(tmp_path):
    assert observe(reference_context(tmp_path / "good"))[0]["disposition"] == "discarded_any_duplicate"
    with pytest.raises(ValueError, match="code, references"):
        observe(reference_context(tmp_path / "damaged", wrong_target=True))


def test_many_loser_queries_reuse_one_winner_proof_and_do_not_rehash_epoch(tmp_path, monkeypatch):
    from match_recovery import linked_comdat_selection as selection
    real = selection.verify_code_region
    calls = []
    def observe_call(*args, **kwargs):
        calls.append(args[1].name)
        return real(*args, **kwargs)
    monkeypatch.setattr(selection, "verify_code_region", observe_call)
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, any_selection(native) as selected:
        first = selected.loser("dependency.obj", 1)
        checks = context.checks
        for _ in range(50):
            assert selected.loser("dependency.obj", 1) == first
        assert context.checks == checks
        assert calls == ["_f"]


def independent_sibling(*, identical=True):
    """B contains discarded K and retained J at the same-sized section shape."""
    raw = bytearray(object_bytes(child=True))
    at, = struct.unpack_from("<I", raw, 8)
    # The former child is now an independently keyed COMDAT, not in K's forest.
    struct.pack_into("<HB", raw, at + 4 * 18 + 12, 0, 2)
    raw[at + 5 * 18:at + 5 * 18 + 8] = b"_j\0\0\0\0\0\0"
    struct.pack_into("<HBB", raw, at + 5 * 18 + 14, 0x20, 2, 0)
    offset, = struct.unpack_from("<I", raw, 20 + 40 + 20)
    if identical:
        raw[offset:offset + 16] = b"\xc3" + b"\xcc" * 15
    return bytes(raw)


@pytest.mark.parametrize("reported_alias", [False, True])
def test_unreported_icf_cannot_borrow_discarded_same_key_copy_in_map_owner(tmp_path, reported_alias):
    # Actual scenario: A::K was chosen, then folded into B::J. B also supplied
    # an unselected K. MAP+matching B::K bytes+B's SC alone pick the wrong K.
    context = fixture(tmp_path, winner=independent_sibling(), loser=object_bytes())
    if reported_alias:
        text = (context.directory / "app.map").read_text().replace(
            " entry point", " 0001:00000000 _j 00401000 f unit0.obj\n entry point")
        update_artifact(context, "app.map", text.encode())
    with pytest.raises(ValueError, match="ICF competitor"):
        observe(context)


def test_immutable_difference_excludes_an_unmapped_independent_code_competitor(tmp_path):
    context = fixture(tmp_path, winner=independent_sibling(identical=False))
    certificate, = observe(context)[1]
    assert certificate["no_other_fold"]["result"] == "pass"
    assert certificate["no_other_fold"]["other_physical_code_sections"] == 1


def test_reserved_pdb_characteristics_remain_unexplained_not_equivalent(tmp_path):
    context = fixture(tmp_path)
    set_contributions(context, contribution(section=1, offset=0, size=16, flags=0x60503020, module=0))
    certificate, = observe(context)[1]
    winner = certificate["winner"]
    assert winner["object_flags"] == 0x60501020
    assert winner["pdb_flags"] == 0x60503020
    assert winner["characteristics_agreement"] == "unknown"
    assert certificate["outside_scope_verification"] == "unknown"
    assert not certificate["ready_for_collection"]


@pytest.mark.parametrize("bit", [0x20, 0x1000, 0x100000, 0x80000000])
def test_known_contribution_flag_contradictions_still_refuse(tmp_path, bit):
    context = fixture(tmp_path)
    set_contributions(context, contribution(section=1, offset=0, size=16, flags=0x60501020 ^ bit, module=0))
    with pytest.raises(ValueError):
        observe(context)


def test_possible_fold_in_unselected_archive_is_not_hidden(tmp_path):
    context = fixture(tmp_path)
    add_archive(context, [("unused.obj/", object_bytes(key=b"_j"))], [])
    with pytest.raises(ValueError, match="ICF competitor"):
        observe(context)


def test_same_key_forest_does_not_hide_identical_associated_code(tmp_path):
    raw = bytearray(object_bytes(child=True))
    offset, = struct.unpack_from("<I", raw, 20 + 40 + 20)
    raw[offset:offset + 16] = b"\xc3" + b"\xcc" * 15
    with pytest.raises(ValueError, match="ICF competitor"):
        observe(fixture(tmp_path, loser=bytes(raw)))


def test_unused_archive_directive_cannot_establish_actual_nonincremental_link(tmp_path):
    from test_binary import coff_fixture
    context = fixture(tmp_path)
    directive = b" /INCREMENTAL:NO "
    raw = bytearray(coff_fixture(directive))
    raw[20:28] = b".drectve"
    size, = struct.unpack_from("<I", raw, 20 + 16)
    raw[60 + len(directive):60 + size] = b" " * (size - len(directive))
    add_archive(context, [("unused.obj/", bytes(raw))], [])
    evidence = json.loads((context.directory / "link.json").read_bytes())
    evidence["arguments"].remove("/INCREMENTAL:NO")
    set_link(context, evidence, (context.directory / "link.log").read_text())
    with pytest.raises(ValueError, match="selection policy"):
        observe(context)
