// ORIGINAL: 0x0048BC60 ?init@PlanWin@@QAEXPAD@Z 0x0048BC60-0x0048BCC5 FILE
// size      101 bytes
// prototype void (__thiscall ?init@PlanWin@@QAEXPAD@Z)(PlanWin* this, int8*)
// callers   3   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00462870 0x00470A90 0x005D7670
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0048BC60
// measured tier  MISMATCH
// divergence     18
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0048bc60/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?init@PlanWin@@QAEXPAD@Z  at 0x0048BC60  (101 bytes)
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

struct ExtDirectDraw;
struct height;
struct width;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Buffer { public:
    int init(int width, int height, int, ExtDirectDraw*);
};
class MapWin { public:
    void clear(int);
    void init(int, int);
};

class PlanWin { public:
    void init(char*);
};

// NOTE: the LIVE emitter's fresh scaffold for this address is currently
// broken - it declares `class GraphicWin { ... Buffer buffer_; ... };`
// (an embedded-by-value member) before Buffer's own definition, so any
// body against it fails with C2079 regardless of content. Confirmed with
// an empty body. This unit therefore keeps the simpler (working) Buffer/
// MapWin shells rather than adopting the fresh ones, per the "on-disk
// scaffold is stale" rule's other direction: prefer whichever compiles.
// Only int8* -> char* was pulled from the fresh brief (PlanWin::init's
// own parameter, unrelated to the GraphicWin/Buffer ordering bug).
void PlanWin::init(char* a1) {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<int *>(self + 0x21a68) = 1;
    *reinterpret_cast<int *>(self + 0x21ff8) = 0;
    *reinterpret_cast<int *>(self + 0x1dd78) = 1;

    // clear()/init() are called with the SAME `this` pointer PlanWin was
    // given - the disassembly never re-derives ecx before either call, so
    // this PlanWin's address is directly a valid MapWin this-pointer (a
    // base subobject at offset 0), reproduced here without redeclaring the
    // (already complete) PlanWin class above.
    reinterpret_cast<MapWin *>(this)->clear(1);
    reinterpret_cast<MapWin *>(this)->init(3, 0);

    *reinterpret_cast<int *>(self + 0x1dd98) = -0xe;

    // vbtable indirection: [this] is the vbtable pointer, slot 1 is the
    // byte displacement to a virtual base subobject.
    int *vbtable = *reinterpret_cast<int **>(self);
    int disp = vbtable[1];
    int height = -*reinterpret_cast<int *>(self + disp + 0x4c8);
    int width = *reinterpret_cast<int *>(self + disp + 0x4c4);

    reinterpret_cast<Buffer *>(self + 0x21a70)->init(width, height, 0, 0);
}
