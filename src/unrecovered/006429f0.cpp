// ORIGINAL: 0x006429F0 FILE
// RULED-OUT: catalogued as nullary returning int; the disassembly reads a
//            real stack argument at [esp+4], so re-signatured as one
//            `int param_1` parameter, `void` return (see also
//            0x0063D450). Tried: caching the byte-write index in a local
//            (`int idx = *(int*)(param_1+0x10)+1;`, matching Ghidra's
//            iVar1 literally) - worse (119B); no caching at all, every
//            field re-read fresh through nested `*(int*)(...)` each time -
//            best so far (121B vs 124); caching the buffer base pointer
//            once as `unsigned char *buf` - worse (116B, divergence moves
//            earlier); caching `count = *(int*)(param_1+0x16b4)` once and
//            reusing it in both the `8 <` and `0 <` guards, matching the
//            original's single register load - no change; two named
//            `unsigned char *dst1/dst2` pointers instead of inline
//            expressions for the two-byte write in the `>8` branch - no
//            change. Every variant is missing one `push`/`pop` (esi) that
//            the original takes at function entry and shares across both
//            branches; my `>8` branch never seems to need a register the
//            original apparently does for its second (reloaded) base
//            pointer, and no source-level restructuring of that branch
//            moved the needle.
// working copy - scaffold materialised by --work
// name      sub_6429f0
// size      124 bytes
// spans     0x006429F0-0x00642A6C
// prototype 
// callers   2   call targets   0
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6429f0  at 0x006429F0  (124 bytes)
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
extern "C" void __cdecl sub_6429f0(int param_1) {
    int count = *(int *)(param_1 + 0x16b4);
    if (8 < count) {
        *(unsigned char *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x10)) =
            *(unsigned char *)(param_1 + 0x16b0);
        *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 1;
        *(unsigned char *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x10)) =
            *(unsigned char *)(param_1 + 0x16b1);
        *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 1;
        *(unsigned short *)(param_1 + 0x16b0) = 0;
        *(int *)(param_1 + 0x16b4) = 0;
        return;
    }
    if (0 < count) {
        *(unsigned char *)(*(int *)(param_1 + 8) + *(int *)(param_1 + 0x10)) =
            *(unsigned char *)(param_1 + 0x16b0);
        *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 1;
    }
    *(unsigned short *)(param_1 + 0x16b0) = 0;
    *(int *)(param_1 + 0x16b4) = 0;
}
