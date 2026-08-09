// ORIGINAL: 0x00532C30 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00532C30
// name           ?synch_display@@YAXHHHH@Z
// size           370 bytes
// measured tier  MISMATCH
// divergence     124
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00532c30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?synch_display@@YAXHHHH@Z  at 0x00532C30  (370 bytes)
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

class NetMsg;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
int __cdecl parse_num(int, int);
void __cdecl do_all_draws();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00532da4 = (int *)0x00532DA4;
static int *const g_0068c584 = (int *)0x0068C584;
static int *const g_0068c594 = (int *)0x0068C594;
static int *const g_0068c5a8 = (int *)0x0068C5A8;
static int *const g_0068c5b8 = (int *)0x0068C5B8;
static int *const g_0068c5c8 = (int *)0x0068C5C8;
static int *const g_0068c5dc = (int *)0x0068C5DC;
static int *const g_00805338 = (int *)0x00805338;

void __cdecl synch_display(int a1, int a2, int a3, int a4) {
    switch (a1) {
    case 0:
        parse_num(0, a2);
        parse_num(1, a3);
        reinterpret_cast<NetMsg *>(g_00805338)->pop((const char *)g_0068c584, 0, 1, 0);
        do_all_draws();
        return;
    case 6:
        parse_num(0, a2);
        parse_num(1, a3);
        parse_num(2, a4);
        reinterpret_cast<NetMsg *>(g_00805338)->pop((const char *)g_0068c594, 0, 1, 0);
        do_all_draws();
        return;
    case 4:
        parse_num(0, a2);
        parse_num(1, a3);
        reinterpret_cast<NetMsg *>(g_00805338)->pop((const char *)g_0068c5a8, 0, 1, 0);
        do_all_draws();
        return;
    case 0x30:
        parse_num(0, a2);
        parse_num(1, a3);
        parse_num(2, a4);
        reinterpret_cast<NetMsg *>(g_00805338)->pop((const char *)g_0068c5b8, 0, 1, 0);
        do_all_draws();
        return;
    case 0x12:
        parse_num(0, a2);
        parse_num(1, a3);
        reinterpret_cast<NetMsg *>(g_00805338)->pop((const char *)g_0068c5c8, 0, 1, 0);
        do_all_draws();
        return;
    case 0x14:
        parse_num(0, a2);
        parse_num(1, a3);
        reinterpret_cast<NetMsg *>(g_00805338)->pop((const char *)g_0068c5dc, 0, 1, 0);
        do_all_draws();
        break;
    }
}
