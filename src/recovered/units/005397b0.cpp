// ORIGINAL: 0x005397B0 FILE
// name      ?net_double_cross@@YAXHHHH@Z
// size      106 bytes
// spans     0x005397B0-0x0053981A
// prototype 
// callers   2   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0050BCC0 0x005315C0 0x00592EE0 0x006262F0
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE, not as a claim. Nothing reads this directory:
// it is on no ratchet, in no build, and scored by no collect.
//
// address        0x005397B0
// measured tier  NO_COMPILE
// refusal        u005397b0.cpp(13) : error C2065: 'g_0093f660' : undeclared identifier u005397b0.cpp(13) : error C2100: illegal indirection u005397b0.cpp(14) : error C2065: 'log
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/005397b0/unit.cpp
// and score it with tools/agent_brief.py.
// 0x005397B0  ?net_double_cross@@YAXHHHH@Z  (106 bytes)
//
// Same shape as net_withdrawal (0x005396d0) one address over, except this
// one's `void` return in the catalogue DOES match the disassembly: neither
// path sets `eax` to a meaningful value before `ret`, unlike net_withdrawal
// which returns 1 or a callee's result.
//
// `g_0068d53c` (string literal) and `g_0093cd90` (NetDaemon instance) are
// both plain immediates here too - pushed/loaded as-is, no arithmetic - so
// the given `static int *const` globals, cast at the point of use, reproduce
// the encoding without an `extern` override.
void __cdecl net_double_cross(int a1, int a2, int a3, int a4) {
    if (*g_0093f660) {
        log_say(reinterpret_cast<char *>(g_0068d53c), a1, a2, a3);
        message_data(0x2440, 0, a1, a2, a3, 0);
        if (a4 != 0) {
            reinterpret_cast<NetDaemon *>(g_0093cd90)->await_diplo(0x440);
        }
    } else {
        double_cross(a1, a2, a3);
    }
}
