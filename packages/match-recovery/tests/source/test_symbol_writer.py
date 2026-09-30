"""Explicit current-symbol updates preserve historical identities and source."""

from dataclasses import replace
from pathlib import Path

import pytest

from match_recovery.source import read_text, write_symbol
from match_recovery.source.publication import publish
from match_recovery import binding_source


PATH = Path("fixture.cpp")
MARKER = b"// ORIGINAL: 0x00401000 ?f@@QAEHXZ 0x00401000-0x00401006 BYTE_EXACT"
NEXT = b"// ORIGINAL: 0x00402000 _next 0x00402000-0x00402001 FILE"
SYMBOL = "?f@@UAEHXZ"
BINDING = binding_source.render("Original callers establish the member", ["call at 0x403000"]).encode()


def records(raw):
    return read_text(raw.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n"), PATH)


@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
@pytest.mark.parametrize("block", [False, True])
def test_insert_preserves_original_marker_code_claim_and_lessons(newline, block):
    prefix = b"/*" + newline if block else b""
    suffix = b"*/" + newline if block else b""
    raw = prefix + MARKER + newline + b"// TRIED: old attempt" + newline + b"//   continuation" + newline + suffix + b"int f() { return 0; }" + newline
    row, = records(raw)
    result = write_symbol(raw, row, SYMBOL)
    assert result == raw.replace(MARKER + newline, MARKER + newline + b"// symbol    " + SYMBOL.encode() + newline)
    after, = records(result)
    assert after.symbol == SYMBOL
    assert after.byte_exact and after.name == row.name
    assert after.image_spans == row.image_spans and after.ruled_out == row.ruled_out
    assert write_symbol(result, after, SYMBOL) == result


@pytest.mark.parametrize("fact", [b"// symbol    _old  \r\n", b" * symbol  _old\t\n"])
def test_replace_only_symbol_bytes_with_mixed_newlines_and_non_utf8(fact):
    raw = b"// opaque \xff\n/*\r\n" + MARKER + b"\r\n" + fact + b"*/\nint f() {}\r\n"
    row, = records(raw)
    assert row.symbol == "_old"
    assert write_symbol(raw, row, SYMBOL) == raw.replace(b"_old", SYMBOL.encode())


@pytest.mark.parametrize("key,value,field", [("symbol", "_next", "symbol"), ("body", "src/next.h", "body"), ("kind", "library", "kind")])
def test_adjacent_marker_facts_are_not_borrowed(key, value, field):
    raw = MARKER + b"\n" + NEXT + f"\n// {key} {value}\n".encode()
    first, second = records(raw)
    assert getattr(first, field) == ""
    assert getattr(second, field) == value
    result = write_symbol(raw, first, SYMBOL)
    first_after, second_after = records(result)
    assert first_after.symbol == SYMBOL
    assert getattr(second_after, field) == value
    assert result.endswith(NEXT + f"\n// {key} {value}\n".encode())


@pytest.mark.parametrize("symbol", ["", "two words", "two\twords", "bad\nline", "x\x00", "x\x7f", "*/int/**/injected;/*", "//", "x\\", "x??/", None])
def test_reject_comment_and_line_splice_injection(symbol):
    raw = MARKER + b"\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="annotation-safe"):
        write_symbol(raw, row, symbol)


@pytest.mark.parametrize("facts", [b"// symbol _one\n// symbol _two\n", b"// symbol prose not a fact\n// symbol _one\n"])
def test_ambiguous_or_shadowed_fact_refuses(facts):
    raw = MARKER + b"\n" + facts + b"int f() {}\n"
    with pytest.raises(ValueError):
        write_symbol(raw, records(raw)[0], SYMBOL)


def test_stale_snapshot_and_changed_provenance_refuse():
    raw = MARKER + b"\n// TRIED: original note\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="current record snapshot"):
        write_symbol(raw.replace(b"original note", b"different note"), row, SYMBOL)
    with pytest.raises(ValueError, match="current record snapshot"):
        write_symbol(raw, replace(row, symbol="invented"), SYMBOL)
    with pytest.raises(ValueError, match="current record snapshot"):
        write_symbol(raw, replace(row, origin=None), SYMBOL)


def test_unterminated_marker_gets_a_separate_fact_line():
    assert write_symbol(MARKER, records(MARKER)[0], SYMBOL) == MARKER + b"\n// symbol    " + SYMBOL.encode() + b"\n"


@pytest.mark.parametrize("source", [
    b'const char *message = "FATAL ERROR";\n',
    b'const char *message = "R";\n',
    b'const char *message = "escaped R\\\"(text)";\n',
    b'// R"(comment)"\n',
    b'/* u8R"(comment)" */\n',
])
def test_ordinary_literals_and_comments_are_not_raw_string_source(source):
    raw = source + MARKER + b"\nint f() {}\n"
    result = write_symbol(raw, records(raw)[0], SYMBOL)
    assert result == raw.replace(MARKER + b"\n", MARKER + b"\n// symbol    " + SYMBOL.encode() + b"\n")


@pytest.mark.parametrize("prefix", [b"R", b"u8R", b"uR", b"UR", b"LR"])
def test_actual_raw_string_source_still_refuses(prefix):
    raw = b'const auto message = ' + prefix + b'"tag(raw)tag";\n' + MARKER + b"\nint f() {}\n"
    with pytest.raises(ValueError, match="raw-string source"):
        write_symbol(raw, records(raw)[0], SYMBOL)


@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
@pytest.mark.parametrize("tail", [b"", b"int f() {}\n"])
def test_insert_keeps_binding_adjacent_and_preserves_its_owner(newline, tail):
    raw = MARKER + newline + BINDING + (newline + tail if tail else b"")
    row, = records(raw)
    result = write_symbol(raw, row, SYMBOL)
    expected = MARKER + newline + BINDING + newline + b"// symbol    " + SYMBOL.encode() + newline + tail
    assert result == expected
    after, = records(result)
    assert binding_source.parse(result.decode(), [after]) == binding_source.parse(raw.decode(), [row])
    assert after.symbol == SYMBOL and after.byte_exact
    assert write_symbol(result, after, SYMBOL) == result


def test_inserting_symbol_does_not_reassign_neighbor_binding():
    raw = MARKER + b"\n" + BINDING + b"\nint f() {}\n" + NEXT + b"\n" + BINDING + b"\n"
    before = records(raw)
    result = write_symbol(raw, before[0], SYMBOL)
    after = records(result)
    old_bindings = binding_source.parse(raw.decode(), before)
    new_bindings = binding_source.parse(result.decode(), after)
    assert [old_bindings[r.line] for r in before] == [new_bindings[r.line] for r in after]
    assert result.endswith(NEXT + b"\n" + BINDING + b"\n")


@pytest.mark.parametrize("binding", [
    b"// RECOVERY_BINDING: v2 {}\n",
    b"// RECOVERY_BINDING: v1 {}\n",
    b"// intervening comment\n" + BINDING + b"\n",
])
def test_invalid_binding_is_not_silently_preserved_or_repaired(binding):
    raw = MARKER + b"\n" + binding + b"int f() {}\n"
    with pytest.raises(ValueError, match="binding"):
        write_symbol(raw, records(raw)[0], SYMBOL)


def test_pointer_assignment_is_code_not_a_symbol_fact():
    raw = b"int *symbol;\nvoid f() {\n" + MARKER + b"\n* symbol =\n 1;\n}\n"
    row, = records(raw)
    assert row.symbol == ""
    result = write_symbol(raw, row, SYMBOL)
    assert result == raw.replace(MARKER + b"\n", MARKER + b"\n// symbol    " + SYMBOL.encode() + b"\n")
    assert b"* symbol =\n 1;" in result


@pytest.mark.parametrize("old", [b"_old*/", b"_old/*", b"_old//"])
def test_unsafe_existing_fact_is_not_repaired_by_deleting_comment_syntax(old):
    raw = b"/*\n" + MARKER + b" FILE\n// symbol " + old + b"\nint f() {}\n"
    with pytest.raises(ValueError, match="existing symbol"):
        write_symbol(raw, records(raw)[0], SYMBOL)


@pytest.mark.parametrize("prefix", [b"R", b"u8R", b"LR"])
def test_raw_string_content_is_not_an_annotation(prefix):
    raw = b'const char *text = ' + prefix + b'"tag(\n' + MARKER + b' FILE\n)tag";\n'
    with pytest.raises(ValueError, match="raw-string"):
        write_symbol(raw, records(raw)[0], SYMBOL)


@pytest.mark.parametrize("splice", [b"\\", b"??/", b"\\ "])
@pytest.mark.parametrize("location", ["marker", "fact", "preceding"])
def test_line_splice_affected_source_refuses(splice, location):
    raw = MARKER + b"\nint f() {}\n"
    if location == "marker":
        raw = raw.replace(MARKER, MARKER + b" " + splice)
    elif location == "fact":
        raw = raw.replace(MARKER, MARKER + b"\n// symbol _old" + splice)
    else:
        raw = b"// continued " + splice + b"\n" + raw
    with pytest.raises(ValueError, match="line-spliced"):
        write_symbol(raw, records(raw)[0], SYMBOL)


@pytest.mark.parametrize("separator", [b"\v", b"\f", b"\x1c", "\u2028".encode()])
def test_unsupported_line_separators_refuse(separator):
    raw = MARKER + b"\n// prose" + separator + b"// more\nint f() {}\n"
    with pytest.raises(ValueError, match="line separator"):
        write_symbol(raw, records(raw)[0], SYMBOL)


def test_guarded_publication_refuses_changed_source(tmp_path):
    path = tmp_path / "fixture.cpp"
    raw = MARKER + b"\nint f() {}\n"
    path.write_bytes(raw)
    row, = read_text(raw.decode(), path)
    rendered = write_symbol(raw, row, SYMBOL)
    changed = raw.replace(b"return 0", b"return 1") + b"// concurrent edit\n"
    path.write_bytes(changed)
    with pytest.raises(ValueError):
        publish({path: raw}, {path: rendered}, staging_directory=tmp_path / "publication", validate=lambda: None)
    assert path.read_bytes() == changed
