// ORIGINAL: 0x00634020 sub_634020 0x00634020-0x006340B5 FILE
// working copy - scaffold materialised by --work
// size      149 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00645930
// indirect  0x00634028 0x00634035 0x0063405B 0x0063406D 0x006340A8

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634020  at 0x00634020  (149 bytes)
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
extern "C" void *memcpy(void *, const void *, unsigned int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669174 = (int *)0x00669174;
static int *const g_0066917c = (int *)0x0066917C;
// Signature change: [ecx]/[ecx+4]/[ecx+8]/[ecx+0xc] are read with no
// matching stack push, so the receiver is `this` (a real __thiscall
// member) rather than a plain __stdcall(a1..a4) free function.
class LinkedObj { public:
    int *head_;
    int pad4_;
    int pad8_;
    unsigned char cs_[24];

    int sub_634020(int a1, int a2, int a3, int a4);
};

int LinkedObj::sub_634020(int a1, int a2, int a3, int a4) {
    typedef void (__stdcall *CritFn)(void *);
    CritFn enterCS = reinterpret_cast<CritFn>(*g_0066917c);
    CritFn leaveCS = reinterpret_cast<CritFn>(*g_00669174);
    void *cs = &cs_;
    enterCS(cs);

    int *node = head_;
    if (node == 0) {
        leaveCS(cs);
        return 0;
    }
    while (node[1] != a2 || node[0] != a3) {
        node = reinterpret_cast<int *>(node[5]);
        if (node == 0) {
            leaveCS(cs);
            return 0;
        }
    }
    if (a4 != 0) {
        *reinterpret_cast<int *>(a4) = node[4];
    }
    if (node[3] != 0 && a1 != 0) {
        memcpy(reinterpret_cast<void *>(a1), reinterpret_cast<void *>(node[3]), node[4]);
    }
    int result = node[2];
    leaveCS(cs);
    return result;
}
