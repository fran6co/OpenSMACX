// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00592250
// name           ?say_loc@@YAXPADHHHHH@Z
// size           429 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00592250/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_loc@@YAXPADHHHHH@Z  at 0x00592250  (429 bytes)
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

typedef int BOOL;
typedef void * LPVOID;
class Strings;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Strings { public:
    int8_t err_flags_;
    LPVOID base_;
    LPVOID current_;
    size_t base_size_;
    size_t free_size_;
    BOOL is_populated_;
    int get(int);
};
extern "C" char *strcat(char *, const char *);
extern "C" int __cdecl _itoa();
int __cdecl base_at(int, int);
int __cdecl base_find(int, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e90 = (int *)0x00682E90;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0097d053 = (int *)0x0097D053;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009b86a0 = (int *)0x009B86A0;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

// The scaffold's `_itoa` is nullary (a stale/generic CRT stub); the real
// CRT export takes (value, buffer, radix). extern "C" symbols are not
// mangled, so calling through a correctly-typed function pointer to the
// same symbol keeps the relocation without redeclaring it conflictingly.
typedef char *(__cdecl *ItoaFn)(int, char *, int);

// The per-location name-fragment table at 0x97D053 (stride 0x134) is
// reached with the address itself doing the work (`lea` folds the disp32
// base directly into a scale-4 SIB addressing mode) - extern array, not a
// pointer constant.
extern char g_0097d053_records[];

// NOT BYTE_EXACT. Closest reached (best of the four flag sets the harness
// tries): 393/429 bytes, mnemonic similarity 0.79, edit_count 23. The
// `/O2` flag sets inline every `strcat` call into a generic strlen+copy
// loop, which the original never does (it always emits a real `call`);
// only the `/O1` (size) flag sets keep real calls, matching the
// original's shape - `verify_recovered_function.py` already picks the
// best of the four automatically, so this is not something the body
// needs to work around. The residual gap under `/O1` is in argument
// staging at call sites (e.g. `push dword ptr [ebp+0x10]` where the
// original pre-loads into eax first) - INSTRUCTION SELECTION, not moved
// by respelling the call. Logic verified field-for-field against
// Ghidra's reconstruction, including one correction to it: `jl 0x592332`
// (`idx < 0`) does NOT return - it jumps INTO the trailer block that
// still runs the `a5 == 1` message-suffix logic regardless of `idx`, so
// this body uses `goto trailer` rather than an early `return`.
void __cdecl say_loc(char *a1, int a2, int a3, int a4, int a5, int a6) {
    char buf[80];
    int idx = base_at(a2, a3);
    if (a4 != 0) {
        if (idx < 0) {
            a6 = 0;
            idx = base_find(a2, a3, -1, -1, -1, *g_00939284);
            if (idx < 0) {
                goto trailer;
            }
            char *base = *reinterpret_cast<char **>(g_009b90f8);
            strcat(a1, reinterpret_cast<char *>(
                           reinterpret_cast<Strings *>(g_009b90d8)->get(
                               *reinterpret_cast<int *>(base + 0xf8))));
            strcat(a1, reinterpret_cast<char *>(g_00682820));
        } else {
            goto skip_first_check;
        }
    }
    if (idx < 0) {
        goto trailer;
    }
skip_first_check:
    if (a6 != 0) {
        char *base = *reinterpret_cast<char **>(g_009b90f8);
        strcat(a1, reinterpret_cast<char *>(
                       reinterpret_cast<Strings *>(g_009b90d8)->get(
                           *reinterpret_cast<int *>(base + 0x20))));
        strcat(a1, reinterpret_cast<char *>(g_00682820));
    }
    strcat(a1, g_0097d053_records + idx * 0x134);
    if (a5 != 0) {
        strcat(a1, reinterpret_cast<char *>(g_00682820));
    }

trailer:
    if (a5 == 1 || (a5 == 2 && idx < 0)) {
        strcat(a1, reinterpret_cast<char *>(g_00682e9c));
        reinterpret_cast<ItoaFn>(_itoa)(a2, buf, 10);
        strcat(a1, buf);
        strcat(a1, reinterpret_cast<char *>(g_00682e90));
        reinterpret_cast<ItoaFn>(_itoa)(a3, buf, 10);
        strcat(a1, buf);
        strcat(a1, reinterpret_cast<char *>(g_00682e98));

        int flags = *g_009a64c0;
        if ((flags & 0x40) != 0 && (flags & 0x1000) != 0) {
            strcat(reinterpret_cast<char *>(g_009b86a0), reinterpret_cast<char *>(g_00682820));
            int index = *g_0068faf0 * a3 + (a2 >> 1);
            uint8_t *tile = *reinterpret_cast<uint8_t **>(g_0094a30c) + index * 44;
            reinterpret_cast<ItoaFn>(_itoa)(tile[3], buf, 10);
            strcat(reinterpret_cast<char *>(g_009b86a0), buf);
        }
    }
}
