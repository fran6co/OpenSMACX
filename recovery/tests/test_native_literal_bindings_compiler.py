"""Real VC6 literal extents; synthetic reference data, not game acceptance."""

from dataclasses import replace
import hashlib
from pathlib import Path
import struct
import sys

import pytest

from match_recovery.literal_data import check_literals, literal_storage
from match_recovery.binary import BoundedTarget
from match_recovery.bound_objects import FunctionBinding, check_objects
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
        section = obj.sections[subject.section - 1]
        assert len(section.relocations) == 1 and section.relocations[0] == (1, index, 6)
        original_code = bytearray(section.data)
        struct.pack_into("<I", original_code, 1, 0x402000)
        raw = image_with_literal(original_code, expected[:-1])
        pin = hashlib.sha256(raw).hexdigest()
        origin = Origin(pin, "pe:va", 0x402000)
        binding = LiteralBinding(origin, Span(0x402000, 0x402000 + len(expected)), symbol,
                                 "src/literals.cpp", 1, Origin(pin, "pe:va", 0x401000))
        targets, rows = check_literals([binding], {binding.unit: obj}, PeImage(raw, pin))
        assert rows[0]["definition"] == rows[0]["content"] == "pass"
        assert targets == {(binding.unit, symbol): BoundedTarget(binding.span)}
        body = FunctionBinding(binding.evidence,
            (Span(0x401000, 0x401000 + len(original_code)),), function, binding.unit)
        unbound, = check_objects([body], {binding.unit: obj}, PeImage(raw, pin))
        assert unbound["result"] == unbound["references"] == "unknown"
        bound, = check_objects([body], {binding.unit: obj}, PeImage(raw, pin), literal_targets=targets)
        assert bound["result"] == "pass"
        wrong_extent = replace(binding, span=Span(origin.address, origin.address + len(expected) + 1))
        refused, rows = check_literals([wrong_extent], {binding.unit: obj}, PeImage(raw, pin))
        assert rows[0]["content"] == "fail" and refused == {(binding.unit, symbol): None}
