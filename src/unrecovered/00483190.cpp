// ORIGINAL: 0x00483190 ?modal_callback@NetWin@@QAEHXZ 0x00483190-0x00483234 FILE
// TRIED: MISMATCH #2 mov/push, do/while call-site folding differs
// working copy - scaffold materialised by --work
// size      164 bytes
// prototype int (__thiscall ?modal_callback@NetWin@@QAEHXZ)(NetWin* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00530320
// indirect  0x004831C1

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?modal_callback@NetWin@@QAEHXZ  at 0x00483190  (164 bytes)
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

class NetDaemon;
class NetWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetDaemon { public:
    int receive();
};


// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 12
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090dd15 = (int *)0x0090DD15;
static int *const g_0093cd90 = (int *)0x0093CD90;
static int *const g_0093d4f8 = (int *)0x0093D4F8;
static int *const g_009b2068 = (int *)0x009B2068;

class NetWin { public:
    int modal_callback();
};
extern "C" char g_90dd15_arr[];

int NetWin::modal_callback() {
    char *self = (char *)this;
    NetDaemon *nd = (NetDaemon *)g_0093cd90;

    if (nd->receive() != 0) {
        int r;
        do {
            r = nd->receive();
        } while (r != 0);
        ((VCall *)self)->slot012();
    }

    int result = 1;
    if (*(int *)(self + 0x7614) == 0) {
        *(int *)(self + 0x7618) = 1;
        int i = 1;
        if (*g_0093d4f8 >= 1) {
            char *p = g_90dd15_arr;
            do {
                if (*p == 0) {
                    *(int *)(self + 0x7618) = 0;
                }
                i = i + 1;
                p = p + 0x17c;
            } while (i <= *g_0093d4f8);
        }
        if (*(int *)(self + 0x7618) != 0) {
            *(int *)(self + 0x7614) = 1;
        }
    }

    if (*(int *)(self + 0x7614) != 0 || *g_009b2068 != 0) {
        result = 0;
    }
    return result;
}
