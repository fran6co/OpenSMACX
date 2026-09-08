"""Conventional source hints use compiler offsets, never guessed scalar sizes."""

import pytest

from match_recovery.cli import main
from match_recovery.conventional_layout import parse_offsets
from match_recovery.layout_check import check_layout
from test_codeview import coff, debug_types
from test_symbol_check import project
from test_layout_check import add_unit


def test_offset_comments_and_names_are_independent_hints():
    rows, errors = parse_offsets("""class Widget {
  CustomWord field_4_; // 0x8
  SomeType *next_; // (+12)
  AnotherType values_[2 * (3 + 1)]; // 0x10
};
""", "code/types.h")
    assert not errors
    assert [(r.member, r.offset, r.basis) for r in rows] == [
        ("field_4_", 8, "trailing_offset_comment"), ("field_4_", 4, "self_naming_member"),
        ("next_", 12, "trailing_offset_comment"), ("values_", 16, "trailing_offset_comment")]
    assert rows[0].line == 2
    assert rows[0].interpretation_id != rows[1].interpretation_id


def test_namespaces_nested_classes_and_packing_need_no_layout_walk():
    rows, errors = parse_offsets("""namespace Engine {
#pragma pack(push, 1)
struct Widget {
  char head_;
  int field_1_;
  class Inner {
    int field_0_;
  };
};
#pragma pack(pop)
}
""", "code/types.h")
    assert not errors
    assert [(r.class_name, r.member, r.offset) for r in rows] == [
        ("Engine::Widget", "field_1_", 1), ("Engine::Widget::Inner", "field_0_", 0)]


def test_comments_methods_parameters_and_local_fields_are_not_members():
    rows, errors = parse_offsets("""/* class Fake {
  int field_4_;
}; */
class Widget {
  void method(); // 0x00401000
  void multiline(int x,
                 int y); // 0x00402000
  int value() { return field_4_; }
  void body() {
    int field_8_;
    const char *text = "struct Fake { int field_0_; }";
  }
  int field_4_;
};
""", "code/types.h")
    assert not errors
    assert [(r.class_name, r.member) for r in rows] == [("Widget", "field_4_")]


@pytest.mark.parametrize("declaration", ["int first_, second_; // 0x4",
    "int (*field_4_)(int);", "int ( *field_4_)(int);", "int slot_ = 1; // 0x4", "unsigned field_4_: 1;"])
def test_unsupported_hinted_declaration_is_reported(declaration):
    _, errors = parse_offsets("class Widget {\n  " + declaration + "\n};", "code/types.h")
    assert errors and errors[0]["line"] == 2


def test_unbalanced_source_cannot_silently_pass():
    _, errors = parse_offsets("class Widget {\n  int field_4_;\n", "code/types.h")
    assert errors


@pytest.mark.parametrize("text", ["class Widget { int field_4_; };",
    "template<class T>\nclass Widget {\n int field_4_;\n};",
    "namespace {\nclass Widget {\n int field_4_;\n};\n}",
    "union Widget {\n int field_4_;\n};"])
def test_unsupported_scopes_are_not_silently_complete(text):
    _, errors = parse_offsets(text, "code/types.h")
    assert errors


@pytest.fixture
def conventional_project(project):
    (project / "code/types.h").write_text("class Widget {\n  ArbitraryType slot_; // 0x4\n};\n")
    return project


def test_compiler_observations_check_hints_without_fabricating_original_evidence(conventional_project):
    before = (conventional_project / "code/types.h").read_bytes()
    result = check_layout(conventional_project, conventional_offsets=True, compiler=lambda *_: coff())
    assert result["result"] == "pass" and result["counts"] == {"pass": 1}
    assert result["expectations"][0]["kind"] == "conventional_member"
    assert result["expectations"][0]["observed_definitions"] == 2
    assert "evidence" not in result["expectations"][0]["expectation"]
    assert result["observations"] == []  # good repeated header observations are aggregated
    assert not result["ready_for_collection"] and result["structure_verification"] == "unknown"
    assert (conventional_project / "code/types.h").read_bytes() == before
    with pytest.raises(ValueError, match="no source-owned"):
        check_layout(conventional_project, compiler=lambda *_: coff())


@pytest.mark.parametrize("damage", ["later_unit", "later_flags"])
def test_one_matching_definition_cannot_hide_a_wrong_offset(conventional_project, damage):
    add_unit(conventional_project)
    def compiler(path, _, flags):
        bad = path.name == "g.cc" if damage == "later_unit" else flags == ("second",)
        return coff(debug_types(8 if bad else 4))
    result = check_layout(conventional_project, conventional_offsets=True, compiler=compiler)
    assert result["result"] == "fail" and result["counts"] == {"fail": 1}
    assert len(result["observations"]) == 2
    assert all(row["actual_offset"] == 8 for row in result["observations"])


@pytest.mark.parametrize("damage", ["absent_class", "missing_member", "missing_flags", "bad_object", "bad_hint"])
def test_unavailable_conventional_evidence_stays_unknown(conventional_project, damage):
    if damage == "bad_hint":
        (conventional_project / "code/types.h").write_text(
            "class Widget {\n  int slot_; // 0x4\n  int a_, b_; // 0x8\n};\n")
    def compiler(path, _, flags):
        if damage == "bad_object": return b"broken"
        if damage == "absent_class" or damage == "missing_flags" and flags == ("second",):
            return coff(debug_types().replace(b"Widget", b"OtherX"))
        if damage == "missing_member": return coff(debug_types().replace(b"slot_", b"other"))
        return coff()
    result = check_layout(conventional_project, conventional_offsets=True, compiler=compiler)
    assert result["result"] == "unknown"


def test_explicit_requirements_and_conventional_hints_share_one_compilation(conventional_project):
    path = conventional_project / "code/types.h"
    path.write_text(path.read_text() + "// RECOVERY_MEMBER: v1 Widget slot_ 0x8 0x00401000\n")
    result = check_layout(conventional_project, conventional_offsets=True, compiler=lambda *_: coff())
    assert result["compile_attempts"] == 2
    assert result["result"] == "fail" and result["counts"] == {"fail": 1, "pass": 1}


def test_seeds_and_noninterface_files_are_not_product_header_requirements(conventional_project):
    (conventional_project / "code/seeds").mkdir()
    for name in ("code/seeds/types.h", "code/unrelated.cc"):
        (conventional_project / name).write_text("class Widget {\n  int slot_; // 0x8\n};")
    result = check_layout(conventional_project, conventional_offsets=True, compiler=lambda *_: coff())
    assert result["result"] == "pass" and result["counts"] == {"pass": 1}


def test_cli_keeps_conventional_failure_details(conventional_project, monkeypatch, capsys):
    import json
    import match_recovery.layout_check as module
    monkeypatch.setattr(module, "Compiler", lambda *_, **__: lambda *_: coff(debug_types(8)))
    assert main(["check-layout", "--root", str(conventional_project), "--conventional-offsets", "--summary"]) == 1
    report = json.loads(capsys.readouterr().out)
    assert report["conventional_offsets"]["enabled"]
    assert report["expectations"] and report["observations"]
