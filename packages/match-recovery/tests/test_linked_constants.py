"""Physical complete-population absolute definitions, never guessed zero VAs."""

from copy import deepcopy
from hashlib import sha256
import os
import struct

import pytest

from match_recovery.linked_objects import SelectedObjects
from test_coff_inventory import imported
from test_linked_objects import inventory, library_inputs
from test_link_input_coverage import inputs
from test_link_file_evidence import stamp


def symbol_object(specs, *, directives=None, machine=0x14C):
    """Ordinary physical COFF, including zero-section absolute definitions."""
    payload = directives or b""
    count = int(directives is not None)
    start = 20 + count * 40
    strings, records = bytearray(), bytearray()
    for name, value, section, kind, storage, auxiliary in specs:
        encoded = name.encode("ascii")
        if len(encoded) > 8:
            label = struct.pack("<II", 0, len(strings) + 4)
            strings.extend(encoded + b"\0")
        else:
            label = encoded.ljust(8, b"\0")
        records.extend(label + struct.pack("<IhHBB", value, section, kind, storage, len(auxiliary) // 18) + auxiliary)
    header = struct.pack("<HHIIIHH", machine, count, 0, start + len(payload), len(records) // 18, 0, 0)
    section = (b".drectve" + struct.pack("<IIIIIIHHI", 0, 0, len(payload), start, 0, 0, 0, 0, 0x00100A00)
               if count else b"")
    return header + section + payload + records + struct.pack("<I", len(strings) + 4) + strings


def constant(value=0, *, name="_K", section=-1, kind=0, storage=2, auxiliary=b"", **kwargs):
    return symbol_object([(name, value, section, kind, storage, auxiliary)], **kwargs)


def observed(root, *, members=None, finals=("constant.obj",)):
    return library_inputs(root, members or [("constant.obj/", constant())], finals)


@pytest.mark.parametrize("value", [0, 7, 0xFFFFFFFF])
def test_selected_archive_absolute_is_a_named_constant_not_an_image_address(tmp_path, value):
    raw = constant(value)
    resolver = SelectedObjects(*observed(tmp_path, members=[("constant.obj/", raw)]))
    result = resolver.resolve_constant("_K")
    assert result == {"symbol": "_K", "value": value, "owner": "RUNTIME:constant.obj",
                      "input": {"path": str(tmp_path / "runtime.lib"), "member": "constant.obj", "occurrence": 0},
                      "object_sha256": sha256(raw).hexdigest()}
    assert "address" not in result and "result" not in result


def test_standalone_selected_absolute_and_long_arbitrary_spelling(tmp_path):
    evidence, log = inputs(tmp_path)
    raw = constant(31, name="a_long_unfamiliar_constant")
    path = tmp_path / "app.obj"
    path.write_bytes(raw)
    row = next(item for item in evidence["files"] if item["path"] == str(path))
    row["before"] = row["after"] = stamp(raw)
    result = SelectedObjects(*inventory(evidence, log)).resolve_constant("a_long_unfamiliar_constant")
    assert result["owner"] == "app.obj" and result["input"] == {"path": str(path)}
    assert result["value"] == 31


@pytest.mark.parametrize("selected", [False, True])
def test_even_unselected_duplicate_definitions_refuse(tmp_path, selected):
    members = [("constant.obj/", constant()), ("duplicate.obj/", constant())]
    finals = ("constant.obj", "duplicate.obj") if selected else ("constant.obj",)
    with pytest.raises(ValueError, match="exactly one"):
        SelectedObjects(*observed(tmp_path, members=members, finals=finals)).resolve_constant("_K")


def test_duplicate_symbols_inside_one_physical_member_are_not_collapsed(tmp_path):
    raw = symbol_object([("_K", 0, -1, 0, 2, b"")] * 2)
    with pytest.raises(ValueError, match="exactly one"):
        SelectedObjects(*observed(tmp_path, members=[("constant.obj/", raw)])).resolve_constant("_K")


@pytest.mark.parametrize("other", [constant(4, section=0), constant(section=1, directives=b"/nologo"),
                                   imported("_K"), imported("K")])
def test_common_strong_and_import_candidates_cannot_be_overridden(tmp_path, other):
    name = "__imp_K" if other == imported("K") else "_K"
    members = [("constant.obj/", constant(name=name)), ("other.obj/", other)]
    with pytest.raises(ValueError, match="common|import"):
        SelectedObjects(*observed(tmp_path, members=members)).resolve_constant(name)


@pytest.mark.parametrize("weak_name", ["_K", "alias"])
def test_weak_definition_or_fallback_to_constant_refuses(tmp_path, weak_name):
    target = "other" if weak_name == "_K" else "_K"
    weak = symbol_object([(target, 0, 0, 0, 2, b""),
                          (weak_name, 0, 0, 0, 105, struct.pack("<II10x", 0, 3))])
    members = [("constant.obj/", constant()), ("weak.obj/", weak)]
    with pytest.raises(ValueError, match="weak alias"):
        SelectedObjects(*observed(tmp_path, members=members)).resolve_constant("_K")


@pytest.mark.parametrize("raw", [constant(kind=32), constant(auxiliary=bytes(18)), constant(machine=0),
                                constant(section=0), constant(storage=3)])
def test_nonordinary_or_nonexternal_absolute_definitions_do_not_qualify(tmp_path, raw):
    with pytest.raises(ValueError):
        SelectedObjects(*observed(tmp_path, members=[("constant.obj/", raw)])).resolve_constant("_K")


def test_unselected_only_definition_cannot_supply_the_linked_value(tmp_path):
    resolver = SelectedObjects(*observed(tmp_path, finals=()))
    with pytest.raises(ValueError, match="selected physical owner"):
        resolver.resolve_constant("_K")


@pytest.mark.parametrize("policy", ["/FORCE", "/DEF:custom.def", "/ALTERNATENAME:_K=other", "/EXPORT:_K=other", "@options.rsp"])
@pytest.mark.parametrize("origin", ["arguments", "unselected_directive"])
def test_unsupported_linker_policy_or_directives_refuse(tmp_path, policy, origin):
    if origin == "arguments":
        evidence, coverage, records = observed(tmp_path)
        evidence["arguments"].append(policy)
    else:
        members = [("constant.obj/", constant()), ("unselected.obj/", constant(name="other", directives=policy.encode()))]
        evidence, coverage, records = observed(tmp_path, members=members)
    with pytest.raises(ValueError, match="unsupported linker policy"):
        SelectedObjects(evidence, coverage, records).resolve_constant("_K")


@pytest.mark.parametrize("damage", ["remove_record", "remove_symbol", "change_value", "invent_symbol", "remove_directive"])
def test_forged_inventory_cannot_hide_physical_candidate_or_policy(tmp_path, damage):
    members = [("constant.obj/", constant()), ("unselected.obj/", constant(name="other", directives=b"/nologo"))]
    evidence, coverage, records = deepcopy(observed(tmp_path, members=members))
    if damage == "remove_record":
        records.pop()
        coverage["record_count"] -= 1
    elif damage == "remove_symbol":
        next(row for row in records if row.get("member") == "unselected.obj")["symbols"].clear()
    elif damage == "change_value":
        next(row for row in records if row.get("member") == "constant.obj")["symbols"][0]["value"] = 99
    elif damage == "invent_symbol":
        records[0]["symbols"].append({"name": "fabricated"})
    else:
        next(row for row in records if row.get("member") == "unselected.obj")["directives"].clear()
    with pytest.raises(ValueError, match="complete physical population"):
        SelectedObjects(evidence, coverage, records).resolve_constant("_K")


@pytest.mark.parametrize("damage", ["library_bytes", "unrelated_object", "symlink", "hardlink", "missing"])
@pytest.mark.parametrize("warm", [False, True])
def test_all_population_inputs_stay_physical_and_fresh(tmp_path, damage, warm):
    resolver = SelectedObjects(*observed(tmp_path))
    if warm:
        resolver.resolve_constant("_K")
    path = tmp_path / ("app.obj" if damage == "unrelated_object" else "runtime.lib")
    if damage in ("library_bytes", "unrelated_object"):
        path.write_bytes(path.read_bytes() + b"changed")
    elif damage == "missing":
        path.unlink()
    else:
        retained = tmp_path / "retained-input"
        path.rename(retained)
        if damage == "symlink":
            path.symlink_to(retained)
        else:
            os.link(retained, path)
    with pytest.raises((ValueError, OSError)):
        resolver.resolve_constant("_K")


def test_defensive_records_and_returned_bindings_cannot_refresh_epoch(tmp_path):
    original = observed(tmp_path)
    resolver = SelectedObjects(*original)
    original[0]["arguments"].append("/FORCE")
    original[2].clear()
    first = resolver.resolve_constant("_K")
    first["input"]["member"] = "invented.obj"
    first["value"] = 123
    assert resolver.resolve_constant("_K")["input"]["member"] == "constant.obj"
    assert resolver.resolve_constant("_K")["value"] == 0


@pytest.mark.parametrize("name", ["missing", "", " _K", "_K\n", "é"])
def test_missing_or_invalid_names_never_become_zero(tmp_path, name):
    with pytest.raises(ValueError):
        SelectedObjects(*observed(tmp_path)).resolve_constant(name)


def test_physical_mutation_during_final_selection_refuses(tmp_path, monkeypatch):
    resolver = SelectedObjects(*observed(tmp_path))
    original = resolver.resolve
    def changed(owner):
        result = original(owner)
        path = tmp_path / "app.obj"
        path.write_bytes(path.read_bytes() + b"changed after selection")
        return result
    monkeypatch.setattr(resolver, "resolve", changed)
    with pytest.raises(ValueError, match="changed during"):
        resolver.resolve_constant("_K")


def test_optional_lookup_returns_none_only_for_verified_no_candidate(tmp_path):
    resolver = SelectedObjects(*observed(tmp_path))
    assert resolver.resolve_constant("missing", optional=True) is None
    assert resolver.resolve_constant("_f", optional=True) is None  # Ordinary image function.
    assert resolver.resolve_constant("_K", optional=True)["value"] == 0


def test_no_constant_candidate_does_not_require_constant_selection_policy(tmp_path):
    evidence, coverage, records = observed(tmp_path)
    evidence["arguments"].append("/UNKNOWN:policy")
    resolver = SelectedObjects(evidence, coverage, records)
    assert resolver.resolve_constant("_f", optional=True) is None
    with pytest.raises(ValueError, match="unsupported linker policy"):
        resolver.resolve_constant("_K", optional=True)
    assert resolver.resolve_constant("_f", optional=True) is None


@pytest.mark.parametrize("raw", [constant(kind=32), constant(auxiliary=bytes(18)), constant(machine=0)])
def test_optional_lookup_cannot_turn_an_unsupported_constant_into_absence(tmp_path, raw):
    resolver = SelectedObjects(*observed(tmp_path, members=[("constant.obj/", raw)]))
    with pytest.raises(ValueError, match="unsupported definition"):
        resolver.resolve_constant("_K", optional=True)


@pytest.mark.parametrize("damage", ["duplicate", "unselected", "common", "import"])
def test_optional_lookup_cannot_fall_back_to_a_map_for_conflicting_constants(tmp_path, damage):
    members = [("constant.obj/", constant())]
    if damage != "unselected":
        members.append(("conflict.obj/", {"duplicate": constant(), "common": constant(4, section=0),
                                          "import": imported("_K")}[damage]))
    resolver = SelectedObjects(*observed(tmp_path, members=members, finals=() if damage == "unselected" else ("constant.obj",)))
    with pytest.raises(ValueError):
        resolver.resolve_constant("_K", optional=True)


def test_optional_lookup_tracks_weak_chains_to_absolute_definitions(tmp_path):
    first = symbol_object([("_K", 0, 0, 0, 2, b""),
                           ("alias", 0, 0, 0, 105, struct.pack("<II10x", 0, 3))])
    second = symbol_object([("alias", 0, 0, 0, 2, b""),
                            ("outer", 0, 0, 0, 105, struct.pack("<II10x", 0, 3))])
    members = [("constant.obj/", constant()), ("weak.obj/", first), ("outer.obj/", second)]
    resolver = SelectedObjects(*observed(tmp_path, members=members))
    for name in ("_K", "alias", "outer"):
        with pytest.raises(ValueError, match="weak alias"):
            resolver.resolve_constant(name, optional=True)


@pytest.mark.parametrize("warm", [False, True])
def test_optional_absence_still_requires_fresh_complete_physical_population(tmp_path, warm):
    resolver = SelectedObjects(*observed(tmp_path))
    if warm:
        assert resolver.resolve_constant("missing", optional=True) is None
    path = tmp_path / "runtime.lib"
    path.write_bytes(path.read_bytes() + b"modified")
    with pytest.raises(ValueError):
        resolver.resolve_constant("missing", optional=True)


def test_forged_absolute_removal_cannot_make_optional_absence(tmp_path):
    evidence, coverage, records = observed(tmp_path)
    next(row for row in records if row.get("member") == "constant.obj")["symbols"].clear()
    with pytest.raises(ValueError, match="complete physical population"):
        SelectedObjects(evidence, coverage, records).resolve_constant("_K", optional=True)


@pytest.mark.parametrize("optional", [False, True])
def test_bulk_lookup_equals_serial_bindings_without_dropping_requested_names(tmp_path, optional):
    members = [("constant.obj/", constant()), ("second.obj/", constant(17, name="_J"))]
    resolver = SelectedObjects(*observed(tmp_path, members=members, finals=("constant.obj", "second.obj")))
    names = ("_K", "_J", "absent", "_f") if optional else ("_K", "_J")
    serial = {name: resolver.resolve_constant(name, optional=optional) for name in names}
    assert resolver.resolve_constants(list(names), optional=optional) == serial
    assert list(resolver.resolve_constants(names, optional=optional)) == list(names)


@pytest.mark.parametrize("names", [None, "_K", (), [], {"_K"}, ["_K", "_K"], ["_K", 1], ["_K", ""],
                                   ["_K", "bad\n"], ["_K"] * 4097])
def test_bulk_lookup_requires_bounded_nonempty_unique_name_sequence(tmp_path, names):
    with pytest.raises(ValueError):
        SelectedObjects(*observed(tmp_path)).resolve_constants(names, optional=True)


def test_bulk_absent_batch_does_not_inherit_constant_policy_refusals(tmp_path):
    evidence, coverage, records = observed(tmp_path)
    evidence["arguments"].append("/UNKNOWN:policy")
    resolver = SelectedObjects(evidence, coverage, records)
    assert resolver.resolve_constants(("absent", "_f"), optional=True) == {"absent": None, "_f": None}
    with pytest.raises(ValueError, match="unsupported linker policy"):
        resolver.resolve_constants(("absent", "_K"), optional=True)


def test_bulk_refuses_entire_batch_if_one_constant_is_conflicted(tmp_path):
    members = [("constant.obj/", constant()), ("second.obj/", constant(17, name="_J")),
               ("duplicate.obj/", constant(17, name="_J"))]
    resolver = SelectedObjects(*observed(tmp_path, members=members, finals=("constant.obj", "second.obj")))
    with pytest.raises(ValueError, match="exactly one"):
        resolver.resolve_constants(("_K", "_J", "absent"), optional=True)


def test_bulk_final_guard_refuses_mutation_during_selected_resolution(tmp_path, monkeypatch):
    members = [("constant.obj/", constant()), ("second.obj/", constant(17, name="_J"))]
    resolver = SelectedObjects(*observed(tmp_path, members=members, finals=("constant.obj", "second.obj")))
    resolver.resolve_constants(("_K", "_J"))  # Warm selected objects and population caches.
    original = resolver.resolve
    calls = []
    def changed(owner):
        result = original(owner)
        calls.append(owner)
        if len(calls) == 1:
            path = tmp_path / "app.obj"
            path.write_bytes(path.read_bytes() + b"changed during bulk selection")
        return result
    monkeypatch.setattr(resolver, "resolve", changed)
    with pytest.raises(ValueError, match="changed during"):
        resolver.resolve_constants(("_K", "_J", "absent"), optional=True)
    assert len(calls) == 2  # The final batch guard, not per-name checks, refused.


def test_bulk_absence_reuses_one_before_and_after_population_guard(tmp_path, monkeypatch):
    import match_recovery.linked_objects as native
    resolver = SelectedObjects(*observed(tmp_path))
    resolver.resolve_constant("_K")
    physical, calls = native._physical, []
    def checked(path):
        calls.append(path)
        return physical(path)
    monkeypatch.setattr(native, "_physical", checked)
    names = ("absent", "_f", "other", "fourth", "fifth")
    assert resolver.resolve_constants(names, optional=True) == dict.fromkeys(names)
    assert len(calls) == 4  # Two physical population files, twice for the batch.


def test_single_lookup_delegates_to_identically_guarded_bulk_api(tmp_path, monkeypatch):
    resolver = SelectedObjects(*observed(tmp_path))
    bulk, calls = resolver.resolve_constants, []
    def checked(names, *, optional=False):
        calls.append((names, optional))
        return bulk(names, optional=optional)
    monkeypatch.setattr(resolver, "resolve_constants", checked)
    assert resolver.resolve_constant("_K")["value"] == 0
    assert resolver.resolve_constant("absent", optional=True) is None
    assert calls == [(("_K",), False), (("absent",), True)]
