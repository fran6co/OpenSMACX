"""Shared compiler failure controls and project-owned VC6 option semantics."""

import json
from pathlib import Path
import shutil
import subprocess
import sys

import pytest

from match_recovery.compilation_database import entries
from match_recovery.processes import run_bounded
from match_recovery.source import read_file
from match_recovery.source.asm import compiled_asm, CompileFailed
from recovery.environment import compile as environment
from recovery.environment.recipe import invocation, build_command, MissingRecipe, flag_dependent


def recipe(tmp_path, args):
    entry, = entries(json.dumps([{"file": "unit.cpp", "directory": str(tmp_path),
                                  "arguments": args}]).encode(), tmp_path)
    return invocation(entry)


def test_recipe_preserves_argument_boundaries_and_resolves_both_include_forms(tmp_path):
    args = ["./vc6-cl", "/I", "headers with spaces", "-I../shared", "/FI", "config.h",
            '-DMESSAGE="hello world"', "/c", "/O2", "/Oy-", "/Z7", "/Foout.obj", "unit.cpp"]
    assert recipe(tmp_path, args) == [str(tmp_path / "vc6-cl"), "/nologo",
        "/I" + str(tmp_path / "headers with spaces"), "/I" + str(tmp_path.parent / "shared"),
        "/FI" + str(tmp_path / "config.h"), '/DMESSAGE="hello world"']


def test_pch_filter_does_not_delete_an_unrelated_define(tmp_path):
    assert recipe(tmp_path, ["cl", "/FI", "cmake_pch.hxx", "/Yucmake_pch.hxx", "/Fp", "pch.bin",
                             "/DRESOURCE=cmake_pch", "unit.cpp"]) == ["cl", "/nologo", "/DRESOURCE=cmake_pch"]


@pytest.mark.parametrize("prefix", ["/", "-"])
@pytest.mark.parametrize("runtime", ["MD", "MDd", "MT", "MTd", "ML", "MLd"])
def test_recipe_preserves_runtime_predefines_and_link_policy(tmp_path, prefix, runtime):
    assert recipe(tmp_path, ["cl", prefix + runtime, "/O2", "/Z7", "unit.cpp"]) == [
        "cl", "/nologo", "/" + runtime]


def test_runtime_switch_order_is_not_reinterpreted_by_recipe_extraction(tmp_path):
    assert recipe(tmp_path, ["cl", "-MD", "/MT", "unit.cpp"]) == ["cl", "/nologo", "/MD", "/MT"]


def test_different_runtime_recipes_are_not_mistaken_for_duplicates(tmp_path):
    database = tmp_path / "compilation.json"
    rows = [{"file": "unit.cpp", "directory": str(tmp_path), "arguments": ["cl", flag, "unit.cpp"]}
            for flag in ("/MD", "-MT")]
    database.write_text(json.dumps(rows))
    with pytest.raises(ValueError, match="conflicting compilation recipes"):
        build_command(database, tmp_path / "unit.cpp")


def test_prepared_project_and_matching_compiler_use_static_multithreaded_runtime(tmp_path):
    from match_recovery.compilation_database import read_database
    from match_recovery.pe_coff import Coff
    from recovery.environment.wine import VC6_ROOT
    root = Path(__file__).resolve().parents[2]
    database = root / "build/compile_commands.json"
    if not database.is_file() or not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("prepared project and VC6/Wine are required")
    commands = [invocation(entry) for entry in read_database(database)]
    assert commands
    for command in commands:
        assert [word for word in command if word in ("/MD", "/MDd", "/MT", "/MTd", "/ML", "/MLd")] == ["/MT"]
    source = tmp_path / "runtime.cpp"
    source.write_text("#if !defined(_MT) || defined(_DLL) || defined(_DEBUG)\n"
                      "#error matching runtime policy is not static multithreaded\n#endif\n"
                      "int runtime_mode_control() { return 0; }\n")
    command = build_command(database, root / "src/alphanet.cpp")
    # This fixture owns a separate source tree, while borrowing the product's
    # parse/runtime policy. Give the explicit compiler identity its real root.
    command = ["--repro-root=" + str(tmp_path) if word.startswith("--repro-root=") else word
               for word in command]
    obj = Coff(environment.compile_object(source, tuple(command), ("/c", "/O2", "/Gy")))
    obj.function("?runtime_mode_control@@YAHXZ")
    # Check the actual compiler's predefines, not just spellings in a database.
    for wrong in ("/MD", "/ML", "/MTd"):
        changed = tuple(wrong if word == "/MT" else word for word in command)
        with pytest.raises(ValueError, match="compilation failed"):
            environment.compile_object(source, changed, ("/c", "/O2", "/Gy"))


def test_release_link_preserves_the_selected_runtime_code_and_relocations():
    """A bounded linked-runtime control, not full original-game acceptance."""
    from match_recovery.component_commands import execute
    from match_recovery.component_source import parse_component
    from match_recovery.pe_coff import PeImage
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("VC6/Wine are required for the actual project link")
    root = Path(__file__).resolve().parents[2]
    manifest = parse_component((root / "src/vendor/vc6-runtime.recovery.h").read_text())
    status, report = execute(root, {"action": "check-link", "component": "vc6-runtime",
                                    "archive": VC6_ROOT / "LIB/LIBCMT.LIB", "library": "LIBCMT"})
    assert status == 0 and report["result"] == "pass", report
    assert len(report["functions"]) == len(manifest.bindings)
    assert all(row["final_input_observation"] == {"matches": 1, "result": "pass"}
               for row in report["functions"])
    assert report["link_inputs"]["input_count"] > len(manifest.bindings)
    assert report["link_inputs"]["input_identity"] == "unknown"
    assert report["link_inputs"]["export_generations"] == 1
    assert report["archive_file_identity"] == report["link_files"]["result"] == "pass"
    assert report["link_files"]["input_coverage"] == "unknown"
    assert report["link_input_coverage"]["result"] == "pass", report["link_input_coverage"]
    assert len(report["link_input_coverage"]["generated_exports"]) == 1
    assert report["link_input_coverage"]["allocation_verification"] == "unknown"
    assert report["common_allocations"]["result"] == "pass", report["common_allocations"]
    assert report["common_allocations"]["counts"] == {"pass": 7}
    assert all(row["size"] == 4 for row in report["common_allocations"]["allocations"])
    assert report["storage_verification"] == report["original_verification"] == "unknown"
    build = report["fresh_build"]
    directory = root / build["directory"]
    output, = (item for item in build["outputs"] if item["format"] == "pe32-i386")
    image = PeImage((directory / output["path"]).read_bytes(), output["sha256"])
    assert not any(item.module.lower() in ("msvcrt.dll", "msvcrtd.dll") for item in image.imports())
    # Project policy only; comparison and map parsing belong to the package.
    assert image.relocations
    # Each application C++ compile must describe the exact file handed to the
    # linker. This is file correspondence, not proof of retained sections or
    # acceptance of an application build-graph change.
    commands = json.loads((directory / "compile_commands.json").read_bytes())
    expected = [Path(item["file"]) for item in commands if Path(item["file"]).suffix == ".cpp"]
    proof = build["compile_link"]
    assert proof["result"] == "pass", proof
    assert len(proof["compilations"]) == len(expected)
    assert proof["sources"] == sorted(map(str, expected))
    assert proof["linked_selection"] == proof["build_graph_verification"] == "unknown"
    bindings = proof["command_bindings"]
    assert bindings["result"] == "pass" and len(bindings["compilations"]) == len(expected)
    assert bindings["link"]["description"]["stage"] == "link-wrapper"
    assert all(row["description"]["stage"] == "compiler-effective" for row in bindings["compilations"])


@pytest.fixture(scope="module")
def private_application_control():
    """Keep one guarded baseline live for three serial counterfactual builds."""
    from match_recovery.application_context import application_build
    from match_recovery.build_check import Build
    from match_recovery.driver import working_state
    from match_recovery.source_replica import replica_snapshot
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("VC6/Wine are required for the actual project link")
    root = Path(__file__).resolve().parents[2]
    before = working_state(root)
    sources = {name: raw for name, raw in replica_snapshot(root, ("src",)).items() if raw is not None}
    settings = Build.read(root)
    graphs = {name: (root / name).read_bytes() for name in settings.versioned_inputs}
    with application_build(root, ("src",), sources, versioned_files=graphs) as application:
        proof = application.report["compile_link"]
        assert proof["result"] == proof["command_bindings"]["result"] == "pass"
        assert proof["sources"] and proof["build_graph_verification"] == "unknown"
        assert all(Path(name).is_relative_to(application.root) for name in proof["sources"])
        assert (application.directory / "OpenSMACX.exe").is_file()
        assert (application.root / "CMakeLists.txt").read_bytes() == graphs["CMakeLists.txt"]
        application.check_current()
        yield root, sources, graphs, application
    assert not application.root.exists() and working_state(root) == before


@pytest.mark.parametrize("added,initializes", [(False, False), (True, False), (True, True)],
                         ids=["same-graph", "added-tu", "added-initializer"])
def test_private_application_context_builds_with_native_command_evidence(added, initializes, private_application_control):
    """Exercise actual isolated application generation, not migration acceptance."""
    from match_recovery.application_context import application_build
    from match_recovery.build_graph_commands import compare_build_commands
    from match_recovery.linked_initializers import compare_build_initializers
    root, sources, graphs, application = private_application_control
    proof = application.report["compile_link"]
    extra = "src/recovery_graph_control.cpp"
    assert extra not in sources
    candidate_sources, candidate_graphs = dict(sources), dict(graphs)
    if added:
        # Fixture-only C++ and graph bytes: neither the product checkout nor
        # any retained recovery candidate is edited to exercise this path.
        candidate_sources[extra] = b"int recovery_graph_control() { return 17; }\n"
        if initializes:
            candidate_sources[extra] = (b"volatile int recovery_graph_seed = 17;\n"
                b"int recovery_graph_control() { return recovery_graph_seed; }\n"
                b"int recovery_graph_value = recovery_graph_control();\n")
        candidate_graphs["CMakeLists.txt"] += b"\ntarget_sources(OpenSMACX PRIVATE " + extra.encode() + b")\n"
    application.check_current()
    with application_build(root, ("src",), candidate_sources, versioned_files=candidate_graphs) as repeated:
        if added:
            with pytest.raises(ValueError, match="membership exceeds editable task scope"):
                compare_build_commands(application, repeated, editable=())
        comparison = compare_build_commands(application, repeated, editable=(extra,) if added else ())
        assert comparison["result"] == "pass", comparison
        assert comparison["sources"]["added"] == ([extra] if added else [])
        assert comparison["sources"]["removed"] == []
        assert len(comparison["compilations"]) == len(proof["sources"])
        assert comparison["removed_operands"]["baseline"] == []
        assert len(comparison["removed_operands"]["current"]) == int(added)
        assert comparison["linked_effects"] == "unknown"
        initializers = compare_build_initializers(application, repeated)
        assert initializers["baseline"]["result"] == initializers["current"]["result"] == "pass", initializers
        ordered = initializers["ordered_code_correspondence"]
        assert ordered == ("fail" if added else "pass"), initializers
        old_slots, new_slots = (initializers[version]["sequence"] for version in ("baseline", "current"))
        # vc6_compat.h forces <string>: even the plain TU emits a library
        # registration. The explicit dynamic initializer adds a second one.
        assert old_slots and len(new_slots) == len(old_slots) + int(added) + int(initializes)
        dependencies = initializers["code_dependencies"]
        assert dependencies["check"] == "application_initializer_code_dependencies"
        assert dependencies["remaining_linked_effects"] == "unknown"
        for version, slots in (("baseline", old_slots), ("current", new_slots)):
            observed = dependencies[version]
            assert observed == initializers[version]["code_dependencies"]
            assert observed["check"] == "linked_relocation_code_graph"
            assert observed["nodes"] and isinstance(observed["gaps"], list)
            assert len(observed["roots"]) == len(slots)
            assert observed["remaining_linked_effects"] == "unknown"
            assert any(node["result"] == "pass" for node in observed["nodes"]), observed
            for node in observed["nodes"]:
                if node["result"] != "pass":
                    continue
                function = node["proof"]
                assert function["result"] == "pass" and function["size"] > 0, node
                assert function["target_contents"] == "unknown"
                assert all(function["code"][key] == "pass"
                           for key in ("result", "boundary", "bytes", "references")), node
            if observed["result"] == "pass":
                assert not observed["gaps"] and observed["graph_fingerprint"], observed
                assert all(node["result"] == "pass" for node in observed["nodes"]), observed
        # Unsupported runtime definitions may leave the reachable closure
        # unknown. A real counterexample in identical builds must not be hidden
        # by the independent, passing ordered initializer-body observation.
        if not added:
            assert dependencies["result"] in ("pass", "unknown"), dependencies
        results = (ordered, dependencies["result"])
        expected = "fail" if "fail" in results else "unknown" if "unknown" in results else "pass"
        assert initializers["result"] == expected, initializers
        assert initializers["remaining_linked_effects"] == "unknown"
        if not added:
            # Reuse these two actual application builds. This fixture supplies
            # explicit diagnostic identity, not a task/admission authorization.
            from dataclasses import replace
            from hashlib import sha256
            import struct
            from match_recovery.build_check import Build
            from match_recovery.build_graph import ApplicationGraph
            from match_recovery.coff_replica import object_signature
            from match_recovery.driver import working_state
            from match_recovery.measurement import Measurement
            from match_recovery.model import digest
            from match_recovery.pe_coff import Coff
            from match_recovery.prepared_context import prepared_compiler, source_roots
            from match_recovery.preservation_objects import ObjectTrial, application_objects
            from match_recovery.source_replica import replica_snapshot, snapshot_digest

            state = working_state(root)
            revision = state["head"].decode().strip()
            task_name = "actual-application-object-diagnostic"
            authorization = {"task": {"task_id": task_name, "base_revision": revision,
                                       "intent": "test-only compiler correspondence, never task acceptance"},
                             "decisions": (), "integration": None}

            def fresh():
                try:
                    application.check_current()
                finally:
                    repeated.check_current()
                assert working_state(root) == state

            graph = ApplicationGraph(root=root, task_name=task_name, baseline_revision=revision,
                                     roots=("src",), baseline=application, current=repeated,
                                     _sources={"baseline": dict(sources), "current": dict(candidate_sources)},
                                     _report={"result": "unknown", "linked_effects": "unknown"},
                                     _authorization=authorization, check_current=fresh)
            settings = Measurement.read(root)
            preparation_roots = source_roots(root, settings)
            preparation_sources = {name: raw for name, raw in replica_snapshot(root, preparation_roots).items()
                                   if raw is not None}
            assert {name: raw for name, raw in preparation_sources.items()
                    if Path(name).is_relative_to("src")} == sources
            preparation = Build.read(root, key="preparation")
            preparation_graphs = {name: (root / name).read_bytes() for name in preparation.versioned_inputs}
            unit = "src/time.cpp"
            application_objects_by_version = {}
            for version, context in (("baseline", application), ("current", repeated)):
                row, = (row for row in context.report["compile_link"]["compilations"]
                        if row["source"] == str(context.root / unit))
                output = Path(row["output"])
                raw = output.read_bytes()
                assert row["compilation"]["output"]["after"] == {"sha256": sha256(raw).hexdigest(), "size": len(raw)}
                application_objects_by_version[version] = (output, raw, object_signature(Coff(raw)))

            with prepared_compiler(root, preparation_roots, preparation_sources, settings,
                                   versioned_files=preparation_graphs) as prepared:
                assert unit in prepared.units
                assert (prepared.root / unit).read_bytes() == sources[unit]
                fingerprint = digest(Measurement.read(prepared.root).snapshot(prepared.root, {"fixture": authorization}))
                with application_objects(graph) as compare_object:
                    # /Od is a separate actual trial, never mislabeled as one
                    # of the configured matching flag sets or used as a waiver.
                    unoptimized = tuple(flag for flag in settings.flag_sets[0] if not flag.startswith("/O")) + ("/Od",)
                    assert "/O2" not in unoptimized
                    for flags in (*settings.flag_sets, unoptimized):
                        prepared.check_current()
                        raw = bytes(prepared(prepared.root / unit, (), flags))
                        obj = Coff(raw)
                        signature = object_signature(obj)
                        for version in ("baseline", "current"):
                            trial = ObjectTrial(raw, root, task_name, revision, digest(authorization),
                                                snapshot_digest(graph.source_versions[version]), unit, version,
                                                tuple(flags), fingerprint)
                            report = compare_object(trial, obj, unit=unit, version=version,
                                                    flags=flags, input_fingerprint=fingerprint)
                            output, application_raw, application_signature = application_objects_by_version[version]
                            assert report["trial_sha256"] == sha256(raw).hexdigest()
                            assert report["application_sha256"] == sha256(application_raw).hexdigest()
                            assert report["application_object"] == str(output)
                            assert report["trial_signature"] == signature
                            assert report["application_signature"] == application_signature
                            assert report["result"] == ("pass" if signature == application_signature else "unknown"), report
                            assert report["flags"] == list(flags) and report["linked_effects"] == "unknown"
                            assert not report["persisted"] and not report["ready_for_collection"]

                        # Damage only an in-memory captured code byte. Neither
                        # source nor linked artifacts change during this epoch.
                        section = next(index for index, row in enumerate(obj.sections) if row.flags & 0x20 and row.data)
                        at, = struct.unpack_from("<I", raw, 20 + obj.optional_header_size + section * 40 + 20)
                        damaged = bytearray(raw)
                        damaged[at] ^= 1
                        assert object_signature(Coff(bytes(damaged))) != signature
                        with pytest.raises(ValueError, match="changed from captured compiler output"):
                            compare_object(replace(trial, raw=bytes(damaged)), obj, unit=unit, version="current",
                                           flags=flags, input_fingerprint=fingerprint)
                        prepared.check_current()
                graph.check_current()
        if added:
            reverse = compare_build_commands(repeated, application, editable=(extra,))
            assert reverse["result"] == "pass", reverse
            assert reverse["sources"]["added"] == [] and reverse["sources"]["removed"] == [extra]
            assert len(reverse["removed_operands"]["baseline"]) == 1
            assert reverse["removed_operands"]["current"] == []
            assert reverse["linked_effects"] == "unknown"
    assert not repeated.root.exists()


@pytest.mark.parametrize("sizes,strong,export", [((4,), False, False), ((8,), False, False),
    ((4, 8), False, False), ((8, 4), False, False), ((4, 4), False, False), ((8,), True, False),
    ((4,), False, True)])
def test_actual_linker_common_requests_and_strong_override(tmp_path, sizes, strong, export):
    """Isolated storage controls: section extent, not neighboring map publics."""
    import hashlib
    from match_recovery.coff_inventory import inventory_object
    from match_recovery.common_allocation import verify_common_allocations
    from match_recovery.msvc_link_map import parse_link_map
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("VC6/Wine are required for common-allocation controls")
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    objects = []
    for index, size in enumerate(sizes):
        source = tmp_path / ("common" + str(index) + ".c")
        source.write_text("char thing[" + str(size) + "];\n" +
                          ("int entry(void) { return thing[0]; }\n" if index == 0 else ""))
        raw = environment.compile_object(source, command, ("/c", "/TC", "/O2", "/Gy", "/MT"))
        declaration, = (row for row in inventory_object(raw)["symbols"] if row["name"] == "_thing")
        assert declaration["declaration"] == "common" and declaration["value"] == size
        obj = source.with_suffix(".obj")
        obj.write_bytes(raw)
        objects.append(obj.name)
    if strong:
        source = tmp_path / "strong.c"
        source.write_text("char thing[4] = {7, 0, 0, 0};\n")
        raw = environment.compile_object(source, command, ("/c", "/TC", "/O2", "/Gy", "/MT"))
        declaration, = (row for row in inventory_object(raw)["symbols"] if row["name"] == "_thing")
        assert declaration["declaration"] == "defined"
        source.with_suffix(".obj").write_bytes(raw)
        objects.append("strong.obj")
    result = run_bounded([sys.executable, str(wrapper), "--tool=link", "/nologo", "/nodefaultlib",
                          "/subsystem:console", "/entry:entry", "/incremental:no", "/opt:noref", "/fixed:no",
                          "/out:common.exe", "/map:common.map", "/implib:common.lib", "/VERBOSE",
                          "--link-diagnostics=" + str(tmp_path / "common.link.log"),
                          *(["/EXPORT:thing,DATA"] if export else []), *objects], cwd=tmp_path, timeout=60)
    assert result.returncode == 0, (result.stdout + result.stderr).decode(errors="replace")
    raw = (tmp_path / "common.exe").read_bytes()
    image = PeImage(raw, hashlib.sha256(raw).hexdigest())
    mapping = parse_link_map((tmp_path / "common.map").read_bytes())
    mapping.validate(image)
    public, = (row for row in mapping.publics if row.symbol == "_thing")
    if strong:
        assert public.owner == "strong.obj"
        assert image.read_initial(Span(public.address, public.address + 4)) == b"\x07\0\0\0"
    else:
        assert public.owner == "<common>"
        region, _, raw_size = image.sections[public.segment - 1]
        assert raw_size == 0 and public.address == region.start
        assert region.end - region.start == max(sizes)
        assert image.read_initial(region) == bytes(max(sizes))
    proof = verify_common_allocations((tmp_path / "common.link.inputs.json").read_bytes(),
                                     (tmp_path / "common.link.log").read_bytes(), raw,
                                     (tmp_path / "common.map").read_bytes(), ("_thing",))
    assert proof["input_coverage"]["result"] == "pass", proof
    assert proof["result"] == ("unknown" if strong or len(set(sizes)) != 1 else "pass"), proof


@pytest.mark.parametrize("args", [["compiler", "@options"], ["compiler", "/I"],
    ["compiler", "/FI"], ["compiler", "/Fo"], ["python", "wrapper.py", "unit.cpp"]])
def test_unsupported_recipe_is_not_silently_changed(tmp_path, args):
    with pytest.raises(ValueError):
        recipe(tmp_path, args)


def test_missing_recipe_is_distinct_from_conflicting_or_malformed_recipe(tmp_path):
    database = tmp_path / "compilation.json"
    rows = [{"file": "unit.cpp", "directory": str(tmp_path), "arguments": ["cl", flag, "unit.cpp"]}
            for flag in ("/DVALUE=1", "/DVALUE=2")]
    database.write_text(json.dumps(rows))
    with pytest.raises(ValueError, match="conflicting") as raised:
        build_command(database, tmp_path / "unit.cpp")
    assert not isinstance(raised.value, MissingRecipe)
    with pytest.raises(MissingRecipe):
        build_command(database, tmp_path / "seed.cpp")
    database.write_text("invalid")
    with pytest.raises(ValueError) as raised:
        build_command(database, tmp_path / "unit.cpp")
    assert not isinstance(raised.value, MissingRecipe)


@pytest.mark.parametrize("code,body,accepted", [(0, b"object", True), (1, b"object", False),
                                               (0, b"", False), (0, None, False), (7, None, False)])
def test_compiler_exit_and_object_are_both_required(tmp_path, monkeypatch, code, body, accepted):
    seen = []
    def run(argv, *, cwd, timeout):
        seen.append(cwd)
        assert timeout == 300 and argv[-2] == "/Founit.obj"
        assert '/DMESSAGE=hello world' in argv
        if body is not None:
            (cwd / "unit.obj").write_bytes(body)
        return subprocess.CompletedProcess(argv, code, b"compiler diagnostic", b"")
    monkeypatch.setattr(environment, "run_bounded", run)
    source = tmp_path / "unit.cpp"
    if accepted:
        assert environment.compile_object(source, ("compiler",), '/c "/DMESSAGE=hello world"') == body
    else:
        with pytest.raises(ValueError):
            environment.compile_object(source, ("compiler",), '/c "/DMESSAGE=hello world"')
    assert seen and not seen[0].exists()


def test_timeout_is_a_comparison_failure_and_temporary_directory_is_removed(tmp_path, monkeypatch):
    seen = []
    def stall(argv, *, cwd, timeout):
        seen.append(cwd)
        raise subprocess.TimeoutExpired(argv, timeout)
    monkeypatch.setattr(environment, "run_bounded", stall)
    source = tmp_path / "unit.cpp"
    source.write_text("// ORIGINAL: 0x00401000 f 0x00401000-0x00401008\nvoid f() {}\n")
    with pytest.raises(CompileFailed, match="exceeded 300 seconds"):
        compiled_asm(read_file(source)[0], ["compiler"], "/c", compiler=environment.compile_object)
    assert seen and not seen[0].exists()


def test_real_failed_process_cannot_supply_an_object_to_the_comparer(tmp_path):
    # A synthetic failing tool actually creates an object before exiting. This
    # reproduced the old subprocess path's mistake without proprietary fixtures.
    script = "from pathlib import Path; Path('unit.obj').write_bytes(b'object'); raise SystemExit(7)"
    with pytest.raises(ValueError, match="compilation failed"):
        environment.compile_object(tmp_path / "unit.cpp", (sys.executable, "-c", script), ())


@pytest.mark.parametrize("code,body,diagnostic", [(7, None, b""), (0, None, b""),
                                                (-6, None, b""), (1, b"object", b"C2248: protected member")])
def test_compiler_failure_preserves_exit_and_object_evidence(tmp_path, monkeypatch, code, body, diagnostic):
    seen = []
    def run(argv, *, cwd, timeout):
        seen.append(cwd)
        if body is not None:
            (cwd / "unit.obj").write_bytes(body)
        return subprocess.CompletedProcess(argv, code, b"", diagnostic)
    monkeypatch.setattr(environment, "run_bounded", run)
    with pytest.raises(ValueError) as raised:
        environment.compile_object(tmp_path / "unit.cpp", ("compiler",), ())
    assert str(raised.value) == (f"VC6 compilation failed: exit {code}; "
                                f"object {'present' if body is not None else 'missing'}; "
                                + (diagnostic.decode() or "no compiler output"))
    assert seen and not seen[0].exists()


def test_vc6_retry_policy_remains_in_the_environment():
    assert not flag_dependent("error C2065: identifier not declared")
    assert flag_dependent("fatal error C1001: internal compiler error")
    assert flag_dependent("fatal error C1060: out of heap space")


def test_unidentified_compilers_cannot_generate_matching_recipes(tmp_path):
    cmake, ninja, failed_compiler = (shutil.which(name) for name in ("cmake", "ninja", "false"))
    if not all((cmake, ninja, failed_compiler)):
        pytest.skip("CMake, Ninja and a failing POSIX command are required")
    root = Path(__file__).resolve().parents[2]
    # No installed compiler or Wine is needed. The forced toolchain used to
    # report success and emit unusable generic recipes even for /bin/false.
    install = tmp_path / "vc6"
    (install / "BIN").mkdir(parents=True)
    (install / "INCLUDE").mkdir()
    build = tmp_path / "build"
    completed = run_bounded([
        cmake, "-S", str(root), "-B", str(build), "-G", "Ninja",
        "-DVC6_ROOT=" + str(install), "-DCMAKE_MAKE_PROGRAM=" + ninja,
        "-DCMAKE_CXX_COMPILER=" + failed_compiler,
        "-DCMAKE_C_COMPILER=" + failed_compiler,
    ], cwd=tmp_path, timeout=60)
    assert completed.returncode != 0
    assert b"Compiler identification failed" in completed.stderr
    assert not (build / "compile_commands.json").exists()
