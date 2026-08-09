// ORIGINAL: 0x00523B50 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00523B50
// name           ?ascent@@YAXXZ
// size           264 bytes
// measured tier  MISMATCH
// divergence     37
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00523b50/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?ascent@@YAXXZ  at 0x00523B50  (264 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// VC6 DIALECT - this must compile under BOTH cl 12.00.8168 and
// i686-w64-mingw32-g++ -std=c++11. Avoid: auto, nullptr, constexpr,
// static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// and declaring `int i` twice in one function (VC6 leaks for-scope).
// static_cast/reinterpret_cast are fine and are the right spelling.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

struct int8;

// Opaque replica of the real `Player` record (src/faction.h) - only the
// three fields this body touches are named; the rest is padding that pins
// the total size to 1436 bytes so `g_00946a50[i]` scales exactly like the
// original's array indexing.
struct PlayerRec {
    int is_leader_female;   // offset 0x00
    char pad1[0x34 - 4];
    char name_leader[24];   // offset 0x34
    char title_leader[24];  // offset 0x4C
    char pad2[1436 - 0x64];
};

// ---- callees, declared and never defined (a definition would be inlined) ----
bool __cdecl has_tech(int techID, int factionID);
int __cdecl ascending(int factionID);
int __cdecl parse_says(int nID, int8* input, int gender, int pluralality);
int __cdecl popp(int8*, int8*, int, int8*, int (__cdecl *)());
void __cdecl interlude(int interludeID, int8* lpszInput, int popupInitType, int);

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0068b57c = (int *)0x0068B57C;
static int *const g_0068b58c = (int *)0x0068B58C;
static int *const g_00691b0c = (int *)0x00691B0C;
static int *const g_00939284 = (int *)0x00939284;
static PlayerRec *const g_00946a50 = (PlayerRec *)0x00946A50;
static int *const g_0096c9e0 = (int *)0x0096C9E0;
static int *const g_009a4d2c = (int *)0x009A4D2C;
static int *const g_009a64d4 = (int *)0x009A64D4;
static int *const g_009bbfec = (int *)0x009BBFEC;
static int *const g_009bbff0 = (int *)0x009BBFF0;

void __cdecl ascent() {
    if (*g_009a64d4 == 0x4b) {
        if (has_tech(*g_009a4d2c, *g_00939284)) {
            interlude(1, 0, 1, 0);
        }
    }

    if (!ascending(*g_00939284)) {
        return;
    }

    int faction_id = *g_00939284;
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096c9e0) + faction_id * 8396) & 0x2000) {
        return;
    }
    *reinterpret_cast<int *>(reinterpret_cast<char *>(g_0096c9e0) + faction_id * 8396) |= 0x2000;

    *g_009bbfec = g_00946a50[faction_id].is_leader_female;
    *g_009bbff0 = 0;
    parse_says(0, reinterpret_cast<int8 *>(g_00946a50[faction_id].title_leader), -1, -1);

    faction_id = *g_00939284;
    *g_009bbfec = g_00946a50[faction_id].is_leader_female;
    *g_009bbff0 = 0;
    parse_says(1, reinterpret_cast<int8 *>(g_00946a50[faction_id].name_leader), -1, -1);

    popp(reinterpret_cast<int8 *>(*g_00691b0c), reinterpret_cast<int8 *>(g_0068b58c), 0,
         reinterpret_cast<int8 *>(g_0068b57c), 0);
}
