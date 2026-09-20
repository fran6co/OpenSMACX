"""Actual geometry bodies and bounded mathematical/aliasing controls.

Host tests are not x87 precision proofs. The pinned-image trace separately
checks arithmetic grouping, store order, input phases and intrinsic choice;
it deliberately does not equate register precision with float spill rounding.
"""

import ast
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess

import pytest

from test_caviar_matrix_operations import braced


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
NAMES = ("vox_matrix_inverse", "vox_cross_product", "caviar_apply_rotation")


def bodies():
    text = (ROOT / "src/caviar.cpp").read_text()
    result = {}
    for name in NAMES:
        match, = re.finditer(r"void __cdecl " + name + r"\([^;{]*\)\s*\{", text)
        result[name] = braced(text, match.start())
    header = (ROOT / "src/caviar.h").read_text()
    result["matrix_type"] = braced(header, header.index("struct VOX_Matrix {")) + ";"
    return result


def run_host(tmp_path, parts):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    program = r'''
        #include <cmath>
        #include <cstring>
        #include <cstdlib>
        #define __cdecl
        MATRIX_TYPE
        BODIES
        void require(bool ok) { if (!ok) std::exit(42); }
        void equal(float a, float b) {
            if (std::isnan(b)) require(std::isnan(a));
            else if (std::isinf(b)) require(a == b);
            else require(std::fabs(a-b) <= 0.00003f*(1+std::fabs(b)));
        }
        void inverse_cases() {
            const float cases[][3][3] = {
                {{1,0,0},{0,1,0},{0,0,1}}, {{2,0,0},{0,-4,0},{0,0,8}},
                {{1,2,3},{0,1,4},{5,6,0}}, {{2,-1,3},{4,2,1},{-2,3,5}},
                {{1,0,0},{0,1,0},{0,0,0}}, {{0,0,0},{0,0,0},{0,0,0}}
            };
            for (int k=0;k<6;++k) {
                float expected[3][3]={}; vox_matrix_inverse(cases[k],expected);
                if (k<4) {
                    for(int r=0;r<3;++r) for(int c=0;c<3;++c) {
                        double product=0;
                        for(int j=0;j<3;++j) product+=double(cases[k][r][j])*expected[j][c];
                        equal(float(product),r==c?1.0f:0.0f);
                    }
                } else for(int r=0;r<3;++r) for(int c=0;c<3;++c) {
                    if(k==4 && r==2 && c==2) require(std::isinf(expected[r][c]) && expected[r][c]>0);
                    else require(std::isnan(expected[r][c]));
                }
                for(int delta=-2;delta<=2;++delta) {
                    float rows[9][3], before[9][3];
                    for(int r=0;r<9;++r) for(int c=0;c<3;++c) rows[r][c]=float(100+r*3+c);
                    std::memcpy(rows+3,cases[k],sizeof(cases[k]));
                    std::memcpy(before,rows,sizeof(rows));
                    vox_matrix_inverse(rows+3,rows+3+delta);
                    for(int r=0;r<9;++r) for(int c=0;c<3;++c)
                        if(r>=3+delta && r<6+delta) equal(rows[r][c],expected[r-3-delta][c]);
                        else require(rows[r][c]==before[r][c]);
                }
            }
        }
        void cross_cases() {
            for(int left=0;left<4;++left) for(int right=0;right<4;++right)
            for(int out=0;out<4;++out) {
                float actual[9],expected[9];
                for(int n=0;n<9;++n) actual[n]=float((n*7)%13-5);
                std::memcpy(expected,actual,sizeof(actual));
                for(int n=0;n<3;++n) {
                    int j=(n+1)%3,k=(n+2)%3;
                    expected[out+n]=expected[left+j]*expected[right+k]-expected[right+j]*expected[left+k];
                }
                vox_cross_product(actual+left,actual+right,actual+out);
                require(std::memcmp(actual,expected,sizeof(actual))==0);
            }
        }
        void rotation_cases() {
            const float angles[][3]={{0,0,0},{0.5f,0,0},{0,0.7f,0},{0,0,-0.3f},{0.4f,-0.7f,1.2f}};
            for(int n=0;n<5;++n) {
                double x=angles[n][0],y=angles[n][1],z=angles[n][2];
                // Independent Rz(x) * Ry(y) * Rx(z) construction.
                double rz[3][3]={{cos(x),-sin(x),0},{sin(x),cos(x),0},{0,0,1}};
                double ry[3][3]={{cos(y),0,sin(y)},{0,1,0},{-sin(y),0,cos(y)}};
                double rx[3][3]={{1,0,0},{0,cos(z),-sin(z)},{0,sin(z),cos(z)}};
                double intermediate[3][3]={},expected[3][3]={};
                for(int r=0;r<3;++r) for(int c=0;c<3;++c) for(int j=0;j<3;++j)
                    intermediate[r][c]+=rz[r][j]*ry[j][c];
                for(int r=0;r<3;++r) for(int c=0;c<3;++c) for(int j=0;j<3;++j)
                    expected[r][c]+=intermediate[r][j]*rx[j][c];
                for(int alias=-1;alias<3;++alias) {
                    struct Guard { float before; VOX_Matrix matrix; float after; } value;
                    value.before=123;value.after=456;
                    for(int r=0;r<3;++r) for(int c=0;c<3;++c) value.matrix.values[r][c]=-99;
                    const float *input=angles[n];
                    if(alias>=0) { std::memcpy(value.matrix.values[alias],input,sizeof(angles[n]));input=value.matrix.values[alias]; }
                    caviar_apply_rotation(input,&value.matrix);
                    for(int r=0;r<3;++r) for(int c=0;c<3;++c) equal(value.matrix.values[r][c],float(expected[r][c]));
                    require(value.before==123 && value.after==456);
                }
            }
        }
        int main() { inverse_cases();cross_cases();rotation_cases(); }
    '''.replace("MATRIX_TYPE", parts["matrix_type"]).replace("BODIES", "\n".join(parts[n] for n in NAMES))
    source, executable = tmp_path / "geometry.cpp", tmp_path / "geometry"
    source.write_text(program)
    compiled = subprocess.run([compiler,"-std=c++11","-O1","-ffp-contract=off",
        "-fsanitize=undefined,bounds","-fno-sanitize-recover=all",str(source),"-o",str(executable)],
        capture_output=True,text=True,timeout=30)
    assert compiled.returncode == 0, compiled.stderr
    return subprocess.run([str(executable)],capture_output=True,text=True,timeout=10)


def test_actual_bodies_math_singularities_aliases_and_guards(tmp_path):
    result = run_host(tmp_path,bodies())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("damage", ["inverse_sign","inverse_late_input","singular_guard",
                                   "cross_snapshot","cross_reverse","rotation_axis","rotation_late_angle"])
def test_behavior_damage_controls(tmp_path,damage):
    parts = bodies()
    inverse,cross,rotation = NAMES
    if damage == "inverse_sign":
        parts[inverse] = parts[inverse].replace("-(reciprocal * c01)","(reciprocal * c01)")
    elif damage == "inverse_late_input":
        declaration = re.search(r"    float c22 = [^;]+;\n",parts[inverse]).group()
        parts[inverse] = parts[inverse].replace(declaration,"").replace("    out[2][2]",declaration+"    out[2][2]")
    elif damage == "singular_guard":
        parts[inverse] = parts[inverse].replace("    out[0][0]", "    if (!std::isfinite(reciprocal)) return;\n    out[0][0]")
    elif damage == "cross_snapshot":
        opening=parts[cross].index("{")+1
        parts[cross]=parts[cross][:opening]+"float temporary[3];"+parts[cross][opening:].replace("out[","temporary[")
        parts[cross]=parts[cross][:-1]+"std::memcpy(out,temporary,sizeof(temporary));}"
    elif damage == "cross_reverse":
        statements=re.findall(r"out\[[^;]+;",parts[cross])
        parts[cross]=parts[cross][:parts[cross].index("{")+1]+"\n".join(reversed(statements))+"}"
    elif damage == "rotation_axis":
        parts[rotation]=parts[rotation].replace("sin(angles[0])","sin(angles[1])")
    else:
        parts[rotation]=parts[rotation].replace("= -sy;","= -(float)sin(angles[1]);")
    result=run_host(tmp_path,parts)
    assert result.returncode == 42, result.stderr


def expression(node,variables):
    if isinstance(node,ast.BinOp):
        symbols={ast.Add:"+",ast.Sub:"-",ast.Mult:"*",ast.Div:"/"}
        return (symbols[type(node.op)],expression(node.left,variables),expression(node.right,variables))
    if isinstance(node,ast.UnaryOp):
        assert isinstance(node.op,ast.USub)
        return ("neg",expression(node.operand,variables))
    if isinstance(node,ast.Call):
        assert isinstance(node.func,ast.Name) and node.func.id in ("sin","cos") and len(node.args)==1
        return (node.func.id,expression(node.args[0],variables))
    if isinstance(node,ast.Constant):return node.value
    if isinstance(node,ast.Name):return variables[node.id]
    indices=[]
    while isinstance(node,ast.Subscript):
        indices.insert(0,node.slice.value);node=node.value
    assert isinstance(node,ast.Name)
    return (node.id,*indices)


def source_trace(body):
    variables={};stores=[]
    body=body[body.index("{")+1:body.rindex("}")]
    body=body.replace("matrix->values","out").replace("(float)","")
    body=re.sub(r"\b(\d+\.\d+)f\b",r"\1",body)
    for statement in body.split(";"):
        if not statement.strip():continue
        left,right=statement.strip().split("=",1)
        value=expression(ast.parse("("+right.strip()+")",mode="eval").body,variables)
        if left.startswith("float "):variables[left[6:].strip()]=value
        else:stores.append((expression(ast.parse(left.strip(),mode="eval").body,variables),value))
    return stores


def original_trace(image,start,end):
    from capstone import Cs,CS_ARCH_X86,CS_MODE_32
    from capstone.x86 import X86_OP_REG
    from match_recovery.model import Span
    decoder=Cs(CS_ARCH_X86,CS_MODE_32);decoder.detail=True
    instructions=list(decoder.disasm(image.read(Span(start,end)),start))
    assert instructions[-1].mnemonic=="ret" and instructions[-1].address+instructions[-1].size==end
    stack=[];scratch={};stores=[];input_reads=[];output_addresses=[];trig=[]
    inverse=start==0x627F50;rotation=start==0x627D00
    output_pointer=False;rotation_offset=20
    def regindex(name):return int(re.search(r"\d+",name).group())
    def read(instruction,operand):
        if operand.type==X86_OP_REG:return stack[regindex(instruction.reg_name(operand.reg))]
        memory=operand.mem;base=instruction.reg_name(memory.base);offset=memory.disp
        assert memory.index==0
        if base=="esp":return scratch[offset]
        if not base:
            assert offset==0x670C08 and image.read(Span(offset,offset+4))==struct.pack("<f",1.0)
            return 1.0
        input_reads.append(instruction.address)
        assert offset%4==0
        if rotation:
            assert base=="ecx"
            return ("angles",offset//4)
        if inverse:
            assert base=="eax" and not output_pointer
            return ("matrix",offset//12,offset//4%3)
        assert base in ("eax","ecx")
        return ("left" if base=="eax" else "right",offset//4)
    for instruction in instructions:
        op=instruction.mnemonic;operands=instruction.operands
        if op=="fld":stack.insert(0,read(instruction,operands[0]))
        elif op=="fxch":
            index=regindex(instruction.op_str);stack[0],stack[index]=stack[index],stack[0]
        elif op in ("fsin","fcos"):
            trig.append(op);stack[0]=(op[1:],stack[0])
        elif op=="fchs":stack[0]=("neg",stack[0])
        elif op in ("fmul","fdivr"):
            value=read(instruction,operands[0])
            stack[0]=("*",stack[0],value) if op=="fmul" else ("/",value,stack[0])
        elif op in ("faddp","fsubp"):
            index=regindex(instruction.op_str)
            stack[index]=("+" if op=="faddp" else "-",stack[index],stack[0]);stack.pop(0)
        elif op in ("fst","fstp"):
            operand=operands[0]
            if operand.type==X86_OP_REG:stack[regindex(instruction.reg_name(operand.reg))]=stack[0]
            else:
                memory=operand.mem;base=instruction.reg_name(memory.base);offset=memory.disp
                if base=="esp":scratch[offset]=stack[0]
                else:
                    if rotation and base=="eax":offset+=rotation_offset
                    target=("out",offset//12,offset//4%3) if inverse or rotation else ("out",offset//4)
                    stores.append((target,stack[0]));output_addresses.append(instruction.address)
            if op=="fstp":stack.pop(0)
        elif inverse and instruction.address==0x628045:output_pointer=True
        elif rotation and instruction.address>=0x627E00 and op=="add" and instruction.op_str=="eax, 4":rotation_offset+=4
        else:assert op in ("mov","lea","add","sub","ret")
    assert not stack
    if inverse or rotation:assert max(input_reads)<min(output_addresses)
    else:
        assert any(output_addresses[0]<read<output_addresses[1] for read in input_reads)
        assert any(output_addresses[1]<read<output_addresses[2] for read in input_reads)
    assert sorted(trig)==(["fcos"]*3+["fsin"]*3 if rotation else [])
    return stores


@pytest.mark.parametrize("name,start,end",[(NAMES[0],0x627F50,0x6280DF),
                                           (NAMES[1],0x628290,0x6282D1),
                                           (NAMES[2],0x627D00,0x627E19)])
def test_pinned_original_grouping_stores_input_phases_and_intrinsics(name,start,end):
    path=Path(os.environ.get("OPENSMACX_IMAGE",ROOT/".opensmacx/game/terranx_original.exe"))
    if not path.is_file():pytest.skip("pinned original unavailable; set OPENSMACX_IMAGE")
    from match_recovery.pe_coff import PeImage
    image=PeImage(path.read_bytes(),PIN)
    assert source_trace(bodies()[name])==original_trace(image,start,end)
