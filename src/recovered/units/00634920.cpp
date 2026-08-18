// ORIGINAL: 0x00634920 sub_634920 0x00634920-0x00634975 FILE
// size      85 bytes
// prototype 
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
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
// address        0x00634920
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00634920/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_634920  at 0x00634920  (85 bytes)
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

// `ecx` used with no corresponding stack slot for it, plus a byte-sized
// `mov al,1`/`xor al,al` result -> a __thiscall member returning bool,
// not the placeholder's free `int __stdcall(int)`. `a1` is dereferenced
// throughout (fld/fcomp against `[a1+...]`), so it is a pointer, not an
// int. Reading `this` and `a1` as flat byte arrays, this walks a 3x3
// (row stride 0xC, 3 floats/row) grid and compares `this[i*12+j*4]`
// against `a1[i*12+j*4]` as single-precision floats, returning false on
// the first mismatch - an elementwise 3x3 float-matrix equality check.
//
// MISMATCH. Shape, loop structure and the fcomp-against-memory form all
// reproduce; the one open gap is that the original recomputes
// `this - a1` INSIDE the outer loop on every iteration (needing `this`
// and `a1` to both stay live across the whole function, which is what
// costs it the extra `push ecx` frame slot and the extra register),
// where /O2 here proves that same subtraction loop-invariant and
// hoists it out regardless of where it is written in the source -
// tried computing it inside the outer loop body explicitly; the
// rebuilt object is identical either way (69 bytes against the
// original's 85). This reads as the same class of backend scheduling/
// optimization decision as the other register-allocation hard cases,
// not a reachable source-shape difference.
class Mat3x3Owner { public:
    bool equals(const float *a1) const;
};

bool Mat3x3Owner::equals(const float *a1) const {
    const char *self = reinterpret_cast<const char *>(this);
    const char *other = reinterpret_cast<const char *>(a1);
    long off = self - other;
    for (int i = 0; i < 3; i++) {
        const char *row = other + i * 12;
        for (int j = 0; j < 3; j++) {
            const char *p = row + j * 4;
            float thisVal = *reinterpret_cast<const float *>(p + off);
            if (thisVal != *reinterpret_cast<const float *>(p)) {
                return false;
            }
        }
    }
    return true;
}
