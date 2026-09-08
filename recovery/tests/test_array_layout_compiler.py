"""Compiler interaction controls for the package's bounded-array verifier.

Synthetic source only. No original executable or game-specific class layouts.
"""

from pathlib import Path
import sys

import pytest

from match_recovery.application_layout import check_member
from match_recovery.codeview import class_layout
from match_recovery.layout_source import parse_members
from match_recovery.pe_coff import Coff
from recovery.environment.compile import compile_object


SOURCE = """
struct Rect { int left, top, right, bottom; };
struct Other { int a, b, c, d; };
struct Owner { int tag; Rect panes[2]; };
int object_size(Owner value) { return sizeof(value); }
"""


@pytest.mark.parametrize("declaration, larger_element, expected", [
    ("Rect panes[2];", False, "pass"),
    ("const Rect panes[2];", False, "pass"),
    ("Other panes[2];", False, "fail"),
    ("int panes[8];", False, "fail"),
    ("Rect *panes[8];", False, "fail"),
    ("Rect panes[1]; int padding[4];", False, "fail"),
    ("Rect panes[1];", True, "fail"),
])
def test_real_compiler_array_shape_not_just_offset_and_size(tmp_path, monkeypatch, declaration, larger_element, expected):
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    source = tmp_path / "arrays.cpp"
    text = SOURCE.replace("Rect panes[2];", declaration)
    if larger_element:
        text = text.replace("int left, top, right, bottom;", "int left, top, right, bottom; int extra[4];")
    source.write_text(text)
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    requirement, = parse_members("// RECOVERY_MEMBER: v4 Owner panes Rect[2] 0x4 0x20 0x10",
                                "code/arrays.cpp", "a" * 64, address_space="synthetic:offset")
    for frame_flags in ((), ("/Oy-",)):
        obj = Coff(compile_object(source, command, ("/c", "/O2", "/Gy", "/GR-", "/GX", "/Z7", *frame_flags)))
        layout = class_layout(obj, "Owner")
        assert layout["size"] == 36  # All damaged declarations retain the object's extent.
        row = check_member(requirement, layout)
        assert row["offset_result"] == "pass"
        assert row["result"] == expected, row
        if larger_element:
            assert row["type_result"] == row["array_size_result"] == "pass"
            assert row["array_count_result"] == "fail"
