"""Actual model/renderer lifetimes with explicit allocation, IO and release doubles.

The host uses its own pointer widths; it does not prove Win32 layout or the
linked CRT startup table. Native member checks and original anchors are separate.
CVR tree release is a dependency double here (its real body has loader tests).
"""

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest

from test_caviar_matrix_operations import braced

ROOT=Path(__file__).resolve().parents[2]
PIN="01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def actual_source():
    text=(ROOT/"src/caviar.cpp").read_text()
    model=(ROOT/"src/caviar_data.cpp").read_text()
    header=(ROOT/"src/caviar.h").read_text()
    signatures={"model_ctor":"CaviarData::CaviarData()",
        "model_close":"void CaviarData::close()",
        "model_init":"int CaviarData::init(char *name)","renderer_ctor":"Caviar::Caviar()",
        "renderer_close":"void Caviar::close()",
        "renderer_init":"void Caviar::init()", "instance_ctor":"VoxInstance::VoxInstance()",
        "normalise":"void __cdecl vox_normalise_vector("}
    sources={"model_ctor":model,"model_close":model,"model_init":model}
    return {"header":header,
            "model_dtor":braced(header,header.index("~CaviarData()")),
            "renderer_dtor":braced(header,header.index("~Caviar()")),
            **{name:braced(sources.get(name,text),re.search("^"+re.escape(signature),sources.get(name,text),re.M).start()) for name,signature in signatures.items()}}


def run_host(tmp_path,parts):
    compiler=shutil.which(os.environ.get("OSMX_HOST_CXX", "g++"))
    if not compiler:pytest.skip("host g++ required")
    header=parts["header"]
    for key,name in (("model_dtor","CaviarData"),("renderer_dtor","Caviar")):
        original=braced(header,header.index("~"+name+"()"))
        header=header.replace(original,parts[key])
    program=r'''
        #include <cstdint>
        #include <cstddef>
        #include <cstdlib>
        #include <cstring>
        #include <cmath>
        #include <vector>
        #include <type_traits>
        #define __cdecl
        #define MEASURED
        struct CvrRecord;
        void caviar_free_record(CvrRecord *);
        int _close(int);
        // Access only changes test visibility. Product declarations and bodies
        // stay intact; Win32 sizeof assertions are checked natively, not here.
        #define private public
        #define static_assert(...)
        HEADER
        #undef static_assert
        #undef private
        void require(bool ok){if(!ok)std::exit(42);}
        std::vector<void*> allocated,released;
        std::vector<CvrRecord*> models,released_models;
        std::vector<int> closed_fds;
        int allocation_count,fail_at,throw_at,open_result=17,opens,loads;
        CvrRecord *next_record;
        void *mem_get(size_t size) {
            ++allocation_count;
            if(allocation_count==throw_at)throw 7;
            if(allocation_count==fail_at)return nullptr;
            require(size==1 || size==12 || size==36);
            void *p=std::malloc(size);require(p!=nullptr);allocated.push_back(p);return p;
        }
        void release_instance(void *p) {
            require(released.size()<allocated.size() && allocated[released.size()]==p);
            released.push_back(p);std::free(p);
        }
        void caviar_free_record(CvrRecord *p) {
            bool known=false;for(auto model:models)known|=p==model;
            require(known);released_models.push_back(p);
        }
        int _open(char *name,int flags) {require(name && flags==0x8000);++opens;return open_result;}
        int _close(int fd) {closed_fds.push_back(fd);return 0;}
        CvrRecord *vox_load_record(int fd) {require(fd==open_result);++loads;return next_record;}
        #define free release_instance
        BODIES
        #undef free
        void reset_allocations() {
            require(allocated==released);allocated.clear();released.clear();allocation_count=0;fail_at=throw_at=-1;
        }
        void require_empty(Caviar &renderer) {
            for(int n=0;n<200;++n) {
                require(!renderer.slots_[n].instance_colour_ && !renderer.slots_[n].instance_vector_ && !renderer.slots_[n].instance_matrix_);
            }
        }
        int main() {
            static_assert(std::is_trivially_destructible<VoxInstance>::value,"no per-instance model destructor");
            static_assert(!std::is_trivially_destructible<CaviarData>::value,"loaded models own records");
            static_assert(std::is_same<decltype(CaviarData().record()),const CvrRecord*>::value,"read-only model accessor");
            static_assert(std::is_same<decltype(CaviarData().fileDescriptor_),int>::value,"integer CRT descriptor");
            VoxInstance blank_instance;
            require(!blank_instance.instance_colour_ && !blank_instance.instance_vector_ && !blank_instance.instance_matrix_);
            CvrRecord first={},second={},third={};models={&first,&second,&third};
            char name[]="model.cvr";
            {
                CaviarData model;require(model.record()==nullptr && model.fileDescriptor_==0 && model.field_0_==0);
                model.field_0_=0xA1B2C3D4;next_record=&first;
                require(model.init(name)==0 && model.record()==&first && closed_fds==std::vector<int>{17});
                next_record=&second;require(model.init(name)==0 && released_models==std::vector<CvrRecord*>{&first});
                model.close();require(model.record()==nullptr && model.field_0_==0xA1B2C3D4);
                model.close();require(released_models.size()==2);
                model.fileDescriptor_=-1; // close owns only the record, not the fd.
            }
            require(released_models==std::vector<CvrRecord*>({&first,&second}) && closed_fds.size()==2);
            released_models.clear();
            try {CaviarData scratch;next_record=&first;require(scratch.init(name)==0);throw 3;}catch(int n){require(n==3);}
            require(released_models==std::vector<CvrRecord*>{&first});
            released_models.clear();
            {
                CaviarData array[3];
                for(int i=0;i<3;++i){next_record=models[i];require(array[i].init(name)==0);}
            }
            require(released_models==std::vector<CvrRecord*>({&third,&second,&first}));
            size_t model_releases=released_models.size();
            {
                CaviarData failure;
                int old_opens=opens,old_loads=loads;require(failure.init(nullptr)==0x10 && opens==old_opens && loads==old_loads);
                open_result=-1;require(failure.init(name)==6 && failure.fileDescriptor_==-1 && !failure.record());
                size_t close_count=closed_fds.size();open_result=23;next_record=nullptr;
                require(failure.init(name)==0x17 && failure.fileDescriptor_==23 && closed_fds.size()==close_count);
                // Original failures leave the descriptor untouched; do not invent
                // new destructor descriptor cleanup as part of this recovery.
            }
            require(released_models.size()==model_releases);
            reset_allocations();
            {
                Caviar renderer;require_empty(renderer);
                require(renderer.field_108_==0x461C4000 && renderer.field_13CC_==0);
                renderer.init();require(allocated.size()==600 && released.empty());
                require(renderer.scene_context_.state_.instances_==renderer.slots_);
            }
            require(allocated==released && released_models.size()==model_releases);
            reset_allocations();
            {
                Caviar renderer;renderer.init();renderer.close();require_empty(renderer);
                require(allocated==released);renderer.close();require(allocated==released);
                require(renderer.field_4_==0 && renderer.scene_scale_==2.0f/3.0f);
            }
            // All partial-allocation return paths and representative C++ unwind
            // positions must free instance allocations, never treat them as CVRs.
            for(int failure=1;failure<=600;++failure) {
                reset_allocations();fail_at=failure;
                {Caviar renderer;renderer.init();require_empty(renderer);}
                require(allocated==released && released.size()==size_t(failure-1));
            }
            for(int failure:{1,2,3,300,600}) {
                reset_allocations();throw_at=failure;
                try {Caviar renderer;renderer.init();require(false);}catch(int n){require(n==7);}
                require(allocated==released && released.size()==size_t(failure-1));
            }
            require(released_models.size()==model_releases);
        }
    '''.replace("HEADER",header).replace("BODIES","\n".join(value for name,value in parts.items() if name not in ("header","model_dtor","renderer_dtor")))
    cpp,exe=tmp_path/"lifetime.cpp",tmp_path/"lifetime"
    cpp.write_text(program)
    compiled=subprocess.run([compiler,"-std=c++11","-O1","-fsanitize=undefined,bounds","-fno-sanitize-recover=all",
        str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=30)
    assert compiled.returncode==0,compiled.stderr
    return subprocess.run([str(exe)],capture_output=True,text=True,timeout=15)


def test_actual_model_and_instance_lifetimes(tmp_path):
    result=run_host(tmp_path,actual_source());assert result.returncode==0,result.stderr


@pytest.mark.parametrize("damage",["no_model_dtor","no_renderer_dtor","short_array","wrong_release","model_clear","model_fd_close","wrong_owner","instance_vector_not_zero"])
def test_lifetime_damage_controls(tmp_path,damage):
    parts=actual_source()
    if damage=="no_model_dtor":parts["model_dtor"]=parts["model_dtor"].replace("close();","")
    elif damage=="no_renderer_dtor":parts["renderer_dtor"]=parts["renderer_dtor"].replace("close();","")
    elif damage=="short_array":parts["renderer_close"]=parts["renderer_close"].replace("index < 200","index < 199")
    elif damage=="wrong_release":parts["renderer_close"]=parts["renderer_close"].replace("free(slots_[index].instance_colour_)","free(slots_[index].instance_vector_)")
    elif damage=="model_clear":parts["model_close"]=parts["model_close"].replace("record_ = nullptr;","record_ = nullptr; field_0_ = 0;")
    elif damage=="model_fd_close":parts["model_dtor"]=parts["model_dtor"].replace("close();","close(); _close(fileDescriptor_);")
    elif damage=="instance_vector_not_zero":parts["instance_ctor"]=parts["instance_ctor"].replace("instance_vector_(0)","instance_vector_(reinterpret_cast<VOX_Vect *>(1))")
    else:parts["renderer_dtor"]=parts["renderer_dtor"].replace("close();","caviar_free_record(reinterpret_cast<CvrRecord*>(slots_[0].instance_matrix_)); close();")
    result=run_host(tmp_path,parts);assert result.returncode==42


def test_original_distinct_destruction_paths():
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file():pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    image=PeImage(path.read_bytes(),PIN);decoder=Cs(CS_ARCH_X86,CS_MODE_32)
    def body(lo,hi):return list(decoder.disasm(image.read(Span(lo,hi)),lo))
    def pairs(lo,hi):return [(i.mnemonic,i.op_str) for i in body(lo,hi)]
    assert pairs(0x456100,0x456105)==[("jmp","0x616c60")]
    assert pairs(0x4210F0,0x4210F5)==[("jmp","0x617020")]
    assert pairs(0x655030,0x655038)==[("lea","ecx, [ebp - 0x1c]"),("jmp","0x456100")]
    assert pairs(0x44D590,0x44D5A8)==[("push","0x456100"),("push","0x616bc0"),("push","2"),
        ("push","0xc"),("push","0x779288"),("call","0x6457c2")]
    assert pairs(0x44D5C0,0x44D5D4)==[("push","0x456100"),("push","2"),("push","0xc"),
        ("push","0x779288"),("call","0x6456e4"),("ret","")]
    assert pairs(0x4BF700,0x4BF70A)==[("mov","ecx, 0x8cc828"),("jmp","0x617020")]
    close=body(0x617020,0x61707F)
    assert [i.op_str for i in close if i.mnemonic=="call"]==["0x644ef2"]*3
    assert close[-1].mnemonic=="ret" and close[-1].address==0x61707E
    ctor=body(0x616DA0,0x616DD6);ins={i.address:(i.mnemonic,i.op_str) for i in ctor}
    assert not any(i.mnemonic=="call" for i in ctor)
    assert ins[0x616DA3]==("mov","esi, 0xc8") and ins[0x616DAA]==("lea","edx, [eax + 0x750]")
    assert ins[0x616DB8]==("add","edx, 0xc")
