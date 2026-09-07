"""Portable baseline/current controls; no game code or proprietary fixtures."""

import json
import re
import struct

import pytest

from match_recovery.cli import main
from match_recovery.model import Origin
from match_recovery.preservation import check_preservation
from match_recovery.tasks import Task, TaskStore
from test_binary import coff_fixture
from test_task_source import git


@pytest.fixture
def project(tmp_path):
    root = tmp_path
    (root / "code").mkdir()
    (root / "original").mkdir()
    (root / "recovery").mkdir()
    (root / ".gitignore").write_text("build/\n")
    (root / "original/PIN").write_text("sha256 " + "a" * 64 + "\n")
    (root / "code/owner.cc").write_text('// ORIGINAL: 0x00401000 _f 0x00401000-0x00401006\n#include "model.h"\nint f() { return 1; }\n')
    (root / "code/caller.cc").write_text('// ORIGINAL: 0x00401010 _h 0x00401010-0x00401016 BYTE_EXACT\n#include "model.h"\nint h() { return 1; }\n')
    (root / "code/model.h").write_text("struct Model { int field; };\n")
    config = {"schema_version": 1, "source": {"image_pin": "original/PIN", "address_space": "pe:va", "roots": ["code"], "include_paths": ["code"]},
              "measurement": {"backend": "pe-coff-i386", "compiler_profile": "portable-profile", "flag_sets": [["optimize"]],
                              "layout_flags": [], "unit_extensions": [".cc"], "inputs": [], "input_roots": []}}
    (root / "recovery/project.json").write_text(json.dumps(config))
    git(root, "init", "--initial-branch=work")
    git(root, "add", ".")
    git(root, "commit", "-m", "Synthetic source baseline")
    origin = Origin("a" * 64, "pe:va", 0x401000)
    task = Task("owner", "portable", "coordinator", "Recover a class", git(root, "rev-parse", "HEAD"),
                (origin,), ("code/owner.cc", "code/model.h"), ("source ownership",))
    TaskStore(root).create(task)
    git(root, "add", ".")
    git(root, "commit", "-m", "Authorize owner")
    return root


class Compiler:
    def __init__(self, root):
        self.root = root
        self.calls = []

    def output(self, source, replacements):
        name = source.relative_to(self.root).as_posix()
        inputs = {path: replacements.get(path, (self.root / path).read_bytes()) for path in (name, "code/model.h")}
        for data in inputs.values():
            if data.startswith(b"#error "):
                raise ValueError(data.splitlines()[0].decode())
        number = (2 if b"return 2" in inputs[name] else 1) + (4 if b"padding" in inputs["code/model.h"] else 0)
        obj = coff_fixture(b"\xb8" + struct.pack("<I", number) + b"\xc3")
        return obj.replace(b"_f", b"_h") if name == "code/caller.cc" else obj

    def __call__(self, source, command, flags):
        self.calls.append((source, flags, None))
        return self.output(source, {})

    def overlay(self, source, replacements, command, flags):
        self.calls.append((source, flags, replacements))
        return self.output(source, replacements)


def check(root, **kwargs):
    return check_preservation(root, "owner", compiler=kwargs.pop("compiler", Compiler(root)), **kwargs)


def test_fresh_noop_comparison_includes_unmapped_units_and_keeps_git_clean(project):
    report = check(project)
    assert report["result"] == "pass" and report["compile_attempts"] == 4
    assert report["translation_units"] == ["code/caller.cc", "code/owner.cc"]
    assert git(project, "status", "--porcelain") == ""
    assert not report["scope_complete"] and not report["ready_for_collection"] and not report["persisted"]
    assert report["outside_scope_verification"] == "unknown"


def test_scoped_byte_regression_preserves_unfinished_outside_code(project):
    path = project / "code/owner.cc"
    path.write_text(path.read_text().replace("return 1", "return 2"))
    before = git(project, "diff")
    report = check(project)
    assert report["result"] == "pass"
    assert report["changed_files"] == ["code/owner.cc"]
    assert report["controls"][0]["files"][0]["result"] == "pass"
    assert report["compile_attempts"] == 7
    assert git(project, "diff") == before


def test_header_change_that_alters_an_outside_caller_is_not_waived(project):
    (project / "code/model.h").write_text("struct Model { int padding; int field; };\n")
    report = check(project)
    assert report["result"] == "fail"
    assert {row["unit"]: row["result"] for row in report["units"]} == {"code/owner.cc": "pass", "code/caller.cc": "fail"}
    assert report["units"][0]["changed_sections"]
    assert report["outside_binding_changes"] == []


@pytest.mark.parametrize("damage", ["address", "symbol", "claim"])
def test_outside_source_bindings_and_obligations_are_load_bearing(project, damage):
    path = project / "code/caller.cc"
    before, after = {"address": ("0x00401010", "0x00401020"), "symbol": ("_h", "_other"), "claim": (" BYTE_EXACT", "")}[damage]
    path.write_text(path.read_text().replace(before, after))
    report = check(project)
    assert report["result"] == "fail" and report["outside_binding_changes"]


@pytest.mark.parametrize("damage", ["added", "removed", "renamed"])
def test_unmaterializable_file_sets_are_unknown_before_compilation(project, damage):
    if damage == "added": (project / "code/shadow.h").write_text("#error hidden header")
    elif damage == "removed": (project / "code/model.h").unlink()
    else: (project / "code/model.h").rename(project / "code/renamed.h")
    report = check(project, compiler=object())
    assert report["result"] == "unknown" and report["compile_attempts"] == 0
    assert report["unsupported_file_set_changes"]


def test_a_partial_unit_selection_cannot_claim_outside_preservation(project):
    report = check(project, units=("code/owner.cc",))
    assert report["result"] == "unknown" and report["unmeasured_units"] == ["code/caller.cc"]
    assert report["units"][0]["result"] == "pass"


@pytest.mark.parametrize("kind", ["ignored-overlay", "generic-error", "replica", "failed-baseline"])
def test_missing_controls_or_baseline_never_become_a_pass(project, kind):
    path = project / "code/owner.cc"
    path.write_text(path.read_text().replace("return 1", "return 2"))
    class Bad(Compiler):
        def overlay(self, source, replacements, command, flags):
            if kind == "ignored-overlay": return self.output(source, {})
            if kind == "generic-error": raise ValueError("unrelated compilation failure")
            if kind == "replica" and not any(b"#error " in data for data in replacements.values()): return coff_fixture(b"\xcc")
            if kind == "failed-baseline" and replacements.get("code/owner.cc", b"").endswith(b"return 1; }\n"):
                raise ValueError("old baseline does not compile")
            return super().overlay(source, replacements, command, flags)
    report = check(project, compiler=Bad(project))
    assert report["result"] == "unknown"


@pytest.mark.parametrize("damage", ["source", "header", "task", "head"])
def test_concurrent_inputs_or_git_changes_are_refused_and_not_reverted(project, damage):
    class Changes(Compiler):
        def __call__(self, *args):
            data = super().__call__(*args)
            if damage == "task":
                TaskStore(project).expand("owner", request_id="concurrent", author="coordinator", subjects=(),
                                           editable_files=("code/caller.cc",), evidence=("new authorization",), expected_revision=0)
            elif damage == "head": git(project, "commit", "--allow-empty", "-m", "Concurrent checkpoint")
            else:
                path = project / ("code/owner.cc" if damage == "source" else "code/model.h")
                path.write_text(path.read_text() + "// newer work\n")
            return data
    with pytest.raises(ValueError, match="changed during"):
        check(project, compiler=Changes(project))


def test_each_flag_trial_must_preserve_the_same_outside_population(project):
    path = project / "recovery/project.json"
    config = json.loads(path.read_text())
    config["measurement"]["flag_sets"].append(["other-flags"])
    path.write_text(json.dumps(config))
    report = check(project)
    assert report["result"] == "pass" and len(report["units"]) == 4 and report["compile_attempts"] == 8


def test_preservation_uses_configuration_only_cli_not_a_project_dispatcher(project, monkeypatch, capsys):
    from match_recovery import preservation
    monkeypatch.setattr(preservation, "Compiler", lambda root, **kwargs: Compiler(root))
    assert main(["tasks", "check-preservation", "owner", "--root", str(project), "--summary"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["result"] == "pass" and not report["ready_for_collection"]


def test_unannotated_product_unit_is_not_silently_omitted(project):
    (project / "code/utility.cc").write_text("int utility() { return 1; }\n")
    git(project, "add", ".")
    git(project, "commit", "-m", "Add synthetic unmapped product unit")
    original = TaskStore(project).read("owner")
    task = Task("unmapped-control", original.target, original.author, original.intent,
                git(project, "rev-parse", "HEAD"), original.subjects, original.editable_files, original.evidence)
    TaskStore(project).create(task)
    report = check_preservation(project, task.task_id, compiler=Compiler(project))
    assert report["result"] == "pass" and "code/utility.cc" in report["translation_units"]
    path = project / "code/utility.cc"
    path.write_text(path.read_text().replace("return 1", "return 2"))
    report = check_preservation(project, task.task_id, compiler=Compiler(project))
    assert report["result"] == "fail"
    assert next(row for row in report["units"] if row["unit"] == "code/utility.cc")["result"] == "fail"


@pytest.mark.parametrize("annotation", ["// RECOVERY_OBJECT: v1 0x00402000 _object",
                                        "// RECOVERY_IMPORT: v1 __imp__Paint@4 DEMO.dll name:Paint",
                                        "// RECOVERY_MEMBER: v1 Model field 0x4 0x00401010",
                                        "// RECOVERY_MEMBER: v3 Model field Other* 0x4 0x00401010",
                                        "// RECOVERY_BASE: v1 Model Base 0x0 0x00401010",
                                        "// RECOVERY_GENERATED: v1 vtable 0x00402000 ? _table 0x00401010"])
def test_object_and_structural_source_bindings_are_not_hidden_by_unchanged_code(project, annotation):
    header = project / "code/model.h"
    header.write_text(header.read_text() + annotation + "\n")
    report = check(project)
    assert all(row["result"] == "pass" for row in report["units"])
    assert report["result"] == "fail" and report["outside_annotation_changes"]


def test_scoped_structural_interpretation_still_requires_the_separate_structural_gate(project):
    header = project / "code/model.h"
    header.write_text(header.read_text() + "// RECOVERY_MEMBER: v1 Model field 0x4 0x00401000\n")
    report = check(project)
    assert report["result"] == "pass" and not report["outside_annotation_changes"]
    assert report["outside_scope_verification"] == "unknown" and not report["ready_for_collection"]


class TreeCompiler(Compiler):
    def output(self, source, replacements):
        files = {path.relative_to(self.root).as_posix(): path.read_bytes()
                 for path in (self.root / "code").rglob("*") if path.is_file()}
        return self.tree_output(source, {**files, **replacements})

    def tree_output(self, source, files):
        name = source.relative_to(self.root).as_posix()
        body = files[name]
        inputs = [body]
        for header in re.findall(rb'#include "([^"]+)"', body):
            path = "code/" + header.decode()
            if path not in files:
                raise ValueError("header absent from supplied tree: " + path)
            inputs.append(files[path])
        for data in inputs:
            if data.startswith(b"#error "):
                raise ValueError(data.splitlines()[0].decode())
        number = (2 if b"return 2" in body else 1) + (4 if any(b"padding" in data for data in inputs) else 0)
        obj = coff_fixture(b"\xb8" + struct.pack("<I", number) + b"\xc3")
        return obj.replace(b"_f", b"_h") if name == "code/caller.cc" else obj

    def materialize(self, source, roots, files, command, flags):
        assert roots == ("code",)
        self.calls.append((source, flags, files))
        return self.tree_output(source, files)


@pytest.mark.parametrize("change", ["added", "removed", "renamed"])
def test_complete_tree_preservation_handles_changed_header_file_sets(project, change):
    header = project / "code/model.h"
    if change == "added":
        (project / "code/new.h").write_text("struct Another {};\n")
    elif change == "removed":
        header.unlink()
    else:
        header.rename(project / "code/new.h")
    for name in ("owner", "caller"):
        path = project / f"code/{name}.cc"
        text = path.read_text()
        if change == "added": text += '#include "new.h"\n'
        elif change == "removed": text = text.replace('#include "model.h"\n', "")
        else: text = text.replace('"model.h"', '"new.h"')
        path.write_text(text)
    before = git(project, "diff")
    report = check(project, compiler=TreeCompiler(project))
    assert report["result"] == "pass", report
    assert report["materialization"] == "complete-tree-v5" and not report["unsupported_file_set_changes"]
    assert all(row["result"] == "pass" for row in report["controls"][0]["files"])
    if change != "added":
        assert next(row for row in report["controls"][0]["files"] if row["path"] == "code/model.h")["tree"] == "baseline"
    assert git(project, "diff") == before
    assert report["outside_scope_verification"] == "unknown"


def test_unobserved_new_file_is_still_a_coverage_gap(project):
    (project / "code/unused.h").write_text("// Not a compiler input\n")
    report = check(project, compiler=TreeCompiler(project))
    assert report["result"] == "unknown" and not report["unsupported_file_set_changes"]
    assert report["controls"][0]["files"][0]["unit"] is None


@pytest.mark.parametrize("change", ["added", "removed", "renamed"])
def test_changed_product_tus_still_require_explicit_recipe_and_coverage_mapping(project, change):
    path = project / "code/owner.cc"
    if change == "added": (project / "code/new.cc").write_text("int new_unit() { return 1; }\n")
    elif change == "removed": path.unlink()
    else: path.rename(project / "code/new.cc")
    report = check(project, compiler=TreeCompiler(project))
    assert report["result"] == "unknown" and report["unsupported_file_set_changes"]
    assert report["compile_attempts"] == 0


def test_duplicate_outside_annotations_are_not_collapsed(project):
    header = project / "code/model.h"
    annotation = "// RECOVERY_MEMBER: v1 Model field 0x0 0x00401010\n"
    header.write_text(header.read_text() + annotation)
    git(project, "add", ".")
    git(project, "commit", "-m", "Synthetic member binding")
    original = TaskStore(project).read("owner")
    task = Task("duplicate-control", original.target, original.author, original.intent,
                git(project, "rev-parse", "HEAD"), original.subjects, original.editable_files, original.evidence)
    TaskStore(project).create(task)
    header.write_text(header.read_text() + annotation)
    report = check_preservation(project, task.task_id, compiler=Compiler(project))
    assert report["result"] == "fail" and len(report["outside_annotation_changes"]) == 1


@pytest.mark.parametrize("changed", [False, True])
def test_diagnostic_mode_never_claims_preservation_even_with_full_unit_coverage(project, changed):
    if changed:
        path = project / "code/owner.cc"
        path.write_text(path.read_text().replace("return 1", "return 2"))
    compiler = Compiler(project)
    report = check(project, compiler=compiler, diagnostic=True)
    assert report["result"] == "unknown" and report["diagnostic_only"]
    assert report["compile_attempts"] == (6 if changed else 4)
    assert all(row["result"] == "unknown" and row["compiled_comparison"] == "pass" and row["replica"] == "pass"
               for row in report["units"])
    assert report["controls"][0]["mode"] == "omitted_for_diagnostics"
    assert report["controls"][0]["omitted_file_controls"] == int(changed)
    assert not report["ready_for_collection"] and not report["persisted"]
    assert not any(b"#error" in data for _, _, overlay in compiler.calls if overlay for data in overlay.values())


def test_diagnostic_code_changes_point_to_current_and_historical_source_bindings(project):
    (project / "code/model.h").write_text("struct Model { int padding; int field; };\n")
    report = check(project, diagnostic=True, units=("code/caller.cc",))
    row, = report["units"]
    assert row["result"] == "unknown" and row["compiled_comparison"] == "fail" and row["changed_section_count"] == 1
    section, = row["changed_sections"]
    assert section["changed_dimensions"] == ["bytes"]
    for version in ("baseline", "current"):
        symbol, = section[version + "_symbols"]
        binding, = symbol["source_bindings"]
        assert symbol["symbol"] == "_h" and binding["source"] == "code/caller.cc"
        assert binding["kind"] == "function" and not binding["task_scope"]
    assert {value["version"] for value in row["affected_source_bindings"]} == {"baseline", "current"}


def test_diagnostic_cli_summary_keeps_actionable_bindings_and_explicit_unknown(project, monkeypatch, capsys):
    from match_recovery import preservation
    monkeypatch.setattr(preservation, "Compiler", lambda root, **kwargs: Compiler(root))
    (project / "code/model.h").write_text("struct Model { int padding; int field; };\n")
    assert main(["tasks", "check-preservation", "owner", "--root", str(project), "--unit", "code/caller.cc",
                 "--diagnostic", "--summary"]) == 3
    report = json.loads(capsys.readouterr().out)
    row, = report["units"]
    assert report["diagnostic_only"] and row["affected_source_bindings"]
    assert row["changed_section_count"] == 1 and row["compiled_comparison"] == "fail"
    assert "changed_sections" not in row and "symbol_changes" not in row


def test_diagnostic_still_requires_an_unchanged_current_replica(project):
    class Broken(Compiler):
        def overlay(self, *args): return coff_fixture(b"\xcc")
    report = check(project, compiler=Broken(project), diagnostic=True)
    assert report["result"] == "unknown" and all("compiled_comparison" not in row for row in report["units"])
    assert all("replica" in row["reason"] for row in report["units"])


def test_symbol_source_navigation_retains_ambiguity_and_distinguishes_table_identity_from_anchor():
    from dataclasses import asdict
    from match_recovery.preservation import _symbol_sources
    anchor = Origin("a" * 64, "pe:va", 0x401000)
    other = Origin("a" * 64, "pe:va", 0x401020)
    table = Origin("a" * 64, "pe:va", 0x402000)
    functions = {anchor.entity_id: [{"unit": "code/a.cc", "symbol": "_f"}],
                 other.entity_id: [{"unit": "code/b.cc", "symbol": "_f"}]}
    annotations = {"table": {"kind": "generated", "origin": asdict(table), "evidence": asdict(anchor),
                              "symbol": "_vtable", "path": "code/model.h"}}
    index = _symbol_sources(functions, annotations, {anchor.entity_id})
    assert [row["source"] for row in index["_f"]] == ["code/a.cc", "code/b.cc"]
    assert len(index["scoped-entry:" + anchor.entity_id]) == 1
    binding, = index["_vtable"]
    assert binding["entity_id"] == table.entity_id and binding["evidence_entity_id"] == anchor.entity_id
    assert not binding["task_scope"]  # A scoped constructor is not permission to change its table.


def test_unannotated_linker_symbols_remain_visible_without_invented_source_ownership():
    from match_recovery.coff_preservation import compare_outside_functions
    from match_recovery.pe_coff import Coff
    from match_recovery.preservation import _explain_sources
    raw = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20))
    report = compare_outside_functions(Coff(raw), Coff(raw.replace(b"_g", b"_h")), {}, {})
    _explain_sources(report, {}, {})
    assert not report["affected_source_bindings"]
    assert {(row["version"], row["symbol"]) for row in report["unbound_linker_symbols"]} == {
        ("baseline", "_f"), ("current", "_f"), ("baseline", "_g"), ("current", "_h")}
    assert report["result"] == "fail"
