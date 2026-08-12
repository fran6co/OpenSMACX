// ORIGINAL: 0x005C93C0 FILE
// name      ?on_button_clicked@Gamma@@QAEXH@Z
// size      70 bytes
// spans     0x005C93C0-0x005C9406
// prototype void (__thiscall ?on_button_clicked@Gamma@@QAEXH@Z)(Gamma* this, int)
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// indirect  0x005C93E4 0x005C93FC
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005C93C0
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005c93c0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_button_clicked@Gamma@@QAEXH@Z  at 0x005C93C0  (70 bytes)
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

// Vtable shim. VC6 rejects a free `__thiscall` function pointer
// (C4234), so an indirect virtual call is spelled by calling the Nth
// virtual of a class that is never defined and never instantiated.
// Only DECLARATION ORDER matters - change a slot's signature freely
// to match the call you need; it will not move.
// This body dispatches through slot(s): 58
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
    virtual void slot058();  // <-- used
};

// ---- fixed globals this body references ----
// 0x0066EB38 is an fmul operand (a double constant). Read through the given
// `int *const` pointer, VC6 first loads the address into a register instead
// of folding it into the fmul's memory operand (`mov eax,0x66eb38; fmul
// [eax]` vs the original's direct `fmul qword ptr [0x66eb38]`). A plain
// `extern double` of our own gets the direct absolute-address encoding.
extern double g_0066eb38_d;

class Gamma { public:
    void on_button_clicked(int);
};

// MNEMONIC_ONLY, closest reached: every mnemonic and every byte agrees except
// one register choice. The two special cases (-2, -1) are written negated and
// nested per the source-form rule above, which reproduces BOTH duplicated
// call-and-return tails the original has (one inline in the a1==-1 branch,
// one shared by the a1==-2 and "neither" paths) - first tail loads the vtable
// pointer into eax, matching. The SECOND tail's vtable-pointer load compiles
// to `mov edx, [ecx]; call [edx+0xe8]` here against the original's `mov eax,
// [ecx]; call [eax+0xe8]` - a pure register-allocation choice at that
// two-predecessor merge point. Tried: a `self` local shared above both
// branches, a `self` local re-declared inside each branch, no local at all
// (inline reinterpret_cast at each use), and a named `VCall *vt` local for
// both call sites - all four produced the identical eax/edx split, so this
// reads as a backend register-allocation artifact of the CFG shape rather
// than something this source form controls.
void Gamma::on_button_clicked(int a1) {
    if (a1 != -2) {
        if (a1 == -1) {
            char *self = reinterpret_cast<char *>(this);
            *reinterpret_cast<double *>(self + 0xee8) =
                static_cast<double>(*reinterpret_cast<int *>(self + 0x191c)) *
                g_0066eb38_d;
            VCall *vt = reinterpret_cast<VCall *>(this);
            vt->slot058();
            return;
        }
    } else {
        char *self = reinterpret_cast<char *>(this);
        *reinterpret_cast<double *>(self + 0xee8) =
            *reinterpret_cast<float *>(self + 0xee4);
    }
    VCall *vt = reinterpret_cast<VCall *>(this);
    vt->slot058();
}
