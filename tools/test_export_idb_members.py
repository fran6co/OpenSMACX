#!/usr/bin/env python3
"""Unit tests for the IDA-database member reader.

The decoding is tested against hand-built byte strings rather than the
database, because the database is an ignored local artifact and a test that
skips without it proves nothing on a clean checkout.
"""

import unittest

import export_idb_members as exporter


class UnpackTest(unittest.TestCase):
    """IDA's packed dword, one width at a time."""

    def test_single_byte_below_0x80(self):
        self.assertEqual(exporter.unpack(b"\x18", 0), (0x18, 1))

    def test_two_bytes_below_0xc0(self):
        # 0x90 0x00 is the header word 0x1000 that Console and Palette carry.
        self.assertEqual(exporter.unpack(b"\x90\x00", 0), (0x1000, 2))

    def test_four_bytes_below_0xe0(self):
        self.assertEqual(exporter.unpack(b"\xc0\x09\x80\xa6", 0), (0x0980A6, 4))

    def test_five_byte_form_reads_four_raw_bytes(self):
        self.assertEqual(exporter.unpack(b"\xff\x12\x34\x56\x78", 0),
                         (0x12345678, 5))

    def test_index_is_honoured(self):
        self.assertEqual(exporter.unpack(b"\x00\x18", 1), (0x18, 2))


class NamedTypeTest(unittest.TestCase):
    """Only the spelled-out form decodes; everything else abstains."""

    def test_named_type_length_byte_counts_the_terminator(self):
        # `HFONT` arrives as 0x3D, length 6, then five characters AND a NUL.
        # Slicing six bytes and asking isidentifier() of "HFONT\0" answers no,
        # which silently dropped every type this decodes.
        self.assertEqual(exporter.named_type(b"=\x06HFONT\x00"), "HFONT")

    def test_ordinal_reference_is_refused(self):
        # `#` is IDA's ordinal marker and the rest is a packed ordinal, not a
        # name. 12,646 of the database's 13,078 members look like this.
        self.assertEqual(exporter.named_type(b"=\x04#\x81\x7c\x00"), "")

    def test_basic_integer_code_is_refused(self):
        self.assertEqual(exporter.named_type(b"\x07\x00"), "")

    def test_empty_and_truncated_are_refused(self):
        self.assertEqual(exporter.named_type(b""), "")
        self.assertEqual(exporter.named_type(b"=\x06HF"), "")

    def test_zero_length_is_refused(self):
        self.assertEqual(exporter.named_type(b"=\x00"), "")


class PlaceholderTest(unittest.TestCase):
    """The database names most members after their own offset, exactly as
    `src/` does, and counting those as named is what made it look like a
    naming oracle."""

    def test_offset_named_members_are_placeholders(self):
        for name in ("field_0", "field_9E8", "unk1", "unk", "gap0", "pad", ""):
            self.assertTrue(exporter.PLACEHOLDER.match(name), name)

    def test_real_names_are_not(self):
        for name in ("fontObj", "lineHeight", "graphicWin", "autoSound",
                     "vtable", "fileName"):
            self.assertFalse(exporter.PLACEHOLDER.match(name), name)


class RenderTest(unittest.TestCase):
    """The CSV is committed, so its ordering has to be total and stable or the
    staleness gate fails on a reordering that changed nothing."""

    def test_rows_sort_by_class_then_numeric_offset(self):
        rows = [
            {"class": "Win", "offset": "0x10", "name": "b", "size": 4,
             "type": ""},
            {"class": "Win", "offset": "0x9", "name": "a", "size": 4,
             "type": ""},
            {"class": "Buffer", "offset": "0x0", "name": "vtable", "size": 4,
             "type": ""},
        ]
        body = exporter.render(rows).splitlines()[1:]
        self.assertEqual([line.split(",")[0] for line in body],
                         ["Buffer", "Win", "Win"])
        # 0x9 before 0x10: sorted as numbers, not as the text "0x10" < "0x9".
        self.assertEqual([line.split(",")[1] for line in body[1:]],
                         ["0x9", "0x10"])

    def test_header_names_every_column(self):
        self.assertEqual(exporter.render([]).strip(),
                         "class,offset,name,size,type")


if __name__ == "__main__":
    unittest.main()
