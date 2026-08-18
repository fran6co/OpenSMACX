// ORIGINAL: 0x005CDAD0 sub_5cdad0 0x005CDAD0-0x005CDB83 FILE
// RULED-OUT: signature changed stdcall(int,int,int) -> fastcall(int*,int*,int,int,int); evidence: ecx/edx read directly at entry with no matching stack loads, `ret 0xc` pops exactly 3 stack dwords. Rebuilt vtable dispatch (slots 6, 0, 2) as void** casts. First divergence at instr #3 mov vs push (prologue register-save order), not chased further.
// working copy - scaffold materialised by --work
// size      179 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CDB3E 0x005CDB56 0x005CDB73

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cdad0  at 0x005CDAD0  (179 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 2, 6
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();
    virtual void slot002();  // <-- used
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066fc28 = (int *)0x0066FC28;
extern "C" int __fastcall sub_5cdad0(int *a1, int *a2, int a3, int a4, int a5) {
    int buf[27] = {0};
    buf[3] = a3;
    buf[2] = a4;
    buf[0] = 0x6c;
    buf[1] = 0x1007;
    buf[26] = 0x4040;
    buf[18] = 0x20;
    buf[19] = 4;
    buf[20] = a5;

    typedef int (__stdcall *Fn3)(void*, void*, void*, int);
    typedef int (__stdcall *Fn1)(void*, void*, void*);
    typedef int (__stdcall *Fn0)(void*);

    void **vtbl = *(void***)a1;
    int rc = ((Fn3)vtbl[6])(a1, buf, &a3, 0);
    if (rc == 0) {
        void *obj = (void*)a3;
        void **vtbl2 = *(void***)obj;
        int rc2 = ((Fn1)vtbl2[0])(obj, (void*)0x0066fc28, &a4);
        if (rc2 == 0) {
            *a2 = a4;
            return 0;
        }
        void **vtbl3 = *(void***)obj;
        ((Fn0)vtbl3[2])(obj);
    }
    return 1;
}
