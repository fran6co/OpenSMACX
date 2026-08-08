// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00402D10
// name           sub_402d10
// size           108 bytes
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00402d10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_402d10  at 0x00402D10  (108 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006693ac = (int *)0x006693AC;
static int *const g_006693f0 = (int *)0x006693F0;
static int *const g_006693f4 = (int *)0x006693F4;
static int *const g_00669418 = (int *)0x00669418;
static int *const g_0066941c = (int *)0x0066941C;
static int *const g_00669424 = (int *)0x00669424;
static int *const g_009b3374 = (int *)0x009B3374;

extern "C" int __stdcall sub_402d10(int a1) {
    int t;
    __asm {
        mov     edx, a1
        mov     eax, ecx
        xor     ecx, ecx
        cmp     edx, ecx
        mov     t, ecx
        je      SHORT LSkip
        mov     dword ptr [eax+4], 669424h
        mov     dword ptr [eax+24h], 6693ACh
        mov     edx, dword ptr ds:[9B3374h]
        mov     dword ptr [eax+28h], edx
        mov     dword ptr ds:[9B3374h], ecx
LSkip:
        mov     edx, dword ptr [eax+4]
        mov     dword ptr [eax], 6693F4h
        mov     edx, dword ptr [edx+4]
        mov     dword ptr [edx+eax+4], 6693F0h
        mov     dword ptr [eax+8], ecx
        mov     dword ptr [eax+0Ch], ecx
        mov     dword ptr [eax+10h], ecx
        mov     dword ptr [eax+14h], ecx
        mov     dword ptr [eax+18h], ecx
        mov     ecx, dword ptr [eax+4]
        mov     dword ptr [eax], 66941Ch
        mov     edx, dword ptr [ecx+4]
        mov     dword ptr [edx+eax+4], 669418h
    }
}
