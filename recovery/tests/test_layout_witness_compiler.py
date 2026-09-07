"""Controlled layout witnesses through the project's real compiler interface."""

from pathlib import Path
import sys

import pytest

from match_recovery.codeview import ClassLayouts, ClassNotEmitted
from match_recovery.layout_witness import missing_layout_witnesses
from match_recovery.layout_witness import _absence_witness, declaration_absent
from match_recovery.pe_coff import Coff
from recovery.environment.compile import compile_object, compile_overlay
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_compiler_variants import variant_project


@pytest.mark.parametrize("extra_flags", [(), ("/Oy-",)])
@pytest.mark.parametrize("condition", ["visible", "absent", "macro", "forward_only"])
def test_real_cpp_probe_witnesses_only_visible_complete_declarations(variant_project, extra_flags, condition):
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source = root / "code/unit.cpp"
    declaration = {"visible": "struct Owner { int prefix; int field; };\n",
                   "absent": "", "forward_only": "struct Owner;\n",
                   "macro": "struct Owner { int field; };\n#define Owner int\n"}[condition]
    source.write_text(declaration + "int work() { return 0; }\n")
    original = source.read_bytes()
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/Z7", *extra_flags)
    baseline = Coff(compile_object(source, command, flags))
    with pytest.raises(ClassNotEmitted):
        ClassLayouts(baseline).layout("Owner")

    class Compiler:
        def overlay(self, source, replacements, arguments, flags):
            return compile_overlay(root, source, replacements, arguments, flags)

    report = missing_layout_witnesses(root, "code/unit.cpp", ["Owner"], baseline, Compiler(), command, flags)
    row, = report["witnesses"]
    if condition == "visible":
        assert row["result"] == row["replica_control"] == row["probe_control"] == "pass"
        assert row["compiler_layout"]["members"] == {"prefix": 0, "field": 4}
    elif condition in ("absent", "forward_only"):
        assert declaration_absent(row) and row["compiler_layout"] is None
    else:
        assert row["result"] == "unknown" and row["compiler_layout"] is None
    assert source.read_bytes() == original
    assert not list((root / "build/recovery/variants").iterdir())


@pytest.mark.parametrize("name,declaration,absent", [
    ("Owner", "", True),
    ("Owner", "struct Owner;", True),
    ("Owner", "struct Owner; Owner* outside;", True),
    ("Owner", "struct Owner { int value; };", False),
    ("Owner", "class Owner { int value; };", False),
    ("Owner", "struct Owner { int value; }; inline int read(Owner* p) { return p->value; }", False),
    ("UI::Owner", "namespace UI { struct Owner; }", True),
    ("UI::Owner", "namespace UI { struct Owner { int value; }; }", False),
    ("Outer::Owner", "struct Outer { struct Owner; };", False),  # Not a namespace path.
    ("Owner", "namespace UI { struct Owner { int value; }; } using UI::Owner;", False),
    ("Owner", "namespace UI { struct Owner { int value; }; } using namespace UI;", False),
    ("Owner", "namespace { struct Owner { int value; }; }", False),
])
def test_counterfactual_definition_controls_do_not_replace_existing_types(variant_project, name, declaration, absent):
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source = root / "code/unit.cpp"
    source.write_text(declaration + "\nint work() { return 0; }\n")
    original = source.read_bytes()
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/Z7")
    compile_object(source, command, flags)  # Every original must compile independently.

    def compile(contents):
        return Coff(compile_overlay(root, source, {"code/unit.cpp": contents}, command, flags))

    def probe():
        return _absence_witness(compile, original, name, "match_recovery_test_absence", "")

    if absent:
        report = probe()
        assert report["result"] == "pass" and report["compiler_layout"] is None
        assert report["definition_controls"] == [{"extent": 17, "result": "pass"}, {"extent": 29, "result": "pass"}]
    else:
        with pytest.raises(ValueError):
            probe()
    assert source.read_bytes() == original
