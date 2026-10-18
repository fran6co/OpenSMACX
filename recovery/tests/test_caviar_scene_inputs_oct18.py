"""Whole actual scene bodies and source constants; bounded host/image controls.

Null setup executes in an isolated Linux child with core dumps disabled and
no sanitizer rewriting: SIGSEGV plus staged-scratch evidence distinguishes
the original unconditional call from a compiling guarded-skip mutation.
This is not a portable C++ or original Windows exception guarantee.
"""
from pathlib import Path
import os
import re
import resource
import signal
import shutil
import struct
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from test_caviar_scene_product_source import extract_definition

ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"

HARNESS = r'''
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <signal.h>
#include <unistd.h>
#define __cdecl
struct VOX_Vect { float values[3]; };
struct VOX_Matrix { float values[3][3]; };
struct CvrShape { float scale_; };
struct CvrStep { CvrShape *shape_; };
struct VoxInstance { uint8_t *instance_colour_; VOX_Vect *instance_vector_; VOX_Matrix *instance_matrix_; };
struct CaviarRenderClipWindowCandidate { int left_,top_,right_,bottom_,width_,height_; };
struct VoxRenderClipBounds { int left_,top_,right_,bottom_,width_,height_; };
struct Setup { void *colour_buffer_,*depth_buffer_,*shadow_buffer_; int colour_row_step_,depth_row_step_,shadow_row_step_; };
struct VoxSceneState { int instance_count_; VoxInstance *instances_; CvrStep *instance_steps_; };
struct CaviarSceneContext { VoxSceneState state_; VOX_Vect light_vector_; uint8_t shadow_enabled_; };
struct Camera { VOX_Matrix rotation; };
struct Bounds { int values_[6]; };
struct VoxCameraBlock { Camera camera_; Bounds bounds_; };
struct VoxRenderRecord {
    typedef void (*SetupCallback)();
    typedef void (*ObjectStartCallback)(VoxRenderRecord*,long,VOX_Vect*,VOX_Matrix*,int32_t*,VOX_Vect*);
    typedef void (*PostPartCallback)(VoxRenderRecord*,int32_t);
    Setup *setup_block_; void *colour_table_; uint16_t **ramp_a_ptr_; const int32_t *ramp_b_ptr_;
    uint8_t setup_size_code_; SetupCallback setup_callback_; ObjectStartCallback object_start_callback_; PostPartCallback post_part_callback_;
    VoxRenderClipBounds primary_clip_,secondary_clip_;
};
struct Scratch { float projected_scale_; VOX_Matrix view_scene_matrix_,shadow_matrix_;
    VOX_Vect camera_offset_,shadow_centre_; float shadow_radius_x_,shadow_radius_y_;
    CaviarRenderClipWindowCandidate part_clip_,shadow_clip_; };
static Scratch CaviarSceneProjectionScratch;
struct Window { struct Views { struct Tail { uint32_t vox_scene_extent_; } tail_; } current_views_; };
static Window CaviarKernelProjectionWindow;
static Setup vox_setup_copy_;
static int vox_scene_busy=0;
static void *vox_active_colour_table_;
static uint16_t **vox_active_ramp_a_;
static const int32_t *vox_active_ramp_b_;
static uint8_t vox_active_size_code_;
static bool behind=false;
static int main_calls,shadow_calls,start_calls,post_calls,clip_calls,last_visible,last_shadow_visibility,setup_calls,stage_calls;
static int setup_mutation=-1,post_mutation=-1;
static CaviarSceneContext *active_context;
static VoxRenderRecord *active_record;
static void require(bool,const char*);
static VOX_Matrix &caviar_scene_shadow_matrix_view(Scratch &s) { return s.shadow_matrix_; }
static void identity(VOX_Matrix &m) { std::memset(&m,0,sizeof(m));for(int i=0;i<3;++i)m.values[i][i]=1; }
static void vox_matrix_vector_mul(const float m[3][3],const float *v,float *out) {
    float result[3];for(int i=0;i<3;++i)result[i]=m[i][0]*v[0]+m[i][1]*v[1]+m[i][2]*v[2];std::memcpy(out,result,sizeof(result));
}
static void vector_add(const float *a,const float *b,float *out) { float r[3];for(int i=0;i<3;++i)r[i]=a[i]+b[i];std::memcpy(out,r,sizeof(r)); }
static void vox_matrix_matrix_mul(const float a[3][3],const float b[3][3],float out[3][3]) {
    float r[3][3];for(int i=0;i<3;++i)for(int j=0;j<3;++j)r[i][j]=a[i][0]*b[0][j]+a[i][1]*b[1][j]+a[i][2]*b[2][j];std::memcpy(out,r,sizeof(r));
}
static const CvrShape *vox_walk_scaled_chain(const CvrShape *s,float) {return s;}
static bool vox_part_behind(const VOX_Vect *,float,const VoxCameraBlock *) {return behind;}
static void vox_stage_scene(VoxRenderRecord *,CaviarSceneContext *,VoxCameraBlock *,int,float *a,float *b) { ++stage_calls;if(a!=b)std::exit(80); }
static void caviar_clip_shape(VoxRenderRecord *,const CvrShape *,const VOX_Vect *,const VOX_Matrix *,const CaviarRenderClipWindowCandidate *,int *out) {
    ++clip_calls;out[0]=1;out[1]=2;out[2]=3;out[3]=4;out[4]=3;out[5]=3;
}
static void vox_draw_part(VoxRenderRecord *,const CvrShape *,const VOX_Vect *,const VOX_Matrix *m,const VOX_Matrix *,const VOX_Vect *,float,int visible) {
    ++main_calls;last_visible=visible;if(m->values[2][2]!=1)std::exit(81);
}
static void vox_draw_shadow(VoxRenderRecord *,const CvrShape *,const VOX_Vect *,const VOX_Matrix *m,int visible) {
    ++shadow_calls;last_shadow_visibility=visible;if(m->values[2][2]!=1)std::exit(82);
}
static bool scratch_staged() {
    Setup&s=*active_record->setup_block_;
    return vox_scene_busy==1 && stage_calls==1 &&
        vox_active_colour_table_==active_record->colour_table_ &&
        vox_active_ramp_a_==active_record->ramp_a_ptr_ && vox_active_ramp_b_==active_record->ramp_b_ptr_ &&
        vox_active_size_code_==active_record->setup_size_code_ &&
        vox_setup_copy_.colour_buffer_==s.colour_buffer_ && vox_setup_copy_.depth_buffer_==s.depth_buffer_ &&
        vox_setup_copy_.shadow_buffer_==s.shadow_buffer_ && vox_setup_copy_.colour_row_step_==s.colour_row_step_ &&
        vox_setup_copy_.depth_row_step_==s.depth_row_step_ && vox_setup_copy_.shadow_row_step_==s.shadow_row_step_ &&
        CaviarKernelProjectionWindow.current_views_.tail_.vox_scene_extent_==
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(s.colour_buffer_))-
            static_cast<uint32_t>(reinterpret_cast<uintptr_t>(s.depth_buffer_));
}
static void setup_callback() {
    ++setup_calls;require(scratch_staged(),"setup-after-complete-scratch-staging");
    require(main_calls==0&&start_calls==0&&post_calls==0,"setup-before-instance-callbacks");
    if(setup_mutation>=0)active_context->shadow_enabled_=setup_mutation;
}
static void start_callback(VoxRenderRecord *,long,VOX_Vect *,VOX_Matrix *,int32_t *,VOX_Vect *) {++start_calls;}
static void post_callback(VoxRenderRecord *,int32_t) {++post_calls;if(post_mutation>=0)active_context->shadow_enabled_=post_mutation;}
/* CONSTANTS */
/* LOAD_BODY */
/* DRAW_BODY */
static void require(bool ok,const char *name) {if(!ok){std::fprintf(stderr,"witness: %s\n",name);std::exit(1);}}
struct Fixture {
    Setup setup;VoxRenderRecord record;CaviarSceneContext context;VoxCameraBlock camera;
    VoxInstance instance;CvrStep step;CvrShape shape;uint8_t colour;VOX_Vect vector;VOX_Matrix matrix;
    uint16_t depth,*depth_rows[1];int32_t relative_rows[1];
    Fixture() {
        std::memset(this,0,sizeof(*this));std::memset(&CaviarSceneProjectionScratch,0,sizeof(CaviarSceneProjectionScratch));
        identity(matrix);identity(camera.camera_.rotation);identity(CaviarSceneProjectionScratch.view_scene_matrix_);identity(CaviarSceneProjectionScratch.shadow_matrix_);
        CaviarSceneProjectionScratch.projected_scale_=0.25f;CaviarSceneProjectionScratch.shadow_radius_x_=CaviarSceneProjectionScratch.shadow_radius_y_=1;
        camera.bounds_.values_[4]=camera.bounds_.values_[5]=100;
        shape.scale_=1;step.shape_=&shape;instance.instance_colour_=&colour;instance.instance_vector_=&vector;instance.instance_matrix_=&matrix;
        context.state_.instance_count_=1;context.state_.instances_=&instance;context.state_.instance_steps_=&step;context.shadow_enabled_=1;
        setup.colour_buffer_=&colour;setup.depth_buffer_=&depth;setup.shadow_buffer_=&colour;
        setup.colour_row_step_=11;setup.depth_row_step_=12;setup.shadow_row_step_=13;
        depth_rows[0]=&depth;record.colour_table_=&colour;record.ramp_a_ptr_=depth_rows;record.ramp_b_ptr_=relative_rows;record.setup_size_code_=0x93;
        record.setup_block_=&setup;record.setup_callback_=setup_callback;record.object_start_callback_=start_callback;record.post_part_callback_=post_callback;
        vox_active_colour_table_=0;vox_active_ramp_a_=0;vox_active_ramp_b_=0;vox_active_size_code_=0;
        std::memset(&vox_setup_copy_,0,sizeof(vox_setup_copy_));
        active_context=&context;active_record=&record;main_calls=shadow_calls=start_calls=post_calls=clip_calls=setup_calls=stage_calls=0;last_visible=last_shadow_visibility=-1;setup_mutation=post_mutation=-1;behind=false;
    }
    void draw() {vox_draw_scene(&record,&context,&camera,0);require(vox_scene_busy==0,"busy-cleared");}
};
static void null_setup_fault(int) {
    // Only async-signal-safe output/termination operations in this child.
    if(!scratch_staged() || active_context->state_.instance_count_!=0 || setup_calls!=0)_exit(91);
    static const char witness[]="witness: null-setup-after-staging\n";
    write(2,witness,sizeof(witness)-1);
    signal(SIGSEGV,SIG_DFL);raise(SIGSEGV);
}
int main() {
    if(std::getenv("CAVIAR_NULL_SETUP")) {
        Fixture f;f.context.state_.instance_count_=0;f.record.setup_callback_=0;
        signal(SIGSEGV,null_setup_fault);f.draw();
        std::puts("null-setup-returned");return 0;
    }
    {Fixture f;f.context.state_.instance_count_=0;f.draw();require(setup_calls==1&&main_calls==0&&shadow_calls==0,"empty-scene-still-calls-setup-once");}
    {Fixture f;f.draw();require(main_calls==1 && shadow_calls==1 && start_calls==1 && post_calls==1,"normal-complete-chain");require(last_visible==1,"visibility-below-cut");require(last_shadow_visibility==1,"shadow-visibility-below-cut");}
    {Fixture f;CaviarSceneProjectionScratch.projected_scale_=0.5f;f.draw();require(last_visible==1,"visibility-equal-cut");require(last_shadow_visibility==1,"shadow-visibility-equal-cut");}
    {Fixture f;CaviarSceneProjectionScratch.projected_scale_=0.75f;f.draw();require(last_visible==0,"visibility-above-cut");require(last_shadow_visibility==0,"shadow-visibility-above-cut");}
    {Fixture f;behind=true;f.draw();require(main_calls==0 && start_calls==0 && post_calls==0,"behind-skips-main");require(shadow_calls==1,"behind-keeps-shadow");require(last_shadow_visibility==1,"behind-shadow-visibility-below-cut");require(f.record.secondary_clip_.width_==3,"load-behind-keeps-shadow");}
    {Fixture f;behind=true;CaviarSceneProjectionScratch.projected_scale_=0.75f;f.draw();require(main_calls==0 && start_calls==0 && post_calls==0,"behind-skips-hidden-main");require(shadow_calls==1,"behind-keeps-hidden-shadow");require(last_shadow_visibility==0,"behind-shadow-visibility-above-cut");}
    {Fixture f;post_mutation=0;f.draw();require(shadow_calls==0,"post-callback-disables-shadow");}
    {Fixture f;f.context.shadow_enabled_=0;post_mutation=1;CaviarSceneProjectionScratch.projected_scale_=0.75f;f.draw();require(shadow_calls==1,"post-callback-enables-shadow");require(last_shadow_visibility==0,"post-callback-keeps-shadow-visibility");}
    {Fixture f;setup_mutation=0;f.draw();require(shadow_calls==0,"setup-disables-shadow");}
    {Fixture f;f.context.shadow_enabled_=0;setup_mutation=1;f.draw();require(shadow_calls==1,"setup-enables-shadow");}
    {Fixture f;f.colour=0x80;f.draw();require(main_calls==0 && shadow_calls==0 && clip_calls==0,"hidden-skips-both");}
    {Fixture f;f.setup.shadow_buffer_=0;f.draw();require(main_calls==1 && shadow_calls==0,"null-shadow-buffer");}
    const float coordinates[]={-128,-129,228,229};
    for(int axis=0;axis<2;++axis)for(int i=0;i<4;++i) {
        Fixture f;f.vector.values[axis]=coordinates[i];f.draw();
        bool inside=(i==0 || i==2);
        require(shadow_calls==(inside?1:0),"radius-inclusive-shadow-boundary");
        require(f.record.secondary_clip_.width_==(inside?3:0),"load-radius-inclusive-boundary");
    }
    // Keep the original unordered-X acceptance and lower-Y rejection.
    // Shadow centre alone carries NaN; identity arithmetic on vectors would
    // propagate it to both axes and fail to discriminate the two tests.
    for(int axis=0;axis<2;++axis) {
        Fixture f;CaviarSceneProjectionScratch.shadow_centre_.values[axis]=std::numeric_limits<float>::quiet_NaN();f.draw();
        require(shadow_calls==(axis==0?1:0),"unordered-shadow-axis");
        require(f.record.secondary_clip_.width_==(axis==0?3:0),"load-unordered-shadow-axis");
    }
}
'''


def _inputs():
    source = (ROOT / "src/caviar.cpp").read_text()
    constants = []
    for name in ("vox_visibility_cut_", "vox_radius_scale_"):
        declaration, = re.findall(r"(?m)^const float " + name + r" = [^;]+;", source)
        constants.append(declaration)
    return "\n".join(constants), extract_definition(source, "vox_load_scene_view"), extract_definition(source, "vox_draw_scene")


def _compile(tmp_path, compiler, constants, load, draw):
    unit = HARNESS.replace("/* CONSTANTS */", constants).replace("/* LOAD_BODY */", load).replace("/* DRAW_BODY */", draw)
    cpp, exe = tmp_path / "scene.cpp", tmp_path / "scene"
    cpp.write_text(unit)
    result = subprocess.run([compiler,"-std=c++98","-O0",str(cpp),"-o",str(exe)],capture_output=True,text=True,timeout=40)
    return result,exe


def _run(tmp_path, compiler, constants, load, draw, null_setup=False):
    result,exe=_compile(tmp_path,compiler,constants,load,draw)
    assert result.returncode==0,result.stderr
    env={**os.environ,**({"CAVIAR_NULL_SETUP":"1"} if null_setup else {})}
    def disable_core():
        resource.setrlimit(resource.RLIMIT_CORE,(0,0))
    return subprocess.run([str(exe)],capture_output=True,text=True,timeout=10,
                          env=env,preexec_fn=disable_core if null_setup else None)


@pytest.mark.parametrize("name", ("g++","clang++"))
def test_whole_actual_scene_bodies_and_constants(tmp_path,name):
    compiler=shutil.which(name)
    if not compiler:pytest.skip(name+" unavailable")
    result=_run(tmp_path,compiler,*_inputs())
    assert result.returncode==0,result.stderr


@pytest.mark.parametrize("name",("g++","clang++"))
def test_actual_null_setup_faults_after_staging_even_for_empty_scene(tmp_path,name):
    compiler=shutil.which(name)
    if not compiler:pytest.skip(name+" unavailable")
    result=_run(tmp_path,compiler,*_inputs(),null_setup=True)
    assert result.returncode==-signal.SIGSEGV,result.stdout+result.stderr
    assert "witness: null-setup-after-staging" in result.stderr
    assert "null-setup-returned" not in result.stdout


def test_compiling_guarded_setup_damage_returns_instead_of_original_null_call(tmp_path):
    compiler=shutil.which("g++") or shutil.which("clang++")
    if not compiler:pytest.skip("host C++ compiler unavailable")
    constants,load,draw=_inputs()
    good="record->setup_callback_();"
    assert draw.count(good)==1
    draw=draw.replace(good,"if (record->setup_callback_) record->setup_callback_();")
    result=_run(tmp_path,compiler,constants,load,draw,null_setup=True)
    assert result.returncode==0,result.stdout+result.stderr
    assert "null-setup-returned" in result.stdout
    assert "witness: null-setup-after-staging" not in result.stderr


@pytest.mark.parametrize("damage,witness", (("cut-zero","visibility-below-cut"),("radius-zero","radius-inclusive-shadow-boundary"),
    ("load-radius-zero","load-radius-inclusive-boundary"),("cut-equality","visibility-equal-cut"),
    ("ordered-X","unordered-shadow-axis"),("load-ordered-X","load-unordered-shadow-axis"),
    ("cached-shadow","post-callback-disables-shadow"),("behind-continue","behind-keeps-shadow")))
def test_compiling_scene_damage_controls(tmp_path,damage,witness):
    compiler=shutil.which("g++") or shutil.which("clang++")
    if not compiler:pytest.skip("host C++ compiler unavailable")
    constants,load,draw=_inputs()
    if damage=="cut-zero":constants=re.sub(r"(vox_visibility_cut_ = )[^;]+",r"\g<1>0.0f",constants)
    elif damage=="radius-zero":constants=re.sub(r"(vox_radius_scale_ = )[^;]+",r"\g<1>0.0f",constants)
    elif damage=="load-radius-zero":
        assert load.count("vox_radius_scale_")==1
        load=load.replace("vox_radius_scale_","0.0f")
    elif damage=="cut-equality":
        assert draw.count("extent > vox_visibility_cut_")==1
        draw=draw.replace("extent > vox_visibility_cut_","extent >= vox_visibility_cut_")
    elif damage in ("ordered-X","load-ordered-X"):
        target=draw if damage=="ordered-X" else load
        target,count=re.subn(r"!\(\s*([^()>\n]+) > shadow.values\[0\]\)",r"(\1 <= shadow.values[0])",target)
        assert count==1
        if damage=="ordered-X":draw=target
        else:load=target
    elif damage=="cached-shadow":
        draw=draw.replace("for (int index = 0;", "const int cached_shadow = context->shadow_enabled_;\n    for (int index = 0;",1)
        draw,count=re.subn(r"if \(context->shadow_enabled_ != 0 &&", "if (cached_shadow != 0 &&",draw)
        assert count==1
    else:
        # Restore the prior loop-skipping edge before the corrected main guard.
        marker="if (vox_part_behind(&position, extent, view) == 0) {"
        assert marker in draw
        draw=draw.replace(marker,"if (vox_part_behind(&position, extent, view)) continue;\n        "+marker,1)
    result=_run(tmp_path,compiler,constants,load,draw)
    assert result.returncode!=0 and "witness: "+witness in result.stderr,result.stderr


@pytest.mark.parametrize("forced,witness", (("0","shadow-visibility-below-cut"),
                                             ("1","shadow-visibility-above-cut")))
def test_compiling_forced_shadow_visibility_damage_fails_actual_scene_body(tmp_path,forced,witness):
    compiler=shutil.which("g++") or shutil.which("clang++")
    if not compiler:pytest.skip("host C++ compiler unavailable")
    constants,load,draw=_inputs()
    call="vox_draw_shadow(record, node, &shadow, &shadow_part, visible);"
    assert draw.count(call)==1
    draw=draw.replace(call,"vox_draw_shadow(record, node, &shadow, &shadow_part, "+forced+");")
    result=_run(tmp_path,compiler,constants,load,draw)
    assert result.returncode!=0 and "witness: "+witness in result.stderr,result.stderr


def test_compiling_old_bounds_pointer_shadow_argument_is_rejected(tmp_path):
    compiler=shutil.which("g++") or shutil.which("clang++")
    if not compiler:pytest.skip("host C++ compiler unavailable")
    constants,load,draw=_inputs()
    call="vox_draw_shadow(record, node, &shadow, &shadow_part, visible);"
    assert draw.count(call)==1
    draw=draw.replace(call,"vox_draw_shadow(record, node, &shadow, &shadow_part, &view->bounds_);")
    result,_=_compile(tmp_path,compiler,constants,load,draw)
    assert result.returncode!=0
    assert "cannot convert" in result.stderr or "invalid conversion" in result.stderr


def test_pinned_constants_and_whole_function_control_flow_anchors():
    configured=os.environ.get("OPENSMACX_IMAGE")
    candidates=([Path(configured)] if configured else [])+[
        parent/".opensmacx/game/terranx_original.exe" for parent in (ROOT,*ROOT.parents)]
    path=next((p for p in candidates if p.is_file()),None)
    if path is None:pytest.skip("pinned original unavailable")
    image=PeImage(path.read_bytes(),PIN)
    assert struct.unpack("<ff",image.read(Span(0x671010,0x671018)))==(0.5,128.0)
    decoded={}
    for span in (Span(0x639E60,0x63A279),Span(0x63A280,0x63A6F0),
                 Span(0x63AD60,0x63AE12),Span(0x63AE20,0x63AED2)):
        cursor=span.start
        for instruction in Cs(CS_ARCH_X86,CS_MODE_32).disasm(image.read(span),span.start):
            assert instruction.address==cursor
            decoded[cursor]=(instruction.mnemonic,instruction.op_str);cursor+=instruction.size
        assert cursor==span.end
    expected={0x639FBC:("fcomp","dword ptr [0x671010]"),0x639FEA:("jne","0x63a0f3"),
        0x639F25:("call","dword ptr [edi + 0x7c]"),
        0x639F28:("mov","eax, dword ptr [ebx + 4]"),
        0x639F33:("jle","0x63a26a"),
        0x63A0EE:("call","eax"),0x63A0F7:("mov","cl, byte ptr [eax + 0x8c]"),
        0x63A144:("fmul","dword ptr [0x671014]"),0x63A47E:("mov","al, byte ptr [edx + 0x8c]"),
        0x63A4D0:("fmul","dword ptr [0x671014]"),
        0x63AD8B:("mov","dword ptr [eax + 0x7c], 0x63af20"),
        0x63ADAF:("mov","dword ptr [eax + 0x7c], 0x63aee0"),
        0x63ADD8:("mov","dword ptr [eax + 0x7c], 0x63af40"),
        0x63ADFC:("mov","dword ptr [eax + 0x7c], 0x63af00"),
        0x63AE4B:("mov","dword ptr [eax + 0x7c], 0x63af20"),
        0x63AE6F:("mov","dword ptr [eax + 0x7c], 0x63aee0"),
        0x63AE98:("mov","dword ptr [eax + 0x7c], 0x63af40"),
        0x63AEBC:("mov","dword ptr [eax + 0x7c], 0x63af00")}
    for address,pair in expected.items():assert decoded[address]==pair,hex(address)
