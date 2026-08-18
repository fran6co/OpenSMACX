// ORIGINAL: 0x006118F0 sub_6118f0 0x006118F0-0x00611901;0x006117E0-0x0061181E FILE
// size      79 bytes
// prototype 
// callers   0   call targets   4
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4DD0 0x00608E10 0x00610120 0x0064557F
// PRESERVED UNIT - measured SHARED_TAIL.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x006118F0
// measured tier  SHARED_TAIL
// refusal        1 span(s) are COMDAT-folded and claimed by another function; no per-function verdict is well defined
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/006118f0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: sub_6118f0  at 0x006118F0  (79 bytes)
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

// ---- callees, declared and never defined (a definition would be inlined) ----
class Dialog { public:
    ~Dialog();
};
class GraphicWin { public:
    ~GraphicWin();
};
class SpriteBox { public:
    ~SpriteBox();
};
int __cdecl fn_0064557f();

// SHARED_TAIL, confirmed by scoring the UNTOUCHED placeholder before
// writing anything here: `tools/verify_recovered_function.py` reports
// "1 span(s) are COMDAT-folded and claimed by another function; no
// per-function verdict is well defined" for this address regardless of
// body content, because its catalogued span is discontiguous -
// 0x006118F0-0x00611901 (this adjustor) plus 0x006117E0-0x0061181E (a
// destructor body a DIFFERENT vtable slot also reaches directly). No
// C++ shape changes that classification, so this is not chased further
// - see the "COMDAT glue" wall in the brief. The body below is still a
// faithful translation for readability/coverage: `sub ecx,[ecx-0x448];
// sub ecx,0x444` is a virtual-base offset adjustment, then the shared
// tail at 0x6117E0 re-derives the true object base (`adj-0x8c`) and
// runs a `scalar deleting destructor` over SpriteBox/Dialog/GraphicWin,
// freeing the object when the low bit of the stack flag is set - the
// receiver reads `[ecx+N]` with no stack arg for `this`, so this is a
// __thiscall member (the placeholder's `__stdcall sub_6118f0(int)`
// free-function signature does not match the evidence).
class Sub6118f0Base { public:
    int deleting_dtor_thunk(unsigned int flags);
};

int Sub6118f0Base::deleting_dtor_thunk(unsigned int flags) {
    char *self = reinterpret_cast<char *>(this);
    int voff = *reinterpret_cast<int *>(self - 0x448);
    char *adj = self - voff - 0x444;
    char *base = adj - 0x8c;
    reinterpret_cast<SpriteBox *>(base + 0x8c)->~SpriteBox();
    reinterpret_cast<Dialog *>(base + 0xaa4)->~Dialog();
    reinterpret_cast<GraphicWin *>(base + 0x8c)->~GraphicWin();
    if (flags & 1) {
        fn_0064557f();
    }
    return reinterpret_cast<int>(base);
}
