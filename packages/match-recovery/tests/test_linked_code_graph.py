"""Physical synthetic linkage controls; not a live compiler or effect proof."""

from collections import defaultdict
import hashlib
import json
import struct

import pytest

from match_recovery.linked_code_graph import compare_code_graphs, observe_code_graph
from match_recovery.linked_initializers import compare_build_initializers, observe_build_initializers
from match_recovery.linked_objects import SelectedObjects
from match_recovery.link_file_evidence import parse_link_files
from match_recovery.link_input_coverage import _input_inventory
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pe_coff import PeImage

from test_linked_initializers import build, stamp, update_artifact


def function(name, offset, code=b"\xc3", *, owner="main.obj", public=False, edges=()):
    return dict(name=name, offset=offset, code=code, owner=owner, public=public, edges=edges)


def object_bytes(functions, roots, data):
    """Separate, strictly bounded code COMDATs with ordinary COFF relocations."""
    symbols = [(row["name"], 0, index + 1, 32, 2 if row["public"] else 3)
               for index, row in enumerate(functions)]
    indices = {row[0]: index for index, row in enumerate(symbols)}
    for row in functions:
        for _, name, _ in row["edges"]:
            if name not in indices:
                indices[name] = len(symbols)
                symbols.append((name, 0, 0, 0 if name in data else 32, 2))
    specs = []
    for row in functions:
        code = row["code"] + b"\xcc" * (-len(row["code"]) % 16)
        specs.append((b".text", code,
                      [(offset, indices[name], kind) for offset, name, kind in row["edges"]], 0x60501020))
    local_roots = [name for name in roots if name in indices and name in {row["name"] for row in functions}]
    if local_roots:
        specs.append((b".CRT$XCU", bytes(4 * len(local_roots)),
                      [(4 * index, indices[name], 6) for index, name in enumerate(local_roots)], 0xC0300040))
    start = 20 + 40 * len(specs)
    headers, payload = [], bytearray()
    for label, raw, relocations, flags in specs:
        at = start + len(payload)
        payload.extend(raw)
        relocation_at = start + len(payload) if relocations else 0
        for relocation in relocations:
            payload.extend(struct.pack("<IIH", *relocation))
        headers.append(label.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(raw), at,
                                                        relocation_at, 0, len(relocations), 0, flags))
    records = b"".join(name.encode().ljust(8, b"\0") + struct.pack("<IhHBB", value, section, kind, storage, 0)
                       for name, value, section, kind, storage in symbols)
    assert all(len(row[0]) <= 8 for row in symbols)
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(payload), len(symbols), 0, 0)
            + b"".join(headers) + payload + records + struct.pack("<I", 4))


def graph_build(root, functions, *, roots=("_$E1",), data=None):
    """Fresh physical inputs, link receipt/log, independent map, and actual PE."""
    context = build(root)
    data = data or {}
    # Native map grammar requires a public table; this unrelated entry-point
    # COMDAT is physically present but is not a fabricated dependency root.
    functions = [*functions, function("_entry", 0x160, public=True, owner="entry.obj")]
    definitions = {row["name"]: row for row in functions}
    assert len(definitions) == len(functions)
    grouped = defaultdict(list)
    for row in functions:
        grouped[row["owner"]].append(row)
    context.objects = []
    for owner, members in grouped.items():
        path = context.directory / owner
        path.write_bytes(object_bytes(members, roots, data))
        context.objects.append(path)
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[512:1024] = b"\xcc" * 512
    raw[1024:1536] = bytes(512)
    pages = defaultdict(list)
    for row in functions:
        linked = bytearray(row["code"] + b"\xcc" * (-len(row["code"]) % 16))
        for offset, target, kind in row["edges"]:
            address = data[target] if target in data else 0x401000 + definitions[target]["offset"]
            if kind == 20:
                address += struct.unpack_from("<i", linked, offset)[0]
                struct.pack_into("<i", linked, offset, address - (0x401000 + row["offset"] + offset + 4))
            else:
                assert kind == 6
                address += struct.unpack_from("<I", linked, offset)[0]
                struct.pack_into("<I", linked, offset, address & 0xFFFFFFFF)
                pages[0x1000].append(0x3000 + row["offset"] + offset)
        raw[512 + row["offset"]:512 + row["offset"] + len(linked)] = linked
    for index, name in enumerate(roots):
        struct.pack_into("<I", raw, 1028 + 4 * index, 0x401000 + definitions[name]["offset"])
        pages[0x2000].append(0x3004 + 4 * index)
    relocations = bytearray()
    for page, entries in sorted(pages.items()):
        if len(entries) % 2:
            entries.append(0)
        relocations.extend(struct.pack("<II", page, 8 + 2 * len(entries)))
        relocations.extend(struct.pack("<" + "H" * len(entries), *entries))
    assert len(relocations) <= 0x80
    raw[512 + 0x180:512 + 0x180 + len(relocations)] = relocations
    struct.pack_into("<II", raw, 0x98 + 96 + 5 * 8, 0x1180, len(relocations))
    struct.pack_into("<I", raw, 0x98 + 16, 0x1160)
    mapping = ("synthetic\n Preferred load address is 00400000\n"
               " Start         Length     Name                   Class\n"
               " 0001:00000000 00000200H .text CODE\n"
               " 0002:00000000 00000004H .CRT$XCA DATA\n"
               f" 0002:00000004 {4 * len(roots):08x}H .CRT$XCU DATA\n"
               f" 0002:{4 + 4 * len(roots):08x} 00000004H .CRT$XCZ DATA\n"
               " Address         Publics by Value              Rva+Base     Lib:Object\n")
    rows = lambda public: "".join(
        f" 0001:{row['offset']:08x} {row['name']} {0x401000 + row['offset']:08x} f {row['owner']}\n"
        for row in functions if row["public"] is public)
    mapping += rows(True)
    mapping += "".join(f" 0002:{address - 0x402000:08x} {name} {address:08x} storage.obj\n"
                       for name, address in data.items())
    mapping += " entry point at 0001:00000160\n Static symbols\n" + rows(False)
    log = "Start Pass1\nEnd Pass1\nStart Pass2\n" + "".join(path.name + "\n" for path in context.objects) + "End Pass2\n"
    receipt = json.loads((context.directory / "link.json").read_bytes())
    receipt.update(arguments=[str(path) for path in context.objects], explicit_inputs=[str(path) for path in context.objects],
                   link_log_sha256=stamp(log.encode())["sha256"],
                   files=[{"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())}
                          for path in [context.root / "linker", *context.objects]])
    for name, content in [("app.exe", bytes(raw)), ("app.map", mapping.encode()),
                          ("link.log", log.encode()), ("link.json", json.dumps(receipt).encode())]:
        update_artifact(context, name, content)
    context.graph_roots = [{"owner": definitions[name]["owner"], "symbol": name} for name in roots]
    return context


def direct_graph(context, **kwargs):
    receipt = (context.directory / "link.json").read_bytes()
    coverage, records = _input_inventory(receipt, (context.directory / "link.log").read_bytes())
    objects = SelectedObjects(parse_link_files(receipt), coverage, records)
    raw = (context.directory / "app.exe").read_bytes()
    return observe_code_graph(objects, parse_link_map((context.directory / "app.map").read_bytes()),
                              PeImage(raw, hashlib.sha256(raw).hexdigest()), context.graph_roots, **kwargs)


def call_graph(root, *, helper=b"\xc3", helper_owner="helper.obj", helper_offset=0x100):
    return graph_build(root, [function("_$E1", 0, b"\xe8\0\0\0\0\xc3", edges=((1, "_helper", 20),)),
                              function("_helper", helper_offset, helper, public=True, owner=helper_owner)])


def test_changed_selected_helper_is_not_hidden_by_unchanged_initializer(tmp_path):
    old, new = call_graph(tmp_path / "old"), call_graph(tmp_path / "new", helper=b"\x90\xc3")
    report = compare_build_initializers(old, new)
    assert report["ordered_code_correspondence"] == "pass"
    assert report["code_dependencies"]["result"] == report["result"] == "fail"
    assert report["code_dependencies"]["baseline"]["result"] == "pass"
    assert report["code_dependencies"]["current"]["result"] == "pass"
    assert report["input_consistency"] == "pass"
    assert report["summary"]["observations_complete"] is True
    assert report["summary"]["ordered_equal"] is True
    assert report["summary"]["added_registrations"] == report["summary"]["removed_registrations"] == 0
    assert report["remaining_linked_effects"] == "unknown" and report["ready_for_collection"] is False
    json.dumps(report)  # Neither selected COFF nor PE bytes leak into reports.


def test_pe_only_helper_damage_is_a_measured_failure(tmp_path):
    old, new = call_graph(tmp_path / "old"), call_graph(tmp_path / "new")
    raw = bytearray((new.directory / "app.exe").read_bytes())
    raw[512 + 0x100] = 0x90
    update_artifact(new, "app.exe", raw)
    report = compare_build_initializers(old, new)
    assert report["ordered_code_correspondence"] == "pass"
    dependency = report["code_dependencies"]["current"]
    assert dependency["result"] == report["result"] == "fail"
    assert report["input_consistency"] == "fail"
    assert dependency["nodes"][1]["proof"]["code"]["result"] == "fail"


def test_helper_rehoming_uses_fresh_selected_owner_and_independent_placement(tmp_path):
    old = call_graph(tmp_path / "old")
    new = call_graph(tmp_path / "new", helper_owner="rehome.obj", helper_offset=0x120)
    report = compare_build_initializers(old, new)
    assert report["result"] == "pass"
    assert report["code_dependencies"]["graph_equal"] is True
    assert report["code_dependencies"]["current"]["nodes"][1]["owner"] == "rehome.obj"


@pytest.mark.parametrize("instruction,kind", [(b"\xe9\0\0\0\0", 20), (b"\x68\0\0\0\0\xc3", 6)])
def test_local_tail_jumps_and_pushed_callbacks_are_followed(tmp_path, instruction, kind):
    def fixture(path, helper):
        return graph_build(path, [function("_$E1", 0, instruction, edges=((1, "_$local", kind),)),
                                  function("_$local", 0x100, helper)])
    old, new = fixture(tmp_path / "old", b"\xc3"), fixture(tmp_path / "new", b"\x90\xc3")
    report = compare_build_initializers(old, new)
    assert report["ordered_code_correspondence"] == "pass"
    assert report["result"] == "fail"
    graph = report["code_dependencies"]["current"]
    assert [node["symbol"] for node in graph["nodes"]] == ["_$E1", "_$local"]
    assert graph["nodes"][0]["edges"][0]["target"] == {"node": 1}


def test_cycles_terminate_without_dropping_back_edges(tmp_path):
    context = graph_build(tmp_path / "cycle", [
        function("_$E1", 0, b"\xe9\0\0\0\0", edges=((1, "_$local", 20),)),
        function("_$local", 0x100, b"\xe9\0\0\0\0", edges=((1, "_$E1", 20),))])
    report = direct_graph(context)
    assert report["result"] == "pass" and len(report["nodes"]) == 2
    assert report["nodes"][1]["edges"][0]["target"] == {"node": 0}


@pytest.mark.parametrize("limit", [0, -1, True, 4097, 1.5])
def test_invalid_resource_limits_refuse(tmp_path, limit):
    with pytest.raises(ValueError, match="bounded positive"):
        direct_graph(call_graph(tmp_path / "bounded"), max_functions=limit)


def test_exhausted_resource_bound_is_unknown_not_a_partial_positive_graph(tmp_path):
    report = direct_graph(call_graph(tmp_path / "bounded"), max_functions=1)
    assert report["result"] == "unknown" and report["graph_fingerprint"] is None
    assert "resource bound" in report["gaps"][0]["reason"]


def test_external_data_is_explicitly_unproved_not_an_invented_code_target(tmp_path):
    context = graph_build(tmp_path / "data", [
        function("_$E1", 0, b"\xa1\0\0\0\0\xc3", edges=((1, "_global", 6),))],
        data={"_global": 0x402100})
    report = direct_graph(context)
    assert report["result"] == "pass" and len(report["nodes"]) == 1
    assert report["data_references"] == [{"node": 0, "offset": 1, "kind": "absolute32",
                                         "target": {"data": {"external": "_global"}},
                                         "address": 0x402100, "addend": 0, "contents": "unknown"}]
    assert report["remaining_linked_effects"] == "unknown" and report["ready_for_collection"] is False


def test_added_noop_initializer_is_not_deduplicated_by_graph_identity(tmp_path):
    old = graph_build(tmp_path / "old", [function("_$E1", 0)])
    new = graph_build(tmp_path / "new", [function("_$E1", 0), function("_$E2", 0x20)], roots=("_$E1", "_$E2"))
    report = compare_build_initializers(old, new)
    assert report["ordered_code_correspondence"] == report["code_dependencies"]["result"] == "fail"
    assert report["code_dependencies"]["current"]["roots"] == [0, 1]


@pytest.mark.parametrize("damage", ["missing", "ambiguous"])
def test_missing_or_ambiguous_selected_helper_cannot_pass_from_map_alone(tmp_path, damage):
    old, new = call_graph(tmp_path / "old"), call_graph(tmp_path / "new")
    receipt = json.loads((new.directory / "link.json").read_bytes())
    log = (new.directory / "link.log").read_text()
    if damage == "missing":
        # A valid refreshed receipt and map still do not establish selection
        # when the final linker pass does not name the helper object.
        log = log.replace("helper.obj\n", "")
    else:
        extra = new.directory / "other/helper.obj"
        extra.parent.mkdir()
        extra.write_bytes((new.directory / "helper.obj").read_bytes())
        receipt["arguments"].append(str(extra))
        receipt["explicit_inputs"].append(str(extra))
        receipt["files"].append({"path": str(extra), "before": stamp(extra.read_bytes()),
                                  "after": stamp(extra.read_bytes())})
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(new, "link.log", log.encode())
    update_artifact(new, "link.json", json.dumps(receipt).encode())
    report = compare_build_initializers(old, new)
    if damage == "ambiguous":
        # Native complete-input coverage refuses ambiguous explicit basenames
        # even before the initializer/dependency split. Do not weaken it to
        # manufacture a superficially measurable root in this fixture.
        assert report["current"]["reason"] == "linked observation requires complete input candidate coverage"
        assert report["ordered_code_correspondence"] == report["result"] == "unknown"
        assert report["code_dependencies"]["result"] == "unknown"
        return
    assert report["ordered_code_correspondence"] == "pass"
    assert report["code_dependencies"]["current"]["result"] == report["result"] == "unknown"
    assert report["code_dependencies"]["current"]["graph_fingerprint"] is None
    assert report["code_dependencies"]["current"]["nodes"][1]["symbol"] == "_helper"


def test_shared_definition_is_not_equivalent_to_two_identical_definitions(tmp_path):
    code = b"\xe8\0\0\0\0\xe8\0\0\0\0\xc3"
    shared = graph_build(tmp_path / "shared", [
        function("_$E1", 0, code, edges=((1, "_$h1", 20), (6, "_$h1", 20))),
        function("_$h1", 0x100)])
    duplicated = graph_build(tmp_path / "duplicated", [
        function("_$E1", 0, code, edges=((1, "_$h1", 20), (6, "_$h2", 20))),
        function("_$h1", 0x100), function("_$h2", 0x120)])
    old, new = direct_graph(shared), direct_graph(duplicated)
    assert old["result"] == new["result"] == "pass"
    assert old["nodes"][0]["edges"][0]["target"] == old["nodes"][0]["edges"][1]["target"] == {"node": 1}
    assert new["nodes"][0]["edges"][1]["target"] == {"node": 2}
    assert len(old["nodes"]) == 2 and len(new["nodes"]) == 3
    assert compare_code_graphs(old, new)["result"] == "fail"


def test_changed_linker_folding_is_not_erased_by_identical_definition_bytes(tmp_path):
    def fixture(root, offset):
        return graph_build(root, [
            function("_$E1", 0, b"\xe8\0\0\0\0\xe8\0\0\0\0\xc3",
                     edges=((1, "_$h1", 20), (6, "_$h2", 20))),
            function("_$h1", 0x100), function("_$h2", offset)])
    old, new = fixture(tmp_path / "distinct", 0x120), fixture(tmp_path / "folded", 0x100)
    report = compare_build_initializers(old, new)
    assert report["ordered_code_correspondence"] == "pass"
    assert report["code_dependencies"]["baseline"]["result"] == "pass"
    assert report["code_dependencies"]["current"]["result"] == "pass"
    assert report["code_dependencies"]["result"] == report["result"] == "fail"
    assert len(report["code_dependencies"]["current"]["nodes"]) == 3


def test_consistent_folded_aliases_remain_separate_definitions_in_a_shared_address_class(tmp_path):
    def fixture(root):
        return graph_build(root, [
            function("_$E1", 0, b"\xe8\0\0\0\0\xe8\0\0\0\0\xc3",
                     edges=((1, "_$h1", 20), (6, "_$h2", 20))),
            function("_$h1", 0x100), function("_$h2", 0x100)])
    report = compare_build_initializers(fixture(tmp_path / "old"), fixture(tmp_path / "new"))
    assert report["result"] == "pass"
    nodes = report["code_dependencies"]["current"]["nodes"]
    assert len(nodes) == 3 and nodes[1]["symbol"] != nodes[2]["symbol"]
    assert nodes[1]["proof"]["address"] == nodes[2]["proof"]["address"]
    assert nodes[0]["edges"][0]["target"] != nodes[0]["edges"][1]["target"]


def test_unknown_dependency_does_not_erase_measured_root_code_damage(tmp_path):
    context = call_graph(tmp_path / "changed")
    raw = bytearray((context.directory / "app.exe").read_bytes())
    raw[512] = 0xE9
    update_artifact(context, "app.exe", raw)
    # Keep the valid independent target placement but remove final selection.
    log = (context.directory / "link.log").read_text().replace("helper.obj\n", "")
    receipt = json.loads((context.directory / "link.json").read_bytes())
    receipt["link_log_sha256"] = stamp(log.encode())["sha256"]
    update_artifact(context, "link.log", log.encode())
    update_artifact(context, "link.json", json.dumps(receipt).encode())
    report = direct_graph(context)
    assert report["result"] == "fail" and report["graph_fingerprint"] is None
    assert report["nodes"][0]["result"] == "fail" and report["nodes"][1]["result"] == "unknown"


def test_changed_helper_file_without_a_new_capture_is_not_reused(tmp_path):
    context = call_graph(tmp_path / "changed")
    assert observe_build_initializers(context)["code_dependencies"]["result"] == "pass"
    path = context.directory / "helper.obj"
    path.write_bytes(path.read_bytes() + b"uncaptured mutation")
    report = observe_build_initializers(context)
    assert report["result"] == "unknown"
    assert not report.get("code_dependencies")


def test_expired_context_cannot_return_even_a_previous_matching_graph(tmp_path):
    old, new = call_graph(tmp_path / "old"), call_graph(tmp_path / "new")
    assert compare_build_initializers(old, new)["result"] == "pass"
    new.alive = False
    with pytest.raises(ValueError, match="expired"):
        compare_build_initializers(old, new)


@pytest.mark.parametrize("self_target", [False, True])
@pytest.mark.parametrize("kind,opcode", [(20, b"\xe9"), (6, b"\x68")])
def test_code_reference_addend_cannot_escape_the_observed_target_body(tmp_path, self_target, kind, opcode):
    def fixture(root, hidden_body):
        target = "_$E1" if self_target else "_$helper"
        addend = 0x120 if self_target else 0x20
        code = opcode + struct.pack("<i", addend) + b"\xc3"
        return graph_build(root, [function("_$E1", 0, code, edges=((1, target, kind),)),
                                  function("_$helper", 0x100), function("_$hidden", 0x120, hidden_body)])
    old, new = fixture(tmp_path / "old", b"\xc3"), fixture(tmp_path / "new", b"\x90\xc3")
    report = compare_build_initializers(old, new)
    # The actual operand lands in _$hidden, not the named symbol's COMDAT.
    # Verifying the same base symbol and stopping there is not code closure.
    assert report["ordered_code_correspondence"] == "pass"
    assert report["code_dependencies"]["result"] == report["result"] == "unknown"


@pytest.mark.parametrize("self_target", [False, True])
@pytest.mark.parametrize("kind,opcode", [(20, b"\xe9"), (6, b"\x68")])
def test_in_bounds_code_addends_keep_complete_target_body_coverage(tmp_path, self_target, kind, opcode):
    target, addend = ("_$E1", 5) if self_target else ("_$helper", 1)
    context = graph_build(tmp_path / "bounded", [
        function("_$E1", 0, opcode + struct.pack("<i", addend) + b"\xc3", edges=((1, target, kind),)),
        function("_$helper", 0x100, b"\x90\xc3")])
    report = direct_graph(context)
    assert report["result"] == "pass"
    assert len(report["nodes"]) == (1 if self_target else 2)


def test_repeated_pointer_to_same_root_still_counts_as_an_added_registration(tmp_path):
    old = graph_build(tmp_path / "old", [function("_$E1", 0)])
    new = graph_build(tmp_path / "new", [function("_$E1", 0)], roots=("_$E1", "_$E1"))
    report = compare_build_initializers(old, new)
    assert report["result"] == report["code_dependencies"]["result"] == "fail"
    assert report["code_dependencies"]["current"]["roots"] == [0, 0]
    assert len(report["code_dependencies"]["current"]["nodes"]) == 1
    assert report["summary"]["observations_complete"] is True
    assert report["summary"]["ordered_equal"] is False
    assert report["summary"]["added_registrations"] == 1
    assert report["summary"]["removed_registrations"] == 0
