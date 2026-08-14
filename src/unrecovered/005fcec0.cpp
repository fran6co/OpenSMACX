// ORIGINAL: 0x005FCEC0 FILE
// RULED-OUT: fall-off-end int return (C4716 error, needed explicit return 0);
//            for(;;)+continue and do/while(true)+continue both compile the
//            same 'continue' back-edge as an inlined re-run of do_video()/
//            check_net() instead of the original's single trailing jmp.
// working copy - scaffold materialised by --work
// name      sub_5fcec0
// size      204 bytes
// spans     0x005FCEC0-0x005FCF8C
// prototype 
// callers   0   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5B0 0x0062D5D0 0x00636300
// indirect  0x005FCEF5 0x005FCF04 0x005FCF0B 0x005FCF2D 0x005FCF38 0x005FCF3F 0x005FCF63 0x005FCF72 0x005FCF79

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5fcec0  at 0x005FCEC0  (204 bytes)
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
void do_net();
void do_video();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669340 = (int *)0x00669340;
static int *const g_00669358 = (int *)0x00669358;
static int *const g_0066935c = (int *)0x0066935C;

struct MSG {
    void *hwnd;
    unsigned int message;
    unsigned int wParam;
    long lParam;
    unsigned long time;
    long pt_x;
    long pt_y;
};

typedef int (__stdcall *PeekMessageAFn)(MSG *, void *, unsigned int, unsigned int, unsigned int);
typedef int (__stdcall *TranslateMessageFn)(const MSG *);
typedef long (__stdcall *DispatchMessageAFn)(const MSG *);

extern "C" int __cdecl sub_5fcec0() {
    MSG msg;
    PeekMessageAFn peek = *reinterpret_cast<PeekMessageAFn *>(g_00669358);
    TranslateMessageFn translate = *reinterpret_cast<TranslateMessageFn *>(g_0066935c);
    DispatchMessageAFn dispatch = *reinterpret_cast<DispatchMessageAFn *>(g_00669340);

    for (;;) {
        do_video();
        check_net();
        if (!peek(&msg, 0, 0x200, 0x209, 1)) {
            break;
        }
        translate(&msg);
        dispatch(&msg);
        if (msg.message == 0x201) {
            if (peek(&msg, msg.hwnd, 0x202, 0x202, 1)) {
                translate(&msg);
                dispatch(&msg);
                continue;
            }
        }
        if (msg.message == 0x204) {
            if (peek(&msg, msg.hwnd, 0x205, 0x205, 1)) {
                translate(&msg);
                dispatch(&msg);
            }
        }
    }
    do_net();
    return 0;
}
