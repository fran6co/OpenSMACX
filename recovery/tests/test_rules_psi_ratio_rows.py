"""Complete actual rules-loader controls for the three two-value PSI rows.

Reuse the independent WorldBuilder stream/row protocol as well as the real
loader extraction. Only service doubles and deliberately damaged variants are
projected; the positive product declarations, clamp and loader are unchanged.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_basic_rules_original_bounds import product_source
from test_worldbuilder_ratio_array import PRELUDE, HARNESS


def parser_control():
    declarations = r"""
static const int psi_input[6]={(-2147483647-1),1001,29,-97,2147483647,43};
static int psi_numbers, psi_row_numbers, energy_numbers;
static int psi_rows[6];
"""
    prelude = PRELUDE.replace(
        "static int row_capacity(int value)",
        declarations + "\nstatic int row_capacity(int value)", 1,
    )
    old = "if(section==1) { ++basic_gets; return; }"
    new = r"""if(section==1) {
        if(basic_gets>=18 && basic_gets<=20)
            assert(psi_row_numbers==2 && "PSI_ROW_COMPLETE");
        psi_row_numbers=0;
        ++basic_gets;
        return;
    }"""
    assert prelude.count(old)==1
    prelude = prelude.replace(old,new,1)
    old = "return basic_numbers%2 ? -basic_numbers*11 : basic_numbers*7;"
    new = r"""if(basic_gets>=18 && basic_gets<=20) {
            assert(psi_numbers<6 && psi_row_numbers<2 && "PSI_ROW_CAPACITY");
            psi_rows[psi_numbers]=basic_gets;
            ++psi_row_numbers;
            return psi_input[psi_numbers++];
        }
        if(basic_gets==21) {
            assert(psi_numbers==6 && basic_numbers==25 && "PSI_STREAM_ALIGNMENT");
            ++energy_numbers;
            return 347;
        }
        return basic_numbers%2 ? -basic_numbers*11 : basic_numbers*7;"""
    assert prelude.count(old)==1
    prelude = prelude.replace(old,new,1)
    old = "fail_open=failing_open;"
    assert prelude.count(old)==1
    prelude = prelude.replace(old,old + "\n    psi_numbers=psi_row_numbers=energy_numbers=0;",1)
    checks = r"""
    static_assert(MaxTriadNum==3,"actual engine triad count");
    const unsigned int expected_atk[3]={1,29,1000};
    const unsigned int expected_def[3]={1000,1,43};
    for(int triad=0;triad<3;++triad) {
        assert(Rules.psi_combat_ratio_atk[triad]==expected_atk[triad] && "PSI_VALUES");
        assert(Rules.psi_combat_ratio_def[triad]==expected_def[triad] && "PSI_VALUES");
        assert(psi_rows[2*triad]==18+triad && psi_rows[2*triad+1]==18+triad);
    }
    assert(psi_numbers==6 && energy_numbers==1);
    assert(Rules.player_start_energy_reserve==347 && "FOLLOWING_ENERGY");
"""
    anchor = "assert(read_basic_rules()==0);"
    assert HARNESS.count(anchor)==1
    return prelude, HARNESS.replace(anchor,anchor + checks,1)


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage,expected_failure", [
    (None,None),
    ("swapped_arrays","PSI_VALUES"),
    ("reversed_row_index","PSI_VALUES"),
    ("two_rows","PSI_ROW_COMPLETE"),
])
def test_complete_loader_psi_rows_and_damage(
    tmp_path: Path, compiler_name: str, damage: str | None,
    expected_failure: str | None,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    product = product_source()
    if damage == "swapped_arrays":
        atk = "Rules.psi_combat_ratio_atk[triad]"
        defense = "Rules.psi_combat_ratio_def[triad]"
        assert product.count(atk)==product.count(defense)==1
        product = product.replace(atk,"PSI_SWAP_TEMP",1).replace(defense,atk,1).replace("PSI_SWAP_TEMP",defense,1)
    elif damage == "reversed_row_index":
        for field in ("atk","def"):
            old = "Rules.psi_combat_ratio_" + field + "[triad]"
            assert product.count(old)==1
            product = product.replace(old,old.replace("[triad]","[MaxTriadNum-1-triad]"),1)
    elif damage == "two_rows":
        old = "triad < MaxTriadNum"
        assert product.count(old)==1
        product = product.replace(old,"triad < 2",1)
    marker = "BOOL __cdecl read_basic_rules() {"
    assert product.count(marker)==1
    product = product.replace(marker,"RulesBasic Rules;\nRulesWorldbuilder WorldBuilder;\n" + marker,1)
    prelude,harness = parser_control()
    source = tmp_path / "psi.cpp"
    binary = tmp_path / "psi"
    source.write_text(prelude + product + harness)
    built = subprocess.run(
        [compiler,"-std=c++11","-Wall","-Wextra","-Werror",
         "-fsanitize=undefined","-fno-sanitize-recover=all",str(source),"-o",str(binary)],
        capture_output=True,text=True,timeout=60,
    )
    assert built.returncode==0, built.stdout + built.stderr
    result = subprocess.run([str(binary)],capture_output=True,text=True,timeout=10)
    if damage is None:
        assert result.returncode==0, result.stdout + result.stderr
    else:
        assert result.returncode!=0 and expected_failure in result.stderr, result.stderr
        assert "runtime error:" not in result.stderr, "damage relied on undefined behavior"
