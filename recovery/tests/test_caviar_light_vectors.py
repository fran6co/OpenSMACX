"""Actual Caviar declarations and vector consumers with explicit allocation doubles.

Host pointer widths are not Win32 layout evidence. Native member requirements
and the coordinated CodeView check cover that separately. These controls do not
claim full renderer lifetime behavior or x87 rounding equivalence.
"""

import os
from pathlib import Path
import shutil
import subprocess

import pytest

from test_caviar_matrix_operations import braced

ROOT=Path(__file__).resolve().parents[2]
PIN="01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def parts():
    source=(ROOT/"src/caviar.cpp").read_text()
    vector=(ROOT/"src/vector.cpp").read_text()
    return {"header":(ROOT/"src/caviar.h").read_text(),
            "init":braced(source,source.index("void Caviar::init() {")),
            "setter":braced(source,source.index("void Caviar::set_shadow_vector(")),
            "add":braced(vector,vector.index("void __cdecl vector_add(")),
            "normalise":braced(source,source.index("void __cdecl vox_normalise_vector("))}


def run_host(tmp_path,source):
    compiler=shutil.which(os.environ.get("OSMX_HOST_CXX", "g++"))
    if not compiler:pytest.skip("host g++ required")
    program=r'''
        #include <cstdint>
        #include <cstddef>
        #include <cmath>
        #include <cstdlib>
        #include <cstring>
        #include <vector>
        #define __cdecl
        #define MEASURED
        // Only the Win32-specific header assertions are disabled in this
        // pointer-sized host fixture; the product header is unchanged.
        #define static_assert(...)
        HEADER
        #undef static_assert
        std::vector<void*> allocations;
        int closes;
        void *mem_get(size_t n) {void *p=std::malloc(n);if(!p)std::abort();allocations.push_back(p);return p;}
        Caviar::Caviar() {}
        void Caviar::close() {++closes;}
        NORMALISE
        ADD_BODY
        INIT
        SETTER
        void require(bool ok) {if(!ok)std::exit(42);}
        void external_vectors(Caviar &renderer) {
            renderer.light_vector().values[0]=2;
            renderer.light_vector().values[1]=3;
            renderer.light_vector().values[2]=4;
            renderer.shadow_vector().values[0]=5;
            renderer.shadow_vector().values[1]=6;
            renderer.shadow_vector().values[2]=7;
        }
        int main() {
            static_assert(sizeof(VOX_Vect)==12,"three floats");
            // Model the original sequential stores, not a snapshot vector sum.
            // All inputs/outputs remain within one actual float array.
            for(int left=0;left<7;++left)for(int right=0;right<7;++right)for(int out=0;out<7;++out) {
                float actual[9],expected[9];
                for(int n=0;n<9;++n)actual[n]=float((n*11)%17-8);
                std::memcpy(expected,actual,sizeof(actual));
                expected[out]=expected[right]+expected[left];
                expected[out+1]=expected[left+1]+expected[right+1];
                expected[out+2]=expected[left+2]+expected[right+2];
                vector_add(actual+left,actual+right,actual+out);
                require(std::memcmp(actual,expected,sizeof(actual))==0);
            }
            struct Guard {uint32_t before;Caviar object;uint32_t after;} value;
            value.before=0x12345678;value.after=0xABCD1234;
            external_vectors(value.object);
            value.object.init();
            require(closes==1 && allocations.size()==600);
            for(int n=0;n<3;++n) {
                require(value.object.light_vector().values[n]==(n==2?-1.0f:0.0f));
                require(value.object.shadow_vector().values[n]==value.object.light_vector().values[n]);
            }
            external_vectors(value.object);
            const float angles[][2]={{0,0},{0.5f,0.7f},{-0.8f,1.2f},{1.5707963f,-0.4f}};
            for(const auto &a:angles) {
                value.object.set_shadow_vector(a[0],a[1]);
                const double expected[]={std::cos(double(a[1]))*std::sin(double(a[0])),
                    std::sin(double(a[1]))*std::sin(double(a[0])),std::cos(double(a[0]))};
                for(int n=0;n<3;++n) {
                    require(value.object.light_vector().values[n]==float(n+2));
                    require(std::fabs(value.object.shadow_vector().values[n]-expected[n])<0.000001);
                }
            }
            require(value.before==0x12345678 && value.after==0xABCD1234);
            for(void *p:allocations)std::free(p);
        }
    '''.replace("HEADER",source["header"]).replace("NORMALISE",source["normalise"]).replace("ADD_BODY",source["add"]).replace("INIT",source["init"]).replace("SETTER",source["setter"])
    cpp,exe=tmp_path/"vectors.cpp",tmp_path/"vectors"
    cpp.write_text(program)
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-ffp-contract=off","-fsanitize=undefined,bounds",
        "-fno-sanitize-recover=all",str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True,timeout=15)


def test_actual_vector_members_and_consumers(tmp_path):
    result=run_host(tmp_path,parts())
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("damage",["setter_light","setter_swap","init_copy","init_seed","add_snapshot","add_reverse","add_component"])
def test_vector_damage_controls(tmp_path,damage):
    source=parts()
    if damage=="setter_light":source["setter"]=source["setter"].replace("shadow_vector_","light_vector_")
    elif damage=="setter_swap":source["setter"]=source["setter"].replace("values[0]","values[2]")
    elif damage=="init_copy":
        old="scene_context_.shadow_vector_.values[2] = scene_context_.light_vector_.values[2]"
        assert source["init"].count(old)==1
        source["init"]=source["init"].replace(old,old.replace("values[2] = scene_context_.light_vector_.values[2]","values[2] = scene_context_.light_vector_.values[0]"),1)
    elif damage=="init_seed":
        old="scene_context_.light_vector_.values[2] = -1.0f"
        assert source["init"].count(old)==1
        source["init"]=source["init"].replace(old,old.replace("-1.0f","1.0f"),1)
    elif damage=="add_snapshot":
        source["add"]=source["add"].replace("    float rx", "    float y = left[1] + right[1], z = left[2] + right[2];\n    float rx").replace("output[1] = left[1] + right[1]","output[1] = y").replace("output[2] = left[2] + right[2]","output[2] = z")
    elif damage=="add_reverse":
        prefix,body=source["add"].split("{",1)
        source["add"]=prefix+"{output[2]=left[2]+right[2];output[1]=left[1]+right[1];output[0]=right[0]+left[0];}"
    else:source["add"]=source["add"].replace("left[2] + right[2]","left[1] + right[2]")
    result=run_host(tmp_path,source)
    assert result.returncode==42


def test_original_vector_groups_and_external_renderer():
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file():pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    image=PeImage(path.read_bytes(),PIN);decoder=Cs(CS_ARCH_X86,CS_MODE_32)
    ins={i.address:(i.mnemonic,i.op_str) for lo,hi in ((0x616DE0,0x617020),(0x6181D0,0x61824F),(0x4BF8D0,0x4BFC40))
         for i in decoder.disasm(image.read(Span(lo,hi)),lo)}
    anchors={0x616E94:("lea","ebp, [esi + 0x60]"),0x616F71:("mov","dword ptr [esi + 0x68], 0xbf800000"),
        0x616F78:("call","0x628220"),0x616F8F:("mov","dword ptr [esi + 0x6c], edx"),
        0x616F92:("mov","dword ptr [esi + 0x70], eax"),0x616FEF:("mov","dword ptr [esi + 0x74], ecx"),
        0x618230:("mov","dword ptr [esi + 0x74], edx"),0x618234:("fstp","dword ptr [esi + 0x6c]"),
        0x61823F:("fstp","dword ptr [esi + 0x70]"),
        0x4BFAFA:("mov","dword ptr [0x8cc888], ecx"),0x4BFB00:("mov","dword ptr [0x8cc88c], edx"),
        0x4BFB11:("mov","dword ptr [0x8cc890], eax"),0x4BFB3B:("mov","dword ptr [0x8cc898], ecx"),
        0x4BFB41:("mov","ecx, 0x8cc828"),0x4BFB4B:("mov","dword ptr [0x8cc894], eax"),
        0x4BFB50:("mov","dword ptr [0x8cc89c], edx")}
    for address,value in anchors.items():assert ins[address]==value
    addition=list(decoder.disasm(image.read(Span(0x628150,0x628175)),0x628150))
    assert [(i.mnemonic,i.op_str) for i in addition]==[
        ("mov","eax, dword ptr [esp + 8]"),("mov","ecx, dword ptr [esp + 4]"),
        ("mov","edx, dword ptr [esp + 0xc]"),("fld","dword ptr [eax]"),
        ("fadd","dword ptr [ecx]"),("fstp","dword ptr [edx]"),
        ("fld","dword ptr [ecx + 4]"),("fadd","dword ptr [eax + 4]"),("fstp","dword ptr [edx + 4]"),
        ("fld","dword ptr [ecx + 8]"),("fadd","dword ptr [eax + 8]"),("fstp","dword ptr [edx + 8]"),("ret","")]
