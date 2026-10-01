"""Execute the actual ranked technology producer with bounded service doubles."""
from pathlib import Path
import shutil
import subprocess
import pytest


def test_ranked_technology_offers_requests_and_project_discount(tmp_path):
    compiler = shutil.which("g++")
    if compiler is None:
        pytest.skip("host C++ compiler unavailable")
    root = Path(__file__).resolve().parents[2]
    text = (root / "src/faction.cpp").read_text()
    start = text.index("void __cdecl tech_analysis(")
    opening = text.index("{", start)
    depth = 0
    for end in range(opening, len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            body = text[start:end + 1]
            break
    else:
        raise AssertionError("unterminated actual producer")
    slots = "F7C0 F7EC F7F4 F800 F80C F814 FA18 FA1C FA20 FA28 FA44 FA6C FA84 FAA4 FAA8".split()
    declarations = "\n".join("int DiploScratch" + slot + ";" for slot in slots)
    fixture = r'''
#include <cassert>
#include <cstdint>
#include <climits>
#include <cstring>
#include <vector>
#define __cdecl
const int MaxSecretProjectNum=64, SP_Unbuilt=-1;
int GameRules, IsMultiplayerNet, RankingFactionIDUnk1, RankingFactionIDUnk2;
struct Player { char search_key[24]; } Players[8];
struct PlayerData { int diff_level; } PlayersData[8];
struct Base { unsigned char faction_id_current; int queue_production_id[10]; } Bases[512];
struct RulesFacility { int preq_tech; } Facility[134];
int BaseCurrentCount, Projects[64], Owned[8][89], Value[8][89];
bool NegativeScores;
std::vector<int> Calls;
int tech_val(int technology,int faction,int flag);
int base_project(int project) { return Projects[project]; }
int has_tech(int technology,int faction) { assert(technology>=0&&technology<89); Calls.push_back(faction*100+technology); return Owned[faction][technology]; }
// DECLARATIONS
int tech_val(int technology,int faction,int flag) {
    assert(flag==0);
    if(NegativeScores) { DiploScratchFAA4=INT_MIN; DiploScratchF7EC=INT_MIN; }
    return Value[faction][technology];
}
// BODY
void reset() {
    std::memset(Owned,0,sizeof Owned); std::memset(Value,0,sizeof Value);
    std::memset(Players,0,sizeof Players); std::memset(PlayersData,0,sizeof PlayersData);
    for(int i=0;i<64;++i)Projects[i]=-1;
    for(int i=0;i<134;++i)Facility[i].preq_tech=-99;
    BaseCurrentCount=0; GameRules=0; IsMultiplayerNet=0;
    RankingFactionIDUnk1=RankingFactionIDUnk2=7; Calls.clear();
    NegativeScores=false;
}
int main() {
    reset(); GameRules=0x2000000; DiploScratchF7F4=18;
    tech_analysis(1,2);
    assert(Calls.empty()&&DiploScratchF7F4==-1&&DiploScratchFAA8==-1&&DiploScratchF800==-1&&DiploScratchF80C==-1);
    assert(DiploScratchFA6C==-1&&DiploScratchFA84==-1&&DiploScratchF814==0&&DiploScratchFAA4==0);
    reset(); int values[]={35,10,20,30,40,15,30,5};
    for(int i=0;i<8;++i){Owned[1][i]=1;Value[2][i]=values[i];}
    tech_analysis(1,2);
    assert(DiploScratchFAA8==4&&DiploScratchFA18==0&&DiploScratchFA1C==6&&DiploScratchFA28==3);
    assert(DiploScratchF814==40&&DiploScratchFA44==35&&DiploScratchFA20==30&&DiploScratchF7C0==30);
    assert(DiploScratchF800==-1&&DiploScratchF80C==-1);
    reset(); Owned[2][1]=Owned[2][2]=Owned[2][3]=1;Value[1][1]=10;Value[1][2]=20;Value[1][3]=20;
    tech_analysis(1,2);
    assert(DiploScratchF800==3&&DiploScratchF80C==2&&DiploScratchFAA4==20&&DiploScratchF7EC==20);
    // Exact 64-project scan reaches its last entry, not the seed's 0x0c stride.
    reset(); Owned[2][1]=1; Value[1][1]=9; PlayersData[1].diff_level=3;
    Facility[133].preq_tech=1; BaseCurrentCount=1; Bases[0].faction_id_current=2;
    Bases[0].queue_production_id[0]=-133; tech_analysis(1,2);
    assert(DiploScratchF7F4==63&&DiploScratchF800==1&&DiploScratchFA6C==63&&DiploScratchFAA4==5);
    // A project-linked secondary request is suppressed; best remains.
    reset(); Owned[2][1]=Owned[2][2]=1;Value[1][1]=8;Value[1][2]=20;
    PlayersData[1].diff_level=5;Facility[70].preq_tech=1;
    tech_analysis(1,2);
    assert(DiploScratchF7F4==-2&&DiploScratchF800==2&&DiploScratchF80C==-1&&DiploScratchFA84==-2);
    reset(); Owned[2][1]=1;Value[1][1]=7;std::strcpy(Players[2].search_key,"UNIV");
    Facility[70].preq_tech=1;BaseCurrentCount=1;Bases[0].faction_id_current=2;Bases[0].queue_production_id[0]=-70;
    tech_analysis(1,2);assert(DiploScratchFA6C==0&&DiploScratchFAA4==4);
    // Service callbacks can leave negative score baselines. Native signed
    // halving truncates toward zero, and INC wraps at the i386 word boundary.
    reset(); NegativeScores=true; Owned[2][1]=1;Value[1][1]=-6;
    PlayersData[1].diff_level=5;Facility[70].preq_tech=1;
    tech_analysis(1,2);assert(DiploScratchFAA4==-2&&DiploScratchF800==1);
    reset(); NegativeScores=true; Owned[2][1]=1;Value[1][1]=INT_MAX;
    PlayersData[1].diff_level=5;Facility[70].preq_tech=1;
    tech_analysis(1,2);assert(DiploScratchFAA4==INT_MIN/2&&DiploScratchF800==1);
}
'''.replace("// DECLARATIONS", declarations).replace("// BODY", body)
    cpp = tmp_path / "tech_analysis.cpp"
    executable = tmp_path / "tech_analysis"
    cpp.write_text(fixture)
    built = subprocess.run([compiler,"-std=c++11","-Wall","-Wextra","-Werror",str(cpp),"-o",str(executable)],capture_output=True,text=True)
    assert built.returncode == 0, built.stderr
    ran = subprocess.run([str(executable)],capture_output=True,text=True)
    assert ran.returncode == 0, ran.stderr
