"""Physical PE/COFF observations through native startup residual consumption.

Compilation receipts describe controlled synthetic outputs; no compiler or
provider is run. The linked observer, physical input selection, PE comparison,
live capability producer and residual consumer are not mocked.
"""

from copy import deepcopy
import json
import struct

import pytest

from match_recovery.build_graph import ApplicationGraph
from match_recovery.coff_preservation import compare_outside_functions
from match_recovery.linked_initializers import observe_build_initializers
from match_recovery.model import digest
from match_recovery.pe_coff import Coff
from match_recovery.preservation_objects import application_objects, ObjectTrial
from match_recovery.source_replica import snapshot_digest
from match_recovery.startup_coverage import consume_startup
from test_build_command_evidence import descriptor, STAGES
from test_build_graph_commands import Context as CommandContext, stamp
from test_linked_initializers import build, update_artifact
from test_startup_coverage import renumber


UNIT = "src/unit0.cpp"
FLAGS = ("/c", "/O2", "/Gy", "/GX")


def context(root, code=b"\xc3", *, codes=None, reference=None, references=None, owners=None, damage=None):
    """Real synthetic link artifacts plus native staged command receipts.

    ``codes`` can create several source owners for global ordering controls;
    returned sources use src/unit0.cpp, src/unit1.cpp, ... and dependency.cpp.
    """
    codes = (code,) if codes is None else tuple(codes)
    references = (reference,) * len(codes) if references is None else references
    linked = build(root, codes, references=references, owners=owners)
    if damage is not None:
        damage(linked)
    names = [*(f"src/unit{index}.cpp" for index in range(len(codes))), "src/dependency.cpp"]
    commands = CommandContext(root, names)
    proof = deepcopy(commands.proof)
    binding = proof["command_bindings"]
    for row, bound, output in zip(proof["compilations"], binding["compilations"], linked.objects, strict=True):
        arguments = [row["source"], "/Fo" + str(output), "-O2", "-DKEEP"]
        row["output"] = str(output)
        row["compilation"]["arguments"] = arguments
        row["compilation"]["output"].update(path=str(output), after=stamp(output.read_bytes()))
        bound.update(output=str(output), arguments=arguments,
                     description=descriptor(arguments, STAGES["compile"], [(0, 0, "source"), (1, 3, "output_file")]))
    arguments = [str(path) for path in linked.objects]
    binding["link"].update(arguments=arguments, cwd=str(linked.directory),
                           description=descriptor(arguments, STAGES["link"],
                                                  [(index, 0, "input_file") for index in range(len(arguments))]))
    linked.report["compile_link"] = proof
    source_files = {name: (root / name).read_bytes() for name in names}
    report_identity = digest(linked.report)
    pins = {path: stamp(path.read_bytes()) for path in [*linked.objects, root / "compiler", root / "linker",
                                                       *(root / name for name in names)]}
    original_guard = linked.check_current

    def fresh():
        original_guard()
        commands.check_current()
        if (digest(linked.report) != report_identity or any(not path.is_file() or stamp(path.read_bytes()) != expected
                                                          for path, expected in pins.items())):
            raise ValueError("synthetic compile/input epoch changed")

    linked.check_current = fresh
    return linked, source_files


def graph_at(tmp_path, *, code=b"\x90\xc3", reference=None, damage=None):
    root = tmp_path / "coordinator"
    root.mkdir()
    before, old_sources = context(tmp_path / "baseline", b"\xc3")
    after, new_sources = context(tmp_path / "current", code, reference=reference, damage=damage)
    changes = [{"entity_id": "entity-" + "a" * 64, "unit": UNIT, "version": version,
                "symbol": "_$E1", "offset": 0, "evidence": ["bounded synthetic source ownership"]}
               for version in ("baseline", "current")]
    permission = {"migration_id": "startup-owner", "author": "coordinator", "allowed": [],
                  "evidence": ["independent bounded startup decision"], "generated_changes": changes}
    authorization = {"task": {"task_id": "owner"}, "decisions": [], "permission": permission}

    def fresh():
        try:
            before.check_current()
        finally:
            after.check_current()

    graph = ApplicationGraph(root, "owner", "1" * 40, ("src",), before, after,
                             {"baseline": old_sources, "current": new_sources}, {"result": "unknown"},
                             authorization, fresh)
    return graph, changes


def cover(run, graph, changes, version, *, renumbered=False):
    context = graph.baseline if version == "baseline" else graph.current
    raw = context.objects[0].read_bytes()
    raw = renumber(raw) if renumbered else raw
    trial = ObjectTrial(raw, graph.root, graph.task_name, graph.baseline_revision, digest(graph.authorization),
                        snapshot_digest(graph.source_versions[version]), UNIT, version, FLAGS, "b" * 64)
    obj = Coff(raw)
    capability = run.cover_startup(trial, obj, unit=UNIT, version=version, flags=FLAGS,
                                    input_fingerprint="b" * 64,
                                    changes=[change for change in changes if change["version"] == version])
    return obj, capability


@pytest.mark.parametrize("renumbered", [False, True])
def test_physical_linked_observer_grants_only_live_exact_native_residual_coverage(tmp_path, renumbered):
    graph, changes = graph_at(tmp_path)
    for context in (graph.baseline, graph.current):
        observed = observe_build_initializers(context)
        assert observed["result"] == observed["code_dependencies"]["result"] == "pass", observed
        entry, = observed["entries"]
        assert entry["input"] == {"path": str(context.objects[0])}
        assert entry["object_sha256"] == stamp(context.objects[0].read_bytes())["sha256"]
    with application_objects(graph) as run:
        old, old_coverage = cover(run, graph, changes, "baseline")
        new, new_coverage = cover(run, graph, changes, "current", renumbered=renumbered)
        expected = ((1, 0, 2),) if renumbered else ((2, 0, 0),)
        assert consume_startup(new_coverage, new) == expected
        assert compare_outside_functions(old, new, {}, {})["result"] == "fail"
        result = compare_outside_functions(old, new, {}, {}, baseline_startup=old_coverage, current_startup=new_coverage)
        assert result["result"] == "pass", result
    for obj, capability in ((old, old_coverage), (new, new_coverage)):
        with pytest.raises(ValueError, match="expired"):
            consume_startup(capability, obj)
    assert graph.report["result"] == "unknown"  # Not application/startup effect acceptance.


def inconsistent(context, kind):
    if kind == "input-code":
        path = context.objects[0]
        raw = bytearray(path.read_bytes())
        raw[100] = 0xC3 if raw[100] != 0xC3 else 0x90
        path.write_bytes(raw)
        receipt = json.loads((context.directory / "link.json").read_text())
        row = next(row for row in receipt["files"] if row["path"] == str(path))
        row["before"] = row["after"] = stamp(raw)
        update_artifact(context, "link.json", json.dumps(receipt).encode())
    elif kind == "missing-selected-input":
        raw = (context.directory / "link.log").read_bytes().replace(b"unit0.obj\n", b"")
        update_artifact(context, "link.log", raw)
        receipt = json.loads((context.directory / "link.json").read_text())
        receipt["link_log_sha256"] = stamp(raw)["sha256"]
        update_artifact(context, "link.json", json.dumps(receipt).encode())
    else:
        raw = bytearray((context.directory / "app.exe").read_bytes())
        if kind == "linked-code":
            raw[512] ^= 1
        elif kind == "pointer":
            struct.pack_into("<I", raw, 1028, 0x401010)
        elif kind == "loader-fixup":
            struct.pack_into("<H", raw, 512 + 0x180 + 8, 0)
        else:
            raw[512 + 0x1E0] = 0x90
        update_artifact(context, "app.exe", bytes(raw))


@pytest.mark.parametrize("kind", ["input-code", "linked-code", "pointer", "loader-fixup", "missing-selected-input",
                                  "linked-helper"])
def test_coherent_receipts_cannot_hide_actual_input_linked_inconsistency(tmp_path, kind):
    helper = kind == "linked-helper"
    graph, changes = graph_at(tmp_path, code=b"\xe8\0\0\0\0\xc3" if helper else b"\x90\xc3",
                             reference="_dep" if helper else None,
                             damage=lambda context: inconsistent(context, kind))
    observed = observe_build_initializers(graph.current)
    if helper:
        assert observed["result"] == "pass"
        assert observed["code_dependencies"]["result"] == "fail"
    else:
        assert observed["result"] in ("fail", "unknown"), observed
    with application_objects(graph) as run:
        with pytest.raises(ValueError, match="consistent fresh linked initializer evidence"):
            cover(run, graph, changes, "current")


@pytest.mark.parametrize("artifact", ["app.exe", "app.map", "link.log", "link.json", "unit0.obj"])
def test_actual_artifact_change_invalidates_existing_capability_and_final_guard(tmp_path, artifact):
    graph, changes = graph_at(tmp_path)
    with pytest.raises(ValueError, match="epoch changed|artifact changed"):
        with application_objects(graph) as run:
            obj, coverage = cover(run, graph, changes, "current")
            path = graph.current.directory / artifact
            path.write_bytes(path.read_bytes() + b"changed-after-observation")
            with pytest.raises(ValueError, match="epoch changed|artifact changed"):
                consume_startup(coverage, obj)
    with pytest.raises(ValueError, match="expired"):
        consume_startup(coverage, obj)


def test_linked_helper_reference_remains_a_strict_residual_after_real_coverage(tmp_path):
    graph, changes = graph_at(tmp_path, code=b"\xe8\0\0\0\0\xc3", reference="_dep")
    observed = observe_build_initializers(graph.current)
    assert observed["result"] == observed["code_dependencies"]["result"] == "pass", observed
    with application_objects(graph) as run:
        old, old_coverage = cover(run, graph, changes, "baseline")
        new, new_coverage = cover(run, graph, changes, "current")
        report = compare_outside_functions(old, new, {}, {}, baseline_startup=old_coverage,
                                           current_startup=new_coverage)
        assert report["result"] == "fail" and report["symbols_changed"]
        assert any(row["symbol"] == "_dep" for row in report["symbol_changes"])


def test_multi_owner_fixture_binds_each_physical_entry_to_its_native_source(tmp_path):
    from match_recovery.build_graph_commands import _context

    linked, sources = context(tmp_path / "multiple", codes=(b"\xc3", b"\xc3"),
                              owners=("left.obj", "right.obj"))
    _, _, rows, _ = _context(linked)
    report = observe_build_initializers(linked)
    assert report["result"] == report["code_dependencies"]["result"] == "pass", report
    assert len(report["sequence"]) == 2
    assert set(rows) == set(sources) == {"src/unit0.cpp", "src/unit1.cpp", "src/dependency.cpp"}
    expected = {str(rows[f"src/unit{index}.cpp"]["output"]): owner
                for index, owner in enumerate(("left.obj", "right.obj"))}
    assert {entry["input"]["path"]: entry["function"]["owner"] for entry in report["entries"]} == expected
