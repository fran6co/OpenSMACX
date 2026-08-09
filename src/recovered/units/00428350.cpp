// ORIGINAL: 0x00428350 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00428350
// name           ?council@@YAXHHH@Z
// size           510 bytes
// measured tier  MISMATCH
// divergence     57
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00428350/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?council@@YAXHHH@Z  at 0x00428350  (510 bytes)
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

class CouncWin;

// ---- callees, declared and never defined (a definition would be inlined) ----
class CouncWin { public:
    void init(int, int);
};
int __cdecl parse_say(int, int, int, int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00683250 = (int *)0x00683250;
static int *const g_00683260 = (int *)0x00683260;
static int *const g_00683270 = (int *)0x00683270;
static int *const g_00683280 = (int *)0x00683280;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_006fec80 = (int *)0x006FEC80;
static int *const g_006ff69c = (int *)0x006FF69C;
static int *const g_006ff6a0 = (int *)0x006FF6A0;
static int *const g_00703de8 = (int *)0x00703DE8;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_009474f4 = (int *)0x009474F4;
static int *const g_00949c38 = (int *)0x00949C38;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_0096cddc = (int *)0x0096CDDC;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a6614 = (int *)0x009A6614;
static int *const g_009a6644 = (int *)0x009A6644;
static int *const g_009a6828 = (int *)0x009A6828;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// ---- extra globals: base addresses used for pointer walks / register-scaled
// indexing that the emitter does not detect on its own (see LEVERS: indexed
// table base). g_00946a50/9c/84 are per-faction embedded fields (stride
// 0x59c, same stride as the g_009474f4 faction table) that the census scan
// never saw as bare operands because they only ever appear as [reg+disp].
extern unsigned char g_009474f4_arr[];
extern unsigned char g_00949c38_mark[];
extern int g_0096cddc_arr[];
extern int g_009a6644_arr[];
extern int g_009a6828_arr[];
extern int g_0096c9e0_arr[];
extern unsigned char g_00946a50_arr[];
extern unsigned char g_00946a9c_arr[];
extern unsigned char g_00946a84_arr[];

void __cdecl council(int a1, int a2, int a3) {
    int origPop = *g_009a6614;

    int p_i = reinterpret_cast<int>(g_009474f4_arr);
    int i = 1;
    do {
        int j = 1;
        if (p_i > reinterpret_cast<int>(g_009474f4_arr)) {
            int p_j = reinterpret_cast<int>(g_009474f4_arr);
            do {
                if (!(*reinterpret_cast<unsigned char *>(p_j) & 0x80)
                    && !(*reinterpret_cast<unsigned char *>(p_i) & 0x80)) {
                    treaty_on(i, j, 8);
                }
                j++;
                p_j += 0x59c;
            } while (j < i);
        }
        p_i += 0x59c;
        i++;
    } while (p_i < reinterpret_cast<int>(g_00949c38_mark));

    int fi, fj, fk;
    if (*g_0093a95c == 0 || *g_00703de8 == 0) {
        int val = *g_009a64d4;
        if (a2 != 0) {
            g_0096cddc_arr[a3 * 2099] = val;
        }
        fi = a1;
        g_009a6644_arr[a2] = val;
        fj = a3;
        fk = a2;
    } else {
        fi = *g_00703de8;
        fj = *g_006ff6a0;
        fk = *g_006ff69c;
    }

    if (fi != *g_00939284
        || ((1 << (fi & 0x1f)) & *reinterpret_cast<unsigned char *>(g_009a64e8)) == 0) {
        int portrait = *reinterpret_cast<int *>(g_00946a50_arr + fi * 0x59c);
        *g_009bbfec = portrait;
        parse_says(0, reinterpret_cast<char *>(g_00946a9c_arr + fi * 0x59c), -1, -1);
        portrait = *reinterpret_cast<int *>(g_00946a50_arr + fi * 0x59c);
        *g_009bbfec = portrait;
        *g_009bbff0 = 0;
        parse_says(1, reinterpret_cast<char *>(g_00946a84_arr + fi * 0x59c), -1, -1);
        parse_say(2, g_009a6828_arr[fk * 3], -1, -1);
        popp(reinterpret_cast<char *>(*g_00691b0c),
             reinterpret_cast<const char *>(g_00683260), 0,
             reinterpret_cast<const char *>(g_00683250), 0);
    }

    *g_009a64c0 = *g_009a64c0 | 0x10000;
    reinterpret_cast<CouncWin *>(g_006fec80)->init(fk, fj);

    if (fk == 0
        && *g_009a6614 == *g_00939284
        && origPop != *g_00939284
        && *g_0093f660 == 0
        && (*g_0093a95c == 0 || *g_00703de8 == 0)) {
        unsigned val = static_cast<unsigned>(g_0096c9e0_arr[*g_00939284 * 2099]);
        if ((val & 0x8000) == 0) {
            g_0096c9e0_arr[*g_00939284 * 2099] = static_cast<int>(val | 0x8000);
            popp(reinterpret_cast<char *>(*g_00691b0c),
                 reinterpret_cast<const char *>(g_00683280), 0,
                 reinterpret_cast<const char *>(g_00683270), 0);
        }
    }
}
