"""Actual matrix bodies, row ownership and sequential-alias controls.

Host float tests establish grouping under host evaluation, not x87 extended
precision equivalence. The optional pinned-image test independently checks the
original expression trees and store order. No renderer/runtime claim is made.
"""

import ast
import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
VECTOR = "vox_matrix_vector_mul"
MATRIX = "vox_matrix_matrix_mul"


def braced(text, start):
    depth = 0
    for end in range(text.index("{", start), len(text)):
        depth += (text[end] == "{") - (text[end] == "}")
        if depth == 0:
            return text[start:end + 1]
    raise AssertionError("unterminated source body")


def sources():
    source = (ROOT / "src/caviar.cpp").read_text()
    result = {}
    for name in (VECTOR, MATRIX, "vox_draw_scene", "vox_transform_camera"):
        match, = re.finditer(r"void __cdecl " + name + r"\([^;{]*\)\s*\{", source)
        result[name] = braced(source, match.start())
    draw = result["vox_draw_scene"]
    result["main_scale"] = braced(draw, draw.index("for (int row ="))
    result["shadow_scale"] = braced(draw, draw.index("for (int shadow_row ="))
    calls = []
    for name in ("vox_draw_scene", "vox_transform_camera"):
        calls += re.findall(r"vox_matrix_(?:vector|matrix)_mul\([^;]*\);", result[name])
    assert len(calls) == 7
    result["calls"] = "\n".join(calls)
    header = (ROOT / "src/caviar.h").read_text()
    result["scene_state_type"] = braced(header, header.index("struct VoxSceneState {")) + ";"
    result["scene_state_type"] = "struct VoxInstance; struct CvrStep;\n" + result["scene_state_type"]
    result["projection_support"] = (
        '#include "' + str(ROOT / "src/caviar_projection_state.h") + '"\n'
        "CaviarSceneProjectionScratchCandidate CaviarSceneProjectionScratch = {};"
    )
    result["types"] = "\n".join(braced(header, header.index("struct " + name + " {")) + ";"
                                   for name in ("VOX_Matrix", "VOX_Vect"))
    return result


def harness(parts):
    return r'''
        #include <cstdint>
        #include <cstring>
        #include <cstdlib>
        #define __cdecl
        TYPES
        SCENE_STATE_TYPE
        PROJECTION_SUPPORT
        VECTOR_BODY
        MATRIX_BODY
        void scale_main(VOX_Matrix &part, float extent) { MAIN_SCALE }
        void scale_shadow(VOX_Matrix &shadow_part, float extent) { SHADOW_SCALE }
        void caller_contract() {
            VOX_Matrix vox_view_rotation_={}, vox_shadow_rotation_={}, part={}, shadow_part={};
            VOX_Vect position={}, shadow={}, rotated={}, offset={}, pivot={};
            struct State { VOX_Matrix scene_rotation_; } state_value={};
            State *state = &state_value;
            struct Instance { VOX_Matrix instance_matrix_[1]; VOX_Vect instance_vector_[1]; } instance={};
            int step = 0;
            CALLS
        }
        void reference_vector(const float (*m)[3], const float *v, float *out) {
            const int order[3][3] = {{1,2,0}, {1,2,0}, {2,1,0}};
            for (int row=0; row<3; ++row) {
                float first=m[row][order[row][0]]*v[order[row][0]];
                float second=m[row][order[row][1]]*v[order[row][1]];
                float third=m[row][order[row][2]]*v[order[row][2]];
                out[row]=(first+second)+third;
            }
        }
        void reference_matrix(const float (*left)[3], const float (*right)[3], float (*out)[3]) {
            const int order[9][3] = {{1,2,0},{0,1,2},{0,2,1},
                                     {0,2,1},{0,2,1},{2,1,0},
                                     {0,1,2},{2,1,0},{0,1,2}};
            for (int index=0; index<9; ++index) {
                int row=index/3, col=index%3;
                float first=left[row][order[index][0]]*right[order[index][0]][col];
                float second=left[row][order[index][1]]*right[order[index][1]][col];
                float third=left[row][order[index][2]]*right[order[index][2]][col];
                out[row][col]=(first+second)+third;
            }
        }
        void fill(float (*rows)[3], int epoch) {
            const float special[7] = {33554432.0f,-33554432.0f,1.0f,0.0f,-1.0f,2.0f,0.5f};
            for (int r=0; r<12; ++r) for (int c=0; c<3; ++c)
                rows[r][c] = epoch == 0 ? ((r*7+c*3)%17-8)*0.25f
                                        : special[(r*3+c+epoch)%7];
        }
        void require(bool ok) { if (!ok) std::exit(42); }
        int main() {
            caller_contract(); // All seven actual call expressions must type-check.
            const int vector_cases[][3] = {{0,4,8},{0,4,4},{0,4,0},{0,4,1},
                                           {0,1,1},{0,0,0},{2,3,4}};
            const int matrix_cases[][3] = {{0,4,8},{0,4,0},{0,4,4},{0,0,0},
                                           {0,4,1},{0,4,5},{0,1,2},{0,0,8}};
            for (int epoch=0; epoch<8; ++epoch) {
                for (auto &indices : vector_cases) {
                    float actual[12][3], expected[12][3];
                    fill(actual, epoch); std::memcpy(expected,actual,sizeof(actual));
                    vox_matrix_vector_mul(actual+indices[0],actual[indices[1]],actual[indices[2]]);
                    reference_vector(expected+indices[0],expected[indices[1]],expected[indices[2]]);
                    require(std::memcmp(actual,expected,sizeof(actual)) == 0);
                }
                for (auto &indices : matrix_cases) {
                    float actual[12][3], expected[12][3];
                    fill(actual, epoch); std::memcpy(expected,actual,sizeof(actual));
                    vox_matrix_matrix_mul(actual+indices[0],actual+indices[1],actual+indices[2]);
                    reference_matrix(expected+indices[0],expected+indices[1],expected+indices[2]);
                    require(std::memcmp(actual,expected,sizeof(actual)) == 0);
                }
            }
            // A cancellation control distinguishes (p1+p2)+p0 from (p0+p1)+p2.
            float ones[3][3]={{1,1,1},{1,1,1},{1,1,1}};
            float v[3]={33554432.0f,-33554432.0f,1.0f}, a[3], e[3];
            vox_matrix_vector_mul(ones,v,a); reference_vector(ones,v,e);
            require(std::memcmp(a,e,sizeof(a)) == 0);
            float right[3][3]={{v[0],v[0],v[0]},{v[1],v[1],v[1]},{v[2],v[2],v[2]}};
            float product[3][3], reference[3][3];
            vox_matrix_matrix_mul(ones,right,product); reference_matrix(ones,right,reference);
            require(std::memcmp(product,reference,sizeof(product)) == 0);
            struct Guard { uint32_t before; VOX_Matrix matrix; uint32_t after; } main, shadow, want_main, want_shadow;
            main.before=0x12345678; main.after=0x87654321;
            for (int r=0;r<3;++r) for (int c=0;c<3;++c) main.matrix.values[r][c]=r*3+c+1;
            shadow=want_main=want_shadow=main;
            for (int r=0;r<3;++r) for (int c=0;c<3;++c) want_main.matrix.values[r][c]*=2.5f;
            for (int r=0;r<2;++r) for (int c=0;c<3;++c) want_shadow.matrix.values[r][c]*=2.5f;
            scale_main(main.matrix,2.5f); scale_shadow(shadow.matrix,2.5f);
            require(std::memcmp(&main,&want_main,sizeof(main)) == 0);
            require(std::memcmp(&shadow,&want_shadow,sizeof(shadow)) == 0);
        }
    '''.replace("TYPES", parts["types"]).replace("SCENE_STATE_TYPE", parts["scene_state_type"]).replace("PROJECTION_SUPPORT", parts["projection_support"]).replace("VECTOR_BODY", parts[VECTOR]).replace(
        "MATRIX_BODY", parts[MATRIX]).replace("MAIN_SCALE", parts["main_scale"]).replace(
        "SHADOW_SCALE", parts["shadow_scale"]).replace("CALLS", parts["calls"])


def run_host(tmp_path, parts):
    compiler = shutil.which(os.environ.get("OSMX_HOST_CXX", "g++"))
    if compiler is None:
        pytest.skip("host g++ required")
    path, executable = tmp_path / "matrix.cpp", tmp_path / "matrix"
    path.write_text(harness(parts))
    compiled = subprocess.run([compiler,"-std=c++11","-O1","-ffp-contract=off",
        "-fsanitize=undefined,bounds","-fno-sanitize-recover=all",str(path),"-o",str(executable)],
        capture_output=True,text=True,timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)],capture_output=True,text=True,timeout=10)


def test_actual_arithmetic_aliases_callers_and_scaling(tmp_path):
    result=run_host(tmp_path,sources())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("operation", [VECTOR,MATRIX])
def test_delayed_output_store_damage_is_rejected(tmp_path,operation):
    parts=sources()
    function=parts[operation]
    opening=function.index("{")+1
    statements=function[opening:function.rindex("}")]
    statements=re.sub(r"//[^\n]*","",statements).replace("out[","temporary[")
    temporary="float temporary[3];" if operation == VECTOR else "float temporary[3][3];"
    parts[operation]=function[:opening]+temporary+statements+"std::memcpy(out,temporary,sizeof(temporary));}"
    assert run_host(tmp_path,parts).returncode == 42


@pytest.mark.parametrize("operation", [VECTOR,MATRIX])
def test_reversed_store_order_is_rejected(tmp_path,operation):
    parts=sources();function=parts[operation]
    statements=re.findall(r"out\[[^;]+;",function)
    assert len(statements) == (3 if operation == VECTOR else 9)
    parts[operation]=function[:function.index("{")+1]+"\n".join(reversed(statements))+"}"
    assert run_host(tmp_path,parts).returncode == 42


@pytest.mark.parametrize("operation", [VECTOR,MATRIX])
def test_regrouped_first_dot_product_is_rejected(tmp_path,operation):
    parts=sources()
    replacement=("out[0] = (matrix[0][0]*vector[0]+matrix[0][1]*vector[1])+matrix[0][2]*vector[2];"
                 if operation == VECTOR else
                 "out[0][0] = (left[0][0]*right[0][0]+left[0][1]*right[1][0])+left[0][2]*right[2][0];")
    parts[operation],count=re.subn(r"out\[0\](?:\[0\])?\s*=[^;]+;",replacement,parts[operation],count=1)
    assert count == 1
    assert run_host(tmp_path,parts).returncode == 42


@pytest.mark.parametrize("name,before,after", [
    ("main_scale","row < 3","row < 2"),
    ("shadow_scale","shadow_row < 2","shadow_row < 3"),
])
def test_wrong_scaling_extent_is_rejected(tmp_path,name,before,after):
    parts=sources();assert before in parts[name]
    parts[name]=parts[name].replace(before,after)
    assert run_host(tmp_path,parts).returncode == 42


@pytest.mark.parametrize("name,object_name,count", [("main_scale","part",9),("shadow_scale","shadow_part",6)])
def test_retired_inner_row_flattening_is_rejected(tmp_path,name,object_name,count):
    parts=sources()
    parts[name]=f"for(int element=0;element<{count};++element) {object_name}.values[0][element]*=extent;"
    result=run_host(tmp_path,parts)
    assert result.returncode != 0 and "out of bounds" in result.stderr


def source_expression(node):
    if isinstance(node,ast.BinOp):
        assert isinstance(node.op,(ast.Add,ast.Mult))
        return ("+" if isinstance(node.op,ast.Add) else "*",source_expression(node.left),source_expression(node.right))
    indices=[]
    while isinstance(node,ast.Subscript):
        indices.insert(0,node.slice.value);node=node.value
    assert isinstance(node,ast.Name)
    return (node.id,*indices)


def test_pinned_original_grouping_loads_and_sequential_stores():
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image unavailable; set OPENSMACX_IMAGE")
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    image=PeImage(path.read_bytes(),PIN)
    decoder=Cs(CS_ARCH_X86,CS_MODE_32);decoder.detail=True
    parts=sources()
    for name,start,end in ((VECTOR,0x6280E0,0x62814B),(MATRIX,0x627E20,0x627F4F)):
        stack=[];original=[]
        def memory(instruction):
            operand=instruction.operands[0].mem
            index=operand.disp//4
            base=instruction.reg_name(operand.base)
            assert operand.disp%4 == 0 and operand.index == 0
            owner={"eax":"matrix" if name==VECTOR else "left",
                   "ecx":"vector" if name==VECTOR else "right","edx":"out"}[base]
            return (owner,index) if name==VECTOR and owner in ("vector","out") else (owner,index//3,index%3)
        for instruction in decoder.disasm(image.read(Span(start,end)),start):
            op=instruction.mnemonic
            if op=="fld":stack.insert(0,memory(instruction))
            elif op=="fmul":stack[0]=("*",stack[0],memory(instruction))
            elif op=="fxch":
                index=int(re.search(r"\d+",instruction.op_str).group())
                stack[0],stack[index]=stack[index],stack[0]
            elif op=="faddp":stack[1]=("+",stack[1],stack[0]);stack.pop(0)
            elif op=="fstp":
                original.append((memory(instruction),stack.pop(0)))
                assert not stack  # No values cached across output stores.
            else:assert op in ("mov","ret")
        assignments=re.findall(r"(out\[\d\](?:\[\d\])?)\s*=\s*([^;]+);",parts[name])
        actual=[(source_expression(ast.parse(left,mode="eval").body),
                 source_expression(ast.parse("("+right.strip()+")",mode="eval").body)) for left,right in assignments]
        assert len(actual)==(3 if name==VECTOR else 9)
        assert actual==original
