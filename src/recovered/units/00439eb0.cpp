// ORIGINAL: 0x00439EB0 FILE
// name      ?delete_abil@DesignWin@@QAEXH@Z
// size      104 bytes
// spans     0x00439EB0-0x00439F18
// prototype void (__thiscall ?delete_abil@DesignWin@@QAEXH@Z)(DesignWin* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00439EB0
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00439eb0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?delete_abil@DesignWin@@QAEXH@Z  at 0x00439EB0  (104 bytes)
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
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

class DesignWin { public:
    void delete_abil(int);
};

void DesignWin::delete_abil(int a1) {
    char *self = reinterpret_cast<char *>(this);
    unsigned int *flags = reinterpret_cast<unsigned int *>(self + 0x14208);

    if (a1 & *flags) {
        // WALL: register allocation. Every one of the 40 mnemonics below
        // matches the original in order; only WHICH callee-saved register
        // (esi vs edi) holds `this` versus the `slots` pointer is swapped
        // throughout (the original saves `this` unconditionally into esi
        // and the slots pointer conditionally into edi; this compile makes
        // the opposite choice). No reordering of these declarations changed
        // which register the backend picked - ruled out as unfixable via
        // source spelling.
        int *slots = reinterpret_cast<int *>(self + 0x14218);
        *flags = ~a1 & *flags;
        slots[0] = -1;
        slots[1] = -1;
        int i = 0;
        do {
            if (*flags & (1 << i)) {
                int j = 0;
                do {
                    if (slots[j] < 0) {
                        slots[j] = i;
                        break;
                    }
                    j++;
                } while (j < 2);
            }
            i++;
        } while (i < 0x1d);
    }
}
