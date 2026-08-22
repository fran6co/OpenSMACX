// ORIGINAL: 0x0062D1E0 sub_62d1e0 0x0062D1E0-0x0062D2B1 FILE
// TRIED: 209 vs 204 bytes; struct-literal field order for the 0x30-byte DrawRegion local drifts the compiler's address-of scheduling (lea moves relative to the field stores) versus the original's register reuse. Tried plain field assigns and an early `DrawRegion *preg = &region;` alias; neither matched.
// working copy - scaffold materialised by --work
// size      209 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0062D207 0x0062D21F 0x0062D233 0x0062D297 0x0062D2A9

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_62d1e0  at 0x0062D1E0  (209 bytes)
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
// This body dispatches through slot(s): 7, 28
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();  // <-- used
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
    virtual void slot028();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006692bc = (int *)0x006692BC;
static int *const g_00669338 = (int *)0x00669338;
static int *const g_009bc2d0 = (int *)0x009BC2D0;
static int *const g_009bc49c = (int *)0x009BC49C;
static int *const g_009bc4a4 = (int *)0x009BC4A4;
// [esp+0x44] with no push before it reads a stack argument, not `this`;
// the contract's nullary `sub_62d1e0()` is wrong. Recast with the one
// `RECT *` argument IDA guessed; see PROPOSALS. Both indirect calls at
// fixed addresses are Win32 import-table slots (IntersectRect, EqualRect),
// and the two `call dword ptr [reg+N]` sites push their own receiver on
// the stack (stdcall), not through ECX, so they are not real C++ virtuals.
struct RECT {
    long left;
    long top;
    long right;
    long bottom;
};

struct DrawRegion {
    int a;
    int flag1;
    int flag2;
    int b;
    RECT rect1;
    RECT rect2;
};

typedef int (__stdcall *IntersectRectFn)(RECT *, const RECT *, const RECT *);
typedef int (__stdcall *EqualRectFn)(const RECT *, const RECT *);
typedef int (__stdcall *Fn1c)(void *, void *, int);
typedef int (__stdcall *Fn70)(void *, void *);

extern "C" void __cdecl sub_62d1e0(RECT *param_1) {
    if (*g_009bc49c == 0 || param_1 == 0) {
        return;
    }

    RECT tmp;
    if (!(reinterpret_cast<IntersectRectFn>(*g_00669338))(
            &tmp, reinterpret_cast<RECT *>(g_009bc2d0), param_1)) {
        return;
    }

    if ((reinterpret_cast<EqualRectFn>(*g_006692bc))(
            reinterpret_cast<RECT *>(g_009bc2d0), &tmp)) {
        void *obj = reinterpret_cast<void *>(*g_009bc49c);
        void *vtbl = *reinterpret_cast<void **>(obj);
        Fn70 fn = *reinterpret_cast<Fn70 *>(reinterpret_cast<char *>(vtbl) + 0x70);
        fn(obj, 0);
        return;
    }

    DrawRegion region;
    region.a = 0x20;
    region.flag1 = 1;
    region.flag2 = 1;
    region.b = 0x10;
    region.rect1 = tmp;
    region.rect2 = tmp;

    void *obj2 = reinterpret_cast<void *>(*g_009bc4a4);
    void *vtbl2 = *reinterpret_cast<void **>(obj2);
    Fn1c fn1c = *reinterpret_cast<Fn1c *>(reinterpret_cast<char *>(vtbl2) + 0x1c);
    fn1c(obj2, &region, 0);

    void *obj = reinterpret_cast<void *>(*g_009bc49c);
    void *vtbl = *reinterpret_cast<void **>(obj);
    Fn70 fn = *reinterpret_cast<Fn70 *>(reinterpret_cast<char *>(vtbl) + 0x70);
    fn(obj, obj2);
}
