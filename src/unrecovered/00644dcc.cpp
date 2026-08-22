// ORIGINAL: 0x00644DCC ICOpen 0x00644DCC-0x00644DD2 FILE
// TRIED: indirect call through a typed function-pointer global (plain deref, extern-declared-slot, __declspec(dllimport)) all lower to push/push/push/call/ret, not the original's bare indirect jmp - VC6 does not tail-call an INDIRECT stdcall callee here
// working copy - scaffold materialised by --work
// size      6 bytes
// prototype HIC (__stdcall ICOpen)(DWORD fccType, DWORD fccHandler, UINT wMode)
// callers   1   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ICOpen  at 0x00644DCC  (6 bytes)
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

typedef unsigned long DWORD;
typedef void * HIC;
typedef unsigned int UINT;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669258 = (int *)0x00669258;
typedef HIC (__stdcall *PFN_ICOpen)(DWORD, DWORD, UINT);

extern "C" HIC __stdcall ICOpen(DWORD a1, DWORD a2, UINT a3) {
    return ((PFN_ICOpen)*g_00669258)(a1, a2, a3);
}
