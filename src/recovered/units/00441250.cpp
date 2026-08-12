// ORIGINAL: 0x00441250 FILE
// name      ?clear_all_offers@@YAXXZ
// size      70 bytes
// spans     0x00441250-0x00441296
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00441250
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00441250/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?clear_all_offers@@YAXXZ  at 0x00441250  (70 bytes)
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
static int *const g_007388dc = (int *)0x007388DC;
static int *const g_0073acdc = (int *)0x0073ACDC;
// Clears a flat 64-element table (8x8, 0x90 bytes/element) at g_offers:
// `flat` walks it linearly (row-major, i*8+j) while `col` walks the same
// table transposed (j*8+i), so every element is touched twice per full
// pass - once as table[i][j], once as table[j][i]. Four ints at relative
// offsets -4,0,+4,+8 are zeroed per touch.
extern char g_offers[];
extern char g_offers_end[];

void __cdecl clear_all_offers() {
    char *flat = g_offers;
    char *row_base = flat;
    do {
        char *col = row_base;
        int n = 8;
        do {
            *reinterpret_cast<int *>(flat - 4) = 0;
            *reinterpret_cast<int *>(flat)     = 0;
            *reinterpret_cast<int *>(col - 4)  = 0;
            *reinterpret_cast<int *>(col)      = 0;
            *reinterpret_cast<int *>(flat + 4) = 0;
            *reinterpret_cast<int *>(flat + 8) = 0;
            *reinterpret_cast<int *>(col + 4)  = 0;
            *reinterpret_cast<int *>(col + 8)  = 0;
            flat += 0x90;
            col += 0x480;
            n = n - 1;
        } while (n != 0);
        row_base += 0x90;
    } while (reinterpret_cast<int>(flat) < reinterpret_cast<int>(g_offers_end));
}
