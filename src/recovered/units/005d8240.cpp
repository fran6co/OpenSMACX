// ORIGINAL: 0x005D8240 BYTE_EXACT FILE
// name      ?fill@Buffer@@QAEHHHHHH@Z
// size      66 bytes
// spans     0x005D8240-0x005D8282
// prototype int (__thiscall ?fill@Buffer@@QAEHHHHHH@Z)(Buffer* this, int xLeft, int yTop, int length, int width, int)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DFCD0
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005D8240
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005d8240/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?fill@Buffer@@QAEHHHHHH@Z  at 0x005D8240  (66 bytes)
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

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_00669274 = (int *)0x00669274;

class Buffer { public:
    int fill(int, int, int, int, int);
    // Sibling overload ?fill@Buffer@@QAEHPAURECT@@H@Z at 0x5DFCD0 - declared,
    // never defined here: a definition would be inlined and the original
    // reaches it with a genuine `call rel32`.
    int fill(int *, int);
};

int Buffer::fill(int a1, int a2, int a3, int a4, int a5) {
    int rect[4];

    // The IAT slot at 0x669274 - SetRect(lprc, xLeft, yTop, xRight, yBottom).
    typedef int (__stdcall *FnSetRect)(int *, int, int, int, int);
    (*reinterpret_cast<FnSetRect *>(g_00669274))(rect, a1, a2, a1 + a3, a2 + a4);

    return fill(rect, a5);
}
