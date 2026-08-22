// ORIGINAL: 0x004E3900 sub_4e3900 0x004E3900-0x004E39CF FILE
// TRIED: register-save push order (ebx/esi/edi prologue) drifts from a struct-model body; single-level VCall casts used for the three argument-taking slots. 207 vs 197 rebuilt bytes.
// working copy - scaffold materialised by --work
// size      207 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00644EF2
// indirect  0x004E3944 0x004E3951 0x004E3972

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_4e3900  at 0x004E3900  (207 bytes)
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
extern "C" void free(void *);

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 0, 1, 2
class VCall { public:
    virtual void slot000();  // <-- used
    virtual void slot001();  // <-- used
    virtual void slot002();  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_0066eaf8 = (int *)0x0066EAF8;
static int *const g_0066eafc = (int *)0x0066EAFC;
static int *const g_009b3374 = (int *)0x009B3374;
// The disassembly reads [ecx-0x1c] with no matching stack slot before any
// push - a __thiscall receiver, not a __stdcall stack argument - and pops
// exactly one stack byte (`ret 4`), matching the one explicit `unsigned
// char flag` parameter. Recast as a __thiscall member; see PROPOSALS.
class VCallArg { public:
    virtual void slot000(int a);
    virtual void slot001(void *a);
    virtual void slot002(int a);
};

class Obj4e3900 {
public:
    int destroy(unsigned char flag);
};

int Obj4e3900::destroy(unsigned char flag) {
    char *ebx = reinterpret_cast<char *>(this) - 0x1c;
    char *esi = ebx;

    *reinterpret_cast<void **>(esi) = g_0066eafc;
    int *field4ptr = *reinterpret_cast<int **>(ebx + 4);
    int delta = *reinterpret_cast<int *>(reinterpret_cast<char *>(field4ptr) + 4);
    *reinterpret_cast<void **>(ebx + 4 + delta) = g_0066eaf8;

    if (*reinterpret_cast<int *>(esi + 8) != 0) {
        int count = *reinterpret_cast<int *>(esi + 0x10);
        int i = 0;
        if (count > 0) {
            do {
                char *element = *reinterpret_cast<char **>(esi + 8);
                *reinterpret_cast<int *>(esi + 0xc) = *reinterpret_cast<int *>(element + 0xc);
                void *sub = *reinterpret_cast<void **>(element + 8);
                reinterpret_cast<VCallArg *>(esi)->slot001(sub);
                if (sub != 0) {
                    reinterpret_cast<VCallArg *>(sub)->slot002(1);
                }

                char *element2 = *reinterpret_cast<char **>(esi + 8);
                *reinterpret_cast<int *>(element2 + 8) = 0;

                char *element3 = *reinterpret_cast<char **>(esi + 8);
                if (element3 != 0) {
                    int *vtbl = *reinterpret_cast<int **>(element3);
                    int offset = vtbl[1];
                    char *adjusted = element3 + offset;
                    reinterpret_cast<VCallArg *>(adjusted)->slot000(1);
                }

                char *next = *reinterpret_cast<char **>(esi + 0xc);
                *reinterpret_cast<char **>(esi + 8) = next;
                i++;
            } while (i < *reinterpret_cast<int *>(esi + 0x10));
        }
        *reinterpret_cast<int *>(esi + 8) = 0;
        *reinterpret_cast<int *>(esi + 0x14) = 0;
        *reinterpret_cast<int *>(esi + 0x10) = 0;
    }

    *reinterpret_cast<int *>(esi + 0x14) = 0;

    int val20 = *reinterpret_cast<int *>(ebx + 0x20);
    *reinterpret_cast<void **>(ebx + 0x1c) = g_006693ac;
    *g_009b3374 = val20;

    if (flag & 1) {
        if (ebx != 0) {
            if (val20 == 0) {
                free(ebx);
            }
            *g_009b3374 = 0;
        }
    }

    return reinterpret_cast<int>(ebx);
}
