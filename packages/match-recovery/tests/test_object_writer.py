"""Pure-byte controls for the generic object annotation API."""

from dataclasses import replace

import pytest

from match_recovery.model import Origin, Span
from match_recovery.object_source import ObjectBinding, parse_objects
from match_recovery.object_writer import write_object_bytes


PIN = "a" * 64
SPACE = "pe:va"
PATH = "src/data.cpp"


def binding(address: int, symbol: str, line: int, *, end: int | None = None,
            evidence: int | None = None) -> ObjectBinding:
    origin = Origin(PIN, SPACE, address)
    if end is None:
        return ObjectBinding(origin, symbol, PATH, line)
    return ObjectBinding(origin, symbol, PATH, line, Span(address, end),
                         Origin(PIN, SPACE, evidence if evidence is not None else address))


def current(raw: bytes) -> tuple[ObjectBinding, ...]:
    return parse_objects(raw.decode("utf-8", errors="surrogateescape"), PATH,
                         PIN, address_space=SPACE)


def test_add_v1_unknown_extent_and_v2_explicit_extent_round_trip_without_io():
    raw = b"int first;\r\nint second;\r\n"
    first = binding(0x007D3B74, "_fixture_arid", 1)
    second = binding(0x00685684, "_fixture_direction", 2,
                     end=0x00685779, evidence=0x005927D0)
    written = write_object_bytes(raw, expected=(), additions=(first, second),
                                 path=PATH, pin=PIN, address_space=SPACE)
    parsed = current(written)
    assert len(parsed) == 2
    assert {row.symbol: row.span for row in parsed} == {
        first.symbol: None,
        second.symbol: Span(0x00685684, 0x00685779),
    }
    assert all(row.evidence is None for row in parsed if row.span is None)
    assert b"\r\n" in written and b"\nint first;\r\n" in written
    assert raw.endswith(b"int second;\r\n")


@pytest.mark.parametrize("symbol", ["x\n// injected", "x//comment", "x\\\nnext", "bad token", "x\x7f"])
def test_rejects_symbol_injection(symbol):
    raw = b"int value;\n"
    proposed = binding(0x401000, symbol, 1)
    with pytest.raises(ValueError, match="annotation-safe"):
        write_object_bytes(raw, expected=(), additions=(proposed,), path=PATH,
                           pin=PIN, address_space=SPACE)


def test_rejects_stale_current_snapshot_and_existing_identity_conflicts():
    raw = b"// RECOVERY_OBJECT: v1 0x00401000 _old\nint old;\n"
    add = binding(0x00402000, "_new", 2)
    with pytest.raises(ValueError, match="current complete source snapshot"):
        write_object_bytes(raw, expected=(), additions=(add,), path=PATH,
                           pin=PIN, address_space=SPACE)
    existing = current(raw)
    same_address = binding(0x00401000, "_other", 2)
    with pytest.raises(ValueError, match="conflicts"):
        write_object_bytes(raw, expected=existing, additions=(same_address,),
                           path=PATH, pin=PIN, address_space=SPACE)
    same_symbol = binding(0x00402000, "_old", 2)
    with pytest.raises(ValueError, match="conflicts"):
        write_object_bytes(raw, expected=existing, additions=(same_symbol,),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_rejects_wrong_owner_image_or_insertion_anchor():
    raw = b"int value;\n"
    candidate = binding(0x401000, "_new", 2)
    with pytest.raises(ValueError, match="another source unit"):
        write_object_bytes(raw, expected=(), additions=(replace(candidate, unit="src/other.cpp"),),
                           path=PATH, pin=PIN, address_space=SPACE)
    wrong_origin = replace(candidate, origin=Origin("b" * 64, SPACE, 0x401000))
    with pytest.raises(ValueError, match="another image"):
        write_object_bytes(raw, expected=(), additions=(wrong_origin,), path=PATH,
                           pin=PIN, address_space=SPACE)
    with pytest.raises(ValueError, match="outside the source"):
        write_object_bytes(raw, expected=(), additions=(replace(candidate, line=3),),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_rejects_intra_batch_duplicates_overlap_and_existing_marker_shadow():
    raw = b"int first;\nint second;\n"
    same_address = (binding(0x400FF0, "_same_a", 1),
                    binding(0x400FF0, "_same_b", 2))
    with pytest.raises(ValueError, match="same original start"):
        write_object_bytes(raw, expected=(), additions=same_address, path=PATH,
                           pin=PIN, address_space=SPACE)
    one = binding(0x401000, "_one", 1, end=0x401010)
    duplicate = binding(0x40100F, "_two", 2, end=0x401020)
    with pytest.raises(ValueError, match="overlap"):
        write_object_bytes(raw, expected=(), additions=(one, duplicate), path=PATH,
                           pin=PIN, address_space=SPACE)
    conflict = replace(duplicate, origin=Origin(PIN, SPACE, 0x401008),
                       span=Span(0x401008, 0x401020))
    with pytest.raises(ValueError, match="overlap"):
        write_object_bytes(raw, expected=(), additions=(one, conflict), path=PATH,
                           pin=PIN, address_space=SPACE)
    marker_raw = b"// RECOVERY_OBJECT: v1 0x00403000 _old\nint old;\n"
    marker_binding = binding(0x00404000, "_new", 1)
    with pytest.raises(ValueError, match="shadow"):
        write_object_bytes(marker_raw, expected=current(marker_raw),
                           additions=(marker_binding,), path=PATH, pin=PIN,
                           address_space=SPACE)


def test_round_trip_keeps_existing_record_and_adds_start_only_array():
    raw = b"// RECOVERY_OBJECT: v1 0x00401000 _existing\nint existing;\n"
    parsed = current(raw)
    add = binding(0x007F65D0, "_fixture_moist", 2)
    written = write_object_bytes(raw, expected=parsed, additions=(add,),
                                 path=PATH, pin=PIN, address_space=SPACE)
    result = current(written)
    assert [row.symbol for row in result] == [parsed[0].symbol, add.symbol]
    assert result[0].origin == parsed[0].origin
    assert result[0].line == 1 and result[1].line == 2

@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
def test_eof_without_terminal_newline_keeps_source_and_separates_marker(newline):
    raw = b"int first;" + newline + b"int last;"
    result = write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 3),),
                                path=PATH, pin=PIN, address_space=SPACE)
    assert result.startswith(raw + newline)
    assert current(result)[0].symbol == "_new"


@pytest.mark.parametrize("raw", [b"/* comment\nint value;\n*/\n", b"/* unterminated\n"])
def test_comment_anchors_refuse_even_when_parser_would_read_the_inserted_marker(raw):
    with pytest.raises(ValueError, match="block comment"):
        write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 2),),
                           path=PATH, pin=PIN, address_space=SPACE)


@pytest.mark.parametrize("raw", [b"int x; \\\r\nint y;\r\n", b"int x; ??/\nint y;\n"])
def test_line_splices_refuse_including_crlf_and_trigraphs(raw):
    with pytest.raises(ValueError, match="line-spliced"):
        write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 2),),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_unbounded_start_cannot_overlap_an_independently_bounded_object():
    bounded = binding(0x403000, "_bounded", 1, end=0x403020)
    start = binding(0x403010, "_start", 2)
    with pytest.raises(ValueError, match="inside another"):
        write_object_bytes(b"int first;\nint second;\n", expected=(), additions=(bounded, start),
                           path=PATH, pin=PIN, address_space=SPACE)


@pytest.mark.parametrize("raw", [b'const char *s = R"(literal)";\n', b"\xef\xbb\xbfint x;\n"])
def test_unsupported_lexical_forms_refuse(raw):
    with pytest.raises(ValueError):
        write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 1),),
                           path=PATH, pin=PIN, address_space=SPACE)


def test_ordinary_literal_with_raw_like_text_is_not_a_raw_string():
    raw = b'const char *s = "FATAL ERROR";\n'
    written = write_object_bytes(raw, expected=(), additions=(binding(0x403000, "_new", 1),),
                                 path=PATH, pin=PIN, address_space=SPACE)
    assert written.endswith(raw)
