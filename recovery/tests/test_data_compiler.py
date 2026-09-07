"""Real compiler initialization controls over synthetic source/image data only."""

import hashlib
from pathlib import Path
import struct
import sys

import pytest

from match_recovery.codeview_data import data_layout
from match_recovery.model import Origin, Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import ObjectBinding
from match_recovery.pe_coff import Coff, PeImage
from recovery.environment.compile import compile_object
from test_object_data import data_image


def test_vc6_initialization_uses_actual_widths_and_symbolic_pointer_targets(tmp_path, monkeypatch):
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    for name in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(name, raising=False)
    source = tmp_path / "data.cpp"
    source.write_text("struct Pixel { Pixel() : value(0) {} int value; };\nPixel image;\nPixel tiles[3];\nPixel *selection = &image;\n"
                      "int count = 7;\nchar narrow = 1;\nunsigned short small = 2;\nint array[3] = {1,2,3};\n"
                      "int get() { return count; }\n")
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    command = (sys.executable, str(wrapper), "--tool=cl", "/nologo")
    flags = ("/c", "/O2", "/Gy", "/Z7")
    obj = Coff(compile_object(source, command, flags))
    assert any(section.name == ".debug$S" and i + 1 in obj.associations for i, section in enumerate(obj.sections))
    names = {"?narrow@@3DA": 1, "?small@@3GA": 2, "?count@@3HA": 4, "?array@@3PAHA": 12,
             "?image@@3UPixel@@A": 4, "?selection@@3PAUPixel@@A": 4, "?tiles@@3PAUPixel@@A": 12}
    assert {name: data_layout(obj, name)["size"] for name in names} == names
    raw = data_image(struct.pack("<I", 0x402000), absolute=0)
    pin = hashlib.sha256(raw).hexdigest()
    origin = Origin(pin, "pe:va", 0x401000)
    binding = ObjectBinding(origin, "?selection@@3PAUPixel@@A", "data.cpp", 1, Span(0x401000, 0x401004), origin)
    image = PeImage(raw, pin)
    def check(current, target=0x402000):
        return check_initial_data((binding,), {"data.cpp": current}, image, {"?image@@3UPixel@@A": target})[0]
    assert check(obj)["content"] == "pass"
    assert check(obj, 0x403000)["proof"]["references"] == "fail"
    source.write_text(source.read_text().replace("selection = &image", "selection = 0"))
    dropped = Coff(compile_object(source, command, flags))
    assert check(dropped)["content"] == "fail"
    source.write_text(source.read_text().replace("selection = 0", "selection = (Pixel *)0x402000"))
    fixed_address = check(Coff(compile_object(source, command, flags)))
    assert fixed_address["proof"]["bytes"] == "pass"
    assert fixed_address["proof"]["references"] == "fail"
