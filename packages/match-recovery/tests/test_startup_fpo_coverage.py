"""Physical native startup FPO capability and strict residual controls."""

from dataclasses import replace
import struct

import pytest

from match_recovery.build_graph import ApplicationGraph
from match_recovery.coff_preservation import compare_outside_functions
from match_recovery.model import digest
from match_recovery.pe_coff import Coff
from match_recovery.preservation_objects import application_objects, ObjectTrial
from match_recovery.source_replica import snapshot_digest
from match_recovery.startup_coverage import consume_startup, consume_startup_fpo
from match_recovery.startup_projection import project_startup
from test_build_graph_commands import stamp
from test_linked_initializers import update_artifact
from test_msf import small_msf
from test_pdb_fixups import parts, SIGNATURE
from test_projected_initializer_order import repeated_root
from test_slot_local_initializer_order import physical_context, associated_fpo


UNIT = "src/unit0.cpp"
FLAGS = ("/c", "/O2", "/Gy", "/GX")


def add_native_fpo(context, *, damage=None):
    structural = {"frame", "addend", "size", "truncated", "relocation-kind", "relocation-target",
                  "additional-reference", "incoming-root", "incoming-metadata", "association"}
    if damage == "shared-slot":
        repeated_root(context)
    associated_fpo(context, damage=damage if damage in structural else None)
    raw = bytearray((context.directory / "app.exe").read_bytes())
    payload = struct.pack("<4sIII", b"NB10", 0, SIGNATURE, 1) + b"app.pdb\0"
    pointer = len(raw)
    raw.extend(payload)
    struct.pack_into("<I", raw, 0x98 + 60, 0x200)
    struct.pack_into("<II", raw, 0x98 + 96 + 6 * 8, 0x1100, 28)
    struct.pack_into("<IIHHIIII", raw, 0x300, 0, 0, 0, 0, 2, len(payload), 0, pointer)
    update_artifact(context, "app.exe", raw)
    if damage == "no-pdb":
        return
    _, streams = parts(())
    streams[5] = bytes(raw[0x178:0x1C8])
    dbi = bytearray(streams[3])
    struct.pack_into("<H", dbi, 64, 6)
    streams[3] = bytes(dbi)
    frame = struct.pack("<IIII", 0x1000, 16, 0, 0)
    if damage == "missing": frame = b""
    elif damage == "duplicate": frame *= 2
    elif damage == "mismatch": frame = struct.pack("<IIII", 0x1000, 16, 1, 0)
    elif damage == "wrong-root": frame = struct.pack("<IIII", 0x1020, 16, 0, 0)
    streams.append(frame)
    pdb = small_msf(streams)
    (context.directory / "app.pdb").write_bytes(pdb)
    context.report["outputs"].append({"path": "app.pdb", "format": "msvc-pdb", "result": "pass", **stamp(pdb)})
    context.pin_outputs()
    if damage == "public-alias":
        mapping = (context.directory / "app.map").read_text()
        alias = " 0001:00000000 _escaped 00401000 f dependency.obj\n"
        update_artifact(context, "app.map", mapping.replace(" entry point", alias + " entry point").encode())


def graph_at(monkeypatch, tmp_path, *, damage=None, source_files=None):
    before = physical_context(monkeypatch, tmp_path / "old", damage=add_native_fpo)
    after = physical_context(monkeypatch, tmp_path / "new", codes=(b"\x90\xc3",),
        damage=lambda context: add_native_fpo(context, damage=damage), source_files=source_files)
    root = tmp_path / "coordinator"
    root.mkdir()
    changes = [{"entity_id": "entity-" + "a" * 64, "unit": UNIT, "version": version,
                "symbol": "_$E1", "offset": 0, "evidence": ["independently intended startup root change"]}
               for version in ("baseline", "current")]
    authorization = {"task": {"task_id": "owner"}, "decisions": [], "permission": {
        "migration_id": "startup-owner", "author": "coordinator", "allowed": [],
        "evidence": ["bounded native control"], "generated_changes": changes}}
    sources = {version: {path.relative_to(context.root).as_posix(): path.read_bytes()
                        for path in (context.root / "src").rglob("*") if path.is_file()}
               for version, context in (("baseline", before), ("current", after))}

    def fresh():
        try: before.check_current()
        finally: after.check_current()

    return ApplicationGraph(root, "owner", "1" * 40, ("src",), before, after,
                            sources, {"result": "unknown"}, authorization, fresh), changes


def renumber(raw):
    original = Coff(raw)
    data = bytearray(raw)
    assert len(original.sections) == 3
    permutation = {1: 3, 2: 1, 3: 2}
    headers = [bytes(data[20 + index * 40:60 + index * 40]) for index in range(3)]
    data[20:140] = headers[1] + headers[2] + headers[0]
    at, count = struct.unpack_from("<II", data, 8)
    for index in range(3):
        pointer, = struct.unpack_from("<I", data, 20 + index * 40 + 24)
        amount, = struct.unpack_from("<H", data, 20 + index * 40 + 32)
        for offset in range(amount):
            target, = struct.unpack_from("<I", data, pointer + offset * 10 + 4)
            struct.pack_into("<I", data, pointer + offset * 10 + 4, target + 2)
    for index, symbol in original.symbols.items():
        if symbol.section > 0:
            struct.pack_into("<h", data, at + index * 18 + 12, permutation[symbol.section])
        if original.auxiliary.get(index) and symbol.name == ".debug$F":
            struct.pack_into("<H", data, at + (index + 1) * 18 + 12, permutation[1])
    struct.pack_into("<I", data, 12, count + 2)
    file = b".file\0\0\0" + struct.pack("<IhHBB", 0, -2, 0, 103, 1) + b"replica.cpp\0".ljust(18, b"\0")
    return bytes(data[:at]) + file + bytes(data[at:])


def cover(run, graph, changes, version, *, moved=False):
    context = graph.baseline if version == "baseline" else graph.current
    raw = context.objects[0].read_bytes()
    if moved:
        raw = renumber(raw)
    trial = ObjectTrial(raw, graph.root, graph.task_name, graph.baseline_revision, digest(graph.authorization),
                        snapshot_digest(graph.source_versions[version]), UNIT, version, FLAGS, "b" * 64)
    obj = Coff(raw)
    capability = run.cover_startup(trial, obj, unit=UNIT, version=version, flags=FLAGS,
        input_fingerprint="b" * 64, changes=[row for row in changes if row["version"] == version])
    return obj, capability


@pytest.mark.parametrize("moved", [False, True])
def test_exact_native_frame_covers_only_its_exclusive_root_and_child(monkeypatch, tmp_path, moved):
    graph, changes = graph_at(monkeypatch, tmp_path)
    with application_objects(graph) as run:
        old, old_cap = cover(run, graph, changes, "baseline")
        new, new_cap = cover(run, graph, changes, "current", moved=moved)
        assert compare_outside_functions(old, new, {}, {})["result"] == "fail"
        result = compare_outside_functions(old, new, {}, {}, baseline_startup=old_cap, current_startup=new_cap)
        assert result["result"] == "pass", result
        slots = consume_startup(new_cap, new)
        frames = consume_startup_fpo(new_cap, new)
        assert len(frames) == 1
        child, index, metadata = frames[0]
        assert child == (2 if moved else 3)
        assert new.symbols[index].name == "_$E1"
        assert dict(metadata) == {"format": "fpo_data", "code_size": 16, "data": struct.pack("<III", 16, 0, 0).hex()}
        _, unbound = project_startup(new, slots)
        assert unbound["retained_root_sections"] == [new.function("_$E1").section]
        residual, coverage = project_startup(new, slots, fpo=frames)
        assert not residual["sections"] and not residual["associations"]
        assert coverage["covered_fpo"][0]["runtime_effects"] == "unknown"
        assert graph.report["result"] == "unknown"
    with pytest.raises(ValueError, match="expired"):
        consume_startup_fpo(new_cap, new)


@pytest.mark.parametrize("damage", ["no-pdb", "missing", "duplicate", "mismatch", "wrong-root",
    "frame", "addend", "size", "truncated", "relocation-kind", "relocation-target", "additional-reference",
    "incoming-root", "incoming-metadata", "association", "shared-slot", "public-alias"])
def test_unverified_or_escaping_fpo_cannot_be_covered(monkeypatch, tmp_path, damage):
    graph, changes = graph_at(monkeypatch, tmp_path, damage=damage)
    with application_objects(graph) as run:
        with pytest.raises(ValueError):
            cover(run, graph, changes, "current")


def test_source_owned_metadata_is_not_anonymous_covered_bookkeeping(monkeypatch, tmp_path):
    graph, changes = graph_at(monkeypatch, tmp_path, source_files={"src/owner.h":
        b"// RECOVERY_OBJECT: v1 0x00403000 .debug$F\nint owned_frame;\n"})
    with application_objects(graph) as run:
        with pytest.raises(ValueError):
            cover(run, graph, changes, "current")


def test_protected_child_binding_is_checked_again_by_residual_consumer(monkeypatch, tmp_path):
    graph, changes = graph_at(monkeypatch, tmp_path)
    with application_objects(graph) as run:
        old, old_cap = cover(run, graph, changes, "baseline")
        new, new_cap = cover(run, graph, changes, "current")
        with pytest.raises(ValueError, match="protected outside source binding"):
            compare_outside_functions(old, new, {}, {}, baseline_startup=old_cap, current_startup=new_cap,
                                      current_outside={".debug$F"})


@pytest.mark.parametrize("damage", ["object", "pdb", "source", "expired"])
def test_fpo_capability_retains_original_object_artifact_and_epoch_guards(monkeypatch, tmp_path, damage):
    graph, changes = graph_at(monkeypatch, tmp_path)
    with pytest.raises(ValueError):
        with application_objects(graph) as run:
            obj, capability = cover(run, graph, changes, "current")
            if damage == "object":
                obj.sections = tuple(replace(section, data=b"changed") if section.name == ".debug$F" else section
                                     for section in obj.sections)
            elif damage == "pdb":
                (graph.current.directory / "app.pdb").write_bytes(b"changed")
            elif damage == "source":
                (graph.current.root / UNIT).write_bytes(b"changed")
            else:
                graph.current.alive = False
            consume_startup_fpo(capability, obj)


def test_child_selection_cannot_be_reused_for_another_root_or_altered_frame(monkeypatch, tmp_path):
    graph, changes = graph_at(monkeypatch, tmp_path)
    with application_objects(graph) as run:
        obj, cap = cover(run, graph, changes, "current")
        slots, frames = consume_startup(cap, obj), consume_startup_fpo(cap, obj)
        child, index, metadata = frames[0]
        for bad in (((child, index + 1, metadata),), ((child, index, ()),), frames * 2):
            with pytest.raises(ValueError):
                project_startup(obj, slots, fpo=bad)
        with pytest.raises(ValueError):
            project_startup(obj, slots, fpo=frames, excluded={child})
