// ORIGINAL: 0x004B43E0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004B43E0
// name           ?say_orders@@YAXPADH@Z
// size           1421 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004b43e0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?say_orders@@YAXPADH@Z  at 0x004B43E0  (1421 bytes)
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
int __cdecl parse_says(int, char *, int, int);
int __cdecl parse_string(char *, char *);
int __cdecl resource_yield(int, int, int, int, int);
void __cdecl say_base(char *, int);
void __cdecl say_loc(char *, int, int, int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00682820 = (int *)0x00682820;
static int *const g_00682e94 = (int *)0x00682E94;
static int *const g_00682e98 = (int *)0x00682E98;
static int *const g_00682e9c = (int *)0x00682E9C;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_00945d48 = (int *)0x00945D48;
static int *const g_0094615c = (int *)0x0094615C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0096c878 = (int *)0x0096C878;
static int *const g_0096c9b0 = (int *)0x0096C9B0;
static int *const g_0097d053 = (int *)0x0097D053;
static int *const g_009ab892 = (int *)0x009AB892;
static int *const g_009b90d8 = (int *)0x009B90D8;
static int *const g_009b90f8 = (int *)0x009B90F8;

// Indexed-table bases (base+register addressing) the emitter's immediate
// scan does not catch: `extern T name[];`, not a pointer constant, per the
// measured indexed-table-base lever.
extern char g_952828x[]; // per-unit order record, stride 0x34
extern char g_97d053x[]; // per-base record, stride 0x134
extern char g_9ab892x[]; // per-terrain-type record, stride 0x34, byte at +0
extern char g_96c878x[]; // order-name string-id table, stride 3 ints (12 bytes), 2 rows
extern char g_94615cx[]; // order-suffix string-id table, stride 8 bytes (2 ints)
extern char g_945d48x[]; // string-id table, stride 4
extern char g_94a30cx[]; // per-tile record, stride 0x2c, byte at +0

// _itoa's scaffold declaration is nullary (a fallback); the real 3-argument
// call is declared here under a different name. The relocation it needs is
// masked by the comparison regardless of the symbol it names.
extern "C" char *__cdecl my_itoa(int, char *, int);

void __cdecl say_orders(char * a1, int a2) {
    char *rec = g_952828x + a2 * 0x34;
#define B(off) (*reinterpret_cast<unsigned char *>(rec + (off)))
#define W(off) (*reinterpret_cast<short *>(rec + (off)))
#define D(off) (*reinterpret_cast<unsigned int *>(rec + (off)))

    Strings *strings = reinterpret_cast<Strings *>(g_009b90d8);
    char *tbl = *reinterpret_cast<char **>(g_009b90f8);

    unsigned int flags = D(4);

    if ((flags & 0x200) != 0 && (flags & 0x1000200) != 0x1000200 &&
        ((flags & 0x2000200) != 0x2000200 || B(0x12) <= B(0x13))) {
        strcat(a1, reinterpret_cast<char *>(g_00682e9c));
        unsigned char orderType = B(0x26);
        if (orderType == 10) {
            char buf1[80];
            buf1[0] = 0;
            strcat(buf1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(tbl + 0x6f0))));
            int found = base_at(W(0x16), W(0x1e));
            if (found >= 0) {
                parse_says(0, g_97d053x + found * 0x134, -1, -1);
                char buf2[80];
                parse_string(buf1, buf2);
                strcat(a1, buf2);
            }
        } else {
            char *sid;
            if (orderType == 0xb) {
                sid = tbl + 0x6f4;
            } else if (g_9ab892x[W(0xa) * 0x34] == 9 && orderType != 0) {
                sid = tbl + 0x62c + orderType * 4;
            } else {
                sid = tbl + 0x62c;
            }
            strcat(a1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(sid))));
        }
        strcat(a1, reinterpret_cast<char *>(g_00682e98));
        strcat(a1, reinterpret_cast<char *>(g_00682820));
    }

    if ((flags & 0x4000) != 0) {
        strcat(a1, reinterpret_cast<char *>(strings->get(*g_0096c9b0)));
        return;
    }

    if ((flags & 0x1000200) == 0x1000200) {
        strcat(a1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(tbl + 0x65c))));
        strcat(a1, reinterpret_cast<char *>(g_00682e94));

        short *xyBase = reinterpret_cast<short *>(rec + 0x14);
        int count = B(0x12) + 1;
        for (int i = 0; i < count; ++i) {
            say_loc(a1, xyBase[i], xyBase[i + 4], 0, 2, 0);
            if (i < B(0x12)) {
                strcat(a1, reinterpret_cast<char *>(g_00682820));
                strcat(a1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(tbl + 0x660))));
                strcat(a1, reinterpret_cast<char *>(g_00682820));
            }
        }
        return;
    }

    {
        int tile = W(2) * (*g_0068faf0) + (W(0) >> 1);
        unsigned char terrain = static_cast<unsigned char>(g_94a30cx[tile * 0x2c] & 0xe0);
        int lowFlag = (terrain < 0x60) ? 1 : 0;
        unsigned char orderCode = B(0x11);
        int idx = lowFlag + (orderCode & 0x3f) * 3;
        strcat(a1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(g_96c878x + idx * 4))));

        if (orderCode == 3) {
            strcat(a1, reinterpret_cast<char *>(g_00682820));
            strcat(a1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(g_94615cx + B(0x26) * 8))));
            int found = base_at(W(0), W(2));
            if (found >= 0) {
                strcat(a1, reinterpret_cast<char *>(g_00682820));
                strcat(a1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(tbl + 0x6c8))));
                strcat(a1, reinterpret_cast<char *>(g_00682820));
                say_base(a1, W(0x2e));
            } else {
                strcat(a1, reinterpret_cast<char *>(g_00682e9c));
                int yield = resource_yield(B(0x26), B(0xe), W(0x2e), W(0), W(2));
                char numbuf[80];
                my_itoa(yield, numbuf, 10);
                strcat(a1, numbuf);
                strcat(a1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(tbl + 0x5ec))));
                strcat(a1, reinterpret_cast<char *>(g_00682e98));
                strcat(a1, reinterpret_cast<char *>(g_00682820));
            }
        }

        if ((B(0x11) & 0x3f) == 0x18) {
            strcat(a1, reinterpret_cast<char *>(g_00682820));
            int mode = ((flags & 0x2000200) == 0x2000200 && B(0x13) < B(0x12)) ? 2 : 1;
            say_loc(a1, W(0x14), W(0x1c), 0, mode, 0);

            if ((flags & 0x2000200) == 0x2000200 && B(0x13) < B(0x12) && B(0x12) != 0) {
                short *xyBase2 = reinterpret_cast<short *>(rec + 0x16);
                for (int i = 0; i < B(0x12); ++i) {
                    strcat(a1, reinterpret_cast<char *>(g_00682820));
                    strcat(a1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(tbl + 0x664))));
                    strcat(a1, reinterpret_cast<char *>(g_00682820));
                    say_loc(a1, xyBase2[i], xyBase2[i + 4], 0, 2, 0);
                }
            }
        }

        if (B(0x11) == 0x19) {
            strcat(a1, reinterpret_cast<char *>(g_00682820));
            strcat(a1, reinterpret_cast<char *>(strings->get(*reinterpret_cast<int *>(g_945d48x + W(0x14) * 4))));
        }

        if (B(0x11) == 0x1b || B(0x11) == 0x1c) {
            strcat(a1, reinterpret_cast<char *>(g_00682820));
            say_loc(a1, W(0x14), W(0x1c), 0, 2, 0);
        }
    }

#undef B
#undef W
#undef D
}
