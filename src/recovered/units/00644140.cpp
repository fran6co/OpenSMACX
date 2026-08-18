// ORIGINAL: 0x00644140 _inflate_codes_new 0x00644140-0x00644179 FILE
// CORRECTED from sub_644140
//   zlib 1.0.2 infcodes.c, 57 bytes, byte-exact from upstream
// size      57 bytes
// prototype 
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x0064414C
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00644140
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00644140/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_644140  at 0x00644140  (57 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 8
//
// UNUSED, deliberately: the callgraph's "slot 8" (offset 0x20) read is a
// FALSE POSITIVE, the same shape already documented for a sibling address
// in this file (0x00644910: "eax is the incoming pointer itself... a raw
// function-pointer FIELD, one indirection, not a virtual call"). The
// disassembly calls [eax+0x20] with NO preceding `mov eax,[eax]` - there
// is no vtable fetch, so a VCall dispatch would call through the wrong
// pointer. The call site also cleans no stack itself (`add esp, 0xc`
// after the call, i.e. the CALLER cleans up), so the pointee is a plain
// `__cdecl` function pointer, not this shim's implicit thiscall.
class VCall { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
};

typedef void *(__cdecl *AllocFn)(int, int, int);

// The object this constructs, allocated via `a5`'s custom allocator:
// [+0x0] zeroed (a type tag or vtable slot), two flag bytes at
// [+0x10]/[+0x11], and two 4-byte fields at [+0x14]/[+0x18] - all offsets
// read straight from the store instructions, none guessed.
struct NewNode {
    int type_or_vtable;   // offset 0x0, zeroed on construction
    char pad1[0xc];
    char byte1;            // offset 0x10
    char byte2;             // offset 0x11
    char pad2[2];
    int field14;            // offset 0x14
    int field18;            // offset 0x18
};

// RULED OUT, near miss only (MNEMONIC_ONLY: same 57 bytes, same 19
// mnemonics, only two 8-bit register OPERANDS disagree). The original
// stores a1 through `dl` and a2 through `cl`; every spelling tried here -
// natural order, reversed assignment order, temp locals read in either
// order, and a raw `char*` store bypassing the struct entirely - always
// gets `cl` for whichever byte is read first and `dl` for the second,
// opposite of the original's preference. Not reachable by rewording the
// body; recorded here so the next agent does not re-try the same four
// spellings.
int __cdecl sub_644140(char a1, char a2, int a3, int a4, char *a5) {
    int ctx = *reinterpret_cast<int *>(a5 + 0x28);
    NewNode *node = reinterpret_cast<NewNode *>(
        (*reinterpret_cast<AllocFn *>(a5 + 0x20))(ctx, 1, 0x1c));
    if (node != 0) {
        node->byte1 = a1;
        node->byte2 = a2;
        node->type_or_vtable = 0;
        node->field14 = a3;
        node->field18 = a4;
    }
    return reinterpret_cast<int>(node);
}
