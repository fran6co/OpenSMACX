// ORIGINAL: 0x004E1290 ?editor_clear@Console@@QAEXXZ 0x004E1290-0x004E12FB FILE
// size      107 bytes
// prototype void (__thiscall ?editor_clear@Console@@QAEXXZ)(Console* this)
// callers   1   call targets   7
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046B190 0x0058CED0 0x00590E90 0x00590ED0 0x00591040 0x0059A220 0x005ABF20
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004E1290
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004e1290/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?editor_clear@Console@@QAEXXZ  at 0x004E1290  (107 bytes)
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
class Path { public:
    void init();
};
int __cdecl size_of_planet(int);
void __cdecl auto_undo();
void __cdecl draw_map(int);
void __cdecl map_init();
void __cdecl map_shutdown();
void __cdecl map_wipe();

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_008e9f60 = (int *)0x008E9F60;
static int *const g_00939288 = (int *)0x00939288;
static int *const g_0093928c = (int *)0x0093928C;
static int *const g_00945b00 = (int *)0x00945B00;
static int *const g_009a64c0 = (int *)0x009A64C0;
static int *const g_009a64c8 = (int *)0x009A64C8;
static int *const g_009a64cc = (int *)0x009A64CC;

class Console { public:
    void editor_clear();
};

// `redraw` is a tail-jmp target the emitter did not surface a class for;
// the direct call's target bytes are relocated and masked regardless of
// which class name they mangle through.
class GraphicWinRedraw { public:
    void redraw();
};

void Console::editor_clear() {
    auto_undo();
    *g_009a64c0 |= 4;
    *g_009a64c8 = 0;
    *g_009a64cc = 0;
    *g_00939288 = 0;
    *g_0093928c = -1;
    map_wipe();
    size_of_planet(2);
    map_shutdown();
    map_init();
    reinterpret_cast<Path *>(g_00945b00)->init();
    map_wipe();
    draw_map(1);
    reinterpret_cast<GraphicWinRedraw *>(g_008e9f60)->redraw();
}
