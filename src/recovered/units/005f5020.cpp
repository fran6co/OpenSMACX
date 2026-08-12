// ORIGINAL: 0x005F5020 FILE
// name      ?update_zorder@Win@@QAAXXZ
// size      96 bytes
// spans     0x005F5020-0x005F5080
// prototype 
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005F4EC0
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005F5020
// measured tier  NO_COMPILE
// refusal        u005f5020.cpp(29) : error C2653: 'Win' : is not a class or namespace name u005f5020.cpp(31) : error C2065: 'g_009b7b34' : undeclared identifier u005f5020.cpp(31
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
// g_009b6e48). The stale on-disk scaffold had this as `fn_005f5020(Win* a1)`;
// the fresh brief's contract head (`void __cdecl Win::update_zorder()`)
// matches the disassembly, so that is what is defined here.
//
// The initial `mov eax, dword ptr [0x9b7b34]` (the loop bound) happens
// BEFORE the `mov dword ptr [0x9b7b30], ebp` store in the original, even
// though the store is unconditional and the load only feeds the guard below
// it. Writing `*g_009b7b30 = 0;` before reading the count reordered the two
// (store first) under /O2; reading the count into a local FIRST, in source
// order, reproduces the original ordering exactly. The do-while's own
// continuation test re-reads `*g_009b7b34` fresh each iteration (matching
// the original's second `mov eax, [0x9b7b34]` at the bottom of the loop) -
// only the ONE-TIME guard above the loop uses the cached local.
//
// `target != 0 && target == arr[i]` short-circuits to exactly the original's
// two-branch `cmp ecx,ebp / je skip; cmp ecx,[esi] / jne skip` - no
// restructuring needed there.
//
// g_009b6e48 (the Win* array base) did NOT need the extern-array override
// the general lever warns about: the given `static Win *const g = ...`
// (cast at the point of use, not redeclared) already reproduces the
// register-walked indexing here - the lever's "address itself does work"
// failure mode did not reproduce for this walk. Recorded so the next agent
// does not re-pay for testing it.
void __cdecl Win::update_zorder() {
    int i = 0;
    int n = *g_009b7b34;
    *g_009b7b30 = 0;
    if (n > 0) {
        Win **arr = reinterpret_cast<Win **>(g_009b6e48);
        Win *target = *reinterpret_cast<Win **>(g_009b7a6c);
        do {
            if (target != 0 && target == arr[i]) {
                *g_009b7b30 = 0;
                *g_009b7a78 = 0;
            }
            if (arr[i]->iSomeFlag_ & 1) {
                recurse_zorder(arr[i]);
                target = *reinterpret_cast<Win **>(g_009b7a6c);
            }
            i++;
        } while (i < *g_009b7b34);
    }
}
