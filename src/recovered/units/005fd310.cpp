// ORIGINAL: 0x005FD310 FILE
// name      sub_5fd310
// size      77 bytes
// spans     0x005FD310-0x005FD35D
// prototype 
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0062D5D0
// indirect  0x005FD33B 0x005FD346 0x005FD34D
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005FD310
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fd310/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_5fd310  at 0x005FD310  (77 bytes)
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
void __cdecl check_net();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669340 = (int *)0x00669340;
static int *const g_00669358 = (int *)0x00669358;
static int *const g_0066935c = (int *)0x0066935C;

// NOT BYTE_EXACT. Closest reached: 77/77 bytes (same total size),
// mnemonic similarity 0.857, edit_count 6 (verify_recovered_function.py
// --json). Return type changed void<-int: the original's final
// instruction is a bare `ret` with eax never set on any path, matching
// Ghidra's `void FUN_005fd310(void)`, not the int the scaffold defaults
// to - an `int` return added a trailing `xor eax,eax` the original does
// not have.
//
// Remaining divergence: the original caches all three IAT reads
// (PeekMessageA/TranslateMessage/DispatchMessageA) into ebx/esi/edi
// ONCE before the loop and never sets up an ebp frame (pure
// `sub esp,0x1c` / `pop;pop;pop;add esp` in/out); this body reproduces
// the same three-register caching (declaring the three function
// pointers as locals is what gets the compiler to hoist the IAT reads
// out of the loop at all - without them, at this optimization level,
// the IAT is re-read every iteration instead, ruled out below) but the
// compiler here additionally spends a fourth callee-saved slot (ebp)
// caching the address of `msg`, which forces a real `push ebp; mov
// ebp,esp` / `leave` frame the original does not have. The original
// instead recomputes `lea reg,[esp+K]` for &msg at each of the three
// call sites independently (three different literal displacements,
// all resolving to the same address as the stack depth changes) rather
// than hoisting that address into a register - this reads as an
// optimizer CSE-aggressiveness difference on `&msg` specifically, not
// a wrong body.
//
// Ruled out: dereferencing the three IAT globals inline at each call
// site with no named locals - compiles smaller (64 bytes) because the
// compiler no longer hoists the IAT reads out of the loop at all,
// which is further from the original than this version.
struct MSG {
    void *hwnd;
    unsigned int message;
    unsigned int wParam;
    long lParam;
    unsigned long time;
    long pt_x;
    long pt_y;
};

typedef int (__stdcall *PeekMessageAFn)(MSG *, void *, unsigned int, unsigned int, unsigned int);
typedef int (__stdcall *TranslateMessageFn)(const MSG *);
typedef long (__stdcall *DispatchMessageAFn)(const MSG *);

extern "C" void __cdecl sub_5fd310() {
    DispatchMessageAFn dispatchMessage = reinterpret_cast<DispatchMessageAFn>(*g_00669340);
    PeekMessageAFn peekMessage = reinterpret_cast<PeekMessageAFn>(*g_00669358);
    TranslateMessageFn translateMessage = reinterpret_cast<TranslateMessageFn>(*g_0066935c);
    MSG msg;
    while (peekMessage(&msg, 0, 0x401, 0x401, 1)) {
        translateMessage(&msg);
        dispatchMessage(&msg);
        check_net();
    }
}

