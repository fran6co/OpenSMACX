"""Every declared code region participates; gaps do not become bytes."""
import pytest

from match_recovery.source.asm import Listing
from match_recovery.source.model import Tier
from match_recovery.source.asm import compare_regions


def pair(original, candidate=None, base=0x401000, mask=frozenset()):
    return (Listing(original, base, mask),
            Listing(original if candidate is None else candidate, base, mask, flags="trial"))


def test_nonadjacent_regions_count_only_their_actual_bytes():
    result = compare_regions((pair(b"\x90\xc3"), pair(b"\xc3", base=0x408000)))
    assert result.verdict == Tier.BYTE_EXACT
    assert result.compared_bytes == 3
    assert result.original_instructions == result.matching_instructions == 3


@pytest.mark.parametrize("damaged", [0, 1])
def test_damage_in_either_region_prevents_exact_aggregate(damaged):
    pairs = [pair(b"\x90\xc3"), pair(b"\x90\xc3", base=0x408000)]
    pairs[damaged] = pair(b"\x90\xc3", b"\xf8\xc3", base=pairs[damaged][0].base)
    result = compare_regions(tuple(pairs))
    assert result.verdict == Tier.MISMATCH
    assert result.first_divergence == damaged * 2
    assert hex(pairs[damaged][0].base)[2:].upper() in result.context[0][0].upper()


def test_cold_region_constant_mismatch_keeps_constant_diagnostic():
    result = compare_regions((pair(b"\xc3"),
                              pair(b"\xb8\x01\0\0\0\xc3", b"\xb8\x02\0\0\0\xc3", base=0x408000)))
    assert result.verdict == Tier.SHAPE_EXACT
    assert result.differing_constants[0][0] == 1
    assert result.first_divergence == 1


def test_masks_are_counted_independently_not_by_global_offset_union():
    pairs = tuple(pair(b"\xb8\x01\0\0\0\xc3", base=base, mask=frozenset(range(1, 5)))
                  for base in (0x401000, 0x408000))
    result = compare_regions(pairs)
    assert result.verdict == Tier.BYTE_EXACT
    assert result.masked_bytes == 8
    assert result.compared_bytes == 4


def test_undecodable_secondary_region_refuses_instead_of_ignoring_bytes():
    result = compare_regions((pair(b"\xc3"), pair(b"\x0f", base=0x408000)))
    assert result.verdict == Tier.REFUSED
    assert result.compared_bytes == 0


@pytest.mark.parametrize("pairs", [(), (pair(b""),),
    (pair(b"\xc3", base=0x408000), pair(b"\xc3", base=0x401000)),
    (pair(b"\x90\xc3"), pair(b"\xc3", base=0x401001)),
    (pair(b"\xc3", mask=frozenset({1})),),
])
def test_invalid_or_incomplete_region_inputs_refuse(pairs):
    with pytest.raises(ValueError):
        compare_regions(pairs)


def test_mixed_flag_trials_cannot_form_one_function_match():
    one, two = pair(b"\xc3"), pair(b"\xc3", base=0x408000)
    two = (two[0], Listing(two[1].code, two[1].base, flags="other"))
    with pytest.raises(ValueError, match="same compiler flag trial"):
        compare_regions((one, two))


def test_only_exact_bounded_coff_alignment_fill_can_be_omitted():
    result = compare_regions((pair(b"\x90\x90\xc3", b"\x90\x90\xc3\xcc"),), alignments=(4,))
    assert result.verdict == Tier.BYTE_EXACT
    assert result.compared_bytes == 3
    assert "1 bytes of bounded COFF alignment fill" in result.diagnostic


@pytest.mark.parametrize("code,alignment", [
    (b"\x90\x90\xc3\xf8", 4),
    (b"\x90\x90\xc3\xcc\xcc\xcc\xcc\xcc", 4),
    (b"\x90\x90\xc3\xcc", 1),
])
def test_extra_code_or_unbounded_padding_cannot_be_omitted(code, alignment):
    result = compare_regions((pair(b"\x90\x90\xc3", code),), alignments=(alignment,))
    assert result.verdict != Tier.BYTE_EXACT
    assert result.data_divergence == 3


def test_candidate_padding_with_a_relocation_is_not_alignment_fill():
    original = Listing(b"\x90\x90\xc3", 0x401000)
    candidate = Listing(b"\x90\x90\xc3\xcc", 0x401000, frozenset({3}), flags="trial")
    result = compare_regions(((original, candidate),), alignments=(4,))
    assert result.verdict != Tier.BYTE_EXACT


def test_original_annotated_padding_is_never_trimmed():
    result = compare_regions((pair(b"\xc3\x90", b"\xc3\xcc"),), alignments=(4,))
    assert result.verdict != Tier.BYTE_EXACT
    assert result.compared_bytes == 2


def test_data_mismatch_in_secondary_region_retains_its_aggregate_offset():
    result = compare_regions((pair(b"\xc3"),
                              pair(b"\x90\x90\xc3", b"\x90\x90\xc3\xf8", base=0x408000)))
    assert result.verdict != Tier.BYTE_EXACT
    assert result.data_divergence == 4
