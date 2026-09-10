"""Advisory retention integration: synthetic objects, no native compiler."""

from dataclasses import replace
import hashlib
import json
import struct
import sys
from types import ModuleType

import pytest

from match_recovery.application_binary import verify_task
from match_recovery.cli import main
from match_recovery.model import Origin
from match_recovery.pe_coff import Coff, Section
from match_recovery.tasks import Task, TaskStore
from test_application_commands import project
from test_binary import coff_fixture, pe_fixture


def header_body(root, header="code/model.hpp"):
    (root / "code/unit.cc").write_text(
        '// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001\n'
        + (f'// body {header}\n' if header else '') + '#include "model.hpp"\n')
    if header:
        path = root / header
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text("inline void f() {}\n")


def configuration(root, **changes):
    path = root / "recovery/project.json"
    value = json.loads(path.read_text())
    value["source"].update(changes)
    path.write_text(json.dumps(value))


@pytest.fixture
def witness(monkeypatch):
    calls = []
    module = ModuleType("match_recovery.function_witness")
    def retained(*args):
        calls.append(args)
        return Coff(coff_fixture()), {"result": "pass", "compile_attempts": 3,
                                      "policy": "test-owned private retained definition"}
    module.retained_definition = retained
    monkeypatch.setitem(sys.modules, module.__name__, module)
    return calls, module


def missing_object():
    return coff_fixture().replace(b"_f", b"_h")


def check(root, *, compiler=None, enabled=True, task="owner"):
    return verify_task(root, task, root / "original.exe", compiler=compiler or (lambda *_: missing_object()),
                       retained_diagnostics=enabled)


def test_default_does_not_probe_missing_definition_or_add_diagnostic_fields(project, witness):
    header_body(project)
    report = check(project, enabled=False)
    assert report["result"] == "unknown"
    assert report["compile_attempts"] == 1 and witness[0] == []
    assert "retained_definitions" not in report and "retained_definition_policy" not in report


def test_successful_private_definition_never_promotes_product_or_other_dimensions(project, witness):
    header_body(project)
    plain = check(project, enabled=False)
    detailed = check(project)
    observations = detailed.pop("retained_definitions")
    assert "advisory_only" in detailed.pop("retained_definition_policy")
    observation, = observations
    assert observation["measurement"]["result"] == "pass"
    assert detailed["result"] == "unknown" and detailed["functions"][0]["result"] == "unknown"
    assert detailed["structure_verification"] == detailed["build_verification"] == detailed["runtime_verification"] == "unknown"
    assert detailed["ready_for_collection"] is False
    assert detailed["compile_attempts"] == 4
    detailed["compile_attempts"] -= observation["compile_attempts"]
    assert detailed.pop("input_fingerprint") != plain.pop("input_fingerprint")
    assert detailed == plain  # Includes product data, tables, imports, coverage and scope.
    args, = witness[0]
    assert args[2] == "code/model.hpp" and args[-2] == ("-trial",)


@pytest.mark.parametrize("case", ["exact", "mismatch", "non-comdat", "data-definition", "absolute", "common",
                                  "weak", "auxiliary", "duplicate", "unsupported-type",
                                  "compile-error", "malformed-object"])
def test_present_or_malformed_function_and_failed_compilation_do_not_probe(project, witness, case):
    header_body(project)
    def compiler(*_):
        if case == "compile-error":
            raise ValueError("test-owned compiler failure")
        if case == "malformed-object":
            return b"not COFF"
        raw = bytearray(coff_fixture(b"\x90" if case == "mismatch" else b"\xc3"))
        symbols = struct.unpack_from("<I", raw, 8)[0]
        if case == "non-comdat":
            struct.pack_into("<I", raw, 56, 0x60500020)
        elif case == "data-definition":
            struct.pack_into("<H", raw, symbols + 14, 0)
        elif case == "absolute":
            struct.pack_into("<h", raw, symbols + 12, -1)
        elif case == "common":
            struct.pack_into("<IhH", raw, symbols + 8, 16, 0, 0)
        elif case in ("weak", "auxiliary", "duplicate", "unsupported-type"):
            struct.pack_into("<h", raw, symbols + 12, 0)
            if case == "weak":
                raw[symbols + 16] = 105
            elif case == "auxiliary":
                raw[symbols + 17] = 1
            elif case == "duplicate":
                raw[symbols + 18:symbols + 20] = b"_f"
            else:
                struct.pack_into("<H", raw, symbols + 14, 0x30)
        return bytes(raw)
    report = check(project, compiler=compiler)
    assert witness[0] == [] and report["retained_definitions"] == []
    assert report["compile_attempts"] == 1
    assert report["result"] == ("pass" if case == "exact" else "fail" if case == "mismatch" else "unknown")


@pytest.mark.parametrize("header", [None, "code/model.cc", "uncovered/model.hpp", "code/model.txt", "code/seeds/model.hpp"])
def test_only_explicit_configured_non_seed_header_body_links_can_probe(project, witness, header):
    header_body(project, header)
    configuration(project, seed_roots=["code/seeds"])
    report = check(project)
    assert witness[0] == [] and report["retained_definitions"] == []
    assert report["result"] == "unknown"


@pytest.mark.parametrize("case", ["seed-owner", "ambiguous-owner", "placeholder", "header-owner"])
def test_unusable_product_ownership_cannot_be_repaired_by_a_probe(project, witness, case):
    header_body(project)
    owner = project / "code/unit.cc"
    if case == "seed-owner":
        configuration(project, seed_roots=["code/seeds"])
        (project / "code/seeds").mkdir()
        owner.rename(project / "code/seeds/unit.cc")
    elif case == "ambiguous-owner":
        (project / "code/other.cc").write_bytes(owner.read_bytes())
    elif case == "placeholder":
        owner.write_text(owner.read_text() + "void f() {\n// BODY GOES HERE.\n}\n")
    else:
        owner.rename(project / "code/owner.hpp")
    report = check(project)
    assert report["result"] == "unknown"
    assert witness[0] == [] and report["retained_definitions"] == []
    assert report["compile_attempts"] == 0


@pytest.mark.parametrize("product_defines_target", [False, True])
def test_probe_reference_targets_require_product_definition_not_probe_storage(project, witness, product_defines_target):
    header_body(project)
    original = pe_fixture(b"\xa1\0\x20\x40\0\xc3", absolute_site=1)
    pin = hashlib.sha256(original).hexdigest()
    (project / "original.exe").write_bytes(original)
    (project / "code/PIN").write_text("sha256 " + pin + "\n")
    owner = project / "code/unit.cc"
    owner.write_text(owner.read_text().replace("-0x00401001", "-0x00401006")
                     + "// RECOVERY_OBJECT: v1 0x00402000 _g\nint g;\n")
    TaskStore(project).create(Task("references", "synthetic", "coordinator", "Inspect retained references", "b" * 40,
                                   (Origin(pin, "pe:va", 0x401000),), ("code/unit.cc",), ("synthetic",)))
    probe = Coff(coff_fixture(b"\xa1\0\0\0\0\xc3", relocation=(1, 1, 6)))
    probe.sections = (*probe.sections, Section(bytes(4), (), 0x40300040, ".data"))
    probe.symbols[1] = replace(probe.symbols[1], section=2)
    def retained(*args):
        witness[0].append(args)
        return probe, {"result": "pass", "compile_attempts": 3}
    witness[1].retained_definition = retained
    def compiler(*_):
        if not product_defines_target:
            return missing_object()
        raw = bytearray(coff_fixture().replace(b"_f", b"_g"))
        symbols = struct.unpack_from("<I", raw, 8)[0]
        struct.pack_into("<H", raw, symbols + 14, 0)
        struct.pack_into("<I", raw, 56, 0x40300040)
        return bytes(raw)
    baseline = check(project, compiler=compiler, task="references", enabled=False)
    report = check(project, compiler=compiler, task="references")
    observed, = report["retained_definitions"]
    assert observed["measurement"]["references"] == ("pass" if product_defines_target else "unknown")
    assert observed["measurement"]["result"] == ("pass" if product_defines_target else "unknown")
    assert report["result"] == "unknown"
    for field in ("functions", "object_definitions", "generated_tables", "generated_coverage", "import_bindings", "shared_functions"):
        assert report[field] == baseline[field]
    assert report["object_definitions"][0]["definition"] == ("pass" if product_defines_target else "unknown")


def test_opt_in_cli_does_not_turn_private_pass_into_success(project, witness, monkeypatch, capsys):
    header_body(project)
    monkeypatch.setattr("match_recovery.application_binary.Compiler", lambda *args, **kwargs: (lambda *_: missing_object()))
    assert main(["tasks", "check-binary", "owner", "--root", str(project), "--image", "original.exe", "--retained-diagnostics"]) == 3
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == "unknown" and report["retained_definitions"][0]["measurement"]["result"] == "pass"


@pytest.mark.parametrize("action", ["check", "check-layout", "check-restoration"])
def test_advisory_cli_flag_is_not_an_acceptance_or_layout_option(project, action):
    with pytest.raises(SystemExit) as error:
        main(["tasks", action, "owner", "--root", str(project), "--retained-diagnostics"])
    assert error.value.code == 2


def test_source_changes_during_probe_invalidate_advisory_and_product_report(project, witness):
    header_body(project)
    def changed(*args):
        (project / "code/model.hpp").write_text("inline void f() { changed(); }\n")
        return Coff(coff_fixture()), {"result": "pass", "compile_attempts": 1}
    witness[1].retained_definition = changed
    with pytest.raises(ValueError, match="changed during measurement"):
        check(project)
