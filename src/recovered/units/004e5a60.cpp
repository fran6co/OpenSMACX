// ORIGINAL: 0x004E5A60 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x004E5A60
// name           ?base_change@@YAXHH@Z
// size           1404 bytes
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e5a60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_change@@YAXHH@Z  at 0x004E5A60  (1404 bytes)
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
int __cdecl base_making(int, int);
int __cdecl parse_say(int, int, int, int);
int __cdecl parse_says(int, char *, int, int);
int __cdecl popp(char *, const char *, int, const char *, int (__cdecl *)());
void __cdecl bitmask(int, int *, int *);
void __cdecl draw_radius(int, int, int, int);
void __cdecl interlude(int, char *, int, int);
void __cdecl wave_it(int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00689444 = (int *)0x00689444;
static int *const g_00689454 = (int *)0x00689454;
static int *const g_00689464 = (int *)0x00689464;
static int *const g_00689474 = (int *)0x00689474;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00939284 = (int *)0x00939284;
static int *const g_00945f44 = (int *)0x00945F44;
static int *const g_0094982c = (int *)0x0094982C;
static int *const g_00949830 = (int *)0x00949830;
static int *const g_0096c9f8 = (int *)0x0096C9F8;
static int *const g_0096ea3c = (int *)0x0096EA3C;
static int *const g_0097d044 = (int *)0x0097D044;
static int *const g_0097d090 = (int *)0x0097D090;
static int *const g_009a63fc = (int *)0x009A63FC;
static int *const g_009a64a4 = (int *)0x009A64A4;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64cc = (int *)0x009A64CC;
static int *const g_009a64e8 = (int *)0x009A64E8;
static int *const g_009a6514 = (int *)0x009A6514;
static int *const g_009a6800 = (int *)0x009A6800;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

// Indexed table bases / address-arithmetic globals: each address does
// work (scaled index or direct subtraction), so fresh externs are used
// instead of the scaffold's const-pointer spellings for these.
extern int g_x97d090[];    // per-base owner, stride 0x134 (elements are ints; index in bytes via char* casts)
extern unsigned char g_x97d044[];  // per-base id byte, stride 0x134
extern int g_x97d084[];
extern int g_x97d0dc[];
extern int g_x97d080[];
extern short g_x97d040[];
extern short g_x97d042[];
extern unsigned char g_x96d438[];
extern int g_x9a6514base;
extern int g_x9a63fc[];
extern unsigned int g_x0096ea3c[];
extern unsigned char g_x946d54[];
extern unsigned char g_x946d34[];
extern int g_x946d4c[];
extern int g_x946d50[];
extern int g_x9a4b68[];
extern unsigned char g_x946a9c[];
extern unsigned char g_x946a84[];
extern int g_x946a50[];
extern unsigned int g_x0096c9f8[];
extern int g_x9a64a4;

static char *rec134(void *base, int idx) {
    return reinterpret_cast<char *>(base) + idx * 0x134;
}

void __cdecl base_change(int a1, int a2) {
    if (a2 == 99999) {
        return;
    }
    int local8 = a1 * 0x134;
    int oldOwner = *reinterpret_cast<int *>(rec134(g_x97d090, a1));
    *reinterpret_cast<int *>(rec134(g_x97d090, a1)) = a2;
    if (a2 == oldOwner) {
        return;
    }

    unsigned char bVar1 = g_x97d044[local8];
    unsigned int uVar8 = bVar1;
    if (oldOwner >= 0) {
        g_x96d438[uVar8 * 0x20cc + oldOwner] = g_x96d438[uVar8 * 0x20cc + oldOwner] - 1;
    }
    if (a2 >= 0) {
        g_x96d438[uVar8 * 0x20cc + a2] = g_x96d438[uVar8 * 0x20cc + a2] + 1;
    }

    int iVar5;
    if (*g_0094982c == 0 || ((*reinterpret_cast<unsigned char *>(g_009a64e8) & (1 << (bVar1 & 0x1f))) == 0)) {
        iVar5 = 0;
    } else {
        int iVar5b = base_making(*reinterpret_cast<int *>(rec134(g_x97d084, a1)), a1);
        int iVar6 = base_making(*reinterpret_cast<int *>(rec134(g_x97d090, a1)), a1);
        if (iVar6 == iVar5b) {
            iVar5 = 0;
        } else {
            int iVar5c = *reinterpret_cast<int *>(rec134(g_x97d0dc, a1));
            if (iVar5c <= *g_00949830) {
                iVar5 = 0;
            } else {
                iVar5 = (iVar5c - (iVar5c - *g_00949830) * (100 - *g_0094982c) / 100) - *g_00949830;
            }
        }
    }
    *reinterpret_cast<int *>(rec134(g_x97d080, a1)) = *reinterpret_cast<int *>(rec134(g_x97d0dc, a1)) - iVar5;
    draw_radius(*reinterpret_cast<short *>(rec134(g_x97d040, a1)), *reinterpret_cast<short *>(rec134(g_x97d042, a1)), 2, 2);

    if (uVar8 == static_cast<unsigned int>(*g_00939284)) {
        return;
    }
    if (*g_00945f44 != 0) {
        return;
    }

    int newOwner = *reinterpret_cast<int *>(rec134(g_x97d090, a1));
    bool bVar2 = false;
    bool bVar4 = false;
    if (newOwner < -0x45 && *reinterpret_cast<int *>(reinterpret_cast<char *>(&g_x9a6514base) - (newOwner * 4 + 0x118)) == -1) {
        bVar2 = true;
        bVar4 = true;
    }

    bool bVar3 = false;
    if (oldOwner < -0x45 && (bVar2 || *reinterpret_cast<int *>(reinterpret_cast<char *>(&g_x9a6514base) - (oldOwner * 4 + 0x118)) == -1)) {
        int absOld = (oldOwner < 0) ? -oldOwner : oldOwner;
        int tblv = g_x9a63fc[absOld];
        if (tblv >= 0 && g_x97d044[tblv * 0x134] == uVar8) {
            bVar3 = true;
        }
    }

    if ((bVar2 || bVar3) &&
        ((*reinterpret_cast<unsigned char *>(g_009a64c0) & 1) == 0 ||
         (*reinterpret_cast<unsigned char *>(g_009a64c0) & 8) != 0)) {
        int absNew = (iVar5 < 0) ? -iVar5 : iVar5;
        int part;
        if (bVar2) {
            part = absNew - 0x46;
        } else {
            int absOld2 = (oldOwner < 0) ? -oldOwner : oldOwner;
            part = absOld2 - 0x46;
        }
        int outC, outMask;
        bitmask(part, &outC, reinterpret_cast<int *>(&outMask));

        int matchOld = 0;
        int matchNew = 0;
        if (*g_009a64cc > 0) {
            for (int i = 0; i < *g_009a64cc; i++) {
                unsigned char *rec = reinterpret_cast<unsigned char *>(g_x97d090) + i * 0x134;
                if (*(rec - 0x4c) == g_x97d044[local8] && i != a1) {
                    int ownerAtI = *reinterpret_cast<int *>(rec);
                    if (ownerAtI == oldOwner) {
                        matchOld = matchOld + 1;
                    }
                    if (ownerAtI == *reinterpret_cast<int *>(rec134(g_x97d090, a1))) {
                        matchNew = matchNew + 1;
                    }
                }
            }
        }

        if (bVar4 && matchNew == 0) {
            if (bVar3 && (matchOld == 0 || *reinterpret_cast<int *>(reinterpret_cast<char *>(&g_x9a6514base) - (oldOwner * 4 + 0x118)) != -1)) {
                if (*g_009a64cc > 0) {
                    for (int i = 0; i < *g_009a64cc; i++) {
                        unsigned char *rec = reinterpret_cast<unsigned char *>(g_x97d090) + i * 0x134;
                        if (*(rec - 0x4c) == uVar8 && *reinterpret_cast<int *>(rec) == oldOwner) {
                            *reinterpret_cast<int *>(rec) = *reinterpret_cast<int *>(rec134(g_x97d090, a1));
                        }
                    }
                }
                if (*g_009a6800 < 1) {
                    interlude(0, reinterpret_cast<char *>(g_x946d54 + uVar8 * 0x59c), -1, -1);
                    int absOld3 = (oldOwner < 0) ? -oldOwner : oldOwner;
                    parse_say(1, g_x9a4b68[absOld3 * 0xc], -1, -1);
                    *g_009bbfec = g_x946d4c[uVar8 * 0x167];
                    *g_009bbff0 = g_x946d50[uVar8 * 0x167];
                    interlude(2, reinterpret_cast<char *>(g_x946d34 + uVar8 * 0x59c), -1, -1);
                    parse_say(3, g_x9a4b68[iVar5 * 0xc], -1, -1);
                    popp(reinterpret_cast<char *>(g_00691b0c), reinterpret_cast<const char *>(g_00689474), 0, reinterpret_cast<const char *>(g_00689464), 0);
                    int idxA = uVar8 * 0x833 + outC;
                    g_x0096ea3c[idxA] = g_x0096ea3c[idxA] | outMask;

                    int absOld4 = (oldOwner < 0) ? -oldOwner : oldOwner;
                    int outC2, outMask2;
                    bitmask(absOld4 - 0x46, &outC2, &outMask2);
                    int idxB = uVar8 * 0x833 + outC2;
                    g_x0096ea3c[idxB] = g_x0096ea3c[idxB] & ~outMask2;
                    return;
                }
            } else {
                if ((g_x0096ea3c[uVar8 * 0x833 + outC] & outMask) == 0) {
                    if (*g_009a6800 > 0 && part != 0x1f) {
                        return;
                    }
                    *g_009bbff0 = 0;
                    int base59c = uVar8 * 0x59c;
                    *g_009bbfec = g_x946a50[uVar8 * 0x167];
                    interlude(0, reinterpret_cast<char *>(g_x946a9c + base59c), -1, -1);
                    *g_009bbfec = g_x946a50[uVar8 * 0x167];
                    *g_009bbff0 = 0;
                    interlude(1, reinterpret_cast<char *>(g_x946a84 + base59c), -1, -1);
                    *g_009bbfec = g_x946d4c[uVar8 * 0x167];
                    *g_009bbff0 = g_x946d50[uVar8 * 0x167];
                    interlude(2, reinterpret_cast<char *>(g_x946d34 + base59c), -1, -1);
                    parse_say(3, g_x9a4b68[iVar5 * 0xc], -1, -1);
                    int wv;
                    if ((g_x0096c9f8[uVar8 * 4 + *g_00939284 * 0x20cc] & 1) == 0) {
                        wv = 0x26;
                    } else {
                        wv = 0x24;
                    }
                    wave_it(wv);
                    popp(reinterpret_cast<char *>(g_00691b0c), reinterpret_cast<const char *>(g_00689454), 0, reinterpret_cast<const char *>(g_00689444), 0);
                    g_x0096ea3c[uVar8 * 0x833 + outC] = g_x0096ea3c[uVar8 * 0x833 + outC] | outMask;
                }
                if (part == 0x1f) {
                    interlude(0xb, 0, 1, 0);
                    g_x9a64a4 = g_x9a64a4 | 0x400;
                }
            }
        }
    }
}
