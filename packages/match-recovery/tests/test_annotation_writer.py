"""Physical source insertion shared by typed annotation writers."""

import pytest

from match_recovery.source.annotation_writer import insert_annotation_lines


def test_insertion_preserves_source_bytes_and_selected_newline():
    raw = b"int a;\r\nint b;\r\n"
    marker = b"// RECOVERY_TEST: v1 item"
    assert insert_annotation_lines(raw, insertions=((2, marker),),
                                   marker_prefix=b"// RECOVERY_TEST:",
                                   label="test") == (
        b"int a;\r\n// RECOVERY_TEST: v1 item\r\nint b;\r\n")


@pytest.mark.parametrize("raw,reason", [
    (b"/* open\ninside\n*/\n", "inside a block comment"),
    (b"int a; \\\nint b;\n", "line-spliced source"),
    (b"\xef\xbb\xbfint a;\n", "BOM-prefixed"),
    (b'const char *s = R"x(\n)x";\n', "raw-string"),
])
def test_shared_placement_guards_refuse_ambiguous_source(raw, reason):
    line = 2 if "block" in reason else 1
    with pytest.raises(ValueError, match=reason):
        insert_annotation_lines(raw, insertions=((line, b"// RECOVERY_TEST: v1 x"),),
                                marker_prefix=b"// RECOVERY_TEST:", label="test")


@pytest.mark.parametrize("marker", [
    b"// RECOVERY_TEST: first\nsecond",
    b"// RECOVERY_TEST: first\rsecond",
    b"// RECOVERY_TEST: nul\x00byte",
    b"// RECOVERY_TEST: vertical\vtab",
    b"// RECOVERY_TEST: form\ffeed",
    b"// RECOVERY_TEST: unicode\xe2\x80\xa8separator",
])
def test_shared_placement_rejects_multi_line_or_control_markers(marker):
    with pytest.raises(ValueError, match="physical line"):
        insert_annotation_lines(b"int x;\n", insertions=((1, marker),),
                                marker_prefix=b"// RECOVERY_TEST:", label="test")


@pytest.mark.parametrize("prefix", [b"", b"// bad\n", b"// bad\r", b"// bad\x00"])
def test_shared_placement_rejects_invalid_marker_prefix(prefix):
    with pytest.raises(ValueError, match="marker prefix"):
        insert_annotation_lines(b"int x;\n", insertions=((1, b"// RECOVERY_TEST: x"),),
                                marker_prefix=prefix, label="test")
