// ORIGINAL: 0x004E3660 sub_4e3660 0x004E3660-0x004E370C FILE
// TRIED: [ecx+N] reads with no stack args -> a thiscall receiver (Obj4e3660), not the given nullary __cdecl. 0.91 mnemonic sim; remaining diff is the prologue's `push ecx` stack-slot local vs a register-kept loop counter, and one lea/add adjustor-call addressing mode (same shape as 0x0051E480's) - not chased.
// working copy - scaffold materialised by --work
// size      172 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x004E36A3 0x004E36B0 0x004E36D1

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4e3660  at 0x004E3660  (172 bytes)
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
// This body dispatches through slot(s): 0, 1, 2
class VCall { public:
    virtual void slot000(int);  // <-- used
    virtual void slot001(void *);  // <-- used
    virtual void slot002(int);  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_0066eaf8 = (int *)0x0066EAF8;
static int *const g_0066eafc = (int *)0x0066EAFC;
static int *const g_009b3374 = (int *)0x009B3374;
class Obj4e3660 {
public:
    void sub_4e3660();
};

void Obj4e3660::sub_4e3660() {
    char *self = reinterpret_cast<char *>(this);
    *reinterpret_cast<int *>(self) = reinterpret_cast<int>(g_0066eafc);
    int adj = *reinterpret_cast<int *>(*reinterpret_cast<int *>(self + 4) + 4);
    *reinterpret_cast<int *>(self + 4 + adj) = reinterpret_cast<int>(g_0066eaf8);
    if (*reinterpret_cast<void **>(self + 8) != 0) {
        int i = 0;
        if (*reinterpret_cast<int *>(self + 0x10) > 0) {
            do {
                void *node = *reinterpret_cast<void **>(self + 8);
                *reinterpret_cast<void **>(self + 0xc) = *reinterpret_cast<void **>(reinterpret_cast<char *>(node) + 0xc);
                void *payload = *reinterpret_cast<void **>(reinterpret_cast<char *>(node) + 8);
                ((VCall *)self)->slot001(payload);
                if (payload != 0) {
                    ((VCall *)payload)->slot002(1);
                }
                *reinterpret_cast<int *>(*reinterpret_cast<char **>(self + 8) + 8) = 0;
                void *head = *reinterpret_cast<void **>(self + 8);
                if (head != 0) {
                    int *vt = *reinterpret_cast<int **>(head);
                    int adjustor2 = *reinterpret_cast<int *>(reinterpret_cast<char *>(vt) + 4);
                    ((VCall *)(reinterpret_cast<char *>(head) + adjustor2))->slot000(1);
                }
                *reinterpret_cast<void **>(self + 8) = *reinterpret_cast<void **>(self + 0xc);
                ++i;
            } while (i < *reinterpret_cast<int *>(self + 0x10));
        }
        *reinterpret_cast<int *>(self + 8) = 0;
        *reinterpret_cast<int *>(self + 0x14) = 0;
        *reinterpret_cast<int *>(self + 0x10) = 0;
    }
    *reinterpret_cast<int *>(self + 0x14) = 0;
    int val = *reinterpret_cast<int *>(self + 0x20);
    *reinterpret_cast<int *>(self + 0x1c) = reinterpret_cast<int>(g_006693ac);
    *g_009b3374 = val;
}
