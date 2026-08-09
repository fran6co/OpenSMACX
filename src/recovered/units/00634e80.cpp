// ORIGINAL: 0x00634E80 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00634E80
// name           ?set_def_ext@FileBox@@QAEHPAD@Z
// size           76 bytes
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00634e80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?set_def_ext@FileBox@@QAEHPAD@Z  at 0x00634E80  (76 bytes)
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

class FileBox;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" char *strcat(char *, const char *);
extern "C" unsigned int strlen(const char *);

class FileBox { public:
    int set_def_ext(char *);
};

// NOT BYTE_EXACT. Closest reached: 67/76 bytes, mnemonic similarity 0.63
// (verify_recovered_function.py --json). Logic matches the Ghidra
// decompile exactly (confirmed against the disassembly) - the remaining
// divergence is entirely in TWO backend decisions that several distinct
// source-form rewrites (sequential guards, negated/nested guards,
// deferring the `this`-cast until first use, forcing `self` before or
// after the a1==0 check) all reproduced identically, which reads as a
// scheduling/tail-merge wall rather than a wrong body:
//
//   1. Prologue interleaving: original is `push esi; mov esi,[esp+8];
//      push edi; mov edi,ecx` (save-then-load, alternating registers).
//      Every rewrite tried here compiles to `push esi; push edi; mov
//      esi,[esp+8]; mov edi,ecx` (both saves, then both loads).
//   2. Tail merging: original has the a1==0 path
//      (`mov byte[edi+0x414],0; pop edi; xor eax,eax; pop esi; ret4`)
//      and the post-strcat success path (`xor eax,eax; pop edi; pop
//      esi; ret4`) as two DISTINCT instruction sequences - same result,
//      different instruction order, not byte-identical, so the real
//      compiler never folded them. This environment's compiler
//      recognizes both as "return 0" and folds them into one shared
//      tail reached by `jmp`, which the original does not do.
//
// Ruled out: negating/nesting the a1!=0 guard instead of two sequential
// ifs (same 67 bytes, and flips jne->je - worse); computing `self` only
// where first needed per-branch instead of once up front (identical
// result - the scheduling choice is not driven by source order here).
int FileBox::set_def_ext(char * a1) {
    char *self = reinterpret_cast<char *>(this);
    if (a1 == 0) {
        self[0x414] = 0;
        return 0;
    }
    if (strlen(a1) > 3) {
        return 3;
    }
    self[0x414] = 0;
    strcat(self + 0x414, a1);
    return 0;
}

