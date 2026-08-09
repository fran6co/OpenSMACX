// ORIGINAL: 0x005FD2C0 FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005FD2C0
// name           sub_5fd2c0
// size           72 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fd2c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5fd2c0  at 0x005FD2C0  (72 bytes)
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
void check_net();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669340 = (int *)0x00669340;
static int *const g_00669358 = (int *)0x00669358;
static int *const g_0066935c = (int *)0x0066935C;
struct MsgPoint_ { long x, y; };
struct Msg_ {
    void *hwnd;
    unsigned int message;
    unsigned int wParam;
    long lParam;
    unsigned long time;
    MsgPoint_ pt;
};

extern "C" __declspec(dllimport) int __stdcall PeekMessageA(
    Msg_ *, void *, unsigned int, unsigned int, unsigned int);
extern "C" __declspec(dllimport) int __stdcall TranslateMessage(const Msg_ *);
extern "C" __declspec(dllimport) int __stdcall DispatchMessageA(const Msg_ *);

extern "C" int __cdecl sub_5fd2c0() {
    Msg_ msg;
    if (!PeekMessageA(&msg, 0, 0x401, 0x401, 1)) {
        return 0;
    }
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
    check_net();
    return 1;
}
