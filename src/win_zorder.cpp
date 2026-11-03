#include "stdafx.h"
#include "win.h"

// ===== homed from src/recovered/units/005f5020.cpp =====

// ORIGINAL: 0x005F5020 ?update_zorder@Win@@QAAXXZ 0x005F5020-0x005F5080 FILE BYTE_EXACT
// symbol    ?update_zorder@Win@@SAXXZ
// size      96 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F4EC0
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005F5020
// measured tier  NO_COMPILE
// refusal        u005f5020.cpp(29) : error C2653: 'Win' : is not a class or namespace name u005f5020.cpp(31) : error C2065: 'WinRootCount' : undeclared identifier u005f5020.cpp(31
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005f5020/unit.cpp
// and score it with tools/agent_brief.py.
// 0x005F5020  ?update_zorder@Win@@QAAXXZ  (96 bytes)
//
// `QAAXXZ`: public, __cdecl, void, no params - a STATIC member (no `this` is
// ever read; every operand is one of the five fixed globals or the array at
// WinRootWindows). The stale on-disk scaffold had this as `fn_005f5020(Win* a1)`;
// the fresh brief's contract head (`void __cdecl Win::update_zorder()`)
// matches the disassembly, so that is what is defined here.
//
// The initial `mov eax, dword ptr [0x9b7b34]` (the loop bound) happens
// BEFORE the `mov dword ptr [0x9b7b30], ebp` store in the original, even
// though the store is unconditional and the load only feeds the guard below
// it. Writing `WinZOrderCount = 0;` before reading the count reordered the two
// (store first) under /O2; reading the count into a local FIRST, in source
// order, reproduces the original ordering exactly. The do-while's own
// continuation test re-reads `WinRootCount` fresh each iteration (matching
// the original's second `mov eax, [0x9b7b34]` at the bottom of the loop) -
// only the ONE-TIME guard above the loop uses the cached local.
//
// `target != 0 && target == arr[i]` short-circuits to exactly the original's
// two-branch `cmp ecx,ebp / je skip; cmp ecx,[esi] / jne skip` - no
// restructuring needed there.
//
// WinRootWindows (the Win* array base) did NOT need the extern-array override
// the general lever warns about: the given `static Win *const g = ...`
// (cast at the point of use, not redeclared) already reproduces the
// register-walked indexing here - the lever's "address itself does work"
// failure mode did not reproduce for this walk. Recorded so the next agent
// does not re-pay for testing it.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?update_zorder@Win@@QAAXXZ  at 0x005F5020  (96 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.
void __cdecl Win::update_zorder() {
    int i = 0;
    int n = WinRootCount;
    WinZOrderCount = 0;
    if (n > 0) {
        Win **arr = WinRootWindows;
        Win *target = WinZOrderWindow;
        do {
            if (target != 0 && target == arr[i]) {
                WinZOrderCount = 0;
                WinZOrderFlag = 0;
            }
            if (arr[i]->iSomeFlag_ & 1) {
                recurse_zorder(arr[i]);
                target = WinZOrderWindow;
            }
            i++;
        } while (i < WinRootCount);
    }
}
