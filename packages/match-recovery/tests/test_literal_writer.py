"""The literal writer adds typed records without changing source contents."""

from dataclasses import replace

import pytest

from match_recovery.literal_source import parse_literals
from match_recovery.literal_writer import write_literal_bytes
from match_recovery.model import Origin, Span


PIN = "a" * 64
SPACE = "custom:offset"
ANNOTATION = b"// RECOVERY_LITERAL: v1 0x1000 0x1003 local_literal 0x2000"


def binding(**changes):
    row, = parse_literals(ANNOTATION.decode(), "code/a.cc", PIN,
                          address_space=SPACE)
    return replace(row, **changes)


def marker(row):
    return ("// RECOVERY_LITERAL: v1 "
            f"0x{row.span.start:08X} 0x{row.span.end:08X} "
            f"{row.symbol} 0x{row.evidence.address:08X}").encode("ascii")


def write(raw, rows, *, expected=(), **changes):
    options = {"path": "code/a.cc", "pin": PIN, "address_space": SPACE}
    options.update(changes)
    return write_literal_bytes(raw, expected=expected, additions=rows, **options)


def test_writer_adds_record_preserves_existing_and_keeps_crlf_and_ordinary_bytes():
    raw = ANNOTATION + b"\r\nconst char *text = \"caf\xc3\xa9\";\r\n"
    expected = parse_literals(raw.decode("utf-8"), "code/a.cc", PIN,
                              address_space=SPACE)
    added = binding(origin=Origin(PIN, SPACE, 0x3000),
                    span=Span(0x3000, 0x3004), symbol="?local@module@@3", line=3,
                    evidence=Origin(PIN, SPACE, 0x2010))
    result = write(raw, (added,), expected=expected)
    assert result == ANNOTATION + b"\r\n" + raw.split(b"\r\n", 1)[1] + marker(added) + b"\r\n"
    parsed = parse_literals(result.decode("utf-8"), "code/a.cc", PIN,
                            address_space=SPACE)
    assert {row.symbol for row in parsed} == {"local_literal", "?local@module@@3"}


def test_writer_can_append_without_a_final_newline():
    raw = b"int ordinary = 7;"
    row = binding(line=2)
    assert write(raw, (row,)) == raw + b"\n" + marker(row) + b"\n"


@pytest.mark.parametrize("symbol", [".LC0", "??_C@_02JKAF@rb?$AA@"])
def test_writer_accepts_common_ascii_compiler_symbols(symbol):
    row = binding(symbol=symbol, line=1)
    result = write(b"int ordinary;\n", (row,))
    assert marker(row) + b"\n" in result


def test_writer_refuses_stale_snapshot_and_foreign_unit_or_original():
    raw = b"int ordinary;\n"
    row = binding(line=1)
    actual = ANNOTATION + b"\n"
    with pytest.raises(ValueError, match="complete source snapshot"):
        write(actual, (row,), expected=())
    for bad in (replace(row, unit="code/b.cc"),
                replace(row, origin=Origin("b" * 64, SPACE, row.origin.address),
                        evidence=Origin("b" * 64, SPACE, row.evidence.address))):
        with pytest.raises(ValueError):
            write(raw, (bad,))
    with pytest.raises(ValueError, match="another source unit"):
        write(raw, (row,), path="code/b.cc")


def test_writer_refuses_duplicate_or_overlapping_original_bindings():
    raw = b"int ordinary;\n"
    row = binding(line=1)
    with pytest.raises(ValueError, match="conflicts"):
        write(raw, (row, row))
    overlap = replace(row, origin=Origin(PIN, SPACE, 0x1002),
                      span=Span(0x1002, 0x1005), symbol="other",
                      evidence=Origin(PIN, SPACE, 0x2010))
    with pytest.raises(ValueError, match="overlap"):
        write(raw, (row, overlap))


@pytest.mark.parametrize("symbol", ["x*/", "caf\u00e9", "x\\ny", "??/"])
def test_writer_refuses_unsafe_compiler_symbol_tokens(symbol):
    row = binding(symbol=symbol, line=1)
    with pytest.raises(ValueError, match="ASCII compiler-symbol token"):
        write(b"int ordinary;\n", (row,))


@pytest.mark.parametrize("raw,reason", [
    (b"/* block\ninside\n*/\n", "inside a block comment"),
    (b"int x; \\\nint y;\n", "line-spliced source"),
    (b"\xef\xbb\xbfint x;\n", "BOM-prefixed"),
    (b'const char *s = R"tag(\n)x";\n', "raw-string"),
    (b"int x;\xe2\x80\xa8int y;", "line separator"),
    (b"// RECOVERY_LITERAL: malformed\n", "invalid literal annotation"),
])
def test_writer_refuses_ambiguous_or_malformed_anchors(raw, reason):
    row = binding(line=2 if "block" in reason else 1)
    with pytest.raises(ValueError, match=reason):
        write(raw, (row,))


def test_writer_refuses_insertion_that_shadows_an_existing_record():
    raw = ANNOTATION + b"\n"
    added = binding(origin=Origin(PIN, SPACE, 0x3000),
                    span=Span(0x3000, 0x3004), symbol="other",
                    evidence=Origin(PIN, SPACE, 0x2010), line=1)
    expected = parse_literals(raw.decode(), "code/a.cc", PIN,
                              address_space=SPACE)
    with pytest.raises(ValueError, match="shadow"):
        write(raw, (added,), expected=expected)


@pytest.mark.parametrize("field,value", [
    ("address", True),
    ("image_sha256", "not-a-digest"),
    ("address_space", " "),
])
def test_writer_revalidates_mutated_nested_origin(field, value):
    row = binding(line=1)
    object.__setattr__(row.origin, field, value)
    with pytest.raises(ValueError):
        write(b"int ordinary;\n", (row,))


def test_writer_revalidates_mutated_nested_evidence_and_span():
    bad_evidence = binding(line=1)
    object.__setattr__(bad_evidence.evidence, "address", True)
    with pytest.raises(ValueError):
        write(b"int ordinary;\n", (bad_evidence,))

    bad_span = binding(line=1)
    object.__setattr__(bad_span.span, "end", True)
    with pytest.raises(ValueError):
        write(b"int ordinary;\n", (bad_span,))
