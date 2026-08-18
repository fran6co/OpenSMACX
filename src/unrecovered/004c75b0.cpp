// ORIGINAL: 0x004C75B0 sub_4c75b0 0x004C75B0-0x004C766D;0x004C8450-0x004C8457;0x00659F20-0x00659F32 FILE
// RULED-OUT: SHARED_TAIL - the 0x4c8450/0x659f20 unwind-handler span is COMDAT-folded with another function's handler, so no tier is reachable regardless of body. Left as a plain field-write transcription (no SEH frame modeled) for the next pass.
// working copy - scaffold materialised by --work
// size      214 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x006465F0
// indirect  0x004C7632

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c75b0  at 0x004C75B0  (214 bytes)
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
extern "C" void *memset(void *, int, unsigned int);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0
class VCall { public:
    virtual void slot000();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00659f28 = (int *)0x00659F28;
static int *const g_0066e3c0 = (int *)0x0066E3C0;
static int *const g_0066e444 = (int *)0x0066E444;
static int *const g_0066e538 = (int *)0x0066E538;
static int *const g_00679168 = (int *)0x00679168;
// `mov ecx, dword ptr[0x9bc074]`-free entry, but the body reads `[ecx+N]`
// with no matching stack slot, so ecx is `this`, not a stack argument - the
// contract's nullary `__cdecl` is wrong. This is a constructor: three
// vtable installs in sequence, an SEH prologue (`fs:[0]`) with a 0x659f20
// unwind handler that resets the vtable to the first-phase value, and a
// span at 0x4c8450/0x659f20/0x659f28 that is PART of this same function
// (the "spans" list is multi-range). The SEH frame is not modeled here -
// see RULED-OUT.
class VCallArg { public:
    virtual void slot000(int a);
};

class Obj4c75b0 {
public:
    void *init();
};

void *Obj4c75b0::init() {
    char *esi = reinterpret_cast<char *>(this);

    *reinterpret_cast<void **>(esi) = g_0066e444;
    *reinterpret_cast<int *>(esi + 4) = 0x7f;
    *reinterpret_cast<int *>(esi + 8) = 0;
    memset(esi + 0xc, 0, 0x24);

    *reinterpret_cast<int *>(esi + 0x30) = 0;
    char *edi = esi + 0x40;
    *reinterpret_cast<void **>(esi) = g_0066e3c0;
    *reinterpret_cast<int *>(esi + 0x44) = 0;
    *reinterpret_cast<int *>(esi + 0x48) = 0;
    *reinterpret_cast<int *>(esi + 0x3c) = 0;
    *reinterpret_cast<int *>(esi + 0x4c) = 0;
    memset(edi, 0, 4);

    *reinterpret_cast<int *>(esi + 0x38) = 1000;
    *reinterpret_cast<unsigned int *>(edi) &= 0xfffffffe;

    void *cb = *reinterpret_cast<void **>(esi + 0x3c);
    if (cb != 0) {
        reinterpret_cast<VCallArg *>(cb)->slot000(1000);
    }

    *reinterpret_cast<int *>(esi + 0x50) = 0;
    char *ebx = esi + 0x54;
    *reinterpret_cast<void **>(esi) = g_0066e538;
    memset(ebx, 0, 1);

    *ebx |= 1;
    *reinterpret_cast<unsigned int *>(edi) |= 8;

    return this;
}
