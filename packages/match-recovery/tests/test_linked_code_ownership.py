"""Native receipt / physical producer correspondence, not acceptance evidence."""

from copy import deepcopy
from dataclasses import replace
from hashlib import sha256
from pathlib import Path
import struct

import pytest

from match_recovery import linked_code_ownership as native
from match_recovery.linked_code_graph import observe_code_graph
from match_recovery.linked_folding import bind_folding
from match_recovery.linked_objects import SelectedObjects
from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pe_coff import PeImage
from test_binary import coff_fixture
from test_build_graph_commands import Context, stamp
from test_link_map import executable
from test_linked_folded_graph import fixture as fold_fixture, graph as fold_graph
from test_linked_import_thunks import physical, image_thunk, mapping as thunk_mapping, OWNER
from test_linked_objects import inventory


CODE = b"\x90" * 15 + b"\xc3"


def install_image(context, raw):
    path = context.directory / "application.exe"
    path.write_bytes(raw)
    context.report["outputs"] = [{"path": path.name, "format": "pe32-i386", **stamp(raw)}]
    return PeImage(raw, sha256(raw).hexdigest())


def compiled(context, raw=None):
    raw = coff_fixture(CODE) if raw is None else raw
    row = context.proof["compilations"][0]
    path = Path(row["output"])
    path.write_bytes(raw)
    row["compilation"]["output"]["after"] = stamp(raw)
    return path


def fixture(root, *, raw=None, storage=2):
    context = Context(root, ["src/a.cpp"])
    path = compiled(context, coff_fixture(CODE, storage=storage) if raw is None else raw)
    tool = root / "linker"
    tool.write_bytes(b"physical linker")
    log = f"Start Pass1\nEnd Pass1\nStart Pass2\n{path.name}\nEnd Pass2\n"
    evidence = {"schema_version": 2, "link_log_sha256": sha256(log.encode()).hexdigest(),
                "cwd": str(root), "tool": str(tool), "arguments": [str(path)],
                "explicit_inputs": [str(path)], "files": [{"path": str(item),
                "before": stamp(item.read_bytes()), "after": stamp(item.read_bytes())} for item in (path, tool)],
                "library_searches": [], "generated_exports": [], "capture_gaps": []}
    objects = SelectedObjects(*inventory(evidence, log))
    image = install_image(context, executable(CODE))
    entry = f" 0001:00000000 _f 00401000 f {path.name}\n"
    mapping = parse_link_map((" Preferred load address is 00400000\n"
        " Address Publics by Value Rva+Base Lib:Object\n"
        + (entry if storage == 2 else f" 0001:00000020 _other 00401020 f {path.name}\n") + " entry point at 0001:00000000\n"
        + (" Static symbols\n" + entry if storage == 3 else "")).encode())
    graph = observe_code_graph(objects, mapping, image, [{"owner": path.name, "symbol": "_f"}])
    assert graph["result"] == "pass", graph["gaps"]
    return context, objects, mapping, graph


def test_compiled_identity_is_source_and_symbol_not_whole_object_bytes(tmp_path):
    before = fixture(tmp_path / "before")
    # COFF timestamp is unrelated to the observed code, but changes the full SHA.
    changed = bytearray(coff_fixture(CODE))
    struct.pack_into("<I", changed, 4, 12345)
    after = fixture(tmp_path / "after", raw=bytes(changed))
    old = native.identities(before[0], before[3]["nodes"])
    new = native.identities(after[0], after[3]["nodes"])
    assert before[3]["nodes"][0]["object_sha256"] != after[3]["nodes"][0]["object_sha256"]
    assert old == new == {0: {"kind": "compiled", "unit": "src/a.cpp", "symbol": "_f"}}
    assert native.resolve(after[0], old.values(), after[1], after[2]) == ({"owner": "a.cpp.obj", "symbol": "_f"},)


@pytest.mark.parametrize("storage", [2, 3])
def test_exact_map_storage_and_root_repetitions_survive_resolution(tmp_path, storage):
    context, objects, mapping, graph = fixture(tmp_path, storage=storage)
    identity, = native.identities(context, graph["nodes"]).values()
    root = {"owner": "a.cpp.obj", "symbol": "_f"}
    assert native.resolve(context, [identity, identity], objects, mapping) == (root, root)
    wrong = replace(mapping, publics=mapping.statics or (), statics=mapping.publics)
    with pytest.raises(ValueError, match="storage"):
        native.resolve(context, [identity], objects, wrong)


@pytest.mark.parametrize("damage", ["duplicate", "unordered", "negative", "bool", "unknown", "symbol", "owner", "sha"])
def test_node_ids_and_original_proofs_are_not_unchecked_metadata(tmp_path, damage):
    context, _, _, graph = fixture(tmp_path)
    nodes = deepcopy(graph["nodes"])
    if damage == "duplicate": nodes += deepcopy(nodes)
    elif damage == "unordered": nodes = [{**nodes[0], "id": 2}, nodes[0]]
    elif damage == "negative": nodes[0]["id"] = -1
    elif damage == "bool": nodes[0]["id"] = True
    elif damage == "unknown": nodes[0]["result"] = "unknown"
    elif damage == "symbol": nodes[0]["symbol"] = "_different"
    elif damage == "owner": nodes[0]["source_owner"] = "different.obj"
    else: nodes[0]["object_sha256"] = "a" * 64
    with pytest.raises(ValueError): native.identities(context, nodes)


def test_folded_member_keeps_original_archive_producer_not_winner(tmp_path):
    context = Context(tmp_path, ["src/unrelated.cpp"])
    compiled(context)
    values = fold_fixture(tmp_path)
    graph = fold_graph(values)
    assert graph["result"] == "pass"
    install_image(context, values[2].data)
    identities = native.identities(context, graph["nodes"])
    assert identities[1] == {"kind": "dependency", "input": {"kind": "project", "path": "runtime.lib"},
        "member": "runtime.obj", "occurrence": 0, "symbol": "_g", "object_sha256": graph["nodes"][1]["object_sha256"]}
    result = native.resolve(context, identities.values(), values[0], values[1], bind_folding(*values))
    assert result == ({"owner": "app.obj", "symbol": "_f"}, {"owner": "RUNTIME:runtime.obj", "symbol": "_g"})
    with pytest.raises(ValueError, match="missing, discarded"):
        native.resolve(context, [identities[1]], values[0], values[1])


def test_short_import_occurrence_resolves_despite_ambiguous_owner_family(tmp_path):
    context = Context(tmp_path, ["src/unrelated.cpp"])
    compiled(context)
    objects = physical(tmp_path)
    image = install_image(context, image_thunk())
    mapping = thunk_mapping()
    graph = observe_code_graph(objects, mapping, image, [{"owner": OWNER, "symbol": "_Paint@4"}])
    assert graph["result"] == "pass"
    with pytest.raises(ValueError): objects.resolve(OWNER)
    identity, = native.identities(context, graph["nodes"]).values()
    assert identity["member"] == "DEMO.dll" and identity["occurrence"] == 0
    assert native.resolve(context, [identity], objects, mapping) == ({"owner": OWNER, "symbol": "_Paint@4"},)
    with pytest.raises(ValueError):
        native.resolve(context, [{**identity, "occurrence": 1}], objects, mapping)


@pytest.mark.parametrize("damage", ["missing_tu", "extra_key", "wrong_symbol", "wrong_owner", "discarded", "duplicate_map"])
def test_missing_discarded_and_ambiguous_current_producers_never_pair_by_bytes(tmp_path, damage):
    context, objects, mapping, graph = fixture(tmp_path)
    identity, = native.identities(context, graph["nodes"]).values()
    if damage == "missing_tu": identity["unit"] = "src/same_bytes.cpp"
    elif damage == "extra_key": identity["object_sha256"] = graph["nodes"][0]["object_sha256"]
    elif damage == "wrong_symbol": identity["symbol"] = "_different"
    elif damage == "wrong_owner": mapping = replace(mapping, publics=(replace(mapping.publics[0], owner="missing.obj"),))
    elif damage == "discarded": mapping = replace(mapping, publics=())
    else: mapping = replace(mapping, publics=mapping.publics * 2)
    with pytest.raises(ValueError): native.resolve(context, [identity], objects, mapping)


@pytest.mark.parametrize("phase", ["identities", "resolve"])
def test_context_expiring_at_return_refuses(tmp_path, phase):
    context, objects, mapping, graph = fixture(tmp_path)
    identity = {"kind": "compiled", "unit": "src/a.cpp", "symbol": "_f"}
    context.fail_at = context.checks + 2
    with pytest.raises(ValueError, match="context changed"):
        if phase == "identities": native.identities(context, graph["nodes"])
        else: native.resolve(context, [identity], objects, mapping)


@pytest.mark.parametrize("phase", ["identities", "resolve"])
def test_changed_compilation_output_cannot_borrow_receipt(tmp_path, phase):
    context, objects, mapping, graph = fixture(tmp_path)
    Path(context.proof["compilations"][0]["output"]).write_bytes(coff_fixture(b"different"))
    with pytest.raises(ValueError):
        if phase == "identities": native.identities(context, graph["nodes"])
        else: native.resolve(context, [{"kind": "compiled", "unit": "src/a.cpp", "symbol": "_f"}], objects, mapping)


def test_object_damage_after_lookup_is_caught_by_final_population_guard(tmp_path, monkeypatch):
    context, objects, mapping, graph = fixture(tmp_path)
    identity, = native.identities(context, graph["nodes"]).values()
    original = objects.resolve
    def damage(owner):
        result = original(owner)
        Path(result["identity"]["path"]).write_bytes(b"changed after independent resolution")
        return result
    monkeypatch.setattr(objects, "resolve", damage)
    with pytest.raises(ValueError, match="changed"):
        native.resolve(context, [identity], objects, mapping)


@pytest.mark.parametrize("namespace", ["build", "project", "external"])
def test_dependencies_relocate_only_inside_their_exact_namespace(tmp_path, namespace):
    contexts = [Context(tmp_path / name, ["src/a.cpp"]) for name in ("before", "after")]
    external = tmp_path / "external"
    external.mkdir()
    entries = []
    for context in contexts:
        compiled(context)
        directory = context.directory if namespace == "build" else context.root if namespace == "project" else external
        objects = physical(directory)
        image = install_image(context, image_thunk())
        mapping = thunk_mapping()
        graph = observe_code_graph(objects, mapping, image, [{"owner": OWNER, "symbol": "_Paint@4"}])
        assert graph["result"] == "pass"
        identity, = native.identities(context, graph["nodes"]).values()
        entries.append((objects, mapping, identity))
    assert entries[0][2] == entries[1][2]
    assert entries[0][2]["input"] == {"kind": namespace,
        "path": str(external / "runtime.lib") if namespace == "external" else "runtime.lib"}
    assert native.resolve(contexts[1], [entries[0][2]], *entries[1][:2]) == ({"owner": OWNER, "symbol": "_Paint@4"},)
    damaged = deepcopy(entries[0][2])
    damaged["object_sha256"] = "a" * 64
    with pytest.raises(ValueError, match="contents differ"):
        native.resolve(contexts[1], [damaged], *entries[1][:2])
    if namespace != "external":
        # The same absolute file cannot be reclassified as an external dependency.
        damaged = deepcopy(entries[1][2])
        physical_root = contexts[1].directory if namespace == "build" else contexts[1].root
        damaged["input"] = {"kind": "external", "path": str(physical_root / "runtime.lib")}
        with pytest.raises(ValueError, match="namespace is not canonical"):
            native.resolve(contexts[1], [damaged], *entries[1][:2])


def test_identities_archive_parse_is_shared_but_final_file_guard_is_not_skipped(tmp_path, monkeypatch):
    context = Context(tmp_path, ["src/a.cpp"])
    compiled(context)
    values = fold_fixture(tmp_path)
    graph = fold_graph(values)
    original = native.CoffArchive
    calls = []
    def parse(raw):
        calls.append(True)
        return original(raw)
    monkeypatch.setattr(native, "CoffArchive", parse)
    member = graph["nodes"][1]
    nodes = [{**member, "id": index} for index in range(5)]
    assert len(native.identities(context, nodes)) == 5 and len(calls) == 1
    def mutate():
        yield nodes[0]
        (tmp_path / "runtime.lib").write_bytes(b"modified after initial read")
        yield nodes[1]
    with pytest.raises(ValueError, match="changed"):
        native.identities(context, mutate())


def test_current_pe_and_folding_must_share_same_physical_epoch(tmp_path):
    context = Context(tmp_path, ["src/a.cpp"])
    compiled(context)
    values = fold_fixture(tmp_path)
    graph = fold_graph(values)
    install_image(context, values[2].data)
    identities = native.identities(context, graph["nodes"])
    folding = bind_folding(*values)
    changed = bytearray(values[2].data)
    changed[600] ^= 1
    install_image(context, bytes(changed))
    with pytest.raises(ValueError, match="another current build"):
        native.resolve(context, identities.values(), values[0], values[1], folding)
