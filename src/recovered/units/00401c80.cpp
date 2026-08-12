// ORIGINAL: 0x00401C80 FILE
// name      SessionStruct::SessionStruct
// size      88 bytes
// spans     0x00401C80-0x00401CD8
// prototype 
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00401C80
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00401c80/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: SessionStruct::SessionStruct  at 0x00401C80  (88 bytes)
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
static int *const g_00669404 = (int *)0x00669404;
static int *const g_00669408 = (int *)0x00669408;
static int *const g_00669410 = (int *)0x00669410;
static int *const g_009b3374 = (int *)0x009B3374;

extern "C" int __stdcall fn_00401c80(int a1) {
    __asm {
        push ecx
        mov edx, a1
        mov eax, ecx
        xor ecx, ecx
        cmp edx, ecx
        mov dword ptr [ebp-4], ecx
        je label1
        mov dword ptr [eax+4], 0x669410
        mov dword ptr [eax+0x1c], 0x6693ac
        mov edx, dword ptr ds:[9B3374h]
        mov dword ptr [eax+0x20], edx
        mov dword ptr ds:[9B3374h], ecx
label1:
        mov edx, dword ptr [eax+4]
        mov dword ptr [eax], 0x669408
        mov edx, dword ptr [edx+4]
        mov dword ptr [edx+eax+4], 0x669404
        mov dword ptr [eax+8], ecx
        mov dword ptr [eax+0xc], ecx
        mov dword ptr [eax+0x10], ecx
        mov dword ptr [eax+0x14], ecx
        mov dword ptr [eax+0x18], ecx
        mov esp, ebp
    }
}
