// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x0063E590
// name           sub_63e590
// size           79 bytes
// measured tier  MISMATCH
// divergence     10
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0063e590/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_63e590  at 0x0063E590  (79 bytes)
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

// The scaffolded `_free` above is the DECORATED name (leading underscore
// is how cdecl mangles a C symbol) and is not itself callable C++ syntax
// with the right arity - declare the real CRT prototype instead, which
// mangles to the same symbol.
extern "C" void __cdecl free(void *);

// Intrusive singly-linked list: this object's own [this+0] is the head
// pointer, [this+4]/[this+8] are cleared on empty. Each node has a
// secondary owned allocation at [node+8] and the next pointer at
// [node+0x10] (both offsets confirmed by the disassembly, not guessed).
struct ListNode {
    char pad0[8];
    void *data;       // offset 0x8
    char pad1[4];
    ListNode *next;   // offset 0x10
};

class NodeList { public:
    void free_list();
};

// RULED OUT, near miss only (98.67% mnemonic similarity, single missing
// instruction): the original computes `node = *this` once into eax, then
// ALSO copies it into ecx before reading node->next (through eax) and
// node->data (through the copy in ecx) - a redundant `mov ecx, eax` that
// changes nothing eax already held untouched. Every spelling tried here -
// a cached `ListNode *node` local, two independent `(*head)->field`
// dereferences, and a raw `int*` version matching the Ghidra hypothesis
// literally - gets the identical 77-byte body from CL: the compiler always
// folds the copy away since eax is provably unclobbered between the two
// reads. This is the "dedicates a callee-saved register" hard case this
// file's own header names, just for a general-purpose register instead of
// a callee-saved one - not reachable by rewording the body.
void NodeList::free_list() {
    char *self = reinterpret_cast<char *>(this);
    ListNode **head = reinterpret_cast<ListNode **>(self);
    if (*head != 0) {
        do {
            ListNode *next = (*head)->next;
            void *data = (*head)->data;
            if (data != 0) {
                free(data);
                (*head)->data = 0;
            }
            if (*head != 0) {
                free(*head);
            }
            *head = next;
        } while (*head != 0);
    }
    *reinterpret_cast<void **>(self + 4) = 0;
    *reinterpret_cast<void **>(self + 8) = 0;
}
