"""Compiler-controlled inline attribution; synthetic COFF, no game fixtures."""

import json
import struct

import pytest

from match_recovery.model import Origin
from match_recovery.preservation import check_preservation
from match_recovery.tasks import Task, TaskStore
from test_preservation import project
from test_task_source import git


def object_bytes(functions, directives=b""):
    specs = [(b".text", code, name.encode(), 2, 0x60501020) for name, code in functions]
    if directives:
        specs.append((b".drectve", directives, None, 0, 0x100A00))
    start = 20 + 40 * len(specs)
    headers, data, symbols = [], bytearray(), bytearray()
    for number, (section, raw, name, selection, flags) in enumerate(specs, 1):
        headers.append(section.ljust(8, b"\0") + struct.pack("<IIIIIIHHI", 0, 0, len(raw),
                       start + len(data), 0, 0, 0, 0, flags))
        data.extend(raw)
        symbols.extend(section.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, 0, 3, 1))
        symbols.extend(struct.pack("<IHHIHB3x", len(raw), 0, 0, 0, 0, selection))
        if name:
            symbols.extend(name.ljust(8, b"\0") + struct.pack("<IhHBB", 0, number, 0x20, 2, 0))
    return (struct.pack("<HHIIIHH", 0x14C, len(specs), 0, start + len(data), len(symbols) // 18, 0, 0)
            + b"".join(headers) + data + symbols + struct.pack("<I", 4))


@pytest.fixture
def inline_project(project):
    (project / "code/owner.cc").write_text(
        '// ORIGINAL: 0x00401000 _f 0x00401000-0x00401006\n// body code/model.h\n#include "model.h"\n')
    (project / "code/model.h").write_text("inline int _f() { return 1; }\n")
    git(project, "add", ".")
    git(project, "commit", "-m", "Source-owned header body")
    task = Task("inline-owner", "portable", "coordinator", "Recover inline function",
                git(project, "rev-parse", "HEAD"), (Origin("a" * 64, "pe:va", 0x401000),),
                ("code/owner.cc", "code/model.h"), ("Explicit header binding",))
    TaskStore(project).create(task)
    git(project, "add", ".")
    git(project, "commit", "-m", "Authorize header ownership")
    (project / "code/model.h").write_text("inline int _f() { return 2; }\n")
    return project


class InlineCompiler:
    def __init__(self, root, *, fault=None):
        self.root, self.fault = root, fault
        self.calls = []

    def output(self, source, replacements, flags):
        unit = source.relative_to(self.root).as_posix()
        self.calls.append((unit, replacements, flags))
        header = replacements.get("code/model.h", (self.root / "code/model.h").read_bytes())
        body = replacements.get(unit, source.read_bytes())
        for contents in (header, body):
            if contents.startswith(b"#error "):
                raise ValueError(contents.splitlines()[0].decode())
        probe = b"for (;;)" in header
        if probe and self.fault == "compile-error":
            raise ValueError("deliberate probe compilation failure")
        if probe and self.fault == "mutated-source":
            (self.root / "code/model.h").write_text("inline int _f() { return 7; }\n")
        number = 2 if b"return 2" in header else 1
        if probe:
            number = 9
            if self.fault == "ignore-copy" and unit == "code/caller.cc":
                number = 2
            if self.fault == "ignore-canonical" and unit == "code/owner.cc":
                number = 2
            if self.fault == "second-flag" and flags == ("second",) and unit == "code/caller.cc":
                number = 2
        if self.fault == "different-copy" and unit == "code/caller.cc":
            number += 1
        functions = [("_f", b"\xb8" + struct.pack("<I", number) + b"\xc3")]
        if unit == "code/caller.cc":
            functions.append(("_h", b"\xb8" + struct.pack("<I", 4 if b"return 4" in body else 1) + b"\xc3"))
        if self.fault == "missing-canonical" and unit == "code/owner.cc":
            functions = [("_x", b"\xc3")]
        directives = b" /EXPORT:_f" if self.fault == "export" and number == 2 else b""
        return object_bytes(functions, directives)

    def __call__(self, source, command, flags):
        return self.output(source, {}, flags)

    def overlay(self, source, replacements, command, flags):
        return self.output(source, replacements, flags)


def check(root, fault=None, **kwargs):
    return check_preservation(root, "inline-owner", compiler=InlineCompiler(root, fault=fault), **kwargs)


def test_verified_header_copies_can_change_without_duplicate_original_bindings(inline_project):
    report = check(inline_project)
    assert report["result"] == "pass"
    caller = next(row for row in report["units"] if row["unit"] == "code/caller.cc")
    assert {row["version"] for row in caller["inline_ownership"]} == {"baseline", "current"}
    assert all(row["result"] == "pass" for row in caller["inline_ownership"])
    assert len(caller["baseline_scoped_functions"]) == len(caller["current_scoped_functions"]) == 1
    assert not report["ready_for_collection"]


def test_inline_attribution_does_not_waive_changed_outside_caller(inline_project):
    path = inline_project / "code/caller.cc"
    path.write_text(path.read_text().replace("return 1", "return 4"))
    report = check(inline_project)
    assert report["result"] == "fail"
    caller = next(row for row in report["units"] if row["unit"] == "code/caller.cc")
    assert caller["inline_ownership"][0]["result"] == "pass"
    assert any(symbol["symbol"] == "_h" for row in caller["changed_sections"]
               for symbol in row["current_symbols"])


def test_inline_attribution_keeps_export_directives_protected(inline_project):
    report = check(inline_project, "export")
    assert report["result"] == "fail"
    assert any(section["name"] == ".drectve" for row in report["units"] for section in row["changed_sections"])


@pytest.mark.parametrize("fault", ["different-copy", "ignore-copy", "ignore-canonical", "compile-error", "missing-canonical"])
def test_missing_compiler_witness_cannot_grant_cross_unit_scope(inline_project, fault):
    report = check(inline_project, fault)
    assert report["result"] == "unknown"
    caller = next(row for row in report["units"] if row["unit"] == "code/caller.cc")
    assert any(row["result"] == "unknown" for row in caller["inline_ownership"])
    assert not caller["current_scoped_functions"]


def test_source_mutation_during_body_probe_refuses_entire_measurement(inline_project):
    with pytest.raises(ValueError, match="changed during preservation"):
        check(inline_project, "mutated-source")


def test_diagnostic_inline_evidence_is_not_preservation_acceptance(inline_project):
    report = check(inline_project, diagnostic=True)
    assert report["result"] == "unknown" and report["diagnostic_only"]
    assert all(row["compiled_comparison"] == "pass" for row in report["units"])


def test_no_body_binding_means_no_inferred_ownership_from_symbol_name(inline_project):
    path = inline_project / "code/owner.cc"
    path.write_text(path.read_text().replace("// body code/model.h\n", ""))
    report = check(inline_project)
    assert report["result"] == "fail"
    caller = next(row for row in report["units"] if row["unit"] == "code/caller.cc")
    assert not caller["current_scoped_functions"]


def test_each_flag_set_needs_its_own_body_witness(inline_project):
    path = inline_project / "recovery/project.json"
    configuration = json.loads(path.read_text())
    configuration["measurement"]["flag_sets"].append(["second"])
    path.write_text(json.dumps(configuration))
    report = check(inline_project, "second-flag")
    assert report["result"] == "unknown"
    callers = [row for row in report["units"] if row["unit"] == "code/caller.cc"]
    assert callers[0]["result"] == "pass"
    assert callers[1]["result"] == "unknown"
    assert callers[1]["inline_ownership"][-1]["result"] == "unknown"


def test_new_body_hint_cannot_waive_old_unbound_emitted_copy(inline_project):
    header = inline_project / "code/model.h"
    owner = inline_project / "code/owner.cc"
    header.write_text("inline int _f() { return 1; }\n")
    owner.write_text(owner.read_text().replace("// body code/model.h\n", ""))
    git(inline_project, "add", ".")
    git(inline_project, "commit", "-m", "No declared header ownership for emitted copy")
    TaskStore(inline_project).create(Task(
        "late-header", "portable", "coordinator", "Bind the header without hiding existing code",
        git(inline_project, "rev-parse", "HEAD"), (Origin("a" * 64, "pe:va", 0x401000),),
        ("code/owner.cc", "code/model.h"), ("Keep historical outside code",)))
    header.write_text("inline int _f() { return 2; }\n")
    owner.write_text(owner.read_text().replace('#include', '// body code/model.h\n#include'))
    report = check_preservation(inline_project, "late-header", compiler=InlineCompiler(inline_project))
    assert report["result"] == "fail"
    caller = next(row for row in report["units"] if row["unit"] == "code/caller.cc")
    assert caller["inline_ownership"][0]["result"] == "pass"
    assert caller["newly_bound_protected_functions"]
    assert not caller["current_scoped_functions"]
