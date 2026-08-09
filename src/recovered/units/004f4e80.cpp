// ORIGINAL: 0x004F4E80 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004F4E80
// name           ?base_research@@YAXXZ
// size           299 bytes
// measured tier  MISMATCH
// divergence     6
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004f4e80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_research@@YAXXZ  at 0x004F4E80  (299 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl bitmask(int, int *, int *);
void __cdecl tech_research(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689370 = (int *)0x00689370;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64d4 = (int *)0x009A64D4;

// Indexed table bases: each address's own multiply-by-register work is done
// by the register (uVar4 * 0x20CC / 0x833, or iVar3 * 0x134), so these need
// extern arrays, not pointer constants - the emitter left them unscaffolded
// since they never appear as a bare immediate, only as [reg+ADDR].
struct SomeObj {
    int field_0;
    uint8_t field_4;
    char pad_5[0x108 - 5];
    int field_108;
};

extern uint8_t g_bitmask_table[];
extern int g_field_cc54[];
extern int g_field_c9e8[];
extern int g_field_d1e4[];
extern int g_field_cdb8[];

void __cdecl base_research() {
    if ((*g_009a649c & 0x4000000) == 0) {
        SomeObj *obj = *reinterpret_cast<SomeObj **>(g_0090ea30);
        int uVar4 = obj->field_4;
        int iVar3 = *g_00689370;
        int iVar1 = obj->field_108;

        int idx, mask;
        bitmask(0x17, &idx, &mask);
        int hasFlag = g_bitmask_table[iVar3 * 0x134 + idx] & static_cast<uint8_t>(mask);
        if (hasFlag != 0) {
            iVar1 = iVar1 / 2;
        }

        int year = *g_009a64d4;
        if (year < g_field_cc54[uVar4 * 0x833] * -5) {
            iVar1 = 0;
        }
        if (year < 5 && g_field_c9e8[uVar4 * 0x833] < 2) {
            iVar1 = 0;
        }
        g_field_d1e4[uVar4 * 0x833] += iVar1;

        int iVar2 = g_field_cc54[uVar4 * 0x833];
        if (iVar2 < -5) {
            iVar2 = -5;
        } else if (iVar2 > 5) {
            iVar2 = 5;
        }
        iVar1 = (iVar2 + 10) * iVar1 * 10;

        int acc2 = g_field_cdb8[uVar4 * 0x833] + iVar1 % 100;
        g_field_cdb8[uVar4 * 0x833] = acc2;
        if (acc2 > 99) {
            iVar1 = iVar1 + 100;
            g_field_cdb8[uVar4 * 0x833] = acc2 - 100;
        }

        tech_research(uVar4, iVar1 / 100);
    }
}
