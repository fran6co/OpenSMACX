"""Physical constant bindings through native root and reachable-code checks."""

import json
import struct

import pytest

from match_recovery.linked_initializers import compare_build_initializers, observe_build_initializers
from test_linked_code_graph import direct_graph, function, graph_build
from test_linked_initializers import stamp, update_artifact
from test_toolchain_components import archive


def constant_object(value):
    return (struct.pack("<HHIIIHH", 0x14C, 0, 0, 20, 1, 0, 0)
            + b"_offset\0" + struct.pack("<IhHBB", value, -1, 0, 2, 0)
            + struct.pack("<I", 4))


def linked_constant(root, *, value=0, addend=0, operand=None, relocated=False, helper=False,
                    selected=True, duplicate=False, relative=False):
    code = b"\x68" + (addend & 0xFFFFFFFF).to_bytes(4, "little") + b"\xc3"
    kind = 20 if relative else 6
    functions = ([function("_$E1", 0, b"\xe8" + bytes(4) + b"\xc3", edges=((1, "_helper", 20),)),
                  function("_helper", 0x80, code, public=True, owner="helper.obj", edges=((1, "_offset", kind),))]
                 if helper else [function("_$E1", 0, code, edges=((1, "_offset", kind),))])
    linked = graph_build(root, functions, data={"_offset": 0x402100})
    offset = 0x81 if helper else 1
    raw = bytearray((linked.directory / "app.exe").read_bytes())
    actual = value + addend if operand is None else operand
    if relative:
        actual -= 0x401000 + offset + 4
    struct.pack_into("<I", raw, 512 + offset, actual & 0xFFFFFFFF)
    fixups = (struct.pack("<IIHH", 0x1000, 12, 0x3000 + offset, 0) if relocated else b"")
    fixups += struct.pack("<IIHH", 0x2000, 12, 0x3004, 0)
    raw[512 + 0x180:512 + 0x180 + len(fixups)] = fixups
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1180, len(fixups))
    update_artifact(linked, "app.exe", raw)
    mapping = (linked.directory / "app.map").read_text()
    update_artifact(linked, "app.map", "".join(row for row in mapping.splitlines(keepends=True)
                                               if " _offset " not in row).encode())
    receipt = json.loads((linked.directory / "link.json").read_text())
    log = (linked.directory / "link.log").read_text()
    for name in ("constant.obj", "duplicate.obj") if duplicate else ("constant.obj",):
        path = linked.directory / name
        path.write_bytes(constant_object(value))
        receipt["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
        receipt["explicit_inputs"].append(str(path))
        receipt["arguments"].append(str(path))
        if selected:
            log = log.replace("End Pass2", name + "\nEnd Pass2")
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(linked, "link.log", log.encode())
    update_artifact(linked, "link.json", json.dumps(receipt).encode())
    return linked


@pytest.mark.parametrize("helper", [False, True])
@pytest.mark.parametrize("value,addend", [(0, 0), (7, 3), (0, -1), (0xFFFFFFFF, 2)])
def test_selected_constant_is_terminal_not_an_image_address(tmp_path, helper, value, addend):
    linked = linked_constant(tmp_path / "app", value=value, addend=addend, helper=helper)
    graph = direct_graph(linked)
    assert graph["result"] == "pass", graph
    node = graph["nodes"][int(helper)]
    edge, = node["edges"]
    assert edge["target"] == {"absolute_constant": {"symbol": "_offset", "value": value}}
    reference, = node["proof"]["references"]
    assert reference["constant_binding"]["input"] == {"path": str(linked.directory / "constant.obj")}
    assert "address" not in reference
    assert not graph["data_references"] and not graph["import_references"]
    observed = observe_build_initializers(linked)
    assert observed["result"] == observed["code_dependencies"]["result"] == "pass", observed
    assert observed["remaining_linked_effects"] == "unknown"


@pytest.mark.parametrize("helper", [False, True])
@pytest.mark.parametrize("damage", ["operand", "relocated"])
def test_wrong_constant_bytes_or_loader_fixup_is_a_counterexample(tmp_path, helper, damage):
    linked = linked_constant(tmp_path / "app", helper=helper,
                             **({"operand": 42} if damage == "operand" else {"relocated": True}))
    graph = direct_graph(linked)
    assert graph["result"] == "fail", graph
    assert graph["nodes"][int(helper)]["proof"]["code"]["result"] == "fail"


@pytest.mark.parametrize("helper", [False, True])
def test_changed_constant_and_coherent_operand_do_not_compare_equal(tmp_path, helper):
    old = linked_constant(tmp_path / "old", value=0, helper=helper)
    new = linked_constant(tmp_path / "new", value=4, helper=helper)
    for linked in (old, new):
        assert direct_graph(linked)["result"] == "pass"
    report = compare_build_initializers(old, new)
    assert report["result"] == report["code_dependencies"]["result"] == "fail", report
    assert report["input_consistency"] == "pass"
    assert report["ordered_code_correspondence"] == ("pass" if helper else "fail")


@pytest.mark.parametrize("kwargs", [{"selected": False}, {"duplicate": True}, {"relative": True}])
def test_unsupported_constant_selection_or_rebase_semantics_stay_unknown(tmp_path, kwargs):
    graph = direct_graph(linked_constant(tmp_path / "app", **kwargs))
    assert graph["result"] == "unknown", graph
    assert not graph["ready_for_collection"]


@pytest.mark.parametrize("kind,value", [(0, 0), (0x10, 0), (0x20, 0), (0, 4)])
def test_fake_map_address_and_matching_rebased_operand_cannot_override_constant(tmp_path, kind, value):
    linked = linked_constant(tmp_path / "app", operand=0x402100, relocated=True)
    mapping = (linked.directory / "app.map").read_text()
    line = " 0002:00000100 _offset 00402100 constant.obj\n"
    update_artifact(linked, "app.map", mapping.replace(" entry point", line + " entry point").encode())
    obj = linked.directory / "main.obj"
    raw = bytearray(obj.read_bytes())
    symbols, count = struct.unpack_from("<II", raw, 8)
    found = [symbols + index * 18 for index in range(count)
             if raw[symbols + index * 18:symbols + index * 18 + 8] == b"_offset\0"]
    assert len(found) == 1
    struct.pack_into("<I", raw, found[0] + 8, value)
    struct.pack_into("<H", raw, found[0] + 14, kind)
    obj.write_bytes(raw)
    receipt = json.loads((linked.directory / "link.json").read_text())
    row = next(row for row in receipt["files"] if row["path"] == str(obj))
    row["before"] = row["after"] = stamp(raw)
    update_artifact(linked, "link.json", json.dumps(receipt).encode())
    graph = direct_graph(linked)
    assert graph["result"] != "pass", graph


@pytest.mark.parametrize("selected", [False, True])
def test_archive_constant_needs_exact_final_pass_member_selection(tmp_path, selected):
    linked = linked_constant(tmp_path / "app")
    old = linked.directory / "constant.obj"
    library = linked.directory / "runtime.lib"
    library.write_bytes(archive([("constant.obj/", old.read_bytes())]))
    receipt = json.loads((linked.directory / "link.json").read_text())
    receipt["files"] = [row for row in receipt["files"] if row["path"] != str(old)]
    receipt["files"].append({"path": str(library), "before": stamp(library.read_bytes()), "after": stamp(library.read_bytes())})
    for field in ("explicit_inputs", "arguments"):
        receipt[field] = [name for name in receipt[field] if name != str(old)]
    receipt["arguments"].append(library.name)
    receipt["library_searches"] = [{"spelling": str(library), "path": str(library)}]
    log = (linked.directory / "link.log").read_text().replace("Start Pass1\n", "Start Pass1\n Searching " + str(library) + ":\n")
    log = log.replace("constant.obj\n", "runtime.lib(constant.obj)\n" if selected else "")
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(linked, "link.log", log.encode())
    update_artifact(linked, "link.json", json.dumps(receipt).encode())
    graph = direct_graph(linked)
    assert graph["result"] == ("pass" if selected else "unknown"), graph
    if selected:
        reference, = graph["nodes"][0]["proof"]["references"]
        assert reference["constant_binding"]["input"] == {"path": str(library), "member": "constant.obj", "occurrence": 0}


def test_same_object_absolute_definition_has_no_inferred_image_address(tmp_path):
    linked = linked_constant(tmp_path / "app", value=12)
    obj = linked.directory / "main.obj"
    raw = bytearray(obj.read_bytes())
    symbols, count = struct.unpack_from("<II", raw, 8)
    target, = [symbols + index * 18 for index in range(count)
               if raw[symbols + index * 18:symbols + index * 18 + 8] == b"_offset\0"]
    struct.pack_into("<Ih", raw, target + 8, 12, -1)
    obj.write_bytes(raw)
    receipt = json.loads((linked.directory / "link.json").read_text())
    obsolete = str(linked.directory / "constant.obj")
    receipt["files"] = [row for row in receipt["files"] if row["path"] != obsolete]
    for field in ("arguments", "explicit_inputs"):
        receipt[field].remove(obsolete)
    row = next(row for row in receipt["files"] if row["path"] == str(obj))
    row["before"] = row["after"] = stamp(raw)
    log = (linked.directory / "link.log").read_text().replace("constant.obj\n", "")
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(linked, "link.log", log.encode())
    update_artifact(linked, "link.json", json.dumps(receipt).encode())
    graph = direct_graph(linked)
    assert graph["result"] == "pass", graph
    reference, = graph["nodes"][0]["proof"]["references"]
    assert reference["constant_binding"]["input"] == {"path": str(obj)}
