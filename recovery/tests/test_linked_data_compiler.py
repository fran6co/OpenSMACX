"""Actual ordinary data/reference evidence in a tiny compiler-owned application.

Run inside a coordinator compiler lease. Product C++ and recovery decisions stay
untouched; these are initial-data controls, not whole-change/game acceptance.
"""

import os
from pathlib import Path
import shutil

import pytest

from match_recovery.application_context import application_build
from match_recovery.driver import working_state
from match_recovery.linked_context import linked_build
from match_recovery.linked_data_ownership import section_producers
from match_recovery.linked_input_epoch import input_epoch
from match_recovery.linked_input_selection import observe_input_selection, compare_selection
from match_recovery.linked_section_domain import observe_sections
from match_recovery.pe_coff import Coff
from match_recovery.resources import INHERITED
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_common_request_compiler import ADDED, WITNESS, tiny_graph


def test_real_data_bytes_callback_and_live_producer_identity():
    if os.environ.get(INHERITED) != "1":
        pytest.skip("native data control requires a coordinator-owned compiler lease")
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("VC6/Wine are required for this actual compiler control")
    root = Path(__file__).resolve().parents[2]
    state = working_state(root)
    sources = {
        WITNESS: b'extern "C" int (*native_pointer)();\nint main() { return native_pointer(); }\n',
        ADDED: b'extern "C" { int native_words[4] = {11, 22, 33, 44};\n'
               b'int native_callback() { return native_words[0]; }\n'
               b'int (*native_pointer)() = native_callback; }\n',
        "src/IMAGE_PIN": (root / "src/IMAGE_PIN").read_bytes(),
    }
    # Keep the actual project compiler/link/PDB policy without a standard-header
    # dependency: this control needs ordinary data and a real callback relocation.
    graph = tiny_graph(added=True).replace(
        b'\n    "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/common_request_stdheader.h"', b"")
    with application_build(root, ("src",), sources, versioned_files={"CMakeLists.txt": graph}) as context:
        assert context.report["compile_link"]["result"] == "pass"
        with linked_build(context) as native:
            placed, = (row for row in native.inputs.mapping.publics if row.symbol == "_native_words")
            selected = native.inputs.objects.resolve(placed.owner)
            obj = Coff(selected["raw"])
            symbol, = (row for row in obj.symbols.values() if row.name == "_native_words")
            with input_epoch(context, native.inputs.objects) as epoch:
                catalog = observe_input_selection(context, native.inputs.objects, epoch=epoch)
                assert catalog["candidate_coverage"] == "pass"
                assert {row["producer"]["unit"] for row in catalog["inventory"]
                        if row["producer"]["kind"] == "compiled"} == {ADDED, WITNESS}
                comparison = compare_selection(catalog, catalog)
                assert comparison["dependency_families"] == "pass" and not comparison["compiled_changes"]
                assert comparison["result"] == comparison["retained_section_selection"] == "unknown"
                domain = observe_sections(context, native.inputs.objects, epoch=epoch)
                assert domain["enumeration"] == "pass"
                assert {"owner": selected["owner"], "section": symbol.section} in domain["data_sections"]
                assert any(row["symbol"] == "_native_callback" for row in domain["code_roots"])
                assert domain["retained_section_selection"] == "unknown"
                with section_producers(context, native.inputs.objects, epoch=epoch) as producers:
                    descriptor = producers.identify(placed.owner, symbol.section)
                    assert descriptor == {"kind": "compiled", "unit": ADDED, "section": ".data"}
                    assert producers.resolve(descriptor) == {"owner": selected["owner"], "section": symbol.section}
                    assert producers.identify(placed.owner, symbol.section) == descriptor
                    proof = native.data_contents(placed.owner, symbol.section)
                    assert proof["result"] == proof["input_consistency"] == "pass", proof
                    assert proof["envelope"]["size"] == 20
                    assert any(row["identity"] == {"external": "_native_callback"} for row in proof["references"])
                    assert proof["allocation_isolation"] == proof["remaining_linked_effects"] == "unknown"
                    assert not proof["ready_for_collection"]
                    producers.check_current()
                assert observe_input_selection(context, native.inputs.objects, epoch=epoch) == catalog
            with pytest.raises(ValueError, match="expired"):
                producers.check_current()
            with pytest.raises(ValueError, match="expired"):
                epoch.check_current()
    assert working_state(root) == state
