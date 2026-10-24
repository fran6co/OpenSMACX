"""Actual retool dispatch against pinned thresholds, including invalid settings.

Service doubles isolate dispatch and callback order; these controls do not prove
stock-data reachability, the facility bitmask implementation or game acceptance.
"""

from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def actual_body():
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", (ROOT / "src/base.cpp").read_text(), flags=re.S)
    signature = "int __cdecl base_making(int production_id, int base_id)"
    assert text.count(signature) == 1
    start = text.index(signature)
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated product definition")


HARNESS = r"""
#include <cassert>
#include <vector>
#define __cdecl
struct { int retool_strictness; } Rules;
struct Base { int queue_production_id[10]; } Bases[1];
const int FAC_SKUNKWORKS=10, FacilityRepStart=65, FacilitySPStart=70;
bool skunk, completed;
std::vector<int> calls;
int has_fac_built(int facility, int base) {
    assert(base==0); calls.push_back(facility);
    return facility==10 ? skunk : completed;
}
// BODY
// MUTANT
int main() {
    const int production[]={-71,-70,-69,-65,-64,-1,0,7};
    // Rows are independently transcribed pinned branch outcomes, not the
    // former switch default. Negative nonzero settings follow category logic.
    const int category[]={1,1,2,2,3,3,0,0};
    const int clamp[]={-70,-70,-69,-65,-64,-1,0,7};
    const int settings[]={-2,-1,0,1,2,3,4,99};
    skunk=completed=false; Bases[0].queue_production_id[0]=7;
    for (int setting : settings) for (int i=0;i<8;++i) {
        Rules.retool_strictness=setting; calls.clear();
        int expected=setting==0 ? 0 : setting==2 ? clamp[i]
                     : setting>1 ? production[i] : category[i];
        assert(base_making(production[i],0)==expected);
        assert(calls==std::vector<int>({10}));
    }
    Rules.retool_strictness=-1;
    assert(base_making(0,0)==0 && base_making_old_switch(0,0)==1);
    Rules.retool_strictness=4;
    assert(base_making(-7,0)==-7 && base_making_old_switch(-7,0)==1);
    // Skunkworks clamps a positive strictness before queue/category dispatch.
    skunk=true; calls.clear();
    assert(base_making(-7,0)==3 && calls==std::vector<int>({10}));
    // A completed queued facility returns before dispatch, retaining call order.
    completed=true; Bases[0].queue_production_id[0]=-5; calls.clear();
    assert(base_making(-7,0)==-1 && calls==std::vector<int>({10,5}));
    // At the repeatable-facility threshold, do not query facility completion.
    Bases[0].queue_production_id[0]=-65; calls.clear();
    assert(base_making(-7,0)==3 && calls==std::vector<int>({10}));
}
"""


@pytest.mark.parametrize("compiler", tuple(filter(None, (shutil.which("g++"), shutil.which("clang++")))) or (None,))
def test_actual_retool_dispatch_and_former_switch_mutant(tmp_path, compiler):
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    header = (ROOT / "src/base.h").read_text()
    for name, value in (("FacilitySPStart", 70), ("FacilityRepStart", 65)):
        assert re.search(rf"const int {name}\s*=\s*{value}\s*;", header)
    body = actual_body()
    start = body.index("if (retool == 0)")
    mutant = body[:start] + """switch (retool) {
      case 0: return 0;
      case 1: return production_id >= 0 ? 0 :
          production_id > -FacilitySPStart ? (production_id > -FacilityRepStart) + 2 : 1;
      case 2: return production_id <= -FacilitySPStart ? -FacilitySPStart : production_id;
      case 3: return production_id;
      default: return 1;
    }
}"""
    mutant = mutant.replace("base_making(", "base_making_old_switch(", 1)
    source = tmp_path / "retool.cpp"
    binary = tmp_path / "retool"
    source.write_text(HARNESS.replace("// BODY", body).replace("// MUTANT", mutant))
    subprocess.run([compiler, "-std=c++11", "-O2", str(source), "-o", str(binary)],
                   check=True, capture_output=True, timeout=30)
    subprocess.run([str(binary)], check=True, capture_output=True, timeout=10)
