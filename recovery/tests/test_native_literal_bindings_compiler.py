"""Real VC6 literal extents; synthetic reference data, not game acceptance."""

from dataclasses import replace
import hashlib
from pathlib import Path
import sys

import pytest

from match_recovery.literal_data import check_literals, literal_storage
from match_recovery.binary import BoundedTarget
from match_recovery.literal_source import LiteralBinding
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from recovery.environment.compile import compile_object
from recovery.environment.wine import VC6_ROOT
from test_pooled_string_constants import image_with_literal


@pytest.mark.parametrize("pooled", [False, True])
def test_real_compiler_literal_extent_preserves_embedded_nuls_and_wide_storage(tmp_path, monkeypatch, pooled):
    if not (VC6_ROOT / "BIN/CL.EXE").is_file():
        pytest.skip("VC6 is not installed")
    for variable in ("CL", "_CL_", "VC6_INCLUDE"):
        monkeypatch.delenv(variable, raising=False)
    source = tmp_path / "literals.cpp"
    source.write_text('extern "C" const char *first() { return "rb"; }\n'
                      'extern "C" const char *embedded() { return "a\\0b"; }\n'
                      'extern "C" const char *empty() { return ""; }\n'
                      'extern "C" const unsigned short *wide() { return L"rb"; }\n')
    wrapper = Path(__file__).resolve().parents[1] / "environment/wine.py"
    flags = ("/c", "/O2", "/Gy", "/GX", "/Z7", *(("/GF",) if pooled else ()))
    obj = Coff(compile_object(source, (sys.executable, str(wrapper), "--tool=cl", "/nologo"), flags))
    for function, expected in (("_first", b"rb\0"), ("_embedded", b"a\0b\0"),
                               ("_empty", b"\0"), ("_wide", b"r\0b\0\0\0")):
        subject = obj.function(function)
        _, index, _ = obj.sections[subject.section - 1].relocations[0]
        symbol = obj.symbols[index].name
        contents, observation = literal_storage(obj, symbol)
        assert contents == expected and observation["size"] == len(expected)
        raw = image_with_literal(b"\xa1\x00\x20\x40\x00" + b"\x90" * 11, expected[:-1], twice=True)
        pin = hashlib.sha256(raw).hexdigest()
        origin = Origin(pin, "pe:va", 0x402000)
        binding = LiteralBinding(origin, Span(0x402000, 0x402000 + len(expected)), symbol,
                                 "src/literals.cpp", 1, Origin(pin, "pe:va", 0x401000))
        targets, rows = check_literals([binding], {binding.unit: obj}, PeImage(raw, pin))
        assert rows[0]["definition"] == rows[0]["content"] == "pass"
        assert targets == {(binding.unit, symbol): BoundedTarget(binding.span)}
        wrong_extent = replace(binding, span=Span(origin.address, origin.address + len(expected) + 1))
        refused, rows = check_literals([wrong_extent], {binding.unit: obj}, PeImage(raw, pin))
        assert rows[0]["content"] == "fail" and refused == {(binding.unit, symbol): None}
