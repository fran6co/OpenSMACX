"""Bounded actual-source controls for social_ai's signed numeric lanes.

The harness compiles only the production local declarations and three numeric
blocks. It does not reproduce social_ai's category search or establish full
function/runtime behavior. Inputs keep signed products and divisions in range.
"""

from pathlib import Path
import shutil
import subprocess
import textwrap

import pytest

from match_recovery.source import read_file


ROOT = Path(__file__).resolve().parents[2]


def _social_ai_body(source_override=None):
    if source_override is None:
        records = [row for row in read_file(ROOT / "src/faction.cpp")
                   if row.address == 0x005B4790]
        assert len(records) == 1
        source = records[0].region
    else:
        source = source_override
    start = source.index("void __cdecl social_ai(")
    opening = source.index("{", start)
    return source[opening + 1:]


def _through(source, first, last):
    start = source.index(first)
    end = source.index(last, start) + len(last)
    return source[start:end]


def _parts(source):
    sum_decl = next(line.strip() for line in source.splitlines()
                    if line.strip().endswith("unk_sum1 = 0;"))
    tech_decl = next(line.strip() for line in source.splitlines()
                     if line.strip().endswith("unk_val3 = 1;"))
    assert sum_decl in ("int unk_sum1 = 0;", "uint32_t unk_sum1 = 0;")
    assert tech_decl in ("int unk_val3 = 1;", "uint32_t unk_val3 = 1;")
    assert sum_decl.count("unk_sum1") == tech_decl.count("unk_val3") == 1
    declarations = sum_decl + "\n" + tech_decl
    economy = _through(
        source,
        "            int effic_val = PlayersData[faction_id].unk_46[",
        "            unk_val6 += econ_weight / unk_val3;",
    )
    morale = _through(
        source,
        "            int morale_val = range(soc_eff.values_[SocialEffect::MORALE], -4, 4) * 2;",
        "            unk_val6 += morale_weight;",
    )
    industry = _through(
        source,
        "                // industry\n",
        "                unk_val6 += industry_weight;",
    )
    return declarations, economy, morale, industry


_HOST_PREFIX = r"""
#include <stdint.h>
enum { MaxContinentNum = 128 };
enum Plan { PLAN_DEFENSIVE = 2 };
struct SocialEffect {
    enum EffectIndex { ECONOMY, EFFICIENCY, SUPPORT, TALENT, MORALE, POLICE,
                       GROWTH, PLANET, PROBE, INDUSTRY, RESEARCH };
    int values_[11];
};
struct PlayerData {
    int region_total_bases[128];
    uint8_t region_base_plan[128];
    int unk_46[9];
    int unk_47;
    int ai_fight;
    int current_num_bases;
    int unk_48;
    int pop_total;
} PlayersData[1];
int range(int value, int low, int high) {
    return value < low ? low : (value > high ? high : value);
}
"""


def _program(source, case, expected):
    declarations, economy, morale, industry = _parts(source)
    declarations = textwrap.dedent(declarations)
    if case in ("economy", "economy_asymmetric"):
        unk47 = -6 if case == "economy" else -10
        body = f"""
int actual() {{
    const int faction_id = 0;
    SocialEffect soc_eff = {{ {{ 0 }} }};
    int growth_val = 0, tech_val = 0, wealth_val = 0, power_val = 0;
    int output = 0, unk_val6 = 0;
    {declarations}
    unk_sum1 = 1;
    unk_val3 = 3;
    soc_eff.values_[SocialEffect::ECONOMY] = 2;
    PlayersData[0].unk_47 = {unk47};
    PlayersData[0].ai_fight = 0;
{textwrap.indent(economy, '    ')}
    return unk_val6;
}}
"""
    elif case == "morale":
        body = f"""
int actual() {{
    const int faction_id = 0;
    SocialEffect soc_eff = {{ {{ 0 }} }};
    int fight_val = -1, growth_val = 0, tech_val = 0;
    int wealth_val = 0, power_val = 0, output = 0, unk_val6 = 0;
    {declarations}
    unk_sum1 = 0;
    unk_val3 = 1;
    soc_eff.values_[SocialEffect::MORALE] = -2;
    PlayersData[0].current_num_bases = 8;
    PlayersData[0].unk_48 = 0;
{textwrap.indent(morale, '    ')}
    return unk_val6;
}}
"""
    elif case == "industry":
        body = f"""
int actual() {{
    const int faction_id = 0;
    SocialEffect soc_eff = {{ {{ 0 }} }};
    int fight_val = 0, tech_val = 0;
    int wealth_val = 0, power_val = 0, unk_val6 = 0;
    {declarations}
    unk_sum1 = 1;
    unk_val3 = 2;
    soc_eff.values_[SocialEffect::INDUSTRY] = -1;
    PlayersData[0].pop_total = 200;
{textwrap.indent(industry, '    ')}
    return unk_val6;
}}
"""
    else:
        raise AssertionError(case)
    # The extracted industry slice references the social-category pointer only
    # at the following social_upheaval statement; the slice ends first, so no
    # category algorithm or helper is required here.
    return _HOST_PREFIX + body + f"\nint main() {{ return actual() == {expected} ? 0 : 1; }}\n"


def _mutated(source, before, after):
    assert source.count(before) == 1, f"actual-source mutant anchor changed: {before}"
    return source.replace(before, after, 1)


def _compile_run(compiler, tmp_path, label, program, expected_pass):
    cpp = tmp_path / (label + ".cpp")
    exe = tmp_path / label
    cpp.write_text(program, encoding="utf-8")
    built = subprocess.run(
        [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", "-pedantic",
         str(cpp), "-o", str(exe)], capture_output=True, text=True, timeout=30)
    assert built.returncode == 0, built.stdout + built.stderr
    ran = subprocess.run([str(exe)], capture_output=True, text=True, timeout=10)
    if expected_pass:
        assert ran.returncode == 0, ran.stdout + ran.stderr
    else:
        assert ran.returncode != 0, "compiled source mutant escaped the numeric oracle"


def test_social_ai_signed_economy_morale_and_industry_actual_blocks(tmp_path):
    body = _social_ai_body()
    declarations, economy, morale, industry = _parts(body)
    assert "int unk_sum1 = 0;" in declarations
    assert "int unk_val3 = 1;" in declarations
    assert "econ_weight /= (unk_sum1 + 1);" in economy
    assert "unk_val6 += econ_weight / unk_val3;" in economy
    assert "(unk_sum1 + 1)" in morale
    assert "int unk_val3_temp = range(unk_val3, 1, 2);" in industry

    compilers = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    if not compilers:
        pytest.skip("GCC/Clang host compilers unavailable")

    for compiler in compilers:
        _compile_run(compiler, tmp_path, "economy_signed_" + Path(compiler).name,
                     _program(body, "economy", -1), True)
        _compile_run(compiler, tmp_path, "economy_asymmetric_" + Path(compiler).name,
                     _program(body, "economy_asymmetric", -1), True)
        bad_economy_parentheses = _mutated(
            body,
            "econ_weight = (PlayersData[faction_id].unk_47 + econ_val * 2 - 4) / (unk_sum1 + 1);",
            "econ_weight = PlayersData[faction_id].unk_47 + econ_val * 2 - 4 / (unk_sum1 + 1);",
        )
        _compile_run(compiler, tmp_path, "mutant_bad_economy_parentheses_" + Path(compiler).name,
                     _program(bad_economy_parentheses, "economy_asymmetric", -1), False)
        _compile_run(compiler, tmp_path, "morale_signed_" + Path(compiler).name,
                     _program(body, "morale", -3), True)
        _compile_run(compiler, tmp_path, "industry_signed_" + Path(compiler).name,
                     _program(body, "industry", -120), True)

        unsigned_tech = _mutated(body, "int unk_val3 = 1;", "uint32_t unk_val3 = 1;")
        _compile_run(compiler, tmp_path, "mutant_unsigned_tech_" + Path(compiler).name,
                     _program(unsigned_tech, "economy", -1), False)

        unsigned_count = _mutated(body, "int unk_sum1 = 0;", "uint32_t unk_sum1 = 0;")
        _compile_run(compiler, tmp_path, "mutant_unsigned_count_" + Path(compiler).name,
                     _program(unsigned_count, "morale", -3), False)

        unsigned_industry_clamp = _mutated(
            body,
            "int unk_val3_temp = range(unk_val3, 1, 2);",
            "uint32_t unk_val3_temp = range(unk_val3, 1, 2);",
        )
        _compile_run(compiler, tmp_path, "mutant_unsigned_industry_clamp_" + Path(compiler).name,
                     _program(unsigned_industry_clamp, "industry", -120), False)
