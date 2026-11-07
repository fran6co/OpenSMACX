"""Causal source-body attribution controls using synthetic COFF only."""

import base64
from dataclasses import replace
import re
from types import SimpleNamespace

import pytest

from match_recovery.body_attribution import (BodyAttribution, _definition_edit,
                                             _renamed_symbol)
from match_recovery.pe_coff import Coff
from test_function_witness import object_bytes


UNIT = "src/owner.cpp"
HEADER = "src/model.h"
TARGET = "_sample"
BASE = object_bytes([(TARGET, b"\xc3", ()), ("_other", b"\xc3", ())])


def _snapshot(root, header, *, owners=1):
    (root / "src").mkdir(exist_ok=True)
    marker_rows = []
    records = []
    for index in range(owners):
        line = index * 2 + 1
        address = 0x401000 + index * 0x10
        marker_rows.extend((
            f"// ORIGINAL: 0x{address:08X} {TARGET} 0x{address:08X}-0x{address + 6:08X}\n",
            f"// body {HEADER}\n",
        ))
        records.append({"entity_id": f"entity-{index}", "path": UNIT,
                        "line": line, "body_path": HEADER, "context_path": UNIT})
    unit_text = "".join(marker_rows)
    (root / UNIT).write_text(unit_text)
    (root / HEADER).write_text(header)
    files = {}
    for path, content in ((UNIT, unit_text.encode()), (HEADER, header.encode())):
        files[path] = {"content_base64": base64.b64encode(content).decode("ascii")}
    return {"files": files, "source_configuration": {"symbol_format": "none"},
            "records": records}


class FakeOverlay:
    def __init__(self, root, snapshot, *, mode="success"):
        self.root, self.snapshot, self.mode = root, snapshot, mode
        self.calls = []
        self.original_header = base64.b64decode(snapshot["files"][HEADER]["content_base64"])

    def overlay(self, source, replacements, arguments, flags):
        self.calls.append((source, dict(replacements), arguments, flags))
        header = replacements[HEADER]
        if header == self.original_header:
            if self.mode == "unchanged-drift":
                return object_bytes([(TARGET, b"\xcc", ()), ("_other", b"\xc3", ())])
            return BASE
        if self.mode == "compile-error":
            raise ValueError("synthetic overlay compile failure")
        text = header.decode("utf-8")
        probe_match = re.search(r"match_recovery_body_probe_[0-9a-f]{24}", text)
        if probe_match is None:
            return object_bytes([("_other", b"\xc3", ())])
        renamed = "_" + probe_match.group(0)
        functions = [("_other", b"\xc3", ())]
        if self.mode in {"wrong-namespace", "wrong-overload", "inline-wrapper-retains-target"}:
            functions.append((TARGET, b"\xc3", ()))
        if self.mode != "no-renamed-definition":
            functions.append((renamed, b"\xc3", ()))
        return object_bytes(functions)


def _qualify(root, snapshot, compiler=None, *, header=None, record_index=0):
    attribution = BodyAttribution(root, snapshot)
    record = attribution.record(f"entity-{record_index}")
    result = attribution.qualify(record, Coff(BASE), unit=UNIT,
                                 arguments=("configured-argument",),
                                 flags=("configured-flag",),
                                 compiler=compiler or FakeOverlay(root, snapshot))
    return attribution, result


def test_definition_rename_preserves_msvc_abi_suffix_and_signature_arity():
    symbol = "?sample@@YAHH@Z"
    renamed = _renamed_symbol(symbol, "sample", "match_recovery_body_probe_0123456789abcdef01234567")
    assert renamed == "?match_recovery_body_probe_0123456789abcdef01234567@@YAHH@Z"
    assert renamed[len("?match_recovery_body_probe_0123456789abcdef01234567@@"):] == "YAHH@Z"
    assert _renamed_symbol("_sample@8", "sample", "probe") == "_probe@8"
    with pytest.raises(ValueError, match="ordinary free-function"):
        _renamed_symbol("?sample@Widget@@QAEHXZ", "Widget::sample", "probe")


def test_definition_edit_balances_multiline_default_argument_lambda_braces():
    source = ("inline int sample(\n"
              "    int value = []() { return 3; }()\n"
              ") {\n"
              "    return value;\n"
              "}\n")
    candidate = SimpleNamespace(line=1, end_line=5)
    start, end, edited = _definition_edit(source, candidate, "sample", "fresh_probe")
    assert start == 0 and end == len(source)
    assert edited.startswith("inline int sample(\n    int value = []() { return 3; }()\n);\n")
    assert "inline int fresh_probe(\n    int value = []() { return 3; }()\n) {" in edited


def test_exact_body_rename_qualifies_with_unchanged_overlay_control(tmp_path):
    header = "inline int sample() { return 1; }\n"
    snapshot = _snapshot(tmp_path, header)
    compiler = FakeOverlay(tmp_path, snapshot)
    attribution, result = _qualify(tmp_path, snapshot, compiler)
    assert result["result"] == "pass"
    assert result["path"] == HEADER and result["symbol"] == TARGET
    assert result["evidence"].startswith("unchanged-overlay equivalence")
    assert attribution.compile_attempts == 2 and len(compiler.calls) == 2
    assert compiler.calls[0][1][HEADER] == header.encode()
    assert compiler.calls[1][1][HEADER] != header.encode()
    assert compiler.calls[0][2:] == (("configured-argument",), ("configured-flag",))


def test_requested_body_cannot_borrow_a_different_snapshot_mapping_probe(tmp_path):
    header = 'inline int sample() { return 1; }\n'
    snapshot = _snapshot(tmp_path, header)
    snapshot['files']['src/other.h'] = {'content_base64': base64.b64encode(header.encode()).decode()}
    attribution = BodyAttribution(tmp_path, snapshot)
    record = replace(attribution.record('entity-0'), body='src/other.h')
    with pytest.raises(ValueError, match='differs from its captured source mapping'):
        attribution.qualify(record, Coff(BASE), unit=UNIT, arguments=(), flags=(),
                            compiler=FakeOverlay(tmp_path, snapshot))


@pytest.mark.parametrize("mode", ["wrong-namespace", "wrong-overload"])
def test_wrong_namespace_or_overload_keeps_mapped_definition_and_refuses(tmp_path, mode):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n")
    compiler = FakeOverlay(tmp_path, snapshot, mode=mode)
    with pytest.raises(ValueError, match="did not remove the selected function definition"):
        _qualify(tmp_path, snapshot, compiler)


def test_inline_wrapper_false_positive_is_rejected_when_exact_root_remains(tmp_path):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n")
    compiler = FakeOverlay(tmp_path, snapshot, mode="inline-wrapper-retains-target")
    with pytest.raises(ValueError, match="did not remove the selected function definition"):
        _qualify(tmp_path, snapshot, compiler)


def test_unchanged_overlay_object_drift_refuses_before_definition_edit(tmp_path):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n")
    compiler = FakeOverlay(tmp_path, snapshot, mode="unchanged-drift")
    with pytest.raises(ValueError, match="unchanged source overlay differs"):
        _qualify(tmp_path, snapshot, compiler)
    assert len(compiler.calls) == 1


def test_unsupported_overlay_client_and_placeholder_refuse(tmp_path):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n")
    class NoOverlay:
        pass
    with pytest.raises(ValueError, match="no source-overlay capability"):
        _qualify(tmp_path, snapshot, NoOverlay())

    placeholder = _snapshot(tmp_path, "int sample() {\n    // BODY GOES HERE.\n}\n")
    with pytest.raises(ValueError, match="placeholder"):
        _qualify(tmp_path, placeholder)


def test_overlapping_same_definition_owners_refuse_without_compiling(tmp_path):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n", owners=2)
    compiler = FakeOverlay(tmp_path, snapshot)
    with pytest.raises(ValueError, match="ranges overlap"):
        _qualify(tmp_path, snapshot, compiler)
    assert not compiler.calls


def test_renamed_definition_must_be_present_in_overlay(tmp_path):
    # Only an inline definition may lack its emitted probe; see the removal
    # control below. An ordinary definition must emit the renamed body.
    snapshot = _snapshot(tmp_path, "int sample() { return 1; }\n")
    compiler = FakeOverlay(tmp_path, snapshot, mode="no-renamed-definition")
    with pytest.raises(ValueError, match="expected one defined COFF function"):
        _qualify(tmp_path, snapshot, compiler)


# In-class member definitions: the ``body`` header pointer's own form. A
# destructor cannot be renamed, so the control removes its body instead.
MEMBER = "??1Widget@@QAE@XZ"
MEMBER_HEADER = ("class Widget {\n"
                 " public:\n"
                 "  void close();\n"
                 "  ~Widget() { close(); }\n"
                 "};\n")
MEMBER_BASE = object_bytes([(MEMBER, b"\xc3", ()), ("_other", b"\xc3", ())],
                           undefined=("?close@Widget@@QAEXXZ",))


def _member_snapshot(root, header):
    snapshot = _snapshot(root, header)
    unit_text = (f"// ORIGINAL: 0x00401000 {MEMBER} 0x00401000-0x00401006\n"
                 f"// body {HEADER}\n")
    (root / UNIT).write_text(unit_text)
    snapshot["files"][UNIT] = {"content_base64": base64.b64encode(unit_text.encode()).decode()}
    snapshot["source_configuration"]["symbol_format"] = "msvc"
    return snapshot


class MemberOverlay:
    def __init__(self, header, *, retain=False):
        self.header, self.retain, self.calls = header.encode(), retain, []

    def overlay(self, source, replacements, arguments, flags):
        self.calls.append(dict(replacements))
        if replacements[HEADER] == self.header:
            return MEMBER_BASE
        functions = [("_other", b"\xc3", ())]
        if self.retain:
            functions.append((MEMBER, b"\xc3", ()))
        return object_bytes(functions, undefined=(MEMBER,))


def _qualify_member(root, snapshot, compiler):
    attribution = BodyAttribution(root, snapshot)
    record = attribution.record("entity-0")
    return attribution, attribution.qualify(record, Coff(MEMBER_BASE), unit=UNIT, arguments=(),
                                            flags=(), compiler=compiler)


def test_in_class_destructor_resolves_and_qualifies_by_body_removal(tmp_path):
    snapshot = _member_snapshot(tmp_path, MEMBER_HEADER)
    compiler = MemberOverlay(MEMBER_HEADER)
    attribution, result = _qualify_member(tmp_path, snapshot, compiler)
    assert result["result"] == "pass" and result["line"] == 4 and result["end_line"] == 4
    assert result["evidence"].endswith("in-class body removal/disappearance")
    assert attribution.compile_attempts == 2
    edited = compiler.calls[1][HEADER].decode()
    assert "  ~Widget();\n" in edited and "void close();" in edited
    assert edited.replace("  ~Widget();\n", "  ~Widget() { close(); }\n") == MEMBER_HEADER


def test_in_class_removal_that_keeps_the_definition_refuses(tmp_path):
    snapshot = _member_snapshot(tmp_path, MEMBER_HEADER)
    with pytest.raises(ValueError, match="removal did not remove the selected function definition"):
        _qualify_member(tmp_path, snapshot, MemberOverlay(MEMBER_HEADER, retain=True))


@pytest.mark.parametrize("header, reason", [
    # A user-declared destructor whose definition is missing.
    ("class Widget {\n public:\n  ~Widget();\n};\n", "declared in its class without"),
    # Two definitions of the owner class are ambiguous.
    ("class Widget {\n  ~Widget() {}\n};\nclass Widget {\n  ~Widget() {}\n};\n",
     "2 class definitions found"),
    # A nested same-named class is not the global owner.
    ("struct Outer {\n  class Widget {\n    ~Widget() {}\n  };\n};\n", "0 definitions found"),
])
def test_in_class_resolution_refusals(tmp_path, header, reason):
    snapshot = _member_snapshot(tmp_path, header)
    with pytest.raises(ValueError, match=reason):
        _qualify_member(tmp_path, snapshot, MemberOverlay(header))


def test_in_class_ordinary_member_requires_one_member_level_occurrence():
    from match_recovery.source.body import _in_class_definition
    text = ("struct Widget : public Base {\n"
            "  int get() const { return value_; }\n"
            "  int get(int index) { return index; }\n"
            "  void close() {\n"
            "    if (value_) { value_ = 0; }\n"
            "  }\n"
            "  int value_;\n"
            "};\n")
    assert "2 member-level occurrences" in _in_class_definition(text, "Widget::get")
    found = _in_class_definition(text, "Widget::close")
    assert (found.form, found.line, found.end_line) == ("in_class", 4, 6)
    assert _in_class_definition(text, "Other::close") == ""


IMPLICIT_HEADER = ("class Widget : public Base {\n"
                   " public:\n"
                   "  void close();\n"
                   "  Member member_;\n"
                   "};\n")


def test_implicit_destructor_qualifies_when_a_user_declaration_drops_it(tmp_path):
    snapshot = _member_snapshot(tmp_path, IMPLICIT_HEADER)
    compiler = MemberOverlay(IMPLICIT_HEADER)
    _, result = _qualify_member(tmp_path, snapshot, compiler)
    assert result["result"] == "pass" and result["form"] == "implicit"
    assert (result["line"], result["end_line"]) == (1, 5)
    assert result["evidence"].endswith("user destructor declaration/disappearance")
    assert compiler.calls[1][HEADER].decode() == IMPLICIT_HEADER.replace(
        "};\n", " public:\n  ~Widget();\n};\n")


def test_implicit_destructor_that_stays_generated_refuses(tmp_path):
    snapshot = _member_snapshot(tmp_path, IMPLICIT_HEADER)
    with pytest.raises(ValueError, match="declaration did not remove"):
        _qualify_member(tmp_path, snapshot, MemberOverlay(IMPLICIT_HEADER, retain=True))


@pytest.mark.parametrize("header, reason", [
    ("class Widget {\n  DECLARE_LIFETIME(Widget);\n};\n", "macro-like member"),
    ("class Widget {\n#ifdef X\n  int extra_;\n#endif\n};\n", "preprocessor-controlled"),
])
def test_implicit_destructor_refuses_hidden_member_declarations(tmp_path, header, reason):
    snapshot = _member_snapshot(tmp_path, header)
    with pytest.raises(ValueError, match=reason):
        _qualify_member(tmp_path, snapshot, MemberOverlay(header))


def test_out_of_class_destructor_qualifies_by_definition_removal(tmp_path):
    header = ("class Widget {\n public:\n  ~Widget();\n  void close();\n};\n"
              "inline Widget::~Widget() {\n  close();\n}\n")
    snapshot = _member_snapshot(tmp_path, header)
    compiler = MemberOverlay(header)
    _, result = _qualify_member(tmp_path, snapshot, compiler)
    assert result["form"] == "qualified" and (result["line"], result["end_line"]) == (6, 8)
    assert compiler.calls[1][HEADER].decode() == header.split("inline Widget")[0]


def test_out_of_class_ordinary_member_still_refuses(tmp_path):
    header = "class Widget {\n  int get();\n};\ninline int Widget::get() { return 1; }\n"
    snapshot = _member_snapshot(tmp_path, header)
    attribution = BodyAttribution(tmp_path, snapshot)
    record = replace(attribution.record("entity-0"), symbol="?get@Widget@@QAEHXZ")
    with pytest.raises(ValueError, match="supports ordinary free functions"):
        attribution.qualify(record, Coff(object_bytes([("?get@Widget@@QAEHXZ", b"\xc3", ())])),
                            unit=UNIT, arguments=(), flags=(), compiler=MemberOverlay(header))


def test_typo_owner_or_member_never_borrows_another_body(tmp_path):
    snapshot = _member_snapshot(tmp_path, MEMBER_HEADER)
    attribution = BodyAttribution(tmp_path, snapshot)
    for symbol in ("??1Widgte@@QAE@XZ", "?clsoe@Widget@@QAEXXZ"):
        record = replace(attribution.record("entity-0"), symbol=symbol)
        with pytest.raises(ValueError, match="declared source body unresolved"):
            attribution.qualify(record, Coff(object_bytes([(symbol, b"\xc3", ())])),
                                unit=UNIT, arguments=(), flags=(), compiler=MemberOverlay(MEMBER_HEADER))


class InlineExportOverlay:
    """VC6 emits a dllexport-declared inline body, never its renamed probe."""

    def __init__(self, header, *, retain_on_removal=False):
        self.header, self.retain, self.calls = header.encode(), retain_on_removal, []

    def overlay(self, source, replacements, arguments, flags):
        self.calls.append(dict(replacements))
        text = replacements[HEADER]
        if text == self.header:
            return BASE
        if b"match_recovery_body_probe_" in text:
            # The probe definition is inline and unreferenced: not emitted.
            return object_bytes([("_other", b"\xc3", ())], undefined=(TARGET,))
        functions = [("_other", b"\xc3", ())]
        if self.retain:
            functions.append((TARGET, b"\xc3", ()))
        return object_bytes(functions, undefined=() if self.retain else (TARGET,))


INLINE_EXPORT = ("__declspec(dllexport) int sample(int value);\n"
                 "inline int sample(int value) {\n"
                 "    return value;\n"
                 "}\n")


def test_inline_definition_without_emitted_probe_qualifies_by_removal(tmp_path):
    snapshot = _snapshot(tmp_path, INLINE_EXPORT)
    compiler = InlineExportOverlay(INLINE_EXPORT)
    attribution, result = _qualify(tmp_path, snapshot, compiler)
    assert result["result"] == "pass"
    assert result["evidence"].endswith("inline definition removal/disappearance")
    removed = compiler.calls[-1][HEADER].decode()
    assert removed == ("__declspec(dllexport) int sample(int value);\n"
                       "inline int sample(int value);\n")
    assert attribution.compile_attempts == 3


def test_inline_definition_removal_that_keeps_the_definition_refuses(tmp_path):
    snapshot = _snapshot(tmp_path, INLINE_EXPORT)
    compiler = InlineExportOverlay(INLINE_EXPORT, retain_on_removal=True)
    with pytest.raises(ValueError, match="inline definition removal did not remove"):
        _qualify(tmp_path, snapshot, compiler)


def test_out_of_class_constructor_qualifies_by_definition_removal(tmp_path):
    symbol = "??0Widget@@QAE@XZ"
    header = ("class Widget {\n public:\n  Widget();\n  void close();\n};\n"
              "Widget::Widget() {\n  close();\n}\n")
    snapshot = _member_snapshot(tmp_path, header)
    attribution = BodyAttribution(tmp_path, snapshot)
    record = replace(attribution.record("entity-0"), symbol=symbol)
    base = object_bytes([(symbol, b"\xc3", ()), ("_other", b"\xc3", ())])

    class CtorOverlay:
        def __init__(self, retain):
            self.retain, self.calls = retain, []

        def overlay(self, source, replacements, arguments, flags):
            self.calls.append(dict(replacements))
            if replacements[HEADER] == header.encode():
                return base
            functions = [("_other", b"\xc3", ())]
            if self.retain:
                functions.append((symbol, b"\xc3", ()))
            return object_bytes(functions, undefined=() if self.retain else (symbol,))

    compiler = CtorOverlay(False)
    result = attribution.qualify(record, Coff(base), unit=UNIT, arguments=(), flags=(),
                                 compiler=compiler)
    assert result["form"] == "qualified" and (result["line"], result["end_line"]) == (6, 8)
    assert compiler.calls[1][HEADER].decode() == header.split("Widget::Widget")[0]
    with pytest.raises(ValueError, match="did not remove the selected function definition"):
        BodyAttribution(tmp_path, snapshot).qualify(record, Coff(base), unit=UNIT, arguments=(),
                                                    flags=(), compiler=CtorOverlay(True))
