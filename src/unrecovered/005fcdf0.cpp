// ORIGINAL: 0x005FCDF0 FILE
// RULED-OUT: per-call reinterpret_cast<Proc*>(g) load (5 call sites) instead
//            of caching Peek/Translate/Dispatch ptrs in registers once
// working copy - scaffold materialised by --work
// name      sub_5fcdf0
// size      208 bytes
// spans     0x005FCDF0-0x005FCEC0
// prototype 
// callers   0   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5D0 0x00636300
// indirect  0x005FCE19 0x005FCE31 0x005FCE3E 0x005FCE60 0x005FCE6B 0x005FCE72 0x005FCEA0 0x005FCEAB 0x005FCEB2

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5fcdf0  at 0x005FCDF0  (208 bytes)
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

extern "C" int __cdecl sub_5fcdf0() {
    int msg[7];
    do_video();
    check_net();
    if (!(*reinterpret_cast<PeekMessageProc *>(g_00669358))(msg, reinterpret_cast<void *>(0), 0x200, 0x209, 1)) {
        return 0;
    }
    (*reinterpret_cast<TranslateMessageProc *>(g_0066935c))(msg);
    (*reinterpret_cast<DispatchMessageProc *>(g_00669340))(msg);
    if (msg[1] == 0x201) {
        if ((*reinterpret_cast<PeekMessageProc *>(g_00669358))(msg, reinterpret_cast<void *>(msg[0]), 0x202, 0x202, 1)) {
            (*reinterpret_cast<TranslateMessageProc *>(g_0066935c))(msg);
            (*reinterpret_cast<DispatchMessageProc *>(g_00669340))(msg);
            return 1;
        }
    }
    if (msg[1] == 0x204) {
        if ((*reinterpret_cast<PeekMessageProc *>(g_00669358))(msg, reinterpret_cast<void *>(msg[0]), 0x205, 0x205, 1)) {
            (*reinterpret_cast<TranslateMessageProc *>(g_0066935c))(msg);
            (*reinterpret_cast<DispatchMessageProc *>(g_00669340))(msg);
        }
    }
    return 1;
}
