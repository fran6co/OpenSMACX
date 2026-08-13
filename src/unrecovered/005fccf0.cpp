// ORIGINAL: 0x005FCCF0 BYTE_EXACT FILE
// LEVER: cast the three fixed IAT slots to __stdcall function-pointer types
//        and called through them directly
// working copy - scaffold materialised by --work
// name      sub_5fccf0
// size      77 bytes
// spans     0x005FCCF0-0x005FCD3D
// prototype 
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0062D5D0 0x00636300
// indirect  0x005FCD10 0x005FCD23 0x005FCD2E

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5fccf0  at 0x005FCCF0  (77 bytes)
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
void do_video();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669340 = (int *)0x00669340;
static int *const g_00669358 = (int *)0x00669358;
static int *const g_0066935c = (int *)0x0066935C;
typedef int (__stdcall *PeekMessageProc)(int *, void *, unsigned int, unsigned int, unsigned int);
typedef int (__stdcall *TranslateMessageProc)(int *);
typedef int (__stdcall *DispatchMessageProc)(int *);

extern "C" int __cdecl sub_5fccf0() {
    int msg[7];
    do_video();
    check_net();
    if (!(*reinterpret_cast<PeekMessageProc *>(g_00669358))(msg, 0, 0x100, 0x108, 1)) {
        return 0;
    }
    (*reinterpret_cast<TranslateMessageProc *>(g_0066935c))(msg);
    (*reinterpret_cast<DispatchMessageProc *>(g_00669340))(msg);
    return 1;
}
