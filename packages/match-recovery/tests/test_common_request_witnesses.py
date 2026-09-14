"""Physical retained source/COFF witnesses, never report-derived permissions."""

from contextlib import ExitStack
import json
import struct

import pytest

from match_recovery.common_request_witnesses import retained_witnesses
from match_recovery.linked_context import linked_build
from test_coff_inventory import common_object
from test_linked_initializers import update_artifact
from test_linked_modules import fixture as module_context, set_link
from test_link_file_evidence import stamp


UNIT = "src/unit.cpp"


def context(root, *, raw=None, selected=True, receipt=True):
    # The physical module fixture contains real PE/PDB/map/log/input artifacts
    # and exact command/compilation observations. No common PASS is fabricated.
    value = module_context(root)
    path = value.objects[0]
    raw = common_object() if raw is None else raw
    path.write_bytes(raw)
    evidence = json.loads((value.directory / "link.json").read_bytes())
    row = next(row for row in evidence["files"] if row["path"] == str(path))
    row["before"] = row["after"] = stamp(raw)
    log = (value.directory / "link.log").read_text()
    if not selected:
        log = log.replace(path.name + "\n", "")
    set_link(value, evidence, log)
    if receipt:
        value.report["compile_link"]["compilations"][0]["compilation"]["output"]["after"] = stamp(raw)
    return value


def pair(tmp_path, **options):
    return context(tmp_path / "before"), context(tmp_path / "after", **options)


def witness(values, *, excluded=frozenset(), requests=None):
    with ExitStack() as stack:
        old, new = [stack.enter_context(linked_build(value)) for value in values]
        return retained_witnesses(old, new, excluded_units=excluded,
                                  requests={"_g": 4} if requests is None else requests)


def test_exact_retained_source_witness_uses_each_native_receipt_not_cross_build_hash(tmp_path):
    raw = bytearray(common_object())
    struct.pack_into("<I", raw, 4, 123456)
    values = pair(tmp_path, raw=bytes(raw))
    result = witness(values)["_g"]
    assert result["unit"] == UNIT
    assert result["baseline"]["object_sha256"] != result["current"]["object_sha256"]
    for version, native in zip(("baseline", "current"), values):
        row = result[version]
        assert row["input"] == {"path": str(native.objects[0])}
        assert row["object_sha256"] == stamp(native.objects[0].read_bytes())["sha256"]
        assert row["symbol"] == "_g" and row["size"] == 4 and row["object_index"] == 1


def test_all_generated_units_are_excluded_even_when_their_objects_are_selected(tmp_path):
    with pytest.raises(ValueError, match="lacks a retained"):
        witness(pair(tmp_path), excluded=frozenset({UNIT}))


def test_same_basename_and_same_bytes_cannot_supply_a_unique_selected_owner(tmp_path):
    values = pair(tmp_path)
    current = values[1]
    original = current.objects[0]
    other = current.directory / "another" / original.name
    other.parent.mkdir()
    other.write_bytes(original.read_bytes())
    evidence = json.loads((current.directory / "link.json").read_bytes())
    evidence["explicit_inputs"].append(str(other))
    evidence["arguments"].append(str(other))
    evidence["files"].append({"path": str(other), "before": stamp(other.read_bytes()),
                              "after": stamp(other.read_bytes())})
    set_link(current, evidence, (current.directory / "link.log").read_text())
    with pytest.raises(ValueError):
        witness(values)


def test_duplicate_same_named_common_records_are_not_an_any_match_witness(tmp_path):
    raw = bytearray(common_object())
    symbols, count = struct.unpack_from("<II", raw, 8)
    end = symbols + 18 * count
    duplicate = raw[symbols + 18:symbols + 36]
    raw[end:end] = duplicate
    struct.pack_into("<I", raw, 12, count + 1)
    with pytest.raises(ValueError, match="ambiguous symbol"):
        witness(pair(tmp_path, raw=bytes(raw)))


def test_native_common_comparison_composes_with_non_generated_compiled_witness(tmp_path):
    from match_recovery.common_preservation import _compare_linked
    from test_common_request_coverage import common_context
    values = [common_context(tmp_path / name, request=bool(index))[0]
              for index, name in enumerate(("baseline", "current"))]
    with ExitStack() as stack:
        old, new = [stack.enter_context(linked_build(value)) for value in values]
        assert _compare_linked(old, new, ("_g",))["result"] == "pass"
        result = retained_witnesses(old, new, excluded_units=frozenset({"src/unit0.cpp"}), requests={"_g": 4})
        assert result["_g"]["unit"] == "src/dependency.cpp"
        assert result["_g"]["current"]["input"] == {"path": str(values[1].objects[1])}


@pytest.mark.parametrize("symbol_type", [0, 0x20])
def test_unrelated_compiled_undefined_reference_does_not_hide_valid_request(tmp_path, symbol_type):
    from match_recovery.common_preservation import _compare_linked
    from test_startup_linked_consumption import context as startup_context
    def configure(value):
        # Three real compiled owners: generated unit0, ordinary unit1 with an
        # undefined _g, and selected dependency with the retained common _g.
        reference, dependency = value.objects[1:]
        raw = bytearray(reference.read_bytes())
        start, count = struct.unpack_from("<II", raw, 8)
        raw[start + count * 18:start + count * 18] = struct.pack("<8sIhHBB", b"_g", 0, 0, symbol_type, 2, 0)
        struct.pack_into("<I", raw, 12, count + 1)
        reference.write_bytes(raw)
        raw = bytearray(dependency.read_bytes())
        start, = struct.unpack_from("<I", raw, 8)
        struct.pack_into("<I", raw, start + 18 + 8, 4)
        dependency.write_bytes(raw)
        evidence = json.loads((value.directory / "link.json").read_bytes())
        for row in evidence["files"]:
            if row["path"] in {str(reference), str(dependency)}:
                path = reference if row["path"] == str(reference) else dependency
                row["before"] = row["after"] = stamp(path.read_bytes())
        update_artifact(value, "link.json", json.dumps(evidence).encode())
        mapping = (value.directory / "app.map").read_text().replace(
            " entry point", " 0002:00000080 _g 00402080 <common>\n entry point")
        update_artifact(value, "app.map", mapping.encode())
    values = [startup_context(tmp_path / name, codes=(b"\xc3", b"\xc3"), damage=configure)[0]
              for name in ("before", "after")]
    with ExitStack() as stack:
        old, new = [stack.enter_context(linked_build(value)) for value in values]
        assert _compare_linked(old, new, ("_g",))["result"] == "pass"
        result = retained_witnesses(old, new, excluded_units=frozenset({"src/unit0.cpp"}), requests={"_g": 4})
        assert result["_g"]["unit"] == "src/dependency.cpp"


@pytest.mark.parametrize("version", [0, 1])
def test_both_original_source_epochs_are_guarded_after_physical_witness_read(tmp_path, monkeypatch, version):
    from match_recovery import common_request_witnesses as native
    from test_common_request_coverage import common_context
    values = [common_context(tmp_path / name)[0] for name in ("baseline", "current")]
    original = native._read
    changed = []
    def read(path, stamp):
        raw = original(path, stamp)
        if path == values[1].objects[1] and not changed:
            changed.append(True)
            source = values[version].root / "src/dependency.cpp"
            source.write_bytes(source.read_bytes() + b"changed during observation")
        return raw
    monkeypatch.setattr(native, "_read", read)
    with pytest.raises(ValueError, match="epoch changed"):
        witness(values, excluded=frozenset({"src/unit0.cpp"}))
    assert changed


@pytest.mark.parametrize("options", [{"selected": False}, {"receipt": False}])
def test_nonselected_or_wrong_receipt_cannot_supply_physical_witness(tmp_path, options):
    with pytest.raises(ValueError):
        witness(pair(tmp_path, **options))


@pytest.mark.parametrize("damage", ["size", "undefined", "strong", "absolute", "weak", "type", "machine"])
def test_noncommon_or_incompatible_native_record_refuses(tmp_path, damage):
    raw = bytearray(common_object())
    offset = struct.unpack_from("<I", raw, 8)[0] + 18
    if damage == "size": struct.pack_into("<I", raw, offset + 8, 8)
    elif damage == "undefined": struct.pack_into("<I", raw, offset + 8, 0)
    elif damage == "strong": struct.pack_into("<h", raw, offset + 12, 1)
    elif damage == "absolute": struct.pack_into("<h", raw, offset + 12, -1)
    elif damage == "weak": raw[offset + 16] = 105
    elif damage == "type": struct.pack_into("<H", raw, offset + 14, 0x20)
    else: struct.pack_into("<H", raw, 0, 0)
    with pytest.raises(ValueError):
        witness(pair(tmp_path, raw=bytes(raw)))


@pytest.mark.parametrize("requests", [{"_absent": 4}, {"_g": 4, "_absent": 4}, {}, {"_g": True}, {"_g": 0}])
def test_no_partial_success_or_untyped_requests(tmp_path, requests):
    with pytest.raises(ValueError):
        witness(pair(tmp_path), requests=requests)


@pytest.mark.parametrize("excluded", [set(), (UNIT,), frozenset({"../unit.cpp"})])
def test_generated_domain_is_immutable_and_canonical(tmp_path, excluded):
    with pytest.raises(ValueError):
        witness(pair(tmp_path), excluded=excluded)


def test_expired_native_context_and_changed_physical_output_refuse(tmp_path):
    values = pair(tmp_path)
    with ExitStack() as stack:
        old, new = [stack.enter_context(linked_build(value)) for value in values]
        assert retained_witnesses(old, new, excluded_units=frozenset(), requests={"_g": 4})
    with pytest.raises(ValueError, match="expired"):
        retained_witnesses(old, new, excluded_units=frozenset(), requests={"_g": 4})
    with pytest.raises(ValueError):
        with ExitStack() as stack:
            old, new = [stack.enter_context(linked_build(value)) for value in values]
            values[1].objects[0].write_bytes(common_object() + b"changed")
            retained_witnesses(old, new, excluded_units=frozenset(), requests={"_g": 4})
