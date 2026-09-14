"""Physical own-PDB controls plus explicitly synthetic algebra damage tests."""
from copy import deepcopy
import struct

import pytest

from match_recovery import linked_runtime_residual as runtime
from match_recovery.linked_context import linked_build
from match_recovery.linked_code_residual import normalize_residual, compare_residual
from match_recovery.msf import SmallMsf
from match_recovery.pe_coff import Coff
from test_linked_modules import fixture as module_fixture, set_modules
from test_linked_codeview import object_bytes, module as procedure_module
from test_codeview_procedures import procedure
from test_codeview_type_graph import graph as type_graph, encode_tpi
from test_pdb_contributions import module, contribution
from test_ordinary_contributions import set_contributions
from test_slot_local_initializer_order import replace_object
from test_link_file_evidence import stamp
from test_linked_initializers import update_artifact
from test_msf import small_msf


def fixture(root, *, member=b"raw", frame=8, wrong_pdb=False):
    context = module_fixture(root)
    types = type_graph(name=member)
    local = (0x1006, struct.pack("<iI", frame, 0x1002) + b"\x05input")
    original = object_bytes(procedure(type_index=0x1000, locals_=[local]))
    sym, = struct.unpack_from("<I", original, 8)
    stream = struct.pack("<I", 2) + b"".join(struct.pack("<H", len(value)) + value for value in types.values())
    header = bytearray(original[:100])
    struct.pack_into("<H", header, 2, 3)
    struct.pack_into("<I", header, 8, sym + 40 + len(stream))
    for at in (20, 60):
        for field in (20, 24):
            value, = struct.unpack_from("<I", header, at + field)
            if value:
                struct.pack_into("<I", header, at + field, value + 40)
    section = struct.pack("<8sIIIIIIHHI", b".debug$T", 0, 0, len(stream), sym + 40, 0, 0, 0, 0, 0x42100040)
    raw = bytes(header) + section + original[100:sym] + stream + original[sym:]
    replace_object(context, raw, index=0)
    context.report["compile_link"]["compilations"][0]["compilation"]["output"]["after"] = stamp(raw)
    image = bytearray((context.directory / "app.exe").read_bytes())
    image[0x200:0x220] = Coff(raw).sections[0].data
    update_artifact(context, "app.exe", bytes(image))
    mapping = (context.directory / "app.map").read_text()
    mapping = mapping.replace(" entry point at", " 0001:00000000 _f 00401000 f unit0.obj\n entry point at")
    mapping = mapping.replace(" 0001:00000000 _$E1 00401000 f unit0.obj\n", "")
    update_artifact(context, "app.map", mapping.encode())
    payload = procedure_module(type_index=0x1000, locals_=[local], section=1)
    objpath = str(context.objects[0]).encode()
    set_modules(context, module(objpath, objpath, stream=6, lengths=(len(payload), 0, 0)))
    set_contributions(context, contribution(offset=0, size=32, flags=0x60501020))
    msf = SmallMsf((context.directory / "app.pdb").read_bytes())
    streams = [msf.stream(i) for i in range(msf.stream_count)]
    streams[2] = encode_tpi(type_graph(name=b"wrong") if wrong_pdb else types)
    while len(streams) <= 6:
        streams.append(None)
    streams[6] = payload
    update_artifact(context, "app.pdb", small_msf(streams))
    return context


def observe(native):
    graph = native.code([{"owner": "unit0.obj", "symbol": "_f"}])
    assert graph["result"] == "pass", graph
    assert graph["nodes"][0]["proof"]["debug_regions"][0]["types"]["format"] == "codeview5_type_graph"
    return graph


def compare(a, b, *, damage=None):
    with linked_build(a) as old, linked_build(b) as new:
        graphs = {"baseline": observe(old), "current": observe(new)}
        producers = {version: {0: {"unit": "src/unit.cpp", "symbol": "_f"}} for version in graphs}
        omitted = {version: set() for version in graphs}
        if damage:
            damage(graphs)
        strict = {version: normalize_residual(graph, producers[version], omitted[version]) for version, graph in graphs.items()}
        result = runtime.compare_runtime_code({"baseline": old, "current": new}, graphs, producers, omitted, max_nodes=4096)
        return compare_residual(strict["baseline"], strict["current"]), result


def test_real_native_own_pdb_type_change_is_separate_from_runtime(tmp_path):
    strict, result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new", member=b"owned"))
    assert strict["result"] == "fail"
    assert result["result"] == "pass", result
    assert result["check"] == "native_runtime_code_residual"
    assert result["projected_type_graphs"] == {"baseline": 1, "current": 1}
    assert result["outside_scope_verification"] == "unknown"
    assert not result["ready_for_collection"]


def test_real_native_changed_local_offset_remains_failure(tmp_path):
    strict, result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new", member=b"owned", frame=12))
    assert strict["result"] == result["result"] == "fail"


def test_real_native_wrong_own_pdb_never_reaches_projection(tmp_path):
    with linked_build(fixture(tmp_path / "project", wrong_pdb=True)) as native:
        graph = native.code([{"owner": "unit0.obj", "symbol": "_f"}])
        assert graph["result"] != "pass"


@pytest.mark.parametrize("damage", ["length", "locals", "runtime", "other_types", "fpo"])
def test_synthetic_algebra_other_fields_are_not_projected(tmp_path, damage):
    def mutate(graphs):
        body = graphs["current"]["canonical_nodes"][0]
        proof = graphs["current"]["nodes"][0]["proof"]
        if damage in ("length", "locals"):
            row = body["debug_regions"][0]
            if damage == "length": row["length"] += 1
            else: row["locals"][0]["name"] = "changed"
            proof["debug_regions"] = deepcopy(body["debug_regions"])
        elif damage == "runtime":
            body["regions"][0]["code_sha256"] = "changed code"
        elif damage == "other_types":
            body["types"] = {"not": "procedure metadata"}
        else:
            for version in graphs:
                current = graphs[version]
                current["canonical_nodes"][0]["debug_regions"].append({"format": "fpo_data", "parent": 0, "data": version})
                current["nodes"][0]["proof"]["debug_regions"] = deepcopy(current["canonical_nodes"][0]["debug_regions"])
    _, result = compare(fixture(tmp_path / "old"), fixture(tmp_path / "new"), damage=mutate)
    assert result["result"] == "fail"


@pytest.mark.parametrize("damage", ["proof", "format", "types", "index"])
def test_synthetic_algebra_unsupported_or_unverified_metadata_refuses(tmp_path, damage):
    def mutate(graphs):
        body = graphs["current"]["canonical_nodes"][0]
        body["debug_regions"] = deepcopy(body["debug_regions"])
        region = body["debug_regions"][0]
        if damage == "proof": region["name"] = "unverified replacement"
        elif damage == "format": region["format"] = "unknown"
        elif damage == "types": region["types"]["format"] = "unknown"
        else: region["types"]["roots"][0] = {"node": 100000}
        if damage != "proof":
            graphs["current"]["nodes"][0]["proof"]["debug_regions"] = deepcopy(body["debug_regions"])
    with pytest.raises(ValueError):
        compare(fixture(tmp_path / "old"), fixture(tmp_path / "new"), damage=mutate)


@pytest.mark.parametrize("damage", ["graph", "producer", "omitted", "epoch"])
def test_final_guards_remain_after_runtime_comparison(tmp_path, monkeypatch, damage):
    a, b = fixture(tmp_path / "old"), fixture(tmp_path / "new")
    with linked_build(a) as old, linked_build(b) as new:
        graphs = {"baseline": observe(old), "current": observe(new)}
        ids = {version: {0: "physical-root"} for version in graphs}
        omitted = {version: set() for version in graphs}
        original = runtime.compare_residual
        def mutate(*args):
            result = original(*args)
            if damage == "graph": graphs["baseline"]["late"] = True
            elif damage == "producer": ids["baseline"][0] = "wrong"
            elif damage == "omitted": omitted["baseline"].add(0)
            else: a.alive = False
            return result
        monkeypatch.setattr(runtime, "compare_residual", mutate)
        try:
            with pytest.raises(ValueError, match="changed|expired"):
                runtime.compare_runtime_code({"baseline": old, "current": new}, graphs, ids, omitted, max_nodes=4096)
        finally:
            a.alive = True  # Teardown only; failed observation is not accepted.
