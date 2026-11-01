"""Complete actual rules-loader controls for the five-value ratio row.

The parser double enforces an independent row protocol before returning a
value, so the six-iteration damage case fails before an out-of-bounds store.
The six-read mutant bounds its destination separately so it cannot fail from
an invalid array subscript. These host controls do not establish native matching
or game acceptance.
"""
from pathlib import Path
import shutil
import subprocess

import pytest

from test_basic_rules_original_bounds import product_source


PRELUDE = r"""
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#define __cdecl
#define MEASURED
typedef int BOOL;
enum { TRIAD_LAND=0, TRIAD_SEA=1, TRIAD_AIR=2, AlphaxFileID=0 };
static int opens, fail_open, section;
static int basic_gets, basic_numbers, tech_calls;
static int world_gets, world_numbers, row, row_numbers;
static int number_rows[31];
// Independent parser input: 25 single-value rows, one five-value row,
// then the islands row. The ratios are signed and deliberately nonuniform.
static const int world_input[31] = {
    301,117,19,29,31,37,41,43,47,53,59,61,7,11,13,17,19,3,
    -97,-83,-71,-59,23,-47,4,
    (-2147483647-1),-103,0,211,2147483647,
    349
};
static int row_capacity(int value) { return value==26 ? 5 : 1; }
static int text_open(int file, const char *key) {
    assert(file==AlphaxFileID);
    ++opens;
    assert(opens<=2);
    assert(strcmp(key,opens==1 ? "RULES" : "WORLDBUILDER")==0);
    if(opens==2) {
        assert(basic_gets==62 && basic_numbers==67 && tech_calls==10);
    }
    if(opens==fail_open) return 1;
    section=opens;
    return 0;
}
static void text_get() {
    if(section==1) { ++basic_gets; return; }
    assert(section==2);
    if(row!=0) assert(row_numbers==row_capacity(row) && "ROW_COMPLETE");
    ++row; ++world_gets; row_numbers=0;
    assert(row<=27 && "ROW_COUNT");
}
static int text_item_number() {
    if(section==1) {
        ++basic_numbers;
        return basic_numbers%2 ? -basic_numbers*11 : basic_numbers*7;
    }
    assert(section==2 && row>=1 && row<=27);
    // Check before returning the value used by a product assignment. In
    // particular, a sixth ratio read aborts here, before indexing element 5.
    assert(row_numbers<row_capacity(row) && "ROW_CAPACITY");
    assert(world_numbers<31 && "NUMBER_COUNT");
    number_rows[world_numbers]=row;
    ++row_numbers;
    return world_input[world_numbers++];
}
static int tech_item() { assert(section==1); ++tech_calls; return 0; }
static void reset_parser(int failing_open) {
    opens=section=basic_gets=basic_numbers=tech_calls=0;
    world_gets=world_numbers=row=row_numbers=0;
    fail_open=failing_open;
    memset(number_rows,0,sizeof number_rows);
}
"""


HARNESS = r"""
static_assert(sizeof(int)==4,"DWORD parser representation");
static_assert(offsetof(RulesWorldbuilder,cont_size_ratio)==0x64,
              "original ratio traversal starts at +64");
static_assert(sizeof(WorldBuilder.cont_size_ratio)==20,
              "five DWORD ratio elements");
static_assert(offsetof(RulesWorldbuilder,islands)==0x78,
              "following islands word");
static_assert(sizeof(RulesWorldbuilder)==0x7c,"network transfer domain");
int main() {
    reset_parser(0);
    WorldBuilder=RulesWorldbuilder();
    for(int i=0;i<5;++i) WorldBuilder.cont_size_ratio[i]=-901-i;
    assert(read_basic_rules()==0);
    assert(opens==2 && basic_gets==62 && basic_numbers==67 && tech_calls==10);
    assert(world_gets==27 && world_numbers==31 && row==27 && row_numbers==1);
    assert(WorldBuilder.land_base==301 && WorldBuilder.land_mod==117);
    assert(WorldBuilder.deep_water==-97 && WorldBuilder.fungus==4);
    const int expected_ratios[5]={(-2147483647-1),-103,0,211,2147483647};
    for(int i=0;i<5;++i) {
        assert(WorldBuilder.cont_size_ratio[i]==expected_ratios[i] && "RATIO_VALUES");
        assert(number_rows[25+i]==26 && "RATIO_ROW");
    }
    assert(number_rows[24]==25 && number_rows[30]==27);
    assert(WorldBuilder.islands==349 && "ISLANDS_VALUE");

    // Both failures precede every WorldBuilder write, including all five
    // ratio words. Compare the complete object, not just one sentinel field.
    for(int failing_open=1;failing_open<=2;++failing_open) {
        reset_parser(failing_open);
        memset(&WorldBuilder,0x5a,sizeof WorldBuilder);
        unsigned char before[sizeof WorldBuilder];
        memcpy(before,&WorldBuilder,sizeof before);
        assert(read_basic_rules()==1);
        assert(opens==failing_open && world_gets==0 && world_numbers==0);
        assert(memcmp(before,&WorldBuilder,sizeof before)==0 && "OPEN_FAILURE_MUTATION");
    }
    return 0;
}
"""


@pytest.mark.parametrize("compiler_name", ["g++", "clang++"])
@pytest.mark.parametrize("damage,expected_failure", [
    (None, None),
    ("four_values", "ROW_COMPLETE"),
    ("six_values", "ROW_CAPACITY"),
    ("rotated_index", "RATIO_VALUES"),
])
def test_actual_loader_ratio_values_rows_and_failures(
    tmp_path: Path, compiler_name: str, damage: str | None,
    expected_failure: str | None,
) -> None:
    compiler = shutil.which(compiler_name)
    if compiler is None:
        pytest.skip(compiler_name + " unavailable")
    product = product_source()
    if damage in ("four_values", "six_values"):
        anchor = "for (int i = 0; i < 5; i++) {"
        assert product.count(anchor)==1, "ratio-loop anchor drift"
        product = product.replace(anchor, anchor.replace("< 5", "< 4" if damage=="four_values" else "< 6"), 1)
        if damage == "six_values":
            # Only the deliberate extra-read variant gets a bounded scratch
            # destination. Iterations 0..4 write their original positions;
            # iteration 5 must fail the independent parser-row contract,
            # regardless of which assignment operand the compiler evaluates
            # first. This tests the extra read without executing undefined
            # behavior or relying on a neighboring field being overwritten.
            store = "WorldBuilder.cont_size_ratio[i] = text_item_number();"
            assert product.count(store)==1
            product = product.replace(store, store.replace("[i]", "[i%5]"), 1)
    elif damage == "rotated_index":
        anchor = "WorldBuilder.cont_size_ratio[i] = text_item_number();"
        assert product.count(anchor)==1, "ratio-store anchor drift"
        # All indices remain inside the real five-element array.
        product = product.replace(anchor, anchor.replace("[i]", "[(i+1)%5]"), 1)
    marker = "BOOL __cdecl read_basic_rules() {"
    assert product.count(marker)==1
    product = product.replace(marker, "RulesBasic Rules;\nRulesWorldbuilder WorldBuilder;\n" + marker, 1)
    source = tmp_path / "ratios.cpp"
    binary = tmp_path / "ratios"
    source.write_text(PRELUDE + product + HARNESS)
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
