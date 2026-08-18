// ORIGINAL: 0x004C5D90 ?release_sound@@YAHXZ 0x004C5D90-0x004C5E42 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      178 bytes
// prototype 
// callers   1   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004C4F80 0x004C57F0 0x004C5A50 0x0064557F
// indirect  0x004C5DA4 0x004C5DC4 0x004C5DE7 0x004C5E26

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?release_sound@@YAHXZ  at 0x004C5D90  (178 bytes)
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

class Midi_Device;
class Wave_Device;
class Wave_In_Device;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Midi_Device { public:
    uint32_t vtable_storage_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    int release();
};

class Wave_Device { public:
    void release();
};

class Wave_In_Device { public:
    uint32_t vtable_storage_;
    uint32_t field_4_;
    uint32_t field_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    int release();
};

int __cdecl fn_0064557f();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 32
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
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669128 = (int *)0x00669128;
static int *const g_0090d950 = (int *)0x0090D950;
static int *const g_0090d978 = (int *)0x0090D978;
static int *const g_0090db20 = (int *)0x0090DB20;
static int *const g_0090db24 = (int *)0x0090DB24;
static int *const g_0090db28 = (int *)0x0090DB28;
static int *const g_0090db30 = (int *)0x0090DB30;
static int *const g_0090db50 = (int *)0x0090DB50;
static int *const g_0090db78 = (int *)0x0090DB78;
static int *const g_0090db7c = (int *)0x0090DB7C;
extern "C" void __cdecl fn_0064557f_arg(void *);

int __cdecl release_sound() {
    typedef void (__cdecl *NullaryFn)();
    typedef void (__cdecl *UnaryCdeclFn)(int);
    typedef void (__stdcall *UnaryStdcallFn)(int);

    if (*g_0090db7c == 0) {
        return 1;
    }
    (*(NullaryFn)*g_0090db30)();
    char *node = *(char **)g_0090db20;
    while (node != 0) {
        int devHandle = *(int *)(node + 0x3c);
        if (devHandle != 0) {
            if (*g_0090db7c != 0) {
                (*(UnaryCdeclFn)*g_0090db28)(devHandle);
            }
            *(int *)(node + 0x3c) = 0;
        }
        int bufPtr = *(int *)(node + 0x4c);
        if (bufPtr != 0) {
            fn_0064557f_arg((void *)bufPtr);
            *(int *)(node + 0x4c) = 0;
        }
        ((VCall *)node)->slot032();
        node = *(char **)g_0090db20;
    }
    ((Midi_Device *)g_0090d950)->release();
    ((Wave_Device *)g_0090d978)->release();
    ((Wave_In_Device *)g_0090db50)->release();
    int hLib = *g_0090db78;
    *g_0090db7c = 0;
    if (hLib != 0) {
        (*(UnaryStdcallFn)*g_00669128)(hLib);
        *g_0090db78 = 0;
    }
    int *slot = g_0090db24;
    int count = 0xb;
    while (count != 0) {
        *slot = 0;
        ++slot;
        --count;
    }
    return 0;
}
