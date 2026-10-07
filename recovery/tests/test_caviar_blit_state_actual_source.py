"""Installable controls for the Caviar blit-state owner migration.

After proposal.patch is integrated this test reads only src/caviar.cpp. It
compiles the actual helper/blitter definitions extracted from that file; the
host fixture substitutes a pointer-width state type and makes no product
layout claim.
"""
from __future__ import annotations

from pathlib import Path
import json
import shutil
import subprocess
import tempfile

import pytest


ROOT = next(path for path in Path(__file__).resolve().parents
            if (path / "src/caviar.cpp").is_file())
PRODUCT = ROOT / "src/caviar.cpp"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def extract(source: str, signature: str) -> str:
    from match_recovery.includes import COMMENTS_OR_STRINGS

    masked = COMMENTS_OR_STRINGS.sub(
        lambda match: "".join("\n" if char == "\n" else " " for char in match[0]),
        source)
    start = masked.find(signature)
    while start >= 0:
        opening = masked.find("{", start)
        semicolon = masked.find(";", start)
        if opening >= 0 and (semicolon < 0 or opening < semicolon):
            break
        start = masked.find(signature, start + len(signature))
    assert start >= 0 and opening >= 0, "missing actual source definition: " + signature
    depth = 0
    for index in range(opening, len(masked)):
        if masked[index] == "{":
            depth += 1
        elif masked[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unclosed actual source body: " + signature)


HOST_TYPES = r'''#include <stdint.h>
#include <stddef.h>
#include <limits.h>
#include <sys/mman.h>
#include <assert.h>
#define __cdecl
// Host-width fixture only. The real source has x86 sizeof/offsetof asserts.
struct CaviarBlitStateCandidate {
    unsigned char * volatile destination_cursor_;
    uint32_t source_row_bytes_;
    uint32_t destination_stride_bytes_;
};
CaviarBlitStateCandidate CaviarBlitState = {0, 0, 0};
static void *low_map(size_t n) {
    void *p=mmap(0,n,PROT_READ|PROT_WRITE,
                 MAP_PRIVATE|MAP_ANONYMOUS|MAP_32BIT,-1,0);
    if(p==MAP_FAILED || (uintptr_t)p > 0x7fffffffU) return 0;
    return p;
}
'''


HOST_MAIN = r'''
int main() {
    unsigned char *dst8=(unsigned char*)low_map(4096);
    unsigned short *dst16=(unsigned short*)low_map(4096);
    unsigned short *source16=(unsigned short*)low_map(4096);
    unsigned char *shade=(unsigned char*)low_map(4096);
    char *source=(char*)low_map(262144);
    assert(dst8 && dst16 && source16 && shade && source);

    const uint32_t expected_offset=(uint32_t)INT_MAX*(uint32_t)INT_MIN+
                                   (uint32_t)INT_MAX;
    assert(caviar_blit_offset(INT_MAX,INT_MAX,INT_MIN)==expected_offset);
    const uint32_t low=(uint32_t)(uintptr_t)dst8;
    assert((uint32_t)(uintptr_t)caviar_blit_address(dst8,0xffffffffU)==
           low+0xffffffffU);

    const char input[24]={1,2,77,0,5,99,11,0,13,14,15,99,
                          21,22,23,24,25,99,31,0,33,34,35,99};
    for(int i=0;i<24;++i) source[i]=input[i];
    for(int i=0;i<32;++i) {
        dst16[i]=(unsigned short)(10+i);
        dst8[i]=(unsigned char)(40+i);
        shade[i]=(unsigned char)(i^0x5a);
    }
    unsigned short palette[65536];
    for(unsigned i=0;i<65536;++i) palette[i]=(unsigned short)(i+1000);

    sub_63b11c(dst16,source,0,0,4,3,0x18000,0x18000,6,8,palette);
    assert(dst16[0]==1010 && dst16[1]==1011 && dst16[2]==12 && dst16[3]==1013);
    assert(dst16[8]==1018 && dst16[9]==19 && dst16[10]==1020 && dst16[11]==1021);
    assert(dst16[16]==1026 && dst16[17]==27 && dst16[18]==1028 && dst16[19]==1029);
    assert(dst16[4]==14 && dst16[12]==22 && dst16[20]==30);
    assert((uintptr_t)CaviarBlitState.destination_cursor_==(uintptr_t)(dst16+24));
    assert(CaviarBlitState.destination_stride_bytes_==16);

    source[0]=7;
    sub_63b1bf(dst8+8,source,0,1,1,2,0x10000,0,-4,0);
    assert(dst8[4]==7 && dst8[0]==7);
    assert((uintptr_t)CaviarBlitState.destination_cursor_==(uintptr_t)dst8-4);

    // Negative 16.16 x step wraps as an unsigned accumulator: the second
    // pixel reads source[65535], with no checked/clipped fallback.
    dst8[0]=dst8[1]=0; source[0]=7; source[65535]=9;
    sub_63b1bf(dst8,source,0,0,2,1,-0x10000,0,0,0);
    assert(dst8[0]==7 && dst8[1]==9);
    assert((uintptr_t)CaviarBlitState.destination_cursor_==(uintptr_t)dst8);

    // Negative y step with negative source stride retains 32-bit x86 wrap:
    // 65535 rows times -1 advances the source by -65535 bytes. Both rows
    // remain inside this deliberately large host mapping.
    for(int i=0;i<8;++i) dst8[i]=0;
    for(int i=0;i<32;++i) shade[i]=(unsigned char)(i^0x5a);
    source[65536]=1; source[1]=1;
    sub_63b241(dst8,source+65536,0,0,1,2,0x10000,-0x10000,-1,2,
               shade);
    assert(dst8[0]==shade[0] && dst8[2]==shade[0]);
    assert((uintptr_t)CaviarBlitState.destination_cursor_==(uintptr_t)(dst8+4));

    for(int i=0;i<8;++i) { dst8[i]=(unsigned char)(40+i); source[i]=(char)(i?1:0); }
    sub_63b241(dst8,source,0,0,4,1,0x10000,0x10000,4,8,
               shade);
    assert(dst8[0]==40 && dst8[1]==shade[41] && dst8[2]==shade[42]);

    for(int i=0;i<16;++i) { dst16[i]=0; source16[i]=(unsigned short)(i+1); }
    source16[1]=0;
    vox_blit_transparent(reinterpret_cast<unsigned char *>(dst16),
                         source16,0,0,2,2,0x10000,2,4,0);
    assert(dst16[0]==1 && dst16[1]==0 && dst16[4]==3 && dst16[5]==4);
    assert(CaviarBlitState.source_row_bytes_==4);
    assert(CaviarBlitState.destination_stride_bytes_==8);
    assert((uintptr_t)CaviarBlitState.destination_cursor_==(uintptr_t)(dst16+8));
    return 0;
}
'''


def test_original_data_origins_and_existing_owner_roles_survive():
    from match_recovery.object_source import parse_objects
    from match_recovery.source import read_file

    source = PRODUCT.read_text()
    project = json.loads((ROOT / "recovery/project.json").read_text())
    pin_path = ROOT / project["source"]["image_pin"]
    configured_pin = next(
        line.split()[1] for line in pin_path.read_text().splitlines()
        if line.startswith("sha256 "))
    assert configured_pin == PIN
    assert "sizeof(CaviarBlitStateCandidate) == 12" in source
    assert "offsetof(CaviarBlitStateCandidate, destination_cursor_) == 0" in source
    assert "offsetof(CaviarBlitStateCandidate, source_row_bytes_) == 4" in source
    assert "offsetof(CaviarBlitStateCandidate, destination_stride_bytes_) == 8" in source
    functions = {row.address: row for row in read_file(PRODUCT)}
    for address in (0x63FA80, 0x6401B0, 0x63B11C, 0x63B1BF, 0x63B241):
        row = functions[address]
        assert row.source_role == ("unclassified" if address in (0x63FA80, 0x6401B0)
                                   else "scaffold")
        assert row.image_spans and row.image_spans[0][0] == address
        assert not row.byte_exact and not row.semantic

    bindings = parse_objects(source, "src/caviar.cpp", PIN, address_space="pe:va")
    by_start = {row.origin.address: row for row in bindings}
    children = {
        0x00698B70: ("?vox_blit_cursor@@3IA", ("destination_cursor_",), 0x0063B11C),
        0x00698B74: ("?vox_blit_row_bytes@@3IA", ("source_row_bytes_",), 0x0063B072),
        0x00698B78: ("?vox_blit_dst_stride@@3IA", ("destination_stride_bytes_",), 0x0063B11C),
    }
    for address, (symbol, member, evidence) in children.items():
        row = by_start[address]
        assert row.symbol == symbol
        assert row.span.start == address and row.span.end == address + 4
        assert row.parent_symbol
        assert row.parent_symbol == by_start[0x00698B70].parent_symbol
        assert row.member_path == member
        assert row.evidence.address == evidence


def test_actual_shared_blit_definitions_match_finite_and_wrap_controls():
    source = PRODUCT.read_text()
    signatures = (
        "static uint32_t caviar_blit_offset(",
        "static unsigned char *caviar_blit_address(",
        "static const char *caviar_blit_source_address(",
        "void __cdecl vox_blit_transparent(",
        "void __cdecl sub_63b11c(",
        "void __cdecl sub_63b1bf(",
        "void __cdecl sub_63b241(",
    )
    definitions="\n".join(extract(source, signature) for signature in signatures)
    compilers=[shutil.which("g++"),shutil.which("clang++")]
    assert all(compilers), "GCC and Clang are required for these bounded controls"
    with tempfile.TemporaryDirectory(prefix="osmx-caviar-blit-actual-source-") as tmp:
        cpp=Path(tmp)/"actual_source.cpp"
        executable=Path(tmp)/"control"
        cpp.write_text(HOST_TYPES+definitions+HOST_MAIN)
        for compiler in compilers:
            built=subprocess.run([compiler,"-std=c++98","-O0",str(cpp),"-o",str(executable)],
                                 capture_output=True,text=True)
            assert built.returncode==0,built.stdout+built.stderr
            ran=subprocess.run([str(executable)],capture_output=True,text=True)
            assert ran.returncode==0,ran.stdout+ran.stderr
