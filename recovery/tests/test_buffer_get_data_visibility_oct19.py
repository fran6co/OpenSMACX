"""Keep Buffer pixel acquisition visible out of line before inline consumers."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def test_zero_argument_get_data_has_one_out_of_line_definition_after_copy_and_pie():
    source = (ROOT / "src/buffer.cpp").read_text(encoding="utf-8")
    header = (ROOT / "src/buffer.h").read_text(encoding="utf-8")

    copy_marker = source.index("// ORIGINAL: 0x005DFF00")
    copy_definition = source.index(
        "int Buffer::copy(Buffer *buffer, int xCoord, int yCoord, int wx, int wy,"
    )
    pie_marker = source.index("// ORIGINAL: 0x005DEFE0")
    pie_definition = source.index("int Buffer::draw_pie(")
    getter_marker = source.index("// ORIGINAL: 0x005E3373")
    getter_definition = source.index("void *Buffer::get_data()")
    next_existing_body = source.index("// Patterned line rendering")

    assert copy_marker < copy_definition < pie_marker < pie_definition
    assert pie_definition < getter_marker < getter_definition < next_existing_body
    assert source.count("void *Buffer::get_data()") == 1
    assert "// body      src/buffer.cpp" in source[getter_marker:getter_definition]

    declaration = "MEASURED void *get_data();"
    assert header.count(declaration) == 1
    assert "MEASURED void *get_data() {" not in header
