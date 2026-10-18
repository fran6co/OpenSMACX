"""Bounded signed API contracts, not original type proof or game acceptance.

Host C++98 controls use actual declarations/selected complete bodies. The native
real-header probe is opt-in, private/unlinked, and coordinator-lease-only; it is
not the application build and does not establish VC6 runtime equivalence.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from match_recovery.source import read_file
from test_infowin_setup_text import _function_end

ROOT = Path(__file__).resolve().parents[2]
SIGNED_RETURNS = (("faction", "aah_ooga", 0x539D40, "int,int"),
    ("faction", "get_mood", 0x53A090, "int"),
    ("veh", "morale_alien", 0x501350, "int,int"),
    ("veh", "best_reactor", 0x57EFA0, "int"),
    ("general", "bit_count_signed", 0x628AB0, "int"),
    ("base", "black_market", 0x4EA1F0, "int"),
    # Newly admitted after direct consuming signed JG at 5817C9/581833;
    # arithmetic/internal source types are deliberately outside this control.
    ("veh", "proto_cost", 0x5A5A60, "int,int,int,int,int"))


def declaration(unit, name):
    text = (ROOT / unit).read_text()
    # These subjects are free declarations, not members or overload families.
    matches = list(re.finditer(r"(?m)^(?:uint32_t|int|void|BOOL)\s+__cdecl\s+" + name + r"\s*\([^;{}]*\);", text))
    assert len(matches) == 1, (unit, name, "sole public declaration")
    return matches[0].group(0)


def body(unit, address, name):
    path = ROOT / unit
    row, = (r for r in read_file(path) if r.address == address)
    assert row.state.value == "implemented" and not row.extract_error
    text = path.read_text()
    matches = list(re.finditer(r"(?m)^(?:uint32_t|int|void)\s+__cdecl\s+" + name + r"\s*\(", text))
    assert len(matches) == 1
    start = matches[0].start()
    return text[start:_function_end(text, start)]


def host_compile(tmp_path, compiler, source, execute=False):
    source_path = tmp_path / "contract.cpp"
    source_path.write_text(source)
    output = tmp_path / ("contract" if execute else "contract.o")
    args = [compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror", str(source_path)]
    if not execute: args += ["-c"]
    built = subprocess.run([*args, "-o", str(output)], capture_output=True, text=True, timeout=30)
    if not execute or built.returncode: return built
    return subprocess.run([str(output)], capture_output=True, text=True, timeout=10)


def has_agenda_probe(decl):
    return "typedef unsigned int uint32_t;\n#define __cdecl\n" + decl + "\nint (*public_agenda)(int,int,int) = &has_agenda;\n"


def chosen_pointer_contracts():
    lines = ["typedef unsigned int uint32_t; typedef int BOOL;", "#define __cdecl"]
    for unit, name, address, args in SIGNED_RETURNS:
        lines += [declaration("src/" + unit + ".h", name),
                  body("src/" + unit + ".cpp", address, name).split("{", 1)[0].strip() + ";",
                  "int (*contract_" + name + ")(" + args + ") = &" + name + ";"]
    lines += [declaration("src/map.h", "quick_zoc"),
              body("src/map.cpp", 0x593830, "quick_zoc").split("{", 1)[0].strip() + ";",
              "void (*contract_zoc)(int,int,int,int,int,int*,int*) = &quick_zoc;",
              declaration("src/base.h", "has_fac_announced"),
              "BOOL (*unsigned_parameter_control)(uint32_t,uint32_t) = &has_fac_announced;"]
    return "\n".join(lines)


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_chosen_actual_header_and_definition_contracts(tmp_path, compiler):
    if not shutil.which(compiler): pytest.skip(compiler + " unavailable")
    result = host_compile(tmp_path, compiler, chosen_pointer_contracts())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_bit_count_signed_retains_shift_count_not_popcount(tmp_path, compiler):
    if not shutil.which(compiler): pytest.skip(compiler + " unavailable")
    actual = body("src/general.cpp", 0x628AB0, "bit_count_signed")
    preamble = "#include <cstdio>\n#define __cdecl\ntypedef unsigned int uint32_t;\n"
    main = r'''
int main(){
 const int inputs[]={0,1,2,3,7,8,0x3fffffff,0x7fffffff};
 const int outputs[]={0,1,2,2,3,4,30,31};
 for(int i=0;i<8;++i)if(bit_count_signed(inputs[i])!=outputs[i]){
  fprintf(stderr,"WITNESS: shift count not population count\n");return 43;
 }
 return 0;
}
'''
    result = host_compile(tmp_path, compiler, preamble + actual + main, execute=True)
    assert result.returncode == 0, result.stderr
    assert actual.count("input >>= 1;") == 1
    mutant = actual.replace("input >>= 1;", "input &= input - 1;")
    damaged = host_compile(tmp_path, compiler, preamble + mutant + main, execute=True)
    assert damaged.returncode == 43 and "WITNESS: shift count" in damaged.stderr
    # Negative input's arithmetic-shift nontermination remains untested, not fixed.


ZOC_HARNESS = r'''
#include <cstdio>
#include <cstdlib>
#define __cdecl
typedef unsigned int uint32_t;
typedef int BOOL;
enum { DTREATY_PACT=1 };
int VehMoveDeltaX[9]={0,1,1,2,3,3,4,6,999};
int VehMoveDeltaY[9]={2,1,2,3,2,4,3,3,999};
const int expected_x[8]={-1,0,0,1,2,2,3,5};
const int expected_y[8]={0,-1,0,1,0,2,1,1};
int stage=-1, mode, source_ocean_reads, owner_reads, treaty_reads;
void require(bool ok,const char*w){if(!ok){fprintf(stderr,"WITNESS: %s\n",w);exit(43);}}
int xrange(int x){++stage;require(stage<8&&x==expected_x[stage],"signed source coordinate and eight-row order");return x;}
BOOL on_map(int x,int y){require(x==expected_x[stage]&&y==expected_y[stage],"paired candidate coordinates");return x>=0&&y>=0;}
int veh_who(int x,int y){
 require(x==expected_x[stage]&&y==expected_y[stage],"owner receives candidate coordinates");++owner_reads;
 if(mode==0||(stage==7&&mode==1))return -1;
 return stage;
}
BOOL is_ocean(int x,int y){
 if(stage==-1){require(x==-1&&y==-2,"source ocean read precedes candidate scan");++source_ocean_reads;return 0;}
 require(x==expected_x[stage]&&y==expected_y[stage],"ocean receives candidate coordinates");return stage==3;
}
int has_treaty(int faction,int owner,int mask){
 require(faction==2&&owner==stage&&mask==DTREATY_PACT,"treaty receiver and pact mask");++treaty_reads;return stage==4;
}
int x_dist(int a,int b){return a-b;}
// ZOC_DECLARATION
// ZOC_BODY
void reset(int chosen){mode=chosen;stage=-1;source_ocean_reads=owner_reads=treaty_reads=0;}
int main(){
 int x=777,y=888;
 reset(0);quick_zoc(-1,-2,2,0,0,&x,&y);
 require(x==777&&y==888,"no eligible candidate leaves both outputs untouched");
 require(stage==7&&source_ocean_reads==1&&owner_reads==6&&treaty_reads==0,"invalid map and negative owner gates");
 reset(1);quick_zoc(-1,-2,2,0,0,&x,&y);
 require(x==3&&y==1,"equal proximity selects later candidate");
 require(treaty_reads==3,"self ocean and pact gates short circuit");
 reset(2);quick_zoc(-1,-2,2,0,0,&x,&y);
 require(x==5&&y==1,"greatest proximity not nearest candidate");
 return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", ["none", "tie", "clear-output"])
def test_complete_quick_zoc_signed_coordinates_and_output_paths(tmp_path, compiler, damage):
    if not shutil.which(compiler): pytest.skip(compiler + " unavailable")
    actual = body("src/map.cpp", 0x593830, "quick_zoc")
    if damage == "tie":
        assert actual.count("proximity >= search_zoc") == 1
        actual = actual.replace("proximity >= search_zoc", "proximity > search_zoc")
    elif damage == "clear-output":
        assert actual.count("int search_zoc = -1;") == 1
        actual = actual.replace("int search_zoc = -1;", "*x_zoc = *y_zoc = 0; int search_zoc = -1;")
    source = ZOC_HARNESS.replace("// ZOC_DECLARATION", declaration("src/map.h", "quick_zoc")).replace("// ZOC_BODY", actual)
    result = host_compile(tmp_path, compiler, source, execute=True)
    assert result.returncode == (0 if damage == "none" else 43), result.stderr
    if damage != "none":
        witness = "equal proximity" if damage == "tie" else "no eligible candidate"
        assert "WITNESS: " + witness in result.stderr


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
def test_actual_has_agenda_public_contract_and_old_header_damage(tmp_path, compiler):
    if not shutil.which(compiler): pytest.skip(compiler + " unavailable")
    decl = declaration("src/faction.h", "has_agenda")
    result = host_compile(tmp_path, compiler, has_agenda_probe(decl))
    assert result.returncode == 0, result.stderr
    old = "uint32_t __cdecl has_agenda(uint32_t faction_id, uint32_t faction_id_with, uint32_t agenda);"
    damaged = host_compile(tmp_path, compiler, has_agenda_probe(old))
    assert damaged.returncode != 0 and "has_agenda" in damaged.stderr


AGENDA_HARNESS = r'''
#include <cstdio>
#include <cstdlib>
typedef unsigned int uint32_t;
#define __cdecl
struct Player { int diplo_agenda[8]; } PlayersData[8];
// DECLARATION
// COMPLETE_BODY
void require(bool ok,const char*w){if(!ok){fprintf(stderr,"WITNESS: %s\n",w);exit(43);}}
int main(){
 int (*contract)(int,int,int)=&has_agenda;
 PlayersData[2].diplo_agenda[5]=0x540;
 PlayersData[5].diplo_agenda[2]=0x200;
 require(contract(2,5,0x440)==0x440,"agenda preserves mask bits not boolean");
 require(contract(5,2,0x440)==0,"agenda faction direction");
 PlayersData[2].diplo_agenda[5]=-1;
 require(contract(2,5,-2147483647)==-2147483647,"agenda preserves signed bit result");
 require(contract(2,5,0)==0,"agenda zero mask");
 return 0;
}
'''


@pytest.mark.parametrize("compiler", ["g++", "clang++"])
@pytest.mark.parametrize("damage", [False, True])
def test_complete_has_agenda_body_preserves_mask_result(tmp_path, compiler, damage):
    if not shutil.which(compiler): pytest.skip(compiler + " unavailable")
    actual = body("src/faction.cpp", 0x5591E0, "has_agenda")
    if damage:
        assert actual.count(" & agenda;") == 1
        actual = actual.replace(" & agenda;", " && agenda;")
    source = AGENDA_HARNESS.replace("// DECLARATION", declaration("src/faction.h", "has_agenda")).replace("// COMPLETE_BODY", actual)
    result = host_compile(tmp_path, compiler, source, execute=True)
    assert result.returncode == (43 if damage else 0), result.stderr
    if damage: assert "WITNESS: agenda preserves mask bits not boolean" in result.stderr


def real_header_probe():
    prefix = b'''#include "stdafx.h"
#include "faction.h"
#include "veh.h"
#include "general.h"
#include "base.h"
#include "map.h"
void signed_api_header_probe() {
    int (__cdecl *agenda)(int,int,int) = &has_agenda;
    BOOL (__cdecl *unsigned_control)(uint32_t,uint32_t) = &has_fac_announced;
    (void)agenda; (void)unsigned_control;
'''
    additions = []
    for _, name, _, args in SIGNED_RETURNS:
        additions.append("    int (__cdecl *probe_" + name + ")(" + args + ") = &" + name + ";\n    (void)probe_" + name + ";\n")
    additions.append("    void (__cdecl *zoc)(int,int,int,int,int,int*,int*) = &quick_zoc;\n    (void)zoc;\n}\n")
    return prefix + "".join(additions).encode()


def test_native_real_headers_reject_private_has_agenda_regression():
    if os.environ.get("OPENSMACX_NATIVE_SIGNED_API") != "1":
        pytest.skip("requires coordinator-held native compiler lease")
    from match_recovery.compiler import Compiler
    from match_recovery.measurement import Measurement
    from match_recovery.pe_coff import Coff
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    donor = ROOT / "src/faction.cpp"  # An existing configured product TU recipe.
    probe = real_header_probe()
    header = (ROOT / "src/faction.h").read_bytes()
    decl = declaration("src/faction.h", "has_agenda").encode()
    assert header.count(decl) == 1
    old = b"uint32_t __cdecl has_agenda(uint32_t faction_id, uint32_t faction_id_with, uint32_t agenda);"
    for flags in settings.flag_sets:
        obj = compiler.overlay(donor, {"src/faction.cpp": probe}, (), flags + settings.layout_flags)
        assert Coff(obj).symbols  # Private COFF only: never linked or published.
        with pytest.raises(ValueError) as error:
            compiler.overlay(donor, {"src/faction.cpp": probe,
                "src/faction.h": header.replace(decl, old, 1)}, (), flags + settings.layout_flags)
        diagnostic = str(error.value)
        compact = re.sub(r"\s+", "", diagnostic)
        assert ("C2440" in diagnostic and
                "unsignedint(__cdecl*)(unsignedint,unsignedint,unsignedint)" in compact and
                "int(__cdecl*)(int,int,int)" in compact), diagnostic
    assert (ROOT / "src/faction.h").read_bytes() == header
