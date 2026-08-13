// ORIGINAL: 0x004A8440 BYTE_EXACT FILE
// name      ?labs_write@@YAXPAUGraphicWin@@HHPAURECT@@PAD@Z
// size      46 bytes
// spans     0x004A8440-0x004A846E
// prototype 
// callers   0   call targets   3
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050E820 0x005B9FE0 0x005DAD10
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x004A8440
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/004a8440/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?labs_write@@YAXPAUGraphicWin@@HHPAURECT@@PAD@Z  at 0x004A8440  (46 bytes)
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

struct GraphicWin;
struct RECT;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Buffer { public:
    void set_text_color3(int, int, int, int);
};
int __cdecl mandate_color(int category);
int __cdecl tech_category(int techID);

// UNREACHABLE (new wall, not previously catalogued): NO_COMPILE for every
// body, including a trivial empty one, independent of anything written
// here. `emit_translation_unit.by_value_first` orders "layout" classes so a
// by-value member's type is defined first, but it computes that order over
// `wanted - defined_later`, and `defined_later` already excludes any class
// that owns a METHOD this subject calls. GraphicWin embeds `Buffer buffer_`
// BY VALUE (offset 0x9C4) and this subject also calls `Buffer::
// set_text_color3` (a `methods_by_class` entry), so Buffer lands in
// `defined_later` and is skipped by the by-value sort - yet GraphicWin's
// full layout (which needs Buffer complete) is still emitted before
// Buffer's own definition in the later "callees" section. Every candidate
// body fails identically: `error C2079: 'buffer_' uses undefined class
// 'Buffer'` at the GraphicWin layout, before the subject's own body is even
// reached. Confirmed body-independent by testing a one-line no-op
// definition - same error, same line. Unfixable from unit.cpp alone (the
// scaffolding above this line is regenerated fresh from tools/, not read
// from this file); needs a tools/emit_translation_unit.py fix. Body below
// is the best-effort semantic reconstruction, kept for when that lands.
int __cdecl labs_write(GraphicWin* a1, int a2, int a3, RECT* a4, char* a5) {
    Buffer *buffer = reinterpret_cast<Buffer *>(reinterpret_cast<char *>(a1) + 0x444);
    buffer->set_text_color3(mandate_color(tech_category(a2)), 0, 1, 1);
    return 0;
}
