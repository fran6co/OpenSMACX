// ORIGINAL: 0x004BDFA0 FILE
// name      ?base_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z
// size      61 bytes
// spans     0x004BDFA0-0x004BDFDD
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0055AF20
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004BDFA0
// measured tier  NO_COMPILE
// refusal        u004bdfa0.cpp(46) : error C2143: syntax error : missing ';' before 'this' u004bdfa0.cpp(46) : error C2143: syntax error : missing ';' before 'this' u004bdfa0.cp
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004bdfa0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?base_draw@TutWin@@QAAXPAUSprite@@PAUGraphicWin@@HHH@Z  at 0x004BDFA0  (61 bytes)
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

struct GraphicWin;
struct Sprite;
struct TutWin;
struct this;

// ---- callees, declared and never defined (a definition would be inlined) ----
void __cdecl base_draw(Buffer*, int, int, int, int, int);

// For a __cdecl non-static member, `this` lands at [ebp+8] and the first
// explicit argument at [ebp+0xc] (docs/recovery/agent-structure-observations
// .csv:34) - so a1 (Sprite*) is the pointer null-tested and offset by 0x444
// (the GraphicWin -> embedded Buffer pattern also seen at address 5D5500, so
// Sprite must carry the same Buffer at the same offset), and a2 (GraphicWin*)
// is the one read back as a plain int for the +2/sign-clamp - a C-style
// pointer-to-int reinterpretation the original made explicitly. a5 is never
// read at all (ebp+0x1c is untouched): it does not reach the callee.
void TutWin::base_draw(Sprite *a1, GraphicWin *a2, int a3, int a4, int a5) {
    int v = (int)a2 + 2;
    if (v < 0) {
        v = -2 - (int)a2;
    }
    Buffer *buf = a1 ? reinterpret_cast<Buffer *>(reinterpret_cast<char *>(a1) + 0x444) : 0;
    ::base_draw(buf, v, a3, a4, 0, 0);
}
