"""Live, per-relocation COFF weak fallback proofs; never standalone aliases."""

import json
import hashlib
import pickle
import struct

import pytest

from match_recovery.linked_code_region import verify_code_region
from match_recovery.linked_context import linked_build
from match_recovery.link_input_coverage import _input_inventory
from match_recovery.linked_map_index import MapIndex
from match_recovery.link_file_evidence import parse_link_files
from match_recovery.linked_objects import SelectedObjects
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span

from test_linked_initializers import build, stamp, update_artifact


def weak_object(*, mode=2, addend=0, g_reference=False):
    """Caller E9 -> weak E; same physical object has fallback G definition."""
    caller = b"\xe9" + struct.pack("<i", addend) + b"\xcc" * 11
    fallback = (b"\xe8\0\0\0\0\xc3" + b"\xcc" * 10 if g_reference
                else b"\xc3" + b"\xcc" * 15)
    fallback_relocations = [(1, 9, 20)] if g_reference else []
    specs = [(caller, [(1, 7, 20)]), (fallback, fallback_relocations)]
    start = 20 + 40 * len(specs)
    headers, payload = [], bytearray()
    for number, (body, relocs) in enumerate(specs, 1):
        raw_at = start + len(payload)
        payload.extend(body)
        reloc_at = start + len(payload) if relocs else 0
        for reloc in relocs:
            payload.extend(struct.pack("<IIH", *reloc))
        headers.append(b".text\0\0\0" + struct.pack(
            "<IIIIIIHHI", 0, 0, len(body), raw_at, reloc_at, 0, len(relocs), 0, 0x60501020))

    def record(name, value, section, kind, storage, aux=b""):
        assert len(name) <= 8 and len(aux) % 18 == 0
        return name.encode().ljust(8, b"\0") + struct.pack(
            "<IhHBB", value, section, kind, storage, len(aux) // 18) + aux

    records = bytearray()
    records.extend(record(".text", 0, 1, 0, 3,
                          struct.pack("<IHHIHB3x", 16, 1, 0, 0, 0, 2)))
    records.extend(record("_caller", 0, 1, 0x20, 2))
    records.extend(record(".text", 0, 2, 0, 3,
                          struct.pack("<IHHIHB3x", 16, len(fallback_relocations), 0, 0, 0, 2)))
    records.extend(record("_G", 0, 2, 0x20, 2))
    records.extend(record("_G", 0, 0, 0x20, 2))
    records.extend(record("_E", 0, 0, 0x20, 105,
                          struct.pack("<II10x", 6, mode)))
    if g_reference:
        records.extend(record("_dep", 0, 0, 0x20, 2))
    count = 10 if g_reference else 9  # Includes section and weak auxiliary records.
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(payload), count, 0, 0)
            + b"".join(headers) + payload + records + struct.pack("<I", 4))


def weak_build(root, *, mode=2, addend=0, bad_map=False, bad_owner=False,
               bad_caller_owner=False, damaged_target=False, damaged_fallback=False,
               g_reference=False, damaged_g_reference=False):
    context = build(root, (b"\xe9\0\0\0\0",), owners=("unit0.obj",))
    obj_path = next(path for path in context.objects if path.name == "unit0.obj")
    obj_path.write_bytes(weak_object(mode=mode, addend=addend, g_reference=g_reference))
    receipt_path = context.directory / "link.json"
    receipt = json.loads(receipt_path.read_text())
    for row in receipt["files"]:
        if row["path"] == str(obj_path):
            row["before"] = row["after"] = stamp(obj_path.read_bytes())
    update_artifact(context, "link.json", json.dumps(receipt).encode())

    mapping = ("synthetic\n Preferred load address is 00400000\n"
               " Start         Length     Name                   Class\n"
               " 0001:00000000 00000200H .text CODE\n"
               " 0002:00000000 00000004H .CRT$XCA DATA\n"
               " 0002:00000004 00000004H .CRT$XCU DATA\n"
               " 0002:00000008 00000004H .CRT$XCZ DATA\n"
               " Address         Publics by Value              Rva+Base     Lib:Object\n"
               f" 0001:00000000 _caller 00401000 f {'dependency.obj' if bad_caller_owner else 'unit0.obj'}\n"
               f" 0001:{0x11 if bad_map else 0x10:08x} _E {0x401011 if bad_map else 0x401010:08x} f {'dependency.obj' if bad_owner else 'unit0.obj'}\n"
               f" 0001:00000010 _G 00401010 f {'dependency.obj' if bad_owner else 'unit0.obj'}\n"
               " 0001:000001e0 _dep 004011e0 f dependency.obj\n"
               " 0001:000001e0 _other 004011e0 f dependency.obj\n"
               " entry point at 0001:000001e0\n Static symbols\n")
    update_artifact(context, "app.map", mapping.encode())
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[512:517] = b"\xe9" + struct.pack("<i", (0x401010 + (1 if damaged_target else 0)) - 0x401005)
    raw[512 + 0x10] = 0x90 if damaged_fallback else 0xC3
    if g_reference:
        raw[512 + 0x10:512 + 0x16] = b"\xe8" + struct.pack(
            "<i", (0x4011E0 + (1 if damaged_g_reference else 0)) - 0x401015) + b"\xc3"
    update_artifact(context, "app.exe", bytes(raw))
    return context


def linked_graph(context):
    with linked_build(context) as live:
        return live.code([{"owner": "unit0.obj", "symbol": "_caller"}])


def test_live_code_graph_qualifies_one_mode2_fallback_and_keeps_weak_identity(tmp_path):
    report = linked_graph(weak_build(tmp_path / "valid"))
    assert report["result"] == "pass", report["gaps"]
    caller, fallback = report["nodes"]
    assert caller["symbol"] == "_caller" and fallback["symbol"] == "_G"
    edge = caller["edges"][0]
    assert edge["target"]["coff_weak_fallback"] == {
        "node": 1, "weak_symbol": "_E", "fallback_symbol": "_G", "search_mode": 2}
    ref = caller["proof"]["references"][0]
    assert ref["weak_resolution"]["relocation_offset"] == 1
    assert ref["address"] == ref["weak_resolution"]["map"]["fallback_address"] == 0x401010
    assert ref["weak_resolution"]["caller"]["object_sha256"] == caller["object_sha256"]
    assert ref["weak_resolution"]["fallback"]["body_fingerprint"] == fallback["proof"]["body_fingerprint"]
    assert report["remaining_linked_effects"] == "unknown" and not report["ready_for_collection"]


def test_live_code_graph_batches_multiple_roots_and_deduplicates_shared_nodes(tmp_path):
    context = weak_build(tmp_path / "multi-root")
    with linked_build(context) as live:
        report = live.code([{"owner": "unit0.obj", "symbol": "_caller"},
                            {"owner": "unit0.obj", "symbol": "_G"}])
    assert report["result"] == "pass", report["gaps"]
    assert [node["symbol"] for node in report["nodes"]] == ["_caller", "_G"]
    assert len({(node["owner"], node["symbol"]) for node in report["nodes"]}) == len(report["nodes"])


def test_standalone_region_does_not_resolve_weak_external_even_with_selected_objects(tmp_path):
    context = weak_build(tmp_path / "standalone")
    receipt = (context.directory / "link.json").read_bytes()
    coverage, records = _input_inventory(receipt, (context.directory / "link.log").read_bytes())
    objects = SelectedObjects(parse_link_files(receipt), coverage, records)
    obj = Coff((context.directory / "unit0.obj").read_bytes())
    mapping = parse_link_map((context.directory / "app.map").read_bytes())
    image_raw = (context.directory / "app.exe").read_bytes()
    import hashlib
    image = PeImage(image_raw, hashlib.sha256(image_raw).hexdigest())
    with pytest.raises(ValueError, match="unsupported linked function relocation identity"):
        verify_code_region(obj, obj.function("_caller"), "unit0.obj", mapping, image, objects=objects,
                           _index=MapIndex(mapping))


@pytest.mark.parametrize(("kwargs", "expected"), [
    ({"mode": 1}, "mode-2"),
    ({"mode": 3}, "mode-2"),
    ({"addend": 1}, "zero-addend"),
    ({"bad_map": True}, "unique same-owner equal-VA placements"),
    ({"bad_owner": True}, "same-owner equal-VA placements"),
    ({"bad_caller_owner": True}, "unique selected owner-qualified placement"),
])
def test_unsupported_weak_modes_and_ambiguous_effective_maps_fail_closed(tmp_path, kwargs, expected):
    report = linked_graph(weak_build(tmp_path / expected.replace(" ", "-"), **kwargs))
    assert report["result"] == "unknown"
    assert expected in report["gaps"][0]["reason"]


@pytest.mark.parametrize(("kwargs", "expected"), [
    ({"damaged_target": True}, "fail"),
    ({"damaged_fallback": True}, "unknown"),
])
def test_live_qualification_does_not_mask_pe_relocation_or_fallback_damage(tmp_path, kwargs, expected):
    report = linked_graph(weak_build(tmp_path / expected, **kwargs))
    assert report["result"] == expected, report["gaps"]
    if expected == "fail":
        assert report["nodes"][0]["proof"]["code"]["references"] == "fail"
    else:
        assert "fallback body or references" in report["gaps"][0]["reason"]


def test_live_weak_scope_expires_when_build_epoch_changes_during_observation(tmp_path):
    context = weak_build(tmp_path / "expired")
    with pytest.raises(ValueError, match="synthetic context expired"):
        with linked_build(context) as live:
            context.checks = 0
            context.on_check = lambda count: setattr(context, "alive", False) if count == 2 else None
            live.code([{"owner": "unit0.obj", "symbol": "_caller"}])


def test_live_weak_scope_rechecks_selected_object_before_result_escapes(tmp_path):
    context = weak_build(tmp_path / "mutated")
    object_path = context.directory / "unit0.obj"
    raw = object_path.read_bytes()

    def mutate_on_final_check(count):
        if count == 2:
            object_path.write_bytes(raw + b"\0")

    with pytest.raises(ValueError, match="input population changed"):
        with linked_build(context) as live:
            context.checks = 0
            context.on_check = mutate_on_final_check
            live.code([{"owner": "unit0.obj", "symbol": "_caller"}])


def test_fallback_reference_field_damage_fails_references_without_changing_code_bytes(tmp_path):
    context = weak_build(tmp_path / "fallback-reference-damage", g_reference=True,
                         damaged_g_reference=True)
    with linked_build(context) as live:
        selected = live.inputs.objects.resolve("unit0.obj")
        obj = Coff(selected["raw"])
        proof = verify_code_region(obj, obj.function("_G"), "unit0.obj", live.inputs.mapping,
                                   live.inputs.image, objects=live.inputs.objects,
                                   fixups=live.inputs.fixups, _index=MapIndex(live.inputs.mapping))
        assert proof["result"] == "fail"
        assert proof["code"]["references"] == "fail"
        assert proof["code"]["relocations"][0]["result"] == "fail"
        assert proof["code"]["differing_offsets"] == [1]

        graph = live.code([{"owner": "unit0.obj", "symbol": "_caller"}])
        assert graph["result"] == "unknown"
        assert "fallback body or references" in graph["gaps"][0]["reason"]


def test_live_weak_scope_and_reference_binding_cannot_be_serialized_or_reused(tmp_path, monkeypatch):
    from match_recovery import linked_code_region

    captured = []
    original_bind = linked_code_region._LiveWeakReferenceScope.bind

    def capture(self, obj, subject, owner, relocation, **kwargs):
        binding = original_bind(self, obj, subject, owner, relocation, **kwargs)
        captured.append((self, binding, obj, subject, owner, relocation))
        return binding

    monkeypatch.setattr(linked_code_region._LiveWeakReferenceScope, "bind", capture)
    with linked_build(weak_build(tmp_path / "capabilities")) as live:
        report = live.code([{"owner": "unit0.obj", "symbol": "_caller"}])
    assert report["result"] == "pass"
    assert len(captured) == 1
    scope, binding, obj, subject, owner, relocation = captured[0]
    with pytest.raises(TypeError, match="cannot be serialized"):
        pickle.dumps(scope)
    with pytest.raises(TypeError, match="cannot be serialized"):
        pickle.dumps(binding)
    with pytest.raises(ValueError, match="scope has expired"):
        scope.require_live()
    with pytest.raises(ValueError, match="scope has expired"):
        binding.consume(obj, subject, owner, relocation)


def _receipt_bind(context, selected, unit="src/unit0.cpp"):
    source = str(context.root / unit)
    output = str(context.directory / "unit0.obj")
    stamp_value = stamp(selected["raw"])
    context.report["compile_link"] = {
        "result": "pass",
        "compilations": [{"result": "pass", "link_correspondence": "pass", "source": source,
                          "output": output, "compilation": {"output": {"after": stamp_value}}}],
        "command_bindings": {"result": "pass", "compilations": [{"result": "pass", "source": source,
                              "output": output, "arguments": ["cl.exe", "/c", source]}]},
    }


def _weak_source_bindings(image, unit="src/unit0.cpp", *, damage_fallback=False):
    pin = hashlib.sha256(image.data).hexdigest()
    caller = FunctionBinding(Origin(pin, "pe:va", 0x401000),
                             (Span(0x401000, 0x401010),), "_caller", unit)
    fallback_span = Span(0x401010, 0x401011) if damage_fallback else Span(0x401010, 0x401020)
    fallback = FunctionBinding(Origin(pin, "pe:va", 0x401010),
                               (fallback_span,), "_G", unit)
    return caller, fallback


def test_linked_source_comparison_uses_original_fallback_only_after_strict_g_pass(tmp_path, monkeypatch):
    from match_recovery import linked_code_region
    original_qualify = linked_code_region._LiveWeakReferenceScope.qualify_original
    captured_targets = []

    def capture(self, *args):
        target = original_qualify(self, *args)
        captured_targets.append(target)
        return target

    monkeypatch.setattr(linked_code_region._LiveWeakReferenceScope, "qualify_original", capture)
    context = weak_build(tmp_path / "original-bridge")
    raw = (context.directory / "unit0.obj").read_bytes()
    _receipt_bind(context, {"raw": raw, "sha256": hashlib.sha256(raw).hexdigest()})
    with linked_build(context) as live:
        selected = live.inputs.objects.resolve("unit0.obj")
        obj = Coff(selected["raw"])
        image = live.inputs.image
        caller, fallback = _weak_source_bindings(image)
        scope = live._weak_scope()
        try:
            current = live._code_in_scope([{"owner": "unit0.obj", "symbol": "_caller"}], scope)
            assert current["result"] == "pass"
            row, = check_objects([caller], {caller.unit: obj}, image,
                                 support_bindings=[fallback], _weak_scope=scope)
            assert row["result"] == "pass", row
            weak, = [item for item in row["relocations"] if item["storage_class"] == 105]
            assert weak["result"] == "pass"
            assert weak["weak_original_target"]["fallback_entity_id"] == fallback.origin.entity_id
            assert weak["weak_original_target"]["fallback_original_address"] == 0x401010
            assert weak["weak_original_target"]["fallback_result"] == "pass"
            assert weak["weak_original_target"]["source_receipt_sha256"]
            with pytest.raises(TypeError, match="cannot be serialized"):
                pickle.dumps(captured_targets[0])
            with pytest.raises(ValueError, match="already consumed"):
                captured_targets[0].consume(caller, obj, obj.function("_caller", comdat=True),
                                            obj.section_subject(obj.function("_caller", comdat=True).section)[1][0])
        finally:
            scope.expire()


def test_original_weak_target_does_not_leak_without_live_scope_or_after_fallback_miss(tmp_path):
    context = weak_build(tmp_path / "original-bridge-damage")
    raw = (context.directory / "unit0.obj").read_bytes()
    _receipt_bind(context, {"raw": raw, "sha256": hashlib.sha256(raw).hexdigest()})
    with linked_build(context) as live:
        selected = live.inputs.objects.resolve("unit0.obj")
        obj = Coff(selected["raw"])
        damaged = bytearray(live.inputs.image.data)
        damaged[512 + 0x10] = 0x90
        original = PeImage(bytes(damaged), hashlib.sha256(damaged).hexdigest())
        caller, fallback = _weak_source_bindings(original)
        ordinary, = check_objects([caller], {caller.unit: obj}, original, support_bindings=[fallback])
        assert ordinary["result"] == "unknown"
        scope = live._weak_scope()
        try:
            current = live._code_in_scope([{"owner": "unit0.obj", "symbol": "_caller"}], scope)
            assert current["result"] == "pass"
            row, = check_objects([caller], {caller.unit: obj}, original,
                                 support_bindings=[fallback], _weak_scope=scope)
            assert row["result"] == "unknown" and "strictly compare PASS" in row.get("reason", ""), row
        finally:
            scope.expire()


def test_original_weak_target_requires_fallback_reference_pass_not_just_equal_body(tmp_path):
    context = weak_build(tmp_path / "original-reference-damage", g_reference=True)
    with linked_build(context) as live:
        selected = live.inputs.objects.resolve("unit0.obj")
        obj = Coff(selected["raw"])
        damaged = bytearray(live.inputs.image.data)
        # Keep the candidate PE and current linked graph intact, but alter only
        # the original G call operand so strict original reference comparison fails.
        damaged[512 + 0x11:512 + 0x15] = struct.pack("<i", 0x4011E1 - (0x401015))
        original = PeImage(bytes(damaged), hashlib.sha256(damaged).hexdigest())
        caller, fallback = _weak_source_bindings(original)
        scope = live._weak_scope()
        try:
            current = live._code_in_scope([{"owner": "unit0.obj", "symbol": "_caller"}], scope)
            assert current["result"] == "pass"
            row, = check_objects([caller], {caller.unit: obj}, original,
                                 support_bindings=[fallback], _weak_scope=scope)
            assert row["result"] == "unknown" and "strictly compare PASS" in row.get("reason", ""), row
        finally:
            scope.expire()


def test_original_caller_reference_mismatch_remains_fail_after_fallback_pass(tmp_path):
    context = weak_build(tmp_path / "original-caller-reference-damage")
    raw = (context.directory / "unit0.obj").read_bytes()
    _receipt_bind(context, {"raw": raw, "sha256": hashlib.sha256(raw).hexdigest()})
    with linked_build(context) as live:
        selected = live.inputs.objects.resolve("unit0.obj")
        obj = Coff(selected["raw"])
        damaged = bytearray(live.inputs.image.data)
        damaged[512 + 1] = (damaged[512 + 1] + 1) & 0xFF
        original = PeImage(bytes(damaged), hashlib.sha256(damaged).hexdigest())
        caller, fallback = _weak_source_bindings(original)
        scope = live._weak_scope()
        try:
            current = live._code_in_scope([{"owner": "unit0.obj", "symbol": "_caller"}], scope)
            assert current["result"] == "pass"
            row, = check_objects([caller], {caller.unit: obj}, original,
                                 support_bindings=[fallback], _weak_scope=scope)
            assert row["result"] == "fail", row
            weak, = [item for item in row["relocations"] if item["storage_class"] == 105]
            assert weak["result"] == "fail"
        finally:
            scope.expire()


def test_original_weak_capability_is_unit_scoped_and_expires(tmp_path):
    context = weak_build(tmp_path / "original-bridge-expiry")
    raw = (context.directory / "unit0.obj").read_bytes()
    _receipt_bind(context, {"raw": raw, "sha256": hashlib.sha256(raw).hexdigest()})
    with linked_build(context) as live:
        selected = live.inputs.objects.resolve("unit0.obj")
        obj = Coff(selected["raw"])
        image = live.inputs.image
        caller, fallback = _weak_source_bindings(image)
        scope = live._weak_scope()
        try:
            current = live._code_in_scope([{"owner": "unit0.obj", "symbol": "_caller"}], scope)
            assert current["result"] == "pass"
            changed = FunctionBinding(caller.origin, caller.spans, caller.symbol, "src/other.cpp")
            row, = check_objects([changed], {changed.unit: obj}, image,
                                 support_bindings=[fallback], _weak_scope=scope)
            assert row["result"] == "unknown", row
        finally:
            scope.expire()
        with pytest.raises(ValueError, match="expired"):
            scope.require_live()


def test_original_weak_target_is_index_and_offset_scoped(tmp_path):
    context = weak_build(tmp_path / "original-bridge-wrong-reference")
    raw = (context.directory / "unit0.obj").read_bytes()
    _receipt_bind(context, {"raw": raw, "sha256": hashlib.sha256(raw).hexdigest()})
    with linked_build(context) as live:
        selected = live.inputs.objects.resolve("unit0.obj")
        obj = Coff(selected["raw"])
        image = live.inputs.image
        caller, fallback = _weak_source_bindings(image)
        scope = live._weak_scope()
        try:
            current = live._code_in_scope([{"owner": "unit0.obj", "symbol": "_caller"}], scope)
            assert current["result"] == "pass"
            g_row, = check_objects([fallback], {fallback.unit: obj}, image)
            section = obj.function("_caller", comdat=True)
            relocation = obj.section_subject(section.section)[1][0]
            from dataclasses import replace
            for wrong in (replace(relocation, offset=relocation.offset + 1),
                          replace(relocation, target=str(int(relocation.target) + 1))):
                with pytest.raises(ValueError, match="no matching live current-edge proof"):
                    scope.qualify_original(obj, section, wrong, caller, fallback, g_row)
        finally:
            scope.expire()


def test_original_weak_target_refuses_changed_selected_object_inside_live_scope(tmp_path):
    context = weak_build(tmp_path / "original-bridge-mutated-object")
    raw = (context.directory / "unit0.obj").read_bytes()
    _receipt_bind(context, {"raw": raw, "sha256": hashlib.sha256(raw).hexdigest()})
    object_path = context.directory / "unit0.obj"
    try:
        with pytest.raises(ValueError, match="changed"):
            with linked_build(context) as live:
                selected = live.inputs.objects.resolve("unit0.obj")
                obj = Coff(selected["raw"])
                image = live.inputs.image
                caller, fallback = _weak_source_bindings(image)
                scope = live._weak_scope()
                try:
                    current = live._code_in_scope([{"owner": "unit0.obj", "symbol": "_caller"}], scope)
                    assert current["result"] == "pass"
                    g_row, = check_objects([fallback], {fallback.unit: obj}, image)
                    relocation = obj.section_subject(obj.function("_caller", comdat=True).section)[1][0]
                    object_path.write_bytes(raw + b"\0")
                    with pytest.raises(ValueError, match="changed"):
                        scope.qualify_original(obj, obj.function("_caller", comdat=True), relocation,
                                               caller, fallback, g_row)
                finally:
                    scope.expire()
    finally:
        object_path.write_bytes(raw)
