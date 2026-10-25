"""Associated-data annotations remain explicit, bounded source evidence."""

from dataclasses import replace
import json
from pathlib import Path

import pytest

from match_recovery.associated_source import (
    PREFIX, AssociatedDataBinding, parse_associated, read_associated,
    validate_associated,
)
from match_recovery.associated_writer import write_associated_bytes
from match_recovery.model import Origin, Span, digest


PIN = "a" * 64
SPACE = "test:va"
ANNOTATION = (
    "// RECOVERY_ASSOCIATED: v1 switch_targets "
    "0x00001000 0x00002000 0x00002008 0x00001004"
)


def binding(**changes) -> AssociatedDataBinding:
    row, = parse_associated(ANNOTATION, "src/a.cpp", PIN,
                            address_space=SPACE)
    return replace(row, **changes)


def test_parse_retains_typed_identity_interpretation_and_evidence() -> None:
    row, = parse_associated("prefix\n  " + ANNOTATION + " \n", "src/a.cpp",
                            PIN, address_space=SPACE)
    assert row == AssociatedDataBinding(
        Origin(PIN, SPACE, 0x1000), Span(0x2000, 0x2008), "switch_targets",
        Origin(PIN, SPACE, 0x1004), "src/a.cpp", 2,
    )
    assert row.interpretation == {
        "kind": "switch_targets",
        "owner": {"image_sha256": PIN, "address_space": SPACE, "address": 0x1000},
        "span": {"start": 0x2000, "end": 0x2008},
        "original_evidence": {"image_sha256": PIN, "address_space": SPACE,
                               "address": 0x1004},
        "unit": "src/a.cpp",
    }
    assert row.interpretation_id == digest({
        "entity_id": row.evidence.entity_id, "value": row.interpretation,
    })


def test_parser_ignores_marker_text_inside_block_and_raw_comments() -> None:
    text = (
        "/*\n" + ANNOTATION + "\n*/\n"
        "const char *text = R\"tag(\n" + ANNOTATION + "\n)tag\";\n"
        + ANNOTATION + "\n"
    )
    rows = parse_associated(text, "src/a.cpp", PIN, address_space=SPACE)
    assert len(rows) == 1
    assert rows[0].line == 7


@pytest.mark.parametrize("text", [
    "int ordinary;\n",
    "// unrelated splice \\\ncontinued comment\n",
    'const char *text = R"tag(ordinary)tag";\n',
])
def test_parser_fast_path_preserves_empty_result_without_prefix(text: str) -> None:
    assert PREFIX not in text
    assert parse_associated(text, "src/a.cpp", PIN,
                             address_space=SPACE) == ()


@pytest.mark.parametrize("splice", ["\\", "??/"])
@pytest.mark.parametrize("prefix", [
    "// ordinary comment ",
    'const char *s = "ordinary ',
])
def test_parser_fails_closed_on_association_candidates_in_spliced_source(
        splice: str, prefix: str) -> None:
    tail = "\nint ordinary;\n" if prefix.startswith("//") else '\n";\n'
    text = prefix + splice + "\n" + ANNOTATION + tail
    with pytest.raises(ValueError, match="line-spliced source"):
        parse_associated(text, "src/a.cpp", PIN, address_space=SPACE)


def test_parser_keeps_empty_behavior_for_spliced_source_without_association() -> None:
    text = "// ordinary comment \\\ncontinued comment\nint ordinary;\n"
    assert parse_associated(text, "src/a.cpp", PIN,
                             address_space=SPACE) == ()


@pytest.mark.parametrize("newline", ["\n", "\r\n"])
@pytest.mark.parametrize("splice", ["\\", "??/"])
@pytest.mark.parametrize("trailing", ["", " \t"])
def test_unspliced_prefix_is_independent_of_a_later_splice(newline, splice, trailing):
    text = newline.join(("int first;", ANNOTATION,
                         "#define LATER " + splice + trailing, "int next;", ""))
    row, = parse_associated(text, "src/a.cpp", PIN, address_space=SPACE)
    assert row == binding(line=2)
    with pytest.raises(ValueError, match="line-spliced source"):
        parse_associated("// earlier " + splice + trailing + newline + text,
                         "src/a.cpp", PIN, address_space=SPACE)


@pytest.mark.parametrize("splice", ["\\", "??/"])
@pytest.mark.parametrize("prefix", ["", "/*\n", 'const char *s = R"tag(\n'])
def test_post_splice_apparent_record_refuses_even_under_untrusted_lexical_context(splice, prefix):
    text = ANNOTATION + "\n#define LATER " + splice + "\nvalue\n" + prefix + ANNOTATION + "\n"
    with pytest.raises(ValueError, match="line-spliced source"):
        parse_associated(text, "src/a.cpp", PIN, address_space=SPACE)


@pytest.mark.parametrize("splice", ["\\", "??/"])
def test_record_on_the_first_spliced_line_refuses(splice):
    with pytest.raises(ValueError, match="line-spliced source"):
        parse_associated(ANNOTATION + " " + splice + "\nvalue\n",
                         "src/a.cpp", PIN, address_space=SPACE)


@pytest.mark.parametrize("newline", [b"\n", b"\r\n"])
@pytest.mark.parametrize("splice", [b"\\", b"??/"])
def test_writer_round_trips_only_the_prefix_before_a_later_macro(newline, splice):
    raw = newline.join((b"int first;", b"int second;", b"#define LATER " + splice, b"value", b""))
    row = binding(line=2)
    changed = write_associated_bytes(raw, expected=(), additions=(row,),
                                    path="src/a.cpp", pin=PIN, address_space=SPACE)
    assert changed == b"int first;" + newline + _marker_bytes(row) + newline + raw.split(newline, 1)[1]
    parsed = parse_associated(changed.decode(), "src/a.cpp", PIN, address_space=SPACE)
    assert len(parsed) == 1 and _identity(parsed[0]) == _identity(row)
    for line in (3, 4, 5):
        with pytest.raises(ValueError, match="line-spliced source"):
            write_associated_bytes(raw, expected=(), additions=(binding(line=line),),
                                   path="src/a.cpp", pin=PIN, address_space=SPACE)


@pytest.mark.parametrize("record", [
    "v2 switch_targets 0x1000 0x2000 0x2008 0x1004",
    "v1 unknown_kind 0x1000 0x2000 0x2008 0x1004",
    "v1 switch_targets 1000 0x2000 0x2008 0x1004",
    "v1 switch_targets 0x100000000 0x2000 0x2008 0x1004",
    "v1 switch_targets 0x1000 0x2008 0x2008 0x1004",
    "v1 switch_targets 0x1000 0x2000 0x100000000 0x1004",
    "v1 switch_targets 0x1000 0x2000 0x100000001 0x1004",
    "v1 switch_targets 0x1000 0x2000 0x2008 0x100000000",
    "v1 switch_targets 0x1000 0x2000 0x2008 0x1004 extra",
])
def test_malformed_or_unbounded_annotation_is_rejected(record: str) -> None:
    with pytest.raises(ValueError):
        parse_associated("// RECOVERY_ASSOCIATED: " + record,
                         "src/a.cpp", PIN, address_space=SPACE)


def test_maximal_supported_end_is_inside_32_bit_address_space() -> None:
    text = ("// RECOVERY_ASSOCIATED: v1 switch_selector 0xfffffffe "
            "0xfffffffe 0xffffffff 0xfffffffd")
    row, = parse_associated(text, "src/a.cpp", PIN, address_space=SPACE)
    assert row.span == Span(0xFFFFFFFE, 0xFFFFFFFF)


@pytest.mark.parametrize("changes", [
    {"unit": "../a.cpp"}, {"unit": "/a.cpp"}, {"unit": "src/./a.cpp"},
    {"line": 0}, {"line": True}, {"kind": "other"},
    {"owner": Origin(PIN, SPACE, 0x100000000)},
    {"span": Span(0x2000, 0x100000001)},
    {"owner": Origin("b" * 64, SPACE, 0x1000)},
    {"evidence": Origin(PIN, "another:space", 0x1004)},
])
def test_direct_binding_construction_cannot_bypass_identity_invariants(changes) -> None:
    with pytest.raises(ValueError):
        binding(**changes)


def test_direct_api_revalidates_mutated_nested_frozen_values() -> None:
    row = binding()
    object.__setattr__(row.span, "end", True)
    with pytest.raises(ValueError):
        validate_associated((row,))


@pytest.mark.parametrize("changes,reason", [
    ({}, "duplicate associated-data"),
    ({"owner": Origin(PIN, SPACE, 0x1008),
      "evidence": Origin(PIN, SPACE, 0x100C)}, "overlap"),
    ({"unit": "src/b.cpp"}, "multiple source units"),
    ({"owner": Origin("b" * 64, SPACE, 0x1000),
      "evidence": Origin("b" * 64, SPACE, 0x1004)}, "different originals"),
    ({"owner": Origin(PIN, "other:space", 0x1000),
      "evidence": Origin(PIN, "other:space", 0x1004)}, "different originals"),
])
def test_population_rejects_duplicates_overlaps_and_owner_conflicts(changes, reason) -> None:
    first = binding()
    second = replace(first, **changes)
    with pytest.raises(ValueError, match=reason):
        validate_associated((first, second))


def project(root: Path) -> Path:
    (root / "src/seeds").mkdir(parents=True)
    (root / "recovery").mkdir()
    configuration = {
        "schema_version": 1,
        "source": {
            "image_pin": "images/original.exe",
            "address_space": SPACE,
            "roots": ["src"],
            "seed_roots": ["src/seeds"],
        },
    }
    (root / "recovery/project.json").write_text(json.dumps(configuration))
    return root


def mapped_source(data_span: str = "0x00002000-0x00002008",
                  extra: str = "") -> str:
    return (
        ANNOTATION + "\n"
        "// ORIGINAL: 0x00001000 parent 0x00001000-0x00001020 FILE\n"
        "int parent() { return 1; }\n" + extra
    )


def test_reader_requires_same_unit_mapped_parent_and_ignores_seed_annotations(tmp_path) -> None:
    root = project(tmp_path)
    (root / "src/a.cpp").write_text(mapped_source())
    (root / "src/seeds/old.cpp").write_text(
        "// RECOVERY_ASSOCIATED: malformed seed annotation\n"
        "// ORIGINAL: 0x00001000 old_parent 0x00001000-0x00001020 FILE\n"
    )
    (root / "src/ignored.txt").write_text(ANNOTATION)
    assert read_associated(root, PIN) == (binding(),)


def test_reader_without_associations_keeps_empty_default_behavior(tmp_path) -> None:
    root = project(tmp_path)
    (root / "src/a.cpp").write_text("int ordinary_source;\n")
    (root / "src/seeds/old.cpp").write_text(
        "// RECOVERY_OBJECT: malformed retained seed record\n"
    )
    assert read_associated(root, PIN) == ()


@pytest.mark.parametrize("extra,annotation,reason", [
    ("", ANNOTATION.replace("0x00001000", "0x00003000", 1), "one source-mapped function"),
    ("// ORIGINAL: 0x00003000 other 0x00003000-0x00003010 FILE\n",
     ANNOTATION.replace("0x00002000 0x00002008", "0x00003004 0x00003008"),
     "overlaps a source-mapped function"),
])
def test_reader_rejects_missing_owner_or_function_overlap(tmp_path, extra,
                                                         annotation, reason) -> None:
    root = project(tmp_path)
    source = mapped_source(extra=extra).replace(ANNOTATION, annotation, 1)
    (root / "src/a.cpp").write_text(source)
    with pytest.raises(ValueError, match=reason):
        read_associated(root, PIN)


@pytest.mark.parametrize("other_record,reason", [
    ("// RECOVERY_OBJECT: v2 0x00002002 0x00002004 item 0x00001004\n",
     "source-owned object"),
    ("// RECOVERY_GENERATED: v1 vtable 0x00002002 0x00002004 ??_7Table@@6B@ 0x00001004\n",
     "compiler-generated table"),
    ("// RECOVERY_LITERAL: v1 0x00002002 0x00002004 lit 0x00001004\n",
     "source-owned literal"),
])
def test_reader_rejects_other_declared_data_ownership(tmp_path, other_record,
                                                       reason) -> None:
    root = project(tmp_path)
    (root / "src/a.cpp").write_text(mapped_source(extra=other_record))
    with pytest.raises(ValueError, match=reason):
        read_associated(root, PIN)


def test_reader_checks_retained_seed_code_spans_for_collisions(tmp_path) -> None:
    root = project(tmp_path)
    (root / "src/a.cpp").write_text(mapped_source())
    (root / "src/seeds/old.cpp").write_text(
        "// ORIGINAL: 0x00003000 old 0x00002004-0x0000200C FILE\n"
    )
    with pytest.raises(ValueError, match="source-mapped function span"):
        read_associated(root, PIN)


@pytest.mark.parametrize("annotation,reason", [
    ("// RECOVERY_OBJECT: v2 0x00002002 0x00002006 seed_item 0x00001004\n",
     "source-owned object"),
    ("// RECOVERY_GENERATED: v1 vtable 0x00002002 0x00002006 ??_7SeedTable@@6B@ 0x00001004\n",
     "compiler-generated table"),
    ("// RECOVERY_LITERAL: v1 0x00002002 0x00002006 seed_literal 0x00001004\n",
     "source-owned literal"),
])
def test_reader_checks_retained_seed_data_spans_for_collisions(
        tmp_path, annotation, reason) -> None:
    root = project(tmp_path)
    (root / "src/a.cpp").write_text(mapped_source())
    (root / "src/seeds/old.cpp").write_text(annotation)
    with pytest.raises(ValueError, match=reason):
        read_associated(root, PIN)


def test_reader_rejects_fragment_and_multi_span_owner(tmp_path) -> None:
    root = project(tmp_path)
    multi = mapped_source().replace(
        "0x00001000-0x00001020 FILE",
        "0x00001000-0x00001010;0x00001020-0x00001030 FILE",
    )
    (root / "src/a.cpp").write_text(multi)
    with pytest.raises(ValueError, match="one independent non-fragment"):
        read_associated(root, PIN)


def test_writer_is_add_only_round_trips_and_preserves_newline_style() -> None:
    raw = ("int first;\r\nint second;\r\n").encode()
    row = binding(line=2)
    result = write_associated_bytes(
        raw, expected=(), additions=(row,), path="src/a.cpp", pin=PIN,
        address_space=SPACE,
    )
    marker = _marker_bytes(row) + b"\r\n"
    assert result == b"int first;\r\n" + marker + b"int second;\r\n"
    assert parse_associated(result.decode(), "src/a.cpp", PIN,
                            address_space=SPACE)[0].span == row.span


def _marker_bytes(row: AssociatedDataBinding) -> bytes:
    return (
        "// RECOVERY_ASSOCIATED: v1 "
        f"{row.kind} 0x{row.owner.address:08X} "
        f"0x{row.span.start:08X} 0x{row.span.end:08X} "
        f"0x{row.evidence.address:08X}"
    ).encode()


def test_writer_snapshot_and_reparse_preserve_existing_identity_and_bytes() -> None:
    old = binding(line=1)
    raw = ANNOTATION.encode() + b"\nint f;\n"
    expected = parse_associated(raw.decode(), "src/a.cpp", PIN,
                                address_space=SPACE)
    new = binding(span=Span(0x3000, 0x3008), kind="switch_selector",
                  evidence=Origin(PIN, SPACE, 0x1008), line=3)
    result = write_associated_bytes(
        raw, expected=expected, additions=(new,), path="src/a.cpp", pin=PIN,
        address_space=SPACE,
    )
    records = parse_associated(result.decode(), "src/a.cpp", PIN,
                               address_space=SPACE)
    assert {_identity(row) for row in records} == {_identity(old), _identity(new)}
    assert b"\nint f;\n" in result


def _identity(row: AssociatedDataBinding) -> tuple:
    return row.owner, row.span, row.kind, row.evidence, row.unit


@pytest.mark.parametrize("changes,raw,reason", [
    ({"expected": ()}, ANNOTATION.encode() + b"\n", "complete source snapshot"),
    ({"additions": ()}, b"int x;\n", "at least one addition"),
    ({"additions": (binding(line=2),)}, b"/*\ninside\n*/\n", "inside a block comment"),
    ({"additions": (binding(line=1),)}, b"int x;\\\nint y;\n", "line-spliced source"),
    ({"additions": (binding(line=1),)}, b"\xef\xbb\xbfint x;\n", "BOM-prefixed"),
    ({"additions": (binding(line=1),)}, b'const char *s = R"x(\n)x";\n', "raw-string"),
    ({"additions": (binding(line=1),)}, "int x;\u2028int y;".encode(), "line separator"),
])
def test_writer_refuses_stale_or_ambiguous_updates(changes, raw, reason) -> None:
    kwargs = {"expected": (), "additions": (binding(line=1),)}
    kwargs.update(changes)
    with pytest.raises(ValueError, match=reason):
        write_associated_bytes(raw, path="src/a.cpp", pin=PIN,
                               address_space=SPACE, **kwargs)


def test_writer_refuses_wrong_unit_or_original_and_overlapping_batch() -> None:
    raw = b"int x;\n"
    for row in (binding(unit="src/b.cpp", line=1),
                 binding(owner=Origin("b" * 64, SPACE, 0x1000),
                         evidence=Origin("b" * 64, SPACE, 0x1004), line=1)):
        with pytest.raises(ValueError):
            write_associated_bytes(raw, expected=(), additions=(row,),
                                   path="src/a.cpp", pin=PIN,
                                   address_space=SPACE)
    first = binding(line=1)
    overlap = replace(first, owner=Origin(PIN, SPACE, 0x1008),
                      span=Span(0x2004, 0x200C),
                      evidence=Origin(PIN, SPACE, 0x100C))
    with pytest.raises(ValueError, match="overlap"):
        write_associated_bytes(raw, expected=(), additions=(first, overlap),
                               path="src/a.cpp", pin=PIN,
                               address_space=SPACE)
