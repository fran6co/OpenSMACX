import json
from pathlib import Path

import pytest

from match_recovery.cli import main
from match_recovery.cpp_model_audit import MODEL_RULES, model_findings
from match_recovery.source import read_text
from match_recovery.source_audits import audit_source, context_audits, numbered_code_lines


def inspect(files, seeds=None):
    records = [(path, record) for path, text in {**(seeds or {}), **files}.items() for record in read_text(text, path)]
    return model_findings(files, records, numbered_code_lines)


def test_original_locations_and_aggregated_name_occurrences():
    counts, per_file, findings = inspect({"code/unit.cc": "// a1 ignored\n\na1 = a1 + a2; // a3 ignored\n"})
    assert counts["scaffold name"] == 3
    assert per_file == {"code/unit.cc": {"scaffold name": 3}}
    assert findings == [{"rule": "scaffold name", "path": "code/unit.cc", "line": 3,
                         "basis": "source_heuristic", "occurrences": 3, "symbols": ["a1", "a2"]}]


def test_naming_refusals_use_exact_tokens_and_directory_local_families():
    counts, per_file, _ = inspect({
        "code/a/widget.hpp": "// EVIDENCE-FREE UNK1\nvoid UNK1(int a1);\n",
        "code/a/widget.cc": "void Widget::UNK1(int a1) {}\nvoid Widget::UNK10(int a1) {}\n",
        "code/b/widget.cc": "void Widget::UNK1(int a1) {}\n"})
    assert counts["evidence-free unk"] == 4
    assert counts["unk-method"] == counts["scaffold name"] == 2
    assert "evidence-free unk" not in per_file["code/b/widget.cc"]


def test_raw_self_access_distinguishes_a_member_pointer_cast():
    counts, _, _ = inspect({"code/owner.cc": """/* reinterpret_cast<int*>(this)
 * prose
 */
*reinterpret_cast<int*>(this) = 0;
Widget *p = (Widget *)this->other;
char *raw = (char *)this;
memcpy(destination, this, sizeof(Object));
int *p2 = reinterpret_cast<int*>(a1);
return reinterpret_cast<int>(this);
"""})
    assert counts["raw self-access"] == 4
    assert counts["pointer-parameter as int"] == counts["pointer-as-int"] == 1


def test_overlapping_seed_spans_do_not_hide_a_function_target():
    seed = """// ORIGINAL: 0x00401000 _long 0x00401000-0x00401100
void f() {}
// ORIGINAL: 0x00401050 _nested 0x00401050-0x00401060
void g() {}
"""
    counts, _, findings = inspect({"code/unit.cc": "int *a = (int *)0x00401095;\nint *b = (int *)0x00401100;\n"},
                                  {"seeds/original.cc": seed})
    assert counts["function-address binding"] == 1
    assert findings[0]["address"] == 0x00401095


def test_orphans_are_unique_names_but_locations_are_mentions():
    counts, per_file, findings = inspect({"code/owner.hpp": "void dead_redirect();\nvoid live_redirect();\n",
                                        "code/owner.cc": "void dead_redirect() {}\nvoid live_redirect() {}\n",
                                        "code/caller.cc": "live_redirect();\n"})
    assert counts["orphan redirect"] == 1
    assert per_file["code/owner.hpp"]["orphan redirect"] == per_file["code/owner.cc"]["orphan redirect"] == 1
    assert all(item["symbols"] == ["dead_redirect"] for item in findings)


def test_short_claim_documentation_is_a_hint_not_matching_measurement():
    code = "// ORIGINAL: 0x00401000 _trivial 0x00401000-0x00401001 BYTE_EXACT\nvoid f() {}\n"
    counts, _, findings = inspect({"code/owner.cc": code})
    assert counts["undocumented trivial body"] == 1 and findings[0]["line"] == 1
    assert inspect({"code/owner.cc": "// Purpose: family of compiler thunks\n" + code})[0]["undocumented trivial body"] == 0
    assert inspect({}, {"seeds/owner.cc": code})[0]["undocumented trivial body"] == 0


@pytest.fixture
def model_project(tmp_path):
    (tmp_path / "recovery").mkdir()
    (tmp_path / "code/seeds").mkdir(parents=True)
    (tmp_path / "code/owner.cc").write_text("void live_redirect() {}\nvoid dead_redirect() {}\n")
    (tmp_path / "code/owner.hpp").write_text("void live_redirect();\nvoid dead_redirect();\n")
    (tmp_path / "code/caller.cc").write_text("live_redirect();\n")
    (tmp_path / "code/seeds/a.cc").write_text("int a1;\n")
    (tmp_path / "recovery/project.json").write_text(json.dumps({"schema_version": 1,
        "source": {"image_pin": "code/PIN", "address_space": "test:offset", "roots": ["code"],
                   "extensions": [".cc", ".hpp"], "seed_roots": ["code/seeds"]},
        "audits": "recovery/policy.json"}))
    (tmp_path / "recovery/policy.json").write_text(json.dumps({"schema_version": 1, "context": ["model"],
        "audits": {"model": {"ruleset": "cpp-model-v1", "unit_extensions": [".cc"], "interface_extensions": [".hpp"],
                             "exclude_roots": [], "ceilings": {**dict.fromkeys(MODEL_RULES, 0), "orphan redirect": 1}, "counters": []}}}))
    return tmp_path


def test_native_detail_selection_keeps_whole_product_reference_analysis(model_project, capsys):
    assert main(["audits", "check", "model", "--root", str(model_project), "--in", "code/owner.cc"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["total"] == 1 and report["per_file"] == {"code/owner.cc": {"orphan redirect": 1}}
    assert report["findings"][0]["symbols"] == ["dead_redirect"]
    assert report["detail_selection"] == {"code/owner.cc": "scanned"}
    assert audit_source(model_project, "model", files=["code/seeds/a.cc"])["detail_selection"] == {"code/seeds/a.cc": "excluded"}
    (model_project / "code/caller.cc").write_text("live_redirect(); int a1;\n")
    report = audit_source(model_project, "model", files=["code/owner.cc"])
    assert report["result"] == "fail"  # An off-selection policy failure is not hidden.


@pytest.mark.parametrize("selected", [["missing.cc"], ["../outside.cc"], []])
def test_invalid_detail_selection_refused(model_project, selected):
    with pytest.raises(ValueError):
        audit_source(model_project, "model", files=selected)


def test_context_keeps_source_locations_and_names_are_not_deletion_permission(model_project):
    hints, = context_audits(model_project, {"code/owner.cc"})
    assert hints["scope"] == "authorized files" and hints["project_policy_result"] == "pass"
    assert hints["structure_verification"] == "unknown"
    assert hints["findings"][0]["path"] == "code/owner.cc"
    assert "reachability" in hints["explanations"]["orphan redirect"]
    assert hints["finding_count"] == 1 and hints["omitted_findings"] == 0


def test_context_bounds_naming_volume_but_prioritizes_structure(model_project):
    (model_project / "code/owner.cc").write_text("int a1;\n" * 100 + "int *p = reinterpret_cast<int*>(this);\n")
    hints, = context_audits(model_project, {"code/owner.cc"})
    assert hints["project_policy_result"] == "fail"
    assert hints["finding_count"] == 101 and hints["omitted_findings"] == 37
    assert len(hints["findings"]) == 64 and hints["findings"][0]["rule"] == "raw self-access"


@pytest.mark.parametrize("policy", [{}, {"schema_version": 1, "audits": {}, "context": ["missing"]},
                                     {"schema_version": True, "audits": {}},
                                     {"schema_version": 1, "audits": {}, "context": "model"}])
def test_missing_context_policy_is_not_replaced_with_empty_hints(model_project, policy):
    (model_project / "recovery/policy.json").write_text(json.dumps(policy))
    with pytest.raises(ValueError):
        context_audits(model_project, {"code/owner.cc"})
