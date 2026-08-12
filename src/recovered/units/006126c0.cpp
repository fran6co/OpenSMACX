// ORIGINAL: 0x006126C0 FILE
// name      ?pass_dialog_focus@EditGroup@@QAEXXZ
// size      56 bytes
// spans     0x006126C0-0x006126F8
// prototype void (__thiscall ?pass_dialog_focus@EditGroup@@QAEXXZ)(EditGroup* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x006126C0
// measured tier  MISMATCH
// divergence     2
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006126c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pass_dialog_focus@EditGroup@@QAEXXZ  at 0x006126C0  (56 bytes)
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

// `[this-0x8c]` reads a vbtable pointer; `vbtable[1]` (offset 4) is the
// displacement from `this` to a virtual base subobject at that vbtable
// slot. `base+0x40/0x44/0x48` are three fields of that virtual base
// (a focus-chain: a "has children" flag, a list head, and a
// container-object whose field at +4 is the actual focus target).
// Slot 64 (offset 0x100) is the virtual called on that target; on any
// missing link this tail-calls Win::pass_dialog_focus with `this`
// re-adjusted by the SAME -0x8c (not simply `base`, so Win's `this`
// is NOT the virtual base pointer - confirmed by the divergence when
// substituting `base` for `base-0x8c` here).
//
// MISMATCH: matches from the third instruction on (every branch, the
// three field guards, the vtable dispatch, and the fallback tail-call
// all reproduce exactly). The one open divergence is the very first
// pointer combine: the original computes `edx = *(vbt+4)` then
// `lea eax, [edx+ecx]` (keeping the loaded offset in its own register,
// combined via lea), where every source form tried here collapses the
// same value into `add eax, ecx` one register earlier - a one-byte-
// shorter encoding. Both give the identical resulting address; only
// the register/opcode choice differs. Ruled out: reordering the
// intermediate `off` variable, inlining the whole expression into one
// statement, and swapping operand order - all three still choose
// `add`. This looks like the same class of backend register/opcode
// selection quirk as the documented "instruction scheduling" cases,
// not something addressable from source shape alone.
class VCall { public:
    void pad00(); void pad01(); void pad02(); void pad03(); void pad04();
    void pad05(); void pad06(); void pad07(); void pad08(); void pad09();
    void pad10(); void pad11(); void pad12(); void pad13(); void pad14();
    void pad15(); void pad16(); void pad17(); void pad18(); void pad19();
    void pad20(); void pad21(); void pad22(); void pad23(); void pad24();
    void pad25(); void pad26(); void pad27(); void pad28(); void pad29();
    void pad30(); void pad31(); void pad32(); void pad33(); void pad34();
    void pad35(); void pad36(); void pad37(); void pad38(); void pad39();
    void pad40(); void pad41(); void pad42(); void pad43(); void pad44();
    void pad45(); void pad46(); void pad47(); void pad48(); void pad49();
    void pad50(); void pad51(); void pad52(); void pad53(); void pad54();
    void pad55(); void pad56(); void pad57(); void pad58(); void pad59();
    void pad60(); void pad61(); void pad62(); void pad63();
    virtual void slot064();
};

class Win { public:
    void pass_dialog_focus();
};

class EditGroup { public:
    void pass_dialog_focus();
};

void EditGroup::pass_dialog_focus() {
    char *self = reinterpret_cast<char *>(this);
    char *base = self + *reinterpret_cast<int *>(
        reinterpret_cast<char *>(*reinterpret_cast<void **>(self - 0x8c)) + 4);
    if (*reinterpret_cast<void **>(base + 0x48) != 0) {
        if (*reinterpret_cast<void **>(base + 0x40) != 0) {
            void *obj = *reinterpret_cast<void **>(base + 0x44);
            void *child = *reinterpret_cast<void **>(reinterpret_cast<char *>(obj) + 4);
            if (child != 0) {
                reinterpret_cast<VCall *>(child)->slot064();
                return;
            }
        }
    }
    reinterpret_cast<Win *>(base - 0x8c)->pass_dialog_focus();
}
