// ORIGINAL: 0x005CC860 FILE
// RULED-OUT: __stdcall(int,int,int) per IDA's guess - entry reads ecx/edx
//            directly with no matching stack slot, so this is __fastcall
//            with param_1/param_2 in ecx/edx. The three vtable-style calls
//            push their receiver as an explicit stack arg rather than
//            loading it into ecx, so they are plain function-pointer calls
//            through *(vtable+off), not C++ virtual dispatch; exact stack
//            slot reuse for the WSA-shaped struct (0x6c/0x1007/0x80/0x20/4)
//            not nailed down byte-for-byte.
// working copy - scaffold materialised by --work
// name      sub_5cc860
// size      213 bytes
// spans     0x005CC860-0x005CC935
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005CC8D1 0x005CC8EE 0x005CC90A 0x005CC928

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5cc860  at 0x005CC860  (213 bytes)
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
// This body dispatches through slot(s): 0, 2, 6, 29
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();
    virtual void slot002();  // <-- used
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();  // <-- used
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
    virtual void slot029();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0066fc28 = (int *)0x0066FC28;

typedef int (__stdcall *Fn3)(void *, void *, void *);
typedef int (__stdcall *Fn2)(void *, void *);
typedef int (__stdcall *Fn1)(void *);

extern "C" int __fastcall sub_5cc860(void *param_1, int *param_2, int param_3, int param_4, int param_5) {
    void *puStack_88 = 0;
    char local_78[12];
    int local_6c[18];

    for (int i = 0; i < 18; ++i) {
        local_6c[i] = 0;
    }

    local_6c[3] = param_3;
    void *puStack_8c = local_78;
    void *puStack_90 = local_6c;
    local_6c[2] = param_4;
    local_6c[0] = 0x6c;
    local_6c[1] = 0x1007;

    void *vt1 = *reinterpret_cast<void **>(param_1);
    Fn3 fn18 = *reinterpret_cast<Fn3 *>(reinterpret_cast<char *>(vt1) + 0x18);
    int result = fn18(param_1, puStack_90, puStack_8c);
    if (result == 0) {
        void *vt2 = *reinterpret_cast<void **>(puStack_88);
        Fn2 fn0 = *reinterpret_cast<Fn2 *>(vt2);
        result = fn0(puStack_88, reinterpret_cast<void *>(g_0066fc28));
        if (result == 0) {
            puStack_90 = reinterpret_cast<void *>(1);
            puStack_8c = reinterpret_cast<void *>(1);
            void *vt3 = *reinterpret_cast<void **>(puStack_88);
            typedef int (__stdcall *Fn3b)(void *, int, void *);
            Fn3b fn74 = *reinterpret_cast<Fn3b *>(reinterpret_cast<char *>(vt3) + 0x74);
            result = fn74(puStack_88, 4, &puStack_90);
            if (result == 0) {
                *param_2 = reinterpret_cast<int>(&puStack_8c);
                return 0;
            }
        }
        void *vt4 = *reinterpret_cast<void **>(param_1);
        Fn1 fn8 = *reinterpret_cast<Fn1 *>(reinterpret_cast<char *>(vt4) + 8);
        fn8(param_1);
    }
    return 1;
}
