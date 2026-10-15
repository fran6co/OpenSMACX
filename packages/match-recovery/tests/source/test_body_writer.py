"""Portable controls for the guarded existing source.write_body API."""

from dataclasses import replace

import pytest

from match_recovery import binding_source
from match_recovery.source import read_text, write_body


PATH = "fixture.cpp"
MARKER = b"// ORIGINAL: 0x00401000 ?f@@QAEHXZ 0x00401000-0x00401006 BYTE_EXACT"
BINDING = binding_source.render("Owner rationale", ["header definition evidence"]).encode()


def records(raw):
    text = raw.decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n")
    return read_text(text, PATH)


@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
def test_replace_preserves_all_bytes_except_existing_body_path(newline):
    raw = (MARKER + newline + BINDING + newline
           + b"// body src/old.h  \t" + newline
           + b"// TRIED: keep historical note" + newline
           + b"int f() {}" + newline)
    row, = records(raw)
    changed = write_body(raw, row, "src/new.cpp")
    assert changed == raw.replace(b"src/old.h", b"src/new.cpp")
    after, = records(changed)
    assert after.body == "src/new.cpp"
    assert after.name == row.name and after.image_spans == row.image_spans
    assert after.byte_exact and after.ruled_out == row.ruled_out
    assert binding_source.parse(raw.decode(), [row]) == binding_source.parse(
        changed.decode(), [after])
    assert write_body(changed, after, "src/new.cpp") == changed


@pytest.mark.parametrize("facts", [
    b"// body src/one.h\n// body src/two.h\n",
    b"// body\n",
    b"// body needs.\n",
    b"// body src/../outside.h\n",
])
def test_duplicate_or_malformed_body_fact_refuses(facts):
    raw = MARKER + b"\n" + facts + b"int f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError):
        write_body(raw, row, "src/new.cpp")


def test_missing_fact_and_stale_or_edited_snapshot_refuse():
    raw = MARKER + b"\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="exactly one existing"):
        write_body(raw, row, "src/new.cpp")

    raw = MARKER + b"\n// body src/old.h\n// TRIED: retained\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="current record snapshot"):
        write_body(raw.replace(b"retained", b"changed"), row, "src/new.cpp")
    with pytest.raises(ValueError, match="current record snapshot"):
        write_body(raw, replace(row, body="src/invented.h"), "src/new.cpp")
    with pytest.raises(ValueError, match="current record snapshot"):
        write_body(raw, replace(row, origin=None), "src/new.cpp")


@pytest.mark.parametrize("body", [
    "", "outside.h", "/absolute/file.h", "C:/absolute/file.h", "scheme:/file.h",
    "src/../file.h", "src/./file.h",
    "src//file.h", "src\\file.h", "src/file name.h", "src/file\n.h",
    "src/file*/x.h", "src/file\\\\path.h", "src/x??/\n.h", "src/file.h\x00",
])
def test_unsafe_body_paths_refuse(body):
    raw = MARKER + b"\n// body src/old.h\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="relative path"):
        write_body(raw, row, body)


def test_adjacent_mapping_body_fact_is_not_borrowed():
    next_marker = b"// ORIGINAL: 0x00402000 _next 0x00402000-0x00402001 FILE"
    raw = MARKER + b"\n" + next_marker + b"\n// body src/next.h\n"
    first, second = records(raw)
    with pytest.raises(ValueError, match="exactly one existing"):
        write_body(raw, first, "src/new.cpp")
    assert second.body == "src/next.h"


def test_macro_and_raw_string_source_refuse():
    macro = (b"#define BODY_MACRO \\\n" + MARKER + b"\n// body src/old.h\nint f() {}\n")
    row, = records(macro)
    with pytest.raises(ValueError, match="#define"):
        write_body(macro, row, "src/new.cpp")

    one_line_macro = (b'#define BODY_MACRO "literal" ' + MARKER
                      + b"\n// body src/old.h\nint f() {}\n")
    row, = records(one_line_macro)
    with pytest.raises(ValueError, match="#define"):
        write_body(one_line_macro, row, "src/new.cpp")

    raw_string = (b'const char *s = R"tag(opaque)tag";\n' + MARKER
                  + b"\n// body src/old.h\nint f() {}\n")
    row, = records(raw_string)
    with pytest.raises(ValueError, match="raw-string"):
        write_body(raw_string, row, "src/new.cpp")


def test_existing_body_edit_preserves_non_utf8_neighbor_bytes():
    raw = MARKER + b"\n// note \xff\n// body src/old.h\nint f() {}\n"
    row, = records(raw)
    assert write_body(raw, row, "src/new.cpp") == raw.replace(
        b"src/old.h", b"src/new.cpp")
