"""Physical short-import archive + map + PE thunk controls; no byte search."""

from copy import deepcopy
from dataclasses import replace
from hashlib import sha256
import struct

import pytest

from match_recovery import linked_import_thunks as native
from match_recovery.linked_code_graph import compare_code_graphs, observe_code_graph
from match_recovery.linked_objects import SelectedObjects
from match_recovery.msvc_link_map import parse_link_map
from test_binary import coff_fixture
from test_coff_inventory import imported, weak_object
from test_imports import image_imports, pe
from test_linked_objects import library_inputs


OWNER = "RUNTIME:DEMO.dll"


def image_thunk(**kwargs):
    raw = bytearray(image_imports(**kwargs))
    raw.extend(bytes(512))
    struct.pack_into("<H", raw, 0x86, 2)
    struct.pack_into("<I", raw, 0x98 + 16, 0x2000)
    raw[0x178:0x180] = b".data\0\0\0"
    struct.pack_into("<I", raw, 0x178 + 36, 0xC0000040)
    raw[0x1A0:0x1A8] = b".text\0\0\0"
    struct.pack_into("<IIIIIIHHI", raw, 0x1A8, 512, 0x2000, 512, 1024, 0, 0, 0, 0, 0x60000020)
    raw[1024:1030] = b"\xff\x25\x60\x11\x40\0"
    struct.pack_into("<I", raw, 0x300, 0x2000)
    return bytes(raw)


def mapping(symbol="_Paint@4"):
    return parse_link_map((" Preferred load address is 00400000\n"
        " Address Publics by Value Rva+Base Lib:Object\n"
        f" 0002:00000000 {symbol} 00402000 f {OWNER}\n"
        f" 0001:00000160 __imp_{symbol} 00401160 {OWNER}\n"
        " entry point at 0002:00000000\n").encode())


def physical(root, *, members=None, finals=None):
    if members is None:
        members = [("DEMO.dll/", imported()), ("DEMO.dll/", imported("_Other@8"))]
    observed = library_inputs(root, members, ["DEMO.dll", "DEMO.dll"] if finals is None else finals)
    return SelectedObjects(*observed)


def verify(objects, image=None, linkmap=None, symbol="_Paint@4"):
    return native.verify_import_thunk(objects, linkmap or mapping(symbol), pe(image or image_thunk()), OWNER, symbol)


def test_repeated_member_family_uses_exact_header_and_independent_slot(tmp_path):
    objects = physical(tmp_path)
    with pytest.raises(ValueError):
        objects.resolve(OWNER)
    assert native.import_thunk_candidates(objects) == frozenset({"_Paint@4", "_Other@8"})
    result = verify(objects)
    assert result["result"] == result["selection"] == "pass"
    assert result["input"]["occurrence"] == 0
    assert result["object_sha256"] == sha256(imported()).hexdigest()
    assert result["size"] == 6 and result["address"] == 0x402000
    assert result["import"] == {"module": "demo.dll", "name": "Paint", "ordinal": None,
                                "address": 0x401160, "symbol": "__imp__Paint@4"}
    assert result["selection_basis"] == "unique_complete_physical_producer_and_named_linked_use"
    assert result["dll_implementation"] == result["runtime"] == result["original_verification"] == "unknown"
    assert not result["ready_for_collection"]


@pytest.mark.parametrize("lookup,bound", [(True, False), (True, True), (False, False)])
def test_lookup_identity_never_comes_from_the_prebound_iat_operand(tmp_path, lookup, bound):
    assert verify(physical(tmp_path), image_thunk(lookup=lookup, bound=bound))["result"] == "pass"


def test_ordinal_import_is_bound_as_ordinal_not_hint(tmp_path):
    objects = physical(tmp_path, members=[("DEMO.dll/", imported(naming=0))], finals=["DEMO.dll"])
    result = verify(objects, image_thunk(ordinal=True))
    assert result["result"] == "pass" and result["import"]["ordinal"] == 7
    assert result["import"]["name"] is None and result["import_header"]["hint"] is None


@pytest.mark.parametrize("damage", ["opcode", "operand", "no_highlow", "wrong_highlow"])
def test_six_byte_recipe_and_exact_loader_fixup_are_both_checked(tmp_path, damage):
    raw = bytearray(image_thunk())
    if damage == "opcode": raw[1025] = 0x15
    elif damage == "operand": struct.pack_into("<I", raw, 1026, 0x401164)
    elif damage == "no_highlow": struct.pack_into("<H", raw, 0x308, 0)
    else: struct.pack_into("<H", raw, 0x308, 0x3001)
    assert verify(physical(tmp_path), bytes(raw))["result"] == "fail"


@pytest.mark.parametrize("damage", ["module", "name", "ordinal", "bound_without_lookup", "code_flags", "slot_flags", "cross_extent"])
def test_invalid_loader_identity_or_placement_is_not_a_similar_thunk(tmp_path, damage):
    raw = bytearray(image_thunk(lookup=damage != "bound_without_lookup", bound=damage == "bound_without_lookup"))
    if damage == "module": raw[0x380] = ord("X")
    elif damage == "name": raw[0x3A2] = ord("X")
    elif damage == "ordinal":
        struct.pack_into("<I", raw, 0x350, 0x80000007)
        struct.pack_into("<I", raw, 0x360, 0x80000007)
    elif damage == "code_flags": struct.pack_into("<I", raw, 0x1A0 + 36, 0x40000040)
    elif damage == "slot_flags": struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)
    elif damage == "cross_extent":
        struct.pack_into("<I", raw, 0x1A8, 5)
        struct.pack_into("<I", raw, 0x1A8 + 8, 5)
    with pytest.raises(ValueError): verify(physical(tmp_path), bytes(raw))


@pytest.mark.parametrize("damage", ["missing_thunk", "missing_iat", "wrong_thunk_owner", "wrong_iat_owner", "wrong_type", "absolute", "wrong_slot", "alias"])
def test_named_map_use_is_not_inferred_from_the_instruction(tmp_path, damage):
    linkmap = mapping()
    rows = list(linkmap.publics)
    if damage == "missing_thunk": rows.pop(0)
    elif damage == "missing_iat": rows.pop(1)
    elif damage == "wrong_thunk_owner": rows[0] = replace(rows[0], owner="OTHER:DEMO.dll")
    elif damage == "wrong_iat_owner": rows[1] = replace(rows[1], owner="OTHER:DEMO.dll")
    elif damage == "wrong_type": rows[0] = replace(rows[0], function=False)
    elif damage == "absolute": rows[1] = replace(rows[1], segment=0)
    elif damage == "wrong_slot": rows[1] = replace(rows[1], address=0x401164, offset=0x164)
    else: rows.append(replace(rows[0], symbol="another_entry", offset=2, address=0x402002))
    with pytest.raises(ValueError): verify(physical(tmp_path), linkmap=replace(linkmap, publics=tuple(rows)))


@pytest.mark.parametrize("other", [imported(), imported(module="other.dll"), imported(kind=1), imported("__imp__Paint@4"), imported("_Paint@4", naming=0)])
def test_even_unselected_competing_import_producer_refuses(tmp_path, other):
    objects = physical(tmp_path, members=[("DEMO.dll/", imported()), ("UNUSED.dll/", other)], finals=["DEMO.dll"])
    with pytest.raises(ValueError, match="complete-population producer"): verify(objects)


@pytest.mark.parametrize("kind", [1, 2])
def test_data_or_const_import_is_not_a_code_recipe(tmp_path, kind):
    objects = physical(tmp_path, members=[("DEMO.dll/", imported(kind=kind))], finals=["DEMO.dll"])
    with pytest.raises(ValueError, match="short code import"): verify(objects)


def declaration(name, *, section=0, value=0, storage=2, weak=False):
    raw = bytearray(weak_object() if weak else coff_fixture())
    start, count = struct.unpack_from("<II", raw, 8)
    at = start if weak else start + 18
    strings = start + count * 18
    raw[strings:] = struct.pack("<I", 5 + len(name)) + name.encode() + b"\0"
    struct.pack_into("<II", raw, at, 0, 4)
    struct.pack_into("<IhHBB", raw, at + 8, value, section, 0, storage, 0)
    return bytes(raw)


@pytest.mark.parametrize("name", ["_Paint@4", "__imp__Paint@4"])
@pytest.mark.parametrize("kind", ["defined", "common", "absolute", "static", "weak"])
def test_regular_competitors_and_weak_fallbacks_cannot_be_hidden(tmp_path, name, kind):
    payload = declaration(name, section={"defined": 1, "absolute": -1}.get(kind, 0),
                          value=4 if kind == "common" else 0, storage=3 if kind == "static" else 2,
                          weak=kind == "weak")
    objects = physical(tmp_path, members=[("DEMO.dll/", imported()), ("unused.obj/", payload)], finals=["DEMO.dll"])
    with pytest.raises(ValueError, match="namespace"): verify(objects)


def test_plain_undefined_reference_is_not_a_competing_producer(tmp_path):
    objects = physical(tmp_path, members=[("DEMO.dll/", imported()), ("caller.obj/", declaration("_Paint@4"))], finals=["DEMO.dll"])
    assert verify(objects)["result"] == "pass"


@pytest.mark.parametrize("option", ["/FORCE", "/DEF:application.def", "/ALTERNATENAME:_Paint@4=_Other", "/unknown"])
def test_unsupported_link_options_never_disappear(tmp_path, option):
    observed = list(library_inputs(tmp_path, [("DEMO.dll/", imported())], ["DEMO.dll"]))
    observed[0] = deepcopy(observed[0])
    observed[0]["arguments"].append(option)
    with pytest.raises(ValueError, match="policy"): verify(SelectedObjects(*observed))


def test_missing_final_family_is_not_replaced_by_unique_unselected_header(tmp_path):
    objects = physical(tmp_path, finals=[])
    with pytest.raises(ValueError, match="final-pass"): verify(objects)


def test_incomplete_saved_inventory_cannot_omit_a_physical_competitor(tmp_path):
    evidence, coverage, records = library_inputs(tmp_path, [("DEMO.dll/", imported()), ("DEMO.dll/", imported())], ["DEMO.dll"])
    records.pop()
    coverage["record_count"] -= 1
    with pytest.raises(ValueError, match="complete physical population"):
        verify(SelectedObjects(evidence, coverage, records))


def test_input_damage_after_cached_detection_refuses(tmp_path):
    objects = physical(tmp_path)
    assert "_Paint@4" in native.import_thunk_candidates(objects)
    (tmp_path / "runtime.lib").write_bytes(b"changed")
    with pytest.raises(ValueError): verify(objects)


def test_input_damage_during_comparison_refuses_final_guard(tmp_path, monkeypatch):
    objects = physical(tmp_path)
    compare = native.compare_bound_code
    def damage(*args, **kwargs):
        result = compare(*args, **kwargs)
        (tmp_path / "runtime.lib").write_bytes(b"changed")
        return result
    monkeypatch.setattr(native, "compare_bound_code", damage)
    with pytest.raises(ValueError, match="changed during"): verify(objects)


@pytest.mark.parametrize("symbol,naming", [("Paint", 1), ("_Paint", 2), ("_Paint@4", 3)])
def test_supported_name_modes_use_the_actual_header_transformation(tmp_path, symbol, naming):
    objects = physical(tmp_path, members=[("DEMO.dll/", imported(symbol, naming=naming))], finals=["DEMO.dll"])
    assert verify(objects, symbol=symbol)["result"] == "pass"


@pytest.mark.parametrize("damage", ["count", "coverage", "multiple_paths", "library_alias"])
def test_final_pass_family_cannot_supply_ambiguous_physical_provenance(tmp_path, damage):
    evidence, coverage, records = library_inputs(tmp_path, [("DEMO.dll/", imported())], ["DEMO.dll"])
    final = coverage["inputs"][-1]
    if damage == "count": final["candidate_count"] = 2
    elif damage == "coverage": final["candidate_coverage"] = "unknown"
    elif damage == "multiple_paths": final["paths"].append(str(tmp_path / "another.lib"))
    else: evidence["library_searches"][0]["spelling"] = "another.lib"
    with pytest.raises(ValueError, match="physical library family"):
        verify(SelectedObjects(evidence, coverage, records))


def test_unsupported_directive_in_an_unselected_record_is_not_ignored(tmp_path):
    text = b" /ALTERNATENAME:_Paint@4=_Other "
    raw = bytearray(coff_fixture(text))
    raw[20:28] = b".drectve"
    size = struct.unpack_from("<I", raw, 36)[0]
    raw[60 + len(text):60 + size] = b" " * (size - len(text))
    objects = physical(tmp_path, members=[("DEMO.dll/", imported()), ("unused.obj/", bytes(raw))], finals=["DEMO.dll"])
    with pytest.raises(ValueError, match="policy"): verify(objects)


def graph(objects, image=None, linkmap=None, roots=None):
    return observe_code_graph(objects, linkmap or mapping(), pe(image or image_thunk()),
                              roots or [{"owner": OWNER, "symbol": "_Paint@4"}])


def test_native_graph_observes_the_selected_recipe_but_not_dll_behavior(tmp_path):
    result = graph(physical(tmp_path))
    assert result["result"] == "pass" and result["graph_fingerprint"]
    assert len(result["nodes"]) == 1
    node, = result["nodes"]
    assert node["proof"]["extent_kind"] == "i386_short_import_thunk"
    imported, = result["import_references"]
    assert imported["selection"] == "pass" and imported["implementation"] == "unknown"
    assert result["remaining_linked_effects"] == "unknown" and not result["ready_for_collection"]


def test_native_graph_keeps_an_observed_thunk_byte_counterexample(tmp_path):
    raw = bytearray(image_thunk())
    raw[1025] = 0x15
    result = graph(physical(tmp_path), bytes(raw))
    assert result["result"] == result["nodes"][0]["result"] == "fail"
    assert result["nodes"][0]["proof"]["proof"]["bytes"] == "fail"
    assert result["graph_fingerprint"] is None


def test_native_graph_does_not_fall_back_to_byte_similarity_on_competing_imports(tmp_path):
    objects = physical(tmp_path, members=[("DEMO.dll/", imported()), ("unused.dll/", imported())], finals=["DEMO.dll"])
    result = graph(objects)
    assert result["result"] == "unknown" and result["graph_fingerprint"] is None
    assert "complete-population producer" in result["gaps"][0]["reason"]


@pytest.mark.parametrize("changed", ["name", "module"])
def test_native_graph_identity_changes_even_when_the_thunk_bytes_do_not(tmp_path, changed):
    baseline_path, current_path = tmp_path / "baseline", tmp_path / "current"
    baseline_path.mkdir()
    current_path.mkdir()
    baseline = graph(physical(baseline_path))
    raw = bytearray(image_thunk())
    if changed == "name":
        # Keep the source thunk symbol, but select NAME instead of UNDECORATE.
        payload = b"\x02\0_Paint@4\0"
        raw[0x3A0:0x3A0 + len(payload)] = payload
        member = imported(naming=1)
    else:
        raw[0x380:0x389] = b"ELSE.dll\0"
        member = imported(module="ELSE.dll")
    current = graph(physical(current_path, members=[("DEMO.dll/", member)], finals=["DEMO.dll"]), bytes(raw))
    assert baseline["result"] == current["result"] == "pass"
    assert image_thunk()[1024:1030] == bytes(raw)[1024:1030]
    assert compare_code_graphs(baseline, current)["result"] == "fail"


@pytest.mark.parametrize("addend,expected", [(-1, "unknown"), (0, "pass"), (5, "pass"), (6, "unknown")])
def test_native_graph_effective_call_target_is_bounded_by_the_six_byte_recipe(tmp_path, addend, expected):
    raw_object = bytearray(coff_fixture(b"\xe8" + struct.pack("<i", addend) + b"\xc3", relocation=(1, 1, 20)))
    symbols = struct.unpack_from("<I", raw_object, 8)[0]
    raw_object[symbols + 18:symbols + 26] = b"_Paint@4"
    objects = physical(tmp_path, members=[("DEMO.dll/", imported()), ("caller.obj/", bytes(raw_object))],
                       finals=["DEMO.dll", "caller.obj"])
    raw = bytearray(image_thunk())
    raw[1056:1072] = b"\xe8" + struct.pack("<i", 0x402000 + addend - 0x402025) + b"\xc3" + b"\xcc" * 10
    linkmap = mapping()
    caller = replace(linkmap.publics[0], symbol="_f", address=0x402020, offset=0x20, owner="RUNTIME:caller.obj")
    linkmap = replace(linkmap, publics=(*linkmap.publics, caller))
    result = graph(objects, bytes(raw), linkmap, [{"owner": "RUNTIME:caller.obj", "symbol": "_f"}])
    assert result["result"] == expected
    assert len(result["nodes"]) == 2 and result["nodes"][1]["result"] == "pass"
    if expected == "unknown":
        assert "effective code reference" in result["gaps"][-1]["reason"]
