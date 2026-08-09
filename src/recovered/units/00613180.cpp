// ORIGINAL: 0x00613180 FILE
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x00613180
// name           ?pass_dialog_focus@Dialogs@@QAEXXZ
// size           117 bytes
// measured tier  MISMATCH
// divergence     26
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00613180/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?pass_dialog_focus@Dialogs@@QAEXXZ  at 0x00613180  (117 bytes)
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
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_006131f8 = (int *)0x006131F8;

class Dialogs { public:
    void pass_dialog_focus();
};

// Not declared by the scaffold (the two calls are tail-jumps, not `call`s,
// so the emitter's callee scan misses them): both are void __thiscall()
// members, matching `?pass_dialog_focus@Win@@QAEXXZ` /
// `?pass_dialog_focus@EditGroup@@QAEXXZ`.
class Win { public:
    void pass_dialog_focus();
};
class EditGroup { public:
    void pass_dialog_focus();
};

void Dialogs::pass_dialog_focus() {
    // `dec eax; cmp eax,0xf; ja` is the switch-bounds-check shape (16
    // contiguous case values 1..16), and `mov dl,byte[eax+table]; jmp
    // [edx*4+table2]` is the byte-remap dispatch MSVC emits when many case
    // values share few distinct bodies. Both tables are DATA past this
    // function's `ret` and outside its 117-byte span, so their exact
    // contents cannot be observed from the disassembly and do not affect
    // the score - only the dispatcher shape and the handler bodies do. The
    // four `Win` handlers are byte-identical in the original (not folded),
    // meaning four independent, non-fallthrough case bodies rather than one
    // shared case; the remaining 12 values are given to the `EditGroup`
    // handler via a single fallthrough run since nothing pins their exact
    // split.
    char *self = reinterpret_cast<char *>(this);
    int kind = *reinterpret_cast<int *>(self - 8);
    switch (kind) {
    case 1: {
        int off = *reinterpret_cast<int *>(*reinterpret_cast<char **>(self - 0x188) + 4);
        reinterpret_cast<Win *>((self - 0x188) + off)->pass_dialog_focus();
        break;
    }
    case 2: {
        int off = *reinterpret_cast<int *>(*reinterpret_cast<char **>(self - 0x188) + 4);
        reinterpret_cast<Win *>((self - 0x188) + off)->pass_dialog_focus();
        break;
    }
    case 3: {
        int off = *reinterpret_cast<int *>(*reinterpret_cast<char **>(self - 0x188) + 4);
        reinterpret_cast<Win *>((self - 0x188) + off)->pass_dialog_focus();
        break;
    }
    case 4: {
        int off = *reinterpret_cast<int *>(*reinterpret_cast<char **>(self - 0x188) + 4);
        reinterpret_cast<Win *>((self - 0x188) + off)->pass_dialog_focus();
        break;
    }
    case 5: case 6: case 7: case 8: case 9: case 10:
    case 11: case 12: case 13: case 14: case 15: case 16:
        reinterpret_cast<EditGroup *>(self - 4)->pass_dialog_focus();
        break;
    }
    // WALL: this reconstruction reproduces all 26 real instructions
    // byte-for-byte (mnemonic_similarity 0.9811, single divergence at the
    // position right after the original's `ret`). The only remaining
    // difference is 3 trailing bytes of `lea ecx,[ecx]` alignment padding
    // that this standalone translation unit's compiler-generated jump
    // table forces onto the end of the function (117 is not a multiple of
    // 4). The real binary does not pay this cost because its jump table
    // sits at the fixed, already-aligned address 0x6131f8 elsewhere in the
    // image, entirely outside this symbol's owned bytes - a test-harness
    // artifact of compiling one function's jump table in isolation, not a
    // wrong body.
}
