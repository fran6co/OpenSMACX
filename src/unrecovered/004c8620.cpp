// ORIGINAL: 0x004C8620 sub_4c8620 0x004C8620-0x004C86C9;0x004C8450-0x004C8457;0x00659F5E-0x00659F70 FILE
// RULED-OUT: byte-exact - the SEH frame prologue (push -1/handler; mov fs:[0]) has no C++ source form to reproduce without a real try/catch that this Sound-constructor body doesn't need (documented wall). Landed as a __thiscall ctor-shaped member returning `this`, with the progressive vtable_storage_ reassignment (0x66e444 -> 0x66e3c0 -> 0x66e790) and the self-clobbering dead `if (device_ != 0)` check (device_ is zeroed immediately before the read) transcribed as-is.
// PROPOSAL: sub_4c8620() -> Sound-shaped __thiscall ctor(void); ecx=this
//           (never a stack arg), eax=this on return (mov eax,esi at
//           0x4C86B9).
// working copy - scaffold materialised by --work
// size      194 bytes
// prototype 
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004C61E0 0x006465F0
// indirect  0x004C86A2

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4c8620  at 0x004C8620  (194 bytes)
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

class Sound;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Sound { public:
    uint32_t vtable_storage_;
    uint32_t volume_;
    int32_t pan_8_;
    uint32_t field_C_;
    uint32_t field_10_;
    uint32_t field_14_;
    uint32_t field_18_;
    uint32_t field_1C_;
    uint32_t field_20_;
    uint32_t field_24_;
    uint32_t field_28_;
    uint32_t field_2C_;
    uint32_t loop_flag_30_;
    uint32_t delay_;
    uint32_t fade_38_;
    void * device_;
    uint32_t flags_40_;
    Sound * chain_prev_;
    Sound * chain_next_;
    void * fname_;
    uint32_t type_;
    void set_type(unsigned int);
};

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
static int *const g_00659f66 = (int *)0x00659F66;
static int *const g_0066e3c0 = (int *)0x0066E3C0;
static int *const g_0066e444 = (int *)0x0066E444;
static int *const g_0066e790 = (int *)0x0066E790;
static int *const g_006791e8 = (int *)0x006791E8;
class VCallArg { public:
    virtual void slot000(unsigned int);
};

class Obj4c8620 { public:
    void *ctor();
};

void *Obj4c8620::ctor() {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<uint32_t *>(self) = 0x66e444;
    *reinterpret_cast<uint32_t *>(self + 4) = 0x7f;
    *reinterpret_cast<uint32_t *>(self + 8) = 0;
    memset(self + 0xc, 0, 0x24);
    *reinterpret_cast<uint32_t *>(self + 0x30) = 0;
    *reinterpret_cast<uint32_t *>(self) = 0x66e3c0;
    *reinterpret_cast<uint32_t *>(self + 0x44) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x48) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x3c) = 0;
    *reinterpret_cast<uint32_t *>(self + 0x4c) = 0;
    memset(self + 0x40, 0, 4);
    uint32_t flags = *reinterpret_cast<uint32_t *>(self + 0x40);
    flags &= 0xfffffffe;
    *reinterpret_cast<uint32_t *>(self + 0x38) = 0x3e8;
    *reinterpret_cast<uint32_t *>(self + 0x40) = flags;
    uint32_t device = *reinterpret_cast<uint32_t *>(self + 0x3c);
    if (device != 0) {
        reinterpret_cast<VCallArg *>(device)->slot000(0x3e8);
    }
    *reinterpret_cast<uint32_t *>(self + 0x50) = 0;
    *reinterpret_cast<uint32_t *>(self) = 0x66e790;
    reinterpret_cast<Sound *>(this)->set_type(4);
    return this;
}
