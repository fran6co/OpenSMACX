// ORIGINAL: 0x005FE500 FILE
// name      ?close@Palette@@QAEXXZ
// size      84 bytes
// spans     0x005FE500-0x005FE554
// prototype void (__thiscall ?close@Palette@@QAEXXZ)(Palette* this)
// callers   5   call targets   3
// kind      game
// flags     sp_ready;purged_ok
// calls     0x00616200 0x00644EF2 0x0064557F
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005FE500
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005fe500/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?close@Palette@@QAEXXZ  at 0x005FE500  (84 bytes)
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
class Time { public:
    ~Time();
};
// The scaffold's placeholders for these two were nullary and missing
// linkage (_free as a mangled C++ name, operator delete as "fn_0064557f").
// Respelled so CL mangles them back to the catalogued symbols.
extern "C" void free(void *);
void operator delete(void *);

class Palette { public:
    void close();
};

void Palette::close() {
    char *base = reinterpret_cast<char *>(this) + 0x408;
    int n = 5;
    do {
        Time *t = *reinterpret_cast<Time **>(base);
        *reinterpret_cast<int *>(base - 4) = -1;
        if (t != 0) {
            t->~Time();
            operator delete(t);
            *reinterpret_cast<Time **>(base) = 0;
        }
        void *ptr = *reinterpret_cast<void **>(base + 8);
        if (ptr != 0) {
            free(ptr);
            *reinterpret_cast<void **>(base + 8) = 0;
        }
        *reinterpret_cast<unsigned char *>(base + 4) = 0;
        *reinterpret_cast<unsigned char *>(base + 5) = 0;
        base += 0x10;
    } while (--n);
}
