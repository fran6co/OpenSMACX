"""Real deleting-destructor ANY selection with unresolved incoming weak uses."""

from pathlib import Path
import shutil

import pytest

from match_recovery.application_context import application_build
from match_recovery.driver import working_state
from match_recovery.linked_comdat_selection import comdat_selection
from match_recovery.linked_context import linked_build
from match_recovery.pe_coff import Coff, weak_external
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_common_request_compiler import ADDED, WITNESS, tiny_graph


def test_real_duplicate_deleting_destructors_keep_incoming_weak_uses_distinct():
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("VC6/Wine are required for this actual compiler control")
    root = Path(__file__).resolve().parents[2]
    state = working_state(root)
    sources = {
        WITNESS: b'''#include "native_owner.h"
volatile int native_sink;
NativeOwner::~NativeOwner() { native_sink = value; }
extern int separate_owner();
int main() { NativeOwner *owner = new NativeOwner; owner->value = 7;
    delete owner; return separate_owner(); }
''',
        ADDED: b'''#include "native_owner.h"
int separate_owner() { NativeOwner *owner = new NativeOwner; owner->value = 11;
    delete owner; return 0; }
''',
        "src/native_owner.h": b"struct NativeOwner { virtual ~NativeOwner(); int value; };\n",
        "src/IMAGE_PIN": (root / "src/IMAGE_PIN").read_bytes(),
    }
    graph = tiny_graph(added=True).replace(
        b'\n    "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/common_request_stdheader.h"', b"")
    try:
        with application_build(root, ("src",), sources,
                               versioned_files={"CMakeLists.txt": graph}) as context:
            assert context.report["compile_link"]["result"] == "pass"
            with linked_build(context) as live, comdat_selection(live) as selection:
                queries = []
                for unit in (WITNESS, ADDED):
                    owner = Path(unit).name + ".obj"
                    physical = live.inputs.objects.resolve(owner)
                    obj = Coff(physical["raw"])
                    function, = (symbol for symbol in obj.symbols.values()
                                 if symbol.name.startswith("??_GNativeOwner@@") and symbol.section > 0)
                    incoming = [symbol for symbol in obj.symbols.values() if symbol.storage == 105
                                and weak_external(obj, symbol)[0].name == function.name]
                    assert len(incoming) == 1
                    target, mode = weak_external(obj, incoming[0])
                    assert mode == 2 and target.section == 0 and target.storage == 2
                    queries.append(selection.loser(owner, function.section))
                removed, = [row for row in queries if row is not None]
                assert removed["disposition"] == "discarded_any_duplicate"
                certificate, = [row for row in selection.certificates()
                                if row["key"].startswith("??_GNativeOwner@@")]
                assert len(certificate["candidates"]) == 2
                assert len(certificate["incoming_weak_references"]) == 2
                assert all(row["resolution"] == "unknown" for row in certificate["incoming_weak_references"])
                assert certificate["outside_scope_verification"] == "unknown"
                live.check_current()
    finally:
        assert working_state(root) == state
