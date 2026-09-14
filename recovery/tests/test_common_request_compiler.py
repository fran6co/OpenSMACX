"""Real VC6 duplicate-common evidence in a tiny private application.

The compiler, linker, command receipts and linked observations are real. Only
the application source set and its versioned CMake graph are test-owned. This
does not recover C++, run the game, or grant whole-change acceptance.
"""

from dataclasses import replace
from pathlib import Path
import shutil

import pytest

from match_recovery.application_context import application_build
from match_recovery.build_graph import ApplicationGraph
from match_recovery.coff_preservation import compare_unit_presence
from match_recovery.common_preservation import _compare_linked
from match_recovery.common_request_witnesses import retained_witnesses
from match_recovery.driver import working_state
from match_recovery.linked_context import linked_build
from match_recovery.linked_initializers import observe_build_initializers
from match_recovery.model import digest
from match_recovery.pe_coff import Coff
from match_recovery.preservation_objects import ObjectTrial, application_objects
from match_recovery.source_replica import snapshot_digest
from match_recovery.startup_coverage import consume_startup, consume_startup_fpo
from match_recovery.startup_projection import project_startup
from recovery.environment.wine import VC6_ROOT


ADDED = "src/common_request_added.cpp"
WITNESS = "src/common_request_entry.cpp"
FLAGS = ("/c", "/O2", "/Gy", "/GX")


def tiny_graph(*, added):
    # Keep the existing target/output names: recovery/build.json owns the
    # native command capture, PDB/FIXUP link policy and output validation.
    return ('''cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/cmake/toolchains/vc6.cmake"
    CACHE FILEPATH "Existing VC6 compiler interaction")
project(OpenSMACX LANGUAGES CXX)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
add_executable(OpenSMACX src/common_request_entry.cpp'''
        + (" src/common_request_added.cpp" if added else "") + ''')
target_compile_options(OpenSMACX PRIVATE /MT /Gy /GX
    "--repro-root=${CMAKE_CURRENT_SOURCE_DIR}"
    "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/common_request_stdheader.h")
target_link_options(OpenSMACX PRIVATE
    "/MAP:${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.map" /MAPINFO:LINES
    /FIXED:NO /VERBOSE
    "--link-diagnostics=${CMAKE_CURRENT_BINARY_DIR}/OpenSMACX.link.log")
''').encode()


def test_real_tiny_application_proves_duplicate_common_request_with_retained_witness():
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("VC6/Wine are required for this actual compiler control")
    # Match the application's Release form: /Z7 is deliberately not added.
    # Its associated debug relocations are a separate unsupported graph form;
    # the native build policy still requires linked PDB/FIXUP evidence here.
    root = Path(__file__).resolve().parents[2]
    state = working_state(root)
    baseline_sources = {
        WITNESS: b"int main() { return 0; }\n",
        "src/common_request_stdheader.h": b"#include <string>\n",
        "src/IMAGE_PIN": (root / "src/IMAGE_PIN").read_bytes(),
    }
    current_sources = {**baseline_sources, ADDED: b"// Test-owned empty TU; the forced standard header initializes.\n"}
    with application_build(root, ("src",), baseline_sources,
                           versioned_files={"CMakeLists.txt": tiny_graph(added=False)}) as baseline:
        with application_build(root, ("src",), current_sources,
                               versioned_files={"CMakeLists.txt": tiny_graph(added=True)}) as current:
            for context, count in ((baseline, 1), (current, 2)):
                proof = context.report["compile_link"]
                assert proof["result"] == proof["command_bindings"]["result"] == "pass"
                assert len(proof["compilations"]) == count
            compiled, = (row for row in current.report["compile_link"]["compilations"]
                         if row["source"] == str(current.root / ADDED))
            output = Path(compiled["output"])
            raw = output.read_bytes()
            obj = Coff(raw)
            observed = observe_build_initializers(current)
            assert observed["result"] == "pass"
            dependencies = observed["code_dependencies"]
            assert dependencies["result"] == "pass", dependencies.get("gaps")
            entries = [row for row in observed["entries"] if row["input"] == {"path": str(output)}]
            assert entries, "The actual standard header must emit a native initializer"
            # Test-owned source ownership is the added TU's forced header. The
            # selected symbols/slots come from this fresh native build, not a
            # historical spelling or a manufactured passing report.
            changes = [{"entity_id": "entity-" + digest({"unit": ADDED, "offset": row["offset"]}),
                        "unit": ADDED, "version": "current", "symbol": row["function"]["symbol"],
                        "offset": row["offset"], "evidence": ["Test-owned added TU forced standard-header initializer"]}
                       for row in entries]
            authorization = {"task": {"task_id": "compiler-common-control"}, "decisions": [],
                "permission": {"migration_id": "compiler-common-control", "author": "test coordinator",
                               "allowed": [], "evidence": ["Native duplicate-request control only"],
                               "generated_changes": changes}}

            def fresh():
                try:
                    baseline.check_current()
                finally:
                    current.check_current()
                assert working_state(root) == state

            graph = ApplicationGraph(root, "compiler-common-control", state["head"].decode().strip(),
                ("src",), baseline, current, {"baseline": baseline_sources, "current": current_sources},
                {"result": "unknown", "linked_effects": "unknown"}, authorization, fresh)
            fingerprint = digest({"native_compilation": compiled, "authorization": authorization})
            trial = ObjectTrial(raw, root, graph.task_name, graph.baseline_revision, digest(authorization),
                                snapshot_digest(current_sources), ADDED, "current", FLAGS, fingerprint)
            arguments = dict(unit=ADDED, version="current", flags=FLAGS, input_fingerprint=fingerprint)
            with application_objects(graph) as run:
                startup = run.cover_startup(trial, obj, changes=changes, **arguments)
                _, projected = project_startup(obj, consume_startup(startup, obj),
                                              fpo=consume_startup_fpo(startup, obj))
                references = {row["target"] for row in projected["covered_root_references"]}
                requests = {symbol.name: symbol.value for index, symbol in obj.symbols.items()
                            if index in references and symbol.section == 0 and symbol.storage == 2
                            and symbol.type == 0 and symbol.value > 0}
                assert requests, "The real removed initializer must reference a common request"
                with linked_build(baseline) as old, linked_build(current) as new:
                    common_proof = _compare_linked(old, new, tuple(sorted(requests)))
                    assert common_proof["result"] == "pass", common_proof
                    assert common_proof["allocation_isolation"] == "unknown"
                    witnesses = retained_witnesses(old, new, excluded_units=frozenset({ADDED}), requests=requests)
                    assert set(witnesses) == set(requests)
                    assert {row["unit"] for row in witnesses.values()} == {WITNESS}
                coverage = run.cover_common_requests(trial, obj, startup=startup, **arguments)
                assert coverage is not None
                before = compare_unit_presence(obj, {}, present="current", startup=startup)
                after = compare_unit_presence(obj, {}, present="current", startup=startup, common=coverage)
                assert before["result"] == "fail"
                assert set(requests) <= {row["symbol"] for row in before["symbol_changes"]}
                assert not set(requests) & {row["symbol"] for row in after["symbol_changes"]}
                assert before["changed_sections"] == after["changed_sections"]
                assert before["residual_sections"] == after["residual_sections"]
                assert [row for row in before["symbol_changes"] if row["symbol"] not in requests] == after["symbol_changes"]
                # This second trial is valid whole-object evidence, but must
                # not borrow the first trial's generated-root authorization.
                altered = replace(trial, input_fingerprint="c" * 64)
                changed_arguments = {**arguments, "input_fingerprint": altered.input_fingerprint}
                assert run(altered, obj, **changed_arguments)["result"] == "pass"
                with pytest.raises(ValueError):
                    run.cover_common_requests(altered, obj, startup=startup, **changed_arguments)
                assert graph.report["result"] == graph.report["linked_effects"] == "unknown"
            with pytest.raises(ValueError, match="expired"):
                compare_unit_presence(obj, {}, present="current", startup=startup, common=coverage)
            fresh()
    assert working_state(root) == state
