#include "stdafx.h"
#include "win.h"

// SDK window-parent relationships; no original-address function adapters.
// RECOVERY_IMPORT: v1 __imp__GetParent@4 USER32.dll name:GetParent
// RECOVERY_IMPORT: v1 __imp__SetParent@8 USER32.dll name:SetParent

// These are distinct HWND-valued dwords. Their starts are observed;
// extent, setup and lifetime remain unresolved.
// RECOVERY_OBJECT: v1 0x009B7B40 ?WinReparentMatchParent@@3PAUHWND__@@A
HWND WinReparentMatchParent;
// RECOVERY_OBJECT: v1 0x009B7B44 ?WinReparentTargetParent@@3PAUHWND__@@A
HWND WinReparentTargetParent;

// ORIGINAL 0x005F4D50 is a free stdcall HWND adapter. The original
// reads its second stack argument only by popping it in ret 8.
// Historical seed catalog (inactive): // prior-original: 0x005F4D50 sub_5f4d50 0x005F4D50-0x005F4D90 FILE historical-byte-exact
// Historical seed catalog (inactive): // symbol    ?sub_5f4d50@@YGHHH@Z
// Historical seed catalog (inactive): // size      64 bytes
// Historical seed catalog (inactive): // prototype
// Historical seed catalog (inactive): // callers   0   call targets   0
// Historical seed catalog (inactive): // kind      game
// Historical seed catalog (inactive): // flags     hidden;sp_ready;purged_ok
// Historical seed catalog (inactive): // calls     (none)
// Historical seed catalog (inactive): // indirect  0x005F4D58 0x005F4D6C 0x005F4D81
// Historical seed catalog (inactive): // PRESERVED UNIT - measured historical-byte-exact.
// Historical seed catalog (inactive): //
// Historical seed catalog (inactive): // Kept for COVERAGE. This directory IS on the ratchet: every file here
// Historical seed catalog (inactive): // carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// Historical seed catalog (inactive): // it, and 336 of the 1,108 now carry a historical-byte-exact claim - better than a
// Historical seed catalog (inactive): // quarter of the project's total. It is still in no build; the earlier
// Historical seed catalog (inactive): // header said "on no ratchet", which stopped being true when the map
// Historical seed catalog (inactive): // moved into src/ and was still being written into new files.
// Historical seed catalog (inactive): //
// Historical seed catalog (inactive): // address        0x005F4D50
// Historical seed catalog (inactive): // measured tier  historical-byte-exact
// Historical seed catalog (inactive): //
// Historical seed catalog (inactive): // The WHOLE unit as measured, scaffolding included: for the units
// Historical seed catalog (inactive): // that are byte-exact yet refuse extraction, the agent tuned the
// Historical seed catalog (inactive): // emitted scaffolding and the body alone will not reproduce the
// Historical seed catalog (inactive): // verdict. To resume, copy everything below back over
// Historical seed catalog (inactive): //   build/byte-match/005f4d50/unit.cpp
// Historical seed catalog (inactive): // and score it with tools/agent_brief.py.
// Historical seed catalog (inactive): // GENERATED SKELETON - tools/emit_translation_unit.py
// Historical seed catalog (inactive): // subject: sub_5f4d50  at 0x005F4D50  (64 bytes)
// Historical seed catalog (inactive): //
// Historical seed catalog (inactive): // A VERIFICATION ARTIFACT, not product source: classes are opaque and
// Historical seed catalog (inactive): // globals are bound to fixed addresses, because both are byte-visible
// Historical seed catalog (inactive): // and both differ from the style src/ is written in.
// Historical seed catalog (inactive): //
// Historical seed catalog (inactive): // VC6 DIALECT - this must compile under cl 12.00.8168, which is the
// Historical seed catalog (inactive): // only compiler this project builds with. Avoid: auto, nullptr, constexpr,
// Historical seed catalog (inactive): // static_assert, enum class, range-for, lambdas, long long, <cstdint>,
// Historical seed catalog (inactive): // and declaring `int i` twice in one function (VC6 leaks for-scope).
// Historical seed catalog (inactive): // static_cast/reinterpret_cast are fine and are the right spelling.
// Historical seed catalog (inactive): //
// Historical seed catalog (inactive): // SOURCE-FORM RULES, each one learned by a fan-out agent that lost
// Historical seed catalog (inactive): // attempts to it. They are here rather than in a prompt so the next
// Historical seed catalog (inactive): // agent does not rediscover them:
// Historical seed catalog (inactive): //
// Historical seed catalog (inactive): //  * A ternary is not an if. `x ? a : b` lowers to `jne` with the arms
// Historical seed catalog (inactive): //    swapped; `if (x) {...}` gives `je`.
// Historical seed catalog (inactive): //  * VC6 NEVER hoists a same-polarity guard's body out of line. To get
// Historical seed catalog (inactive): //    two special cases branching FAR away, write them negated and
// Historical seed catalog (inactive): //    NESTED - `if (a != X) { if (a != Y) { return D; } ...; }` - not as
// Historical seed catalog (inactive): //    sequential guard clauses, which inline each body instead.
// Historical seed catalog (inactive): //  * VC6 rejects `__thiscall` on a free function pointer (C4234). For
// Historical seed catalog (inactive): //    an indirect virtual call use the generated VCall shim below; for a
// Historical seed catalog (inactive): //    thiscall function POINTER, take a member-function pointer of a
// Historical seed catalog (inactive): //    dummy class instead of spelling the convention.
// Historical seed catalog (inactive): //  * Loop form is visible: while / do-while / for lower differently,
// Historical seed catalog (inactive): //    and so does counting up versus down.
// Historical seed catalog (inactive): //  * If the original dedicates a callee-saved register to a constant
// Historical seed catalog (inactive): //    across the whole body (an extra `push ebx`/`push edi` in the
// Historical seed catalog (inactive): //    prologue), it had enough register pressure to do so. That is a
// Historical seed catalog (inactive): //    hard case - the tool reports a similarity ratio so you can tell a
// Historical seed catalog (inactive): //    near miss from a wrong body.
// Historical seed catalog (inactive): address=0x005F4D50
// Historical seed catalog (inactive): mode=file; state=implemented
// Historical seed catalog (inactive): name=sub_5f4d50; symbol=?sub_5f4d50@@YGHHH@Z
// Historical seed catalog (inactive): spans=((6245712, 6245776),)
// Historical seed catalog (inactive): kind='game'; exclusion=''
// Historical seed catalog (inactive): byte_exact=True; semantic=False
// Historical seed catalog (inactive): recipe=verbatim
// Historical seed catalog (inactive): levers=()
// Historical seed catalog (inactive): ruled_out=()
// Historical seed catalog (inactive): unrecoverable=()
// Historical seed catalog (inactive): deferred=()
// ORIGINAL: 0x005F4D50 sub_5f4d50 0x005F4D50-0x005F4D90 BYTE_EXACT
// symbol    ?win_reparent_window@@YGHPAUHWND__@@H@Z
int __stdcall win_reparent_window(HWND hwnd, int unused) {
    (void)unused;
    Win *const owner = reinterpret_cast<Win *>(
        GetWindowLongA(hwnd, GWL_USERDATA));
    if (owner != 0 &&
        (owner->iFlags_ & WinFlagClipToParent) == 0) {
        const HWND parent = GetParent(hwnd);
        if (parent == WinReparentMatchParent) {
            SetParent(hwnd, WinReparentTargetParent);
        }
    }
    return 1;
}

// Window hierarchy queries share this Win relationship implementation.
// Their original translation-unit filenames remain unproved.

/*
Purpose: Determine whether this window and every ancestor are visible.
// ORIGINAL: 0x005F7E90 ?is_visible@Win@@QAEHXZ 0x005F7E90-0x005F7EB6 BYTE_EXACT
// compile_context src/win_reparent.cpp
// size      38 bytes
// prototype int (__thiscall ?is_visible@Win@@QAEHXZ)(Win* this)
// callers   120   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F7E90
// notes     Runtime redirect installed by DllMain after byte-signature validation
Status: Complete
*/
int Win::is_visible() {
    if (!(iSomeFlag_ & 1U)) {
        return 0;
    }
    // A detached window is visible on its own; otherwise the whole parent
    // chain must be visible, which the legacy body walks recursively.
    if (win_parent_ && !win_parent_->is_visible()) {
        return 0;
    }
    return 1;
}

/*
Purpose: Report whether a window is anywhere below this one in the child
         tree, searching depth first. A direct child counts, and so does any
         descendant of one.
// ORIGINAL: 0x005ECE20 ?is_descendant@Win@@QAEHPAVWin@@@Z 0x005ECE20-0x005ECE73
// compile_context src/win_reparent.cpp
// symbol    ?is_descendant@Win@@QAEHPAV1@@Z
// size      83 bytes
// prototype int (__thiscall ?is_descendant@Win@@QAEHPAVWin@@@Z)(Win* this, Win*)
// callers   4   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECE20
Return Value: 1 when the candidate is a descendant, 0 otherwise
Status: Complete
Verification note: hoisting the count out of the loop is an EQUIVALENT mutant
         and will survive the sweep. The original reloads it every iteration,
         so this transcription does too, but the whole call tree here is
         read-only - nothing reachable from is_descendant writes to any
         object - so no execution can observe the reload. It is kept because
         it is what the bytes do, not because a test pins it. If this function
         ever gains a seam, that changes and the reload becomes testable.
         The absent null check on children_[index] is likewise faithful: the
         original dereferences the slot unconditionally at 0x005ECE2E, so a
         null slot faults in both.

// LEVER: the image walks `children_` with an incrementing POINTER
// (`lea esi, [ebx+0x1a4]` once, `add esi, 4` per iteration, `mov ecx,[esi]`)
// rather than re-deriving `children_[index]` from a scaled index each time.
// Spelling it as `Win *const *child = children_; ... ++child;` instead of
// `children_[index]` moved agreement from 2/37 to 17/37 (0.691 -> 0.727
// similar) at `/c /O2 /Gy /GR- /GX`. `index` is kept only for the trailing
// `index >= count` bound check, which the image still does with `cmp
// edi, eax` against a separately-incremented edi.
//
// TRIED: hoisting `int index = 0;` to sit between the `count =
// child_count_` load and the `count <= 0` check (matching the image's
// `xor edi, edi` appearing before `test eax, eax`) - this makes VC6 pick
// edi instead of ebx for `this` and is WORSE (10/37). The remaining
// divergence (a missing early `xor edi, edi` and the resulting jump-target
// shift) did not yield to reordering the two post-count-check statements
// either; both orders compile identically.
//
// TRIED: loop shape, via try_spellings. `do { ... } while (index <
// count);` ties the committed `for (;;) { ...; if (index >= count) return
// 0; }` exactly (17/37, same bytes) - the image's single shared "return 1"
// landing pad (reached by both `je` and the post-recursion `jne`) is not
// reproduced by either; this tree's compiler duplicates that block instead of
// sharing it. A top-tested `while (index < count) { ... }` is WORSE (9/37),
// and merging the two early-return tests into one `||` condition is worse
// still (5/37) - the image tests them separately, matching the existing
// two-statement shape already committed.
*/
// TRIED: 17 of 37, and two restructurings made it no better or worse.
// A `do { ... } while (index < count);` gets the image's BACKWARD `jl` in
// place of this form's forward `jge` + repeated compare, but scores the
// same 17 and adds a differing run. Merging the two `return 1` arms into
// one `||` - the image has a single found-case epilogue where this emits
// two - collapses the loop instead: 4 of 37 at 32 instructions against the
// image's 37. What is left is instruction ORDER (the image zeroes edi
// before loading children_, this body after) and the duplicated epilogue,
// neither of which source order reached.
int Win::is_descendant(Win *candidate) {
    if (!candidate) {
        return 0;
    }
    int count = child_count_;
    if (count <= 0) {
        return 0;
    }
    Win *const *child = children_;
    int index = 0;
    for (;;) {
        if (*child == candidate) {
            return 1;
        }
        if ((*child)->is_descendant(candidate)) {
            return 1;
        }
        // Re-read, do NOT hoist. The original reloads the count at 0x005ECE50
        // after every recursive call returns, so a recursion that adds or
        // removes children is seen by the very next iteration. A loop-invariant
        // count is the obvious tidy-up and it changes behaviour.
        count = child_count_;
        ++index;
        ++child;
        if (index >= count) {
            return 0;
        }
    }
}

// ORIGINAL: 0x005ECDC0 ?UNK2@Win@@QAEHH@Z 0x005ECDC0-0x005ECE15 FILE
// compile_context src/win_reparent.cpp
// symbol    ?is_descendant_of@Win@@QAEHPAV1@@Z
// notes     PAV1@, not PAVWin@@: MSVC back-references the enclosing class
//           for a parameter of its own type.
// size      85 bytes
// prototype int (__thiscall ?UNK2@Win@@QAEHH@Z)(Win* this, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005ECDC0
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005ECDC0
// measured tier  MISMATCH
// divergence     4
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005ecdc0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK2@Win@@QAEHH@Z  at 0x005ECDC0  (85 bytes)
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


// TRIED: 4 of 38. Named and retyped from `UNK2(int)` - it walks
// `ancestor`'s children looking for `this`, so it is the mirror of
// is_descendant - and the members replaced a `char *` walk of 0x1a4/0x3fc.
// The bytes did not follow: 41 instructions against 38, with ebx and ebp
// swapped from the first instruction on, which is the same allocation wall
// its sibling is_descendant (0x005ECE20) sits behind at 17 of 37.
// Flattening the two guards into `ancestor == 0 || child_count_ <= 0`
// costs one more instruction and changes nothing else.
int Win::is_descendant_of(Win *ancestor) {
    if (ancestor != 0) {
        if (ancestor->child_count_ > 0) {
            Win *const *children = ancestor->children_;
            int i = 0;
            do {
                Win *child = *children;
                if (child == this) {
                    return 1;
                }
                if (is_descendant_of(child)) {
                    return 1;
                }
                ++i;
                ++children;
            } while (i < ancestor->child_count_);
        }
    }
    return 0;
}
