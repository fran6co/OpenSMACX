"""Serialized, test-owned controls for additional bounded array actions."""

import pytest

from match_recovery.source.eh_regions import bind_eh_regions, _array_adjustment, _decoded
import test_eh_vector_regions as fixture


@pytest.mark.parametrize("immediate", [0, 1, 0x180, 0x7FFFFFFF, 0x80000000, 0xFFFFFFFF])
def test_accumulator_subtraction_binds_and_compares_all_bits(tmp_path, immediate):
    built = fixture._build([("array", {"add_form": "subtract", "base_adjust": immediate}),
                            ("tail", 8)])
    _obj, regions, result = fixture._measure(built, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    expected = ((-immediate + 2**31) % 2**32) - 2**31
    assert regions[0].ownership.original_actions[0].base_adjustment == expected
    assert regions[0].ownership.compiled_actions[0].base_adjustment == expected


@pytest.mark.parametrize("values", [
    {}, {"count_width": 4, "stride_width": 4, "frame_width": 4, "frame_disp": -256},
])
def test_unadjusted_array_mixes_with_existing_action_kinds(tmp_path, values):
    built = fixture._build([("array", {"add_form": "none", "base_adjust": 0, **values}),
                            ("tail", 14), ("array", {})])
    _obj, regions, result = fixture._measure(built, tmp_path)
    assert result.verdict.value == "BYTE_EXACT", result
    original, compiled = (regions[0].ownership.original_actions[0],
                          regions[0].ownership.compiled_actions[0])
    assert original.base_adjustment == compiled.base_adjustment == 0
    assert len(original.transfers) == len(compiled.transfers) == 2
    assert original.transfers[0].operand_offset == compiled.transfers[0].operand_offset == 1


@pytest.mark.parametrize("form,base", [("none", 0), ("subtract", 0x180)])
@pytest.mark.parametrize("field,value", [("count", 4), ("stride", 16), ("frame_disp", -20)])
def test_changed_valid_array_operands_do_not_match(tmp_path, form, base, field, value):
    built = fixture._build([("array", {"add_form": form, "base_adjust": base})],
                            object_array=0, object_damage={field: value})
    _obj, _regions, result = fixture._measure(built, tmp_path)
    assert result.verdict.value == "MISMATCH", result


@pytest.mark.parametrize("form,base", [("none", 0), ("subtract", 0x180)])
@pytest.mark.parametrize("damage", [
    "missing_callback", "callback_wrong_kind", "missing_helper", "helper_wrong_kind",
    "nonzero_callback_addend", "nonzero_helper_addend", "extra_relocation",
    "callback_nonfunction", "helper_nonfunction",
])
def test_added_action_forms_preserve_relocation_requirements(form, base, damage):
    raw, image, spans, *_ = fixture._build(
        [("array", {"add_form": form, "base_adjust": base})], object_reloc_damage=damage)
    with pytest.raises(ValueError):
        bind_eh_regions(raw, fixture.ROOT_NAME, image, spans)


@pytest.mark.parametrize("form,base", [("none", 0), ("subtract", 0x180)])
@pytest.mark.parametrize("damage", ["call_to_jump", "missing_ret", "wrong_register", "wrong_load_base"])
def test_malformed_added_action_forms_refuse(form, base, damage):
    raw, image, spans, *_ = fixture._build(
        [("array", {"add_form": form, "base_adjust": base})],
        object_array=0, object_damage={"damage": damage})
    with pytest.raises(ValueError):
        bind_eh_regions(raw, fixture.ROOT_NAME, image, spans)


def test_equal_signed_adjustment_does_not_normalize_different_code(tmp_path):
    built = fixture._build([("array", {"base_adjust": -0x180})], object_array=0,
                            object_damage={"add_form": "subtract", "base_adjust": 0x180})
    _obj, regions, result = fixture._measure(built, tmp_path)
    assert regions[0].ownership.original_actions[0].base_adjustment == -0x180
    assert regions[0].ownership.compiled_actions[0].base_adjustment == -0x180
    assert result.verdict.value == "MISMATCH", result


def test_omitted_zero_adjustment_does_not_hide_instruction_difference(tmp_path):
    built = fixture._build([("array", {"base_adjust": 0})], object_array=0,
                            object_damage={"add_form": "none"})
    _obj, regions, result = fixture._measure(built, tmp_path)
    assert regions[0].ownership.original_actions[0].base_adjustment == 0
    assert regions[0].ownership.compiled_actions[0].base_adjustment == 0
    assert result.verdict.value == "MISMATCH", result


@pytest.mark.parametrize("code", [
    "81e980010000",  # SUB ECX, imm32: wrong destination.
    "35 80010000",  # XOR EAX, imm32: wrong operation.
    "83e804",       # SUB EAX, imm8: not the admitted accumulator encoding.
    "81e880010000", # SUB EAX, imm32 group encoding is not admitted yet.
    "66 2d 8001",   # SUB AX, imm16: wrong width.
])
def test_other_subtraction_shapes_are_not_guessed(code):
    instructions = _decoded(bytes.fromhex(code), 0)
    assert len(instructions) == 1
    assert _array_adjustment(instructions[0]) is None
