"""Actual loader body/header with explicit renderer, model-I/O and text doubles.

The host verifies orchestration, not CVR parsing, Win32 layout, or x87 rounding.
The pinned call manifest is independent of the source being exercised.
"""
import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_caviar_matrix_operations import braced

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
MANIFEST = "a9b10f9db671482287d888d97ccdf09dc2e04ffefbd58d543bd3c8f24a22e117"
OFFSETS = ("VehGunMountOffset", "VehDropOffset", "VehHoverGunOffset", "VehHoverLightsOffset")


def sources():
    game = (ROOT / "src/game.cpp").read_text()
    veh = (ROOT / "src/veh.cpp").read_text()
    return (braced(game, game.index("void __cdecl load_voxels()")),
            braced(veh, veh.index("void __cdecl vehdraw_setup()")))


def actual_globals():
    source = (ROOT / "src/caviar.cpp").read_text()
    return re.findall(r"^CaviarData (\w+)(\[\d+\])?;", source, re.M)


HARNESS = r'''
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
#include <vector>
#define __cdecl
#define MEASURED
#define static_assert(...)
HEADER
#undef static_assert
struct Fault {};
void require(bool b) {if(!b) std::exit(42);}
bool tracking=false;
int stage=0, calls=0, temp_loads=0, closes=0, item=0, normalization=0;
int mode=0, throw_at=0;
CaviarData *temporary=0;
std::map<CaviarData*,std::string> names;
std::vector<std::string> events;
CvrRecord records[4];
CvrPart parts[2];
VOX_Vect vectors[2]={{{3,-5,8}},{{11,2,-4}}};
VOX_Vect poison[2]={{{100,100,100}},{{-100,-100,-100}}};
VOX_Vect VehLightDirMap={{9,10,11}};
VOX_Vect VehGunMountOffset,VehDropOffset,VehHoverGunOffset,VehHoverLightsOffset;
VOX_Vect *outputs[]={&VehGunMountOffset,&VehDropOffset,&VehHoverGunOffset,&VehHoverLightsOffset};
unsigned char VoxelRowColorTables[24][256];
GLOBALS
Caviar g_VEHDRAW_CAVIAR;
Caviar::Caviar() {}
void Caviar::close() {}
void trigger(int point) {if(throw_at==point) throw Fault();}
void Caviar::init() {
    require(this==&g_VEHDRAW_CAVIAR && stage==0); stage=1; trigger(1);
}
void __cdecl vehdraw_compute_facet_normals() {
    require(stage==1); stage=2; trigger(2);
}
SETUP
int CaviarData::init(char *filename) {
    if(!tracking) std::abort();
    require(stage==2);
    ++calls;
    trigger(100+calls);
    auto found=names.find(this);
    const bool scratch=found==names.end();
    events.push_back((scratch?std::string("scratch"):found->second)+"|"+filename);
    if(scratch) {
        require(!temporary || temporary==this); temporary=this;
        require(temp_loads<4);
        // The first two loads were explicitly closed; the third remains
        // owned when init replaces it with the fourth model.
        require(closes==(temp_loads==0?0:temp_loads==1?1:2));
        require(record_==(temp_loads==3 && mode!=1?&records[2]:nullptr));
        record_=mode==1?nullptr:&records[temp_loads];
        ++temp_loads;
    }
    return 99; // A reported load failure must not short-circuit orchestration.
}
void CaviarData::close() {
    if(!tracking) return;
    require(names.find(this)==names.end());
    ++closes; record_=nullptr;
}
void __cdecl leaf_00628180_redirect(const float *a,const float *b,float *out) {
    require(temp_loads>0 && temp_loads<=4);
    require(a==vectors[1].values && b==vectors[0].values);
    require(out==outputs[temp_loads-1]->values);
    out[0]=a[0]-b[0];out[1]=a[1]-b[1];out[2]=a[2]-b[2];
}
int text_open(const char *file,const char *section) {
    require(calls==133 && closes==2 && temp_loads==4);
    require(!std::strcmp(file,"system") && !std::strcmp(section,"VEH_LIGHT_DIR_MAP"));
    trigger(3); return mode==3?1:0;
}
char *text_get() {require(item==0);item=1;trigger(4);return nullptr;}
int text_item_number() {
    require(item>=1 && item<=3);trigger(4+item);
    if(item>=2) require(VehLightDirMap.values[0]==16777216.0f);
    if(item>=3) require(VehLightDirMap.values[1]==-3.0f);
    const int input[]={16777217,-3,0};return input[item++-1];
}
void __cdecl vox_normalise_vector(const float *in,float *out) {
    require(item==4 && in==VehLightDirMap.values && out==in);
    require(in[0]==16777216.0f && in[1]==-3 && in[2]==0);
    ++normalization;trigger(8);out[0]=7;out[1]=8;out[2]=9;
}
LOADER
int main(int argc,char **argv) {
    require(argc==3); mode=std::atoi(argv[1]);throw_at=std::atoi(argv[2]);
    REGISTRY
    for(int n=0;n<4;++n) {
        records[n].step_count_=mode==2?3:2;
        records[n].part_count_=0; // Not the branch used by the original.
        records[n].parts_=mode==2?nullptr:parts;
        for(int k=0;k<3;++k) outputs[n]->values[k]=float(20+n*3+k);
    }
    parts[0].vectors_=vectors;parts[1].vectors_=poison;
    std::memset(VoxelRowColorTables,0xAD,sizeof(VoxelRowColorTables));
    tracking=true; bool threw=false;
    try {load_voxels();} catch(const Fault&) {threw=true;}
    require(threw==(throw_at!=0));
    // All exits unwind the actual header destructor, even before the first
    // temporary-model init. Only the first two derivations close explicitly.
    const int explicit_closes=temp_loads<2?temp_loads:2;
    require(closes==explicit_closes+1);
    if(threw) {
        for(auto &row:VoxelRowColorTables)for(unsigned char v:row)require(v==0xAD);
    } else {
        require(calls==133 && temp_loads==4 && closes==3);
        const float difference[]={8,7,-12};
        for(int n=0;n<4;++n)for(int k=0;k<3;++k) {
            float expected=mode==1?0:mode==2?float(20+n*3+k):difference[k];
            require(outputs[n]->values[k]==expected);
            if(mode==1) {uint32_t bits;std::memcpy(&bits,&outputs[n]->values[k],4);require(bits==0);}
        }
        require(normalization==(mode==3?0:1));
        if(mode==3)require(item==0 && VehLightDirMap.values[0]==9 && VehLightDirMap.values[1]==10 && VehLightDirMap.values[2]==11);
        for(int r=0;r<24;++r)for(int c=0;c<256;++c)require(VoxelRowColorTables[r][c]==r);
        for(const auto &event:events)std::puts(event.c_str());
    }
    tracking=false;
}
'''


def compile_host(tmp_path, loader=None):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    body, setup = sources()
    header = (ROOT / "src/caviar.h").read_text()
    header = re.sub(r"^#.*$", "", header, flags=re.M)
    declarations, registry = [], []
    for name, array in actual_globals():
        declarations.append("CaviarData " + name + array + ";")
        if array:
            for n in range(int(array[1:-1])):
                key = f"{name}[{n}]"
                registry.append(f'names[&{key}]="{key}";')
        else:
            registry.append(f'names[&{name}]="{name}";')
    source = HARNESS
    for token, value in (("HEADER", header), ("GLOBALS", "\n".join(declarations)),
                         ("REGISTRY", "\n".join(registry)), ("SETUP", setup),
                         ("LOADER", loader if loader is not None else body)):
        source = source.replace(token, value)
    cpp = tmp_path / "loader.cpp"
    cpp.write_text(source)
    binary = tmp_path / "loader"
    result = subprocess.run([compiler, "-std=c++17", "-Wno-write-strings", "-O1",
                             "-fsanitize=undefined", "-fno-sanitize-recover=all",
                             str(cpp), "-o", str(binary)], capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return binary


def run(binary, mode=0, failure=0):
    return subprocess.run([str(binary), str(mode), str(failure)], capture_output=True, text=True)


def test_actual_loader_modes_and_unwind(tmp_path):
    binary = compile_host(tmp_path)
    for mode in range(4):
        result = run(binary, mode)
        assert result.returncode == 0, result.stderr
        assert len(result.stdout.splitlines()) == 133
        assert hashlib.sha256(result.stdout.encode()).hexdigest() == MANIFEST
    # Model callbacks, both renderer stages, configuration and normalization.
    for failure in (1, 2, 101, 182, 183, 186, 203, 204, 233, 3, 4, 5, 6, 7, 8):
        result = run(binary, failure=failure)
        assert result.returncode == 0, (failure, result.stderr)


@pytest.mark.parametrize("damage", ["reverse", "part_count", "clear_stale", "close_third",
                                    "double_close", "skip_load", "ramp", "unsigned_light",
                                    "deferred_light", "return_early"])
def test_damage_controls(tmp_path, damage):
    body, _ = sources()
    changes = {
        "reverse": ("vectors[1].values, vectors[0].values", "vectors[0].values, vectors[1].values"),
        "part_count": ("record->step_count_ == 2", "record->part_count_ == 2"),
        "clear_stale": ("if (!record)", "if (!record || record->step_count_ != 2)"),
        "close_third": ('scratch.init("AVHT-LightsReposition.cvr");', 'scratch.close(); scratch.init("AVHT-LightsReposition.cvr");'),
        "double_close": ("    if (text_open", "    scratch.close();\n    if (text_open"),
        "skip_load": ('g_VW_CAVIARDATA[14].init("VW12.CVR");', ''),
        "ramp": ("(unsigned char)row", "(unsigned char)(row * 8)"),
        "unsigned_light": ("(float)text_item_number()", "(float)(unsigned)text_item_number()"),
        "deferred_light": ("VehLightDirMap.values[0] = (float)text_item_number();", "float saved = (float)text_item_number();"),
        "return_early": ('g_SSF_CAVIARDATA.init("ssf.cvr");', 'if (g_SSF_CAVIARDATA.init("ssf.cvr")) return;'),
    }
    old, new = changes[damage]
    assert old in body
    result = run(compile_host(tmp_path, body.replace(old, new)), mode=2 if damage == "clear_stale" else 0)
    assert result.returncode != 0


def test_pinned_manifest_and_arithmetic():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    image = PeImage(path.read_bytes(), PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    def instructions(lo, hi):
        return list(decoder.disasm(image.read(Span(lo, hi)), lo))
    code = instructions(0x451030, 0x451A09)
    assert code[-1].address + code[-1].size == 0x451A09
    addresses = {0x78E968: "g_VBP_CAVIARDATA"}
    for name, count, start in re.findall(r"CaviarData (\w+)(?:\[(\d+)\])?;  // (0x[0-9A-Fa-f]+)", (ROOT / "src/caviar.cpp").read_text()):
        for n in range(int(count or 1)):
            addresses[int(start, 16) + n * 12] = name + (f"[{n}]" if count else "")
    manifest = []
    for n, ins in enumerate(code):
        if (ins.mnemonic, ins.op_str) == ("call", "0x616bd0"):
            push, receiver = code[n-2:n]
            assert push.mnemonic == "push"
            address = int(push.op_str, 16)
            filename = image.read(Span(address, address + 64)).split(b"\0")[0].decode()
            name = addresses[int(receiver.op_str.split(", ")[1], 16)] if receiver.mnemonic == "mov" else "scratch"
            manifest.append(name + "|" + filename)
    assert len(manifest) == 133
    assert hashlib.sha256(("\n".join(manifest) + "\n").encode()).hexdigest() == MANIFEST
    pairs = {i.address: (i.mnemonic, i.op_str) for i in code}
    assert pairs[0x4519BA] == ("mov", "eax, 0x2aaaaaab")
    assert pairs[0x4519C1] == ("imul", "edi")
    assert pairs[0x4519C3] == ("sar", "edx, 2")
    assert pairs[0x4519DF] == ("add", "edi, 0x18")
    palette = {i.address: (i.mnemonic, i.op_str) for i in instructions(0x4242E9, 0x4242FD)}
    assert palette[0x4242E9] == ("mov", "eax, 0x2aaaaaab")
    assert palette[0x4242EE] == ("imul", "ecx")
    assert palette[0x4242F0] == ("sar", "edx, 2")
    assert palette[0x4242FA] == ("shr", "ecx, 0x1f")
    # Signed high product, arithmetic shift, sign correction; independently
    # exercise negative values rather than repeating a candidate /24 formula.
    def original_divide(value):
        high = (value * 0x2AAAAAAB) >> 32
        quotient = high >> 2
        return quotient + ((quotient & 0xffffffff) >> 31)
    assert [original_divide(24*n) for n in range(24)] == list(range(24))
    for value, expected in [(-25, -1), (-24, -1), (-23, 0), (-1, 0), (23, 0), (24, 1), (25, 1)]:
        assert original_divide(value) == expected
    assert [(i.mnemonic, i.op_str) for i in instructions(0x4C1D10, 0x4C1D15)] == [("jmp", "0x4c1b10")]
    assert [(i.mnemonic, i.op_str) for i in instructions(0x655030, 0x655038)] == [("lea", "ecx, [ebp - 0x1c]"), ("jmp", "0x456100")]
    assert [i.address for i in code if (i.mnemonic, i.op_str) == ("call", "0x616c60")] == [0x4515A1, 0x451618, 0x4519F4]
