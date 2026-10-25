"""Portable controls for the guarded existing source.write_body API."""

from dataclasses import replace

import pytest

from match_recovery import binding_source
from match_recovery.source import read_text, remove_body, write_body


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


@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
@pytest.mark.parametrize("with_binding", [False, True])
def test_explicit_insert_preserves_binding_claims_lessons_and_neighbors(newline, with_binding):
    prefix = MARKER + newline + (BINDING + newline if with_binding else b"")
    tail = (b"// TRIED: historical evidence" + newline + b"int f() {}" + newline
            + b"// ORIGINAL: 0x00402000 _next 0x00402000-0x00402001 FILE" + newline
            + b"// body src/next.h" + newline + b"int next() {}" + newline)
    raw = prefix + tail
    first, second = records(raw)
    changed = write_body(raw, first, "src/header.h", insert=True)
    assert changed == prefix + b"// body src/header.h" + newline + tail
    after, neighbor = records(changed)
    assert after.body == "src/header.h" and after.byte_exact
    assert after.ruled_out == first.ruled_out
    assert neighbor.body == second.body == "src/next.h"
    before_bindings = binding_source.parse(raw.decode(), [first, second])
    after_bindings = binding_source.parse(changed.decode(), [after, neighbor])
    assert [before_bindings.get(r.line) for r in (first, second)] == \
        [after_bindings.get(r.line) for r in (after, neighbor)]
    assert write_body(changed, after, "src/header.h", insert=True) == changed


def test_explicit_insert_handles_unterminated_marker_and_non_utf8_neighbors():
    row, = records(MARKER)
    assert write_body(MARKER, row, "src/header.h", insert=True) == \
        MARKER + b"\n// body src/header.h\n"
    raw = MARKER + b"\n// neighbor \xff\nint f() {}\n"
    row, = records(raw)
    assert write_body(raw, row, "src/header.h", insert=True) == \
        MARKER + b"\n// body src/header.h\n// neighbor \xff\nint f() {}\n"


@pytest.mark.parametrize("permission", [None, 0, 1, "true", [], {}])
def test_insert_requires_explicit_boolean(permission):
    raw = MARKER + b"\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="Boolean"):
        write_body(raw, row, "src/header.h", insert=permission)


@pytest.mark.parametrize("facts", [b"// body\n", b"// body src/../outside.h\n",
    b"// body src/a.h\n// body src/b.h\n"])
def test_insert_does_not_replace_malformed_or_duplicate_facts(facts):
    raw = MARKER + b"\n" + facts + b"int f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError):
        write_body(raw, row, "src/header.h", insert=True)


def test_insert_keeps_macro_raw_string_snapshot_and_unbound_refusals():
    samples = (
        b"#define FN \\\n" + MARKER + b"\nint f() {}\n",
        b'const char *s = R"tag(opaque)tag";\n' + MARKER + b"\nint f() {}\n",
        b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401006 UNBOUND\n",
    )
    for raw in samples:
        row, = records(raw)
        with pytest.raises(ValueError):
            write_body(raw, row, "src/header.h", insert=True)
    raw = MARKER + b"\n// TRIED: old\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="current record snapshot"):
        write_body(raw.replace(b"old", b"changed"), row, "src/header.h", insert=True)
    with pytest.raises(ValueError, match="relative path"):
        write_body(raw, row, "../escape.h", insert=True)


def test_remove_body_retires_only_selected_location_fact_and_preserves_peer():
    next_marker = b"// ORIGINAL: 0x00402000 _next 0x00402000-0x00402001 FILE"
    raw = (MARKER + b"\r\n" + BINDING + b"\r\n"
           + b"// symbol    ?f@@QAEHXZ\r\n"
           + b"// compile_context src/owner.cpp\r\n"
           + b"// LEVER: moved canonical body location\r\n"
           + b"// TRIED: retain the historical experiment\r\n"
           + b"// body src/old.h  \t\r\n// note \xff\r\nint f() {}\r\n"
           + next_marker + b"\r\n// body src/next.cpp\r\nint next() {}\r\n")
    first, second = records(raw)

    changed = remove_body(raw, first)

    assert changed == raw.replace(b"// body src/old.h  \t\r\n", b"", 1)
    after_first, after_second = records(changed)
    assert after_first.body == ""
    assert after_first.address == first.address
    assert after_first.name == first.name
    assert after_first.image_spans == first.image_spans
    assert after_first.mode is first.mode
    assert after_first.byte_exact == first.byte_exact
    assert after_first.symbol == first.symbol == "?f@@QAEHXZ"
    assert after_first.context == first.context == "src/owner.cpp"
    assert after_first.levers == first.levers
    assert after_first.ruled_out == first.ruled_out == \
        ("retain the historical experiment",)
    assert after_second.body == second.body == "src/next.cpp"
    old_bindings = binding_source.parse(raw.decode("utf-8", errors="replace"),
                                        [first, second])
    new_bindings = binding_source.parse(changed.decode("utf-8", errors="replace"),
                                        [after_first, after_second])
    assert [old_bindings.get(row.line) for row in (first, second)] == \
        [new_bindings.get(row.line) for row in (after_first, after_second)]
    with pytest.raises(ValueError, match="exactly one"):
        remove_body(changed, after_first)


@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
def test_remove_body_preserves_newline_style_and_unterminated_eof(newline):
    raw = MARKER + newline + b"// body src/old.h" + newline + b"int f() {}" + newline
    row, = records(raw)
    assert remove_body(raw, row) == MARKER + newline + b"int f() {}" + newline

    eof = MARKER + newline + b"// body src/old.h"
    row, = records(eof)
    assert remove_body(eof, row) == MARKER + newline


@pytest.mark.parametrize("facts", [
    b"// body src/one.h\n// body src/two.h\n",
    b"// body\n",
    b"// body src/../outside.h\n",
    b"",
])
def test_remove_body_refuses_missing_duplicate_or_malformed_facts(facts):
    raw = MARKER + b"\n" + facts + b"int f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError):
        remove_body(raw, row)


def test_remove_body_refuses_stale_snapshot_and_unsafe_source_contexts():
    raw = MARKER + b"\n// body src/old.h\n// TRIED: retained\nint f() {}\n"
    row, = records(raw)
    with pytest.raises(ValueError, match="current record snapshot"):
        remove_body(raw.replace(b"retained", b"changed"), row)
    with pytest.raises(ValueError, match="current record snapshot"):
        remove_body(raw, replace(row, body="src/other.h"))

    unsafe_sources = (
        b"#define BODY_MACRO \\\n" + MARKER + b"\n// body src/old.h\nint f() {}\n",
        b'const char *s = R"tag(opaque)tag";\n' + MARKER
        + b"\n// body src/old.h\nint f() {}\n",
        MARKER + b"\nint f() {} \\\n" + b"// body src/old.h\n",
    )
    for unsafe in unsafe_sources:
        row, = records(unsafe)
        with pytest.raises(ValueError):
            remove_body(unsafe, row)
