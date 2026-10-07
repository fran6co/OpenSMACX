"""Installed-source continuity and bounded controls for five Caviar bodies."""
import dataclasses
from pathlib import Path
import shutil
import subprocess
import tempfile
import pytest

from match_recovery.includes import COMMENTS_OR_STRINGS
from match_recovery.source import reader

ROOT = next(p for p in Path(__file__).resolve().parents
            if (p / "src/caviar.cpp").is_file())
PRODUCT = ROOT / "src/caviar.cpp"

def extract(source, signature):
    masked = COMMENTS_OR_STRINGS.sub(
        lambda m: "".join("\n" if c == "\n" else " " for c in m[0]), source)
    start = masked.find(signature)
    while start >= 0:
        opening = masked.find("{", start)
        semicolon = masked.find(";", start)
        if opening >= 0 and (semicolon < 0 or opening < semicolon): break
        start = masked.find(signature, start + len(signature))
    assert start >= 0 and opening >= 0, "missing actual Caviar definition: " + signature
    depth = 0
    for i in range(opening, len(masked)):
        if masked[i] == "{": depth += 1
        elif masked[i] == "}":
            depth -= 1
            if depth == 0: return source[start:i + 1]
    raise AssertionError("unbalanced actual body: " + signature)

HARNESS = r'''#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#define __cdecl
typedef struct { int16_t word_00_,word_02_,word_04_,word_06_,word_08_,word_0A_;
  int32_t dword_0C_,dword_10_,dword_14_; } CaviarFixedPointProjectionMatrix;
// Host-width state fixture: typed source shape, not a 32-bit ABI/layout test.
struct CaviarBlitStateCandidate {
  unsigned char * volatile destination_cursor_;
  uint32_t source_row_bytes_;
  uint32_t destination_stride_bytes_;
};
CaviarBlitStateCandidate CaviarBlitState={0,0,0};
CaviarFixedPointProjectionMatrix CaviarFixedPointProjection={};
float *CaviarProjectionScaledMatrix=0;
int32_t vox_projection_render_stage_=9, vox_projection_prepared_=7;
const float CaviarProjectionDirectionScale=256.0f;
const float CaviarProjectionRoundThreshold=0.0f;
const float CaviarProjectionRoundBelow=-0.5f, CaviarProjectionRoundAbove=0.5f;
'''

MAIN = r'''
static void *low_map(size_t n) {
  void *p=mmap(0,n,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT,-1,0);
  if(p==MAP_FAILED || (uintptr_t)p>0x7fffffffU) return 0; return p;
}
static void ck(bool b,const char *s){if(!b){fprintf(stderr,"FAIL %s\n",s);exit(1);}}
int main(){
  float *mem=(float*)low_map(4096); ck(mem!=0,"low mapping");
  CaviarProjectionScaledMatrix=mem+16;
  const float in[9]={.10f,-.10f,0,.25f,-.25f,.5f,1,-1,2};
  for(int i=0;i<9;++i)mem[i]=in[i];
  sub_63fa80(mem);
  ck(CaviarFixedPointProjection.word_00_==26 && CaviarFixedPointProjection.word_02_==-26,
     "scaled rounded words");
  ck(CaviarFixedPointProjection.word_06_==64 && CaviarFixedPointProjection.word_08_==-64 &&
     CaviarFixedPointProjection.dword_0C_==256 && CaviarFixedPointProjection.dword_14_==512,
     "short/dword destinations");
  ck(vox_projection_render_stage_==0 && vox_projection_prepared_==0,"stage clearing");
  CaviarFixedPointProjection.word_00_=256; CaviarFixedPointProjection.word_02_=0;
  CaviarFixedPointProjection.word_04_=0; CaviarFixedPointProjection.word_06_=0;
  CaviarFixedPointProjection.word_08_=256; CaviarFixedPointProjection.word_0A_=0;
  short zero_centre[3]={256,512,768}, zero_extent[3]={0,0,0}; int zero_rect[6]={};
  sub_6401b0(zero_centre,zero_extent,zero_rect);
  ck(zero_rect[0]==254&&zero_rect[1]==510&&zero_rect[2]==258&&zero_rect[3]==514&&
     zero_rect[4]==5&&zero_rect[5]==5,"identity projection with zero extent");
  CaviarFixedPointProjection.word_00_=127; CaviarFixedPointProjection.word_02_=0;
  CaviarFixedPointProjection.word_04_=0; CaviarFixedPointProjection.word_06_=0;
  CaviarFixedPointProjection.word_08_=0; CaviarFixedPointProjection.word_0A_=0;
  short bias_centre[3]={1,0,0}, bias_extent[3]={0,0,0}; int bias_rect[6]={};
  ck(caviar_sar8_bits(0x7fu)==0,"+0x80/SAR boundary bit pattern");
  ck(caviar_sar8_bits(0xffffff00u)==0xffffffffu,"negative SAR sign fill");
  sub_6401b0(bias_centre,bias_extent,bias_rect);
  ck(bias_rect[0]==-2&&bias_rect[2]==2&&bias_rect[4]==5,
     "bias boundary through center projection");
  CaviarFixedPointProjection.word_00_=32767; CaviarFixedPointProjection.word_02_=0;
  CaviarFixedPointProjection.word_04_=0; CaviarFixedPointProjection.word_06_=0;
  CaviarFixedPointProjection.word_08_=0; CaviarFixedPointProjection.word_0A_=0;
  short low16_centre[3]={0,0,0}, low16_extent[3]={32767,0,0}; int low16_rect[6]={};
  ck(fixed8((uint32_t)(32767*32767))==-256,
     "extent full SAR then original AX low16 sign extension");
  sub_6401b0(low16_centre,low16_extent,low16_rect);
  ck(low16_rect[0]==-258&&low16_rect[1]==-2&&low16_rect[2]==2&&
     low16_rect[3]==2&&low16_rect[4]==261&&low16_rect[5]==5,
     "nonzero extent low16 end-to-end projection");
  CaviarFixedPointProjection.word_00_=256; CaviarFixedPointProjection.word_02_=-256;
  CaviarFixedPointProjection.word_04_=128; CaviarFixedPointProjection.word_06_=-128;
  CaviarFixedPointProjection.word_08_=256; CaviarFixedPointProjection.word_0A_=256;
  short centre[3]={300,200,150}, extent[3]={10,-20,30}; int rect[6]={};
  sub_6401b0(centre,extent,rect);
  ck(rect[0]==173&&rect[1]==173&&rect[2]==222&&rect[3]==232&&rect[4]==50&&rect[5]==60,
     "independent eight-corner projection oracle");
  unsigned short *dst16=(unsigned short*)low_map(4096); unsigned char *dst8=(unsigned char*)low_map(4096);
  unsigned char *shade=(unsigned char*)low_map(4096); ck(dst16&&dst8&&shade,"pixel mappings");
  char *src=(char*)low_map(4096); ck(src!=0,"low source mapping");
  const char source_bytes[24]={1,2,77,0,5,99,11,0,13,14,15,99,21,22,23,24,25,99,31,0,33,34,35,99};
  for(int i=0;i<24;++i)src[i]=source_bytes[i];
  unsigned short pal[256]; for(int i=0;i<256;++i){pal[i]=(unsigned short)(1000+i);shade[i]=(unsigned char)(i^0x5a);}
  for(int i=0;i<24;++i) dst16[i]=(unsigned short)(10+i);
  sub_63b11c(dst16,src,0,0,4,3,0x18000,0x18000,6,8,pal);
  ck(dst16[0]==1010&&dst16[1]==1011&&dst16[2]==12&&dst16[3]==1013&&
     dst16[8]==1018&&dst16[9]==19&&dst16[10]==1020&&dst16[11]==1021&&
     dst16[16]==1026&&dst16[17]==27&&dst16[18]==1028&&dst16[19]==1029,
     "16-bit independent pitches/fractional carry/source gate");
  ck(dst16[4]==14&&dst16[12]==22&&dst16[20]==30&&
     (uintptr_t)CaviarBlitState.destination_cursor_==(uintptr_t)(dst16+24),
     "16-bit padded columns/final cursor");
  for(int i=0;i<24;++i) dst8[i]=(unsigned char)(50+i);
  sub_63b1bf(dst8,src,0,0,4,3,0x18000,6,8,0);
  ck(dst8[0]==1&&dst8[1]==2&&dst8[2]==52&&dst8[3]==5&&dst8[8]==11&&dst8[9]==59&&
     dst8[10]==14&&dst8[11]==15&&dst8[16]==31&&dst8[17]==67&&dst8[18]==34&&dst8[19]==35,
     "transparent walker fractional carry/pitches");
  ck(dst8[4]==54&&dst8[12]==62&&dst8[20]==70&&
     (uintptr_t)CaviarBlitState.destination_cursor_==(uintptr_t)(dst8+24),
     "transparent padded columns/final cursor");
  for(int i=0;i<24;++i)dst8[i]=(unsigned char)(60+i);
  sub_63b241(dst8,src,0,0,4,3,0x18000,0x18000,6,8,shade);
  ck(dst8[0]==shade[60]&&dst8[1]==shade[61]&&dst8[2]==62&&dst8[3]==shade[63]&&
     dst8[8]==shade[68]&&dst8[9]==69&&dst8[10]==shade[70]&&dst8[11]==shade[71]&&
     dst8[16]==shade[76]&&dst8[17]==77&&dst8[18]==shade[78]&&dst8[19]==shade[79],
     "shade walker raw destination-index lookup");
  ck(dst8[4]==64&&dst8[12]==72&&dst8[20]==80&&
     (uintptr_t)CaviarBlitState.destination_cursor_==(uintptr_t)(dst8+24),
     "shade padded columns/final cursor");
  return 0;
}
'''

DEFS = (
    ("int round_view_scaled(", "helper"),
    ("static int32_t caviar_i32_from_bits(", "helper"),
    ("static uint32_t caviar_sar8_bits(", "helper"),
    ("static int32_t caviar_fixed8_low16(", "helper"),
    ("static int32_t caviar_add_i32(", "helper"),
    ("static int32_t caviar_sub_i32(", "helper"),
    ("int32_t fixed8(uint32_t value_bits)", "helper"),
    ("static uint32_t caviar_blit_offset(", "helper"),
    ("static unsigned char *caviar_blit_address(", "helper"),
    ("static const char *caviar_blit_source_address(", "helper"),
    ("void __cdecl sub_63fa80(", "body"), ("void __cdecl sub_6401b0(", "body"),
    ("void __cdecl sub_63b11c(", "body"), ("void __cdecl sub_63b1bf(", "body"),
    ("void __cdecl sub_63b241(", "body"),
)

def compile_run(compiler, definitions, expected):
    with tempfile.TemporaryDirectory() as d:
        cpp=Path(d)/"actual.cpp"; exe=Path(d)/"actual"
        cpp.write_text(HARNESS+"\n"+"\n".join(definitions)+"\n"+MAIN)
        built=subprocess.run([compiler,"-std=c++98","-O0",str(cpp),"-o",str(exe)],
                             stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
        assert built.returncode==0, built.stdout
        result=subprocess.run([str(exe)],stdout=subprocess.PIPE,stderr=subprocess.STDOUT)
        assert (result.returncode==0)==expected, result.stdout.decode(errors="replace")

def test_actual_caviar_projection_and_blitters_gcc_clang():
    source=PRODUCT.read_text(); defs={sig:extract(source,sig) for sig,_ in DEFS}
    compilers=[shutil.which("g++"),shutil.which("clang++")]
    assert all(compilers),"GCC and Clang required"
    ordered=[defs[sig] for sig,_ in DEFS]
    for compiler in compilers: compile_run(compiler,ordered,True)

def test_extract_masks_whole_translation_unit_comments_and_literals():
    signature = "void __cdecl sub_63fa80("
    body = signature + "float *input) { (void)input; }"
    fake = "/* " + body + " */\nconst char *hint = \"" + body + "\";\n"
    assert extract(fake + body, signature) == body
    with pytest.raises(AssertionError):
        extract(fake, signature)

def test_moved_records_and_bodies_retain_pinned_source_history():
    baseline="7db715fbc"
    old=subprocess.check_output(
        ["git","show",baseline+":src/leaf_recoveries.cpp"],cwd=ROOT,text=True)
    current=PRODUCT.read_text()
    owner_history=subprocess.check_output(
        ["git","show","2fa8557f4:src/caviar.cpp"],cwd=ROOT,text=True)
    old_rows={row.address:row for row in reader.read_text(old,"src/leaf_recoveries.cpp")}
    current_rows={row.address:row for row in reader.read_text(current,PRODUCT)}
    addresses=(0x63FA80,0x6401B0,0x63B11C,0x63B1BF,0x63B241)
    fields=tuple(field.name for field in dataclasses.fields(next(iter(old_rows.values())))
                 if field.name not in {"mode","path","line","region","recipe",
                                       "extract_error","origin","source_role_reason"})
    unchanged_bodies = {0x63FA80}
    for address in addresses:
        assert address in old_rows and address in current_rows,hex(address)
        before,after=old_rows[address],current_rows[address]
        assert all(getattr(before,name)==getattr(after,name) for name in fields),hex(address)
        signature="void __cdecl "+before.name+"("
        if address in unchanged_bodies:
            assert extract(old,signature)==extract(current,signature),hex(address)
        else:
            def envelope(text):
                marker=f"// ORIGINAL: 0x{address:08X}"
                marker_at=text.index(marker)
                start=text.rfind("/*",0,marker_at)
                end=text.index("*/",marker_at)+2
                lines=text[start:end].splitlines()
                # Helpers are outside the original comment envelope. Retain
                # all original comment text except these two documented edits.
                block="\n".join(line for line in lines
                                 if not line.startswith("// RECOVERY_ROLE:"))
                if address == 0x63B241:
                    old_phrase="with table[destination byte + table base]."
                    new_phrase="with table[destination byte]."
                    assert block.count(new_phrase) + block.count(old_phrase) == 1
                    block=block.replace(new_phrase,old_phrase,1)
                return block
            assert envelope(owner_history)==envelope(current),hex(address)

def test_actual_body_mutants_are_rejected_by_bounded_oracle():
    source=PRODUCT.read_text(); defs={sig:extract(source,sig) for sig,_ in DEFS}
    mutations=(
      ("void __cdecl sub_63b11c(","if (source[carried_x >> 16] != 0)","if (source[carried_x >> 16] == 0)"),
      ("void __cdecl sub_63b1bf(","if (value != transparent)","if (value == transparent)"),
      ("void __cdecl sub_63b241(","*pixel = table[*pixel];",
       "*pixel = table[(unsigned char)(*pixel + 1)];"),
      ("static uint32_t caviar_sar8_bits(","bits + 0x80u","bits + 0x81u"),
      ("static uint32_t caviar_sar8_bits(","shifted |= 0xff000000u;",
       "shifted |= 0x00000000u;"),
      ("static int32_t caviar_fixed8_low16(",
       "const uint16_t low = (uint16_t)caviar_sar8_bits(bits);",
       "const uint16_t low = 0;"),
      ("void __cdecl sub_63b11c(","source_rows * static_cast<uint32_t>(source_stride)","source_rows * static_cast<uint32_t>(destination_stride)"),
      ("void __cdecl sub_63b11c(","carried_x += static_cast<unsigned int>(source_x_step);","carried_x += 0x10000U;"),
      ("void __cdecl sub_63b1bf(","carried = (carried + step) & 0xffff;","carried = 0;"),
      ("void __cdecl sub_6401b0(",
       "x_walk = caviar_sub_i32(x_walk, extent0_x);",
       "x_walk = caviar_add_i32(x_walk, extent0_x);"),
    )
    compilers=(shutil.which("g++"),shutil.which("clang++"))
    assert all(compilers),"GCC and Clang required"
    for signature,before,after in mutations:
        mutant=dict(defs); assert before in mutant[signature],before
        mutant[signature]=mutant[signature].replace(before,after,1)
        ordered=[mutant[sig] for sig,_ in DEFS]
        for compiler in compilers: compile_run(compiler,ordered,False)
