"""Implicit destructor aliases are class declarations, not authored bodies."""

from dataclasses import replace
import hashlib
import struct

import pytest

from match_recovery.generated_member_source import (
    GeneratedMemberBinding, parse_generated_members, validate_generated_members,
)
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.ownership_source import project_view
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.shared_binary import check_shared
from match_recovery.shared_source import (
    read_shared_bindings, resolve_shared_contexts, validate_shared_bindings,
)
from test_binary import pe_fixture
from test_shared_functions import code_object, project


PIN = "a" * 64
COMPLETE = "??1Widget@@UAE@XZ"
DELETING = "??_GWidget@@UAEPAXI@Z"
EVIDENCE = "0x00401020"


def annotation(kind="complete_destructor", symbol=COMPLETE, evidence=EVIDENCE,
               start="0x00401000", end="0x00401001"):
    return (f"// RECOVERY_GENERATED_MEMBER: v1 {kind} {start} {end} "
            f"{symbol} {evidence}")


def class_source(*annotations, body="Widget();"):
    return "\n".join((*annotations, "class Widget {", " public:", f"  {body}", "};", ""))


def long_symbol_object(symbol, code):
    raw = code + b"\xcc" * (-len(code) % 16)
    data_at = 60
    symbols_at = data_at + len(raw)
    record = (bytes(4) + struct.pack("<I", 4) + struct.pack("<IhHBB", 0, 1, 0x20, 2, 0))
    strings = struct.pack("<I", 4 + len(symbol.encode()) + 1) + symbol.encode() + b"\0"
    header = struct.pack("<HHIIIHH", 0x14C, 1, 0, symbols_at, 1, 0, 0)
    section = (b".text\0\0\0" + struct.pack("<IIIIIIHHI", 0, 0, len(raw), data_at,
        0, 0, 0, 0, 0x60501020))
    return header + section + raw + record + strings


def parse(text):
    return parse_generated_members(text, "code/widget.h", PIN, address_space="pe:va")


def test_implicit_complete_and_deleting_aliases_share_a_real_class_anchor():
    source = "#pragma once\n#ifndef WIDGET_H\n#define WIDGET_H\n" + class_source(
        annotation(), annotation("deleting_destructor", DELETING)) + "#endif\n"
    complete, deleting = parse(source)
    assert (complete.owner, complete.kind, complete.symbol) == ("Widget", "complete_destructor", COMPLETE)
    assert (deleting.owner, deleting.kind, deleting.symbol) == ("Widget", "deleting_destructor", DELETING)
    assert complete.origin == deleting.origin and complete.span == deleting.span
    assert complete.interpretation_id != deleting.interpretation_id
    assert "line" not in complete.interpretation
    assert replace(complete, line=complete.line + 100).interpretation_id == complete.interpretation_id
    assert complete.object_unit is None  # Context is always derived for this annotation kind.


@pytest.mark.parametrize("source,reason", [
    (annotation(), "missing"),
    (class_source(annotation()) + class_source(), "ambiguous"),
    (annotation() + "\n#if 0\nclass Widget {};\n#endif\n", "preprocessor"),
    (annotation() + "\nnamespace N { class Widget {}; }\n", "adjacent"),
    (annotation() + "\nclass Outer { class Widget {}; };\n", "disagrees"),
    (annotation() + "\nclass Widget { DECLARE_DTOR(Widget) };\n", "macro"),
    ("#define dtor() ~Widget() {}\n" + annotation() +
     "\nclass Widget { Widget(); dtor(); };\n", "macro"),
    ("#define DECLARE_DTOR ~Widget() {}\n" + annotation() +
     "\nclass Widget { Widget(); DECLARE_DTOR; };\n", "macro"),
    ("#define DECLARE_DTOR ~Widget() {}\n" + annotation() +
     "\nclass Widget { Widget(); DECLARE_DTOR };\n", "macro"),
    (annotation() + "\nclass Widget {\n#if ENABLE_DTOR\n};\n#endif\n", "preprocessor"),
])
def test_missing_ambiguous_disabled_and_unsupported_class_anchors_refuse(source, reason):
    with pytest.raises(ValueError, match=reason):
        parse(source)


def test_nested_class_and_block_comment_markers_cannot_mint_implicit_members():
    nested = ("class Outer {\n" + annotation() + "\nclass Widget {};\n};\n")
    with pytest.raises(ValueError, match="nested"):
        parse(nested)
    commented = ("/*\n" + annotation() + "\n*/\nclass Widget { Widget(); };\n")
    with pytest.raises(ValueError, match="active line comment"):
        parse(commented)
    raw = ('const char *s = R"tag(\n' + annotation() + '\n)tag";\nclass Widget {};\n')
    with pytest.raises(ValueError, match="active line comment"):
        parse(raw)
    continued = ('const char *s = "prefix\\\n' + annotation() +
                 '\n";\nclass Widget { Widget(); };\n')
    with pytest.raises(ValueError, match="line-spliced"):
        parse(continued)


def test_complete_kind_rejects_explicit_destructor_but_deleting_kind_allows_it():
    explicit = class_source(annotation(), body="~Widget() = default;")
    with pytest.raises(ValueError, match="requires an implicit destructor"):
        parse(explicit)
    assert parse(class_source(annotation("deleting_destructor", DELETING),
                             body="~Widget() = default;"))[0].kind == "deleting_destructor"
    with pytest.raises(ValueError, match="requires an implicit destructor"):
        parse(annotation() + "\nclass Widget { public: ~Widget() {} };\n")


@pytest.mark.parametrize("kind,symbol", [
    ("complete_destructor", DELETING),
    ("deleting_destructor", COMPLETE),
    ("complete_destructor", "??1Other@@UAE@XZ"),
    ("deleting_destructor", "??_GOther@@UAEPAXI@Z"),
    ("complete_destructor", "??_EWidget@@UAEPAXI@Z"),
])
def test_symbol_kind_and_owner_must_match_the_anchored_class(kind, symbol):
    with pytest.raises(ValueError):
        parse(class_source(annotation(kind, symbol)))


def test_constructor_evidence_must_be_a_unique_mapped_constructor_of_the_same_class():
    binding, = parse(class_source(annotation()))
    evidence = binding.evidence.entity_id
    base = {"seed": False, "unbound": False, "path": "code/widget.cpp",
            "symbol": "??0Widget@@QAE@XZ"}
    view = {"groups": [{"entities": [{"entity_id": evidence, "conflicts": [],
                                        "candidates": [base]}]}]}
    validate_generated_members([binding], view)
    for candidates in ([], [{**base, "symbol": "??0Other@@QAE@XZ"}], [base, base]):
        broken = {"groups": [{"entities": [{"entity_id": evidence, "conflicts": [],
                                               "candidates": candidates}]}]}
        with pytest.raises(ValueError, match="mapped constructor"):
            validate_generated_members([binding], broken)
    original = {"entity_id": binding.origin.entity_id, "conflicts": [], "candidates": [{
        "seed": False, "unbound": False, "component": None, "symbol": COMPLETE,
        "spans": [{"start": binding.span.start, "end": binding.span.end}]}]}
    conflicting = {"groups": [{"entities": [*view["groups"][0]["entities"], original]}]}
    with pytest.raises(ValueError, match="duplicates an existing source binding"):
        validate_shared_bindings([binding], conflicting)


def test_generated_aliases_do_not_create_canonical_original_function_counts():
    source = ("// ORIGINAL: 0x00401000 _canonical 0x00401000-0x00401001 FILE\n"
              "// symbol _canonical\nvoid canonical() {}\n" +
              class_source(annotation(), annotation("deleting_destructor", DELETING)))
    from match_recovery.source.reader import read_text

    records = read_text(source, "code/widget.cpp")
    aliases = parse(source)
    assert len(records) == 1
    assert len(aliases) == 2
    assert {row.interpretation["kind"] for row in aliases} == {
        "compiler_generated_complete_destructor", "compiler_generated_deleting_destructor"}


def test_generated_aliases_reuse_shared_copy_aggregation_and_require_carrier():
    raw = pe_fixture(b"\xc3")
    pin = hashlib.sha256(raw).hexdigest()
    origin = Origin(pin, "pe:va", 0x401000)
    evidence = Origin(pin, "pe:va", 0x401020)
    binding = GeneratedMemberBinding("complete_destructor", "Widget", origin,
        Span(origin.address, origin.address + 1), COMPLETE, evidence,
        "code/widget.h", 3, compiler_unit="code/widget.cc")
    image = PeImage(raw, pin)
    objects = {"code/widget.cc": Coff(long_symbol_object(COMPLETE, b"\xc3"))}
    targets, rows = check_shared([binding], objects, image, {})
    assert targets == {COMPLETE: origin.address} and rows[0]["result"] == "pass"
    assert rows[0]["compiler_context"] == "code/widget.cc"
    uncontexted = replace(binding, compiler_unit=None)
    targets, rows = check_shared([uncontexted], objects, image, {})
    assert not targets and rows[0]["result"] == "unknown"
    targets, rows = check_shared([binding], {}, image, {})
    assert not targets and rows[0]["result"] == "unknown"
    objects["code/other.cc"] = Coff(long_symbol_object(COMPLETE, b"\x90"))
    targets, rows = check_shared([binding], objects, image, {})
    assert not targets and rows[0]["result"] == "fail"
    assert [(row["unit"], row["result"]) for row in rows[0]["copies"]] == [
        ("code/other.cc", "fail"), ("code/widget.cc", "pass")]


def test_high_level_source_resolution_uses_mapped_same_class_constructor(project):
    source = project / "code/methods.cc"
    source.write_text("// ORIGINAL: 0x00401020 ??0Widget@@QAE@XZ 0x00401020-0x00401021\n"
                      "Widget::Widget() {}\n")
    header = project / "code/widget.h"
    header.write_text("#pragma once\n#ifndef WIDGET_H\n#define WIDGET_H\n" +
                      class_source(annotation(end="0x00401006")) + "#endif\n")
    pin = hashlib.sha256((project / "image.exe").read_bytes()).hexdigest()
    view, _ = project_view(project)
    aliases = read_shared_bindings(project, pin)
    resolved = resolve_shared_contexts(project, aliases, settings=Measurement.read(project))
    validate_shared_bindings(resolved, view)
    assert len(resolved) == 1 and resolved[0].object_unit == "code/methods.cc"
    assert resolved[0].context_error is None
    assert resolved[0].interpretation_id == aliases[0].interpretation_id


def test_placeholder_constructor_evidence_stays_unknown(project):
    source = project / "code/methods.cc"
    source.write_text("// ORIGINAL: 0x00401020 ??0Widget@@QAE@XZ 0x00401020-0x00401021\n"
                      "Widget::Widget() {\n// BODY GOES HERE.\n}\n")
    (project / "code/widget.h").write_text(class_source(annotation()))
    pin = hashlib.sha256((project / "image.exe").read_bytes()).hexdigest()
    aliases = read_shared_bindings(project, pin)
    resolved = resolve_shared_contexts(project, aliases, settings=Measurement.read(project))
    assert resolved[0].object_unit is None
    assert "not an implemented" in resolved[0].context_error


def test_missing_and_ambiguous_constructor_carriers_stay_unknown(project):
    source = project / "code/methods.cc"
    source.write_text("// ORIGINAL: 0x00401020 ??0Widget@@QAE@XZ 0x00401020-0x00401021\n"
                      "Widget::Widget() {}\n")
    header = project / "code/widget.h"
    header.write_text(class_source(annotation(evidence="0x00401040")))
    pin = hashlib.sha256((project / "image.exe").read_bytes()).hexdigest()
    bindings = read_shared_bindings(project, pin)
    resolved = resolve_shared_contexts(project, bindings, settings=Measurement.read(project))
    assert resolved[0].object_unit is None and "missing or ambiguous" in resolved[0].context_error

    header.write_text(class_source(annotation()))
    (project / "code/other.cc").write_text(
        "// ORIGINAL: 0x00401020 ??0Widget@@QAE@XZ 0x00401020-0x00401021\n"
        "Widget::Widget() {}\n")
    bindings = read_shared_bindings(project, pin)
    resolved = resolve_shared_contexts(project, bindings, settings=Measurement.read(project))
    assert resolved[0].object_unit is None and "missing or ambiguous" in resolved[0].context_error


def test_preservation_indexes_generated_member_owner_and_excludes_derived_context(project):
    from match_recovery.preservation import _outside_annotations, _symbol_sources
    from match_recovery.source_config import SourceConfiguration

    source = project / "code/methods.cc"
    source.write_text("// ORIGINAL: 0x00401020 ??0Widget@@QAE@XZ 0x00401020-0x00401021\n"
                      "Widget::Widget() {}\n")
    header = project / "code/widget.h"
    header.write_text(class_source(annotation()))
    pin = hashlib.sha256((project / "image.exe").read_bytes()).hexdigest()
    configuration = SourceConfiguration.read(project)
    files = {"code/widget.h": header.read_bytes(), "code/methods.cc": source.read_bytes()}
    records = _outside_annotations(files, configuration, pin, set())
    row, = [item for item in records.values() if item["kind"] == "generated_member"]
    assert row["member_kind"] == "complete_destructor"
    assert row["symbol"] == COMPLETE and row["unit"] == "code/widget.h"
    assert "line" not in row and "compiler_unit" not in row and "context_error" not in row
    indexed = _symbol_sources({}, records, set())
    assert indexed[COMPLETE][0]["kind"] == "generated_member"
    assert indexed[COMPLETE][0]["source"] == "code/widget.h"

