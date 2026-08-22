// ORIGINAL: 0x0062D2C0 sub_62d2c0 0x0062D2C0-0x0062D38E FILE
// TRIED: IntersectRect/EqualRect are called through the fixed IAT-style slots (0x669338/0x6692bc) with `this` pushed as an explicit stdcall arg, not via C++ virtual dispatch - modelled with raw `PFN_*` casts. Every early-out in the original shares ONE epilogue (add esp,0x40; ret); separate `if (x) return 0;` statements each get their own inline return instead of jumping to a shared tail, which is most of the 0.82 mnemonic-similarity gap.
// working copy - scaffold materialised by --work
// size      206 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0062D2E4 0x0062D2FC 0x0062D310 0x0062D374 0x0062D386

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_62d2c0  at 0x0062D2C0  (206 bytes)
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
struct SimpleRect { int left, top, right, bottom; };
typedef int (__stdcall *PFN_IntersectRect)(SimpleRect *, const SimpleRect *, const SimpleRect *);
typedef int (__stdcall *PFN_EqualRect)(const SimpleRect *, const SimpleRect *);
typedef void (__stdcall *PFN_Slot28)(int, int);
typedef void (__stdcall *PFN_Slot7)(int, void *, int);

extern "C" int __cdecl sub_62d2c0() {
    if (*g_009bc49c == 0) {
        return 0;
    }
    SimpleRect *src = reinterpret_cast<SimpleRect *>(g_009bc2d0);
    SimpleRect tmpRect;
    int ok = (*reinterpret_cast<PFN_IntersectRect *>(g_00669338))(&tmpRect, src, src);
    if (ok == 0) {
        return 0;
    }
    int eq = (*reinterpret_cast<PFN_EqualRect *>(g_006692bc))(src, &tmpRect);
    if (eq == 0) {
        int obj = *g_009bc49c;
        int vt = *reinterpret_cast<int *>(obj);
        PFN_Slot28 fn = *reinterpret_cast<PFN_Slot28 *>(vt + 0x70);
        fn(obj, 0);
        return 0;
    }

    int localBuf[12];
    localBuf[0] = 0x20;
    localBuf[1] = 1;
    localBuf[2] = 1;
    localBuf[3] = 0x10;
    localBuf[4] = tmpRect.left;
    localBuf[5] = tmpRect.top;
    localBuf[6] = tmpRect.right;
    localBuf[7] = tmpRect.bottom;
    localBuf[8] = tmpRect.left;
    localBuf[9] = tmpRect.top;
    localBuf[10] = tmpRect.right;
    localBuf[11] = tmpRect.bottom;

    int obj2 = *g_009bc4a4;
    int vt2 = *reinterpret_cast<int *>(obj2);
    PFN_Slot7 fn7 = *reinterpret_cast<PFN_Slot7 *>(vt2 + 0x1c);
    fn7(obj2, localBuf, 0);

    int obj1 = *g_009bc49c;
    int vt1 = *reinterpret_cast<int *>(obj1);
    PFN_Slot28 fn28 = *reinterpret_cast<PFN_Slot28 *>(vt1 + 0x70);
    fn28(obj1, obj2);
    return 0;
}
