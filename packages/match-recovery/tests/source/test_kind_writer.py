"""Portable controls for the proposed source.write_kind API."""

from dataclasses import replace

import pytest

from match_recovery import binding_source
from match_recovery.source import read_text, write_kind


PATH = "fixture.cpp"
MARKER = b"// ORIGINAL: 0x00401000 ?f@@QAEHXZ 0x00401000-0x00401006 BYTE_EXACT"
NEXT = b"// ORIGINAL: 0x00402000 _next 0x00402000-0x00402001 FILE"
BINDING = binding_source.render("Owner rationale", ["call at 0x00403000"]).encode()


def records(raw):
    text = raw.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n")
    return read_text(text, PATH)


@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
def test_insert_and_replace_roundtrip_preserves_identity_claims_lessons_and_bytes(newline):
    raw = (MARKER + newline + b"// TRIED: preserve this note" + newline
           + b"int f() {}" + newline)
    row, = records(raw)
    inserted = write_kind(raw, row, "game")
    assert inserted == MARKER + newline + b"// kind game" + newline + b"// TRIED: preserve this note" + newline + b"int f() {}" + newline
    after, = records(inserted)
    replaced = write_kind(inserted, after, "library")
    assert replaced == inserted.replace(b"kind game", b"kind library")
    current, = records(replaced)
    assert current.kind == "library"
    assert current.name == row.name and current.image_spans == row.image_spans
    assert current.byte_exact and current.ruled_out == row.ruled_out
    assert write_kind(replaced, current, "library") == replaced


def test_update_keeps_binding_attached_and_does_not_change_neighbor_kind():
    raw = (MARKER + b"\n" + BINDING + b"\nint f() {}\n"
           + NEXT + b"\n// kind library\n")
    before = records(raw)
    changed = write_kind(raw, before[0], "game")
    after = records(changed)
    assert [row.kind for row in after] == ["game", "library"]
    assert changed == raw.replace(BINDING + b"\nint f() {}", BINDING + b"\n// kind game\nint f() {}")
    assert binding_source.parse(raw.decode(), before) == binding_source.parse(changed.decode(), after)


@pytest.mark.parametrize("facts", [
    b"// kind game\n// kind library\n",
    b"// kind\n",
    b"// kind two words\n",
    b"// kind prose is not a fact\n",
    b"// kind game*/\n",
])
def test_duplicate_or_malformed_kind_fact_refuses(facts):
    raw = MARKER + b"\n" + facts + b"int f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError):
        write_kind(raw, row, "game")


def test_stale_or_edited_reader_snapshot_refuses():
    raw = MARKER + b"\n// TRIED: retained\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="current record snapshot"):
        write_kind(raw.replace(b"retained", b"changed"), row, "game")
    with pytest.raises(ValueError, match="current record snapshot"):
        write_kind(raw, replace(row, kind="invented"), "game")
    with pytest.raises(ValueError, match="current record snapshot"):
        write_kind(raw, replace(row, origin=None), "game")


@pytest.mark.parametrize("kind", ["", "two words", "tab\tvalue", "line\nfeed", "x\x00", "x\x7f", "x*/injected", "x\\", "x??/"])
def test_unsafe_new_kind_token_refuses(kind):
    raw = MARKER + b"\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="annotation-safe"):
        write_kind(raw, row, kind)


def test_adjacent_marker_fact_is_not_borrowed():
    raw = MARKER + b"\n" + NEXT + b"\n// kind library\n"
    first, second = records(raw)
    changed = write_kind(raw, first, "game")
    after_first, after_second = records(changed)
    assert (after_first.kind, after_second.kind) == ("game", "library")
    assert changed.endswith(NEXT + b"\n// kind library\n")


def test_existing_kind_edit_does_not_touch_non_utf8_neighbor_bytes():
    raw = MARKER + b"\n// note \xff\n// kind library\nint f() {}\n"
    row, = records(raw)
    assert write_kind(raw, row, "game") == raw.replace(b"kind library", b"kind game")
