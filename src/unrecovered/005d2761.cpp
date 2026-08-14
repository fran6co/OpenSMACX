// ORIGINAL: 0x005D2761 FILE
// RULED-OUT: original prologue saves 6 registers (ebx,ecx,edx,esi,edi,ebp);
//            plain __cdecl(uint*,uint*,int) body only pressures 1-2, so the
//            reservation never reproduces. Explicit per-register temps for
//            each S-box term didn't move it either.
// working copy - scaffold materialised by --work
// name      sub_5d2761
// size      211 bytes
// spans     0x005D2761-0x005D2834
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2761  at 0x005D2761  (211 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
// `char`, NOT `signed char`. They are distinct MSVC types and mangle
// differently - D against C - and the catalogue's `int8` means the first:
// counted over every catalogued mangled name, `PAD` appears 508 times and
// `PAC` once. Spelling it `signed char` made 150 derived prototypes emit a
// symbol no target object holds. `int8_t` keeps its C meaning below; neither
// catalogue ever uses it.
typedef char int8;
typedef unsigned char uint8;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
extern unsigned int tbl_6f2c[];
extern unsigned int tbl_772c[];
extern unsigned int tbl_7f2c[];
extern unsigned int tbl_872c[];
extern unsigned int tbl_8bf4[];
extern unsigned int tbl_900c[];

extern "C" int __cdecl sub_5d2761(unsigned int *src, unsigned int *dst, int count) {
    do {
        unsigned int w = *src;
        unsigned int sum1 = tbl_772c[(w & 0xff00) >> 8] + tbl_6f2c[w >> 24] + tbl_7f2c[w & 0xff];
        unsigned int sum2 = tbl_6f2c[((unsigned char *)src)[2]] + tbl_7f2c[w & 0xff] + tbl_772c[(w & 0xff00) >> 8];
        count = count - 1;
        *dst = ((tbl_872c[sum2 >> 0x17] + tbl_900c[sum2 & 0x1ff] + tbl_8bf4[(sum2 & 0xff800) >> 0xb]) & 0xffff0000)
             | ((tbl_872c[sum1 >> 0x17] + tbl_8bf4[(sum1 & 0xff800) >> 0xb] + tbl_900c[sum1 & 0x1ff]) & 0xffff);
        dst++;
        src++;
    } while (count != 0);
    return 0;
}
