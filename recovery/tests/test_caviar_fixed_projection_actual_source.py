"""Actual-source fixed projection and typed table-lookup controls.

Extracted definitions are never rewritten. Host-width fixtures do not
establish native ABI, storage ownership or complete original caller closure.
"""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/caviar.cpp").is_file())
PRODUCT = ROOT / "src/caviar.cpp"


def extract(source, signature):
    # Mask comments and strings so historical annotations cannot masquerade as
    # installed definitions.
    from match_recovery.includes import COMMENTS_OR_STRINGS
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), source)
    start = masked.find(signature)
    while start >= 0:
        opening = masked.find("{", start)
        semi = masked.find(";", start)
        if opening >= 0 and (semi < 0 or opening < semi): break
        start = masked.find(signature, start + len(signature))
    assert start >= 0, "installed function missing: " + signature
    depth = 0
    for i in range(opening, len(masked)):
        if masked[i] == "{": depth += 1
        elif masked[i] == "}":
            depth -= 1
            if depth == 0: return source[start:i + 1]
    raise AssertionError("unbalanced installed function: " + signature)


def extract_definition(source, signature):
    return extract(source, signature)


PROLOGUE = r"""
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#define __cdecl
typedef struct { int16_t word_00_,word_02_,word_04_,word_06_,word_08_,word_0A_;
  int32_t dword_0C_,dword_10_,dword_14_; } CaviarFixedPointProjectionMatrix;
CaviarFixedPointProjectionMatrix CaviarFixedPointProjection = {};
struct CaviarBlitStateCandidate { unsigned char * volatile destination_cursor_;
  uint32_t source_row_bytes_; uint32_t destination_stride_bytes_; };
CaviarBlitStateCandidate CaviarBlitState = {};
"""


def compile_run(definitions, main):
    compilers = [shutil.which("g++"), shutil.which("clang++")]
    assert all(compilers), "host g++ and clang++ required"
    for compiler in compilers:
        with tempfile.TemporaryDirectory() as d:
            cpp = Path(d) / "actual.cpp"
            exe = Path(d) / "actual"
            cpp.write_text(PROLOGUE + "\n" + "\n".join(definitions) + "\n" + main)
            build = subprocess.run([compiler, "-std=c++11", "-O0", str(cpp), "-o", str(exe)],
                                   text=True, stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT)
            assert build.returncode == 0, build.stdout
            run = subprocess.run([str(exe)], text=True, stdout=subprocess.PIPE,
                                 stderr=subprocess.STDOUT)
            assert run.returncode == 0, run.stdout


def test_installed_6401b0_and_63b241_bodies_have_typed_candidate_semantics():
    source = PRODUCT.read_text()
    fixed8 = extract_definition(source, "int32_t fixed8(uint32_t value_bits)")
    functions = [
        extract_definition(source, "static int32_t caviar_i32_from_bits("),
        extract_definition(source, "static uint32_t caviar_sar8_bits("),
        extract_definition(source, "static int32_t caviar_fixed8_low16("),
        extract_definition(source, "static int32_t caviar_add_i32("),
        extract_definition(source, "static int32_t caviar_sub_i32("),
        fixed8,
        extract_definition(source, "void __cdecl sub_6401b0("),
    ]
    blit = extract_definition(source, "void __cdecl sub_63b241(")
    assert "const unsigned char *table" in blit
    assert "*pixel = table[*pixel];" in blit
    assert "table_base" not in blit
    # The production pointer arithmetic is retained verbatim; low mappings
    # keep its 32-bit address representation valid in this host-only fixture.
    functions.extend([
        extract_definition(source, "static uint32_t caviar_blit_offset("),
        extract_definition(source, "static unsigned char *caviar_blit_address("),
        extract_definition(source, "static const char *caviar_blit_source_address("),
    ])
    functions.append(blit)
    main = r"""
static void ck(bool b,const char*s){if(!b){fprintf(stderr,"FAIL %s\n",s);exit(1);}}
static void *lowmap(size_t n){void*p=mmap(0,n,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT,-1,0);return p==MAP_FAILED?0:p;}
static int32_t frombits(uint32_t b){return b<=0x7fffffffu?(int32_t)b:-1-(int32_t)(0xffffffffu-b);}
static int32_t oracle_sar(uint32_t b){b+=0x80u;return frombits((b>>8)|((b&0x80000000u)?0xff000000u:0));}
int main(){
  const uint32_t bits[]={0u,0x7fu,0x80u,0xffu,0x7fffffffu,0x80000000u,0xffffff7fu,0xffffff80u,0xffffffffu};
  for(unsigned i=0;i<sizeof(bits)/sizeof(bits[0]);++i)ck(frombits(caviar_sar8_bits(bits[i]))==oracle_sar(bits[i]),"installed SAR");
  CaviarFixedPointProjection.word_00_=32767; CaviarFixedPointProjection.word_02_=0;
  CaviarFixedPointProjection.word_04_=0; CaviarFixedPointProjection.word_06_=0;
  CaviarFixedPointProjection.word_08_=0; CaviarFixedPointProjection.word_0A_=0;
  short c[3]={32767,0,0},e[3]={0,0,0}; int r[6]={};
  int32_t x=oracle_sar((uint32_t)((int64_t)32767*32767));
  sub_6401b0(c,e,r); ck(r[0]==32765&&r[2]==x+2&&r[4]==x-32762,"installed full-width center");
  CaviarFixedPointProjection.word_00_=32767; CaviarFixedPointProjection.word_02_=0;
  CaviarFixedPointProjection.word_04_=0; CaviarFixedPointProjection.word_06_=0;
  CaviarFixedPointProjection.word_08_=0; CaviarFixedPointProjection.word_0A_=0;
  short zc[3]={0,0,0},ze[3]={32767,0,0};
  ck(caviar_sar8_bits((uint32_t)((int64_t)32767*32767))==0x003fff00u&&
     caviar_fixed8_low16((uint32_t)((int64_t)32767*32767))==-256,
     "installed full SAR then AX/low16");
  for(int i=0;i<6;++i)r[i]=0;sub_6401b0(zc,ze,r);
  ck(r[0]==-258&&r[2]==2&&r[1]==-2&&r[3]==2&&r[4]==261&&r[5]==5,
     "installed nonzero extent AX behavior");
  CaviarFixedPointProjection.word_02_=32767; CaviarFixedPointProjection.word_04_=32767;
  c[1]=32767;c[2]=32767;uint32_t sum=(uint32_t)((int64_t)32767*32767*3);
  int32_t wx=oracle_sar(sum);for(int i=0;i<6;++i)r[i]=0;sub_6401b0(c,e,r);
  ck(r[0]==wx-2&&r[2]==-32765,"installed modulo-32 center sum");
  unsigned char *dst=(unsigned char*)lowmap(4096);char*src=(char*)lowmap(4096);
  unsigned char *table=(unsigned char*)lowmap(4096);ck(dst&&src&&table,"low memory mapping");
  dst[0]=3;dst[1]=9;dst[2]=2;dst[3]=4;src[0]=1;src[1]=0;src[2]=2;src[3]=3;
  for(unsigned i=0;i<256;++i)table[i]=(unsigned char)(255-i);
  sub_63b241(dst,src,0,0,4,1,0x10000,0x10000,4,8,table);
  ck(dst[0]==252&&dst[1]==9&&dst[2]==253&&dst[3]==251,"installed typed lookup and zero skip");
  return 0;
}
"""
    compile_run(functions, main)
