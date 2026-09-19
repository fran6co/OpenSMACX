"""Real VC6 ordinary retention; incomplete linked coverage must stay unknown."""

from pathlib import Path
import shutil

import pytest

from match_recovery.application_context import application_build
from match_recovery.driver import working_state
from match_recovery.linked_context import linked_build
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_common_request_compiler import ADDED, WITNESS, tiny_graph


def test_real_ordinary_payload_retention_does_not_certify_pdb_characteristics():
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("VC6/Wine are required for this actual compiler control")
    root = Path(__file__).resolve().parents[2]
    state = working_state(root)
    sources = {
        WITNESS: b'extern "C" int (*native_pointer)();\nint main() { return native_pointer(); }\n',
        ADDED: (b'extern "C" { int native_words[4] = {11,22,33,44}; '
                b'int native_callback() { return native_words[0]; } '
                b'int (*native_pointer)() = native_callback; }\n'),
        "src/IMAGE_PIN": (root / "src/IMAGE_PIN").read_bytes(),
    }
    graph = tiny_graph(added=True).replace(
        b'\n    "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/common_request_stdheader.h"', b"")
    try:
        with application_build(root, ("src",), sources,
                               versioned_files={"CMakeLists.txt": graph}) as context:
            assert context.report["compile_link"]["result"] == "pass"
            with linked_build(context) as live:
                placed, = (row for row in live.inputs.mapping.publics if row.symbol == "_native_words")
                selected = live.inputs.objects.resolve(placed.owner)
                report = live.ordinary_allocations()
                targets = [row for row in report["sections"] if row["input"] == selected["identity"]
                           and row["classification"] == "ordinary_data"]
                assert targets
                for row in targets:
                    assert row["retention"] == row["input_consistency"] == "pass", row
                    # This pinned VC6 producer adds a flag to the PDB record.
                    # Native payload correspondence cannot explain it away.
                    assert row["flags"] == 0xC0300040 and row["pdb_flags"] == 0xC0302040
                    assert row["characteristics_agreement"] == row["allocation_isolation"] == "unknown"
                assert report["empty_contributions"]
                assert set(report["empty_contributions"]) <= set(report["unmatched_contributions"])
                # Commons have no input COFF section to join. Native requests
                # and PDB extents retain them without explaining producer flags.
                names = {row.symbol for row in live.inputs.mapping.publics if row.owner == "<common>"}
                assert names
                commons = {row["symbol"]: row for row in report["common_storage"]["allocations"]}
                assert names <= commons.keys()
                # These runtime commons have possible strong definitions in
                # fpinit.obj. A MAP placement does not prove those candidates
                # absent; keep this real conservative-selection refusal.
                floating = {"__FPinit", "__FPmtinit", "__FPmtterm"}
                assert floating <= commons.keys()
                assert "__FPinit" in names
                for name in floating:
                    row = commons[name]
                    assert row["retention"] == row["input_consistency"] == "unknown"
                    assert "contribution" not in row
                    assert any(item["declaration"] == "defined" and item["member"].endswith("/fpinit.obj")
                               for item in row["proof"]["blockers"])
                assert "__crtheap" in names - floating
                for name in names:
                    if name in floating:
                        continue
                    row = commons[name]
                    assert row["retention"] == row["input_consistency"] == "pass", row
                    assert row["pdb_flags"] & 0x2000
                    assert row["characteristics_agreement"] == row["allocation_isolation"] == "unknown"
                    assert row["contribution"] not in report["unmatched_contributions"]
                assert any(row.get("symbol") in names and "characteristics remain unexplained" in row["reason"]
                           for row in report["gaps"])
                assert report["result"] == report["allocation_domain"] == "unknown"
                assert report["allocation_isolation"] == report["remaining_linked_effects"] == "unknown"
                assert not report["ready_for_collection"]
    finally:
        assert working_state(root) == state
