// ORIGINAL: 0x005DB930 ?write_right_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z 0x005DB930-0x005DB984 FILE BYTE_EXACT
// size      84 bytes
// prototype int (__thiscall ?write_right_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z)(Buffer* this, Font*, int8*, RECT*, int)
// callers   0   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005DD450
// PRESERVED UNIT - measured BYTE_EXACT.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005DB930
// measured tier  BYTE_EXACT
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005db930/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?write_right_l@Buffer@@QAEHPAVFont@@PADPAURECT@@H@Z  at 0x005DB930  (84 bytes)
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

struct Font;
struct RECT;

// ---- callees, declared and never defined (a definition would be inlined) ----

class Buffer { public:
    int write_right_l(Font*, int8*, RECT*, int);
};

// The scaffold's Buffer stub only carries this one overload, but the body
// calls the SIBLING overload `write_right_l(char*, RECT*, int)` (same
// object, same vtable-less class) on itself. That overload cannot be added
// to the Buffer declaration above without redeclaring it, so a same-layout
// shadow class stands in for the call target; casting `this` across it is
// safe since both are the empty stub the emitter deliberately gives every
// class, and the call site is what needs to type-check, not the class name.
class Buffer3 {
public:
    int write_right_l(int8*, RECT*, int);
};

// Fields 0x52c/0x530/0x534 are saved, temporarily overridden, and restored
// around the recursive call - a 12-byte record, not three loose ints: the
// compiler only keeps a single narrow pointer (lea [this+0x52c]) resident
// across the call when the save/restore is written as one struct copy;
// written as three scalar loads it re-derives each field from `this` with
// a fresh 4-byte displacement instead of reusing that pointer.
struct FontSave {
    int font;
    int f530;
    int f534;
};

int Buffer::write_right_l(Font* a1, int8* a2, RECT* a3, int a4) {
    FontSave *p = reinterpret_cast<FontSave*>(reinterpret_cast<char*>(this) + 0x52c);
    FontSave saved = *p;
    if (a1 != 0 && *reinterpret_cast<int*>(reinterpret_cast<char*>(a1) + 8) != 0) {
        p->font = reinterpret_cast<int>(a1);
        *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x530) = 0;
        *reinterpret_cast<int*>(reinterpret_cast<char*>(this) + 0x534) = 0;
    }
    Buffer3 *self3 = reinterpret_cast<Buffer3*>(this);
    int ret = self3->write_right_l(a2, a3, a4);
    *p = saved;
    return ret;
}
