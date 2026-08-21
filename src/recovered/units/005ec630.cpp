// ORIGINAL: 0x005EC630 ?init@Win@@QAEHPAURECT@@PADHPAVWin@@PAVMenu@@PAUBorderSizing@@@Z 0x005EC630-0x005EC678 FILE BYTE_EXACT
// symbol    ?init@Win@@QAEHPAURECT@@PAUint8@@HPAV1@PAUMenu@@PAUBorderSizing@@@Z
// size      72 bytes
// prototype int (__thiscall ?init@Win@@QAEHPAURECT@@PADHPAVWin@@PAVMenu@@PAUBorderSizing@@@Z)(Win* this, RECT*, int8*, int, Win*, Menu*, BorderSizing*)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005EBD80
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005EC630
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ec630/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@Win@@QAEHPAURECT@@PADHPAVWin@@PAVMenu@@PAUBorderSizing@@@Z  at 0x005EC630  (72 bytes)
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
//
// SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// attempts to it. They are here rather than in a prompt so the next
// agent does not rediscover them:
//
//  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
//    swapped; `if (x) {...}` gives `je`.
//  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
//    two special cases branching FAR away, write them negated and
//    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
//    sequential guard clauses, which inline each body instead.
//  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
//    an indirect virtual call use the generated VCall shim below; for a
//    thiscall function POINTER, take a member-function pointer of a
//    dummy class instead of spelling the convention.
//  * Loop form is visible: while / do-while / for lower differently,
//    and so does counting up versus down.
//  * If the original dedicates a callee-saved register to a constant
//    across the whole body (an extra `push ebx`/`push edi` in the
//    prologue), it had enough register pressure to do so. That is a
//    hard case - the tool reports a similarity ratio so you can tell a
//    near miss from a wrong body.

typedef int int32_t;
typedef unsigned int uint32_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef signed char int8_t;
typedef unsigned char uint8_t;

struct BorderSizing;
struct Menu;
struct RECT;
struct int8;

// ---- callees, declared and never defined (a definition would be inlined) ----

class Win { public:
    int init(RECT*, int8*, int, Win*, Menu*, BorderSizing*);
};

// The catalog's callee at 0x5EBD80 is another Win::init overload (RECT
// unpacked to 4 ints); it isn't declared in this scaffold and `class Win`
// above is already closed, so it's reached through a same-shaped shadow
// class rather than reopening Win.
class WinInit9 { public:
    int init(int, int, int, int, int8 *, int, Win *, Menu *, BorderSizing *);
};

int Win::init(RECT* a1, int8* a2, int a3, Win* a4, Menu* a5, BorderSizing* a6) {
    if (a1 == 0) {
        return 3;
    }
    char *r = reinterpret_cast<char *>(a1);
    int top = *reinterpret_cast<int *>(r + 4);
    int left = *reinterpret_cast<int *>(r);
    return reinterpret_cast<WinInit9 *>(this)->init(
        left, top,
        *reinterpret_cast<int *>(r + 8) - left,
        *reinterpret_cast<int *>(r + 0xc) - top,
        a2, a3, a4, a5, a6);
}
