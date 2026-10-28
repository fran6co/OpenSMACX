"""Opt-in test-owned VC6 local-debug records, with and without FPO metadata."""
from dataclasses import replace
from pathlib import Path
import os
import sys

import pytest

from match_recovery.codeview_procedures import S_LDATA32_ST, S_LABEL32_ST
from match_recovery.linked_codeview import source_procedure
from match_recovery.linked_fpo import validate_fpo_object
from match_recovery.pe_coff import Coff
from match_recovery.processes import run_bounded
from recovery.environment.compile import compile_object
from recovery.tests.test_branched_eh_compiler import _require_toolchain

ROOT = Path(__file__).resolve().parents[2]
OWNER = """
extern int selected(int);
int dispatch(int mode, int value) {
    switch (mode) {
    case 0: return selected(value + 7);
    case 1: return selected(value * 3);
    case 2: return selected(value - 5);
    case 3: return selected(value ^ 9);
    default: return -1;
    }
}
"""
ENTRY = """
extern int dispatch(int, int);
volatile int Input;
int selected(int value) { return value + Input; }
int main() { return dispatch(Input, Input); }
"""


@pytest.mark.skipif(os.environ.get("OPENSMACX_ASSOCIATED_DEBUG_CONTROL") != "1",
                    reason="requires an explicitly leased VC6 compiler control")
@pytest.mark.parametrize("omit_frame", [False, True])
def test_real_local_debug_ownership_and_retarget_damage(tmp_path, omit_frame):
    _require_toolchain()
    command = (sys.executable, str(ROOT / "recovery/environment/wine.py"), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/GR-", "/GX", "/Z7", "/MT", "/Ob0",
             "/Oy" if omit_frame else "/Oy-")
    objects = {}
    for name, source in (("owner", OWNER), ("entry", ENTRY)):
        path = tmp_path / (name + ".cpp"); path.write_text(source)
        objects[name] = compile_object(path, command, flags)
        (tmp_path / (name + ".obj")).write_bytes(objects[name])
    linked = run_bounded([sys.executable, str(ROOT / "recovery/environment/wine.py"),
        "--tool=link", "/nologo", "/nodefaultlib", "/subsystem:console",
        "/incremental:no", "/opt:noref", "/opt:noicf", "/out:" + str(tmp_path / "control.exe"),
        str(tmp_path / "owner.obj"), str(tmp_path / "entry.obj"),
        "libcmt.lib", "oldnames.lib", "kernel32.lib"], cwd=tmp_path, timeout=120)
    assert linked.returncode == 0, linked.stdout + linked.stderr
    assert (tmp_path / "control.exe").read_bytes().startswith(b"MZ")
    obj = Coff(objects["owner"]); root = obj.function("?dispatch@@YAHHH@Z")
    children = [number for number, parent in obj.associations.items() if parent == root.section]
    debug, = [n for n in children if obj.sections[n - 1].name == ".debug$S"]
    procedure = source_procedure(obj, debug, root, local_addresses=True)
    assert {address.kind for address in procedure.addresses} == {S_LDATA32_ST, S_LABEL32_ST}
    fpo = [n for n in children if obj.sections[n - 1].name == ".debug$F"]
    assert len(fpo) == int(omit_frame)
    if fpo:
        assert validate_fpo_object(obj, fpo[0], root, source_codeview=True)["code_size"] == procedure.length
    with pytest.raises(ValueError):
        source_procedure(obj, debug, root)
    address = next(a for a in procedure.addresses if a.kind == S_LABEL32_ST)
    external, = [s for s in obj.symbols.values() if s.name == "?selected@@YAHH@Z"]
    section = obj.sections[debug - 1]
    relocations = tuple((offset, external.index if offset in
                        (address.record_offset + 4, address.record_offset + 8) else target, kind)
                       for offset, target, kind in section.relocations)
    obj.sections = tuple(replace(s, relocations=relocations) if i == debug else s
                         for i, s in enumerate(obj.sections, 1))
    with pytest.raises(ValueError, match="unique owned root label"):
        source_procedure(obj, debug, root, local_addresses=True)
