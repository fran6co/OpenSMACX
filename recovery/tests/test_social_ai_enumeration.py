"""Executable controls; original anchors 5B4831..483F, 5B4A5A..4ACD,
5B4A90/4AAB -> 5B5481.
Extracts product snippets rather than maintaining a second recovered loop.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest


def root_path():
    explicit = os.environ.get("OPENSMACX_SOURCE_ROOT")
    if explicit:
        return Path(explicit)
    for parent in Path(__file__).resolve().parents:
        if (parent / "src/faction.cpp").is_file():
            return parent
    raise RuntimeError("set OPENSMACX_SOURCE_ROOT")


def snippets():
    root = root_path()
    source = (root / "src/faction.cpp").read_text()
    source = source[source.index("void __cdecl social_ai("):]
    bound = source[source.index("uint32_t unk_val2 = 4;"):]
    bound = bound[:bound.index("// future pop growth")]
    prefix = source[source.index("for (uint32_t player = 0; player < unk_val2;"):]
    prefix = prefix[:prefix.index("do_all_non_input();")]
    header = (root / "src/faction.h").read_text()
    category = re.search(r"struct SocialCategory\s*\{.*?\n\};", header, re.S).group()
    return bound, prefix, category


def harness(bound, prefix, category):
    # Surround the real object with initialized storage, making the old
    # index-by-combination defect observable without relying on a crash.
    prefix = prefix.replace("SocialCategory soc_cat;", "Guarded storage; SocialCategory &soc_cat = storage.category;")
    return r'''
#include <cstdio>
typedef unsigned int uint32_t;
''' + category + r'''
struct Guarded {
    uint32_t before;
    SocialCategory category;
    uint32_t after[1024];
    Guarded() : before(0x13579BDFu) {
        for (int i=0;i<4;++i) category.models_[i]=0xDEADBEEFu;
        for (int i=0;i<1024;++i) after[i]=0x2468ACE0u;
    }
    bool intact() const {
        if (before!=0x13579BDFu) return false;
        for(int i=0;i<1024;++i) if(after[i]!=0x2468ACE0u) return false;
        return true;
    }
};
struct SocialEffect {};
struct PlayerFixture { int soc_anti_ideology_category, soc_anti_ideology_model; } Players[1];
struct RulesFixture { int preq_tech[4]; } SocialCategories[4];
const int MaxSocialCatNum=4;
int denied_tech;
bool has_tech_call(int tech, int) { return tech!=denied_tech; }

int trial(int anti_category,int anti_model,int blocked_category,int blocked_model) {
    const int faction_id=0;
    Players[0].soc_anti_ideology_category=anti_category;
    Players[0].soc_anti_ideology_model=anti_model;
    for(int c=0;c<4;++c) for(int m=0;m<4;++m)
        SocialCategories[c].preq_tech[m]=100+c*4+m;
    denied_tech=blocked_category<0 ? -1 : 100+blocked_category*4+blocked_model;
    bool seen[256]={false};
''' + bound + r'''
    if(unk_val2!=256) return 10;
''' + prefix + r'''
        if(player>=256 || !storage.intact()) return 11;
        for(int c=0;c<4;++c)
            if(soc_cat.models_[c] != ((player>>(2*c))&3u)) return 12;
        if(seen[player]) return 13;
        seen[player]=true; // Evaluation marker: invalid candidates must never reach here.
    }
    for(unsigned encoding=0;encoding<256;++encoding) {
        bool expected=true;
        for(int c=0;c<4;++c) {
            int m=(encoding>>(2*c))&3;
            if(m && ((c==anti_category && m==anti_model)
                || (c==blocked_category && m==blocked_model))) expected=false;
        }
        if(seen[encoding]!=expected) return 14;
    }
    return 0;
}
int main() {
    int result=trial(-1,-1,-1,-1); // Every one of 256 encodings evaluated.
    if(!result) result=trial(1,2,-1,-1); // Neither category alone nor model alone excludes.
    if(!result) result=trial(-1,-1,2,3); // Asymmetric axes expose transposition.
    if(!result) result=trial(1,2,2,3); // Both failure paths bypass evaluation.
    if(result) std::fprintf(stderr,"enumeration failure %d\n",result);
    return result;
}
'''


@pytest.mark.parametrize("damage", [None, "bound", "write_index", "anti_or", "axes", "invalid_fallthrough"])
def test_actual_social_ai_enumeration(tmp_path, damage):
    bound, prefix, category = snippets()
    mutations = {
        "bound": ("bound", "i > 0", "i >= 0"),
        "write_index": ("prefix", "soc_cat.models_[j] = model", "soc_cat.models_[player] = model"),
        "anti_or": ("prefix", "&& Players[faction_id].soc_anti_ideology_model", "|| Players[faction_id].soc_anti_ideology_model"),
        "axes": ("prefix", "SocialCategories[j].preq_tech[model]", "SocialCategories[model].preq_tech[j]"),
        "invalid_fallthrough": ("prefix", "if (invalid_combination) continue;", "if (invalid_combination) { /* old fallthrough */ }"),
    }
    if damage:
        target, old, new = mutations[damage]
        text = bound if target == "bound" else prefix
        if old not in text:
            pytest.fail("damage anchor absent; review the new product spelling")
        text = text.replace(old, new, 1)
        if target == "bound":
            bound = text
        else:
            prefix = text
    compiler = shutil.which("c++")
    if not compiler:
        pytest.fail("host C++ compiler is required; missing evidence is not a pass")
    candidate = tmp_path / "enumeration.cpp"
    candidate.write_text(harness(bound, prefix, category))
    executable = tmp_path / "enumeration"
    compiled = subprocess.run([compiler, "-std=c++98", "-O0", str(candidate), "-o", str(executable)], capture_output=True, text=True)
    assert compiled.returncode == 0, compiled.stderr
    checked = subprocess.run([str(executable)], capture_output=True, text=True)
    if damage is None:
        assert checked.returncode == 0, checked.stderr
    else:
        assert checked.returncode != 0, "damage escaped the executable oracle"
