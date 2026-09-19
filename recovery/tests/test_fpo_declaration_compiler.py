"""Real compiler-emitted deleting-destructor FPO declaration references.

The disposable source model is a control, not recovered application behavior.
Only metadata correspondence is asserted; weak calls and class lifetime still
need their own independent proofs.
"""

from dataclasses import replace
from pathlib import Path
import shutil

import pytest

from match_recovery.application_context import application_build
from match_recovery.driver import working_state
from match_recovery.linked_context import linked_build
from match_recovery.linked_fpo import verify_fpo
from match_recovery.pe_coff import Coff
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_common_request_compiler import WITNESS, tiny_graph


def test_real_deleting_destructor_fpo_references_earlier_external_declaration():
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine") is None:
        pytest.skip("VC6/Wine are required for this actual compiler control")
    root = Path(__file__).resolve().parents[2]
    state = working_state(root)
    sources = {
        WITNESS: b'''volatile int native_sink;
struct NativeOwner { virtual ~NativeOwner(); int value; };
NativeOwner::~NativeOwner() { native_sink = value; }
int main() { NativeOwner *owner = new NativeOwner; owner->value = 7;
    delete owner; return native_sink; }
''',
        "src/IMAGE_PIN": (root / "src/IMAGE_PIN").read_bytes(),
    }
    graph = tiny_graph(added=False).replace(
        b'\n    "/FI${CMAKE_CURRENT_SOURCE_DIR}/src/common_request_stdheader.h"', b"")
    try:
        with application_build(root, ("src",), sources,
                               versioned_files={"CMakeLists.txt": graph}) as context:
            assert context.report["compile_link"]["result"] == "pass"
            with linked_build(context) as live:
                inputs = live.inputs
                placed, = (row for row in inputs.mapping.publics
                           if row.symbol.startswith("??_GNativeOwner@@"))
                physical = inputs.objects.resolve(placed.owner)
                obj = Coff(physical["raw"])
                function = obj.function(placed.symbol)
                child, = (number for number, parent in obj.associations.items()
                          if parent == function.section and obj.sections[number - 1].name == ".debug$F")
                offset, target, kind = obj.sections[child - 1].relocations[0]
                declaration = obj.symbols[target]
                assert offset == 0 and kind == 7
                assert declaration.index < function.index
                assert declaration.name == function.name
                assert (declaration.storage, declaration.section, declaration.value,
                        declaration.type) == (2, 0, 0, 32)
                assert not obj.auxiliary.get(declaration.index)
                proof = verify_fpo(obj, child, function, inputs.image, placed.address, inputs.fixups)
                assert proof["format"] == "fpo_data" and proof["code_size"] > 0
                assert set(proof) == {"format", "data", "code_size"}
                with pytest.raises(ValueError, match="metadata"):
                    verify_fpo(obj, child, function, inputs.image, placed.address,
                               replace(inputs.fixups, fpo_records=()))
                live.check_current()
    finally:
        assert working_state(root) == state
