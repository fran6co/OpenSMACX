// ORIGINAL: 0x0055C430 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0055C430
// name           ?comm_check@@YAXHH@Z
// size           758 bytes
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0055c430/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?comm_check@@YAXHH@Z  at 0x0055C430  (758 bytes)
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
int __cdecl parse_says(int, char *, int, int);
void __cdecl communicate(int, int, int);
void __cdecl net_treaty_on(int, int, int, int);
void __cdecl treaty_on(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068e5c4 = (int *)0x0068E5C4;
static int *const g_0068e5cc = (int *)0x0068E5CC;
static int *const g_00805338 = (int *)0x00805338;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_0093a95c = (int *)0x0093A95C;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f678 = (int *)0x0093F678;
static int *const g_0093f698 = (int *)0x0093F698;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

void __cdecl comm_check(int a1, int a2) {
    int flags = *((int *)0x9a64e8) & 0xff;
    int edx = 1 << a1;

    if ((flags & edx) == 0) {
        return;
    }

    int offset = (a1 << 6) + a1 + (a1 + a2) * 2 * 8;
    int idx = offset * 4;

    int dword_val = *((int *)(idx + 0x96c9f8));
    int edx_val = dword_val & 8;

    if ((edx_val != 0) && ((dword_val & 0x2000) != 0)) {
        int time_val = *((int *)(idx + 0x96ca58));
        if (time_val >= 0) {
            int time_diff = *((int *)0x9a64d4) - time_val;
            if (time_diff < 0x10) {
                return;
            }
        }
    }

    int ecx = *((int *)0x93f660);
    if (ecx == 0) {
        return;
    }

    if (a1 == *((int *)0x939284)) {
        int edx2 = 1 << a2;
        if ((flags & edx2) == 0) {
            int val = *((int *)(a2 * 4 + 0x93f678));
            val |= 1;
            *((int *)(a2 * 4 + 0x93f678)) = val;

            if (edx_val == 0) {
                val |= 2;
                *((int *)(a2 * 4 + 0x93f678)) = val;
                net_treaty_on(a1, a2, 0x10004, 0);
                return;
            }
        }
    }

    int eax = 1 << a2;
    if ((flags & eax) != 0 && edx_val == 0) {
        treaty_on(a1, a2, 0x10004);
    }

    if (a1 == *((int *)0x939284) && ((*((int *)(idx + 0x96c9f8)) & 0x8000000) == 0)) {
        int base_idx = ((a1 + a1 * 4) * 9 - a1) * 4;
        *((int *)0x9bbff0) = 0;
        *((int *)0x9bbfec) = *((int *)(base_idx + 0x946a50));
        parse_says(0, (char *)(base_idx + 0x946a9c), -1, -1);

        *((int *)0x9bbfec) = *((int *)(base_idx + 0x946a50));
        *((int *)0x9bbff0) = 0;
        parse_says(1, (char *)(base_idx + 0x946a84), -1, -1);

        *((int *)0x9bbfec) = *((int *)(base_idx + 0x946d4c));
        *((int *)0x9bbff0) = *((int *)(base_idx + 0x946d50));
        parse_says(2, (char *)(base_idx + 0x946d34), -1, -1);
    }

    if (a2 == *((int *)0x939284) && ((*((int *)(idx + 0x96c9f8)) & 0x8000000) == 0)) {
        int base_idx2 = ((a2 + a2 * 4) * 9 - a2) * 4;
        *((int *)0x9bbff0) = 0;
        *((int *)0x9bbfec) = *((int *)(base_idx2 + 0x946a50));
        parse_says(0, (char *)(base_idx2 + 0x946a9c), -1, -1);

        *((int *)0x9bbfec) = *((int *)(base_idx2 + 0x946a50));
        *((int *)0x9bbff0) = 0;
        parse_says(1, (char *)(base_idx2 + 0x946a84), -1, -1);

        *((int *)0x9bbfec) = *((int *)(base_idx2 + 0x946d4c));
        *((int *)0x9bbff0) = *((int *)(base_idx2 + 0x946d50));
        parse_says(2, (char *)(base_idx2 + 0x946d34), -1, -1);
    }
}
