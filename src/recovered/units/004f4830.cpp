// ORIGINAL: 0x004F4830 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004F4830
// name           ?base_check_support@@YAXXZ
// size           1419 bytes
// measured tier  MISMATCH
// divergence     0
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004f4830/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_check_support@@YAXXZ  at 0x004F4830  (1419 bytes)
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

class Sprite;

class Sprite { public:
    int ppszFileName_;
    int pcBits_;
    char cTransparentIndex_;
    char pad1_;
    char pad2_;
    char pad3_;
    int iSpriteWidth2_;
    int iSpriteWidth_;
    int iSpriteHeight_;
    int iWidth_;
    int iHeight_;
    int iLeftOffset_;
    int iTopOffset_;
    int fObj1Exists_;
};

// ---- callees, declared and never defined (a definition would be inlined) ----
extern "C" int abs(int);
extern "C" int rand();
int __cdecl has_abil(int, int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl popb(const char *, int, int, const char *, Sprite *);
void __cdecl bitmask(int, int *, int *);
void __cdecl kill(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689370 = (int *)0x00689370;
static int *const g_00689b68 = (int *)0x00689B68;
static int *const g_00689b7c = (int *)0x00689B7C;
static int *const g_0068faf0 = (int *)0x0068FAF0;
static int *const g_0090e91c = (int *)0x0090E91C;
static int *const g_0090ea30 = (int *)0x0090EA30;
static int *const g_00949870 = (int *)0x00949870;
static int *const g_0094988c = (int *)0x0094988C;
static int *const g_0094a30c = (int *)0x0094A30C;
static int *const g_0095282c = (int *)0x0095282C;
static int *const g_00952856 = (int *)0x00952856;
static int *const g_0096cc34 = (int *)0x0096CC34;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009ab891 = (int *)0x009AB891;
static int *const g_009ab892 = (int *)0x009AB892;

void __cdecl base_check_support() {
    char *baseRec = *reinterpret_cast<char **>(0x90ea30);
    unsigned int local_8 = *reinterpret_cast<unsigned char *>(baseRec + 2);

    if (*reinterpret_cast<int *>(baseRec + 0x62 * 2) < *reinterpret_cast<int *>(0x90e91c)) {
        do {
            int local_c = -1;
            int local_14 = -1;
            int local_10 = 0;
            if (*reinterpret_cast<int *>(0x9a64c8) < 1)
                break;

            char *rec = reinterpret_cast<char *>(0x952856);
            do {
                if (*reinterpret_cast<char *>(rec - 0x20) == *reinterpret_cast<char *>(baseRec + 4) &&
                    *reinterpret_cast<short *>(rec) == *reinterpret_cast<int *>(0x689370)) {
                    int idxA = *reinterpret_cast<short *>(rec - 0x24);
                    if (*reinterpret_cast<char *>(0x9ab892 + idxA * 0x34) == 10) {
                        if (*reinterpret_cast<char *>(rec - 0x1d) == 3 &&
                            *reinterpret_cast<char *>(rec - 8) == 1) {
                            int tileY = *reinterpret_cast<short *>(rec - 0x2c);
                            int tileX = *reinterpret_cast<short *>(rec - 0x2e);
                            int cell = *reinterpret_cast<int *>(0x94a30c) +
                                       (tileY * (*reinterpret_cast<int *>(0x68faf0)) + (tileX >> 1)) * 0x2c;
                            if ((*reinterpret_cast<unsigned char *>(cell + 8) & 1) != 0 &&
                                (*reinterpret_cast<unsigned char *>(cell + 2) & 0xf) < 8) {
                                int iVar7 = 99999;
                                if (local_c < iVar7) {
                                    local_14 = local_10;
                                    local_c = iVar7;
                                }
                            }
                        }
                    } else if ((*reinterpret_cast<unsigned char *>(rec - 0x2a) & 0x10) != 0 &&
                               static_cast<unsigned char>(*reinterpret_cast<char *>(0x9ab892 + idxA * 0x34)) < 10) {
                        int baseY = *reinterpret_cast<short *>(baseRec + 2 * 1);
                        int tileY = *reinterpret_cast<short *>(rec - 0x2c);
                        int dx = abs(*reinterpret_cast<short *>(rec - 0x2e) - *reinterpret_cast<short *>(baseRec));
                        if ((*reinterpret_cast<unsigned char *>(0x94988c) & 1) == 0 &&
                            *reinterpret_cast<int *>(0x68faf0) < dx) {
                            dx = *reinterpret_cast<int *>(0x949870) - dx;
                        }
                        int dy = abs(tileY - baseY);
                        int adx = abs(dx);
                        int ady = abs(dy);
                        int hi = (adx > ady) ? adx : ady;
                        int lo = (ady > adx) ? adx : ady;
                        int iVar7 = hi - (((adx + ady) / 2 - lo + 1) / 2);
                        baseRec = *reinterpret_cast<char **>(0x90ea30);
                        if (local_c < iVar7) {
                            local_14 = local_10;
                            local_c = iVar7;
                        }
                    }
                }
                local_10 = local_10 + 1;
                rec = rec + 0x34;
            } while (local_10 < *reinterpret_cast<int *>(0x9a64c8));

            if (local_c < 0)
                break;

            int recIdx = *reinterpret_cast<short *>(0x952832 + local_14 * 0x34);
            if (*reinterpret_cast<char *>(0x9ab892 + recIdx * 0x34) == 10) {
                *reinterpret_cast<char *>(0x952839 + local_14 * 0x34) = 0;
                *reinterpret_cast<int *>(0x90e91c) -= 1;
            } else {
                unsigned int usedIdx = local_8;
                if ((*reinterpret_cast<unsigned char *>(0x9a64e8) & static_cast<unsigned char>(1 << (local_8 & 0x1f))) == 0) {
                    if (*reinterpret_cast<char *>(0x9ab892 + recIdx * 0x34) == 8) {
                        *reinterpret_cast<unsigned int *>(0x95282c + local_14 * 0x34) &= 0xffffffef;
                        int morale = *reinterpret_cast<int *>(0x96cc34 + local_8 * 0x20cc);
                        int killed = has_abil(recIdx, 0x10000);
                        if (killed == 0) {
                            *reinterpret_cast<int *>(0x90e91c) -= (morale < -3) ? 2 : 1;
                        }
                    }
                } else {
                    parse_says(1, reinterpret_cast<char *>(0x9ab868 + recIdx * 0x34), -1, -1);
                    popb(reinterpret_cast<const char *>(0x689b7c), 0x8000, 0xd, reinterpret_cast<const char *>(0x689b68), 0);
                    usedIdx = local_8;
                }
                int morale2 = *reinterpret_cast<int *>(0x96cc34 + usedIdx * 0x20cc);
                int killed2 = has_abil(recIdx, 0x10000);
                baseRec = *reinterpret_cast<char **>(0x90ea30);
                if (killed2 != 0)
                    break;
                *reinterpret_cast<int *>(0x90e91c) -= (morale2 < -3) ? 2 : 1;
                kill(local_14);
                baseRec = *reinterpret_cast<char **>(0x90ea30);
            }
        } while (*reinterpret_cast<int *>(baseRec + 0x62 * 2) < *reinterpret_cast<int *>(0x90e91c));
    }

    int faction = *reinterpret_cast<int *>(0x689370);
    unsigned char bitMask = static_cast<unsigned char>(1 << (local_8 & 0x1f));

    if (((*reinterpret_cast<unsigned char *>(0x9a64e8) & bitMask) == 0 ||
         ((*reinterpret_cast<unsigned int *>(baseRec + 0x1c * 2) & 0x40) != 0 &&
          (*reinterpret_cast<unsigned int *>(baseRec + 0x1c * 2) & 0x80000000u) != 0)) &&
        (*reinterpret_cast<unsigned char *>(baseRec + 0x18 * 2) & 2) != 0) {
        int local_1c, local_18;
        bitmask(0x23, &local_1c, &local_18);

        int rowBase = local_8 * 0x20cc;
        int bonus = ((*reinterpret_cast<unsigned char *>(0x97d0cc + faction * 0x134 + local_1c) & local_18) != 0) ? 2 : 0;
        if (*reinterpret_cast<int *>(0x96cc40 + rowBase) + bonus < -2 &&
            (*reinterpret_cast<int *>(baseRec + 0x8c) < *reinterpret_cast<int *>(baseRec + 0x8e) ||
             *reinterpret_cast<int *>(baseRec + 0x68) < 0)) {
            int local_10 = 0;
            int local_c = -1;
            int local_14 = -1;

            if (*reinterpret_cast<int *>(0x9a64c8) > 0) {
                char *unit = reinterpret_cast<char *>(0x95282c);
                do {
                    if (*reinterpret_cast<unsigned char *>(unit + 10) == local_8 &&
                        *reinterpret_cast<short *>(unit + 0x2a) == faction) {
                        unsigned int flags = *reinterpret_cast<unsigned int *>(unit);
                        if ((flags & 0x200000) != 0 &&
                            ((bitMask & *reinterpret_cast<unsigned char *>(0x9a64e8)) == 0 ||
                             ((flags & 0x200) != 0 && (flags & 0x1000200) != 0x1000200 &&
                              ((flags & 0x2000200) != 0x2000200 ||
                               *reinterpret_cast<unsigned char *>(unit + 0xe) <= *reinterpret_cast<unsigned char *>(unit + 0xf)) &&
                              *reinterpret_cast<char *>(unit + 0x22) != 0x0b &&
                              *reinterpret_cast<char *>(unit + 0x22) != 0x0a &&
                              *reinterpret_cast<char *>(unit + 0x22) == 0))) {
                            int prob = *reinterpret_cast<int *>(0x96cdbc + rowBase);
                            if (prob == -1 || prob + 1 < 0 || rand() % (prob + 2) == 0) {
                                int uY = *reinterpret_cast<short *>(unit - 2);
                                int bY = *reinterpret_cast<short *>(baseRec + 2);
                                int dx2 = abs(*reinterpret_cast<short *>(unit - 4) - *reinterpret_cast<short *>(baseRec));
                                if ((*reinterpret_cast<unsigned char *>(0x94988c) & 1) == 0 &&
                                    *reinterpret_cast<int *>(0x68faf0) < dx2) {
                                    dx2 = *reinterpret_cast<int *>(0x949870) - dx2;
                                }
                                unsigned char reach = *reinterpret_cast<unsigned char *>(0x9ab891 + *reinterpret_cast<short *>(unit + 6) * 0x34);
                                unsigned int reachV = reach;
                                if (reach == 0)
                                    reachV = 1;
                                else if (reach > 99)
                                    reachV = 99;
                                int dy2 = abs(uY - bY);
                                int iVar7 = ((dx2 + dy2) / 2 * 100) / static_cast<int>(reachV);
                                char kind = *reinterpret_cast<char *>(0x9ab892 + *reinterpret_cast<short *>(unit + 6) * 0x34);
                                if (kind == 2)
                                    iVar7 = iVar7 * 2;
                                if (kind == 1)
                                    iVar7 = iVar7 / 2;
                                if (local_c < iVar7) {
                                    local_14 = local_10;
                                    local_c = iVar7;
                                }
                            }
                        }
                    }
                    local_10 = local_10 + 1;
                    unit = unit + 0x34;
                } while (local_10 < *reinterpret_cast<int *>(0x9a64c8));

                if (local_14 >= 0) {
                    kill(local_14);
                    *reinterpret_cast<unsigned int *>(baseRec + 0x18) &= 0xfffffffd;
                }
            }
        }
    }
}
