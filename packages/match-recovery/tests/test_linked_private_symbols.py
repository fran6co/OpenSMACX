"""Private linked data requires physical uniqueness, not an operand guess."""

from copy import deepcopy
from hashlib import sha256
import json
import struct

import pytest

from match_recovery.linked_objects import SelectedObjects
from match_recovery.pe_coff import PeImage
from test_linked_constants import symbol_object, constant
from test_linked_objects import library_inputs
from test_pe_debug import embedded_image


def local_object(name="_private", *, value=0, section=1, kind=0, storage=3, auxiliary=b"", flags=0xC0300040):
    raw = bytearray(symbol_object([(name, value, section, kind, storage, auxiliary)], directives=bytes(8)))
    raw[20:28] = b".data\0\0\0"
    struct.pack_into("<I", raw, 56, flags)
    return bytes(raw)


def observed(root, *, raw=None, extra=(), selected=True):
    return library_inputs(root, [("private.obj/", local_object() if raw is None else raw), *extra],
                          ["private.obj"] if selected else [])


def image(symbols=None):
    raw = embedded_image(symbols)
    return PeImage(raw, sha256(raw).hexdigest())


def resolve(evidence, pe=None, *, owner="RUNTIME:private.obj", names=("_private",)):
    return SelectedObjects(*evidence).resolve_local_symbols(owner, names, image() if pe is None else pe)


def test_unique_selected_static_data_uses_embedded_rva_and_preserves_provenance(tmp_path):
    binding = resolve(observed(tmp_path))["_private"]
    assert binding == {"symbol": "_private", "address": 0x401000, "owner": "RUNTIME:private.obj",
        "input": {"path": str(tmp_path / "runtime.lib"), "member": "private.obj", "occurrence": 0},
        "object_sha256": sha256(local_object()).hexdigest(), "object_index": 0,
        "object_section": 1, "object_offset": 0, "placement": "embedded_coff"}
    assert "result" not in binding and "contents" not in binding


def test_associated_data_label_can_bind_without_placing_or_proving_its_code_parent(tmp_path):
    from match_recovery.pe_coff import Coff
    from test_multispan import object_fixture
    raw = bytearray(object_fixture())
    symbols_at, = struct.unpack_from("<I", raw, 8)
    raw[60:68] = raw[symbols_at + 3 * 18:symbols_at + 3 * 18 + 8] = b".xdata$x"
    struct.pack_into("<I", raw, 60 + 36, 0x40301040)
    raw[symbols_at + 5 * 18 + 16] = 3
    assert Coff(raw).associations == {2: 1}
    binding = resolve(observed(tmp_path, raw=bytes(raw)),
        image([("$tail2", 0x1004, 1, 0, 3, b"")]), names=("$tail2",))["$tail2"]
    assert binding["object_section"] == 2 and binding["address"] == 0x401004
    assert "code" not in binding and "contents" not in binding


@pytest.mark.parametrize("extra", [local_object(), constant(name="_private"),
    constant(name="_private", section=0), local_object(storage=2), local_object(kind=32)])
def test_unselected_occurrences_and_declarations_cannot_be_hidden(tmp_path, extra):
    with pytest.raises(ValueError, match="exactly one"):
        resolve(observed(tmp_path, extra=[("unselected.obj/", extra)]))


@pytest.mark.parametrize("options", [{"value": 8}, {"section": 0}, {"section": -1}, {"kind": 32},
    {"storage": 2}, {"auxiliary": bytes(18)}, {"flags": 0x60500020}, {"flags": 0xE0300040},
    {"flags": 0}, {"name": ".data"}])
def test_metadata_code_and_out_of_extent_object_labels_refuse(tmp_path, options):
    with pytest.raises(ValueError):
        resolve(observed(tmp_path, raw=local_object(**options)), names=(options.get("name", "_private"),))


@pytest.mark.parametrize("symbols", [[], [("other", 0x1000, 1, 0, 3, b"")],
    [("_private", 0x1000, 1, 0, 2, b"")], [("_private", 0x1000, 1, 32, 3, b"")],
    [("_private", 0x1000, 1, 0, 3, b"")] * 2,
    [("_private", 0x1000, 1, 0, 3, b""), ("_private", 0, 0, 0, 2, b"")]])
def test_missing_incompatible_or_ambiguous_embedded_symbols_refuse(tmp_path, symbols):
    with pytest.raises(ValueError):
        resolve(observed(tmp_path), image(symbols))


def test_executable_image_data_does_not_qualify_as_private_storage(tmp_path):
    raw = bytearray(embedded_image())
    struct.pack_into("<I", raw, 0x178 + 36, 0xE0000040)
    with pytest.raises(ValueError, match="not plain image data"):
        resolve(observed(tmp_path), PeImage(raw, sha256(raw).hexdigest()))


def test_same_name_from_unselected_or_wrong_owner_does_not_bind(tmp_path):
    with pytest.raises(ValueError):
        resolve(observed(tmp_path, selected=False))
    with pytest.raises(ValueError, match="selected physical owner"):
        resolve(observed(tmp_path), owner="app.obj")


@pytest.mark.parametrize("damage", ["remove_record", "remove_symbol", "change_value", "bytes"])
def test_complete_physical_population_is_revalidated(tmp_path, damage):
    evidence = deepcopy(observed(tmp_path, extra=[("other.obj/", local_object("other"))]))
    row = next(row for row in evidence[2] if row.get("member") == "other.obj")
    if damage == "remove_record":
        evidence[2].remove(row)
        evidence[1]["record_count"] -= 1
    elif damage == "remove_symbol": row["symbols"].clear()
    elif damage == "change_value": row["symbols"][0]["value"] = 5
    else: (tmp_path / "runtime.lib").write_bytes(b"changed")
    with pytest.raises(ValueError): resolve(evidence)


def test_cached_population_checks_physical_inputs_again(tmp_path):
    resolver = SelectedObjects(*observed(tmp_path))
    resolver.resolve_local_symbols("RUNTIME:private.obj", ["_private"], image())
    (tmp_path / "runtime.lib").write_bytes(b"changed")
    with pytest.raises(ValueError):
        resolver.resolve_local_symbols("RUNTIME:private.obj", ["_private"], image())


def test_mutation_during_resolution_refuses_before_returning_bindings(tmp_path, monkeypatch):
    resolver = SelectedObjects(*observed(tmp_path))
    original = resolver.resolve
    def changed(owner):
        selected = original(owner)
        (tmp_path / "app.obj").write_bytes(b"changed unrelated input")
        return selected
    monkeypatch.setattr(resolver, "resolve", changed)
    with pytest.raises(ValueError, match="population changed"):
        resolver.resolve_local_symbols("RUNTIME:private.obj", ["_private"], image())


def test_batch_refuses_instead_of_returning_partial_bindings(tmp_path):
    with pytest.raises(ValueError, match="exactly one"):
        resolve(observed(tmp_path), names=("_private", "missing"))


@pytest.mark.parametrize("names", [[], [""], ["_private", "_private"], "_private", [None], ["bad name"]])
def test_invalid_name_batches_refuse(tmp_path, names):
    with pytest.raises(ValueError): resolve(observed(tmp_path), names=names)


@pytest.mark.parametrize("options", [["/DEBUG", "/DEBUGTYPE:COFF", "/PDB:NONE", "/OPT:REF", "/OPT:ICF"],
                                      ["/DEBUG", "/DEBUGTYPE:BOTH", "/PDB:NONE"]])
def test_debug_metadata_options_do_not_assume_release_addresses(tmp_path, options):
    evidence = observed(tmp_path)
    evidence[0]["arguments"].extend(options)
    binding = resolve(evidence, image([("_private", 0x1004, 1, 0, 3, b"")]))["_private"]
    assert binding["address"] == 0x401004


@pytest.mark.parametrize("option", ["/FORCE", "/DEF:alias.def", "/ALTERNATENAME:_private=other"])
def test_unknown_linker_policy_refuses_private_bindings(tmp_path, option):
    evidence = observed(tmp_path)
    evidence[0]["arguments"].append(option)
    with pytest.raises(ValueError, match="unsupported linker policy"):
        resolve(evidence)


def private_graph(root, *, operand=0x402100, placement=0x2100, fixup=True, map_row=None):
    from test_linked_code_region import physical
    from test_linked_initializers import update_artifact

    context = physical(root, cross_section=True)
    raw = bytearray((context.directory / "app.exe").read_bytes())
    struct.pack_into("<I", raw, 512 + 0x101, operand)
    if not fixup:
        struct.pack_into("<H", raw, 512 + 0x180 + 8, 0)
    payload = (struct.pack("<8I", 1, 32, 0, 0, 0x1000, 0x1200, 0x2000, 0x2200)
               + b"inside\0\0" + struct.pack("<IhHBB", placement, 2, 0, 3, 0) + struct.pack("<I", 4))
    at = len(raw)
    raw.extend(payload)
    struct.pack_into("<I", raw, 0x98 + 60, 512)
    struct.pack_into("<II", raw, 0x8C, at + 32, 1)
    struct.pack_into("<II", raw, 0x98 + 96 + 6 * 8, 0x2180, 28)
    struct.pack_into("<IIHHIIII", raw, 1024 + 0x180, 0, 0, 0, 0, 1, len(payload), 0, at)
    update_artifact(context, "app.exe", raw)
    if map_row:
        mapping = (context.directory / "app.map").read_text()
        update_artifact(context, "app.map", (mapping + map_row + "\n").encode())
    return context


def test_native_graph_binds_private_data_without_granting_contents_or_collection(tmp_path):
    from test_linked_code_graph import direct_graph
    from test_linked_code_region import helper
    report = direct_graph(private_graph(tmp_path / "graph"))
    assert report["result"] == "pass", report["gaps"]
    node = helper(report)
    assert node["proof"]["code"]["references"] == "pass"
    reference = node["proof"]["references"][0]
    assert reference["private_binding"]["address"] == reference["address"] == 0x402100
    assert report["data_references"][0]["contents"] == "unknown"
    assert report["remaining_linked_effects"] == "unknown" and not report["ready_for_collection"]
    assert '"raw"' not in json.dumps(report)


@pytest.mark.parametrize("options", [{"operand": 0x402104}, {"placement": 0x2104}, {"fixup": False}])
def test_operand_or_embedded_placement_damage_and_missing_highlow_fail(tmp_path, options):
    from test_linked_code_graph import direct_graph
    from test_linked_code_region import helper
    report = direct_graph(private_graph(tmp_path / "damage", **options))
    assert report["result"] == helper(report)["proof"]["code"]["result"] == "fail", report["gaps"]


@pytest.mark.parametrize("row", [" 0002:00000100 inside 00402100 other.obj",
    " 0000:00000000 inside 00000000 helper.obj", " 0001:00000100 inside 00401100 f other.obj"])
def test_conflicting_map_rows_do_not_fall_back_to_private_debug(tmp_path, row):
    from test_linked_code_graph import direct_graph
    report = direct_graph(private_graph(tmp_path / "conflict", map_row=row))
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None
