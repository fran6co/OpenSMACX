// ORIGINAL: 0x00635450 sub_635450 0x00635450-0x006354CA FILE
// TRIED: falling off the end without a return (C4716, error under the scoring flags); "return 0" adds an extra "xor eax,eax" the original's bare "pop esi; ret" tail does not have (#36). Modelled the 3 vtable dispatches as raw __stdcall function pointers (COM-style, this pushed as an explicit arg) rather than the VCall thiscall shim, since the disassembly pushes "this" onto the stack instead of loading it into ecx.
// working copy - scaffold materialised by --work
// size      122 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EFD00
// indirect  0x0063546A 0x00635482 0x0063548D 0x006354A0 0x006354B7

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_635450  at 0x00635450  (122 bytes)
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
static int *const g_009be618 = (int *)0x009BE618;
static int *const g_009be61c = (int *)0x009BE61C;
static int *const g_009be624 = (int *)0x009BE624;
static int *const g_009be628 = (int *)0x009BE628;
extern "C" int __cdecl sub_635450() {
    typedef int (__stdcall *VFnThisArg)(int, int);
    typedef int (__stdcall *VFnThis)(int);
    typedef int (__stdcall *DestroyWindowFn)(int);
    typedef int (__stdcall *UnregisterClassFn)(int, int);

    if (*g_009be624 != 0 && *g_009be628 != 0) {
        int obj = *g_009be624;
        int vtable = *reinterpret_cast<int *>(obj);
        VFnThisArg fn = *reinterpret_cast<VFnThisArg *>(vtable + 0x80);
        fn(obj, *g_009be628);
        *g_009be628 = 0;
    }
    if (*g_009be61c != 0) {
        int obj = *g_009be61c;
        int vtable = *reinterpret_cast<int *>(obj);
        VFnThis fn = *reinterpret_cast<VFnThis *>(vtable + 0x4c);
        fn(obj);
        obj = *g_009be61c;
        vtable = *reinterpret_cast<int *>(obj);
        VFnThis fn2 = *reinterpret_cast<VFnThis *>(vtable + 8);
        fn2(obj);
        *g_009be61c = 0;
    }
    if (*g_009be618 != 0) {
        (*reinterpret_cast<DestroyWindowFn *>(g_0066930c))(*g_009be618);
        *g_009be618 = 0;
        (*reinterpret_cast<UnregisterClassFn *>(g_006692a4))(
            reinterpret_cast<int>(g_00697f3c), *g_009b7b14);
    }
    *g_009be624 = 0;
    sub_5efd00();
    return 0;
}
