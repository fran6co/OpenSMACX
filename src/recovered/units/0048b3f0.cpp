// ORIGINAL: 0x0048B3F0 FILE
// name      ?back_redraw@PlanWin@@QAEXXZ
// size      85 bytes
// spans     0x0048B3F0-0x0048B445
// prototype void (__thiscall ?back_redraw@PlanWin@@QAEXXZ)(PlanWin* this)
// callers   4   call targets   3
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046A550 0x005D5250 0x005D95B0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0048B3F0
// measured tier  MISMATCH
// divergence     13
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0048b3f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?back_redraw@PlanWin@@QAEXXZ  at 0x0048B3F0  (85 bytes)
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
    int copy(Buffer*, int, int, int, int);
};
class GraphicWin { public:
    void fill(int);
};
class MapWin { public:
    void draw_map(int);
};

class PlanWin { public:
    void back_redraw();
};

void PlanWin::back_redraw() {
    char *self = reinterpret_cast<char *>(this);

    *reinterpret_cast<uint32_t *>(self + 0x1dd70) = 0x84010002;

    // GraphicWin is reached through a this-adjustor read off the vtable
    // (vtbl[1], i.e. *(int*)(vtbl+4)), not a fixed offset - the same
    // adjustment is redone below for the Buffer access, since the call to
    // draw_map in between forces a reload.
    int vtbl1 = *reinterpret_cast<int *>(self);
    int offset1 = *reinterpret_cast<int *>(vtbl1 + 4);
    reinterpret_cast<GraphicWin *>(self + offset1)->fill(0);

    reinterpret_cast<MapWin *>(self)->draw_map(0);

    int vtbl2 = *reinterpret_cast<int *>(self);
    int offset2 = *reinterpret_cast<int *>(vtbl2 + 4);
    char *adjusted = self + offset2;
    Buffer *dst = reinterpret_cast<Buffer *>(adjusted + 0x444);
    Buffer *src = reinterpret_cast<Buffer *>(self + 0x21a70);
    dst->copy(src, 0, 0,
              *reinterpret_cast<int *>(adjusted + 0x4c4),
              -*reinterpret_cast<int *>(adjusted + 0x4c8));
}
