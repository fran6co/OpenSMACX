// ORIGINAL: 0x004043D0 ??1UV2Player@@QAE@XZ 0x004043D0-0x0040441C;0x00650740-0x00650758 FILE
// size      100 bytes
// prototype void (__thiscall ??1UV2Player@@QAE@XZ)(UV2Player* this)
// callers   0   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004BF400 0x005D7410
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004043D0
// measured tier  MISMATCH
// divergence     7
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004043d0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ??1UV2Player@@QAE@XZ  at 0x004043D0  (100 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class Buffer { public:
    ~Buffer();
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_0065074e = (int *)0x0065074E;
static int *const g_00671a30 = (int *)0x00671A30;

class UV2Player { public:
    void close();
    ~UV2Player();
};

// WALL: EH unwind funclet. The original protects the `close()` call with a
// frame-based SEH record so that a Buffer member at offset 0x8dc (the
// operand of `lea ecx,[esi+0x8dc]` before the second call) still gets
// destroyed if `close()` throws; the actual unwind thunk lives at 0x650740,
// outside this 100-byte span. UV2Player is opaque here (no data member can
// be added to the already-closed class), so the compiler cannot be made to
// emit that member automatically - a local guard object reproduces the same
// idea but its destructor cannot be inlined into this translation unit,
// which the verifier refuses (a second .text symbol appears - the original
// does not inline its funclet either, but that funclet is not part of the
// symbol this file may define). A `try`/`catch(...)` is the closest
// single-symbol approximation: it reproduces the SEH prologue byte-for-byte
// (frame push, `push -1`, handler install) before diverging on the extra
// catch-dispatch state a real `catch` needs and the implicit member cleanup
// does not.
UV2Player::~UV2Player() {
    try {
        close();
    } catch (...) {
        reinterpret_cast<Buffer *>(reinterpret_cast<char *>(this) + 0x8dc)->~Buffer();
        throw;
    }
    reinterpret_cast<Buffer *>(reinterpret_cast<char *>(this) + 0x8dc)->~Buffer();
}
