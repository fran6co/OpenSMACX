import json

import pytest

from match_recovery.cli import main
from match_recovery.cpp_storage_audit import STORAGE_RULES, storage_findings, scaled_sums
from match_recovery.source_audits import audit_source, context_audits
from test_source_audits import project, modify_policy


PIN = "a" * 64


def findings(files, format="none"):
    return storage_findings(files, pin=PIN, address_space="custom:address", symbol_format=format)


def storage_project(root):
    (root / "code/PIN").write_text("sha256 " + PIN + "\n")
    (root / "code/a.cc").write_text("static int *const Alias = (int *)0xAB001234;\n")
    (root / "code/product/a.cc").write_text("int Actual; // 0xAB001234\n")
    (root / "code/a.hpp").write_text("extern int Actual; // 0xAB001234\n")
    modify_policy(root, lambda c: c.update(ruleset="cpp-storage-v1", counters=[],
                                          ceilings={"duplicate global identities": 1, "scaled class-pointer sum": 0}))
    return root


def test_native_storage_audit_is_readonly_and_retains_cross_file_context(project, capsys):
    storage_project(project)
    before = {p: p.read_bytes() for p in project.rglob("*") if p.is_file()}
    assert main(["audits", "check", "quality", "--root", str(project)]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["total"] == 1 and report["files_scanned"] == 3
    assert {f["path"] for f in report["findings"]} == {"code/a.cc", "code/a.hpp", "code/product/a.cc"}
    assert all(f["symbols"] == ["Actual", "Alias"] and len(f["related"]) == 3 for f in report["findings"])
    assert report["structure_verification"] == "unknown" and not report["persisted"]
    assert before == {p: p.read_bytes() for p in project.rglob("*") if p.is_file()}


@pytest.mark.parametrize("ceiling,status", [(0, "growth"), (2, "slack")])
def test_storage_growth_and_slack_both_fail(project, ceiling, status):
    storage_project(project)
    modify_policy(project, lambda c: c["ceilings"].update({"duplicate global identities": ceiling}))
    result = audit_source(project, "quality")
    assert result["result"] == "fail" and result["rules"][0]["status"] == status


def test_seed_and_dependency_hints_do_not_create_product_storage(project):
    storage_project(project)
    for name in ("code/seed/nested/a.cc", "code/dependency/a.cc"):
        (project / name).write_text("int Separate; // 0xAB001234\n")
    assert all("Separate" not in f["symbols"] for f in audit_source(project, "quality")["findings"])


def test_detail_selection_never_narrows_conflict_detection(project):
    storage_project(project)
    report = audit_source(project, "quality", files=["code/a.cc"])
    assert report["total"] == 1 and len(report["findings"]) == 1
    assert {r["path"] for r in report["findings"][0]["related"]} == {"code/a.cc", "code/a.hpp", "code/product/a.cc"}


def test_structural_context_receives_related_storage_locations(project):
    storage_project(project)
    path = project / "recovery/quality.json"
    value = json.loads(path.read_text())
    value["context"] = ["quality"]
    path.write_text(json.dumps(value))
    report, = context_audits(project, ["code/a.cc"])
    assert report["finding_count"] == 1 and len(report["findings"][0]["related"]) == 3


@pytest.mark.parametrize("format,symbol", [("none", "Global"), ("msvc", "?Global@@3HA")])
def test_compact_objects_and_legacy_aliases_share_identity(format, symbol):
    counts, _, rows = findings({"nested/unit.cc": f"// RECOVERY_OBJECT: v1 0x12345678 {symbol}\nint Global;\n",
                               "other.hpp": "static int *const Alias = (int *)0x12345678;\n"}, format)
    assert counts["duplicate global identities"] == 1
    assert rows[0]["symbols"] == ["Alias", "Global"]


def test_same_global_declaration_and_definition_are_not_a_conflict():
    counts, _, rows = findings({"a.cc": "int Global; // 0x12345678\n", "a.hpp": "extern int Global; // 0x12345678\n"})
    assert sum(counts.values()) == 0 and rows == []


@pytest.mark.parametrize("padding", ["", "00"])
def test_first_member_and_whole_object_are_not_two_globals(padding):
    counts, _, rows = findings({"scene.cc":
        f"struct Scene {{\n  int first; // 0x{padding}9BB438\n}};\nScene Owner; // 0x009BB438\n"})
    assert sum(counts.values()) == 0 and rows == []


def test_class_scopes_ignore_comment_braces_and_forward_declarations():
    counts, _, _ = findings({"scene.cc": "struct Scene;\nstruct Scene {\n// }\nint first; // 0x12345678\n};\n"
                           "int Owner; // 0x12345678\nint Other; // 0x12345678\n"})
    assert counts["duplicate global identities"] == 1


def test_elaborated_return_type_does_not_hide_function_local_bindings():
    counts, _, _ = findings({"scene.cc": "struct Scene *get() {\nstatic int *const Alias = (int *)0x12345678;\n}\n"
                           "int Other; // 0x12345678\n"})
    assert counts["duplicate global identities"] == 1


@pytest.mark.parametrize("source", ["// RECOVERY_OBJECT: v9 0x12345678 Global\n",
                                   "// RECOVERY_OBJECT: v1 0x12345678 Global\n// RECOVERY_OBJECT: v1 0x12345678 Other\n",
                                   "struct Scene {\n"])
def test_invalid_object_or_scope_evidence_is_refused(source):
    with pytest.raises(ValueError):
        findings({"a.cc": source})


def test_scaled_pointer_check_is_function_scoped_and_reports_actual_line():
    text = ("void f(Window *parent) {\n /* comment\n more */\n"
            "  int x = *reinterpret_cast<int *>(parent + 0xcc);\n}\n"
            "void g(char *parent) {\n  int x = *reinterpret_cast<int *>(parent + 0xcc);\n}\n")
    rows = list(scaled_sums(text))
    assert len(rows) == 1 and rows[0][0] == 4 and rows[0][2] == "parent"


def test_cast_before_arithmetic_and_comment_examples_are_not_reported():
    text = ("void f(Window *parent) {\n int x = *((char *)parent + 0xcc);\n"
            "// reinterpret_cast<int *>(parent + 0xcc)\n/* reinterpret_cast<int *>(parent + 0xcc) */\n}\n")
    assert not list(scaled_sums(text))


def test_changed_pin_is_part_of_storage_input_identity(project):
    storage_project(project)
    before = audit_source(project, "quality")
    (project / "code/PIN").write_text("sha256 " + "b" * 64 + "\n")
    assert audit_source(project, "quality")["input_fingerprint"] != before["input_fingerprint"]
    (project / "code/PIN").unlink()
    with pytest.raises(OSError):
        audit_source(project, "quality")


def test_empty_storage_scope_cannot_pass_with_zero_ceilings(project):
    storage_project(project)
    modify_policy(project, lambda c: c.update(exclude_roots=["code"],
                                             ceilings={name: 0 for name in STORAGE_RULES}))
    with pytest.raises(ValueError, match="no product source coverage"):
        audit_source(project, "quality")


def test_pin_change_during_storage_census_is_refused(project, monkeypatch):
    import match_recovery.source_audits as module
    storage_project(project)
    original = module.storage_findings
    def changing(*args, **kwargs):
        result = original(*args, **kwargs)
        (project / "code/PIN").write_text("sha256 " + "b" * 64 + "\n")
        return result
    monkeypatch.setattr(module, "storage_findings", changing)
    with pytest.raises(ValueError, match="changed during the census"):
        audit_source(project, "quality")
