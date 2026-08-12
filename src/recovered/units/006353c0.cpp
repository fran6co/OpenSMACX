// ORIGINAL: 0x006353C0 FILE
// name      ?get_mem@TexHeap@@QAEHH@Z
// size      66 bytes
// spans     0x006353C0-0x00635402
// prototype int (__thiscall ?get_mem@TexHeap@@QAEHH@Z)(#120* this, int)
// callers   1   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4680
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x006353C0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006353c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?get_mem@TexHeap@@QAEHH@Z  at 0x006353C0  (66 bytes)
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

struct reqSize;

// ---- callees, declared and never defined (a definition would be inlined) ----
class Heap { public:
    void* get(int reqSize);
};

// The generated class name above is IDA's unnamed-type placeholder
// (`#120`, not valid C++); the brief's exact contract head names it
// TexHeap, which is used directly instead.
//
// MNEMONIC_ONLY, closest reached: every mnemonic matches position-for-
// position and the byte count is exact (66/66), but `count` and the loop
// index `i` land in the opposite registers from the original throughout
// (original: index in eax, count in edx; here: count in eax, index in edx -
// a full, consistent swap). Tried reordering the two locals' declarations
// both ways and flipping the comparison direction (`i < count` vs
// `count > i`); neither changed which one VC6 puts in eax, so this reads as
// the allocator's own tie-break rather than something this source form
// controls. Separately: a `do { } while` (or a `for` loop) for the
// index/count guard both cause VC6 to PEEL the loop - duplicating the
// found-check into a second copy ahead of the real loop, growing the body
// past the original's 66 bytes - so the loop is written as a plain `while`
// nested in an explicit `if (count > 0)` guard, which reproduces the
// original's single guard-then-loop shape exactly.
class TexHeap { public:
    int get_mem(int);
};

int TexHeap::get_mem(int a1) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self) != 0) {
        int i = 0;
        int count = *reinterpret_cast<int *>(self + 0xc);
        ++count;
        if (count > 0) {
            int **arr = *reinterpret_cast<int ***>(self + 8);
            while (i < count) {
                if (*reinterpret_cast<int *>(
                        reinterpret_cast<char *>(arr[i]) + 0x10) >= a1) {
                    return reinterpret_cast<int>(
                        reinterpret_cast<Heap *>(arr[i])->get(a1));
                }
                ++i;
            }
        }
    }
    return 0;
}
