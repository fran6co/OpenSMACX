// ORIGINAL: 0x005D2D20 sub_5d2d20 0x005D2D20-0x005D2DF2 FILE
// TRIED: `extern "C" int __stdcall sub_5d2d20(int a1, int a2, int a3)` per the given def head - `test edx,edx` and `lea eax,[ecx+7]` read both registers with no matching stack loads, so two args (src ptr in ecx, mode in edx) are __fastcall register params, not part of a 3-int __stdcall; changed to `void __fastcall(char*,int,int,char*,char)` with 5 total params (2 register + 3 stack), void return (eax never set). `int step = stride * 4;` hoisted above the `if` (matching the unconditional `lea edi,[eax*4]` before the branch) gets close; best found is 83.6% mnemonic similarity (`--dir` picked /O1 framed over /O2 frameless) - remaining diff is a 4-instruction reorder of which register (edx vs ecx) holds `dst` while `adjust`/`i` are set up, which is register-allocation scheduling, not a control-flow gap.
// working copy - scaffold materialised by --work
// size      210 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5d2d20  at 0x005D2D20  (210 bytes)
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
// `src` (ecx) and `mode` (edx) are __fastcall register args; `stride`,
// `dst` and `adjust` are the three stack args the `ret 0xc` pops - see
// TRIED above for the signature evidence.
extern "C" void __fastcall sub_5d2d20(char *src, int mode, int stride,
                                      char *dst, char adjust) {
    int step = stride * 4;
    char *p;
    char *q;
    int i;
    if (mode != 0) {
        p = src + 7;
        q = dst + 6;
        i = 8;
        do {
            q[-4] = p[-5] + adjust;
            q[0] = p[0] + adjust;
            q[4] = p[-1] + adjust;
            q[8] = p[4] + adjust;
            q[0xc] = p[3] + adjust;
            q[0x10] = p[8] + adjust;
            q[0x14] = p[7] + adjust;
            q[0x18] = p[0xc] + adjust;
            p = p + step;
            --i;
            q = q + 0x40;
        } while (i != 0);
        return;
    }
    p = src + 2;
    q = dst + 6;
    i = 8;
    do {
        q[-4] = p[1] + adjust;
        q[0] = p[0] + adjust;
        q[4] = p[5] + adjust;
        q[8] = p[4] + adjust;
        q[0xc] = p[9] + adjust;
        q[0x10] = p[8] + adjust;
        q[0x14] = p[0xd] + adjust;
        q[0x18] = p[0xc] + adjust;
        p = p + step;
        --i;
        q = q + 0x40;
    } while (i != 0);
}
