// ORIGINAL: 0x004ACA90 ?report_projects@@YAXXZ 0x004ACA90-0x004ACADC FILE
// TRIED: explicit __thiscall function-pointer typedef for the vtable call (C4234); used a member-pointer/union cast instead
// working copy - scaffold materialised by --work
// size      76 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004A3950
// indirect  0x004ACAC0

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?report_projects@@YAXXZ  at 0x004ACA90  (76 bytes)
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

class ReportIf;

// ---- callees, declared and never defined (a definition would be inlined) ----
class ReportIf { public:
    void report(int, int, int);
};


// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005398e0 = (int *)0x005398E0;
static int *const g_007ae778 = (int *)0x007AE778;
static int *const g_007ae820 = (int *)0x007AE820;
static int *const g_007d392c = (int *)0x007D392C;
static int *const g_00876478 = (int *)0x00876478;
static int *const g_00885f38 = (int *)0x00885F38;
static int *const g_00885f40 = (int *)0x00885F40;
static int *const g_008a4160 = (int *)0x008A4160;
class ReportVCallHelper {
public:
    void call(int, void *, void *);
};

union ReportVCallConverter {
    void (ReportVCallHelper::*asMember)(int, void *, void *);
    void *asRaw;
};

void __cdecl report_projects() {
    int index = *g_007d392c;
    *g_00885f40 = 5;
    if (g_007ae778[index] == 5) {
        int value = *g_008a4160;
        reinterpret_cast<ReportIf *>(g_00885f38)->report(5, value, 0);
    } else {
        void **vtable = *reinterpret_cast<void ***>(g_00876478);
        ReportVCallConverter converter;
        converter.asRaw = vtable[0x39];
        (reinterpret_cast<ReportVCallHelper *>(g_00876478)->*converter.asMember)(0, g_005398e0, g_007ae820);
    }
}
