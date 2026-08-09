// ORIGINAL: 0x0055CE20 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0055CE20
// name           ?enemies_unpact@@YAXHH@Z
// size           529 bytes
// measured tier  MISMATCH
// divergence     24
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0055ce20/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?enemies_unpact@@YAXHH@Z  at 0x0055CE20  (529 bytes)
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

class NetMsg;

// ---- callees, declared and never defined (a definition would be inlined) ----
class NetMsg { public:
    void pop(const char *, int, int, const char *);
};
char * __cdecl get_pact_hood(int, int);
int __cdecl parse_says(int, char *, int, int);
void __cdecl pact_unpact(int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068e610 = (int *)0x0068E610;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a6524 = (int *)0x009A6524;
static int *const g_009a6614 = (int *)0x009A6614;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// The emitter's fixed-address globals for these three tables are plain
// pointer constants, but every reference here indexes them by a register
// (`[eax*4 + 0x96c9f8]`, `[edx*4 + 0x97d044]`, `[eax + 0x946d4c]` with eax a
// scaled index) - a base-immediate/scaled-index form the emitter does not
// detect, and one `/O2` would fold away if spelled as the constant pointer.
// Declared fresh as arrays instead, per the working lever.
extern unsigned int g_96c9f8_arr[];
extern unsigned char g_97d044_arr[];
extern unsigned char g_946d34_arr[];

void __cdecl enemies_unpact(int a1, int a2) {
    int idx1 = a1 * 0x833 + a2;
    if ((g_96c9f8_arr[idx1] & 1) != 0) {
        int civ = *g_00939284;
        int argA = a1, argB = a2;
        if ((g_96c9f8_arr[a1 * 0x833 + civ] & 1) != 0) {
            pact_unpact(argA, argB);
        } else {
            argA = a2;
            argB = a1;
            if ((g_96c9f8_arr[civ + a2 * 0x833] & 1) != 0) {
                pact_unpact(argA, argB);
            }
        }

        civ = *g_00939284;
        if ((g_96c9f8_arr[civ * 0x833 + a1] & 0x1000) != 0) {
            goto notify;
        }
        if (*g_009a6524 >= 0 && g_97d044_arr[*g_009a6524 * 0x134] == civ) {
            goto notify;
        }
        if (civ == *g_009a6614 &&
            (g_946d34_arr[a1 * 0x59c + 0x224] & 0x80) == 0) {
            goto notify;
        }
        if ((g_96c9f8_arr[civ * 0x833 + a2] & 0x1000) != 0) {
            goto notify;
        }
        if (*g_009a6524 >= 0 && g_97d044_arr[*g_009a6524 * 0x134] == civ) {
            goto notify;
        }
        if (civ == *g_009a6614 &&
            (g_946d34_arr[a2 * 0x59c + 0x224] & 0x80) == 0) {
            goto notify;
        }
        if ((*reinterpret_cast<unsigned char *>(g_009a64c0) & 0x80) == 0) {
            goto skip_notify;
        }

    notify:
        *g_009bbfec = *reinterpret_cast<int *>(g_946d34_arr + a1 * 0x59c + 0x18);
        *g_009bbff0 = *reinterpret_cast<int *>(g_946d34_arr + a1 * 0x59c + 0x1c);
        parse_says(0, reinterpret_cast<char *>(g_946d34_arr + a1 * 0x59c), -1, -1);

        *g_009bbfec = *reinterpret_cast<int *>(g_946d34_arr + a2 * 0x59c + 0x18);
        *g_009bbff0 = *reinterpret_cast<int *>(g_946d34_arr + a2 * 0x59c + 0x1c);
        parse_says(1, reinterpret_cast<char *>(g_946d34_arr + a2 * 0x59c), -1, -1);

        parse_says(2, get_pact_hood(a1, a2), -1, -1);

        reinterpret_cast<NetMsg *>(g_00805338)->pop(
            reinterpret_cast<const char *>(g_0068e610), 5000, 0, 0);

    skip_notify:
        g_96c9f8_arr[idx1] &= 0xfeeffffe;
        g_96c9f8_arr[a1 + a2 * 0x833] &= 0xfeeffffe;
    }
}
