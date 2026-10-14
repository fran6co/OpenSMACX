"""Guard conversion placement in the actual coefficient producer body.

The executable establishes stored bit patterns, not an original C++ typename.
These controls enforce the reviewed source representation; they do not prove
the custom-register ABI or out-of-frame writes.
"""
import re

import pytest
from match_recovery.includes import COMMENTS_OR_STRINGS
from recovery.tests.test_movie_decoder_six_block_math_source import SRC, _function


def _actual_body():
    return _function((SRC / "movie_media_decoder.cpp").read_text(),
                     "DWORD __cdecl sub_5d0d82(")


def _require_producer_conversions(body):
    masked = COMMENTS_OR_STRINGS.sub(lambda m: " " * len(m[0]), body)
    compact = re.sub(r"\s+", "", masked)
    assert compact.count("coefficients[0]=movie_media_signed32(dc);") == 1, \
        "DC producer missing bit-preserving conversion"
    assert compact.count(
        "coefficients[coefficient_index]="
        "movie_media_signed32(amplitude*weights[coefficient_index]);") == 1, \
        "AC producer missing bit-preserving conversion"


def test_actual_signed_workspace_producers_preserve_bits():
    _require_producer_conversions(_actual_body())


@pytest.mark.parametrize(("before", "after", "diagnostic"), (
    ("coefficients[0] = movie_media_signed32(dc);",
     "coefficients[0] = static_cast<int>(dc);", "DC producer"),
    ("movie_media_signed32(amplitude * weights[coefficient_index]);",
     "static_cast<int>(amplitude * weights[coefficient_index]);", "AC producer"),
))
def test_implicit_host_wrapping_cannot_replace_reviewed_bit_conversion(
        before, after, diagnostic):
    body = _actual_body()
    assert body.count(before) == 1
    with pytest.raises(AssertionError, match=diagnostic):
        _require_producer_conversions(body.replace(before, after, 1))
