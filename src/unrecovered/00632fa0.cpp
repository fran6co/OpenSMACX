// ORIGINAL: 0x00632FA0 ?check_message@Net@@QAEHKK@Z 0x00632FA0-0x00633007 FILE
// RULED-OUT: do-while loop form (same #21 jne/je divergence); reordering the i==0x10 branch (early-return vs nested if) did not change it.
// working copy - scaffold materialised by --work
// size      103 bytes
// prototype int (__thiscall ?check_message@Net@@QAEHKK@Z)(Net* this, unsigned int, unsigned int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
// indirect  0x00632FB6

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?check_message@Net@@QAEHKK@Z  at 0x00632FA0  (103 bytes)
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

class Net;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669318 = (int *)0x00669318;
static int *const g_00697efc = (int *)0x00697EFC;
static int *const g_00697f10 = (int *)0x00697F10;

typedef void *HWND;
typedef const char *LPCSTR;
typedef unsigned int UINT;
typedef int (__stdcall *MessageBoxAFn)(HWND, LPCSTR, LPCSTR, UINT);

class Net { public:
    int check_message(unsigned long, unsigned long);
};
int Net::check_message(unsigned long a1, unsigned long a2) {
    unsigned long *rec = (unsigned long *)((char *)this + 0x154);
    if (rec == 0) {
        ((MessageBoxAFn)*(void **)g_00669318)(0, (LPCSTR)g_00697f10, (LPCSTR)g_00697efc, 0);
        return 1;
    }
    int i;
    for (i = 0; i < 0x10; ++i, rec += 0x16) {
        if (*rec == a1) {
            break;
        }
    }
    if (i == 0x10) {
        return 0;
    }
    unsigned long *counter = (unsigned long *)((char *)this + 0x158 + i * 0x58);
    if (a2 <= *counter) {
        return 1;
    }
    *counter = a2;
    return 0;
}
