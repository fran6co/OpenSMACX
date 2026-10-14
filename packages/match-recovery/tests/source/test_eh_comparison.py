"""Native comparison path must include every independently bound EH region."""
import struct
from dataclasses import replace

import pytest

from match_recovery.pe_coff import Coff
from match_recovery.source import read_text
from match_recovery.source.asm import (RegionCandidate, SpanComparisonRefused,
                                       compare_record, compare_subject, subject_asm)
from match_recovery.source.model import Tier
from test_eh_regions import ROOT, SPANS, _coff, _pe


def fixture(tmp_path, *, image_damage=None, original_cold_size=21):
    path = tmp_path / "owner.cpp"
    original_spans = (SPANS[0], (SPANS[1][0], SPANS[1][0] + original_cold_size))
    spans = ";".join(f"0x{low:08X}-0x{high:08X}" for low, high in original_spans)
    text = (f"// ORIGINAL: 0x{SPANS[0][0]:08X} owner {spans} FILE\n"
            f"// symbol {ROOT}\nvoid owner() {{}}\n")
    path.write_text(text)
    image = tmp_path / "original.exe"
    image.write_bytes(_pe(damage=image_damage, cold_size=original_cold_size))
    return read_text(text, path)[0], image


def change_code(raw, section, offset, value):
    changed = bytearray(raw)
    pointer, = struct.unpack_from("<I", raw, 20 + (section - 1) * 40 + 20)
    changed[pointer + offset] = value
    return bytes(changed)


def test_complete_owned_regions_measure_exact_in_the_real_comparison_path(tmp_path):
    row, image = fixture(tmp_path)
    candidate = subject_asm(_coff(), row, "trial")
    assert isinstance(candidate, RegionCandidate)
    result = compare_subject(row, image, candidate)
    assert result.verdict == Tier.BYTE_EXACT
    # 22 primary bytes +21 cold bytes, minus four-byte pointer/branch fields.
    assert result.compared_bytes + result.masked_bytes == 43
    assert result.masked_bytes == 16
    assert "0x00401100" in result.diagnostic


@pytest.mark.parametrize(("original_size", "object_size"), [(18, 18), (18, 21), (21, 18)])
def test_compact_and_wide_forms_bind_independently_but_length_change_is_a_miss(
        tmp_path, original_size, object_size):
    row, image = fixture(tmp_path, original_cold_size=original_size)
    candidate = subject_asm(_coff(cold_size=object_size), row, "trial")
    assert isinstance(candidate, RegionCandidate)
    result = compare_subject(row, image, candidate)
    if original_size == object_size:
        assert result.verdict == Tier.BYTE_EXACT
    else:
        assert result.verdict != Tier.BYTE_EXACT
        assert result.verdict != Tier.REFUSED


@pytest.mark.parametrize("region,offset,value", [(1, 21, 0xF8), (2, 2, 0xE0)])
def test_damage_to_main_or_cleanup_prevents_a_whole_function_claim(tmp_path, region, offset, value):
    row, image = fixture(tmp_path)
    candidate = subject_asm(change_code(_coff(), region, offset, value), row, "trial")
    result = compare_subject(row, image, candidate)
    assert result.verdict != Tier.BYTE_EXACT
    assert result.compared_bytes + result.masked_bytes == 43


def test_gap_damage_does_not_borrow_or_compare_unmapped_storage(tmp_path):
    row, image = fixture(tmp_path)
    changed = bytearray(image.read_bytes())
    changed[0x480] = 0xF8
    image.write_bytes(changed)
    result = compare_subject(row, image, subject_asm(_coff(), row, "trial"))
    assert result.verdict == Tier.BYTE_EXACT
    assert result.compared_bytes + result.masked_bytes == 43


@pytest.mark.parametrize("damage", ["wrong_cleanup_edge", "wrong_runtime", "missing_info_edge"])
def test_invalid_compiled_ownership_refuses_without_primary_only_fallback(tmp_path, damage):
    row, image = fixture(tmp_path)
    result = compare_record(row, image, [], ("trial",), compiler=lambda *args: _coff(damage=damage))
    assert result.verdict == Tier.REFUSED
    assert result.compared_bytes == 0


def test_another_flag_trial_can_supply_a_complete_supported_owner(tmp_path):
    row, image = fixture(tmp_path)
    calls = []
    def compiler(path, command, flags):
        calls.append(flags)
        return _coff(damage="wrong_runtime") if len(calls) == 1 else _coff()
    result = compare_record(row, image, [], ("first", "second"), compiler=compiler)
    assert result.verdict == Tier.BYTE_EXACT
    assert result.flags == "second"
    assert len(calls) == 2


@pytest.mark.parametrize("first", ["missing_child", "extra_child"])
def test_early_candidate_selection_refusal_does_not_skip_supported_later_trial(tmp_path, first):
    row, image = fixture(tmp_path)
    calls = []
    if first == "missing_child":
        changed = bytearray(_coff())
        # Keep a valid independent root COMDAT but no associated CODE child.
        # The former child's section remains associated initialized data.
        struct.pack_into("<I", changed, 20 + 40 + 36, 0x40401040)
        unsupported = bytes(changed)
        parsed = Coff(unsupported)
        assert parsed.code_sections(parsed.function(ROOT)) == (1,)
    else:
        unsupported = _coff(damage="extra_code_child")
    def compiler(path, command, flags):
        calls.append(flags)
        return unsupported if len(calls) == 1 else _coff()
    result = compare_record(row, image, [], ("first", "second"), compiler=compiler)
    assert result.verdict == Tier.BYTE_EXACT
    assert result.flags == "second"
    assert len(calls) == 2


@pytest.mark.parametrize("failure", ["absent", "ambiguous"])
def test_absent_or_ambiguous_mapped_symbol_in_all_trials_retains_stale_fact_error(tmp_path, failure):
    row, image = fixture(tmp_path)
    if failure == "absent":
        row = replace(row, symbol="?missing@Owner@@QAEHXZ")
    calls = []
    def compiler(*args):
        calls.append(args)
        return _coff(damage="ambiguous_root" if failure == "ambiguous" else None)
    with pytest.raises(ValueError, match="annotation is absent or ambiguous"):
        compare_record(row, image, [], ("first", "second"), compiler=compiler)
    assert len(calls) == 2


def test_compiled_candidate_cannot_be_reassigned_to_another_mapping(tmp_path):
    row, image = fixture(tmp_path)
    candidate = subject_asm(_coff(), row)
    with pytest.raises(SpanComparisonRefused, match="different original mapping"):
        compare_subject(replace(row, name="other"), image, candidate)


def test_original_unwind_damage_refuses_the_entire_function(tmp_path):
    row, image = fixture(tmp_path, image_damage="original_unwind_action")
    result = compare_record(row, image, [], ("trial",), compiler=lambda *args: _coff())
    assert result.verdict == Tier.REFUSED
    assert result.compared_bytes == 0
