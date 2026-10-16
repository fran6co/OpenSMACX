"""Signed Dialog dimensions and defined low-32-bit grid arithmetic."""

from pathlib import Path
import shutil
import subprocess
import textwrap
import re

import pytest


ROOT = Path(__file__).resolve().parents[2]
HEADER = ROOT / "src/dialog.h"
SPRITEBOX = ROOT / "src/spritebox.cpp"
RADIOBUTTON = ROOT / "src/jackal/radiobutton.cpp"
CHECKBOX = ROOT / "src/jackal/checkbox.cpp"
EDITGROUP = ROOT / "src/editgroup.cpp"
LISTBOX = ROOT / "src/listbox.cpp"


def _normalized(path: Path) -> str:
    return " ".join(path.read_text(encoding="utf-8").split())


def _production_dimension_declarations() -> str:
    header = HEADER.read_text(encoding="utf-8")
    declarations = []
    for line in header.splitlines():
        stripped = line.strip()
        if re.fullmatch(r"(?:int|uint32_t) field_(?:2C|30|40|4C|64|68)_;", stripped):
            declarations.append(stripped)
        elif stripped == "int row_count_;":
            declarations.append(stripped)
    assert len([line for line in declarations if "field_" in line]) == 6
    assert "int row_count_;" in declarations
    return "\n".join(declarations)


def test_shared_fields_and_production_divisions_are_signed():
    header = HEADER.read_text(encoding="utf-8")
    for offset in ("2C", "30", "40", "4C", "64", "68"):
        assert f"int field_{offset}_;" in header

    spritebox = _normalized(SPRITEBOX)
    radiobutton = _normalized(RADIOBUTTON)
    checkbox = _normalized(CHECKBOX)
    editgroup = _normalized(EDITGROUP)
    listbox = _normalized(LISTBOX)
    assert "dialog.row_count_ = dialog.field_30_ / dialog.field_68_;" in spritebox
    assert "dialog.column_count_ = dialog.field_2C_ / dialog.field_64_;" in spritebox
    assert "dialog.field_2C_ / dialog.field_4C_" in radiobutton
    assert "width_numerator / dialog.field_4C_" in radiobutton
    assert "dialog.field_2C_) - static_cast<uint32_t>(dialog.field_40_) * 2u" in radiobutton
    assert "dialog.row_count_ = dialog.field_30_ / dialog.field_68_;" in radiobutton

    # These existing unsigned intermediates preserve the original dword
    # wrap before the resulting bit pattern is interpreted as signed.
    assert "uint32_t rows_numerator = static_cast<uint32_t>(dialog.row_count_) - 1u" in spritebox
    assert "static_cast<int32_t>(rows_numerator) / dialog.row_count_" in spritebox
    assert "uint32_t product = static_cast<uint32_t>(columns)" in radiobutton
    assert "static_cast<int32_t>(product) < dialog.item_list_.entry_count_" in radiobutton

    # Bind runtime wrap controls below to the actual production fragments in
    # both draw_item owners and the other consumers affected by the type edit.
    for source in (checkbox, radiobutton):
        assert "const uint32_t row_step = static_cast<uint32_t>(Dialog::field_68_) + static_cast<uint32_t>(cell_height);" in source
        assert "const uint32_t row_position = static_cast<uint32_t>(position) - static_cast<uint32_t>(column) * static_cast<uint32_t>(columns);" in source
        assert "const uint32_t y_bits = (static_cast<uint32_t>(cell_height >> 1) + static_cast<uint32_t>(extra_height)) + row_position * row_step;" in source
        assert "const uint32_t x_bits = (static_cast<uint32_t>(column_width) + static_cast<uint32_t>(cell_height)) * static_cast<uint32_t>(column) + static_cast<uint32_t>(Dialog::field_40_);" in source
        assert "static_cast<uint32_t>(x) + static_cast<uint32_t>(Dialog::field_64_)" in source
    assert "static_cast<uint32_t>(measured_edit) + static_cast<uint32_t>(dialog.field_40_) * 2u" in editgroup
    assert "static_cast<uint32_t>(dialog.field_2C_) - static_cast<uint32_t>(ScrollDefaultThickness)" in listbox
    assert "static_cast<uint32_t>(dialog.field_30_) - static_cast<uint32_t>(ScrollDefaultThickness)" in listbox


HARNESS = r"""
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdlib.h>
#include <type_traits>

static_assert(sizeof(int) == 4, "the recovered Dialog fields are 32-bit");
// This declaration block is inserted verbatim from src/dialog.h, so the
// exercised member types cannot drift from production declarations.
struct ProductionDialogFields {
PRODUCTION_MEMBER_DECLARATIONS
};
static_assert(std::is_same<decltype(ProductionDialogFields::field_2C_), int>::value, "2C");
static_assert(std::is_same<decltype(ProductionDialogFields::field_30_), int>::value, "30");
static_assert(std::is_same<decltype(ProductionDialogFields::field_40_), int>::value, "40");
static_assert(std::is_same<decltype(ProductionDialogFields::field_4C_), int>::value, "4C");
static_assert(std::is_same<decltype(ProductionDialogFields::field_64_), int>::value, "64");
static_assert(std::is_same<decltype(ProductionDialogFields::field_68_), int>::value, "68");

using Count = decltype(ProductionDialogFields::row_count_);
using WidthLimit = decltype(ProductionDialogFields::field_2C_);
using HeightLimit = decltype(ProductionDialogFields::field_30_);
using Spacing = decltype(ProductionDialogFields::field_40_);
using Columns = decltype(ProductionDialogFields::field_4C_);
using CellWidth = decltype(ProductionDialogFields::field_64_);
using CellHeight = decltype(ProductionDialogFields::field_68_);
using CellMargin = decltype(ProductionDialogFields::field_40_);

static int radio_wrap_width(WidthLimit width, Spacing spacing, Columns columns) {
    return (width - spacing * 2) / columns;
}

static int sprite_columns(WidthLimit width, CellWidth cell_width) {
    return width / cell_width;
}

static int sprite_rows(HeightLimit height, CellHeight cell_height) {
    return height / cell_height;
}

static int32_t signed_bits(uint32_t bits) {
    int32_t result;
    std::memcpy(&result, &bits, sizeof result);
    return result;
}

static int wrapped_sprite_rows(Count rows, int entries) {
    uint32_t numerator = static_cast<uint32_t>(rows) - 1u
                       + static_cast<uint32_t>(entries);
    return signed_bits(numerator) / rows;
}

static bool wrapped_radio_product_is_short(int columns, Count rows, int entries) {
    uint32_t product = static_cast<uint32_t>(columns)
                     * static_cast<uint32_t>(rows);
    return signed_bits(product) < entries;
}

static int32_t wrapped_draw_x(CellWidth width, int cell_height, int column,
                              CellMargin margin) {
    uint32_t bits = (static_cast<uint32_t>(width)
                   + static_cast<uint32_t>(cell_height))
                  * static_cast<uint32_t>(column)
                  + static_cast<uint32_t>(margin);
    return signed_bits(bits);
}

static int32_t wrapped_draw_y(CellHeight height, int cell_height,
                              int extra_height, int row_position) {
    uint32_t row_step = static_cast<uint32_t>(height)
                      + static_cast<uint32_t>(cell_height);
    uint32_t bits = (static_cast<uint32_t>(cell_height >> 1)
                   + static_cast<uint32_t>(extra_height))
                  + static_cast<uint32_t>(row_position) * row_step;
    return signed_bits(bits);
}

int main() {
    assert(radio_wrap_width(101, 4, 3) == 31);
    assert(radio_wrap_width(-17, 4, 3) == -8);
    assert(sprite_columns(-17, 5) == -3);
    assert(sprite_rows(19, 5) == 3);
    assert(sprite_rows(-13, 5) == -2);

    // The original unsigned dword additions/multiplications wrap first;
    // test their signed interpretation without signed overflow or division
    // by zero (including the INT_MIN / -1 undefined case).
    assert(wrapped_sprite_rows(0x40000000, 0x40000010) == -1);
    assert(wrapped_radio_product_is_short(0x40000001, 4, 5));
    assert(wrapped_draw_x(0x40000000, 0x40000000, 2, 7) == 7);
    assert(wrapped_draw_y(0x40000000, 0x40000000, 3, 2) == 0x20000003);
}
"""


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
def test_signed_dimensions_with_production_declarations(tmp_path, compiler_name):
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(f"{compiler_name} unavailable")
    source = tmp_path / "dialog_signed_dimensions.cpp"
    binary = tmp_path / "dialog_signed_dimensions"
    harness = HARNESS.replace(
        "PRODUCTION_MEMBER_DECLARATIONS", _production_dimension_declarations()
    )
    source.write_text(textwrap.dedent(harness), encoding="utf-8")
    subprocess.run(
        [
            compiler,
            "-std=c++11",
            str(source),
            "-o",
            str(binary),
        ],
        check=True,
        capture_output=True,
        timeout=60,
    )
    subprocess.run([str(binary)], check=True, capture_output=True, timeout=10)
