// ORIGINAL: 0x005F4C30 FILE
// name      ?redraw_nc_buffer@Win@@QAEHH@Z
// size      82 bytes
// spans     0x005F4C30-0x005F4C82
// prototype int (__thiscall ?redraw_nc_buffer@Win@@QAEHH@Z)(Win* this, int)
// callers   1   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005F4C30
// measured tier  MISMATCH
// divergence     31
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f4c30/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?redraw_nc_buffer@Win@@QAEHH@Z  at 0x005F4C30  (82 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 67
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
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044();
    virtual void slot045();
    virtual void slot046();
    virtual void slot047();
    virtual void slot048();
    virtual void slot049();
    virtual void slot050();
    virtual void slot051();
    virtual void slot052();
    virtual void slot053();
    virtual void slot054();
    virtual void slot055();
    virtual void slot056();
    virtual void slot057();
    virtual void slot058();
    virtual void slot059();
    virtual void slot060();
    virtual void slot061();
    virtual void slot062();
    virtual void slot063();
    virtual void slot064();
    virtual void slot065();
    virtual void slot066();
    virtual void slot067(int, int);  // <-- used
};

// ---- fixed globals this body references ----
// The const-pointer spelling reproduces the original's
// encoding including the address; `extern T *g` does not.
static int *const g_005f4c84 = (int *)0x005F4C84;

class Win { public:
    int redraw_nc_buffer(int);
};

// RULED OUT, near miss only: every one of the 31 real instructions below
// matches the original exactly (mnemonics_in_common == original_mnemonics
// across every flag set tried) - the switch compiles to the same dense
// `cmp/ja/jmp [reg*4+table]` shape, same four field reads, same register
// choices. What does not match is the OBJECT'S size: this is a
// single-function translation unit, so VC6 (/Gy) places the jump table
// inside the SAME COMDAT right after the code, then pads the COMDAT to a
// 16-byte boundary. `byte_match.clip_jump_table` (tools/byte_match.py)
// already strips a trailing table when its last relocation ends exactly at
// the COMDAT boundary; here code(82) + table(16) = 98, which pads to 112,
// leaving an 8-16 byte gap between the table's last entry and the
// boundary depending on flag set (frame-pointer on/off shifts the code
// length). That gap fails the exact-adjacency check, so the table is not
// clipped and decodes as ~14-18 bytes of phantom `add byte ptr [eax],al`.
// The ORIGINAL excludes the table entirely - it sits in the shared gap
// between functions, inside no symbol's own span - so there is no
// candidate spelling on this side that reproduces a boundary the original
// never had. Tried both /O2 flag sets (with and without /Oy-); both
// produce the identical real-code match and the identical trailing-table
// artifact. This is the harness's copy of `clip_jump_table`'s own
// documented case (see its docstring: "28 table bytes decoded as 14
// phantom add byte ptr [eax], al") minus the exact-adjacency the original
// measurement had - a size-of-COMDAT wall, not a body defect.
int Win::redraw_nc_buffer(int a1) {
    char *self = reinterpret_cast<char *>(this);
    reinterpret_cast<VCall *>(this)->slot067(0, a1);
    switch (a1) {
        case 0:
            return *reinterpret_cast<int *>(self + 0xb4);
        case 1:
            return *reinterpret_cast<int *>(self + 0xb8);
        case 2:
            return *reinterpret_cast<int *>(self + 0xc0);
        case 3:
            return *reinterpret_cast<int *>(self + 0xbc);
        default:
            return 0;
    }
}
