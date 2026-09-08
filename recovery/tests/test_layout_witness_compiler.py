"""Controlled layout witnesses through the project's real compiler interface."""

from pathlib import Path
import sys

import pytest

from match_recovery.codeview import ClassLayouts, ClassNotEmitted
from match_recovery.layout_witness import missing_layout_witnesses
from match_recovery.layout_witness import _absence_witnesses, declaration_absent
from match_recovery.pe_coff import Coff
from recovery.environment.compile import compile_object, compile_overlay
from recovery.environment.wine import VC6_ROOT
from recovery.tests.test_compiler_variants import variant_project


@pytest.mark.parametrize("damage", [None, "macro", "body", "layout"])
def test_real_present_classes_share_controls_and_isolate_a_bad_sibling(variant_project, damage):
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source = root / "code/unit.cpp"
    source.write_text("struct First { int field; };\nstruct Second { int field; };\n"
                      "struct Third { int field; };\n" +
                      ("#define Third int\n" if damage == "macro" else "") + "int work() { return 0; }\n")
    original = source.read_bytes()
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/Z7")
    baseline = Coff(compile_object(source, command, flags))
    calls = []

    class Compiler:
        def overlay(self, source, replacements, command, flags):
            data, = replacements.values()
            calls.append(data)
            if damage == "body":
                data = data.replace(b"Second* object) { return 1;", b"Second* object) { return 0;")
            elif damage == "layout" and b"return 1;" in data:
                data = data.replace(b"struct Second { int field; };", b"struct Second { char field; };")
            return compile_overlay(root, source, {"code/unit.cpp": data}, command, flags)

    report = missing_layout_witnesses(root, "code/unit.cpp", ["First", "Second", "Third"], baseline,
                                     Compiler(), command, flags)
    assert report["compile_attempts"] == len(calls)
    outcomes = {row["class_name"]: row for row in report["witnesses"]}
    if damage is None:
        assert report["compile_attempts"] == 3  # One replica, one whole batch, one changed-body batch.
        assert all(row["result"] == row["probe_control"] == "pass" for row in outcomes.values())
        assert all(name.encode() + b"* object" in calls[1] for name in outcomes)
    else:
        failed = "Third" if damage == "macro" else "Second"
        assert outcomes[failed]["result"] == "unknown" and outcomes[failed]["compiler_layout"] is None
        assert all(row["result"] == "pass" for name, row in outcomes.items() if name != failed)
        assert not declaration_absent(outcomes[failed])
        assert len(calls) > 3  # Fresh independent groups, not partial success borrowed from a failed batch.
    assert source.read_bytes() == original


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
        return _absence_witnesses(compile, original, [(name, "match_recovery_test_absence", "")])[0]

    if absent:
        report = probe()
        assert report["result"] == "pass" and report["compiler_layout"] is None
        assert report["definition_controls"] == [{"extent": 17, "result": "pass"}, {"extent": 29, "result": "pass"}]
    else:
        with pytest.raises(ValueError):
            probe()
    assert source.read_bytes() == original


@pytest.mark.parametrize("blocked", [False, True])
@pytest.mark.parametrize("extra_flags", [(), ("/Oy-",)])
def test_real_absence_batches_share_compiles_and_isolate_a_refused_class(variant_project, blocked, extra_flags):
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source = root / "code/unit.cpp"
    source.write_text(("#define Blocked int\n" if blocked else "") + "int work() { return 0; }\n")
    original = source.read_bytes()
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/Z7", *extra_flags)
    baseline = Coff(compile_object(source, command, flags))
    calls = []

    class Compiler:
        def overlay(self, source, replacements, arguments, flags):
            contents, = replacements.values()
            calls.append(contents)
            return compile_overlay(root, source, replacements, arguments, flags)

    names = ["First", "Second", "UI::Third"] + (["Blocked"] if blocked else [])
    report = missing_layout_witnesses(root, "code/unit.cpp", names, baseline, Compiler(), command, flags)
    assert [row["class_name"] for row in report["witnesses"]] == names
    assert all(declaration_absent(row) for row in report["witnesses"][:3])
    assert report["compile_attempts"] == len(calls)
    if blocked:
        assert report["witnesses"][-1]["result"] == "unknown"
        assert report["witnesses"][-1]["compiler_layout"] is None
        assert report["witnesses"][-1]["absence_reason"]
        assert any(b"struct First" in data and b"struct Blocked" in data for data in calls)
        assert any(b"struct First" in data and b"struct Blocked" not in data for data in calls)
    else:
        assert report["compile_attempts"] == 8  # Replica + five failed split pointer groups + two shared definitions.
        assert all(all(("struct " + name).encode() in data for name in ("First", "Second", "Third"))
                   for data in calls[-2:])
    assert source.read_bytes() == original
    assert not list((root / "build/recovery/variants").iterdir())


@pytest.mark.parametrize("damage", ["definition", "body"])
def test_real_batch_cannot_borrow_another_class_control(variant_project, damage):
    import re
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    root = variant_project
    source = root / "code/unit.cpp"
    source.write_text("int work() { return 0; }\n")
    original = source.read_bytes()
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/Z7")
    baseline = Coff(compile_object(source, command, flags))
    contents_seen = []

    class Compiler:
        def overlay(self, source, replacements, arguments, flags):
            contents, = replacements.values()
            contents_seen.append(contents)
            if damage == "definition":
                contents = re.sub(rb"(struct Second \{ char \w+\[)29(\];)", rb"\g<1>17\2", contents)
            else:
                contents = contents.replace(b"return sizeof(Second);", b"return 17;")
            return compile_overlay(root, source, {"code/unit.cpp": contents}, arguments, flags)

    report = missing_layout_witnesses(root, "code/unit.cpp", ["First", "Second"], baseline, Compiler(), command, flags)
    first, second = report["witnesses"]
    assert declaration_absent(first)
    assert second["result"] == "unknown" and second["compiler_layout"] is None
    assert report["compile_attempts"] == 10  # Replica, three failed pointer groups, failed definition pair, two singleton pairs.
    assert any(b"struct First" in data and b"struct Second" in data for data in contents_seen)
    assert any(b"struct First" in data and b"struct Second" not in data for data in contents_seen)
    assert source.read_bytes() == original
