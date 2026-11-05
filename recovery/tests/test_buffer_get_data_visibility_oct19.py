"""Keep canonical pixel calls separate from the shared inline operation."""

from pathlib import Path

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def test_zero_argument_get_data_has_one_separate_owner_and_shared_operation():
    source = (ROOT / "src/buffer.cpp").read_text(encoding="utf-8")
    pixel_owner = source
    header = (ROOT / "src/buffer.h").read_text(encoding="utf-8")

    copy_marker = source.index("// ORIGINAL: 0x005DFF00")
    copy_definition = source.index(
        "int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,"
    )
    pie_marker = source.index("// ORIGINAL: 0x005DEFE0")
    pie_definition = source.index("int Buffer::draw_pie(")
    getter_marker = pixel_owner.index("// ORIGINAL: 0x005E3373")
    getter_definition = pixel_owner.index("void *Buffer::get_data()")
    next_existing_body = source.index("// Patterned line rendering")

    assert copy_marker < copy_definition < pie_marker < pie_definition
    assert pie_definition < next_existing_body
    assert source.count("void *Buffer::get_data()") == 1
    assert pixel_owner.count("void *Buffer::get_data()") == 1
    getter = next(row for row in read_file(ROOT / "src/buffer.cpp")
                  if row.address == 0x005E3373)
    assert getter_marker < getter_definition
    assert "void *Buffer::get_data()" in getter.region
    assert "void *Buffer::get_data(int" not in getter.region
    assert "acquire_pixels()" not in source[copy_definition:next_existing_body]
    assert source[copy_definition:pie_marker].count("get_data()") == 10
    assert source[pie_definition:next_existing_body].count("get_data()") == 1

    declaration = "MEASURED void *get_data();"
    assert header.count(declaration) == 1
    assert "MEASURED void *get_data() {" not in header
    assert "void *Buffer::get_data()" not in header
    assert header.count("inline void *Buffer::acquire_pixels()") == 1
    assert "return acquire_pixels();" in pixel_owner[getter_definition:]
