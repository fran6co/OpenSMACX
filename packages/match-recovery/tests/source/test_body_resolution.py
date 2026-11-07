from pathlib import Path

import pytest

from match_recovery.source.body import resolve_body
from match_recovery.source.reader import read_text


MARKER = ("// ORIGINAL: 0x00401000 ?f@Widget@@QAEHH@Z "
          "0x00401000-0x00401010\n// body src/widget.h\n")


def test_external_body_fact_never_claims_following_marker_tu_code():
    source = ("/*\n// ORIGINAL: 0x00401000 ?f@Widget@@QAEHH@Z "
              "0x00401000-0x00401010\n// body src/widget.h\n"
              "Prose: f() { return 0; }\n*/\n"
              "void unrelated() { f(); }\n")
    row, = read_text(source, Path("src/owner.cpp"))
    assert row.body == "src/widget.h"
    assert row.region == ""
    assert row.extract_error == ""


def test_external_body_reader_does_not_open_declared_file(monkeypatch):
    def forbidden(*_args, **_kwargs):
        raise AssertionError("read_text must not read an external body path")

    monkeypatch.setattr(Path, "read_text", forbidden)
    row, = read_text(MARKER + "void unrelated() { return; }\n",
                     Path("src/owner.cpp"))
    assert row.body == "src/widget.h"
    assert row.region == ""
    assert row.extract_error == ""


@pytest.mark.parametrize('host', ['src/owner.cpp', '/project/src/nested/src/owner.cpp'])
def test_explicit_body_fact_never_guesses_same_file_from_path_suffix(host):
    source = ("// ORIGINAL: 0x00401000 ?f@@YAHXZ 0x00401000-0x00401010\n"
              "// body src/owner.cpp\nint f() { return 1; }\n")
    row, = read_text(source, Path(host))
    assert row.body == "src/owner.cpp"
    assert not row.region
    assert not row.extract_error


def mapping(mangled):
    text = f"// ORIGINAL: 0x00401000 {mangled} 0x00401000-0x00401010\n// body src/widget.h\n"
    return read_text(text, Path("src/owner.cpp"))[0]


def test_snapshot_resolver_returns_only_a_lexical_candidate():
    row = mapping("?f@Widget@@QAEHH@Z")
    header = "int Widget::f(int value) {\n  return value + 1;\n}\n"
    result = resolve_body(row, marker_path="src/owner.cpp",
                          source_texts={"src/widget.h": header})
    assert result.status == "candidate"
    assert result.path == "src/widget.h"
    assert result.line == 1 and result.end_line == 3
    assert result.body == "int Widget::f(int value) {\n  return value + 1;\n}"
    assert "unproven" in result.identity_evidence
    assert "unproven" in result.context_evidence


@pytest.mark.parametrize("header", [
    "int Other::f(int value) { return value; }\n",
    "int Widget::f(int value) { return value; }\n"
    "int Widget::f(char value) { return value; }\n",
])
def test_snapshot_resolver_fails_closed_for_wrong_scope_or_overloads(header):
    result = resolve_body(mapping("?f@Widget@@QAEHH@Z"),
                          marker_path="src/owner.cpp",
                          source_texts={"src/widget.h": header})
    assert result.status == "unresolved"
    assert result.body == ""
    assert result.identity_evidence == "unproven"


@pytest.mark.parametrize("header", [
    "namespace N { int authored() { return 0; } }\n",
    "class C { int authored() { return 0; } };\n",
])
def test_unqualified_global_symbol_does_not_resolve_inside_namespace_or_class(header):
    row = mapping("?authored@@YAHXZ")
    result = resolve_body(row, marker_path="src/owner.cpp",
                          source_texts={"src/widget.h": header})
    assert result.status == "unresolved"
    assert "nested" in result.reason


def test_snapshot_resolver_refuses_missing_body_path_and_unsupported_name():
    row = mapping("?f@Widget@@QAEHH@Z")
    missing = resolve_body(row, marker_path="src/owner.cpp", source_texts={})
    assert missing.status == "unresolved"
    assert "absent from the supplied source snapshot" in missing.reason

    unsupported = resolve_body(mapping("??_7Widget@@6B@"),
                               marker_path="src/owner.cpp",
                               source_texts={"src/widget.h": "int f() { return 0; }\n"})
    assert unsupported.status == "unresolved"
    assert "no supported source spelling" in unsupported.reason


def test_snapshot_resolver_never_reopens_a_single_line_snapshot(monkeypatch):
    row = mapping("?authored@@YAHXZ")

    def forbidden(*_args, **_kwargs):
        raise AssertionError("resolver must use supplied snapshot text only")

    monkeypatch.setattr(Path, "read_text", forbidden)
    result = resolve_body(row, marker_path="src/owner.cpp",
                          source_texts={"src/widget.h": "int authored() { return 0; }"},
                          symbol_format="none")
    assert result.status == "candidate"
    assert result.line == 1


def test_in_class_and_implicit_member_bodies_resolve_without_compiler_claims():
    header = ("class Widget : public Base {\n"
              " public:\n"
              "  ~Widget() { close(); }\n"
              "  void close();\n"
              "};\n"
              "struct Plain {\n"
              "  int value_;\n"
              "};\n")
    texts = {"src/widget.h": header}
    found = resolve_body(mapping("??1Widget@@UAE@XZ"), marker_path="src/owner.cpp",
                         source_texts=texts)
    assert (found.status, found.form, found.line, found.body) == (
        "candidate", "in_class", 3, "  ~Widget() { close(); }")
    declared = resolve_body(mapping("?close@Widget@@QAEXXZ"), marker_path="src/owner.cpp",
                            source_texts=texts)
    assert declared.status == "unresolved" and "without an in-class definition" in declared.reason
    implicit = resolve_body(mapping("??1Plain@@QAE@XZ"), marker_path="src/owner.cpp",
                            source_texts=texts)
    assert (implicit.status, implicit.form, implicit.line, implicit.end_line, implicit.body) == (
        "implicit", "implicit", 6, 8, "")
    absent = resolve_body(mapping("??1Missing@@QAE@XZ"), marker_path="src/owner.cpp",
                          source_texts=texts)
    assert absent.status == "unresolved"
