// ORIGINAL: 0x005396D0 FILE
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005396D0
// name           ?net_withdrawal@@YAXHHH@Z
// size           104 bytes
// measured tier  NO_COMPILE
// refusal        u005396d0.cpp(14) : error C2065: 'g_0093f660' : undeclared identifier u005396d0.cpp(14) : error C2100: illegal indirection u005396d0.cpp(15) : error C2065: 'log
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005396d0/unit.cpp
// and score it with tools/agent_brief.py.
// 0x005396D0  ?net_withdrawal@@YAXHHH@Z  (104 bytes)
//
// The catalogue's `void` return disagrees with the disassembly: both paths
// set `eax` before `ret` (1 in the log/message/diplo branch, the
// `do_withdrawal` result otherwise), and there is no callee-pop, so this is
// `int __cdecl`, not `void __cdecl`.
//
// `g_0068d514` (a string literal's address) and `g_0093cd90` (a NetDaemon
// instance's address) are both used as plain immediates - pushed as an
// argument and loaded into `ecx` for a call, never added to or indexed -
// so the given `static int *const` globals, cast at the point of use, are
// the right form; no `extern` override needed.
int __cdecl net_withdrawal(int a1, int a2, int a3) {
    if (*g_0093f660) {
        log_say(reinterpret_cast<char *>(g_0068d514), a1, a2, 0);
        message_data(0x244a, 0, a1, a2, 0, 0);
        if (a3 != 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x44a);
        }
        return 1;
    }
    return do_withdrawal(a1, a2);
}
