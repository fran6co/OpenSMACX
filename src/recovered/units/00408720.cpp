// ORIGINAL: 0x00408720 ?psych_row@BaseWin@@QAEXHHHHHHH@Z 0x00408720-0x00408B1B;0x00651567-0x00651579 FILE
// size      1037 bytes
// prototype void (__thiscall ?psych_row@BaseWin@@QAEXHHHHHHH@Z)(BaseWin* this, int, int, int, int, int, int, int)
// callers   1   call targets   6
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E4020 0x005E4B9A 0x00625730 0x00625740 0x00625750 0x00625770
// indirect  0x00408786
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00408720
// measured tier  MISMATCH
// divergence     13
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00408720/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?psych_row@BaseWin@@QAEXHHHHHHH@Z  at 0x00408720  (1037 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

struct Buffer;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Random { public:
    Random();
    unsigned int get(int, int);
    void reseed(unsigned int);
    ~Random();
};
class Sprite { public:
    int draw(Buffer*, int, int, int);
};
int __cdecl best_specialist();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065156f = (int *)0x0065156F;
static int *const g_00669368 = (int *)0x00669368;
static int *const g_00672400 = (int *)0x00672400;
static int *const g_0075b180 = (int *)0x0075B180;
static int *const g_0075b188 = (int *)0x0075B188;
static int *const g_0075b1ac = (int *)0x0075B1AC;
static int *const g_0075b1b4 = (int *)0x0075B1B4;
static int *const g_0075b710 = (int *)0x0075B710;
static int *const g_00776420 = (int *)0x00776420;
static int *const g_0077644c = (int *)0x0077644C;
static int *const g_00779b78 = (int *)0x00779B78;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00946f58 = (int *)0x00946F58;
static int *const g_009a6488 = (int *)0x009A6488;

// PopupWaveTimeSlot's pointee type (src/popup.h): unsigned long __stdcall(void).
typedef unsigned long(__stdcall FnTimeSource)(void);

// Fresh re-reads (the original reloads *BaseCurrent at almost every use
// rather than caching it across calls) - textual macros, not helper
// functions, so nothing here gets a definition that could be inlined.
#define CUR_BASE (*reinterpret_cast<char **>(g_0090ea30))
#define POP_SIZE(b) (*reinterpret_cast<signed char *>((b) + 6))
#define FID(b) (*reinterpret_cast<unsigned char *>((b) + 4))
#define SPEC_TOTAL(b) (*reinterpret_cast<int *>((b) + 0x7c))
#define SPEC_WORD(b, idx) (reinterpret_cast<int *>((b) + 0x84)[(idx)])

class BaseWin { public:
    void psych_row(int, int, int, int, int, int, int);
};

void BaseWin::psych_row(int a1, int a2, int a3, int a4, int a5, int a6, int a7) {
    Random rng;

    int pop = POP_SIZE(CUR_BASE);
    int spacing;
    if (pop <= 1) {
        spacing = a4;
    } else {
        int step = (a3 - a4) / (pop - 1);
        if (step < 1 || a4 < 1) {
            spacing = 1;
        } else if (step > a4) {
            spacing = a4;
        } else {
            spacing = step;
        }
    }

    rng.reseed((*reinterpret_cast<FnTimeSource **>(g_00669368))());

    if (*g_009a6488) {
        if (*(reinterpret_cast<unsigned char *>(g_00946f58) + FID(CUR_BASE) * 0x59C) & 0x80) {
            // ---- PART A: expansion enabled and faction flag bit set ----
            int x;

            x = a1;
            if (a5 > 0) {
                int n1 = a5;
                do {
                    int buf;
                    if (reinterpret_cast<char *>(this)) {
                        buf = reinterpret_cast<int>(reinterpret_cast<char *>(this) + 0x444);
                    } else {
                        buf = 0;
                    }
                    reinterpret_cast<Sprite *>(g_0075b180)->draw(
                        reinterpret_cast<Buffer *>(buf),
                        *g_0075b188 & 0xff,
                        x, a2);
                    x += spacing;
                    n1--;
                } while (n1 != 0);
            }

            int i1 = 0;
            if (POP_SIZE(CUR_BASE) - SPEC_TOTAL(CUR_BASE) - a5 - a6 > 0) {
                do {
                    int buf;
                    if (reinterpret_cast<char *>(this)) {
                        buf = reinterpret_cast<int>(reinterpret_cast<char *>(this) + 0x444);
                    } else {
                        buf = 0;
                    }
                    reinterpret_cast<Sprite *>(g_0075b1ac)->draw(
                        reinterpret_cast<Buffer *>(buf),
                        *g_0075b1b4 & 0xff,
                        x, a2);
                    x += spacing;
                    i1++;
                } while (i1 < POP_SIZE(CUR_BASE) - SPEC_TOTAL(CUR_BASE) - a5 - a6);
            }

            int i2 = 0;
            if (a6 > 0) {
                a5 = a6 - a7;
                do {
                    int buf;
                    if (reinterpret_cast<char *>(this)) {
                        buf = reinterpret_cast<int>(reinterpret_cast<char *>(this) + 0x444);
                    } else {
                        buf = 0;
                    }
                    int mood = 2 + (i2 >= a5);
                    char *sp = reinterpret_cast<char *>(g_0075b180) + mood * 0x2c;
                    reinterpret_cast<Sprite *>(sp)->draw(
                        reinterpret_cast<Buffer *>(buf),
                        *reinterpret_cast<unsigned char *>(sp + 8),
                        x, a2);
                    x += spacing;
                    i2++;
                } while (i2 < a6);
            }

            int i3 = 0;
            if (SPEC_TOTAL(CUR_BASE) > 0) {
                do {
                    if (reinterpret_cast<char *>(this)) {
                        a6 = reinterpret_cast<int>(reinterpret_cast<char *>(this) + 0x444);
                    } else {
                        a6 = 0;
                    }
                    int specType;
                    if (i3 < 16) {
                        specType = (SPEC_WORD(CUR_BASE, i3 >> 3) >> ((i3 & 7) * 4)) & 0xf;
                    } else {
                        specType = best_specialist();
                    }
                    char *sp = reinterpret_cast<char *>(g_0075b710) + specType * 0x2c;
                    reinterpret_cast<Sprite *>(sp)->draw(
                        reinterpret_cast<Buffer *>(a6),
                        *reinterpret_cast<unsigned char *>(sp + 8),
                        x, a2);
                    x += spacing;
                    i3++;
                } while (i3 < SPEC_TOTAL(CUR_BASE));
            }
            goto done;
        }
    }
    {
        // ---- PART B: not expansion, or flag bit clear ----
        int x;

        if (a5 > 0) {
            x = a1;
            a4 = a5;
            do {
                int buf;
                if (reinterpret_cast<char *>(this)) {
                    buf = reinterpret_cast<int>(reinterpret_cast<char *>(this) + 0x444);
                } else {
                    buf = 0;
                }
                unsigned int frame = rng.get(0, 2);
                char *sp = reinterpret_cast<char *>(g_00776420) + frame * 0xb0;
                reinterpret_cast<Sprite *>(sp)->draw(
                    reinterpret_cast<Buffer *>(buf),
                    *reinterpret_cast<unsigned char *>(sp + 8),
                    x, a2);
                x += spacing;
                a4--;
            } while (a4 != 0);
        } else {
            x = a1;
        }

        int i4 = 0;
        if (POP_SIZE(CUR_BASE) - SPEC_TOTAL(CUR_BASE) - a5 - a6 > 0) {
            do {
                if (reinterpret_cast<char *>(this)) {
                    a1 = reinterpret_cast<int>(reinterpret_cast<char *>(this) + 0x444);
                } else {
                    a1 = 0;
                }
                unsigned int frame = rng.get(0, 2);
                char *sp = reinterpret_cast<char *>(g_0077644c) + frame * 0xb0;
                reinterpret_cast<Sprite *>(sp)->draw(
                    reinterpret_cast<Buffer *>(a1),
                    *reinterpret_cast<unsigned char *>(sp + 8),
                    x, a2);
                x += spacing;
                i4++;
            } while (i4 < POP_SIZE(CUR_BASE) - SPEC_TOTAL(CUR_BASE) - a5 - a6);
        }

        int i5 = 0;
        if (a6 > 0) {
            a5 = a6 - a7;
            do {
                if (reinterpret_cast<char *>(this)) {
                    a1 = reinterpret_cast<int>(reinterpret_cast<char *>(this) + 0x444);
                } else {
                    a1 = 0;
                }
                unsigned int frame = rng.get(0, 2);
                int mood = 2 + (i5 >= a5);
                char *sp = reinterpret_cast<char *>(g_00776420) + (mood + frame * 4) * 0x2c;
                reinterpret_cast<Sprite *>(sp)->draw(
                    reinterpret_cast<Buffer *>(a1),
                    *reinterpret_cast<unsigned char *>(sp + 8),
                    x, a2);
                x += spacing;
                i5++;
            } while (i5 < a6);
        }

        int i6 = 0;
        if (SPEC_TOTAL(CUR_BASE) > 0) {
            do {
                if (reinterpret_cast<char *>(this)) {
                    a6 = reinterpret_cast<int>(reinterpret_cast<char *>(this) + 0x444);
                } else {
                    a6 = 0;
                }
                int specType;
                if (i6 < 16) {
                    specType = (SPEC_WORD(CUR_BASE, i6 >> 3) >> ((i6 & 7) * 4)) & 0xf;
                } else {
                    specType = best_specialist();
                }
                char *sp = reinterpret_cast<char *>(g_00779b78) + specType * 0x2c;
                reinterpret_cast<Sprite *>(sp)->draw(
                    reinterpret_cast<Buffer *>(a6),
                    *reinterpret_cast<unsigned char *>(sp + 8),
                    x, a2);
                x += spacing;
                i6++;
            } while (i6 < SPEC_TOTAL(CUR_BASE));
        }
    }
done:
    ;
}
