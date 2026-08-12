// ORIGINAL: 0x005396D0 FILE
// name      ?net_withdrawal@@YAXHHH@Z
// size      104 bytes
// spans     0x005396D0-0x00539738
// prototype 
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005315C0 0x00548F80 0x00592EE0 0x006262F0
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x005396D0
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
