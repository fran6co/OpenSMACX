// ORIGINAL: 0x005D44FC DirectDrawCreate 0x005D44FC-0x005D4502 FILE
// TRIED: homed into general.cpp (its real TU) 2026-09-03 and reverted for one reason:
// its fixed-address bindings have no naming evidence. image_data.py reads
// 1 of them and every one is zero at load - runtime-filled lookup tables
// with no .data content, no string, no arithmetic identity beyond 'a table
// indexed here' - and none is modelled anywhere in src. The homing-first rule
// does not ask for them to be invented: an unclaimed body with evidence-free
// bindings waits for the class pass that measures it in context.
// working copy - scaffold materialised by --work
// size      6 bytes
// prototype HRESULT (__stdcall DirectDrawCreate)(GUID* lpGUID, LPDIRECTDRAW* lplpDD, IUnknown* pUnkOuter)
// callers   3   call targets   0
// kind      thunk
// flags     hidden;thunk;sp_ready;purged_ok
// calls     (none)

// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: DirectDrawCreate  at 0x005D44FC  (6 bytes)
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

struct GUID;
typedef long HRESULT;
struct IUnknown;
typedef void * LPDIRECTDRAW;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669018 = (int *)0x00669018;

typedef HRESULT (__stdcall *DirectDrawCreateProc)(GUID*, LPDIRECTDRAW*, IUnknown*);

HRESULT __stdcall DirectDrawCreate(GUID* a1, LPDIRECTDRAW* a2, IUnknown* a3) {
    DirectDrawCreateProc fn = *reinterpret_cast<DirectDrawCreateProc *>(g_00669018);
    return fn(a1, a2, a3);
}
