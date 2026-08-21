// ORIGINAL: 0x0063D450 _flush_pending 0x0063D450-0x0063D4C2 FILE BYTE_EXACT
// symbol    _sub_63d450
// LEVER: `rep movsd`/`rep movsb` is VC6's /Oi expansion of memcpy(dst, src, n) for a runtime-determined n, not a hand-rolled loop (matches the strcpy lesson already in src/recovered/00639390.cpp). Catalogued as nullary returning int; the disassembly reads a real stack argument at [esp+4] and never sets eax on purpose, so it is a one-`int`-parameter `void` function instead. Min-of-two-sizes has to be spelled `if (count > mySize) count = mySize;` (condition polarity `>`, not `<` on the flipped operands) to get the original's `jbe`; the algebraically-equivalent `if (mySize < count)` compiles to `jae`, and a `cond ? a : b` ternary compiles to `ja` - same logic, three different branch mnemonics.
// CORRECTED from sub_63d450
//   zlib 1.0.2 deflate.c, 114 bytes, byte-exact from upstream
// size      114 bytes
// prototype 
// callers   4   call targets   0
// kind      
// flags     
// calls     (none)
// working copy - scaffold materialised by --work

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63d450  at 0x0063D450  (114 bytes)
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
typedef unsigned int size_t;
extern "C" void *memcpy(void *, const void *, size_t);

extern "C" void __cdecl sub_63d450(int param_1) {
    unsigned int count = *(unsigned int *)(*(int *)(param_1 + 0x1c) + 0x10);
    if (count > *(unsigned int *)(param_1 + 0x10)) {
        count = *(unsigned int *)(param_1 + 0x10);
    }
    if (count != 0) {
        memcpy(*(void **)(param_1 + 0xc), *(void **)(*(int *)(param_1 + 0x1c) + 0xc), count);
        *(unsigned int *)(param_1 + 0xc) = *(unsigned int *)(param_1 + 0xc) + count;
        *(unsigned int *)(*(int *)(param_1 + 0x1c) + 0xc) =
            *(unsigned int *)(*(int *)(param_1 + 0x1c) + 0xc) + count;
        *(unsigned int *)(param_1 + 0x14) = *(unsigned int *)(param_1 + 0x14) + count;
        *(unsigned int *)(param_1 + 0x10) = *(unsigned int *)(param_1 + 0x10) - count;
        *(unsigned int *)(*(int *)(param_1 + 0x1c) + 0x10) =
            *(unsigned int *)(*(int *)(param_1 + 0x1c) + 0x10) - count;
        int other = *(int *)(param_1 + 0x1c);
        if (*(int *)(other + 0x10) == 0) {
            *(unsigned int *)(other + 0xc) = *(unsigned int *)(other + 8);
        }
    }
}
