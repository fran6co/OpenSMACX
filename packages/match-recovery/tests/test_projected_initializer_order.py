"""Global startup order from physical synthetic PE/COFF and native receipts.

These are bounded linkage controls, not runtime equivalence or game acceptance.
No compiler/provider executes; linked observations and source/output binding are
the real native implementations, not fabricated passing report dictionaries.
"""

from copy import deepcopy
import json
import struct

import pytest

from match_recovery.linked_initializers import compare_build_initializers, observe_build_initializers
from match_recovery.initializer_order import _input_identity
from test_build_graph_commands import stamp
from test_linked_initializers import update_artifact
from test_startup_linked_consumption import context


def selection(**changes):
    return {"entity_id": "entity-" + "a" * 64, "unit": "src/unit1.cpp", "version": "current",
            "symbol": "_$E1", "offset": 0, "evidence": ["Independently reviewed scoped registration"],
            **changes}


def pair(tmp_path, old=(b"\xc3",), new=(b"\xc3", b"\xc3"), *, damage=None):
    baseline, _ = context(tmp_path / "baseline", codes=old)
    current, _ = context(tmp_path / "current", codes=new, damage=damage)
    return baseline, current


def check(baseline, current, changes):
    return compare_build_initializers(baseline, current, generated_changes=changes)


def swap_slots(linked):
    raw = bytearray((linked.directory / "app.exe").read_bytes())
    raw[1028:1032], raw[1032:1036] = raw[1032:1036], raw[1028:1032]
    update_artifact(linked, "app.exe", raw)


def repeated_root(linked):
    """One physical initializer root with two identical table occurrences."""
    path = linked.objects[0]
    raw = bytearray(path.read_bytes())
    table_data, table_relocations = struct.unpack_from("<II", raw, 60 + 20)
    symbol_table, = struct.unpack_from("<I", raw, 8)
    assert table_relocations == table_data + 4 and symbol_table == table_relocations + 10
    raw = (raw[:table_data] + bytes(8) + struct.pack("<IIHIIH", 0, 0, 6, 4, 0, 6)
           + raw[symbol_table:])
    struct.pack_into("<I", raw, 8, symbol_table + 14)
    struct.pack_into("<I", raw, 60 + 16, 8)
    struct.pack_into("<I", raw, 60 + 24, table_data + 8)
    struct.pack_into("<H", raw, 60 + 32, 2)
    path.write_bytes(raw)
    receipt = json.loads((linked.directory / "link.json").read_text())
    record = next(row for row in receipt["files"] if row["path"] == str(path))
    record["before"] = record["after"] = stamp(raw)
    update_artifact(linked, "link.json", json.dumps(receipt).encode())
    image = bytearray((linked.directory / "app.exe").read_bytes())
    image[1032:1036] = image[1028:1032]
    struct.pack_into("<H", image, 512 + 0x180 + 10, 0x3008)
    update_artifact(linked, "app.exe", image)
    mapping = (linked.directory / "app.map").read_text().replace(
        "00000004H .CRT$XCU", "00000008H .CRT$XCU").replace(
        "0002:00000008 00000004H .CRT$XCZ", "0002:0000000c 00000004H .CRT$XCZ")
    update_artifact(linked, "app.map", mapping.encode())


def test_explicit_added_slot_preserves_global_unowned_order_not_whole_startup_equivalence(tmp_path):
    baseline, current = pair(tmp_path)
    changes = [selection()]
    before = deepcopy(changes)
    report = check(baseline, current, changes)
    order = report["projected_order"]
    assert order["result"] == "pass", order
    assert order["baseline"]["sequence"] == order["current"]["sequence"]
    assert len(order["baseline"]["sequence"]) == 1
    assert not order["baseline"]["covered"] and len(order["current"]["covered"]) == 1
    assert report["ordered_code_correspondence"] == report["result"] == "fail"
    assert report["remaining_linked_effects"] == "unknown"
    assert not report["ready_for_collection"] and changes == before


def test_explicit_removed_slot_is_version_specific(tmp_path):
    baseline, current = pair(tmp_path, old=(b"\xc3", b"\xc3"), new=(b"\xc3",))
    order = check(baseline, current, [selection(version="baseline")])["projected_order"]
    assert order["result"] == "pass", order
    assert len(order["baseline"]["covered"]) == 1 and not order["current"]["covered"]


def test_explicit_empty_selection_proves_only_unchanged_unowned_order(tmp_path):
    baseline, current = pair(tmp_path, new=(b"\xc3",))
    order = check(baseline, current, [])["projected_order"]
    assert order["result"] == "pass", order
    assert not order["baseline"]["covered"] and not order["current"]["covered"]


def test_unapproved_added_identical_root_remains_an_order_difference(tmp_path):
    baseline, current = pair(tmp_path)
    order = check(baseline, current, [])["projected_order"]
    assert order["result"] == "fail"
    assert order["mismatch_count"] == len(order["mismatches"]) == 1
    row, = order["mismatches"]
    assert row["index"] == 1 and row["baseline"] is None
    assert row["current"] == {key: order["current"]["sequence"][1][key]
                              for key in ("identity", "body_fingerprint")}


def test_global_swap_of_identical_root_bytes_is_not_hidden_by_fingerprints(tmp_path):
    baseline, current = pair(tmp_path, old=(b"\xc3", b"\xc3"), new=(b"\xc3", b"\xc3"), damage=swap_slots)
    report = check(baseline, current, [])
    assert report["ordered_code_correspondence"] == "pass"
    assert report["projected_order"]["result"] == "fail", report["projected_order"]
    order = report["projected_order"]
    assert order["mismatch_count"] == len(order["mismatches"]) == 2
    assert [row["index"] for row in order["mismatches"]] == [0, 1]
    for row in order["mismatches"]:
        assert row["baseline"]["body_fingerprint"] == row["current"]["body_fingerprint"]
        assert row["baseline"]["identity"] != row["current"]["identity"]


def test_object_basename_changes_do_not_replace_bound_source_ownership(tmp_path):
    baseline, _ = context(tmp_path / "baseline", codes=(b"\xc3", b"\xc3"), owners=("left.obj", "right.obj"))
    current, _ = context(tmp_path / "current", codes=(b"\xc3", b"\xc3"), owners=("moved-left.obj", "moved-right.obj"))
    assert check(baseline, current, [])["projected_order"]["result"] == "pass"


@pytest.mark.parametrize("changes", [
    {"unit": "src/missing.cpp"}, {"unit": "src/dependency.cpp"}, {"version": "baseline"},
    {"offset": 4}, {"symbol": "_$E2"},
])
def test_unused_wrong_selection_cannot_pass(tmp_path, changes):
    baseline, current = pair(tmp_path)
    assert check(baseline, current, [selection(**changes)])["projected_order"]["result"] == "unknown"


def test_unowned_root_body_change_remains_a_failure_after_approved_addition(tmp_path):
    baseline, current = pair(tmp_path, new=(b"\x90\xc3", b"\xc3"))
    assert check(baseline, current, [selection()])["projected_order"]["result"] == "fail"


def test_explicit_two_version_change_preserves_only_unowned_sequence(tmp_path):
    baseline, current = pair(tmp_path, new=(b"\x90\xc3",))
    changes = [selection(unit="src/unit0.cpp", version=version) for version in ("baseline", "current")]
    report = check(baseline, current, changes)
    order = report["projected_order"]
    assert order["result"] == "pass", order
    assert order["baseline"]["sequence"] == order["current"]["sequence"] == []
    assert len(order["baseline"]["covered"]) == len(order["current"]["covered"]) == 1
    assert report["result"] == "fail" and report["remaining_linked_effects"] == "unknown"


def test_unowned_external_reference_identity_is_not_hidden_by_equal_addresses(tmp_path):
    code = b"\xe8\0\0\0\0\xc3"
    baseline, _ = context(tmp_path / "baseline", code, reference="_dep")
    current, _ = context(tmp_path / "current", code, reference="_other")
    assert check(baseline, current, [])["projected_order"]["result"] == "fail"


def test_repeated_unselected_root_retains_multiplicity(tmp_path):
    baseline, current = pair(tmp_path, new=(b"\xc3",), damage=repeated_root)
    observed = observe_build_initializers(current)
    assert observed["result"] == "pass" and len(observed["sequence"]) == 2, observed
    assert check(baseline, current, [])["projected_order"]["result"] == "fail"


def test_selected_repeated_target_is_ambiguous_even_with_exact_local_offset(tmp_path):
    baseline, current = pair(tmp_path, new=(b"\xc3",), damage=repeated_root)
    assert observe_build_initializers(current)["result"] == "pass"
    order = check(baseline, current, [selection(unit="src/unit0.cpp")])["projected_order"]
    assert order["result"] == "unknown", order


def test_native_compile_receipt_must_match_actual_linked_object(tmp_path, monkeypatch):
    import test_startup_linked_consumption as fixture

    baseline, _ = context(tmp_path / "baseline")
    calls = 0

    def wrong_first_receipt(raw):
        nonlocal calls
        calls += 1
        result = stamp(raw)
        return {**result, "sha256": "f" * 64} if calls == 1 else result

    # Establish the controlled mismatched receipt before the immutable epoch;
    # linked inputs and final guard still independently use real physical bytes.
    monkeypatch.setattr(fixture, "stamp", wrong_first_receipt)
    current, _ = context(tmp_path / "current")
    assert observe_build_initializers(current)["result"] == "pass"
    assert check(baseline, current, [])["projected_order"]["result"] == "unknown"


def test_expiring_context_cannot_publish_an_order_result(tmp_path):
    baseline, current = pair(tmp_path)

    def expire(checks):
        if checks >= 4:
            current.alive = False

    current.on_check = expire
    with pytest.raises(ValueError, match="expired"):
        check(baseline, current, [selection()])


def identity_paths(tmp_path):
    root = tmp_path / "project"
    directory = root / "build"
    directory.mkdir(parents=True)
    return root, directory


def test_same_basename_external_inputs_keep_distinct_physical_identities(tmp_path):
    root, directory = identity_paths(tmp_path)
    identities = []
    for parent in ("vendor-a", "vendor-b"):
        path = tmp_path / parent / "runtime.lib"
        path.parent.mkdir()
        path.write_bytes(b"identical synthetic input")
        identities.append(_input_identity(str(path), root, directory))
    assert identities[0] != identities[1]
    assert all(row["kind"] == "external" for row in identities)
    assert {row["path"] for row in identities} == {
        str(tmp_path / parent / "runtime.lib") for parent in ("vendor-a", "vendor-b")}


def test_project_and_build_inputs_have_distinct_namespace_tags(tmp_path):
    root, directory = identity_paths(tmp_path)
    for path in (root / "runtime.lib", directory / "runtime.lib"):
        path.write_bytes(b"same bytes do not merge ownership")
    assert _input_identity(root / "runtime.lib", root, directory) == {
        "kind": "project", "path": "runtime.lib"}
    assert _input_identity(directory / "runtime.lib", root, directory) == {
        "kind": "build", "path": "runtime.lib"}


def test_root_prefix_lookalike_is_not_rebased_into_project(tmp_path):
    root, directory = identity_paths(tmp_path)
    path = tmp_path / "project-other" / "runtime.lib"
    path.parent.mkdir()
    path.write_bytes(b"external physical input")
    assert str(path).startswith(str(root))
    assert _input_identity(path, root, directory) == {"kind": "external", "path": str(path)}


@pytest.mark.parametrize("kind", ["symlink", "parent-symlink", "dotdot", "relative", "missing", "directory"])
def test_noncanonical_or_nonphysical_input_identity_refuses(tmp_path, kind):
    root, directory = identity_paths(tmp_path)
    target = root / "runtime.lib"
    target.write_bytes(b"physical input")
    if kind == "symlink":
        path = root / "alias.lib"
        path.symlink_to(target)
    elif kind == "parent-symlink":
        alias = tmp_path / "alias-root"
        alias.symlink_to(root, target_is_directory=True)
        path = alias / "runtime.lib"
    elif kind == "dotdot":
        path = directory / ".." / "runtime.lib"
    elif kind == "relative":
        path = "runtime.lib"
    elif kind == "missing":
        path = root / "missing.lib"
    else:
        path = directory
    with pytest.raises(ValueError, match="canonical physical input"):
        _input_identity(path, root, directory)


@pytest.mark.parametrize("mode", ["symbol", "body", "added", "removed", "equal"])
def test_mismatch_diagnostics_are_bounded_and_do_not_change_order_verdict(monkeypatch, mode):
    from match_recovery import initializer_order as native

    # Exercise only diagnostic formatting over controlled projection rows.
    # The physical controls above independently cover actual PE/source binding.
    old = [{"identity": {"kind": "compiled", "unit": "src/unit.cpp", "symbol": "_$E" + str(index)},
            "body_fingerprint": "a" * 64, "slot": 0x401000 + 4 * index, "offsets": [4 * index]}
           for index in range(21)]
    new = deepcopy(old)
    if mode == "symbol":
        for row in new:
            row["identity"]["symbol"] += "renumbered"
    elif mode == "body":
        for row in new:
            row["body_fingerprint"] = "b" * 64
    elif mode == "added":
        old = []
    elif mode == "removed":
        new = []
    projected = {"baseline": old, "current": new}
    monkeypatch.setattr(native, "_project", lambda context, observed, changes: {
        "sequence": projected[context], "covered": []})
    report = native._compare_order("baseline", "current", {"result": "pass"}, {"result": "pass"}, [])
    assert report["baseline"]["sequence"] == old and report["current"]["sequence"] == new
    assert report["remaining_linked_effects"] == "unknown" and not report["ready_for_collection"]
    if mode == "equal":
        assert report["result"] == "pass"
        assert "mismatch_count" not in report and "mismatches" not in report
    else:
        assert report["result"] == "fail"  # Renumbering is still not an alias.
        assert report["mismatch_count"] == 21 and len(report["mismatches"]) == 16
        assert [row["index"] for row in report["mismatches"]] == list(range(16))
        for row in report["mismatches"]:
            index = row["index"]
            for version, sequence in (("baseline", old), ("current", new)):
                expected = ({key: sequence[index][key] for key in ("identity", "body_fingerprint")}
                            if index < len(sequence) else None)
                assert row[version] == expected
