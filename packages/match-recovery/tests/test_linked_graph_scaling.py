"""Large physical COFF/MAP/PE graph controls, not application acceptance."""

from dataclasses import replace
import json
import struct

import pytest

from match_recovery import linked_map_index, linked_interval_index
from match_recovery.linked_objects import SelectedObjects
from match_recovery.linked_map_index import MapIndexes, index_for
from match_recovery.msvc_link_map import LinkMap, Public
from test_binary import pe_fixture
from test_linked_code_graph import function, object_bytes, direct_graph
from test_linked_initializers import build, stamp, update_artifact


def wide_graph(root, count, *, damage=False, chain=False):
    context = build(root)
    functions = [function(f"_f{index:05d}", index * 16, public=True, owner="wide.obj") for index in range(count)]
    if chain:
        for index, row in enumerate(functions[:-1]):
            row.update(code=b"\xe8\0\0\0\0\xc3", edges=((1, functions[index + 1]["name"], 20),))
    obj = context.directory / "wide.obj"
    obj.write_bytes(object_bytes(functions, (), {}))
    context.objects = [obj]
    raw = bytearray(pe_fixture((b"\xc3" + b"\xcc" * 15) * count))
    if chain:
        for index in range(count - 1):
            raw[512 + index * 16:512 + (index + 1) * 16] = b"\xe8\x0b\0\0\0\xc3" + b"\xcc" * 10
    struct.pack_into("<I", raw, 0x98 + 16, 0x1000)
    struct.pack_into("<IIII", raw, 0x180, count * 16, 0x1000, count * 16, 512)
    struct.pack_into("<I", raw, 0x178 + 36, 0x60000020)
    if damage:
        raw[512 + (count - 1) * 16] = 0x90
    mapping = ("large physical fixture\n Preferred load address is 00400000\n"
               " Address         Publics by Value              Rva+Base     Lib:Object\n")
    mapping += "".join(f" 0001:{index * 16:08x} {row['name']} {0x401000 + index * 16:08x} f wide.obj\n"
                       for index, row in enumerate(functions))
    mapping += " entry point at 0001:00000000\n Static symbols\n"
    log = b"Start Pass1\nEnd Pass1\nStart Pass2\nwide.obj\nEnd Pass2\n"
    receipt = json.loads((context.directory / "link.json").read_bytes())
    receipt.update(arguments=[str(obj)], explicit_inputs=[str(obj)], link_log_sha256=stamp(log)["sha256"],
                   files=[{"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())}
                          for path in (context.root / "linker", obj)])
    for name, contents in (("app.exe", bytes(raw)), ("app.map", mapping.encode()), ("link.log", log),
                           ("link.json", json.dumps(receipt).encode())):
        update_artifact(context, name, contents)
    context.graph_roots = [{"owner": "wide.obj", "symbol": row["name"]} for row in functions]
    return context


@pytest.mark.parametrize("damage", [False, True])
def test_complete_physical_graph_beyond_old_limit_keeps_last_function(tmp_path, monkeypatch, damage):
    count = 4099
    context = wide_graph(tmp_path / "large", count, damage=damage)
    with pytest.raises(ValueError, match="resource bound"):
        direct_graph(context)  # The standalone default still explicitly refuses.
    constructions, visits = [], []
    original_index = linked_map_index.MapIndex.__init__
    original_collect = linked_interval_index._collect

    def index(self, mapping):
        constructions.append(True)
        original_index(self, mapping)

    def collect(*args):
        visits.append(True)
        return original_collect(*args)

    monkeypatch.setattr(linked_map_index.MapIndex, "__init__", index)
    monkeypatch.setattr(linked_interval_index, "_collect", collect)
    report = direct_graph(context, max_functions=count)
    assert report["result"] == ("fail" if damage else "pass"), report.get("gaps")
    assert len(report["nodes"]) == len(report["roots"]) == count
    assert report["nodes"][-1]["symbol"] == "_f04098"
    assert report["nodes"][-1]["result"] == ("fail" if damage else "pass")
    assert len(constructions) == 1
    assert len(visits) < count * 100
    assert report["remaining_linked_effects"] == "unknown"


def test_map_indexes_preserve_duplicates_absolute_rows_and_exact_views():
    row = Public("_same", 1, 0, 0x401000, True, "A.OBJ")
    absolute = replace(row, segment=0, address=7, function=False)
    mapping = LinkMap(0x400000, (1, 0), (row, row, absolute), statics=(row, row))
    cache = MapIndexes()
    index = cache.for_map(mapping)
    assert index.publics["_same"] == (row, row, absolute)
    assert index.statics["a.obj", "_same"] == (row, row)
    assert cache.for_map(mapping) is index
    rewritten = replace(mapping, statics=(replace(row, owner="B.obj"),))
    other = cache.for_map(rewritten)
    assert other is not index and ("b.obj", "_same") in other.statics
    with pytest.raises(ValueError, match="another physical or owner-rewritten"):
        index_for(rewritten, index)


def test_duplicate_root_flood_is_bounded_without_inventing_more_definitions(tmp_path):
    context = wide_graph(tmp_path / "small", 1)
    context.graph_roots *= 17
    with pytest.raises(ValueError, match="root request"):
        direct_graph(context, max_functions=1)


def test_large_reachable_chain_batches_absent_constant_population_scans(tmp_path, monkeypatch):
    count = 4099
    context = wide_graph(tmp_path / "chain", count, chain=True)
    context.graph_roots = context.graph_roots[:1]
    population, resolutions = [], []
    original_population = SelectedObjects._constant_population
    original_resolve = SelectedObjects.resolve_constants

    def scanned(self):
        population.append(True)
        return original_population(self)

    def resolved(self, *args, **kwargs):
        resolutions.append(True)
        return original_resolve(self, *args, **kwargs)

    monkeypatch.setattr(SelectedObjects, "_constant_population", scanned)
    monkeypatch.setattr(SelectedObjects, "resolve_constants", resolved)
    report = direct_graph(context, max_functions=count)
    assert report["result"] == "pass", report.get("gaps")
    assert len(report["nodes"]) == count and len(report["roots"]) == 1
    assert report["nodes"][-1]["symbol"] == "_f04098"
    assert len(population) == 2  # Batch entry and independently guarded import advisory.
    assert not resolutions  # Ordinary function references are not absolute constants.
    assert report["remaining_linked_effects"] == "unknown"
