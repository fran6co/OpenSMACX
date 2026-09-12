"""Synthetic physical COFF/PE bytes; bindings are not selection or effect proof."""

from dataclasses import FrozenInstanceError
import hashlib
import json
import struct

import pytest

from match_recovery.binary import AbsoluteConstant, compare_bound_code
from match_recovery.model import Span
from match_recovery.pe_coff import Coff, PeImage

from test_binary import coff_fixture, pe_fixture


def physical(tmp_path, original, candidate, targets, *, kind=6, highlow=None):
    """Read real object relocations and PE HIGHLOW entries, not a mock proof."""
    obj_path, image_path = tmp_path / "trial.obj", tmp_path / "linked.exe"
    obj_path.write_bytes(coff_fixture(candidate, relocation=(1, 1, kind)))
    image_path.write_bytes(pe_fixture(original, absolute_site=highlow))
    obj = Coff(obj_path.read_bytes())
    raw = image_path.read_bytes()
    image = PeImage(raw, hashlib.sha256(raw).hexdigest())
    code, relocations, alignment = obj.subject(obj.function("_f"))
    span = Span(0x401000, 0x401000 + len(original))
    return compare_bound_code(span, image.read(span), code, relocations, targets,
                              original_absolute_sites=image.absolute_sites(span), alignment=alignment)


def operand(value, opcode=b"\xa1"):
    return opcode + struct.pack("<I", value & 0xFFFFFFFF) + b"\xc3"


@pytest.mark.parametrize("value,addend", [(0, 0), (7, 3), (7, -3), (0, -1), (0xFFFFFFFF, 1), (0x401000, 0)])
def test_constants_apply_actual_coff_addend_without_invented_highlow(tmp_path, value, addend):
    report = physical(tmp_path, operand(value + addend), operand(addend), {"1": AbsoluteConstant(value)})
    assert report["result"] == report["bytes"] == report["references"] == "pass"
    assert report["constant_absolute_relocations"] == [1]
    assert report["unexpected_constant_base_relocations"] == []
    assert report["extra_absolute_relocations"] == report["unresolved_absolute_relocations"] == []
    assert report["relocations"] == [{"offset": 1, "target": "1",
                                      "binding": {"kind": "absolute_constant", "value": value}, "result": "pass"}]
    json.dumps(report)


@pytest.mark.parametrize("value", [0, 19, 0x401000])
def test_pe_highlow_on_a_constant_is_a_measured_rebase_failure(tmp_path, value):
    report = physical(tmp_path, operand(value), operand(0), {"1": AbsoluteConstant(value)}, highlow=1)
    assert report["bytes"] == "pass"
    assert report["result"] == report["references"] == "fail"
    assert report["constant_absolute_relocations"] == report["unexpected_constant_base_relocations"] == [1]


@pytest.mark.parametrize("value", [0, 0x401000])
@pytest.mark.parametrize("highlow", [None, 1])
def test_plain_integer_addresses_keep_existing_highlow_contract(tmp_path, value, highlow):
    report = physical(tmp_path, operand(value), operand(0), {"1": value}, highlow=highlow)
    assert report["result"] == ("pass" if highlow else "fail")
    assert report["bytes"] == "pass"
    assert report["extra_absolute_relocations"] == ([] if highlow else [1])
    assert report["constant_absolute_relocations"] == report["unexpected_constant_base_relocations"] == []


@pytest.mark.parametrize("constant,addend", [(9, 0), (8, 1)])
def test_wrong_value_or_addend_is_not_masked(tmp_path, constant, addend):
    report = physical(tmp_path, operand(8), operand(addend), {"1": AbsoluteConstant(constant)})
    assert report["result"] == report["bytes"] == report["references"] == "fail"
    assert report["relocations"][0]["result"] == "fail" and 1 in report["differing_offsets"]


def test_constant_never_masks_a_nonoperand_opcode_difference(tmp_path):
    report = physical(tmp_path, operand(0, b"\xa1"), operand(0, b"\xa3"), {"1": AbsoluteConstant(0)})
    assert report["result"] == report["bytes"] == "fail" and report["references"] == "pass"
    assert report["differing_offsets"] == [0]


@pytest.mark.parametrize("opcode", [b"\xe8", b"\xe9"])
def test_relative_constant_is_unsupported_without_hiding_other_byte_failures(tmp_path, opcode):
    report = physical(tmp_path, operand(0xFFBF0FFB, b"\xe8"), operand(0, opcode),
                      {"1": AbsoluteConstant(0)}, kind=20)
    assert report["result"] == report["bytes"] == ("unknown" if opcode == b"\xe8" else "fail")
    assert report["references"] == "unknown"
    assert report["relocations"][0]["result"] == "unknown"
    assert "unsupported" in report["relocations"][0]["reason"]
    assert report["differing_offsets"] == ([] if opcode == b"\xe8" else [0])
    assert report["constant_absolute_relocations"] == []


def test_unresolved_constant_hint_stays_unknown_and_plain_zero_does_not_resolve_it(tmp_path):
    report = physical(tmp_path, operand(0), operand(0), {})
    assert report["result"] == "unknown" and report["unresolved_absolute_relocations"] == [1]
    assert report["constant_absolute_relocations"] == []
    report = physical(tmp_path, operand(0), operand(0), {"1": 0})
    assert report["result"] == "fail" and report["extra_absolute_relocations"] == [1]


@pytest.mark.parametrize("value", [True, False, -1, 2**32, 0.0, "0", None])
def test_constant_requires_a_true_unsigned_32_bit_integer(value):
    with pytest.raises(ValueError, match="unsigned 32-bit"):
        AbsoluteConstant(value)


def test_binding_is_frozen():
    binding = AbsoluteConstant(0)
    with pytest.raises(FrozenInstanceError):
        binding.value = 1


@pytest.mark.parametrize("value", [True, -1, 2**32, "0"])
def test_consumption_revalidates_a_forged_dataclass(tmp_path, value):
    binding = AbsoluteConstant(0)
    object.__setattr__(binding, "value", value)
    with pytest.raises(ValueError, match="unsigned 32-bit"):
        physical(tmp_path, operand(0), operand(0), {"1": binding})


@pytest.mark.parametrize("binding", [{"value": 0}, {"kind": "absolute_constant", "value": 0}, True, None])
def test_untyped_constant_descriptions_are_not_accepted(tmp_path, binding):
    with pytest.raises(ValueError, match="32-bit address"):
        physical(tmp_path, operand(0), operand(0), {"1": binding})


def test_constant_does_not_waive_an_unrelated_loader_relocation(tmp_path):
    report = physical(tmp_path, operand(0), operand(0), {"1": AbsoluteConstant(0)}, highlow=0)
    assert report["result"] == report["references"] == "fail"
    assert report["missing_absolute_relocations"] == [0]


def test_constant_does_not_waive_extent_failure(tmp_path):
    report = physical(tmp_path, operand(0), operand(0) + b"\xc3", {"1": AbsoluteConstant(0)})
    assert report["result"] == report["boundary"] == "fail"


def test_instance_attribute_cannot_disable_consumption_validation(tmp_path):
    binding = AbsoluteConstant(0)
    object.__setattr__(binding, "value", -1)
    object.__setattr__(binding, "__post_init__", lambda: None)
    with pytest.raises(ValueError, match="unsigned 32-bit"):
        physical(tmp_path, operand(0), operand(0), {"1": binding})


@pytest.mark.parametrize("highlow", [None, 1, 6])
def test_mixed_constant_and_image_address_keep_distinct_rebase_obligations(tmp_path, highlow):
    code = b"\xa1\0\0\0\0\xb8\0\0\0\0\xc3"
    raw = bytearray(coff_fixture(code))
    symbols_at = struct.unpack_from("<I", raw, 8)[0]
    relocations = struct.pack("<IIHIIH", 1, 1, 6, 6, 0, 6)
    raw[symbols_at:symbols_at] = relocations
    struct.pack_into("<I", raw, 8, symbols_at + len(relocations))
    struct.pack_into("<I", raw, 44, symbols_at)
    struct.pack_into("<H", raw, 52, 2)
    obj_path = tmp_path / "mixed.obj"
    obj_path.write_bytes(raw)
    obj = Coff(obj_path.read_bytes())
    image_path = tmp_path / "mixed.exe"
    image_path.write_bytes(pe_fixture(b"\xa1\x07\0\0\0\xb8\0\x10\x40\0\xc3", absolute_site=highlow))
    raw_image = image_path.read_bytes()
    image = PeImage(raw_image, hashlib.sha256(raw_image).hexdigest())
    candidate, references, alignment = obj.subject(obj.function("_f"))
    span = Span(0x401000, 0x40100B)
    report = compare_bound_code(span, image.read(span), candidate, references,
                                {"1": AbsoluteConstant(7), "0": 0x401000},
                                original_absolute_sites=image.absolute_sites(span), alignment=alignment)
    assert report["bytes"] == "pass"
    assert report["result"] == ("pass" if highlow == 6 else "fail")
    assert report["constant_absolute_relocations"] == [1]
    assert report["extra_absolute_relocations"] == ([] if highlow == 6 else [6])
    assert report["unexpected_constant_base_relocations"] == ([1] if highlow == 1 else [])
