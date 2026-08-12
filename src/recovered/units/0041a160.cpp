// ORIGINAL: 0x0041A160 FILE
// name      ?next@BaseWin@@QAEXXZ
// size      333 bytes
// spans     0x0041A160-0x0041A2AD
// prototype void (__thiscall ?next@BaseWin@@QAEXXZ)(BaseWin* this)
// callers   2   call targets   4
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0041A920 0x004E39D0 0x005BF310 0x0064FD20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0041A160
// measured tier  MISMATCH
// divergence     30
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0041a160/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?next@BaseWin@@QAEXXZ  at 0x0041A160  (333 bytes)
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

class BaseWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int __cdecl _strcmpi();
int __cdecl X_pop(const char *, int (__cdecl *)());
void __cdecl set_base(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682c64 = (int *)0x00682C64;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a938 = (int *)0x0093A938;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64cc = (int *)0x009A64CC;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void zoom(int, int);
    void next();
};

// The scaffolding's `_strcmpi()` takes no arguments (a fallback), so the
// real 2-argument call is declared here under a different name; the
// relocation it needs is masked by the comparison regardless of the
// symbol it names.
extern "C" int __cdecl my_strcmpi(const char *, const char *);

// g_0097d044/g_0097d053 are indexed table bases (a per-record flag byte
// and name string, stride 0x134, offset by a runtime byte counter), so
// they need the extern-array form rather than a pointer constant.
extern unsigned char g_0097d044_arr[];
extern char g_0097d053_arr[];

void BaseWin::next() {
    char *self = reinterpret_cast<char *>(this);
    int *cur_idx = reinterpret_cast<int *>(self + 0x40b0c);

    set_base(*cur_idx);

    char *ptr = reinterpret_cast<char *>(*g_0090ea30);
    if (*reinterpret_cast<unsigned char *>(ptr + 4) == *g_00939284
        || (*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) != 0) {
        if (*g_0093a938 != 0) {
            X_pop(reinterpret_cast<const char *>(g_00682c64), 0);
            return;
        }

        int result = *cur_idx;
        int best_c = -1;
        if (*g_009a64cc > 0) {
            int best_lt_off = -0x134;
            int best_ge_off = *cur_idx * 0x134;
            int i = 0;
            int off = 0;
            do {
                if (i != *cur_idx) {
                    char *name = g_0097d053_arr + off;
                    int cand_off = *cur_idx * 0x134;
                    if (g_0097d044_arr[off] == g_0097d044_arr[cand_off]) {
                        char *cand_name = g_0097d053_arr + cand_off;
                        int cmp1 = my_strcmpi(name, cand_name);
                        if (cmp1 > 0
                            && (best_c < 0
                                || my_strcmpi(name, g_0097d053_arr + best_lt_off) < 0)) {
                            best_lt_off = off;
                            best_c = i;
                        }
                        if (my_strcmpi(name, g_0097d053_arr + best_ge_off) < 0) {
                            best_ge_off = off;
                            result = i;
                        }
                    }
                }
                i++;
                off += 0x134;
            } while (i < *g_009a64cc);

            if (best_c >= 0) {
                result = best_c;
            }
        }

        *cur_idx = result;
        zoom(result, 0);
    }
}
