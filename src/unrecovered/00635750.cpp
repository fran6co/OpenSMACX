// ORIGINAL: 0x00635750 sub_635750 0x00635750-0x006357C1 FILE BYTE_EXACT
// symbol    ?teardown@TeardownObj@@QAEXXZ
// LEVER: __cdecl callback typedef -> __stdcall; the two extra `add esp,N` instructions were cdecl caller-side stack cleanup the original (callee-cleans) convention never performs.
// working copy - scaffold materialised by --work
// size      113 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EFD00
// indirect  0x00635765 0x0063577C 0x00635785 0x00635796 0x006357AD

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_635750  at 0x00635750  (113 bytes)
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
extern "C" int __cdecl sub_5efd00();

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 2, 19, 32
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();  // <-- used
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
    virtual void slot019();  // <-- used
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
static int *const g_006692a4 = (int *)0x006692A4;
static int *const g_0066930c = (int *)0x0066930C;
static int *const g_00697f3c = (int *)0x00697F3C;
static int *const g_009b7b14 = (int *)0x009B7B14;
typedef void (__stdcall *Fn1Arg)(void *);
typedef void (__stdcall *Fn2Arg)(void *, int);
typedef int (__stdcall *DestroyWindowFn)(void *);
typedef int (__stdcall *UnregisterClassAFn)(const char *, int);

class TeardownObj { public:
    void *hwnd_;
    void *obj2_;
    void *pad8_;
    void *obj1_;
    int val10_;

    void teardown();
};

void TeardownObj::teardown() {
    void *obj1 = obj1_;
    if (obj1 != 0 && val10_ != 0) {
        Fn2Arg fn = *reinterpret_cast<Fn2Arg *>((char *)*(void **)obj1 + 0x80);
        fn(obj1, val10_);
        val10_ = 0;
    }
    void *obj2 = obj2_;
    if (obj2 != 0) {
        Fn1Arg fn1 = *reinterpret_cast<Fn1Arg *>((char *)*(void **)obj2 + 0x4c);
        fn1(obj2);
        obj2 = obj2_;
        Fn1Arg fn2 = *reinterpret_cast<Fn1Arg *>((char *)*(void **)obj2 + 8);
        fn2(obj2);
        obj2_ = 0;
    }
    if (hwnd_ != 0) {
        DestroyWindowFn destroy = (DestroyWindowFn)*g_0066930c;
        destroy(hwnd_);
        hwnd_ = 0;
        UnregisterClassAFn unreg = (UnregisterClassAFn)*g_006692a4;
        unreg((const char *)g_00697f3c, *g_009b7b14);
    }
    obj1_ = 0;
    sub_5efd00();
}
