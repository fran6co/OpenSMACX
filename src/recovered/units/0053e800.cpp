// ORIGINAL: 0x0053E800 ?tech_analysis@@YAXHH@Z 0x0053E800-0x0053EB42 FILE
// size      834 bytes
// prototype 
// callers   7   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20 0x005BCBE0 0x00645660
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0053E800
// measured tier  MISMATCH
// divergence     3
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0053e800/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?tech_analysis@@YAXHH@Z  at 0x0053E800  (834 bytes)
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
bool __cdecl has_tech(int, int);
extern "C" int strcmp(const char *, const char *);
int __cdecl tech_val(int, int, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068d6a4 = (int *)0x0068D6A4;
static int *const g_0093f660 = (int *)0x0093F660;
static int *const g_0093f7c0 = (int *)0x0093F7C0;
static int *const g_0093f7ec = (int *)0x0093F7EC;
static int *const g_0093f7f4 = (int *)0x0093F7F4;
static int *const g_0093f800 = (int *)0x0093F800;
static int *const g_0093f80c = (int *)0x0093F80C;
static int *const g_0093f814 = (int *)0x0093F814;
static int *const g_0093fa18 = (int *)0x0093FA18;
static int *const g_0093fa1c = (int *)0x0093FA1C;
static int *const g_0093fa20 = (int *)0x0093FA20;
static int *const g_0093fa28 = (int *)0x0093FA28;
static int *const g_0093fa44 = (int *)0x0093FA44;
static int *const g_0093fa6c = (int *)0x0093FA6C;
static int *const g_0093fa84 = (int *)0x0093FA84;
static int *const g_0093faa4 = (int *)0x0093FAA4;
static int *const g_0093faa8 = (int *)0x0093FAA8;
static int *const g_00946a54 = (int *)0x00946A54;
static int *const g_0096c9e8 = (int *)0x0096C9E8;
static int *const g_0097d090 = (int *)0x0097D090;
static int *const g_009a589c = (int *)0x009A589C;
static int *const g_009a649c = (int *)0x009A649C;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a650c = (int *)0x009A650C;
static int *const g_009a6510 = (int *)0x009A6510;
static int *const g_009a6514 = (int *)0x009A6514;

extern int g_0096c9e8arr[];
extern char g_00946a54arr[];

void __cdecl tech_analysis(int a1, int a2) {
    *g_0093f7f4 = -1;
    *g_0093fa28 = -1;
    *g_0093fa1c = -1;
    *g_0093fa18 = -1;
    *g_0093faa8 = -1;
    *g_0093f80c = -1;
    *g_0093f800 = -1;
    *g_0093f7ec = 0;
    *g_0093fa44 = 0;
    *g_0093faa4 = 0;
    *g_0093f814 = 0;
    *g_0093f7c0 = 0;
    *g_0093fa20 = 0;
    *g_0093fa84 = -1;
    *g_0093fa6c = -1;

    if ((*g_009a649c & 0x2000000) == 0) {
        for (int i = 0; i < 0x59; i++) {
            if (!has_tech(i, a2)) {
                if (has_tech(i, a1)) {
                    int v = tech_val(i, a2, 0);
                    int oldA20 = *g_0093fa20;
                    int oldA814 = *g_0093f814;
                    if (v < *g_0093f814) {
                        if (v < *g_0093fa44) {
                            if (v < *g_0093fa20) {
                                if (*g_0093f7c0 <= v) {
                                    *g_0093f7c0 = v;
                                    *g_0093fa28 = i;
                                }
                            } else {
                                *g_0093f7c0 = *g_0093fa20;
                                *g_0093fa28 = *g_0093fa1c;
                                *g_0093fa1c = i;
                                *g_0093fa20 = v;
                            }
                        } else {
                            *g_0093fa20 = *g_0093fa44;
                            *g_0093f7c0 = oldA20;
                            *g_0093fa28 = *g_0093fa1c;
                            *g_0093fa1c = *g_0093fa18;
                            *g_0093fa18 = i;
                            *g_0093fa44 = v;
                        }
                    } else {
                        *g_0093fa28 = *g_0093fa1c;
                        *g_0093fa1c = *g_0093fa18;
                        *g_0093fa18 = *g_0093faa8;
                        *g_0093f7c0 = *g_0093fa20;
                        *g_0093f814 = v;
                        *g_0093fa20 = *g_0093fa44;
                        *g_0093fa44 = oldA814;
                        *g_0093faa8 = i;
                    }
                }
            } else if (!has_tech(i, a1)) {
                int matched = -1;
                if (g_0096c9e8arr[a1 * 0x833] > 2 ||
                    strcmp((const char *)(g_00946a54arr + a2 * 0x59c), (const char *)g_0068d6a4) == 0) {
                    int j = 0;
                    int rank = -2;
                    int *p1 = g_009a6514;
                    int *p2 = g_009a589c;
                    do {
                        if (*p1 == -1 && *p2 == i) {
                            if (*g_009a64cc > 0) {
                                int *rec = g_0097d090;
                                int k = *g_009a64cc;
                                do {
                                    if (*((unsigned char *)rec - 0x4C) == a2 && *rec == rank - 0x44) {
                                        *g_0093f7f4 = j;
                                        matched = j;
                                    }
                                    rec = (int *)((char *)rec + 0x134);
                                    k--;
                                } while (k != 0);
                            }
                            if (((g_0096c9e8arr[a1 * 0x833] > 4 && (*g_0093f660 == 0 || a1 != *g_009a6510)) ||
                                 (*g_0093f660 != 0 && a1 == *g_009a650c)) &&
                                *g_0093f7f4 == -1) {
                                *g_0093f7f4 = rank;
                                matched = rank;
                            }
                        }
                        j++;
                        p2 = (int *)((char *)p2 + 0xc);
                        p1++;
                        rank--;
                    } while (p2 < g_009a649c);
                }
                int v = tech_val(i, a1, 0);
                if (matched != -1) {
                    v = (v + 1) / 2;
                }
                if (v < *g_0093faa4) {
                    if (*g_0093f7ec <= v) {
                        *g_0093f7ec = v;
                        *g_0093f80c = i;
                        *g_0093fa84 = matched;
                    }
                } else {
                    *g_0093f7ec = *g_0093faa4;
                    *g_0093fa84 = *g_0093fa6c;
                    *g_0093f80c = *g_0093f800;
                    *g_0093f800 = i;
                    *g_0093fa6c = matched;
                    *g_0093faa4 = v;
                }
            }
        }
        if (*g_0093fa84 != -1) {
            *g_0093f80c = -1;
        }
    }
}
