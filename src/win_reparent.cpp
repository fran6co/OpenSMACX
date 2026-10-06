#include "stdafx.h"
#include "win.h"

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
        (*reinterpret_cast<const uint8_t *>(&owner->iFlags_) &
         WinFlagClipToParent) == 0) {
        const HWND parent = GetParent(hwnd);
        if (parent == WinReparentMatchParent) {
            SetParent(hwnd, WinReparentTargetParent);
        }
    }
    return 1;
}
