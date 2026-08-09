// ORIGINAL: 0x00416DE0 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00416DE0
// name           ?add_queue@BaseWin@@QAEXHH@Z
// size           456 bytes
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00416de0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?add_queue@BaseWin@@QAEXHH@Z  at 0x00416DE0  (456 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0090ea30 = (int *)0x0090EA30;

class BaseWin { public:
    uint8_t pad_0_[0x40B10];
    uint32_t field_40b10_;

    void draw_production(int);
    void draw_queue(int);
    void add_queue(int, int);
};

// The production-queue table is indexed by several variable offsets below
// (`+idx*4`), so the base needs the extern-symbol lever, not the pointer
// constant above.
extern char g_0090ea30_ext[];

void BaseWin::add_queue(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    char *tbl = g_0090ea30_ext;
    int idx = *reinterpret_cast<int *>(self + 0x40B2C);

    if (idx == 0 || idx == 9) {
        return;
    }
    if (a1 == 0 &&
        *reinterpret_cast<int *>(tbl + 0x50 + idx * 4) == a2 &&
        *reinterpret_cast<int *>(tbl + 0x4C) >= idx) {
        return;
    }

    if (a2 < 0 && (-a2 < 0x41 || (a2 != -0x44 && -a2 > 0x43))) {
        int slot = 1;
        char *p = tbl + 0x54;
        while (slot < 9) {
            if (*reinterpret_cast<int *>(p) == a2) {
                if (slot < 9) {
                    if (slot < idx) {
                        *reinterpret_cast<int *>(self + 0x40B2C) = idx - 1;
                    }
                    *reinterpret_cast<int *>(tbl + 0x4C) -= 1;
                    if (slot < 8) {
                        int off = slot * 4 + 0x50;
                        do {
                            int next = off + 4;
                            *reinterpret_cast<int *>(tbl + off) =
                                *reinterpret_cast<int *>(tbl + next);
                            off = next;
                        } while (off < 0x70);
                    }
                }
                break;
            }
            ++slot;
            p += 4;
        }
    }

    if (a1 != 0) {
        int end = 9;
        if (*reinterpret_cast<int *>(self + 0x40B2C) < 9) {
            int off = 0x74;
            do {
                --end;
                *reinterpret_cast<int *>(tbl + off) =
                    *reinterpret_cast<int *>(tbl + off - 4);
                off -= 4;
            } while (*reinterpret_cast<int *>(self + 0x40B2C) < end);
        }
        if (*reinterpret_cast<int *>(tbl + 0x4C) < 9) {
            *reinterpret_cast<int *>(tbl + 0x4C) += 1;
        }
    }

    idx = *reinterpret_cast<int *>(self + 0x40B2C);
    *reinterpret_cast<int *>(tbl + 0x50 + idx * 4) = a2;
    if (*reinterpret_cast<int *>(tbl + 0x4C) < idx) {
        *reinterpret_cast<int *>(tbl + 0x4C) += 1;
    }

    int i = 0;
    if (a1 != 0) {
        int count0 = *reinterpret_cast<int *>(tbl + 0x4C);
        *reinterpret_cast<int *>(self + 0x40B30) = 0;
        *reinterpret_cast<int *>(self + 0x40B2C) = count0 + 1;
    }

    int count = *reinterpret_cast<int *>(tbl + 0x4C);
    if (count + 1 > 0) {
        int off = 0x50;
        do {
            if (*reinterpret_cast<int *>(tbl + off) == -0x45 && i < count) {
                for (int j = off; j < 0x74; j += 4) {
                    *reinterpret_cast<int *>(tbl + j) =
                        *reinterpret_cast<int *>(tbl + j + 4);
                }
                *reinterpret_cast<int *>(tbl + 0x4C) -= 1;
                count = *reinterpret_cast<int *>(tbl + 0x4C);
                if (count < *reinterpret_cast<int *>(self + 0x40B2C)) {
                    if (count < 2) {
                        count = 1;
                    }
                    *reinterpret_cast<int *>(self + 0x40B2C) = count;
                }
                draw_queue(1);
            }
            draw_production(1);
            ++i;
            off += 4;
            count = *reinterpret_cast<int *>(tbl + 0x4C);
        } while (i < count + 1);
    }

    draw_queue(1);
}
