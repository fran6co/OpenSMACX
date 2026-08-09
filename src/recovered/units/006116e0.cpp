// ORIGINAL: 0x006116E0 FILE
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x006116E0
// name           sub_6116e0
// size           72 bytes
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006116e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6116e0  at 0x006116E0  (72 bytes)
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
extern "C" int __cdecl sub_611730();
extern "C" void free(void *);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_009b3374 = (int *)0x009B3374;

// reads [ecx-0x28] with no stack access before any call - the receiver is
// `this`, so this is a __thiscall member, not a free function.
class Handle { public:
    void prep();  // sub_611730, thiscall on the same object
    int detach(char a1);
};

int Handle::detach(char a1) {
    // `mov al, byte ptr [esp+0xc]` reads a single byte off the stack, not a
    // full dword - the parameter is a byte-sized type, not `int`.
    char *outer = reinterpret_cast<char *>(this) - 0x28;
    Handle *self_ptr = reinterpret_cast<Handle *>(outer + 0x28);
    self_ptr->prep();
    int *self = reinterpret_cast<int *>(self_ptr);
    self[0] = reinterpret_cast<int>(g_006693ac);
    int val = self[1];
    *g_009b3374 = val;
    if (a1 & 1) {
        if (outer != 0) {
            if (val == 0) {
                free(outer);
            }
            *g_009b3374 = 0;
        }
    }
    return reinterpret_cast<int>(outer);
}
