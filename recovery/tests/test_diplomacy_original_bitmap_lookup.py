"""Source witnesses for pinned unguarded pricing reads, not invalid-index oracles.

Whole-body host controls cover valid bitmap indices separately. These witnesses
retain the original read-before-clamp sequence without predicting adjacent bytes
or claiming native runtime, exact matching, or game acceptance.
"""

import re

import pytest

from recovery.tests.test_pact_ascent import ROOT, definition


CASES = (
    ("buy_tech", "if (is_commlink != 0)", "value /= divisor;", """
        if (is_commlink != 0) value = diplo_mul(value, 3) / 2;
        int divisor = bit_count(
            GameDataStateStorage.fields_.game_tech_achieved_[DiploScratchF800]) - 1;
        if (divisor < 1) divisor = 1;
        if (divisor > 999) divisor = 999;
        value /= divisor;
    """),
    ("energy_trade", "if (great_satan(faction, 0))", "scale /= divisor;", """
        if (great_satan(faction, 0)) scale = diplo_mul(scale, 3) / 4;
        int divisor = diplo_clamp(bit_count(
            GameDataStateStorage.fields_.game_tech_achieved_[DiploScratchF800]) - 1, 1, 99);
        scale /= divisor;
    """),
)


def check_pricing_sequence(body, beginning, ending, expected):
    # These bounded pricing clauses contain no literals. Strip commentary only;
    # a source guard or conditional fallback must remain visible to this witness.
    code = re.sub(r"/\*.*?\*/|//[^\n]*", "", body, flags=re.S)
    start = code.index(beginning)
    end = code.index(ending, start) + len(ending)
    assert "".join(code[start:end].split()) == "".join(expected.split())


@pytest.mark.parametrize("name,beginning,ending,expected", CASES)
def test_original_lookup_precedes_population_clamp(name, beginning, ending, expected):
    source = (ROOT / "src/faction.cpp").read_text()
    body = definition(source, r"int __cdecl " + name + r"\([^)]*\)")
    check_pricing_sequence(body, beginning, ending, expected)


@pytest.mark.parametrize("limit", ("89", "700"))
@pytest.mark.parametrize("name,beginning,ending,expected", CASES)
def test_bitmap_fallback_damage_is_rejected(name, beginning, ending, expected, limit):
    source = (ROOT / "src/faction.cpp").read_text()
    body = definition(source, r"int __cdecl " + name + r"\([^)]*\)")
    lookup = "GameDataStateStorage.fields_.game_tech_achieved_[DiploScratchF800]"
    assert body.count(lookup) == 1
    guarded = "(DiploScratchF800 >= 0 && DiploScratchF800 < " + limit + " ? " + lookup + " : 0)"
    damaged = body.replace(lookup, guarded, 1)
    with pytest.raises(AssertionError):
        check_pricing_sequence(damaged, beginning, ending, expected)
